#ifndef _TAU_LOG_H
#define _TAU_LOG_H

#include "sys.h"

namespace tau
{
    namespace log
    {
        enum Level
        {
            Info,
            Error
        };
        
        class Trace
        {
        public:
            void operator()( Level level, const char *format, ... );
            
            Trace( const char* scope, void* _instance = NULL );
            Trace( const Trace& trace )
            : m_instance( trace.m_instance ), m_scope( trace.m_scope ), m_handler( trace.m_handler )
            {
            }
            virtual ~Trace()
            {
                
            }
            
        protected:
            typedef void ( Trace::*Handler )( Level level, Data& data );
            
            void handler( Handler handler )
            {
                m_handler = handler;
            }
            
            const Data& scope() const
            {
                return m_scope;
            }
            
        private:
            void context( const char* _scope );
            void handle( Level level, Data& data );
            
        private:
            void* m_instance;
            Data m_scope;
            Handler m_handler; 
        };
                
        struct Enter
        {
            Trace trace;   
        
            Enter( const Trace& trace );
            ~Enter( );
        };
    }
}

#endif 
