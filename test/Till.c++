#include "Test.h"

using namespace tau;
using namespace tau::io;

TEST();

class TestTill: public Test
{
public:
    TestTill(  )
    : m_count( 0 )
    {
        Test::setInterval( Time( 100 ) );
        
        handler( io::Net::Accept, ( Test::Handler ) &TestTill::onAccept );
    }
   
    virtual ~TestTill()
    {
    }
    
private:
    struct Data: Test::Data
    {
        Till* data;
        
        Data()
        : data( &Till::get() )
        {
            
        }
        
        Till& till()
        {
            return *data;
        }
        
        ~Data()
        {
            data->deref();
        }
    };
    virtual void run( )
    {
        ENTER();        

        auto data = new Data();
        data->till().add( si::Data::get( 100 ) );
        auto& server = Test::server();
        Test::assign( server, *data );
        
        auto& client = Test::client( server );
        client.out().add( data->till() );
    }
    
    void onAccept( Grain& grain )
    {
        ENTER();
        
        auto& net = dynamic_cast< Net& >( grain );
        net.females().add( *this );
        
        Test::assign( net, Test::data( *net.parent() ) );
        
        if ( !m_count )
        {
            auto& client = Test::client( *net.parent() );
            client.out().add( net.in() );
            m_count ++;
        }
        
        net.enable();
    }
    
    virtual void onTimer( Event& timer )
    {
        ENTER();
        
    }
    
    void onRead( Grain& grain )
    {
        ENTER();
        
        auto& net = dynamic_cast< Net& >( grain );        
        
        out( "%s", ( const char* ) net.in().data() );
        
        auto& data = dynamic_cast< Data& > ( Test::data( net ) );
        assert( data.till().data() == net.in().data() );
        Test::set( "data" );
    }
    
    void onWrite( Grain& grain )    
    {
        ENTER();        
    }
    
    void onClose( Grain& grain )
    {
        ENTER();
    }
    
    virtual void check()
    {
        ENTER();
        Test::check( "data" );
    }
    
private:
    unsigned int m_count;
};

int main()
{
   return TestTill()();
}