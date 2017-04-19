#ifndef _TAU_MEM_H_
#define _TAU_MEM_H_

namespace tau
{
    namespace mem
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
            
            struct Nodes
            {
                Node* node;            
            
                Nodes();            
                ~Nodes();
            
                Node* get();
                void free( Node* node );
            };
            
            Nodes& nodes()
            {
                return m_nodes;
            }
            
            Mem();
            ~Mem();
                
            
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
            Nodes m_nodes;
        };
        
        
        Mem& mem();
        void disable();
    }
}

#endif0