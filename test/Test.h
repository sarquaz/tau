#ifndef TEST_H
#define	TEST_H

#include "liner.h"
#include "../src/trace.h"

using namespace tau;
using namespace io;

namespace tau 
{
    class Test;
    
    class Test: public in::Female
    {
    public:
        Test( ui tries = 1, ui threads = 1 )
        : m_manager( NULL ), m_tries( tries )
        {
            handler( Event::Timeout, ( in::Female::Handler ) &Test::timerEvent );
            handler( Line::Start, ( in::Female::Handler ) &Test::lineStart );
            handler( Line::Stop, ( in::Female::Handler ) &Test::lineStop );
            handler( Liner::Line, ( in::Female::Handler ) &Test::liner );
            
            handler( Set::Data, ( Test::Handler ) &Test::read );
            handler( Set::Wrote, ( Test::Handler ) &Test::write );
            handler( Set::End, ( Test::Handler ) &Test::end );
            handler( Set::Error, ( Test::Handler ) &Test::error );

            s_instance = this;

            m_manager = &Liner::instance( *this, threads );
            si::Signals::assign( new Signals() );
        }
        
        ui start()
        {
            m_manager->start();
            return 0;
        }
        
        ui operator()()
        {
            return start();    
        }
        
        virtual ~Test()
        {
            ENTER();
            
            if ( m_manager )
            {
                delete m_manager;
            }
        }
        
        
        static Test& get()
        {
            return *s_instance;
        }
        
        static void out( const char* format, ... )
        {
            si::Data data;
            PRINT( data, format );
            data.add( "\n" );
            si::out( data );
        }
        
        ui tries() const
        {
            return m_tries;
        }
        
    protected:
        struct State
        {
            typedef li::List< Set* > Sets;
            li::Map< Sets > sets;
            Event* event;
            ui tries;
            li::Strings strings;
            
            ~State()
            {   
               clear();
            }
            
            State()
            : event( NULL ), tries( Test::get().tries() )
            {
                
            }
            
            void set( const si::Data& name, Set& set )
            {
                sets[ name.hash() ].add( &set );
            }
            
            void clear()
            {
                ENTER();
                
                sets.all( [ & ] ( ul hash, const Sets& sets ) 
                {
                    sets.all( [ ] ( Set* set ) { set->deref(); } );
                } );
                
               sets.clear();
               event = NULL;
            }
        };
        
        State& state()
        {
            if ( !s_state )
            {
                s_state = new State();
            }
            return *s_state;
        }
        
        si::Data string( )
        {
            auto string = si::Data::get();
            state().strings[ string ] = string;
            
            return string;
        }
                        
        virtual void onTerminate()
        {
            ENTER();
            
            if ( m_manager )
            {
                m_manager->stop();
                delete m_manager;
            }
        }
        
        void setInterval( const Time& interval )
        {
            m_interval = interval;
        }
        
        Set& start( const Set::Options& options, const si::Data& name )
        {
            ENTER( );
            
            auto& set = Set::get( name );
            set.females().add( *this );
            
            si::Data s;
            options.all( [ & ] ( const si::Data& key, const si::Data& value ) 
            {
                s.add( si::Data()( "%s: %s\t", key.c(), value.c() ) );
            } );
            
            TRACE( "options: %s", s.c() );
            
            try
            {
                set.start( options );
                state().set( name, set );
            }
            catch ( const Error& e )
            {
                TRACE( "exception: %s", ( const char* ) e.message );
                assert( false );
            }
                
            return set;
        }
                
        Net& server( si::Link::Type type = si::Link::Tcp )
        {
            ENTER();
            TRACE( "creating server of type %d", type );
            
            Set::Options options;
            
            auto stype = si::Link::stype( type );
            
            
            options[ "type" ] = stype;
            options[ "mode" ] = "server";
            
            
            if ( type == si::Link::Local )
            {   
                options[ "host" ] = si::Data()( "/tmp/%s", ( const char* ) si::Data::get() );
            }
            else
            {
                options[ "host" ] = "localhost";
                options[ "port" ] = si::Data()( "%u", si::random( 10000 ) + 9000 );
            }
            
            return dynamic_cast< Net& >( start( options, "net" ) );
        }
        
