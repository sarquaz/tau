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
                enum State
                {
                    Initial,
                    Added,
                    Disabled,
                    Deleted    
                };
                
                enum Type
                {
                    Default = 0,
                    Timer,
                    Process,
                    Stop,
                    Read,
                    Write
                };
                
                Handle fd;
                Type type;
                Time time;
                Loop* loop;
                Request* request;
                void* custom;
                State state;
        
                Event( Type _type, Handle _fd = 0 )
                    : time( Time::Infinite ), fd( _fd ), type( _type ), loop( NULL ), request( NULL ), custom( NULL ), state( Initial )
                {
                
                }
            
                Event( const Time& _time = Time(), Type _type = Default )
                    : fd( 0 ), type( _type ), time( _time ), loop( NULL ), request( NULL ), custom( NULL ), state( Initial )
                {
                    ENTER();
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
                       try
                       {
                           {
                               loop->remove( *request );       
                           }
                       }
                       catch ( Error* e )
                       {
                           mem::mem().detype< Error >( e );
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
                    custom = event.custom;
                    state = event.state;
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
                    if ( m_stop )
                    {
                        TRACE( "need to stop", "" );
                        return;
                    }
                    
                    TRACE( "running event loop", "" );
                    ui changes = 0;
                    
                    try
                    {                    
    #ifdef __MACH__
                        changes = si::check( ::kevent( m_handle, NULL, 0, m_events, lengthof( m_events ), NULL ) )( "kevent" );
    #else
                        changes = si::check( ::epoll_wait( m_handle, m_events, lengthof( m_events ), -1 ) )( "epoll");
    #endif
                    }
                    catch ( const Error& error )
                    {
                        TRACE( "error %s", error.message.c() );
                        return;
                    }
                    
                    for ( auto i = 0; i < changes; i++ )
                    {
    
                        Hevent& e = m_events[ i ];
#ifdef __MACH__                        
                        
                        if ( e.flags & EV_ERROR )
                        {
                            TRACE( "error when processing event %d", e.data );
                            assert( false );
                        }
                        
                        auto event = static_cast< Event* > ( e.udata );
                        
                        TRACE( "event 0x%x event ident %d", event, e.ident );
                        
                        assert( event );
                                                    
                        
    #else
                        auto event = static_cast< Event* > ( e.data.ptr );                        
                        assert( event );
                        
                        if ( event->type == Event::Default )
                        {
                            event->state = Event::Disabled;
                        }
                        
                        
                        
    #endif
                        TRACE( "event 0x%x type %d state %d", event, event->type, event->state );
                        
                        if ( event->type == Event::Stop )
                        {
                            TRACE( "need to stop", "" );
                            event->deref();
                            return;
                        }
                        
#ifdef __linux__
                        m_setup.process( *event );
#endif                        
                        
                        callback( *event );
                        
#ifdef __linux__
                        m_setup.complete( *event );
#endif
                    }
                }
            }
            
            void stop( ) 
            {
                ENTER();
                m_stop = true;
                
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
                
#ifdef __linux__
                
                
                class Event: public Reel
                {
                public:
                    Event()
                    {
                        
                    }
                    
                    virtual ~Event()
                    {
                        
                    }
                    
                    
                    
                    virtual void post( Loop::Event& );
                    virtual void pre( Loop::Event& );
                    
                    
                    
                    virtual void destroy()
                    {
                        mem::mem().detype< Event >( this );
                    }
                    
                    static Event* processor( Loop::Event& );
                    
                protected:

                    
                protected:
                    fs::File m_file;
                };
                
                class Timer: public Event
                {
                };
                
                class Signal: public Event
                {
                };
                
                
#endif
                
            public:
                
                
                Setup()
                {
                    
                }
                ~Setup()
                {
                    
                }
                
                void operator()( Loop::Event& );
                
#ifdef __linux__
                void process( Loop::Event& );
                void complete( Loop::Event& );
#endif
                
            private:
                
                
#ifdef __MACH__
        li::Set< Handle > m_fds;        
#else
        li::Map< ui, li::List< Event* > > m_map;

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
            bool m_stop;
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
                    clear();
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
                
                void clear()
                {
                    m_list.all( [ & ]( Request* request ) 
                        {
                            if ( m_set.remove( request ) )
                            {
                                request->deref();
                            }
                        } );
                        
                    m_list.clear();    
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
            
            Request( Parent& parent );
            virtual ~Request();
            
            
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