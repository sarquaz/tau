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
        }
        
        Loop::~Loop()
        {
            ENTER();
            
            ::close( m_handle );
        }
        

        void Loop::act( Action action, Event& event ) const
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
        
#ifdef __MACH__
            auto flags = 0;
            long time = 0;
            if ( action == Add )
            {
                TRACE( "time value: %u sec %u usec %u", event.time.value, event.time.time.tv_sec, event.time.time.tv_usec );
                    
                if ( event.type == Event::Default )
                {
                    flags = NOTE_USECONDS;
                    time = event.time.us();
                } 
                else if ( event.time.infinite() )
                {
                    flags = NOTE_SECONDS;
                    time = INT_MAX;
                }
            
                TRACE( "setting time value %d", time );
            }
        
            EV_SET( &handle, event.fd, event.filter( ), act, flags, time, &event );
            si::check( ::kevent( m_handle, &handle, 1, NULL, 0, NULL ) )( "kevent" );
        
#else
            Eevent set;
            set.events = event.filter( );
            set.data.ptr = event.data;
        
        
            si::check( ::epoll_ctl( m_queue, act, event.fd, &set ) )( "epoll_ctl" );
#endif
        }
    

    }

}


    