#include "Test.h"

class Process: public Test
{
public:
    Process()
    {
        start();
    }
    
    virtual ~Process()
    {
            
    }
    
    virtual void event( ui event, ev::Request& request )
    {
        switch ( event )
        {
            case io::Process::Read:
                read( request );
                break;
                
            case io::Process::Write:
                wrote( request );
                break;
                
            case io::Process::Exit:
                exited( request );
                break;    
                
            case io::Event::Error:
                out( request.error()->message );
                assert( false );    
            
        }
    }
    
    virtual void wrote( ev::Request& request )
    {
        ENTER();
        
        checks()[ "write" ] = true;
        auto file = request.file();
        
        request.deref();
        file->close();
    }
    
    virtual void read( ev::Request& request )
    {
        ENTER();
        
        auto& process = dynamic_cast< io::Event& >( request.parent() );
        auto& text = request.data();
        
        
        text.length( text.length() - 1 );
        printf( text.c() );
        
        auto type = ( long ) request.custom();
        
        if ( type == sys::Out )
        {
            strings().remove( text );
            checks()[ "read" ] = strings().empty();    
        }
        
        if ( type == sys::Err)
        {
            checks()[ "error" ] = true;    
        }
            
        
        
        request.deref();
        
    }
    
    virtual void exited( ev::Request& request )
    {
        ENTER();
        
        auto& process = dynamic_cast< io::Event& >( request.parent() );
        
        checks()[ "exit" ] = true;
        
        request.parent().deref();
        request.deref();
    }
    
    virtual void run()
    {
        ENTER();
        
        checks()[ "write" ] = false;
        checks()[ "read" ] = false;
        checks()[ "error" ] = false;
        checks()[ "exit" ] = false;
        
        
        auto& process = io::process( *this, "test=`cat`; echo $test" );
        process.write( Test::string() );
        
        
        io::process( *this, "swdfgsg" );
    }
    
    
    virtual void check( )
    {
        ENTER();
    
        checks().entries( [ & ] ( const Data& data, bool value ) 
            {
                if ( !value )
                {
                    out( "check %s failed", data.c() );
                    assert( false );
                }
            } );
    }

    
private:
    Data m_test;
};

int main()
{
    new Process();
}
