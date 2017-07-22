#include "tau.h"

namespace tau
{
    namespace newev
    {        
        ui Event::id() 
        {
            if ( !m_file.fd() )
            {
                ui fd = 0;
                bool nb = false;
#ifdef __MACH__
                fd = smac();
#else
                fd = slinux();
                nb = true;
#endif
                m_file.assign( fd, nb );    
            }
            
            return m_file.fd();
        }
        
        ui Event::smac()
        {
            ENTER();
            
            ui fd = 0;
            
            for ( ;; )
            {
                fd = r::random( INT_MAX );
                if ( !m_loop->ids().contains( fd ) )
                {
                    m_loop->ids()[ fd ] = fd;
                    break;
                }
            }
            
            return fd;
        }
        
#ifdef __linux__
        
        ui Event::slinux()
        {
            ENTER();
            return si::check( ::eventfd( m_value, 0 ) )( "eventfd" );
        }
        
        
        void Event::after()
        {
            ENTER();
            assert( m_value );
            
            m_value = 0;
            long long value = 0;
            m_file.readraw( ( char* ) &value, sizeof( value ) );
        }
        
        void Event::before()
        {
            ENTER();
            if ( !m_value )
            {
                m_value += 1;
                m_file.writeraw( ( char* ) &m_value, sizeof( m_value ) );
            }
        }
        
#endif
        
        void Event::on( ui operation )
        {
            ENTER();
            ref();

            assert( m_parent );
            m_parent->event( operation, *this );
            
#ifdef __linux__
            after();
#endif
            
            deref();
        }
        
        bool Event::assigned( Loop& loop ) const
        {
            return ( m_loop == &loop || m_loops.contains( &loop ) );
        }
        
        bool Event::assign( Loop& loop )
        {
            ENTER();
            bool ret = false;
            if ( m_loop )
            {
                if ( m_loop != &loop )
                {
                    m_loops.set( m_loop );
                    m_loops.set( &loop );
                    ret = true;    
                }
            }
            else
            {
                ret = true;
            }
            
            m_loop = &loop;
            
            TRACE( "0x%x", m_loop );
            
            return ret;
        }
        
        void Event::cancel()
        {
            ENTER();
#ifdef __MACH__            
            m_file.assign( 0, false );            
#endif            
            if ( m_loops.length() )
            {
                m_loops.values( [ & ] ( Loop* loop ) { loop->op( Cancel, *this ); } );
                m_loops.clear();    
            }
            else
            {
                assert( m_loop );
                m_loop->op( Cancel, *this );
            }
            
        }
        
        void Event::schedule( ui what, Loop* loop )
        {
            ENTER();
            
            if ( !loop )
            {
                assert( false );
            }
            
#ifdef __linux__
            before();
#endif
            
            TRACE( "setting event 0x%x", this );
            
            if ( m_parent )
            {
                m_parent->events().set( this );
            }

            loop->op( what, *this );
            
        }
        
         Event::~Event()
         {
            ENTER();
            if ( m_type != Stop )
            {
                cancel();    
            }
        }
        
#ifdef __linux__
        ui Timer::slinux()
        {
            ENTER();
            return si::check( ::timerfd_create( CLOCK_MONOTONIC, 0 ) )( "timerfd" );
        }
        
        void Timer::after()
        {
            ENTER();
                        
            long value = 0;
            file().readraw( ( char* ) &value, sizeof( value ) );
        }
        
        void Timer::before()
        {
            ENTER();
            
            struct itimerspec timeout;
            m_timeout.totimespec( &timeout.it_interval );
            m_timeout.totimespec( &timeout.it_value );
            
            ::timerfd_settime( file(), 0, &timeout, NULL );
        }
        
#endif

        void Event::Parent::cancel()
        {
            ENTER();
            TRACE( "have %d events", m_events.length() );
            
            m_events.values( [ & ] ( Event* event ) 
                {    TRACE( "have event 0x%x", event );
                     if ( event )
                     {
                         event->deref(); 
                     }

                 } );
            m_events.clear();
        }
        
        Event& Event::Parent::last()
        {
             if ( !m_events.empty() )
            {
                return *m_events.last();
            }
            
            throw tau::Error();
        }
        
