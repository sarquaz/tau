tau is a C++ system library designed to be fast, minimal and help in producing beautiful C++ code that does a lot of things. tau is multithreaded, has functions for asynchronous file, network and process IO, memory management with reference counting and has minimalistic replacements for std::string, std::list and std::map

tau is still under development

Following C++ code that starts a thread pool and reads file asynchronously is possible with tau:


	#include "tau"
	
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

