#ifndef TAU_TEST_H_
#define	TAU_TEST_H_

#include "tau.h"
#include "../src/trace.h"

using namespace tau;

namespace tau 
{
    typedef li::Set< Data > Strings;
    
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
        
        struct Timeout: Timer
        {
            Timeout( Test& test, const Time& time )
                : Timer( test, time )
            {
                ENTER();
            } 
            
            virtual ~Timeout()
            {
                ENTER();
            }
            
            virtual void fire( )
            {
                ENTER();
                
                Test::out( "timed out" );

                test.status( 1 );   
                test.deref();
            }
            
            
            
            virtual void destroy()
            {
                delete this;
            }
            
            
        };
        
        friend class Run;
        friend class Timeout;
        
    public:
        Test( ui tries = 1, const Time& attempt = Time( 500 )  )
        : m_tries( tries ), m_attempt( attempt ), m_run( NULL ), m_timeout( NULL ), m_status( 0 )
        {
            os::Signals::assign( new Signals( *this ) );

        }
        
        void start()
        {
            tau::start( [ & ] ( ) 
                { 
                    this->attempt();
                    m_timeout = new Timeout( *this, Time( m_tries * m_attempt.ms() + 1000 ) );
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
            data::Data data;
            PRINT( data, format );
            data.add( "\n" );
            out::out( data );
        }
        
        ui tries() const
        {
            return m_tries;
        }
        
        ui status() const
        {
            return m_status;
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
        
                
        data::Data string( )
        {
            auto string = Data::get();
            m_strings.set( string );
            
            return string;
        }
                        
        virtual void terminated()
        {
            ENTER();
            
            deref();
        }
        
        
                
        
        virtual void cleanup( )
        {
            ENTER();
            
                
             if ( m_status )
             {
                 m_run->abort();
             }
            else
            {
                m_timeout->abort();
            }
            
            m_events.all( []( io::Event* event ) { event->deref(); } );
            
        }
        
                    
    private:
        
        ui status( ui status )
        {
            m_status = status;
            return m_status;
        }
        
        virtual void check() = 0;
        
        
        void attempt()
        {
            ENTER();
            
            if ( m_tries > 0 )
            {
                
                m_tries --;
                TRACE( "new try, %d tries remaining", m_tries );
                
                m_run = new Run( *this, m_attempt );
            }
            else
            {
                TRACE( "end of tries", "" );
                m_run = NULL;
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
        li::Set< Data > m_strings;
        Timeout* m_timeout;
        ui m_status;
    };
    
// #define TEST() \
//     Test* Test::s_instance = NULL; \
//     //__thread Test::State* Test::s_state = NULL;
    
};

#endif	

