#include "tau.h"

namespace tau
{
    namespace ev 
    {   
        void Loop::Event::callback()
        {
            assert( request );
            request->callback();
        }
        
        int Loop::Event::filter() const
        {
            switch ( type )
            {
                case Read:
#ifdef __MACH__
                    return EVFILT_READ;
#else
                    return EPOLLIN;
#endif

                case Write:
#ifdef __MACH__
                    return EVFILT_WRITE;
#else
                    return EPOLLOUT;
#endif

                case Default:
                
#ifdef __MACH__
                    return EVFILT_TIMER;
#else
                    return EPOLLIN;
#endif
                    
                default:
                    return EVFILT_TIMER;
            }
        }
    
        Loop::Loop( )
        {
            ENTER();
#ifdef __MACH__
            m_handle = ::kqueue( );
#else
            m_handle = ::epoll_create1( 0 );
#endif
            std::memset( m_events, 0, sizeof( m_events ) );
            
        }
        
        Loop::~Loop()
        {
            ENTER();
            
            ::close( m_handle );
        }
        

        void Loop::act( Action action, Event& event ) 
        {
            ENTER();
            auto act = 0;

#ifdef __MACH__
            act = action == Add ? EV_ADD : EV_DELETE;
#else
            act = action == Add ? EPOLL_CTL_ADD : EPOLL_CTL_DEL;
#endif
        
            TRACE( "%s event 0x%x with fd %d filter %d", action == Add ? "adding" : "removing", &event, event.fd, event.filter() );
        
            Hevent handle;
            
            m_setup( event );
        
#ifdef __MACH__
            auto flags = 0;
            long time = 0;
            if ( action == Add )
            {
                TRACE( "time value: %u sec %u usec %u", event.time.value, event.time.s(), event.time.us() );
                    
                if ( event.type == Event::Default || event.type == Event::Timer )
                {
                    flags = NOTE_USECONDS;
                    time = event.time.us();
                } 
                else if ( event.time.infinite() )
                {
                    flags = NOTE_SECONDS;
                    time = INT_MAX;
                }
                else if ( event.type == Event::Once )
                {
                    TRACE( "oneshot event", "" );
                    act |= EV_ONESHOT;
                }
                
                TRACE( "setting time value %d", time );
            }
            
            EV_SET( &handle, event.fd, event.filter( ), act, flags, time, &event );
            try
            {
                si::check( ::kevent( m_handle, &handle, 1, NULL, 0, NULL ) )( "kevent" );    
            }
            catch ( const Error& error )
            {
                TRACE( "error %s", error.message.c() );
            }
            
        
#else
            Eevent set;
            set.events = event.filter( );
            set.data.ptr = event.data;
        
        
            si::check( ::epoll_ctl( m_queue, act, event.fd, &set ) )( "epoll_ctl" );
#endif
        }
        
        void Loop::add( Request& request ) 
        {
            auto& event = request.event();
            event.loop = this;
            act( Add, event );
        }
        void Loop::remove( Request& request ) 
        {
            act( Remove, request.event() );
        }
        
        void Loop::Setup::operator()( Event& event )
        {
            ENTER();
#ifdef __MACH__
            auto fd = r::random( INT_MAX );
            if ( !m_fds.exists( fd ) )
            {
                m_fds.set( fd );
                event.fd = fd;
                TRACE( "assigning fd %d", fd );
            }
            else
            {
                ( *this )( event );
            }
#else
            
#endif
        }
    }

}


    