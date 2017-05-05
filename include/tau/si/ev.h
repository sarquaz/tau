#ifndef _TAU_SI_EV_H_
#define _TAU_SI_EV_H_

#include "../../src/trace.h"

namespace tau
{
    namespace ev
    { 
        typedef int Handle;
        
        class Request;
        
        class Loop
        {
        public:                     
            
            struct Event: Reel
            {
                enum Type
                {
                    Default,
                    Timer,
                    Read,
                    Write,
                    Once,
                    Stop
                };
                
                Handle fd;
                Type type;
                Time time;
                Loop* loop;
                Request* request;
        
                Event( Type _type, Handle _fd = 0 )
                    : time( Time::Infinite ), fd( _fd ), type( _type ), loop( NULL ), request( NULL )
                {
                
                }
            
                Event( const Time& _time = Time(), Type _type = Default )
                    : fd( 0 ), type( _type ), time( _time ), loop( NULL ), request( NULL )
                {
                }
            
                Event( const Event& event )
                {
                    operator = ( event );
                }
            
                ~Event()
                {
                   ENTER();
               
                   if ( loop )
                   {
                       assert( request );
                       if ( type != Once )
                       {
                           loop->remove( *request );
                       }
                   
                   }
                }
        
                void operator = ( const Event& event )
                {
                    fd = event.fd;
                    type = event.type;
                    request = event.request;
                    loop = event.loop;
                    time = event.time;
                }
        
                int filter() const;
            
                virtual void destroy()
                {
                    mem::mem().detype< Event >( this );            
                }
            
                template < class ... Args > static Event* get ( Args&&... args )
                {
                    return mem::mem().type< Event >( std::forward< Args >( args ) ... );
                }
            
                void callback();
            };
            
            
            Loop( );
            ~Loop();      
            
        
            template < class Callback > void run( Callback callback )
            {
                ENTER();
                
                for ( ;; )
                {
                    TRACE( "running event loop", "" );
                    ui changes = 0;
                    
                    try
                    {
                        
                    
    #ifdef __MACH__
                        changes = si::check( ::kevent( m_handle, NULL, 0, m_events, lengthof( m_events ), NULL ) )( "kevent" );
    #else
                        changes = si::check( ::epoll_wait( m_queue, m_events, lengthof( m_events ), -1 ) )( "epoll");
    #endif
                    }
                    catch ( const Error& error )
                    {
                        TRACE( "error %s", error.message.c() );
                        return;
                    }
                    
                    for ( auto i = 0; i < changes; i++ )
                    {
    #ifdef __MACH__
                        Hevent& e = m_events[ i ];
                        TRACE( "event ident %d", e.ident );
                        
                        auto event = static_cast< Event* > ( e.udata );
        
                        TRACE( "event 0x%x type %d", event, event->type );
                        
                        if ( event->type == Event::Stop )
                        {
                            TRACE( "need to stop", "" );
                            event->deref();
                            return;
                        }
                        
                        callback( *event );
    #else
                        Eevent& event = m_events[ i ];
                        Set set( 0, event.data.ptr );
                        auto type = event.events;
    #endif
                    }
                }
            }
            
            void stop( ) 
            {
                ENTER();
                
                act( Add, *Event::get( Time( ), Event::Stop ) );
            }
            
            void add( Request& ); 
            void remove( Request& );

        private:
            enum Action
            {
                Add,
                Remove
            };
            
            class Setup
            {
            public:
                Setup()
                {
                    
                }
                ~Setup()
                {
                    
                }
                
                void operator()( Event& );
                
            private:
#ifdef __MACH__
        li::Set< Handle > m_fds;        
#endif
            };

#ifdef __MACH__
            typedef struct kevent Hevent;
#else
            typedef struct epoll_event Hevent;
#endif

            void act( Action action, Event& event );
            
            

        private:
            Handle m_handle;
            Hevent m_events[ 128 ];
            Setup m_setup;
        };
        
        
        
        class Request: public Reel
        {
        public:
            class Parent: public Reel
            {
            public:
                virtual ~Parent()
                {
                    
                }
                
                virtual void configure( Loop::Event& )
                {
                    
                }
                
                virtual void callback()
                {
                    
                }
                
                Reel* reel() const
                {
                    return m_reel;
                }
            
                Reel* reel( Reel* reel ) 
                {
                    m_reel = reel;
                    return m_reel;
                }
                
            protected:
                Parent()
                    : m_reel( NULL )
                {
                    
                }
                
            private:
                Reel* m_reel;
            };
            
            Request( Parent& parent )
                : m_error( NULL ), m_parent( parent ), m_event( *( Loop::Event::get() ) ), m_callback( NULL ), m_custom( NULL )
            {
                
                m_event.request = this;
                m_parent.configure( m_event );
            }
            virtual ~Request()
            {
                ENTER();
                m_event.deref();
                
                if ( m_callback )
                {
                    m_callback->destroy();
                }
            }
            
            virtual void callback() 
            {
                ( *m_callback )( *this );
                m_parent.callback();
                
            }
            
            const Data& data() const
            {
                return m_data;
            }
                        
            Error* error( ) const
            {
                return m_error;
            }
            
            Data& data()
            {
                return m_data;
            }
                        
            Error* error( Error* error )
            {
                m_error = error;
                return m_error;
            }
            
            Loop::Event& event()
            {
                return m_event;
            }
            
            Parent& parent()
            {
                return m_parent;
            }

            Loop::Event::Type type() const
            {
                return m_event.type; 
            }
            
            void* custom() const
            {
                return m_custom;
            }
            
            void* custom( void* custom )
            {
                m_custom = custom;
                return custom;
            }
            
            template < class Callback > void assign( Callback callback )
            {
                m_callback = mem::mem().type< si::Callback< Callback, Request& > >( callback );
            }
            
            virtual void destroy()
            {
                this->~Request();
                mem::mem().free( this );
            }
            
            
        private:
            Data m_data;
            Error* m_error;
            si::Call< Request& >* m_callback;
            Loop::Event& m_event;
            Parent& m_parent;
            void* m_custom;
        };
        
    }

}

#endif