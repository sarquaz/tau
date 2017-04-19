#include "tau.h"
#include "trace.h"

namespace tau
{
    static Main s_main;
    __thread si::os::Thread* t_thread = NULL;

    Main& Main::instance()
    {
        return s_main;
    }
    
    si::os::Thread& Main::thread()
    {
        assert( t_thread );
        
        return *t_thread;
    }
        
    void Main::started( si::os::Thread* thread )
    {
        t_thread = thread;
    }
}
