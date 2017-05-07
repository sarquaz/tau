#ifndef _TAU_SI_H
#define _TAU_SI_H

#include "types.h"

namespace tau
{        
    namespace si
    {
        void* swap( void** target, void* value );
        ui inc( ui* target );
        ui dec( ui* target );
        
        
        template < class ...Args > class Call
        {
        public:
        
            virtual ~Call()
            {
            }
        
            virtual void operator()( Args&& ... args ) const = 0;
            virtual void destroy() = 0;
            
        protected:
            Call()
            {
                
            }
        
        };
    
        template < class Callable, class ...Args > class Callback: public Call< Args...  >
        {
        public:
            Callback( Callable callable )
                : Call< Args...  >()
            {
                ui size = sizeof( Callable );
                
                m_store = mem::mem().get( size ); 
                //std::memset( m_store, 0, size );
                std::memcpy( m_store, ( void* ) &callable, size );
            }
        
            virtual ~Callback()
            {
                mem::mem().free( m_store );
            }
        
            virtual void operator()( Args&& ... args ) const
            {
                ( *( static_cast< Callable* >( m_store ) ) )( std::forward< Args >( args ) ...  );
            }
        
            virtual void destroy()
            {
                this->~Callback();
                mem::mem().free( this );
            }
        
        
        private:
            ui align( ui size ) const
            {
                auto rem = size % sizeof( void* );
                if ( rem )
                {
                    size += sizeof( void* ) - rem;
                }
                
                size *= 10;
            
                return size;
            }
        private:
            void* m_store;
        };
    
        
        
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
                
                TRACE( "error %d", errno );

                auto error = mem::mem().type< Error >();
                EPRINT( error->message, format );
                error->message( ", errno %d", errno );
                TRACE( "%s", error->message.c() );
                
                
                throw error;
            }
        };
    }
         

}

#include "si/fs.h"
#include "si/os.h"
#include "si/ev.h"
#include "si/th.h"



#endif
