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
                           try
                           {
                               loop->remove( *request );           
                           }
                           catch ( Error* e )
                           {
                               mem::mem().detype< Error >( e );
                           }
                           
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
                friend class Request;
                
            public:
                virtual ~Parent()
                {
                    m_list.all( [ & ]( Request* request ) 
                        {
                            if ( m_set.remove( request ) )
                            {
                                request->deref();
                            }
                        } );
                }
                
                virtual void before( Request& request )
                {
                    
                }
                
                virtual void callback( Request& request )
                {
                    
                }
                                
                Parent* parent() const
                {
                    return m_parent;
                }
                
            protected:
                Parent( Parent* parent = NULL )
                    : m_parent( parent )
                {
                    
                }
                
            private:
                void add( Request& request )
                {
                    ENTER();
                    m_set.set( &request );
                    m_list.append( &request );
                }
                
                void remove( Request& request )
                {
                    m_set.remove( &request );
                }
                
            private:
                Parent* m_parent;
                li::Set< Request* > m_set;
                li::List< Request* > m_list;
            };
            
            Request( Parent& parent )
                : m_error( NULL ), m_parent( parent ), m_event( *( Loop::Event::get() ) ), m_custom( NULL ), m_file( NULL )
            {
                m_event.request = this;
                m_parent.before( *this );
                m_parent.add( *this );
            }
            virtual ~Request()
            {
                ENTER();
                m_event.deref();
                m_parent.remove( *this );
            }
            
            virtual void callback() 
            {
                ENTER();
                
                m_parent.callback( *this );
                
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
            
            virtual void destroy()
            {
                mem::mem().detype< Request >( this );
            }
            
            fs::File*& file() 
            {
                return m_file;
            }
            
        private:
            Data m_data;
            Error* m_error;
            Loop::Event& m_event;
            Parent& m_parent;
            void* m_custom;
            fs::File* m_file;
        };
        
    }

}

#endif