#include "Test.h"

using namespace tau;
using namespace tau::io;

TEST();

class TestNet: public Test
{
public:
    TestNet( si::Link::Type type )
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
    
    virtual void onAccept( Grain& grain )
    {
        ENTER();
        auto& net = dynamic_cast< Net& >( grain );
        net.females().add( *this );

        net.enable();
    }
    
    virtual void onRead( Grain& grain )
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
    
    virtual void onTimer( Event& timer )
    {
        ENTER();
        assert( state().strings.empty() );
        
        state().clear();   

        start( );
        timer.deref();
    }
    
    virtual void check()
    {
        Test::check( "read" );
    }
    
    void start( )
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
    si::Link::Type m_type;
};

int main()
{
    li::cycle< si::Link::Type >( { si::Link::Local, si::Link::Udp, si::Link::Tcp } )( []( si::Link::Type type )
    {
        ( TestNet( type ) ) (); 
        
    } );
    
   return 0;
}