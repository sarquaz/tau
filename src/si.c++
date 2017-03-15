#include "si.h"
#include "trace.h"
#include "types.h"


#ifdef __MACH__
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace tau
{
    namespace si
    {
        void* swap( void** target, void* value )
        {
            void* oldValue = __sync_fetch_and_add( target, 0 );
            return __sync_val_compare_and_swap( target, oldValue, value );
        }

        ui inc( ui* target )
        {
            ui result = __sync_fetch_and_add( target, 1 );
            return result + 1;
        }

        ui dec( ui* target )
        {
            ui result = __sync_fetch_and_sub( target, 1 );
            return result - 1;

        }

        ui millis( )
        {
            timeval time;
            ::gettimeofday( &time, NULL );
            
            return ( ( time.tv_sec ) * 1000 + time.tv_usec / 1000 );
        }
        
        ul check::operator()( const char* format, ... )
        {
            if ( errno == skip || result != -1 )
            {
                return result;
            }

            Error error;
            EPRINT( error.message, format );
            error.message( ", errno %d", errno );
            throw error;
        }
        
        void backtrace( ui length, Stream stream )
        {
            void* array[ length ];
            auto size = ::backtrace( array, length );
            ::backtrace_symbols_fd( array, size, stream );
        }
        
        void out( const Data& data, Stream stream  )
        {
            auto wrote = ::write( stream, data, data.length() );
        }
 
        
//         void Fs::chdir( const Data& path )
//         {
//             STRACE( "%s", path.c() );
//             auto r = ::chdir( path );
//         }
//
//         Data Fs::cwd( )
//         {
//             Data data;
//             data.space( 0x200 );
//             auto c = ::getcwd( data, data.size() );
//
//             return data;
//         }
//
//
//         Fs::Pair Fs::split( const Data& path )
//         {
//             Pair pair;
//             auto offset = 0;
//
//             for ( ;; )
//             {
//                 auto found = path.find( "/", offset );
//
//                 if ( found == -1 )
//                 {
//                     break;
//                 }
//                 offset = found + 1;
//             }
//
//             if ( offset )
//             {
//                 pair.first = tau::Data( path, offset - 1 );
//                 pair.second = tau::Data( path + offset );
//             }
//             else
//             {
//                 pair.first = path;
//             }
//
//             return pair;
//
//         }
//
//         Fs::Info Fs::info( const Data& name )
//         {
//             Info info;
//             check( ::stat( name, info ) )( "stat" );
//             return info;
//         }
//
//         Fs::Type Fs::Info::type() const
//         {
//             ENTER();
//
//             switch( stat.st_mode & S_IFMT )
//             {
//                 case S_IFDIR:
//                     return Dir;
//
//                 case S_IFREG:
//                     return File;
//
//             }
//
//             return File;
//         }
//
//         bool Fs::exists( const Data& name )
//         {
//             bool exists = false;
//             try
//             {
//                 info( name );
//                 exists = true;
//             }
//             catch( ... )
//             {
//
//             }
//
//             return exists;
//         }
//
//         Lock::Lock( )
//         {
//             pthread_mutexattr_t mutexAttributes;
//             pthread_mutexattr_init( &mutexAttributes );
//             pthread_mutexattr_settype( &mutexAttributes, PTHREAD_MUTEX_ERRORCHECK );
//             pthread_mutex_init( &m_lock, &mutexAttributes );
//         }
//
//         Lock::~Lock( )
//         {
//             ::pthread_mutex_destroy( &m_lock );
//         }
//
//         void Lock::lock( )
//         {
//             ::pthread_mutex_lock( &m_lock );
//         }
//
//         void Lock::unlock( )
//         {
//             ::pthread_mutex_unlock( &m_lock );
//         }
//
//         Semaphore::Semaphore( )
//         {
// #ifdef __MACH__
//             m_handle = ::sem_open( Data::get(), O_CREAT, S_IRUSR | S_IWUSR, 0 );
// #else
//             ::sem_init( &m_handle, 0, 0 );
// #endif
//         }
//
//         Semaphore::~Semaphore( )
//         {
//
// #ifdef __MACH__
//             ::sem_destroy( m_handle );
// #else
//             ::sem_destroy( &m_handle );
// #endif
//         }
//
//         void Semaphore::post( )
//         {
// #ifdef __MACH__
//             ::sem_post( m_handle );
// #else
//             ::sem_post( &m_handle );
// #endif
//         }
//
//         void Semaphore::wait( )
//         {
// #ifdef __MACH__
//             ::sem_wait( m_handle );
// #else
//             ::sem_wait( &m_handle );
// #endif
//         }
//
//         void Thread::start( )
//         {
//             pthread_attr_t attr;
//             ::pthread_attr_init( &attr );
//             ::pthread_attr_setstacksize( &attr, 1024 * 1024 * 4 );
//
//             check(
//                     ::pthread_create( &m_handle, &attr, ( void *( * )( void* ) ) routineStatic, this )
//                     ) ( "thread create" );
//
//             ::pthread_attr_destroy( &attr );
//         }
//
//         void Thread::routineStatic( void* data )
//         {
//             Thread* instance = ( Thread* ) data;
//
//             if ( instance )
//             {
//                 instance->routine( );
//             }
//         }
//
//         void Thread::join( ) const
//         {
//             void* ret = NULL;
//             ::pthread_join( m_handle, &ret );
//         }
//
//         Process::Process( )
//         : m_pid( 0 )
//         {
//             auto type = 0;
//             streams( [ & ] ( Stream& s )
//             {
//                 s = Stream( type );
//                 type ++;
//             } );
//         }
//
//         void Process::start( const Data& command )
//         {
//             if ( m_pid )
//             {
//                 return;
//             }
//
//             ENTER();
//
//             m_command =  command;
//             TRACE( "%s", command.c() );
//
//             streams( [ ] ( Stream& s ) { s.open(); } );
//
//             auto pid = check( ::fork( ) )( "fork" );
//             bool child = pid == 0;
//
//             streams( [ & ] ( Stream& s ) { s.init( child ); } );
//
//             if ( child )
//             {
//                 ::execlp( "bash", "bash", "-c", command.c(), ( char* ) NULL );
//                 ::exit( -1 );
//             }
//             else
//             {
//                 m_pid = pid;
//             }
//         }
//
//         void Process::stop()
//         {
//             if ( !m_pid )
//             {
//                 return;
//             }
//
//             streams( [ ] ( Stream& s ) { s.close( ); } );
//             signal( SIGKILL );
//             m_pid = 0;
//         }
//
//         void Process::signal( int signal ) const
//         {
//             if ( m_pid )
//             {
//                 TRACE( "sending signal %d to pid %d", signal, pid() );
//                 ::kill( m_pid, signal );
//             }
//         }
//
//         int Process::code()
//         {
//             int code = 0;
//             int result = ::waitpid( pid( ), &code, WNOHANG );
//
//             if ( result != m_pid )
//             {
//                 throw Error();
//             }
//
//             TRACE( "%d", result );
//
//             return code;
//         }
//
//         void Process::Stream::open( )
//         {
//             check( ::pipe( fds ) )( "pipe" );
//         }
//
//         void Process::Stream::close( )
//         {
//             if ( fd )
//             {
//                 ::close( readFd( ) );
//                 ::close( writeFd( ) );
//                 fd = 0;
//             }
//         }
//
//         void Process::Stream::init( bool child )
//         {
//             switch ( type )
//             {
//                 case File::In:
//                     fd = writeFd( child );
//                     break;
//
//                 case File::Out:
//                 case File::Err:
//                     fd = readFd( child );
//                     break;
//             }
//
//             if ( type == File::In )
//             {
//                 ::close( readFd( child ) );
//             }
//             if ( child )
//             {
//                 ::dup2( fd, type );
//                 if ( type == File::In )
//                 {
//                     ::close( fd );
//                 }
//             }
//         }
//
//         Module::Module( const Data& path, const Data& entry )
//         : m_handle( NULL ), m_entry( NULL )
//         {
//             Data name( path );
//
// #ifdef __MACH__
//             Data directory = Fs::cwd();
//
//             if ( path.find( "/" ) != -1 )
//             {
//                 auto split = Fs::split( path );
//                 Fs::chdir( split.first );
//                 name = split.second;
//             }
// #endif
//             m_handle = ::dlopen( name, RTLD_LAZY );
//
//             if ( !m_handle )
//             {
//                 throw Error( ::dlerror() );
//             }
//
//             if ( entry.length() )
//             {
//                 m_entry = symbol( entry );
//                 if ( !m_entry )
//                 {
//                     throw Error( ::dlerror( ) );
//                 }
//             }
//
// #ifdef __MACH__
//             Fs::chdir( directory );
// #endif
//         }
//         Module::~Module(  )
//         {
//             if ( m_handle )
//             {
//                 ::dlclose( m_handle );
//             }
//         }
//
//         void* Module::symbol( const Data& name ) const
//         {
//             return ::dlsym( m_handle, name );
//         }
//
//         Signals* Signals::s_instance = NULL;
//         Signals::Terminate* Signals::s_terminate = NULL;
//
//         Signals::Signals( )
//         {
//             ::signal( SIGSEGV, signal );
//             ::signal( SIGINT, signal );
//             ::signal( SIGTERM,  signal );
//             ::signal( SIGABRT, signal );
//
//             ::signal( SIGPIPE, SIG_IGN );
//         }
//
//         void Signals::assign( Signals* signals )
//         {
//             s_instance = signals;
//         }
//
//         void Signals::signal( int signal )
//         {
//             SENTER();
//
//             if ( signal == SIGINT )
//             {
//                 signal = SIGTERM;
//             }
//
//             switch ( signal )
//             {
//                 case SIGSEGV:
//                 case SIGABRT:
//
//                     backtrace( 15 );
//                     break;
//
//
//                 case SIGTERM:
//                         if ( !s_terminate )
//                         {
//                             s_terminate = new Terminate();
//                             s_terminate->join();
//                         }
//                 break;
//             }
//
//             instance().onAbort( ( What ) signal );
//         }
//
//         void Signals::Terminate::routine()
//         {
//             instance().onTerminate();
//         }
//
//         void Timer::start( const Interval& time )
//         {
//
//             if ( m_started )
//             {
//                 return;
//             }
//
//             ENTER();
//
//
// #ifdef __linux__
//
//             auto index = type( time );
//             auto& fd = m_fds[ index ];
//
//             if ( fd )
//             {
//                 m_index = index;
//                 m_started = true;
//                 m_time = time;
//
//                 if ( ( long ) m_time )
//                 {
//                     clock( m_time );
//                 }
//                 else
//                 {
//                     inc();
//                 }
//             }
//             else
//             {
//                 if ( index == First )
//                 {
//                     fd = check( ::timerfd_create( CLOCK_MONOTONIC, 0 ) )( "timerfd" );
//                 }
//                 else
//                 {
//                     fd = check( ::eventfd( 0, 0 ) )( "eventfd" );
//                 }
//
//                 start( time );
//             }
// #endif
//
// #ifdef __MACH__
//             m_started = true;
//             m_fds[ m_index ] = random( INT_MAX );
//             m_time = time;
// #endif
//         }
//
//         Timer::~Timer( )
//         {
//             ENTER();
//
// #ifdef __linux__
//             for ( auto i = 0; i < lengthof( m_fds ); i++ )
//             {
//                 auto fd = m_fds[ i ];
//
//                 if ( fd )
//                 {
//                     ::close( fd );
//                 }
//             }
// #endif
//         }
//
//
// #ifdef __linux__
//         void Timer::inc()
//         {
//             long long value = 1;
//             auto wrote = ::write( fd(), &value, sizeof( value ) );
//         }
//
//         struct Clock
//         {
//
//             typedef struct itimerspec Spec;
//             typedef struct timespec Time;
//
//             Spec spec;
//
//             Clock( const Interval& time )
//             {
//                 ::memset( &spec, 0, sizeof( spec ) );
//                 set( time );
//             }
//
//             operator Spec*( )
//             {
//                 return &spec;
//             }
//
//             void set( const Interval& time )
//             {
//                 field( spec.it_interval, time );
//                 field( spec.it_value, time );
//             }
//
//             void field( Time& field, const Interval& time )
//             {
//                 if ( time.infinite() )
//                 {
//                     field.tv_sec = INT_MAX;
//                 }
//                 else
//                 {
//                     long long secs = ( long long ) time / 1000000;
//
//                     field.tv_sec = secs;
//                     field.tv_nsec = ( ( long long ) time % 1000000 ) * 1000;
//
//                 }
//             }
//         };
//
//         void Timer::clock( const Interval& time )
//         {
//             Clock clock( time );
//
//             check( ::timerfd_settime( fd(), 0, clock, NULL ) )( "timerfd" );
//         }
//
// #endif
//
//         void Timer::stop( )
//         {
//
//             if ( !m_started )
//             {
//                 return;
//             }
//
//             ENTER();
//
// #ifdef __linux__
//             if ( ( long ) m_time )
//             {
//                 clock();
//             }
// #endif
//
//             m_started = false;
//         }
//
//         void Timer::timer()
//         {
//             if ( !m_started )
//             {
//                 return;
//             }
//
//             ENTER();
//
// #ifdef __linux__
//             long long value;
//             auto read = check( ::read( fd(), &value, sizeof( value ) ) )( "read" );
// #endif
//         }
//
//         int Queue::Set::filter() const
//         {
//             switch ( type )
//             {
//                 case Read:
// #ifdef __MACH__
//                     return EVFILT_READ;
// #else
//                     return EPOLLIN;
// #endif
//
//                 case Write:
// #ifdef __MACH__
//                     return EVFILT_WRITE;
// #else
//                     return EPOLLOUT;
// #endif
//
//                 case Event:
// #ifdef __MACH__
//                     return EVFILT_TIMER;
// #else
//                     return EPOLLIN;
// #endif
//             }
//         }
//
//         Queue::Type Queue::type( int filter )
//         {
//             switch( filter )
//             {
// #ifdef __MACH__
//                 case EVFILT_READ:
//                     return Read;
//
//                case EVFILT_WRITE:
//                     return Write;
//
//                case EVFILT_TIMER:
//                     return Event;
// #else
//                 case EPOLLIN:
//                     return Read;
//
//                case EPOLLOUT:
//                     return Write;
// #endif
//             }
//
//             return Read;
//         }
//
//         Queue::Queue( )
//         : m_stop( false ), m_handler( NULL )
//         {
//             ENTER();
// #ifdef __MACH__
//             m_queue = ::kqueue( );
// #else
//             m_queue = ::epoll_create1( 0 );
// #endif
//         }
//
//         void Queue::act( Action action, const Set& event, long time )
//         {
//             ENTER();
//             auto act = 0;
//
// #ifdef __MACH__
//             act = action == Add ? EV_ADD : EV_DELETE;
// #else
//             act = action == Add ? EPOLL_CTL_ADD : EPOLL_CTL_DEL;
// #endif
//
//             TRACE( "%s event with fd %d filter %d", action == Add ? "adding" : "removing", event.fd, event.filter() );
//
// #ifdef __MACH__
//             Kevent set;
//
//             auto flags = 0;
//             if ( action == Add )
//             {
//                 if ( event.type == Event )
//                 {
//                     flags = NOTE_USECONDS;
//                 }
//
//                 if ( time == Interval::Infinite )
//                 {
//                     flags = NOTE_SECONDS;
//                     time = INT_MAX;
//                 }
//
//                 TRACE( "setting time value %d", time );
//             }
//
//             EV_SET( &set, event.fd, event.filter( ), act, flags, time, event.data );
//             check( ::kevent( m_queue, &set, 1, NULL, 0, NULL ) )( "kevent" );
//
// #else
//             Eevent set;
//             set.events = event.filter( );
//             set.data.ptr = event.data;
//
//
//             check( ::epoll_ctl( m_queue, act, event.fd, &set ) )( "epoll_ctl" );
// #endif
//
//         }
//
//         void Queue::run()
//         {
//             for ( ;; )
//             {
//                 TRACE( "running event loop with %d events", length()  );
// #ifdef __MACH__
//                 auto changes = check( ::kevent( m_queue, NULL, 0, m_events, lengthof( m_events ), NULL ) )( "kevent" );
// #else
//                 auto changes = check( ::epoll_wait( m_queue, m_events, lengthof( m_events ), -1 ) )( "epoll");
// #endif
//                 for ( auto i = 0; i < changes; i++ )
//                 {
// #ifdef __MACH__
//                     Kevent& event = m_events[ i ];
//                     Set set( event.ident, event.udata );
//                     auto type = event.filter;
// #else
//                     Eevent& event = m_events[ i ];
//                     Set set( 0, event.data.ptr );
//                     auto type = event.events;
// #endif
//                     set.type = Queue::type( type );
//                     if ( m_handler )
//                     {
//                         ( this->*m_handler )( set );
//                     }
//                 }
//
//                 if ( m_stop )
//                 {
//                     return;
//                 }
//             }
//         }
//
//
//
//         File File::open( const Data& name )
//         {
//             int flags = 0;
//             int mode = 0;
//
//             if ( !Fs::exists( name ) )
//             {
//                 flags |= O_CREAT;
//                 mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
//             }
//             else
//             {
//                 flags |= O_APPEND;
//             }
//
//             flags |= O_RDWR;
//
//             return File( check(
//                     ::open( name, flags, mode )
//                     )( "file open, name %s, flags %d, mode %d", ( const char* ) name, flags, mode ) );
//         }
//
//         ui File::available( ) const
//         {
//             ui available = 0;
//             ::ioctl( fd( ), FIONREAD, &available );
//
//             return available;
//         }
//         ul File::write( const Data& data, ul offset ) const
//         {
//             seek( offset );
//             return check( ::write( fd( ), data, data.length( ) ) )( "write" );
//         }
//         ul File::read( Data& data, ul offset )
//         {
//             ENTER( );
//             seek( offset );
//             return check( ::read( fd( ), data, data.length( ) ) )( "read" );
//         }
//
//         void File::assign( Handle fd, bool nb  )
//         {
//             m_fd = fd;
//             if ( nb )
//             {
//                 check( ::fcntl( fd, F_SETFL, ::fcntl( fd, F_GETFL, 0 ) | O_NONBLOCK ) )( "fcntl" );
//             }
//         }
//
//         void File::close( )
//         {
//             ENTER();
//
//             if ( fd( ) )
//             {
//                 ::close( fd( ) );
//                 m_fd = 0;
//             }
//         }
//
//         void File::seek( ul offset ) const
//         {
//             if ( offset )
//             {
//                 check( ::lseek( fd( ), offset, SEEK_SET ) )( "seek" );
//             }
//         }
//
//         Link::Type Link::type( const Data& key )
//         {
//             if ( key == "local" )
//             {
//                 return Local;
//             }
//
//             return key == "tcp" ? Tcp : Udp;
//         }
//
//         Data Link::stype( Type type )
//         {
//             Data data;
//
//             switch ( type )
//             {
//                 case Local:
//                     data.add( "local" );
//                     break;
//
//                 case Udp:
//                     data.add( "udp" );
//                     break;
//
//                 case Tcp:
//                     data.add( "tcp" );
//                     break;
//             }
//
//             return data;
//         }
//
//         Link::Lookup::Lookup( Type type )
//         {
//             ::memset( &info, 0, sizeof( info ) );
//             info.ai_socktype = type == si::Link::Tcp ? SOCK_STREAM : SOCK_DGRAM;
//             info.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
//         }
//
//         Link::Address Link::Lookup::operator()( const Data& name )
//         {
//             Info* info;
//             auto result = ::getaddrinfo( name, NULL, *this, &info );
//             if ( !result )
//             {
//                  Address address( info );
//                  ::freeaddrinfo( info );
//                  return address;
//             }
//             else
//             {
//                 throw Error( ::gai_strerror( result ) );
//             }
//         }
//
//         void Link::Address::parse( )
//         {
//             auto peer = ( Peer* ) &a;
//
//             family = peer->sa_family;
//             const void* target = NULL;
//
//             if ( family == AF_INET )
//                 {
//                 Ip* s = ( Ip* ) peer;
//                 target = &s->sin_addr;
//                 port = s->sin_port;
//             }
//             else
//             {
//                 Ip6* s = ( Ip6* ) peer;
//                 target = &s->sin6_addr;
//                 port = s->sin6_port;
//             }
//
//             host.space( 0x100 );
//             ::inet_ntop( family, target, host, host.size() );
//         }
//
//         void Link::Address::parse( const Info& address )
//         {
//             family = address.ai_family;
//             type = address.ai_socktype == SOCK_STREAM ? Tcp : Udp;
//
//             if ( address.ai_family == AF_INET6 )
//             {
//                 length = sizeof( Ip6 );
//                 Ip6* addr = ip6( );
//                 addr->sin6_addr = ( ( Ip6* ) address.ai_addr )->sin6_addr;
//                 addr->sin6_family = address.ai_family;
//             }
//             else
//             {
//                 length = sizeof( Ip );
//                 Ip* addr = ip4( );
//                 addr->sin_addr = ( ( Ip* ) address.ai_addr )->sin_addr;
//                 addr->sin_family = address.ai_family;
//             }
//         }
//
//         Link::Address::Address( const Info* info )
//         : family( AF_INET ), type( Tcp ), port( 0 ), length( sizeof( a ) )
//         {
//             ::memset( &a, 0, length );
//
//             if ( info )
//             {
//                 parse( *info );
//             }
//         }
//
//         Link::Address::Address( Type _type,  const Data& _host, ui _port  )
//         : type( _type ), host( _host ), port( _port ), length( sizeof( a ) ), family( AF_INET )
//         {
//             ::memset( &a, 0, length );
//             if ( type != Local )
//             {
//                 setPort( port );
//             }
//             else
//             {
//                 local( host );
//             }
//         }
//
//         void Link::Address::local( const Data& name )
//         {
//             family = AF_LOCAL;
//             ::strcpy( un.sun_path, name );
//             un.sun_family = family;
//             length = sizeof( un );
//         }
//
//         Link::Address::operator Peer*( ) const
//         {
//             if ( type == Local )
//             {
//                 return ( Peer* ) &un;
//             }
//
//             return ( Peer* ) &a;
//         }
//
//         Data Link::Address::tostring() const
//         {
//             Data data;
//             data( "%s://%s:%d", ( const char* ) stype( type ), ( const char* ) host, port );
//             return data;
//         }
//
//         void Link::Address::operator = ( const Link::Address& address )
//         {
//             type = address.type;
//
//             void* source =  address.type == Local ? ( void* ) &address.un : ( void* ) &address.a ;
//             length =  address.type == Local ? sizeof( address.un ) : sizeof( address.a );
//
//             ::memcpy( *this, source, length );
//
//             length = address.length;
//             host = address.host;
//             family = address.family;
//
//             setPort( address.port );
//         }
//
//         void Link::Address::setPort( ui _port )
//         {
//             if ( family == AF_INET )
//             {
//                 ip4( )->sin_port = htons( _port );
//             }
//             else
//             {
//                 ip6( )->sin6_port = htons( _port );
//             }
//
//             port = _port;
//         }
//
//         void Link::open( const Address& address )
//         {
//             m_address = address;
//
//             auto fd = check(
//                     ::socket( address.family, address.type == Udp ? SOCK_DGRAM : SOCK_STREAM, 0 )
//                     )( "socket" );
//
//             int set = 1;
// #ifdef __MACH__
//             ::setsockopt( fd, SOL_SOCKET, SO_NOSIGPIPE, ( void * ) &set, sizeof(int ) );
// #else
//             ::setsockopt( fd, SOL_SOCKET, MSG_NOSIGNAL, ( void * ) &set, sizeof(int ) );
//             ::setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &set, sizeof( set ) );
// #endif
//
//             File::assign( fd );
//         }
//
//         int Link::error( )
//         {
//             int error = 0;
//             ui length = sizeof( error );
//
//             ::getsockopt( fd( ), SOL_SOCKET, SO_ERROR, &error, &length );
//
//             return error;
//         }
//
//         Link::Accept Link::accept( )
//         {
//             Accept accept;
//             accept.fd =  check( ::accept( fd( ), accept.address, &accept.address.length ) )( "accept" );
//             accept.address.parse( );
//
//             return accept;
//         }
//         void Link::connect( )
//         {
//             ENTER( );
//             check( ::connect( fd( ), address( ), address( ).length ), EINPROGRESS )( "connect" );
//         }
//
//         void Link::listen( )
//         {
//             ENTER( );
//             bind( );
//
//             if ( udp( ) )
//             {
//                 return;
//             }
//
//             check( ::listen( fd( ), SOMAXCONN ) )( "listen" );
//         }
//
//         void Link::bind( )
//         {
//             STRACE( "binding to %s:%d", ( const char* ) address().host, address().port );
//             check( ::bind( fd(), address(), address().length ) )( "bind to %s", ( const char* ) address().tostring() );
//         }
//
//         ul Link::write( const Data& data, ul offset ) const
//         {
//             ENTER( );
//
//             if ( local( ) )
//             {
//                 return File::write( data, offset );
//             }
//
//             long result;
//
//             if ( udp( ) )
//             {
//                 result = ::sendto( fd( ), data, data.length( ), 0, address( ), address( ).length );
//             }
//             else
//             {
//                 result = ::send( fd( ), data, data.length( ), 0 );
//             }
//
//             return check( result )( "send " );
//         }
//
//         ul Link::read( Data& data, ul offset )
//         {
//             if ( local( ) )
//             {
//                 return File::read( data, offset );
//             }
//
//             long result;
//
//             if ( udp( ) )
//             {
//                 result = ::recvfrom( fd( ), data, data.length( ), 0, address( ), &address( ).length );
//                 address( ).parse( );
//             }
//             else
//             {
//                 result = ::recv( fd( ), data, data.length( ), 0 );
//             }
//
//             return result;
//         }

        
    }   
}