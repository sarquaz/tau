#include "log.h"
#include "trace.h"

namespace tau
{
    void Trace::handler( log::Level level, data::Data& data )
    {
        data( "%s [%u] %u", TAU_NAME, out::id( ), time().ms() );
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
        
        namespace d
        {
            __thread data::Data* t_data = NULL;
            
            data::Data& data( ) 
            {
                if ( !t_data )
                {
                    t_data = new data::Data();
                }
                
                return *t_data;
            }
        }
        
        Trace::Trace( const char* scope, void* _instance  )
            : m_instance( _instance ), m_handler( NULL ) 
        {
            if ( scope )
            {
                context( scope );
            }
        }
        
        void Trace::handle( Level level, data::Data& data )
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
            auto& data = d::data();
            auto file = out::Stream::Out;
            
            handle( level, data );
            
            if ( level == Error )
            {
                file = out::Stream::Err;
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
                        
            PRINT( data, format );
            data.add( "\n" );
            
            out::out( data, file );
            data.clear();
        }
    }
}
    