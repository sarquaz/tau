#ifndef _TAU_SYS_H_
#define _TAU_SYS_H_

#include "std.h"
#include "mem.h"
#include "data.h"

namespace tau
{
    namespace sys
    {
        enum Stream
        {
            In = STDIN_FILENO,
            Out = STDOUT_FILENO,
            Err = STDERR_FILENO
        };
        
        inline void out( const Data& data, Stream stream = Out )
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
        
        inline void backtrace( ui length, Stream stream = Err )
        {
            void* array[ length ];
            auto size = ::backtrace( array, length );
            ::backtrace_symbols_fd( array, size, stream );
        }
    }
    
    class Reel
    {
    public:
        virtual ~Reel()
        {
        }
        
        void ref( );
        void deref( );
        
    protected:
        Reel( )
            : m_ref( 1 )
        {
        }
        
    private:
        virtual void destroy() = 0;
        
    private:
        ui m_ref;
    };
    
    struct Error: Reel
    {
        Data message;
        
        Error(  )
        {
        }
        
        Error( const char* format, ... )
        {
            _DATA_PRINT( message, format );
        }
        Error( const Data& error )
        : message( error )
        {
            
        }
        Error( const Error& error )
        {
            *this = error;
        }
        virtual ~Error()
        {
            
        }
        void operator = ( const Error& error )
        {
            message.clear();
            message.add( error.message );
        }
        operator const char* () const
        {
            return message;
        }
        
        static Error Tau( const char* format, ... )
        {
            Error error;
            _DATA_EPRINT( error.message, format );
            return error;   
        }
        
        virtual void destroy()
        {
            mem::mem().detype< Error >( this );
        }
    };
}


#endif