#ifndef TAU_IN_H
#define	TAU_IN_H

#include "types.h"
    

namespace tau
{    
    namespace in
    {
        class Male;
        
        class Female
        {
            friend class Male;
            
        public:
            virtual ~Female()
            {
                
            }

            typedef void ( Female::*Handler )( Grain& grain );
            
            void male( Male& male );
            void unmale( Male& male );
            
        protected:
            void handler( ui type, Handler handler )
            {
                m_handlers[ type ].handler = handler;
            }
            
            Female()
            : m_type( 0 )
            {
            }
            
            virtual bool handle( ui type, Grain& grain );
            void clear();
            
            ui current() const
            {
                return m_type;
            }
            
        private:
            struct Value
            {
                Handler handler;
                void clear()
                {
                    handler = NULL;
                }
            };
            

            template < class Type > void types( Type type ) const
            {
                m_handlers.keys( [ & ] ( ul key ){ type( key ); } );
            }
            
            
            li::Map< Value > m_handlers;
            li::Map< Male* > m_males;
            
            ui m_type;

        };
        
        class Male: public Rock    
        {
        public:
            virtual ~Male()
            {
            }
            
            ui dispatch( ui type )
            {
                return dispatch( type, *this );
            }            
            ui dispatch( ui type, Grain& grain );            
            
            typedef li::Map< li::Map< Female* > > Types;
            
            struct Females
            {
                
                Types& types;
                Male& male;
                
                bool empty() const
                {
                    return types.empty();
                }
                
                void add( Female&, bool ref = false );
                void remove( const Female&, bool ref = false );
                void clear( );
                
                Females( Male& male )
                : male( male ), types( male.m_types  )
                {
                }
            };
            
            Females& females()
            {
                return m_manager;
            }
            
            const Females& females() const
            {
                return m_manager;
            }
            
        protected:
            Male()
            : m_manager( *this )
            {
            }
            
            virtual void cleanup()
            {
                females().clear();
            }
            
        private:
            Types m_types;
            Females m_manager;
        };
    }
}

#endif