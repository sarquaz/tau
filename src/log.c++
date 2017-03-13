#include "trace.h"
#include "si.h"

namespace tau
{
    void Trace::handler( log::Level level, si::Data& data )
    {
        data( "%s [%u] %u", TAU_NAME, 0, /*si::Thread::id( ),*/ si::millis( ) );
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
            trace ( Info, "Leaving" );
        }
        
        namespace data
        {
            __thread si::Data* t_data = NULL;
            
            si::Data& data( ) 
            {
                if ( !t_data )
                {
                    t_data = new si::Data();
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
        
        void Trace::handle( Level level, si::Data& data )
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
                m_scope.setLength( brace );
            }
            
            m_scope.add( "()" );
        }
        
        void Trace::operator()( Level level, const char *format, ... )
        {
            auto& data = data::data();
            auto file = si::Stream::Out;
            
            handle( level, data );
            
            if ( level == Error )
            {
                file = si::Stream::Err;
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
            
            si::out( data, file );
            data.clear();
        }
    }
}
    