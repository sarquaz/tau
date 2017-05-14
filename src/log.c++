#include "log.h"
#include "trace.h"

namespace tau
{
    void Trace::handler( log::Level level, Data& data )
    {
        data( "%s [%u] %u", TAU_NAME, sys::id( ), time().ms() );
    }
            
    namespace log
    {
        Enter::Enter( const Trace& _trace )
            : trace( _trace )
        {
            trace( Info, "Entering" );
        }

        Enter::~Enter( )
        {
            trace( Info, "Leaving" );
        }
                
        Trace::Trace( const char* scope, void* _instance  )
            : m_instance( _instance ), m_handler( NULL ) 
        {
            if ( scope )
            {
                context( scope );
            }
        }
        
        void Trace::handle( Level level, Data& data )
        {
            if ( m_handler )
            {
                ( this->*m_handler )( level, data );
            }
        }
        
        void Trace::context( const char* _scope ) 
        {
            m_scope.add( _scope );
            
            int brace = m_scope.find( "(" );
            if ( brace > 0 )
            {
                m_scope.length( brace );
            }
            
            m_scope.add( "()" );
        }
        
        void Trace::operator()( Level level, const char *format, ... )
        {
            Data data;
            auto file = sys::Stream::Out;
            
            handle( level, data );
            
            if ( level == Error )
            {
                file = sys::Stream::Err;
            }
            else if ( level == Info )
            {
                if ( data.length() )
                {
                    data.add( " " );
                }
                
                data.add( m_scope );

                if ( m_instance )
                {
                    data( "(0x%x)", m_instance );
                }
            }

            if ( data.length() )
            {
                data.add( ": " );
            }
                        
            _DATA_PRINT( data, format );
            data.add( "\n" );
            
            sys::out( data, file );
        }
    }
}
    