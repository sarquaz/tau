#ifndef _TAU_BOX_H_
#define _TAU_BOX_H_

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
            template < class, class > class List;
                    
            template < class Data, class Allocator > class _Node
            {                
            public:
                _Node( const Data& data, List< Data, Allocator >* list = NULL )
                    : m_data( data ), m_next( 0 ), m_prev( 0 ), m_list( list ), m_custom( NULL )
                {
                }
            
                virtual ~_Node()
                {
                }
            
        
                void before( _Node* node, const Data& data )
                {
                    m_prev = node;
                    node->m_next = this;
                
                    update( node, m_list->m_head );
                }

                void after( _Node* node, const Data& data )
                {
                    m_next = node;
                    node->m_prev = this;
            
                    update( node, m_list->m_tail );
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
                    m_list->update( List< Data, Allocator >::Removed );
                
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
                void update( _Node* node, _Node*& list )
                {
                    assert( m_list != NULL );
                
                    node->m_list = m_list;
            
                    //
                    //  update list pointer if necessary
                    //  
                    if ( list == this )
                    {
                        list = node;
                    }
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
                enum Direction 
                {
                    Tail,
                    Head
                };
                
                enum Action
                {
                    Added,
                    Removed
                };
            
                typedef _Node<  Data, Allocator > Node;
                
            
                List( )
                    : m_tail( NULL ), m_head( NULL ), m_length( 0 )
                {
                }
            
                List( const List& list )
                    : m_head( list.m_head ), m_tail( list.m_tail ), m_length( list.m_length )
                {
                }
            
                virtual ~List()
                {
                    clear();
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
                    auto head = direction == Head;
                
                    auto node = head ? m_head : m_tail;
                    
                    for ( ;; )
                    {
                        if ( !node )
                        {
                            break;
                        }
                        //
                        //  get next node since node might be freed in provided function
                        //
                        auto next = head ? node->next() : node->prev();
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
                    nodes( [ & ]( Node* node ) { box::detype< Node, Allocator >( node ); }, Tail );
//                    m_head = NULL;
  //                  m_tail = NULL;
                }    
            
            private:
                Node* add( const Data& data, Direction direction = Head )
                {
                    auto node = this->node( data );
                
                    if ( !m_head || !m_tail )
                    {
                        this->init( node );
                    }
                    else
                    {
                        direction == Head ? m_head->before( node, data ) : m_tail->after( node, data );
                    }
                
                    update( Added );

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
            
                void update( Action action )
                {
                    switch ( action )
                    {
                        case Added:
                            m_length ++;
                            break;
                    
                        case Removed:
                            m_length --;
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
                
                void inc( ui size )
                {
                    m_size += size;
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