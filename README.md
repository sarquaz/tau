tau is a C++ asynchronous IO library designed to be fast, minimal and to allow producing beautiful C++ code that does complex things without low-level complexity of system calls.

tau is not using any external libraries, is multithreaded, has functions for asynchronous file, network and process IO. to be as efficient as possible it has memory management with reference counting and minimalistic replacements for std::string, std::list and std::map

tau is working under OS X, linux support, extensive testing and documentation is planned. 

Building

`make` builds the library (edit `config.mk` to select between static and shared builds)
		
`make test` runs test cases		

Usage

consider following code that creates TCP echo server on port 10000 (compile with `-I../include/tau  -std=c++11` or `cd examples && make`):

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

	


