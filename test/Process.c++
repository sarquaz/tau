#include "Test.h"

class Process: public Test
{
public:
    Process()
    : m_test( Data::get( 10 ) )
    {
        
    }
    
    virtual ~Process()
    {
            
    }
    
    virtual void event( ui event, ev::Request& request )
    {
        switch ( event )
        {
            case io::Proces::Read
        }
    }
    
    virtual void onWrite( Grain& grain )
    {
        ENTER();
    }
    
    virtual void onRead( Grain& grain )
    {
        ENTER();
        
        auto& process = dynamic_cast< Bot& >( grain );
        auto& text = process.in().data();
        
        out( "%s", ( const char* ) text );
        
        try
        {
            auto& data = dynamic_cast< Data& >( Test::data( process ) );
            assert( data.in.data() == process.in().data() );
            assert( process.in().data().find( data.out.data() )  > -1 );
            return;
        }
        catch ( ... )
        {
            
        }
        
       Test::set( "data" );
       
        if ( text.length( ) )
        {
            Test::set( "string", text.find( m_test ) > -1 );
        }
        
        Test::process( "swdfgsg" );
    }
    
    virtual void onError( Grain& grain )
    {
        auto& process = dynamic_cast< Bot& >( grain );
        TRACE( "error: %s", ( const char *) process.error().message );
        Test::set( "error" );
    }
   
    void onEnd( Grain& grain )
    {
        ENTER();

        auto& process = dynamic_cast< Bot& >( grain );
        try
        {
            Test::data( process );
        }
        catch ( ... )
        {
            if ( !process.status() )
            {
                Test::set( "exit" );
                event( this )();
            }
        }
    }   
    
    virtual void run()
    {
        ENTER();
        
        auto& process = Test::process( "test=`cat`; echo $test" );
        process.out().add( m_test );
    }
    
    void map()
    {
       ENTER();
        auto& data = *new Data();
        data.out.add( data::Data::get() );
        
        auto& process = Test::process( "test=`cat`; echo $test" );
        
        process.out().add( data.out );
        data.in.add( process.in() );
        
        Test::assign( process, data );
    }
    
    virtual void onTimer( Event& timer )
    {
        ENTER();
        map();
        timer.deref();
    }
    
    virtual void si::check( )
    {
        ENTER();
        Test::si::check( "data" );
        Test::si::check( "string" );
        Test::si::check( "error" );
        Test::si::check( "exit" );
    }

    struct Data: Test::Data
    {
        ev::Till& in;
        ev::Till& out;

        Data( )
        : in( Till::get() ), out( Till::get() ) 
        {
        }

        ~Data( )
        {
            in.deref( );
            out.deref( );
        }
    };
    
private:
    Data m_test;
};

int main()
{
    new Process();
}
