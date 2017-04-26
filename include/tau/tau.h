#ifndef _TAU_TAU_H_
#define _TAU_TAU_H_

#include "std.h"
#include "out.h"
#include "mem.h"


namespace tau
{
    typedef data::Data Data;
}

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
            Usec
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