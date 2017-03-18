#ifndef _TAU_MEM_H
#define _TAU_MEM_H

#include "std.h"

namespace tau
{
    namespace si
    {
        namespace mem
        {
            struct Node
            {
                void* data;
                Node* next;
                
                void free();
                static Node& get( void* data );
                
                void clear()
                {
                    next = NULL;
                    data = NULL;
                }
            };
            
            template < class Data, ui Size > struct Root
            {
                Data data[ Size ];
                
                virtual ~Root()
                {
                }
                
                Data& operator []( ui index )
                {
                    return data[ index ];
                }
                
                ui size() const
                {
                    return Size;
                }
            };
            
            struct Nodes: Root< Node*, 0x20 >
            {
                enum 
                {
                    Size = 0x20
                };
                
                ui count;
                ul type;
                
                Nodes()
                {
                }
            };
            
            struct Sizes: Root< Nodes, 0x500 >
            {
                enum 
                {
                    Size = 0x500
                };                
                
                Sizes()
                : used( 0 ), max( 1024 * 1024 * 5 /* 5 MB default */)
                {
                    std::memset( data, 0, sizeof( data ) );
                }
                
                ui used;
                ui max;
            };
            
            Sizes& sizes();
                    
            template < class Type > class Bytes
            {
            public:
                Bytes( ui chunk = 0 )
                : m_chunk( chunk ? chunk : sizeof( Type ) ), m_null( NULL ), m_hash( typeid( Type ).hash_code() )
                {                          
                    m_nodes = &mem::sizes()[ m_hash % Sizes::Size ];
                            
                    if ( !m_nodes->type  )
                    {
                        m_nodes->type = m_hash;
                    }
                    else
                    {
                        if ( m_nodes->type != m_hash )
                        {
                            assert( false );
                        }
                    }
                }
                
                Type* get( ui chunks = 1, bool* reuse = NULL )
                {
                    void* data = NULL;
                    //
                    //  lookup the  node
                    //
                    auto& node = this->node( chunks );

                    //
                    //  if it was found reuse it
                    //    
                    if ( node )
                    {
                        data = node->data;
                        auto found = node;
                        node = node->next;
                                                
                        found->free();
                        m_nodes->count -= 1;
                        if ( reuse )
                        {
                            *reuse = true;
                        }
                    }
                    //
                    //  if not call malloc
                    //
                    if ( !data )
                    {
                        data = std::malloc( chunks * m_chunk );
                    }
                    
                    return static_cast< Type* >( data );
                }
                
                void free( Type* data, uint chunks = 1 )
                {
                    auto freed = sizeof( *data ) * chunks * m_chunk;
                    auto& first = this->node( chunks );
                    
                    if ( sizes().used < sizes().max )
                    {
                        auto& node = Node::get( data );
                                
                        node.next = first;
                        first = &node;
                        
                        sizes().used += freed;
                    }
                    else
                    {
                        std::free( data );
                    }
                }
            protected:
                ui m_chunk;
                
            private:
                Node*& node( ui chunks )
                {                    
                    if ( chunks > Nodes::Size )
                    {
                        return m_null;
                    }

                    auto& node = ( *m_nodes ) [ chunks - 1 ];
                    return node;
                }
                
            private:
                Nodes* m_nodes;                
                Node* m_null;
                ul m_hash;
            };
            
            typedef Bytes< char > Char;
            
            template< class Type > class Types: public Bytes< Type >
            {
            public:
                bool reuse;
                
                Types()
                : Bytes< Type >( sizeof( Type ) ), reuse( false )
                {
                }
                
                template < class ... Args > Type& get( Args&&... args )
                {   
                    reuse = false;
                    
                    auto type = Bytes< Type >::get( 1, &reuse );
                    new ( type ) Type( std::forward< Args >( args ) ... );
                    
                    return *type;
                }
                
                void free( Type& type )
                {
                    type.~Type();
                    Bytes< Type >::free( &type );
                }
            };
        }
    }
}

#endif