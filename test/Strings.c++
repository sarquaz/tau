#include "si.h"

using namespace tau;

#include "../../src/trace.h"

//
using namespace tau;
using namespace si;

int main( )
{
    SENTER();
    
    li::Strings strings;
    
    
//    for ( auto i = 0; ; i++ )
//    {
//        
//        auto key = Data::get( );
//        
//        
//        STRACE( "%d string %s hash %lu", i, key.c(), key.hash() );
//        
//        try
//        {
//            strings.get( key );            
//            assert( false );
//        }
//        catch ( ... )
//        {
//            
//        }
//        
//        strings.set( key );
//        
//        if ( i == 20000 )
//        {
//            break;
//        }
//    }
    
    Data path = "test";
    STRACE( "%s", path.c() );
    auto info = Fs::info( path );
    
    STRACE( "dir %d", info.type() == Fs::Dir );
        
}


