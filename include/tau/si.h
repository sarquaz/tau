#ifndef _TAU_SI_H
#define _TAU_SI_H

#include "types.h"

namespace tau
{        
    namespace si
    {
        void* swap( void** target, void* value );
        ui inc( ui* target );
        ui dec( ui* target );
        
        
        template < class ...Args > class Call
        {
        public:
        
            virtual ~Call()
            {
            }
        
            virtual void operator()( Args&& ... args ) const = 0;
            virtual void destroy() = 0;
            
        protected:
            Call()
            {
                
            }
        
        };
    
        template < class Callable, class ...Args > class Callback: public Call< Args...  >
        {
        public:
            Callback( Callable callable )
                : Call< Args...  >()
            {
                m_store = mem::mem().get( align( sizeof( Callable ) ) ); 
                std::memcpy( m_store, ( void* ) &callable, sizeof( Callable ) );
            }
        
            virtual ~Callback()
            {
                mem::mem().free( m_store );
            }
        
            virtual void operator()( Args&& ... args ) const
            {
                ( *( static_cast< Callable* >( m_store ) ) )( std::forward< Args >( args ) ...  );
            }
        
            virtual void destroy()
            {
                this->~Callback();
                mem::mem().free( this );
            }
        
        
        private:
            ui align( ui size ) const
            {
                auto rem = size % sizeof( void* );
                if ( rem )
                {
                    size += sizeof( void* ) - rem;
                }
            
                return size;
            }
        private:
            void* m_store;
        };
    
        
        
        struct check
        {
            ui skip;
            long result;

            check( long _result, ui _skip = EWOULDBLOCK )
            : skip( _skip ), result( _result )
            {
            }

            ul operator()( const char* format, ... )
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
        };
    }
        
        
//         class Timer
//         {
//         public:
//             Timer( )
//             : m_started( false ), m_index( 0 )
//             {
//                 ::memset( m_fds, 0, sizeof( m_fds ) );
//             }
//
//             ~Timer();
//
//             void start( const Interval& );
//             void timer();
//             void stop();
//
//             operator Handle() const
//             {
//                 return m_fds[ m_index ];
//             }
//
//             long time() const
//             {
//                 return m_time;
//             }
//
//             bool started() const
//             {
//                 return m_started;
//             }
//
//         private:
// #ifdef __linux__
//             void clock( const Interval& = Interval() );
//             void inc();
//
//             enum Type
//             {
//                 First,
//                 Second
//             };
//
//             Type type( const Interval& time )
//             {
//                 return ( long ) time ? First : Second;
//             }
//
//             Handle fd() const
//             {
//                 return *this;
//             }
// #endif
//
//             enum
//             {
// #ifdef _MACH__
//                 Handles = 1
// #else
//                 Handles = 2
// #endif
//             };
//
//         private:
//             Interval m_time;
//             Handle m_fds[ Handles ];
//             bool m_started;
//             ui m_index;
//         };
        
       //  class Queue
//         {
//         public:
//             enum Type
//             {
//                 Read,
//                 Write,
//                 Event
//             };
//
//             struct Set
//             {
//                 Handle fd;
//                 void* data;
//                 Type type;
//
//                 Set(  Handle _fd = 0, void* _data = NULL )
//                 : fd( _fd ), type( Read ), data( _data )
//                 {
//                 }
//
//                 void operator = ( const Set& set )
//                 {
//                     fd = set.fd;
//                     type = set.type;
//                     data = set.data;
//                 }
//
//                 int filter() const;
//
//             };
//
//             void run( );
//             void stop( )
//             {
//                 m_stop = true;
//             }
//
//
//         protected:
//             Queue( );
//             virtual ~Queue()
//             {
//                 ::close( m_queue );
//             }
//
//             void add( const Set& event, long time = 0 )
//             {
//                 act( Add, event, time );
//             }
//             void remove( const Set& event )
//             {
//                 act( Remove, event );
//             }
//
//         protected:
//             typedef void ( Queue::*Handler )( const Set& );
//             void setHandler( Handler handler )
//             {
//                 m_handler = handler;
//             }
//
//         private:
//             enum Action
//             {
//                 Add,
//                 Remove
//             };
//
// #ifdef __MACH__
//             typedef struct kevent Kevent;
// #else
//             typedef struct epoll_event Eevent;
// #endif
//
//
//             void act( Action action, const Set& event, long time = 0 );
//             virtual void onEvent( const Set& ) = 0;
//             virtual ui length() = 0;
//
//             static Type type( int );
//
//
//
//         private:
//             Handle m_queue;
//             bool m_stop;
// #ifdef __MACH__
//             Kevent m_events[ 128 ];
// #else
//             Eevent m_events[ 128 ];
// #endif
//             Handler m_handler;
//         };
// 
        
        
         

}

#include "si/fs.h"
#include "si/os.h"
#include "si/ev.h"
#include "si/th.h"



#endif
