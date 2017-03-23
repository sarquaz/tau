#include "tau.h"
#include "trace.h"

namespace tau
{
    static Main main;
    
    Data Strings::def( const Data& key, const Data& value ) const
    {
        Data ret;
        
        try
        {
            ret = li::Set< Data >::get( key );
        }
        catch ( ... )
        {
            ret = value;
        }
        
        TRACE( "%s", ret.c() );
        
        return ret;
    }
    
    void Main::Thread::run()
    {
        ENTER();
    }
    
    void start( )
    {
        start( {} );
    }
    
    void start( const Strings& options )
    {
        SENTER();
        auto threads = options.number( "threads" );
        STRACE( "%d", threads );
        
        main.start();
    }
    
    void stop()
    {
        SENTER();
    }
    
    void Main::start()
    {
        ENTER();
    }
}
