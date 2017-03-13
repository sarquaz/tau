#ifndef TAU_MEM_H
#define TAU_MEM_H

#include "std.h"
#include "../../src/trace.h"

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
            
            template < class Data, unsigned int Size > struct Root
            {
                Data data[ Size ];
                
                virtual ~Root()
                {
                }
                
                Data& operator []( unsigned int index )
                {
                    return data[ index ];
                }
                
                unsigned int size() const
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
                
                unsigned int count;
                unsigned long type;
                
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
                {
                    std::memset( data, 0, sizeof( data ) );
                }
            };
            
            Sizes& sizes();
                    
            template < class Type > class Bytes
            {
            public:
                Bytes( unsigned int chunk = 0 )
                : m_chunk( chunk ? chunk : sizeof( Type ) ), m_nodes( mem::sizes()[ typeid( Type ).hash_code() % Sizes::Size ] ),
                 m_max( 500 ), m_null( NULL ), m_hash( typeid( Type ).hash_code() )
                {                           
                    if ( !m_nodes.type  )
                    {
                        m_nodes.type = m_hash;
                    }
                    else
                    {
                        if ( m_nodes.type != m_hash )
                        {
                            assert( false );
                        }
                    }
                }
                
                Type* get( unsigned int chunks = 1, bool* reuse = NULL )
                {
                    auto& node = this->node( chunks );
                    
                    void* data = NULL;
                                        
                    if ( node )
                    {
                        data = node->data;
                        auto found = node;
                        node = node->next;
                                                
                        found->free();
                        m_nodes.count -= 1;
                        if ( reuse )
                        {
                            *reuse = true;
                        }
                    }
                    
                    if ( !data )
                    {
                        data = std::malloc( chunks * m_chunk );
                    }
                    
                    return static_cast< Type* >( data );
                }
                
                void free( Type* data, unsigned int chunks = 1 )
                {
                    auto& first = this->node( chunks );
                                        
                    if ( m_max > m_nodes.count )
                    {
                        auto& node = Node::get( data );
                                
                        node.next = first;
                        first = &node;
                                
                        m_nodes.count += 1;
                    }
                    else
                    {
                        std::free( data );
                    }
                }
                
                void setMax( unsigned int max )
                {
                    m_max = max;
                }
                
            protected:
                unsigned int m_chunk;
                
                
            private:
                Node*& node( unsigned int chunks )
                {                    
                    if ( chunks > Nodes::Size )
                    {
                        return m_null;
                    }

                    auto& node = m_nodes[ chunks - 1 ];
                    return node;
                }
                
            private:
                Nodes& m_nodes;
                unsigned int m_max;
                Node* m_null;
                unsigned long m_hash;
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
            
            namespace list
            {
                template < class Value > struct Node
                {
                    Value v;
                    Node* next;
                    Node* prev;

                    Node( const Value& value )
                    : next( NULL ), prev( NULL ), v( value )
                    {
                    }

                    Node( )
                    : next( NULL ), prev( NULL )
                    {
                    }
                    
                    void remove( )
                    {
                        if ( prev )
                        {
                            prev->next = next;
                        }

                        if ( next )
                        {
                            next->prev = prev;
                        }
                    }

                    void after( Node& node )
                    {
                        this->next = &node;
                        node.prev = this;
                        
                    }
                };
            }

            template < class Type > class Pile
            {               
                struct Chunk
                {
                    unsigned int got;
                    unsigned int count;
                    unsigned int freed;
                    void* start;
                    
                    Chunk( unsigned int _count = 0 )
                    : count( _count ), got( 0 ), freed( 0 ) 
                    {
                        
                    }
                };
                
                typedef list::Node< Chunk > Node;
                
                template < class T > struct _Item
                {
                    Node* node;
                    T type;
                };
                
                typedef _Item< Type > Item;
                
                enum 
                {
                    Count = 10
                };
                
            public:
                unsigned int count;
                
                Pile( unsigned int _count = Count )
                : count( _count ), m_list( NULL ), m_size( 0 )
                {
                }
                
                ~Pile()
                {
                    clear();
                }
                
                template < class ... Args > Type& get( Args&&... args )
                {
                    if ( m_list )
                    {
                        
                        auto& count = m_list->v.got;
                                                
                        if ( count < m_list->v.count )
                        {
                            auto item = ( Item* ) ( ( unsigned long ) m_list->v.start + count * sizeof( Item ) );                            
                            auto type = &item->type;
                            item->node = m_list;
                                                        
                            count ++;
                            set( [ & ] ( ) { m_size ++; } );
                            
                            new ( type ) Type( std::forward< Args >( args ) ... );
                            return *type;
                        }
                    }

                    auto& node = this->node( );
                    if ( m_list )
                    {
                        m_list->after( node );
                    }

                    m_list = &node;
                    
                    return get( std::forward< Args >( args ) ... );
                }
                
                void free( Type& type )
                {
                    type.~Type();
                    
                    auto item = ( Item* ) ( ( unsigned long ) &type - sizeof( Node* ) );
                    auto node = item->node;
                    
                    auto& freed = node->v.freed;
                    freed ++;

                    set( [ & ] ( ) { m_size --; } );
                    
                    if ( freed == node->v.count )
                    {
                        if ( m_list == node )
                        {
                            m_list = dynamic_cast< Node* > ( node->prev );
                        }
                        
                        free( *node );
                    }
                }
                
                void free( Node& node )
                {
                    node.remove();
                    m_items.free( ( Item* ) ( node.v.start ), node.v.count );
                    m_nodes.free( node );                        
                }
                
                void clear()
                {
                    auto& node = m_list;
                    while ( node )
                    {
                        auto prev = node->prev;
                        free( *node );
                        node = prev;
                    }
                    
                    m_size = 0;
                    count = Count;
                }
                
                unsigned int size() const
                {
                    return m_size;
                }
                
            private:
                template < class Op > void set( Op op )
                {
                    auto size = m_size;
                    op( );
                    
                    if ( !( m_size % Count ) )
                    {
                        auto& count = this->count;
                        auto change = m_size / count;
                        count = ( m_size > size ? +change : -change ) + count;
                    }                    
                }
                
                Node& node()
                {
                    auto& node = m_nodes.get();
                    
                    node.v.start = m_items.get( count );
                    node.v.count = count;
                    
                    return node;
                }
                                
            private:
                Types< Node > m_nodes;
                Bytes< Item > m_items;
                Node* m_list;
                unsigned int m_size;
            };
        }
    }
}

#endif