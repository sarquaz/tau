#include "liner.h"
#include "trace.h"

namespace tau
{
    __thread Line* t_line = NULL;
    
    Liner* Liner::s_instance = NULL;
    
    Line::Line( unsigned int id )
    : m_id( id )
    {
        TRACE( "starting thread id %d", id );
    }

    Line::~Line()
    {
        ENTER();
    }
         
    void Line::routine( )
    {
        ENTER( );        
        
        ref();
        m_loop = &loop();
        ev::Loop::Client::timer();
        
        dispatch( _Start, *this );
        
        loop().run();
        
        dispatch( Stop, *this );
        
        delete m_loop;
        
        ie::cleanup();
        
        deref();
        Liner::instance().stopped();
    }
    
   void Line::stop()
   {
        ENTER();
        try
        {
            m_loop->stop();
        }
        catch ( const Error& e )
        {
            TRACE( "%s", e.message.c() );
            assert( false );
        }

        join();
   }
   
   void Line::onEvent( ev::Loop::Event& event )
   {
       ENTER();
              
       dispatch( Start, *this );
       event.destroy();
   }

    Liner::Liner( unsigned int threads )
    : m_count( threads ), m_stopped( 0 )
    {
        ENTER();
        
        handler( Line::_Start, ( in::Female::Handler ) &Liner::started );
        
    }
    
    Liner& Liner::instance( in::Female& female, unsigned int threads )
    {
        if ( !s_instance )
        {
            s_instance = new Liner( threads );
            s_instance->females().add( female );
        }
        
        return *s_instance;
    }
    
    Liner::~Liner()
    {
        ENTER();
        
        stop();
        
        ie::cleanup();
        s_instance =  NULL;
        
    }
    
    void Liner::start( )
    {
        ENTER();
        if ( !m_lines.empty() )
        {
            return;
        }
        
        unsigned int id = 0;
        
        TRACE( "starting %d threads", m_count );
        while( id < m_count  )
        {
            auto& line = *new tau::Line( id ++ );
            line.females().add( *this );
            line.start();
        }
        
        m_semaphore.wait();
    }
    
    tau::Line* Liner::pline()
    {
        return t_line;
    }
    
    tau::Line& Liner::line( )
    {
        auto line = Liner::pline( );
        assert( line );

        return *line;
    }
    
    void Liner::stopped(  )
    {
        ENTER( );
        
        si::inc( &m_stopped );
        if ( m_stopped == m_count  )
        {
            m_semaphore.post();
        }
    }
    
    void Liner::started( Grain& grain )
    {
        ENTER( );
        auto& line = dynamic_cast< tau::Line& >( grain );
        m_lock.with( [ & ] ( ) { m_lines.add( &line ); } );
        t_line = &line;
        
        in::Male::dispatch( Line, line );
    }
    
    void Liner::stop()
    {
        ENTER();

        tau::Line* line = NULL;
        
        while ( m_lines.length() )
        {
            m_lock.with( [ & ] ( ) { line = m_lines.get(); } );
            line->stop();
            line->deref();
        };
        
        m_lines.clear();
    }
    
    void Event::start( const Time& interval )
    {
        m_interval = interval;
        TRACE( "starting with interval %d", ( long ) interval );
        ev::Loop::event( *this )( interval );
    }
    

}    

