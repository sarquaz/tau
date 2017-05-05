tau is a C++ system library designed to be fast, minimal and to allow producing beautiful C++ code that does complex things without low-level complexity of system calls. tau is multithreaded, has functions for asynchronous file, network and process IO, memory management with reference counting and has minimalistic replacements for std::string, std::list and std::map

tau is under heavy development

Following C++ code that starts a thread pool and reads file asynchronously is possible with tau:


	#include "tau.h"
	
	using namespace tau;
	
	int main()
	{
	        tau::start( [ ] ( )
	        { 
	            io::file( "Makefile" ).read( [ & ]( ev::Request& request )
	                {
	                     request.data().add( '\0' );
	                     printf( "%s", request.data().c() );
                 
	                     tau::stop( [](){ printf("stopped\n"); } );
                     
	                } ).deref();
	        } );
		
		return 0;
	}

