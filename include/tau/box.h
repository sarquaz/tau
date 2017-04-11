#ifndef _TAU_BOX_H_
#define _TAU_BOX_H_

#include "../../src/trace.h"

namespace tau
{
    namespace box
    {
        enum 
        {
            Size = 2
        };

        struct Allocator
        {
            static void* allocate( ui size )
            {
                return std::malloc( size );
            }
    
            static void deallocate( void* data )
            {
                return std::free( data );
            }
        };
        
        template < class Allocator > inline void* allocate( ui size )
        {
            return Allocator::allocate( size );
        };
        
        template < class Allocator > inline void deallocate( void* what )
        {
            return Allocator::deallocate( what );
        };
        
        
        template < class Type, class Allocator, class ... Args > inline Type* type( Args&&... args )
        {
            //
            //  allocate memory
            //
            auto type = ( Type* ) Allocator::allocate( sizeof( Type ) );
            //
            //  call constructor
            //
            new( type ) Type( std::forward< Args >( args ) ... );
            
            return type;
        };
        
        template < class Type, class Allocator > inline void detype( Type* type )
        {
            //
            //  call destructor
            //
            type->~Type();
            //
            //  free memory
            //    
            return Allocator::deallocate( ( void* ) type );
        };
        
        class Hash
        {
        public:
            Hash( uchar* what, ui size = sizeof( ul ) )
                : m_what( what ), m_size( size )
            {
            }

            ul operator()() const;

            ~Hash ()
            {
            }

        private:
            unsigned char* m_what;
            ui m_size;
        };
        
        namespace h
        {
            template < class What > struct hash
            {
                ul operator()( const What& what ) const;
            };
            
            //
            //  Template specializations for integral types
            //
    
            //
            //  ushort
            //
            template <> struct hash < us >
            {
                ul operator()( const us& what ) const
                {
                    return box::Hash( ( uchar* ) &what, sizeof( us ) )();
                }
            }; 
            //
            //  uint
            //
            template <> struct hash < ui >
            {
                ul operator()( const ui what ) const
                {
                    return box::Hash( ( uchar* ) &what, sizeof( ui ) )();
                }
            }; 
            //
            //  ulong
            //
            template <> struct hash < ul >
            {
                ul operator()( const ul& what ) const
                {
                    return box::Hash( ( uchar* ) &what )();
                }
            }; 
            //
            //  int
            //
            template <> struct hash < int >
            {
                ul operator()( const int& what ) const
                {
                    return box::Hash( ( uchar* ) &what, sizeof( int ) )();
                }
            };
            //
            //  long
            //
            template <> struct hash < long >
            {
                ul operator()( const long& what ) const
                {
                    return box::Hash( ( uchar* ) &what, sizeof( long ) )();
                }
            };
        }
        
        namespace list
        {
            enum Direction 
            {
                Tail,
                Head
            };
            
            template < class, class > class List;
                    
            template < class Data, class Allocator > class _Node
            {                
                template < class, class > friend class List;
                
            public:
                
                _Node( const Data& data, List< Data, Allocator >* list = NULL )
                    : m_data( data ), m_next( 0 ), m_prev( 0 ), m_list( list ), m_custom( NULL )
                {
                }
            
                ~_Node()
                {
                }
            
                void link( _Node* node, Direction direction = Tail )
                {
                    if ( direction == Tail )
                    {
                        m_next = node;
                        node->m_prev = this;
                    }
                    else
                    {
                        m_prev = node;
                        node->m_next = this;
                    }
                }

                const Data& data() const
                {
                    return m_data;
                }
            
                Data& data() 
                {
                    return m_data;
                }
            
                _Node* next() const
                {
                    return m_next;
                }
            
                _Node* next( _Node* next )
                {
                    m_next = next;
                    return next;    
                }
            
                _Node* prev() const
                {
                    return m_prev;
                }
            
                _Node* prev( _Node* prev )
                {
                    m_prev = prev;
                    return prev;
                }
            
                List< Data, Allocator >* list() const
                {
                    return m_list;
                }
            
                void remove()
                {
                    //
                    //  update pointers
                    //
                    if ( m_prev )
                    {
                        m_prev->m_next = m_next;
                    }
                
                    if ( m_next )
                    {
                        m_next->m_prev = m_prev;
                    }
                    //
                    //  let list know
                    //
                    m_list->update( this, List< Data, Allocator >::Remove );
                
                    //
                    //  free memory
                    //  
                    box::detype< _Node, Allocator >( this );
                }
            
                void* custom() const
                {
                    return m_custom;
                }
            
                void* custom( void* custom )
                {
                    m_custom = custom;
                    return m_custom;
                }
            
            private:
                Data m_data;
                _Node* m_next;
                _Node* m_prev;
                List< Data, Allocator >* m_list;
                void* m_custom;
            };
            
