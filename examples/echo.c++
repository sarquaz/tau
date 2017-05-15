#include "tau.h"

using namespace tau;    

int main( )
{
    struct Result: io::Result
    {
            virtual void event( ui event, ev::Request& request )
            {                
                auto& net = dynamic_cast< io::Net& >( request.parent() );
                
                switch ( event )
                {
                    case io::Net::Listen:
                        printf( "listening on port %u \n", net.port() );
                        break;
                        
                    case io::Net::Accept:
                    {
                        auto address = ( fs::Link::Address* ) request.custom();
                        printf( "accepted connection from %s \n", address->tostring().c() );
                    }    
                    
                    case io::Net::Read:
                        net.write( request.data() );
                        break;
                        
                    case io::Net::Close:
                        printf( "connection closed \n");
                        net.deref();
                        break;    
                        
                    case io::Net::Error:
                    case io::Event::Error:
                        printf( "error: %s \n", request.error()->message.c() );
                        assert( false );
                        break;
                }
            }
            
            virtual void destroy()
            {
                mem::mem().detype< Result >( this );
            }
            
            virtual ~Result()
            {
                
            }
        };
    
       
        tau::start( [ & ] ( )
        {
            auto result = mem::mem().type< Result >();
            
            io::net( *result, {{ options::Port, 10000 }, { options::Server, true } } );
            result->deref();
        } );

        return 0;
}