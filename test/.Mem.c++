#include "types.h"

using namespace tau;

#include "../../src/trace.h"
#include <map>

int main( )
{
    SENTER();
    li::Mass< int > mass;
    
    for ( auto i = 0; i < 20; ++i )
    {
        mass.add( i );
    }
    
    
    mass.all( []( int n ){ printf( "%d \n", n ); } );
}