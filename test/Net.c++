#include "Test.h"

class Net: public Test
{
public:
    Net( fs::Link::Type type )
    : Test( 2 ), m_count( 2 ), m_type( type )
    {
        ENTER();
        start();
    }
    
    virtual ~Net()
    {
    }
    
private:
    virtual void run( )
    {
        ENTER();

        for ( auto i = 0; i < m_count; i++ )
        {
            server( m_type );
        }        
    }
    
    virtual void event( ui event, ev::Request& request )
    {
        switch ( event )
        {
            case io::Net::Accept:
                accepted( request );
                break;
                
            case io::Net::Close:
                closed( request );
                break;
                
            
            case io::Net::Read:
                read( request );
                break;    

                
           case io::Net::Write:
                wrote( request );
                break;                
                
          case io::Net::Listen:
               listened( request );
               break;                    
        
            
            case io::Net::Error:
                out( request.error()->message.c() );
                assert( false );
                break;                    
        }
    }
    
    void accepted( ev::Request& request )
    {
        ENTER();
    }
    
    void closed( ev::Request& request )
    {
        ENTER();
        
        request.parent().deref();
    }
    
    void read( ev::Request& request )
    {
        ENTER();
        auto& net = dynamic_cast< io::Net& >( request.parent() );
        
        TRACE( "read event, server %d", net.server() );
        
        Data string;
        
        string.add( request.data() );
        
        if ( net.remote() )
        {
            out("echo %s", string.c() );
            net.write( string );
            return;
        }
        
        out( "%s", string.c() );
        
        strings().remove( string );
        //net.deref();
        
    }
    
    void wrote( ev::Request& request )
    {
        ENTER();
        //auto& net = dynamic_cast< io::Net& >( grain );
        
    }
    
    void listened( ev::Request& request )
    {
        ENTER();
        auto& server = dynamic_cast< io::Net& >( request.parent() );
        
        auto s = Test::string();
        
        TRACE( "adding string %s", s.c() );
        client( server ).write( s ); 
    }
        
    io::Net& server( fs::Link::Type type = fs::Link::Tcp )
    {
        ENTER();
        TRACE( "creating server of type %d", type );
        
        Options options;

        options[  options::Type ] = type;
        options[  options::Server ] = true;
        
        Data host;
        
        if ( type == fs::Link::Local )
        {   
            host = "/tmp/";
            host.add( Data::get() );
            host.add( ".sock" );
        }
        else
        {
            options[ options::Port ] = r::random( 10000 ) + 9000;
        }
        
        auto& server = io::net( *this, options, host );
        add( server );
        return server;
    }
    
    io::Net& client( io::Net& server )
    {
        ENTER();
        TRACE( "creating client of type %d", server.type() );
        
        Options options;

        options[  options::Type ] = server.type();
        
        auto& host = server.host();
        options[ options::Port ] = server.port();

        auto& client = io::net( *this, options, host );
        
        add( client );
        return client;
    }
        
    
    virtual void check()
    {
        ENTER();
        
        strings().values( [ & ] ( const Data& value ) 
            {
                TRACE("strings: value %s", value.c() );
            } );
        
        
        assert( strings().empty() );
    }
    
private:
    ui m_count;
    fs::Link::Type m_type;
    
};

int main()
{   
    li::cycle< fs::Link::Type >( { fs::Link::Local , fs::Link::Udp , fs::Link::Tcp  } )( []( fs::Link::Type type )
        {
            STRACE( "%d", type );
            new Net( type );
        } );

    
    
   return 0;
}