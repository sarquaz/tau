#include "tau.h"

using namespace tau;

int main()
{
    li::Set< int > set;
    
    for ( auto i = 0; i < 10; i++ )
    {
        set.set( i );
    }
    
    STRACE( "length %d", set.length() );
    set.values( [ ] ( int n ) { STRACE( "%d", n ); } );
    
    return 0;
}