        Net& client( Net& server )
        {
            ENTER( );
           
            Set::Options options = server.options();
            options[ "mode" ] = "client";
            
            return dynamic_cast< Net& >( start( options, "net" ) );
        }
        
        Bot& process( const si::Data& command )
        {
            return dynamic_cast< Bot& >( start(  { { "command", command } } , "process" ) );
        }
        
//        File& file( const si::Data& name )
//        {
//            ENTER();
//            return dynamic_cast< File& >( start(  { { "name", name } } , "file" ) );
//        }
        
        
        virtual void cleanup( )
        {
            if ( s_state )
            {
                delete s_state;
                s_state = NULL;
            }
        }
        
        void set( const si::Data& name, bool value = true )
        {
            m_checks[ name.hash() ].value = value;
        }
        
        void check( const si::Data& name )
        {
            out( "checking %s", name.c() );
            
            try
            {
                assert( m_checks[ name.hash() ].value );
            }
            catch ( ... )
            {
                assert( false );
            }
        }
        
        struct Data
        {
            virtual ~Data()
            {
            }
        };
        
        void assign( const Set& set, Data& data )
        {
            m_data[ ( ul ) &set ] = &data;
        }
        
        Data& data( const Set& set )
        {
            return *m_data.get( ( ul ) &set );
        }
        
            
    private:
        class Signals: public si::Signals
        {
        private:
            virtual void onAbort( si::Signals::What what )
            {
                if ( what != si::Signals::Error )
                {
                    Test::get().cleanup();
                }
             
                if ( what != si::Signals::Exit )
                {
                    out( "exiting with error" );
                }
                
                ::exit( what == si::Signals::Exit ? 0 : 1 );
            }
            
            virtual void onTerminate()
            {
                Test::get().onTerminate();
            }
        };
        
        virtual void run( )
        {
        }
        
        virtual void  check()
        {
            
        }
        
        virtual void onTimer( Event& timer )
        {
            
        }
        
        void timerEvent( Grain& grain )
        {
            ENTER();
            
            auto& timer = dynamic_cast< Event& >( grain );

            TRACE( "timer type %d", timer.type() );
            
            if ( !timer.type() )
            {
                onTimer( timer );
                return;
            }
            
            check();
            
            if ( !tau::line().id() )
            {
                m_manager->stop();
            }
            
            timer.deref();
        }
        
        void liner( Grain& grain )
        {
            Line& line = dynamic_cast< Line& >( grain );
            line.females().add( *this );
        }
                
        void lineStop( Grain& )
        {
            cleanup();
        }
        
        void lineStart( Grain& )
        {
            ENTER();
            auto& event = tau::event( this )( m_interval );
            event.setType( 1 );
            
            ie::toker().setMax( 100 );
            
            Set::populate();
            
            run( );
        }
        
        void read( Grain& grain )
        {
            onRead( grain );
        }
        
        virtual void onRead( Grain& grain )
        {
        }
        
        void write( Grain& grain )
        {
            onWrite( grain );
        }
        
        virtual void onWrite( Grain& grain )
        {
        }
        
        void end( Grain& grain )
        {
            onEnd( grain );
        }
        
        virtual void onEnd( Grain& grain )
        {
        }
        
        void error( Grain& grain )
        {
            onError( grain );
        }
        
        virtual void onError( Grain& grain )
        {
            auto& set = dynamic_cast< Set& >( grain );
            TRACE( "%s", ( const char* ) set.error().message );
                    
            assert( false );
        }
                
        struct Check
        {
            bool value;
            Check()
            : value( false )
            {
                
            }
            void clear()
            {
                
            }
        };
    private:
        Liner* m_manager;
        static Test* s_instance;
 
        Time m_interval;
        ui m_tries;
        static __thread State* s_state;
        li::Map< Check > m_checks;
        li::Map< Data* > m_data;
    };
    
#define TEST() \
    Test* Test::s_instance = NULL; \
    __thread Test::State* Test::s_state = NULL;
    
};

#endif	

