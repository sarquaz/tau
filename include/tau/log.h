#ifndef TAU_DEBUG_H
#define TAU_DEBUG_H

#include "out.h"

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
            typedef void ( Trace::*Handler )( Level level, data::Data& data );
            
            void handler( Handler handler )
            {
                m_handler = handler;
            }
            
            const data::Data& scope() const
            {
                return m_scope;
            }
            
        private:
            void context( const char* _scope );
            void handle( Level level, data::Data& data );
            
        private:
            void* m_instance;
            data::Data m_scope;
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
