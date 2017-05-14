#ifndef _TAU_TRACE_H_
#define _TAU_TRACE_H_

#include "log.h"

namespace tau
{
    struct Trace: log::Trace
    {
        Trace( const char* scope, void* instance = NULL )
        : log::Trace( scope, instance )
        {
            log::Trace::handler( ( log::Trace::Handler ) &Trace::handler );
        }
        
        Trace( const Trace& trace )
        : log::Trace( trace )
        {
            
        }
        
        virtual ~Trace()
        {
        }
        
        void handler( log::Level level, Data& data );
        
    };
    
#ifdef LOG 
    #define ENTER() log::Enter _e( Trace( __PRETTY_FUNCTION__, ( void* ) this  ) );
    #define SENTER() log::Enter _e( Trace( __PRETTY_FUNCTION__ ) );
    #define TRACE( format, ... ) Trace( __PRETTY_FUNCTION__, ( void* ) this  )( log::Info, format, __VA_ARGS__ );
    #define STRACE( format, ... ) Trace( __PRETTY_FUNCTION__ )( log::Info, format, __VA_ARGS__ );
#else
    #define ENTER() 
    #define SENTER() 
    #define TRACE( format, ... )
    #define STRACE( format, ... )
#endif 
}

#endif 
