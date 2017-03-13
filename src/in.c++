#include "in.h"
#include "trace.h"

namespace tau
{
    namespace in
    {
        unsigned int Male::dispatch( unsigned int type, Grain& grain )
        {
            ref();
            
            auto handled = 0;
            auto& females = m_types[ type ];
            
            TRACE( "dispatching event %d with dispatched 0x%x have %d listeners", type, &grain, females.size() );
            
            females.values( [ & ] ( Female* female )
            { 
                if ( female->handle( type, grain ) )
                {
                    handled ++;
                }
            } );
            
            deref();
            
            return handled;
        }
        
        
        bool Female::handle( unsigned int type, Grain& grain )
        {
            ENTER();
            
            try
            {
                m_type = type;
                
                auto handler = m_handlers.get( type ).handler;
                ( this->*handler )( grain );
                return true;
            }
            catch ( ... )
            {
                TRACE( "handler for %d not found", type );
            }
            
            return false;
        }   
        
        void Female::male( Male& male )
        {
            ENTER();
            male.females().add( *this, true );
            m_males[ ( unsigned long ) &male ] = &male;
        }
        
        void Female::unmale( Male& male )
        {          
            TRACE( "removing male 0x%x", &male );
            if ( m_males.remove( ( unsigned long ) &male ) )
            {
                male.females().remove( *this, true );
            }
        }
        
        void Female::clear()
        {
            ENTER();
            m_males.values( [ & ]( Male* male) { male->females().remove( *this, true ); } );
            m_males.clear();
        }
        
        void Male::Females::add( Female& female, bool ref )
        {
            TRACE( "0x%x: adding listener 0x%x with %d listeners", &male, &female, female.m_handlers.size() );
            
            female.types( [ & ] ( unsigned int type ) 
            { 
 //               TRACE( "found listener for event %u", type );
                types[ type ][ &female ] = &female;
            } );
        
            if ( ref )
            {
                male.ref();
            }
        }
        
        void Male::Females::remove( const Female& female, bool ref )
        {
            ENTER();
            female.types( [ & ] ( unsigned int type ) 
            { 
                bool removed = types[ type ].remove( ( unsigned long ) &female );
                TRACE( "0x%x: removed %d listener 0x%x for type %u", &male, removed, &female, type );
                if ( ref && removed )
                {
                    male.deref( );
                    ref = false;
                }
            } );
        }
        
        void Male::Females::clear( )
        {
            ENTER();
            types.clear();
        }
    }
}