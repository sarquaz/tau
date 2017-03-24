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

        auto threads = options.number( "threads" );
        threads = threads ? threads : 1;
        STRACE( "need to start %d threads", threads );        
        
        main.start( threads );
    }
    
    void stop()
    {
        SENTER();
        
        main.stop();
    }
    
    void listen( Listener* listener )
    {
        main.listener( listener );
    }
    
    void Result::dispatch( Result* what ) const
    {
        ENTER();
        
        m_listeners.all( [ & ] ( Listener* listener ) { listener->result( what ); } );
    }
    
    void Main::start( us threads )
    {
        ENTER();
        
        for ( auto i = 0; i < threads; i++ )
        {
            //
            //  create new thread
            //
            auto thread = new Thread();
            
            //
            //  send it to listener
            //
            Result::dispatch( thread );
            
            //
            //  start the thread
            //
            thread->start();
            
            m_lock.with( [ & ] ( ) { m_threads.add( thread ); } );
        }                
    }
    
    void Main::stop( )
    {
        m_threads.all( [ & ] ( Thread* thread ) { thread->join(); } );
    }
    
    
    
}