            template < class Data, class Allocator = box::Allocator > class List
            {
                template< class _Data, class _Allocator > friend class _Node;
            
            public:
                enum Action
                {
                    Add,
                    Remove
                };
            
                typedef _Node<  Data, Allocator > Node;
            
                List( )
                    : m_tail( NULL ), m_head( NULL ), m_length( 0 )
                {
                }
            
                List( const List& list )
                    : m_tail( NULL ), m_head( NULL ), m_length( 0 )
                {
                    operator = ( list );
                }
            
                virtual ~List()
                {
                    clear();
                }
                
                void operator = ( const List& list )
                {
                    list.nodes( [ & ]( Node* node ) { append( node->data() ); } );
                }
            
                Node* tail() const
                {                    
                    return m_tail;
                }
            
                Node* head() const
                {                    
                    return m_head;
                } 
            
                Node* append( const Data& data )
                {
                    return add( data, Tail );
                }
            
                Node* prepend( const Data& data )
                {
                    return add( data, Head );
                }
            
                template < class All > void all( All all, Direction direction = Head ) const
                {
                    ( const_cast< List* >( this ) )->nodes( [ & ]( Node* node ){ all( node->data() ); }, direction );
                }
            
                template < class Nodes > void nodes( Nodes nodes, Direction direction = Head ) 
                {
                    auto node = direction == Head ? m_head : m_tail;
                    
                    for ( ;; )
                    {
                        if ( !node )
                        {
                            break;
                        }
                        //
                        //  get next node since node might be freed in provided function
                        //
                        auto next = direction == Head ? node->next() : node->prev();
                        //
                        //  call provided function
                        //
                        nodes( node );
                        //
                        //  next item in the list
                        //
                        node = next; 
                    }
                }
            
                template < class First > Node* first( First first, Direction direction = Head )
                {
                    Node* matched;
                
                    try
                    {
                        nodes( [ & ]( Node* node )
                            {
                                 if ( first( node->data() ) ) 
                                 {
                                     throw node;
                                 }
                              }, direction );
                    }
                    catch ( Node* node )
                    {
                        return node;
                    }
                
                    return NULL;
                }
                
                ui length() const
                {
                    return m_length;
                }
            
                void clear()
                {
                    nodes( [ & ]( Node* node ) { node->remove(); }, Tail );
                }    
            
            private:
                Node* add( const Data& data, Direction direction = Head )
                {
                    auto node = this->node( data );
                
                    if ( !m_head || !m_tail )
                    {
                        init( node );
                    }
                    else
                    {
                        Node*& list = direction == Head ? m_head : m_tail;
                        list->link( node, direction );
                        list = node;
                    }
                
                    update( node, Add );

                    return node;
                }
            
                void init( Node* node )
                {
                    m_head = node;
                    m_tail = node;
                }
                
                Node* node( const Data& data )
                {
                    return box::type< Node, Allocator >( data , this );
                }
            
                void update( Node* node, Action action, Direction direction = Head )
                {
                    switch ( action )
                    {
                        case Add:
                            {
                                node->m_list = this;
                                m_length ++;
                            }
                            
                            break;
                    
                        case Remove:
                            {
                                if ( m_tail == node )
                                {
                                    m_tail = NULL;
                                }
                            
                                if ( m_head == node )
                                {
                                    m_head = NULL;
                                }
                            
                                m_length --;
                            }
                            
                            break;    
                    }
                }
            
            private:
                Node* m_head;
                Node* m_tail;
                ui m_length;
            };
        }
        
        namespace map
        {
            class Sizeable
            {
            public:
                virtual ~Sizeable()
                {
                }
                
                ui size() const
                {
                    return m_size;
                }
                
                void inc( ui size = 1 )
                {
                    m_size += size;
                }
                
                void dec( ui size = 1 )
                {
                    m_size -= size;
                }
                
            protected:
                Sizeable() 
                    : m_size( 0 )
                {
                    
                }
                
