#ifndef _TAU_SI_EV_H_
#define _TAU_SI_EV_H_

#include "../../src/trace.h"

namespace tau
{
    namespace si
    {
        namespace ev
        { 
            class Loop
            {
            public:         

                struct Event: Reel
                {
                    typedef int Handle;
                    
                    enum Type
                    {
                        Default,
                        Read,
                        Write,
                        Timer,
                        Stop
                    };
                        
                    Handle fd;
                    void* data;
                    Type type;
                    Time time;
                    Loop* loop;
                
                    Event( Type _type, Handle _fd = 0, void* _data = NULL )
                        : time( Time::Infinite ), fd( _fd ), type( _type ), data( _data ), loop( NULL )
                    {
                    }
                    
                    Event( const Time& _time = Time(), Type _type = Default, void* _data = NULL )
                    : fd( 0 ), type( _type ), data( _data ), time( _time ), loop( NULL )
                    {
                    }
                    
                    Event( const Event& event )
                    {
                        operator = ( event );
                    }
                    
                    virtual ~Event()
                    {
                       ENTER();
                       
                       if ( loop )
                       {
                           loop->remove( *this );
                       }
                    }
                
                    void operator = ( const Event& event )
                    {
                        fd = event.fd;
                        type = event.type;
                        data = event.data;
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
                };
                
                Loop( );
                ~Loop();      
            
                template < class Callback > void run( Callback callback )
                {
                    for ( ;; )
                    {
                        TRACE( "running event loop", "" );
        #ifdef __MACH__
                        auto changes = check( ::kevent( m_handle, NULL, 0, m_events, lengthof( m_events ), NULL ) )( "kevent" );
        #else
                        auto changes = check( ::epoll_wait( m_queue, m_events, lengthof( m_events ), -1 ) )( "epoll");
        #endif
                        for ( auto i = 0; i < changes; i++ )
                        {
        #ifdef __MACH__
                            Hevent& e = m_events[ i ];
                            TRACE( "event ident %d", e.ident );
                            
                            auto event = static_cast< Event* > ( e.udata );
                            event->fd = e.ident;

                            TRACE( "event 0x%x", event );
                            
                            if ( event->type == Event::Stop )
                            {
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
                void stop( ) const
                {
                    add( *Event::get( Time( ), Event::Stop ) );
                }
                
                void add( Event& event ) const
                {
                    event.loop = const_cast < Loop* > ( this );
                    act( Add, event );
                }
                void remove( Event& event ) const
                {
                    act( Remove, event );
                }
            
            
            private:
                enum Action
                {
                    Add,
                    Remove
                };

    #ifdef __MACH__
                typedef struct kevent Hevent;
    #else
                typedef struct epoll_event Hevent;
    #endif

                void act( Action action, Event& event ) const;
                static Event::Type type( int );

            private:
                Event::Handle m_handle;
                Hevent m_events[ 128 ];
            };
        }
    }
}

#endif