        Loop::Loop( )
            : m_stop( NULL )
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
            m_handle = 0;
        }
        
        Loop::Request* Loop::op( ui operation, Event& event )
        {
            ENTER();
            
            auto type = event.type();
            
            
            Request* request = NULL;
            auto assigned = event.assign( *this );;
            
            if ( operation != Event::Cancel )
            {
                request = mem::mem().type< Request >( operation, event );        
            }             
            
            long what = 0;
            long filter = 0;
            
            Hevent handle;    
            
#ifdef __MACH__
            int flags = 0;
            long timeout = 0;
            
            if ( event.type() == Event::Timer )
            {
                timeout = ( dynamic_cast< Timer& >( event ) ).timeout().ms();
                flags = NOTE_USECONDS | NOTE_CRITICAL;
            }
            
            if ( operation == Event::Cancel )
            {
                what = EV_DELETE;
            }
            else
            {
                what = EV_ADD;
            }
            
                        
            switch ( operation )
            {
                default:
                    filter = EVFILT_TIMER;
                    break;                    

            }    
        
            
            TRACE( "event 0x%x operation %u id %u what %d filter %d flags %d timeout %d", &event, operation, event.id(), what, filter, flags, timeout );
            
            EV_SET( &handle, event.id(), filter, what, flags, timeout, request );
            
#else
            if ( assigned )
            {
                what = EPOLL_CTL_ADD;
            }
            else
            {
                if ( operation == Event::Cancel )
                {
                    what = EPOLL_CTL_DEL; 
                }
                else
                {
                    what = EPOLL_CTL_MOD;
                }
            }
            
            switch ( operation )
            {
                default:
                    filter = EPOLLIN;
                    break;                    

            }
            
            handle.events = filter;
            handle.data.ptr = request;
            
            TRACE( "event 0x%x operation %u id %u what %d filter %d", &event, operation, event.id(), what, filter );
#endif
            
            try
            {
#ifdef __MACH__
                si::check( ::kevent( m_handle, &handle, 1, NULL, 0, NULL ), ENOENT )( "kevent" );    
#else
                si::check( ::epoll_ctl( m_handle, what, event.id(), &handle ) )( "epoll_ctl" );
#endif
            }
            catch ( Error* error )
            {
                TRACE( "error %s", error->message.c() );
                error->deref();
                assert( false );
            }
            
            return request;
        }
        
        void Loop::stopped()
        {
            ENTER();
            if ( m_stop )
            {
                m_stop->event.deref();
                m_stop->deref();
                m_stop = NULL;
            }
        }
            
        void Loop::run( )
        {
            ENTER();
            
            for ( ;; )
            {
                if ( m_stop )
                {
                    stopped();
                    return;
                }
                
                TRACE( "running event loop", "" );
                auto changes = 0;
                
                try
                {                    
#ifdef __MACH__
                    changes = si::check( ::kevent( m_handle, NULL, 0, m_events, lengthof( m_events ), NULL ) )( "kevent" );
#else
                    changes = si::check( ::epoll_wait( m_handle, m_events, lengthof( m_events ), -1 ), EINTR )( "epoll");
#endif
                }
                catch ( Error* error )
                {
                    TRACE( "error %s", error->message.c() );
                    assert( false );
                    return;
                }
                
                for ( auto i = 0; i < changes; i++ )
                {

                    Request* request = NULL;
                    Hevent& e = m_events[ i ];
                    
#ifdef __MACH__                        
                    
                    if ( e.flags & EV_ERROR )
                    {
                        TRACE( "error when processing event %d", e.data );
                        assert( false );
                    }
                    
                    request = static_cast< Request* > ( e.udata );
                    
#else
                    request = static_cast< Request* > ( e.data.ptr );                        
                    
#endif
                    TRACE( "request 0x%x operation %d event 0x%x", request, request->operation, &request->event );
                    
                    if ( request->operation == Stop )
                    {
                        stopped();
                        return;
                    }
                    
                    request->event.processor( *this );    
                    request->event.on( request->operation );    
                    
                    if ( request )
                    {
                        request->deref();
                    }
                }
            }
        }    
    }
}