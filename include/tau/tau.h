#ifndef _TAU_TAU_H_
#define _TAU_TAU_H_

#include "std.h"
#include "sys.h"

#include "li.h"

namespace tau
{
    namespace options
    {
        enum 
        {
            Threads,
            Repeat,
            Msec,
            Usec,
            Server,
            Port,
            Type
        };
    }
    
    namespace action
    {
        enum Action
        {
            Start,
            Stop
        };
    }
    
    typedef li::Set< ui > Options;
    
}

#include "si.h"
#include "io.h"
#include "main.h"


#endif