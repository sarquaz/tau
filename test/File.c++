#include "Test.h"

class File: public Test
{
public:
    File()    
    : m_name( Data::get() )
    {
        start();    
    }
    
    virtual ~File()
    {
            
    }
    
    virtual void event( ui event, ev::Request& request )
    {
        switch ( event )
        {
            case io::File::Read:
                read( request );
                break;

            case io::File::Write:
                wrote( request );
                break;
                
            case io::Event::Error:            
                out( request.error()->message.c() );
                assert( false );
                break;                    
        }
    }
    
private:
    virtual void read( ev::Request& request )
    {
        ENTER();
        
        auto& read = request.data();
        
        checks()[ "read" ] = m_write == read;
        
        read.add( '\0' );
        out( read );
        out( "\n" );
        
        //
        //  close file
        //  
        auto file = dynamic_cast< io::File* >( request.parent().parent() );
        file->deref();        
    }
    
    virtual void wrote( ev::Request& request )
    {
        ENTER();
        auto file = dynamic_cast< io::File* >( request.parent().parent() );
        file->read();
        
        checks()[ "write" ] = true;
    }
        
    virtual void run()
    {
        ENTER();

        m_name.add( ".test" );
        //
        //  open file
        //
        auto& file = io::file( *this, m_name );
        //
        //  get test  string
        //
        m_write = Data::get( 100 );
        //
        //  write to file
        //
        file.write( m_write );
    }
    
    virtual void cleanup()
    {
        ENTER();
        fs::File::remove( m_name );
        Test::cleanup();
    }
    
private:
    Data m_name;
    Data m_write;
};

int main()
{
    new File();
}
