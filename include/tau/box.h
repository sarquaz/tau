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
        
        template < class Allocator, class Type > inline void detype( Type* type )
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
        
        
        template < class Data, class Allocator > class List;
        
        namespace node
        {
            template < class Data, class Allocator = box::Allocator > class Node
            {                
                
                public:
            
                Node( const Data& data, List< Data, Allocator >* list = NULL )
                    : m_data( data ), m_next( 0 ), m_prev( 0 ), m_list( list ), m_custom( NULL )
                {
                }
                
                virtual ~Node()
                {
                    
                }
                
            
                void before( Node* node, const Data& data )
                {
                    ENTER();
                    m_prev = node;
                    node->m_next = this;
                    
                    update( node, m_list->m_head );
                }
    
                void after( Node* node, const Data& data )
                {
                    ENTER();
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
                
                Node* next() const
                {
                    return m_next;
                }
                
                Node* prev() const
                {
                    return m_prev;
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
                    m_list->update( List< Data, Allocator >::Listener::Removed );
                    
                    //
                    //  free memory
                    //  
                    box::deallocate< Allocator >( this );
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
                void update( Node* node, Node*& list )
                {
                    ENTER();
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
                Node* m_next;
                Node* m_prev;
                List< Data, Allocator >* m_list;
                void* m_custom;
            };
        }

        
        template < class Data, class Allocator = box::Allocator > class List
        {
            template< class _Data, class _Allocator > friend class node::Node;
            
        public:
            enum Direction 
            {
                Tail,
                Head
            };
            
            typedef node::Node<  Data, Allocator > Node;
                
                struct Listener
                {
                    enum What
                    {
                        Added,
                        Removed
                    };
                    
                    virtual void on( What what ) = 0;
                    
                    virtual ~Listener()
                    {
                    }
                };    
                
                List( Listener* listener = NULL )
                    : m_listener( listener ), m_tail( NULL ), m_head( NULL ), m_length( 0 )
                {
                }
                
                List( const List& list )
                    : m_head( list.m_head ), m_tail( list.m_tail ), m_listener( list.m_listener ), m_length( list.m_length )
                {
                }
                
                virtual ~List()
                {
                    ENTER();
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
                
                template < class All > void all( All all, bool head = true ) const
                {
                    ( const_cast< List* >( this ) )->nodes( [ & ]( Node* node ){ all( node->data() ); }, head );
                }
                
                template < class Nodes > void nodes( Nodes nodes, Direction direction = Head ) 
                {
                    ENTER();
                    
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
                    ENTER();
                    
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
                
            private:
                Listener* listener() const
                {
                    return m_listener;
                }
                
                Node* add( const Data& data, Direction direction )
                {
                    ENTER();
                    
                    auto node = this->node( data );
                    
                    if ( !m_head || !m_tail )
                    {
                        this->init( node );
                    }
                    else
                    {
                        direction == Head ? m_head->before( node, data ) : m_tail->after( node, data );
                    }
                    
                    update( Listener::Added );

                    return node;
                }
                
                void init( Node* node )
                {
                    ENTER();
                    m_head = node;
                    m_tail = node;
                }
                
                Node* node( const Data& data )
                {
                    return box::type< Node, Allocator >( data , this );
                }
                
                void update( typename Listener::What what )
                {
                    switch ( what )
                    {
                        case Listener::Added:
                            m_length ++;
                            break;
                        
                        case Listener::Removed:
                            m_length --;
                            break;    
                    }
                    
                    if ( m_listener )
                    {
                        m_listener->on( what );    
                    }
                    
                }
                
            private:
                Node* m_head;
                Node* m_tail;
                Listener* m_listener;
                ui m_length;
        };
        
        template < class Allocator = box::Allocator > struct Type
        {
            ul hash;
            void* data;
    
            Type( ul _hash )
                : hash( _hash ), data( NULL )
            {
            }
            
            
            Type( const Type& _type )
                : hash( _type.hash ), data ( _type.data )
            {
            }
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
    
        template < ui Size = box::Size, class Allocator = box::Allocator > class Map : public box::List<  Type< Allocator >, Allocator >::Listener
        {
            typedef box::Type< Allocator > Type;
            typedef node::Node< Type, Allocator > Node;
            typedef box::List<  Type, Allocator > List;
            
        public:
            Map()
                : m_length( 0 )
            { 
                std::memset( m_nodes, 0, sizeof( m_nodes ) );                
            }
            virtual ~Map ()
            {
                //
                //  remove all nodes
                //
                nodes( []( Node* node )
                    {
                        List* list = NULL;
                        if ( node->custom() )
                        {
                            list = static_cast< List* >( node->custom() );
                            
                        }
                        node->remove(); 
                        if ( list )
                        {
                            box::detype< Allocator, List >( list );
                        }
                    },
                    List::Tail 
                    );
            }
            
            template < class All > void all( All all ) const
            {
                ( const_cast< Map* >( this ) )->nodes( [ & ]( Node* node ){ all( node->data().hash, node->data().data ); } );
            }
            
            void*& operator []( ul hash ) 
            {
                ENTER();
                TRACE( "getting node hash %d\n", hash );
        
                //
                //  find nodes by hash
                //
                auto find = this->find( hash );
                TRACE( "find.start 0x%x find.found 0x%x find.index %d", find.start, find.found, find.index );
                
                //  
                //  if node was found
                //  
                if ( find.found )
                {
                    return find.found->data().data;
                }
                else
                {                    
                    if ( !find.start )
                    {
                        //
                        //  if need to create a new node
                        //
                        auto start = this->start( hash );
                        
                        TRACE( "created new node 0x%x at index %d", start, find.index );
                        
                        m_nodes[ find.index ] = start;
                        return start->data().data;
                    }
                    
                    //
                    //  create new  node and add it to one of list ends
                    //
                    auto start = find.start;
                    
                    
                    assert( start );
                    auto list = start->list();
                    Type type( hash );
                    auto newnode = hash > list->tail()->data().hash ? list->append( type ) : list->prepend( type );
                    return newnode->data().data;
                }
            }
            
            void* get( ul hash )
            {
                Find find = this->find( hash );
                if ( find.found )
                {
                    return find.found->data().data;
                }    
                
                throw Error( "node with hash %d not found", hash );
            }
            
            void remove( ul hash )
            {
                ENTER();
                
                auto find = this->find( hash );
                
                if ( !find.found )
                {
                    throw Error( "node with hash %d not found", hash );
                }
                
                find.found->remove();
            }
                
            
            ui length() const
            {
                return m_length;
            }

        private:
            struct Find
            {
                Node* start;
                Node* found;
                ui index;
                
                Find( Node* _start = NULL, Node* _found = NULL )
                    : start( _start ), found( _found ), index( 0 )
                {
                }
                
                Find( const Find& find )
                {
                    ENTER();
                }
            };
            
            Node* start( ul hash )
            {
                Node* node;
                
                //
                //  allocate new list
                //
                auto list = box::type< List, Allocator >( this );
                TRACE( "list 0x%x head 0x%x tail 0x%x", list, list->tail(), list->head() );
                //
                //  create new node
                //
                node = list->append( Type( hash ) );
                node->custom( list );
                return node;
            }
            
            Find find( ul hash ) const
            {
                ENTER();
                List* list;
                Find find;
                
                auto index = hash % Size;
                auto start = m_nodes[ index ];
                
                TRACE( "start 0x%x index %d", start, index );
                
                if ( !start )
                {
                    find.index = index;
                    return find;
                }
                
                
                list = start->list();
                find.start = start;
                
                typename List::Direction direction = List::Head;
                
                if ( hash != start->data().hash )
                {
                    //
                    //  see what is closer: tail or head
                    //
                    if ( hash <= list->tail()->data().hash && hash >= list->head()->data().hash )
                    {
                        auto diff = list->tail()->data().hash;
                        if ( diff < hash - list->head()->data().hash )
                        {
                            direction = List::Tail;
                        }
                    }
                    find.found = list->first( [ & ]( Type& type ){ return type.hash == hash; }, direction );
                }
                else
                {
                    find.found = find.start;
                }
                
                
                return find;
            }
            
            Type type( ul hash ) 
            {
                Type type( hash );        
                return type;
            }

            
            template < class Nodes > void nodes( Nodes nodes, typename List::Direction direction = List::Head ) 
            {
                ENTER();
                for( auto i = 0; i < Size; ++i )
                {
                    auto node = m_nodes[ i ];
                    
                    if ( !node )
                    {
                        continue;
                    }
                    
                    assert( node->list() );
                    
                    node->list()->nodes( [ & ]( Node* node ){ nodes( node ); }, direction );
                }
            }
            
            //
            //  List::Listener overload
            //
            void on( typename List::Listener::What what ) 
            {
                switch ( what )
                {
                    case List::Listener::Added:
                        m_length ++;
                        break;
                        
                    case List::Listener::Removed:
                        m_length --;
                        break;    
                }
            }
            
              
        private:
            Node* m_nodes[ Size ];    
            ui m_length;
        };
    }
}

#endif 