#ifndef TAU_TEST_H_
#define	TAU_TEST_H_

#include "tau.h"
#include "../src/trace.h"

using namespace tau;

typedef li::Set< Data > Strings;
typedef li::Map< Data, bool > Checks;

class Test: public io::Result
{
    
    struct Timer: io::Result
    {
        io::Timer* timer;
        Test& test;
        
        Timer( Test& _test, const Time& time )
            : test( _test )
        {
            timer = &io::timer( *this, time );    
        } 
        
        virtual ~Timer()
        {
            
        }
        
        virtual void destroy()
        {
            delete this;
        } 
        
        void abort()
        {
            timer->deref();
            deref();
        }
        
        virtual void fire() = 0;
        
        virtual void event( ui event, ev::Request& request )
        {
            ENTER();
            
            fire();
            
            request.parent().deref();    
            deref();
            
        }
    };
    
    struct Run: Timer
    {            
        Run( Test& test, const Time& time )
            : Timer( test, time )
        {
            test.run();
        }
        
        virtual ~Run()
        {
            ENTER();        
        }
        
        virtual void fire()
        {
            ENTER();
            
            test.check();
            test.attempt();
        }
    };
    
    
    friend class Run;
    
public:
    Test( ui tries = 1, const Time& attempt = Time( 500 )  )
    : m_tries( tries ), m_attempt( attempt ), m_run( NULL )
    {
        os::Signals::assign( new Signals( *this ) );
    }
    
    void start()
    {
        tau::start( [ & ] ( ) 
            { 
                this->attempt();
            } );
    }
                        
    virtual ~Test()
    {
        ENTER();
        
        cleanup();
        tau::stop( [ & ]( ) {   } );

    }
            
    static void out( const char* format, ... )
    {
        Data data;
        _DATA_PRINT( data, format );
        data.add( "\n" );
        sys::out( data );
    }
    
    ui tries() const
    {
        return m_tries;
    }
            
protected:
    virtual void event( ui, ev::Request& )
    {
        ENTER();
    }
    virtual void run()
    {
    }
            
    void add( io::Event& event )
    {
        ENTER();
        m_events.append( &event );
    }
    
    Data string( )
    {
        Data d;
        d.add( Data::get() );
        m_strings.set( d );
        
        return d;
    }
    
    Strings& strings()
    {
        return m_strings;
    }
    
    Checks& checks()
    {
        return m_checks;        
    }    
                    
    virtual void terminated()
    {
        ENTER();
        
        deref();
    }
    
    virtual void cleanup( )
    {
        ENTER();
        
        if ( m_run )
        {
            m_run->abort();
            m_run = NULL;
       }
         
       m_events.all( []( io::Event* event ) { event->deref(); } );
       m_events.clear();
    }
    
                
private:
    
    virtual void check() = 0;
    
    
    void attempt()
    {
        ENTER();
        
        m_run = NULL;
        
        if ( m_tries > 0 )
        {
            m_tries --;
            TRACE( "new try, %d tries remaining", m_tries );
            
            m_run = new Run( *this, m_attempt );
        }
        else
        {
            TRACE( "end of tries", "" );
            cleanup();
            deref();    
        }
    }
    
    virtual void destroy()
    {
        delete this;
    }
    
    
    class Signals: public os::Signals
    {
    public:
        Signals( Test& test )
            : m_test( test )
        {
            
        }
    private:
        
        virtual void aborted( os::Signals::What what )
        {
            ENTER();
            TRACE( "signal %s", what == os::Signals::Error ? "error" : "exit" );
            
            if ( what != os::Signals::Error )
            {
                m_test.deref();
            }
         
            if ( what != os::Signals::Exit )
            {
                out( "exiting with error" );
            }
            
            ::exit( what == os::Signals::Exit ? 0 : 1 );
        }
        
        virtual void terminated()
        {
            m_test.terminated();
        }
        
    private:
        Test& m_test;
    };
    
            
private:
    
    ui m_tries;
    
    li::List< io::Event* > m_events;
    Run* m_run;
    Time m_attempt;
    Strings m_strings;
    Checks m_checks;
    
};        
    

#endif	

