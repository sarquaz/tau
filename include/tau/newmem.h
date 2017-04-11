#ifndef _TAU_NEWMEM_H_
#define _TAU_NEWMEM_H_

#include "box.h"
#include "../../src/trace.h"

namespace tau
{
    namespace newmem
    {
        struct Allocator
        {
            static void* allocate( ui size );

            static void deallocate( void* data );
        };

        typedef box::list::_Node< void*, Allocator > Node;
        
        typedef box::list::List< void*, Allocator > List;
        
        class Mem
        {
        public:
            Mem()
                : m_total( 1024 * 1024 * 10 ), m_used( 0 )
            {
            }
            
            void* get( ui size );
            void free( void* data );
            
            template < class Type, class ... Args > Type* type( Args&&... args )
            {
                auto type = ( Type* ) get( sizeof( Type ) );
                
                new ( type ) Type( std::forward< Args >( args ) ... );
                
                return type;
            }
            template< class Type > void detype( Type* type )
            {
                type->~Type();
                free( type );
            }
            
             
        private:
            box::map::Map< List, 0x100 > m_map;
            ul m_total;
            ul m_used;
        };
    }
}

#endif