#ifndef _TAU_TAU_H_
#define _TAU_TAU_H_

#include "std.h"

namespace tau
{
    // namespace si
    // {
    //     void* swap( void** target, void* value );
    //     ui inc( ui* target );
    //     ui dec( ui* target );
    // }
}

#include "out.h"
#include "mem.h"
#include "box.h"

namespace tau
{
    typedef data::Data Data;
    
    namespace box
    {
        namespace h
        {
        
            //
            //  Data
            //   
            template <> struct hash< data::Data >
            {
                ul operator()( const data::Data& data ) const
                {
                    return data.hash();
                }
            };
        }
    }
    
}



#include "si.h"
#include "li.h"
#include "main.h"


#endif