#ifndef _TAU_STREAM_H_
#define _TAU_STREAM_H_

#include "std.h"
#include "data.h"

namespace tau
{
    namespace out
    {
        enum Stream
        {
            In = STDIN_FILENO,
            Out = STDOUT_FILENO,
            Err = STDERR_FILENO
        };
        
        inline void out( const data::Data& data, Stream stream = Out )
        {
            ::write( stream, data, data.length() );
        }
        
        inline ul id()
        {
#ifdef __MACH__
                uint64_t tid;
                ::pthread_threadid_np( NULL, &tid );
                return tid;
#else
                return ::pthread_self();    
#endif
            
        }
        
        inline void backtrace( ui length, Stream stream )
        {
            void* array[ length ];
            auto size = ::backtrace( array, length );
            ::backtrace_symbols_fd( array, size, stream );
        }
    }
}


#endif