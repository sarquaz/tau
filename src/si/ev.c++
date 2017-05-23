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
                    return EPOLLIN | EPOLLONESHOT;
#endif
                case Process:
#ifdef __MACH__
                    return EVFILT_PROC;
#else
                    return EPOLLIN;
#endif                                        
                    
                    
                default:
#ifdef __MACH__        
                    return EVFILT_TIMER;
#else                    
                    return EPOLLIN;
#endif                    
            }
        }
    
        Loop::Loop( )
            : m_stop( false )
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
            auto filter = event.filter( );

#ifdef __MACH__
            act = action == Add ? EV_ADD : EV_DELETE;
#else
            act = action == Add ? EPOLL_CTL_ADD : EPOLL_CTL_DEL;
            
            if ( event.state == Event::Disabled  )
            {
                TRACE( "modyfiying existing event", "" );
                act = EPOLL_CTL_MOD;
            }
            
#endif
            event.state = action == Add ? Event::Added : Event::Deleted;
            
        
             if ( event.type < 3 && action == Add )
             {
                 m_setup( event );
             }
                        
            TRACE( "%s event 0x%x with fd %d filter %d, act %d state %u type %d", action == Add ? "adding" : "removing", &event, event.fd, filter, act, event.state, event.type );
        
            Hevent handle;
            
            
#ifdef __MACH__
            int flags = 0;
            long time = 0;
            
            if ( event.type == Event::Default && action == Remove )
            {
                return;
            }

            if ( action == Add )
            {
                TRACE( "time value: %u sec %u usec %u", event.time.value, event.time.s(), event.time.us() );
                    
                if ( event.type == Event::Default || event.type == Event::Timer )
                {
                    flags = NOTE_USECONDS | NOTE_CRITICAL;
                    time = event.time.us();
                } 
                
                if ( event.time.infinite() )
                {
                    flags = NOTE_SECONDS;
                    time = INT_MAX;
                }
                
                if ( event.type == Event::Default )
                {
                    act |= EV_ONESHOT;
                }
                else if ( event.type == Event::Process )
                {
                    flags = NOTE_EXIT;
                }
            }
            
            EV_SET( &handle, event.fd, filter, act, flags, time, &event );
            
#else
            handle.events = filter;
            handle.data.ptr = &event;
#endif
            
            try
            {
#ifdef __MACH__
                si::check( ::kevent( m_handle, &handle, 1, NULL, 0, NULL ), ENOENT )( "kevent" );    
#else
                si::check( ::epoll_ctl( m_handle, act, event.fd, &handle ) )( "epoll_ctl" );
#endif
            }
            catch ( Error* error )
            {
                TRACE( "error %s", error->message.c() );
                error->deref();
                assert( false );
            }
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
        
        void Loop::Setup::operator()( Loop::Event& event )
        {
            ENTER();
            TRACE( "need to setup event type %u", event.type );
            
            
            
#ifdef __MACH__
            if ( event.fd )
            {
                return;
            }
            
            
            auto fd = 0;
            
            for ( ;; )
            {
                fd = r::random( INT_MAX );
                if ( !m_fds.contains( fd ) )
                {
                    m_fds.set( fd );
                    event.fd = fd;
                    TRACE( "assigning fd %d", fd );
                    break;
                }
            }
#else
            Event* processor = NULL;
            auto& list = m_map[ event.type ];
            
            TRACE( "list length: %u", list.length() );
            
            if ( !list.empty() )
            {
                processor = list.pop();
            }
            else
            {
                switch ( event.type )
                {
                    case Loop::Event::Default:
                        processor = mem::mem().type< Event >();
                        break;
                        
                    case Loop::Event::Timer:
                        processor = mem::mem().type< Timer >();
                        break;
                        
                    default:
                        break;
                }
            }
            
            
            if ( !processor )
            {
                TRACE( "no processor", "" );
                assert( false );
            }
            else
            {
                TRACE( "using processor 0x%x", processor );
            }
            
            event.custom = processor;
            processor->pre( event );
#endif
            
        }
        
#ifdef __linux__
        
        Loop::Setup::Event* Loop::Setup::Event::processor( Loop::Event& event )
        {
            SENTER();
            
            if ( !event.custom )
            {
                return NULL;
            }
            
            auto processor = static_cast< Event* >( event.custom );
            
            STRACE( "0x%x", processor );
            return processor;
        }
        
        void Loop::Setup::process( Loop::Event& event )
        {
            auto processor = Event::processor( event );
            if ( processor )
            {
                processor->post( event );    
            }
            
        }
        
        
        void Loop::Setup::complete( Loop::Event& event )
        {
            ENTER();
            TRACE( "event type %d", event.type );
            
            auto processor = Event::processor( event );
            if ( processor )
            {
                m_map[ event.type ].append( processor );    
            }
        }
        
        void Loop::Setup::Event::pre( Loop::Event& event )
        {
            ENTER();
            
            TRACE( "file fd %d", m_file.fd() );
            
            if ( !m_file.fd() )
            {
                auto fd = ::eventfd( 1, 0 );
                m_file.assign( fd );
                event.fd = fd;
            }
            else
            {
                event.fd = m_file.fd();
                long long value = 1;
                m_file.writeraw( ( char* ) &value, sizeof( value ) );
            }
        }
        
        void Loop::Setup::Event::post( Loop::Event& event )
        {
            ENTER();
            long long value = 0;
            m_file.readraw( ( char* ) &value, sizeof( value ) );
            TRACE( "read %u", value );
        }
        
#endif
        
        Request::Request( Request::Parent& parent )
            : m_error( NULL ), m_parent( parent ), m_event( *( Loop::Event::get() ) ), m_custom( NULL ), m_file( NULL )
        {
            ENTER();
            m_event.request = this;
            m_parent.before( *this );
            m_parent.add( *this );
            thread().ref();
        }
        
        Request::~Request()
        {
            ENTER();
            m_event.deref();
            m_parent.remove( *this );
            
            if ( m_error )
            {
                m_error->deref();
            }
            
            thread().deref();
        }

    }

}


    