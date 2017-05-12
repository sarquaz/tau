#include "Test.h"

using namespace tau;
using namespace tau::io;

TEST();

class TestNet: public Test
{
public:
    TestNet( Link::Type type )
    : Test( 3 ), m_count( 5 ), m_type( type )
    {
        setInterval( Time( 1000 ) );
        in::Female::handler( io::Net::Accept, ( Test::Handler ) &TestNet::onAccept );
    }
    
    virtual ~TestNet()
    {
    }
    
private:
    virtual void run( )
    {
        ENTER();
        start();
    }
    
    virtual void accepted( ev::Request& request )
    {
        ENTER();
    }
    
    virtual void read( ev::Request& request )
    {
        ENTER();
        auto& net = dynamic_cast< Net& >( grain );
        
        data::Data string = net.in();        
        
        if ( net.remote() )
        {
            out("echo %s", string.c() );
            net.out().add( string );
            return;
        }
        
        state().strings.remove( string );
        out( "%s", string.c() );
        Test::set( "read" );
    }
    
    virtual void onWrite( Grain& grain )
    {
        ENTER();
        auto& net = dynamic_cast< Net& >( grain );
        if ( net.remote() )
        {
            net.deref();
        }
    }
    
    virtual void onClose( Grain& grain )
    {
        ENTER();
    }
    
    io::Net& server( fs::Link::Type type = fs::Link::Tcp )
    {
        ENTER();
        TRACE( "creating server of type %d", type );
        
        Options options;

        options[  options::Type ] = type;
        options[  options::Server ] = true;
        
        Data host;
        
        if ( type == Link::Local )
        {   
            host = Data()( "/tmp/%s", ( const char* ) Data::get() );
        }
        else
        {
            host = localhost;
            options[ options::Port ] = Data()( "%u", random( 10000 ) + 9000 );
        }
        
        auto& server = io::net( options, host );
        add( server );
        return server;
    }
    
    virtual void onTimer( Event& timer )
    {
        ENTER();
        assert( state().strings.empty() );
        
        state().clear();   

        start( );
        timer.deref();
    }
    
    
    virtual void si::check()
    {
        Test::si::check( "read" );
    }
    
    virtual void started( )
    {
        try
        {
            li::List< Net* > servers;
            for ( auto i = 0; i < m_count; i++ )
            {
                servers.add( &Test::server( m_type ) );
            }
            
            servers.all( [ & ] ( Net* net ) 
            {
                auto s = string();
                TRACE( "adding string %s", s.c() );
                client( *net ).out().add( s ); } );
        }
            
        catch( const Error& e )
        {
            TRACE( "exception: %s", ( const char* ) e.message );
            assert( false );
        }
        
        if ( !state().event && state().tries > 1 )
        {
            state().event = &tau::event( this )( Time( 100 ) );
            state().tries --;
        }
    }
    
private:
    ui m_count;
    ui m_tries;
    Link::Type m_type;
};

int main()
{
        li::cycle< fs::Link::Type >( { fs::Link::Local, fs::Link::Udp, fs::Link::Tcp } )( []( fs::Link::Type type )
        {
            TestNet test( type );
        } );

    
    
   return 0;
}