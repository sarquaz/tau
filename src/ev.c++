#include "ev.h"

namespace tau
{
    namespace ev
    {
        void Loop::Event::fire()
        {
            ENTER();
            
            m_timer.timer();
            client().onEvent( *this );
            
        }
        
        void Loop::Event::destroy( )
        {
            ENTER();
            //client().remove( *this );    
            loop( ).remove( *this );
            m_timer.stop();
            ie::Tok::destroy( );
        }
        
        void Loop::Event::assign( const Time& time )
        {
            m_timer.start( time );
            set( ).fd = m_timer;
            set().type = si::Queue::Event;
        }
        
        void Loop::Event::assign( si::Handle fd, Type type )
        {
            ENTER();
            
            set( ).type = type;
            set().fd = fd;
        }
        
        void Loop::Client::clear( )
        {
            ENTER( );
            m_lock.with( [ & ] ( ) { m_events.values( [ & ] ( Event* event ) 
            {
                TRACE( "need to remove event 0x%x", event );
                event->destroy( ); } ); 
            } );
            
            m_events.clear();
        }
        
        void Loop::event::assign( Event& event, Loop* _loop )
        {
            ENTER( );

            event.setClient( client );
            Loop& loop = _loop ? *_loop : tau::loop();
            loop.add( event );
        }

        void Loop::event::operator()( const Time& time, Loop* _loop, Grain* grain )
        {
            auto& event = tok< Event >()();    
            event.setGrain( grain );
            event.assign( time );
            assign( event, _loop );
        }

        void Loop::event::operator()( const si::File& file, Type type )
        {
            auto& event = tok< Event >()();    
            event.assign( file, type );
            assign( event );
        }
        

        void Loop::remove( const Event& event )
        {
            ENTER();
            auto removed = true;
            
            //m_lock.with( [ & ] ( ) { removed = m_events.remove( ( void* ) &event ); } );
            
            if ( removed )
            {
                try
                {
                    si::Queue::remove( event );
                }
                catch( const Error& e )
                {
                    TRACE( "error %s", e.message.c( ) );
                    assert( false );
                }
            }
        }
        
        void Loop::onEvent( const Set& set )
        {
            try
            {
#ifdef __MACH__
                m_events.get( set.data );
#endif
                Event::instance( set.data ).fire( );
            }
            catch( ... )
            {
            }
        }

        void Loop::add( Event& event )
        {
            ENTER( );

            event.start( *this );
            
        //    m_lock.with( [ & ] ( ) { m_events[ &event ] = &event; } );
            
            try
            {
                si::Queue::add( event, event.time() );
            }
            catch ( const Error& e )
            {
                TRACE( "error %s", e.message.c() );
                assert( false );
            }
        }
        
        void Loop::Stop::onEvent( Event&  )
        {
            ENTER();
            
            loop.clear();

            delete this;
        }
        
        void Loop::clear( )
        {
            ENTER( );

            si::Queue::stop( );
          //  m_events.values( [ ] ( Event * event ) { event->destroy( ); } );
        }
        
        void File::Client::onEvent( Loop::Event& event )
        {
            ENTER( );

            TRACE( "event type %d fd %d", event.type( ), event.fd( ) );

            auto bytes = 0;

            if ( event.type( ) == Loop::Read )
            {
                bytes = doRead( );
                TRACE( "read %d bytes", bytes );
                if ( bytes )
                {
                    onRead();
                }
            }
            else
            {
                bytes = doWrite( );
                TRACE( "wrote %d bytes", bytes );
                if ( !bytes )
                {
                    onWrite();
                }
            }

            if ( !bytes )
            {
                event.destroy( );
            }
        }
    }
    
    __thread ev::Loop* t_loop = NULL;
   
    ev::Loop& loop()
    {
        if ( !t_loop )
        {
            t_loop = new ev::Loop();
        }
        
        return *t_loop;
    }
}
