#ifndef _TAU_SI_H
#define _TAU_SI_H

namespace tau
{        
    namespace si
    {
        void* swap( void** target, void* value );
        ui inc( ui* target );
        ui dec( ui* target );
        
        struct check
        {
            ui skip;
            long result;

            check( long _result, ui _skip = EWOULDBLOCK )
            : skip( _skip ), result( _result )
            {
            }

            ul operator()( const char* format, ... )
            {
                if ( errno == skip || result != -1 )
                {
                    return result;
                }
                
                

                auto error = mem::mem().type< Error >();
                _DATA_EPRINT( error->message, format );
                error->message( ", errno %d", errno );
                sys::out( error->message.c() );
                
                
                throw error;
            }
        };
    }
}

#include "si/fs.h"
#include "si/os.h"
#include "si/ev.h"
#include "si/newev.h"
#include "si/th.h"



#endif
