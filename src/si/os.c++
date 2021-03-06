#include "tau.h"
#include "../trace.h"

namespace tau
{
    namespace os
    {
        Lock::Lock( )
        {
            pthread_mutexattr_t attributes;
            ::pthread_mutexattr_init( &attributes );
            ::pthread_mutexattr_settype( &attributes, PTHREAD_MUTEX_ERRORCHECK );
            ::pthread_mutex_init( &m_lock, &attributes );
        }

        Lock::~Lock( )
        {
            ::pthread_mutex_destroy( &m_lock );
        }

        void Lock::lock( )
        {
            ::pthread_mutex_lock( &m_lock );
        }

        void Lock::unlock( )
        {
            ::pthread_mutex_unlock( &m_lock );
        }
        
        Condition::Condition()
            : m_count( 0 )
        {
            si::check( 
                ::pthread_cond_init( &m_cond, NULL )  
                ) ( "pthread_cond_init" );
                
                
            si::check( 
                ::pthread_mutex_init( &m_mutex, NULL ) 
                ) ( "pthread_mutex_init" );
        }
        
        Condition::~Condition()
        {
            ::pthread_cond_destroy( &m_cond );
            ::pthread_mutex_destroy( &m_mutex );
        }
        
        void Condition::wait( const Time* time )
        {
            ENTER();
            
            si::inc( &m_count );
            
            //
            //  lock the mutex
            //  
            ::pthread_mutex_lock( &m_mutex );
            //
            //  wait
            //
            if ( time )
            {
                struct timespec t;
                t.tv_sec = time->s();
                t.tv_nsec = ( time->us() - t.tv_sec * 1000000 ) * 1000;
                
                TRACE( "secs: %u", t.tv_sec );
                auto ret = ::pthread_cond_timedwait( &m_cond, &m_mutex, &t );
                TRACE( "return %d", ret );
            }
            else
            {
                ::pthread_cond_wait( &m_cond, &m_mutex );
            }
            
        }
        
        void Condition::signal()
        {
            ENTER();
            
            if ( !m_count )
            {
                throw Error();
            }
            
            TRACE( "count %d", m_count );    
            
            si::dec( &m_count );
            //
            //  signal
            //
            ::pthread_cond_signal( &m_cond );
            //
            //  unlock mutex
            //
            ::pthread_mutex_unlock( &m_mutex );
        }
        
        void Thread::start()
        {
            si::check( 
                    ::pthread_create( &m_handle, NULL, ( void *( * )( void* ) ) routine, ( void* ) this ) 
                    ) ( "thread create" );
            
            m_started = true;
        }
    
        void Thread::join() const
        {
            void* ret = NULL;
            int res = ::pthread_join( m_handle, &ret );
            
            
        }
    
        void Thread::routine( void* data )
        {
            auto instance = ( Thread* ) data;

            if ( instance )
            {
                instance->run( );
            }
            else
            {
                //
                //  should not get here
                //
                assert( false );
            }
        }
        
        ul Thread::id( )
        {
            return sys::id();
        }

        Process::Process( )
        : m_pid( 0 )
        {
            auto type = 0;
            streams( [ & ] ( Stream& s )
            {
                s = Stream( type );
                type ++;
            } );
        }
        
        void Process::start( const Data& command )
        {
            if ( m_pid )
            {
                return;
            }
            
            ENTER();
            
            m_command =  command;
            TRACE( "%s", command.c() );
            
            //
            //  establish pipes
            //
            streams( [ ] ( Stream& s ) { s.open(); } );
            //
            //  fork
            //
            auto pid = si::check( ::fork( ) )( "fork" );
            bool child = pid == 0;
            
            streams( [ & ] ( Stream& s ) { s.init( child ); } );
            
            if ( child )
            {
                ::execlp( "bash", "bash", "-c", command.c(), ( char* ) NULL );
                ::exit( -1 );
            }
            else
            {
                m_pid = pid;
            }
        } 
        
        void Process::stop()
        {
            if ( !m_pid )
            {
                return;
            }
            
            streams( [ ] ( Stream& s ) { s.close( ); } );
            signal( SIGKILL );
            m_pid = 0;
        }
        
        void Process::signal( int signal ) const
        {
            if ( m_pid )
            {
                TRACE( "sending signal %d to pid %d", signal, pid() );
                ::kill( m_pid, signal );
            }
        }
        
        long Process::code()
        {
            int code = 0;
            int result = ::waitpid( pid( ), &code, WNOHANG );
            
            if ( result != m_pid )
            {
                throw mem::mem().type< Error >();
            }
            
            return code;
        }

        void Process::Stream::open( )
        {
            si::check( ::pipe( m_fds ) )( "pipe" );
        }

        void Process::Stream::close( )
        {
            ENTER();
            
            if ( fd() )
            {
                ::close( readFd( ) );
                ::close( writeFd( ) );
                fs::File::assign( 0, false );
            }
        }

        void Process::Stream::init( bool child )
        {
            auto fd = 0;
            switch ( m_type )
            {
                case sys::In:
                    fd = writeFd( child );
                    break;

                case sys::Out:
                case sys::Err:
                    fd = readFd( child );
                    break;
            }

            if ( m_type == sys::In )
            {
                ::close( readFd( child ) );
            }
            if ( child )
            {
                ::dup2( fd, m_type );
                
                if ( m_type == sys::In )
                {
                    ::close( fd );
                }
            }
            
            fs::File::assign( fd, false );
        }
        
        Module::Module( const Data& path, const Data& entry )
        : m_handle( NULL ), m_entry( NULL )
        {
            Data name( path );
            
#ifdef __MACH__
            Data directory = fs::cwd();
            
            if ( path.find( "/" ) != -1 )
            {
                auto split = fs::split( path );
                fs::chdir( split.first );
                name = split.second;
            }
#endif
            m_handle = ::dlopen( name, RTLD_LAZY );
            
            if ( !m_handle )
            {
                throw Error( ::dlerror() );
            }
            
            if ( entry.length() )
            {
                m_entry = symbol( entry );
                if ( !m_entry )
                {
                    throw Error( ::dlerror( ) );
                }
            }
            
#ifdef __MACH__
            fs::chdir( directory );
#endif
        }
        
        Module::~Module(  )
        {
            if ( m_handle )
            {
                ::dlclose( m_handle );
            }
        }
        
        void* Module::symbol( const Data& name ) const
        {
            return ::dlsym( m_handle, name );
        }
        
        Signals* Signals::s_instance = NULL;
        Signals::Terminate* Signals::s_terminate = NULL;
        
        Signals::Signals( )
        {
            ::signal( SIGSEGV, signal );
            ::signal( SIGINT, signal );
            ::signal( SIGTERM,  signal );
            ::signal( SIGABRT, signal );
             
            ::signal( SIGPIPE, SIG_IGN );
        }
        
        void Signals::assign( Signals* signals )
        {
            s_instance = signals;
        }
        
        void Signals::signal( int signal )
        {
            SENTER();
            STRACE( "signal %d", signal );
            
            auto terminated = false;
            
            if ( signal == SIGINT )
            {
                signal = SIGTERM;
            }
            
            switch ( signal )
            {
                case SIGSEGV:
                case SIGABRT:
                
                    sys::backtrace( 15 );
                    break;
                    
                 
                case SIGTERM:
                        if ( !s_terminate )
                        {
                            s_terminate = new Terminate();
                            s_terminate->join();
                            instance().terminated();
                            terminated = true;
                        }
                        
                break;
            }
            
            if ( !terminated )
            {
                instance().aborted( ( What ) signal );    
            }
            
        }
    }
}