                Sizeable( const Sizeable& other )
                    : m_size( other.m_size )
                {
                }
                
            private:
                ui m_size;
            };
            
            template < class, ui, class > class Map;
            
            template < class Data, ui Size, class Allocator = box::Allocator > class _Node
            {
                template < class, ui, class  > friend class Map;
                
            public:
                _Node( ul hash )
                    : m_map( NULL ), m_hash( hash ), m_sizeable( NULL )
                {
                }
                
                ~_Node()
                {
                    if ( m_map )
                    {
                        box::detype< Map< Data, Size, Allocator >, Allocator >( m_map );
                    }
                    
                }
                
                Data& data()
                {
                    return m_data;
                }
                
                Map< Data, Size,  Allocator >* map() const
                {
                    return m_map;
                }
                
                ul hash() const
                {
                    return m_hash;
                }
                
                _Node*& operator []( ul hash )
                {
                    if ( !m_map )
                    {
                        m_map = box::type< Map< Data, Size,  Allocator >, Allocator >( );              
                        m_map->sizeable() = m_sizeable;
                    }
                    
                    return m_map->find( hash );
                }
                
                Sizeable*& sizeable()
                {
                    return m_sizeable;
                }
                
            private:
                ul m_hash;
                Data m_data;
                Map< Data, Size,  Allocator >*  m_map;
                Sizeable* m_sizeable;
            };
            
            template < class Data, ui Size = box::Size, class Allocator = box::Allocator > class Map: Sizeable
            {
                template < class _Data, ui _Size, class _Allocator  > friend class _Node;
                
            protected:
                typedef _Node< Data, 0x10, Allocator > Node;
                
            public:
                Map( )
                    : Sizeable(), m_sizeable( NULL )
                {
                    std::memset( m_nodes, 0, sizeof( m_nodes ) );
                }
                
                Map( const Map& map )
                    : Sizeable(), m_sizeable( NULL )
                {
                    std::memset( m_nodes, 0, sizeof( m_nodes ) );
                    operator = ( map );
                }
                
                virtual ~Map()
                {
                    for ( auto i = 0; i < Size; i++ )
                    {
                        auto node = m_nodes[ i ];
                        if ( !node )
                        {
                            continue;
                        }
                        
                        box::detype< Node, Allocator >( node );
                    }
                }
                
                ui length() const
                {
                    return Sizeable::size();
                }
                
                template< class All > void all( All all ) const
                {
                    nodes( [ & ]( Node* node ){ all( node->hash(), node->data() ); } );
                }
                
                void operator = ( const Map& map ) 
                {
                    map.all( [ & ]( ul hash, const Data& data ) { ( *this )[ hash ] = data; } );
                }
                
                Data& operator []( ul hash ) 
                {
                    auto& node = find( hash );
                    
                    if ( node )
                    {
                        return node->data();
                    } 
                    else
                    {
                        node = box::type< Node, Allocator >( hash );
                        
                        auto& sizeable = node->sizeable();
                        
                        sizeable = m_sizeable ? m_sizeable : this;
                        sizeable->inc(  1  );
                        
                        return node->data();
                    }
                }
                
                template< class Nodes > void nodes( Nodes nodes ) const
                {
                    for ( auto i = 0; i < Size; i++ )
                    {
                        auto node = m_nodes[ i ];
                        if ( !node )
                        {
                            continue;
                        }
                        
                        if ( node->map() )
                        {
                            node->map()->nodes( nodes );
                        }
                        
                        nodes( node );
                    }    
                }
                
                bool remove( ul hash )
                {
                    auto& node = find( hash );
                    if ( node )
                    {
                        node->sizeable()->dec();
                        box::detype< Node, Allocator >( node );
                        node = NULL;
                        
                        return true;
                    }
                    
                    return false;
                }
                
            protected:
                Node*& find( ul& hash )
                {
                    auto rem = hash % Size;
                    auto& node = m_nodes[ rem ];
                                        
                    if ( node )
                    {
                       if ( node->hash() == hash )
                       {
                           return node;
                       }  
                       
                       hash = ( hash  - rem ) / Size;
                       return ( *node )[ hash ];  
                   }
                   //
                   //   not found
                   //
                   return node;
                }
                
                Sizeable*& sizeable()
                {
                    return m_sizeable;
                }
                
                
            private:
                Node* m_nodes[ Size ];    
                Sizeable* m_sizeable;
            };
        }
    }
}

#endif 