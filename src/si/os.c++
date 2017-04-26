#include "tau.h"
#include "../trace.h"

namespace tau
{
    namespace os
    {
        Lock::Lock( )
        {
            pthread_mutexattr_t attributes;
            pthread_mutexattr_init( &attributes );
            pthread_mutexattr_settype( &attributes, PTHREAD_MUTEX_ERRORCHECK );
            pthread_mutex_init( &m_lock, &attributes );
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
            ::pthread_join( m_handle, &ret );
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
            return out::id();
        }
    }
}