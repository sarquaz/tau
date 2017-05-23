#include "tau.h"
#include "trace.h"

namespace tau
{
    static Main s_main;
    __thread Main::Thread* t_thread = NULL;

    Main& Main::instance()
    {
        return s_main;
    }
    
    Main::Thread& Main::thread()
    {
        assert( t_thread );
        
        return *t_thread;
    }
            
    void Main::started( Main::Thread* thread )
    {
        t_thread = thread;
    }
    
    void Reel::ref( )
    {
        if ( !m_ref )
        {
            TRACE( "%u", m_ref );
            
            assert( false );
            return;
        }

        m_ref++;
        
        TRACE( "%d", m_ref );
    }

    void Reel::deref( )
    {
        TRACE( "%u", m_ref );
        
        if ( !m_ref )
        {
            assert( false );
            return;
        }

        m_ref--;
        

        if ( !m_ref )
        {
            destroy( );
        }
    }
    
    namespace box
    {
        ul Hash::operator()() const
        {
            unsigned long long hash = 100253314709U;
            ul last = 179426173;
            ul store = hash ^ m_size;
            auto pos = 0;
            ul* next;
            
            auto step = sizeof( next );
            
            ul number = hash ^ m_size;
            
            for ( ;; )
            {
                
                auto length = 0;
                auto rem = m_size - pos;

                if ( rem >= sizeof( store ) )
                {
                    next =  ( ul* ) ( m_what + pos ); 

                }
                else
                {
                    next = &store;
                    for ( auto i = 0; i < rem; i++ )
                    {

                        ( ( uchar* ) next )[ i ] = m_what[ pos + i ];
                    }
                    
                }
                
                hash ^= ( *next * last );
                last *=  m_what[ pos ];
                
                pos += MIN( step, m_size - pos );

                if ( pos >= m_size )
                {
                    break;
                }
            }

            return hash;
        }
        
    }
    
    namespace r
       {
           __thread Random* t_random = NULL;
        
           Random::Random( )
           : m_random( time().ms() + os::Thread::id( ) )
           {
           }
        
           ui Random::operator()( ui max )
           {
               auto random = m_random( );

               if ( max )
               {
                   if ( max )
                   {
                       random = random % max;
                   }
               }

               return random;
           }
        
           ul random( ui max )
           {
               if ( !t_random )
               {
                   t_random = new Random( );
               }

               return( *t_random ) ( max );
           }
            
       }
    

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
        
       }
    
}
