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
            void handler( unsigned int type, Handler handler )
            {
                m_handlers[ type ].handler = handler;
            }
            
            Female()
            : m_type( 0 )
            {
            }
            
            virtual bool handle( unsigned int type, Grain& grain );
            void clear();
            
            unsigned int current() const
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
                m_handlers.keys( [ & ] ( unsigned long key ){ type( key ); } );
            }
            
            
            li::Map< Value > m_handlers;
            li::Map< Male* > m_males;
            
            unsigned int m_type;

        };
        
        class Male: public Rock    
        {
        public:
            virtual ~Male()
            {
            }
            
            unsigned int dispatch( unsigned int type )
            {
                return dispatch( type, *this );
            }            
            unsigned int dispatch( unsigned int type, Grain& grain );            
            
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