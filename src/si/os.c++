#include "tau.h"

namespace tau
{
    namespace si
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
            {
                check( 
                    ::pthread_cond_init( &m_cond, NULL )  
                    ) ( "pthread_cond_init" );
                    
                    
                check( 
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
                //
                //  lock the mutex
                //  
                ::pthread_mutex_lock( &m_mutex );
                //
                //  wait
                //
                if ( time )
                {
                    ::pthread_cond_timedwait( &m_cond, &m_mutex, *time );
                }
                else
                {
                    ::pthread_cond_wait( &m_cond, &m_mutex );
                }
            }
            
            void Condition::signal()
            {
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
                check( 
                        ::pthread_create( &m_handle, NULL, ( void *( * )( void* ) ) routine, ( void* ) this ) 
                        ) ( "thread create" );
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
}