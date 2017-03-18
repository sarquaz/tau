#ifndef _TAU_LI_H
#define _TAU_LI_H

#include "mem.h"

namespace tau
{
    namespace li
     {
         /**
          * Array wrapper
         **/
        template < class Data, ui Size = 16 > class Mass
        {
        public:
            Mass( )
                : m_data( m_default ), m_size( Size ), m_length( 0 )
            {
            }
            
            /**
             * Number of items added
            **/
            ui length( ) const  
            {
                return m_length;
            }

            /**
             * Add item
            **/
            void add( const Data& value )
            {
                check( );
                m_data[ m_length ] = value;
                m_length ++;
            }
            
            /**
             * Get last item
            **/
            Data& get( )
            {
                if ( m_length )
                {
                    return m_data[ m_length - 1 ];
                }
                else
                {
                    throw Error( );
                }
            }

            /**
             * Clear all items
            **/
            void clear( )
            {
                m_length = 0;
            }
 
            /**
             * Iterate through all contained items
            **/
            template < class All > void all( All all ) const
            {
                for ( auto i = 0; i < m_length; i++ )
                {
                    all( m_data[ i ] );
                }
            }

        private:
            void check( )
            {                
                //
                //  check if there is enough space
                //
                if ( m_length == m_size )
                {
                    //
                    //  double the size
                    //
                    auto size = m_size * 2;
                    //
                    //  allocate space
                    //
                    auto data = m_bytes.get( size );
                    //
                    //  copy data
                    //
                    std::memcpy( data, m_data, m_size * sizeof( Data )  );
                    //
                    //  free old space if needed
                    //
                    if ( m_data != m_default )
                    {
                        m_bytes.free( m_data, m_size );    
                    }
                    m_size = size;
                    m_data = data;
                }
            }

            ui size( ) const
            {
                return m_size;
            }

        private:
            Data m_default[ Size ];
            Data* m_data;
            ui m_size;
            ui m_length;
            si::mem::Bytes< Data > m_bytes;
        };
        
        template < class Value > struct _Node
        {
            Value v;
            _Node* next;
            _Node* prev;

            _Node( const Value& value )
                : next( NULL ), prev( NULL ), v( value )
            {
            }

            _Node( )
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

            void after( _Node& node )
            {
                this->next = &node;
                node.prev = this;
            
            }
        };

        template < class Type > class Pile
        {               
            struct Chunk
            {
                ui got;
                ui count;
                ui freed;
                void* start;
        
                Chunk( ui _count = 0 )
                    : count( _count ), got( 0 ), freed( 0 ) 
                {
                }
            };
     
            typedef _Node< Chunk > Node;
    
            template < class _Type > struct _Item
            {
                Node* node;
                _Type type;
            };
    
            typedef _Item< Type > Item;
    
            enum 
            {
                Count = 10
            };
    
        public:
            ui count;
    
            Pile( ui _count = Count )
                : count( _count ), m_list( NULL ), m_size( 0 )
            {
            }
    
            ~Pile()
            {
                clear();
            }
    
            template < class ... Args > Type& get( Args&&... args )
            {
                //
                //  check if the space was allocated
                //
                if ( m_list )
                {
                    auto& count = m_list->v.got;
                    //
                    //  if the new item fits in the allocated storage
                    //                
                    if ( count < m_list->v.count )
                    {
                        //
                        //  get the pointer
                        //
                        auto item = ( Item* ) ( ( ul ) m_list->v.start + count * sizeof( Item ) );                            
                        auto type = &item->type;
                        item->node = m_list;
                        //
                        //  increase count
                        //
                        count ++;
                        //
                        //  increase size
                        //
                        set( [ & ] ( ) { m_size ++; } );
                        //
                        //  call item constructor
                        //
                        new ( type ) Type( std::forward< Args >( args ) ... );
                        return *type;
                    }
                }
                //
                //  allocate space
                //
                auto& node = this->node( );
                if ( m_list )
                {
                    m_list->after( node );
                }
                m_list = &node;
                //
                //  call itself again (with enough space)
                //
                return get( std::forward< Args >( args ) ... );
            }
    
            void free( Type& type )
            {
                //
                //  call desctuctor
                //
                type.~Type();
        
                //
                //  get pointer to item
                //
                auto item = ( Item* ) ( ( ul ) &type - sizeof( Node* ) );
                auto node = item->node;
                auto& freed = node->v.freed;
                freed ++;
                //
                //  decrease size
                //
                set( [ & ] ( ) { m_size --; } );
        
                //
                //  if all items contained in this node were freed 
                //
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
                //
                //  remove node
                //  
                node.remove();
                //
                //  free allocated space
                //
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
    
            ui size() const
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
            si::mem::Types< Node > m_nodes;
            si::mem::Bytes< Item > m_items;
            Node* m_list;
            ui m_size;
        };

        /**
         * List class (std::list replacement)
        **/
        template < class Data > class List
        {
        public:
            typedef _Node< Data > Item;

            List( )
                : m_list( NULL )
            {
            }

            List( std::initializer_list< Data > list )
                : m_list( NULL )
            {
                for ( auto i = list.begin( ); i != list.end( ); i++ )
                {
                    add( *i );
                }
            }

            List( const List& list )
                : m_list( NULL )
            {
                list.all( [ & ] ( const Data & value ) { add( value ); } );
            }

            ~List( )
            {
                clear( );
            }

            /**
             * Add item to the list
            **/
            Item& add( const Data& value )
            {
                return add( m_items.get( value ) );
            }

            /**
             * Remove item from list
            **/
            void remove( Item& item )
            {                
                if ( m_list == &item )
                {
                    m_list = item.next;
                }

                item.remove();
                m_items.free( item );
            }
            
            /**
             * Get last item
            **/
            Item& tail( )
            {
                if ( !m_list )
                {
                    throw Error( );
                }
                else
                {
                    return *m_list;
                }
            }

            const Item& tail( ) const
            {
                return const_cast< List* >( this )->tail();
            }

            Data pop()
            {
                auto& item = tail( );
                auto data = item.v;

                remove( item );
                return data;
            }

            template < class Logic > Mass< Data > filter( Logic logic ) const
            {
                Mass< Data > mass;
                all( [ & ]( const Data& data )
                {
                    if ( logic( data ) )
                    {
                        mass.add( data );
                    }
                } );

                return mass;
            }

            void clear( )
            {
                m_items.clear();
                m_list = NULL;
            }

            template< class All > void all( All all ) const
            {
                items( [ & ] ( const Item & node ) { all( node.v ); } );
            }

            template< class All > void all( All all )
            {
                items( [ & ] ( Item & node ) { all( node.v ); } );
            }

            template < class Items > void items( Items items ) const
            {
                return const_cast< List* >( this )->items( items );
            }

            template < class Items > void items( Items items )
            {
                auto tail = m_list;
                while ( tail )
                {
                    auto prev = tail->prev;
                    items( *tail );
                    tail = prev;
                }
            }

            ui size( ) const
            {
                return m_items.size();
            }

            bool empty( ) const
            {
                return !size();
            }

        private:
            Item& add( Item& item )
            {
                if ( m_list )
                {
                    m_list->after( item );
                }
                
                m_list = &item;
                return *m_list;
            }

            
        private:
            Item* m_list;
            Pile< Item > m_items;
        };
        
        /**
         * Map class (std::map replacement)
        **/
        template < class Key, class Value, ui Size = 64 > class Map
        {
            template < ui S > struct _Hash
            {
                ul value;
                ul mod;
                ui id;
                
                _Hash( ul _value = 0, ui _id = 0 )
                    : value( _value ), mod( _value ), id( _id )
                {
                }
                
                _Hash( const _Hash& hash )
                    : value( hash.value ), mod( hash.mod ), id( hash.id )
                {
                }
                                
                void operator =( const _Hash& hash )
                {
                    value = hash.value;
                    mod = hash.mod;
                    id = hash.id;
                }
                
                void operator()()
                {
                    mod = mod > S ? mod / S : mod;
                }
                
                operator ul() const
                {
                    return value;
                }
                
                bool operator ==( const _Hash& hash ) const
                {
                    return value == hash.value && id == hash.id;
                }
            };
            
            template < class K, class V, ui S > struct _Item
            {
                typedef _Hash< S > Hash;
                
                K key;
                V value;
                Hash hash;
                
                typename List< _Item* >::Item* list;
                
                _Item( )
                : list( NULL )
                {
                }
            };
            
            template < class K, class V, ui S > struct _Node 
            {
                typedef _Item< K, V, S > Item;
                typedef typename Item::Hash Hash;
                
                Item item;
                _Node*& ref;
                _Node** nodes;
                _Node* prev; 

                _Node( _Node*& _ref )
                    : ref( _ref ), nodes( NULL ), prev( NULL )
                {
                }
                
                _Node( ul hash, const K& key, _Node*& _ref )
                    : item( key, hash ), ref( _ref ), nodes( NULL ), prev( NULL )
                {
                }

                ~_Node( )
                {
                    ref = NULL;
                }

                Item* get( Hash& hash, _Node** parent = NULL )
                {
                    if ( parent )
                    {
                        *parent = this;
                    }
                    
                    hash(); 
                    
                    if ( item.hash == hash )
                    {
                        return &item;
                    }
                    
                    if ( nodes )
                    {
                        auto node = ( *this )[ hash ];
                        if ( node )
                        {
                            auto& item = node->item;
                            
                            if ( item.hash == hash )
                            {
                                return &item;
                            }
                            else
                            {
                                return node->get( hash, parent );
                            }
                        }
                    }

                    return NULL;
                }
                
                _Node*& operator []( const Hash& hash )
                {
                    return nodes[ hash.mod % S ];
                }
            };
            
            typedef _Item< Key, Value, Size > Item; 
            typedef typename Item::Hash Hash; 
            typedef _Node< Key, Value, Size > Node;
            typedef List< Item* > Items;

        public:
            Map( )
                : m_null( NULL ), m_id( 0 ) 
            {
                m_node = &m_nodes.get( m_null );
                m_last = m_node;
            }
            
            virtual ~Map( )
            {
                clear( true );
            }

            Value& operator[ ]( const Key& key )
            {
                auto hash = this->hash( key );

                Node* parent = NULL;
                auto item = m_node->get( hash, &parent );

                if ( item )
                {
                    return item->value;
                }

                return set( hash, key, *parent ).item.value;
            }

            Value& get( const Key& key )
            {
                auto hash = this->hash( key );
                auto item = m_node->get( hash );

                if ( item )
                {
                    return item->value;
                }

                throw Error( );
            }
            
            const Value& get( const Key& key ) const
            {
                return const_cast < Map* > ( this )->get( key );
            }
            
            bool remove( const Key& key )
            {
                auto hash = this->hash( key );
                auto item = m_node->get( hash );

                if ( item )
                {
                    remove( *item );
                    return true;
                }

                return false;
            }

            ui size( ) const
            {
                return m_items.size( );
            }
            
            bool empty() const
            {
                return !size();
            }
            
            void clear( bool pools = false )
            {
                if ( pools )
                {
                    free();
                }
                else
                {
                    m_id ++;
                }
                
                m_items.clear();
            }

            template < class Values > void values( Values values ) const
            {
                m_items.all( [ & ] ( const Item* item ) { values( item->value ); } );
            }

            template < class Keys > void keys( Keys keys ) const
            {
                m_items.all( [ & ] ( const Item* item ) { keys( item->key ); } );
            }

            template < class All > void all( All all ) const
            {
                m_items.all( [ & ] ( const Item* item ) { all( item->key, item->value ); } );
            }

        private:
            Hash hash( const Key& key ) const
            {
                return Hash( si::h::hash< Key >( )( key ), m_id );
            }
            
            void item( Item& item, const Hash& hash, const Key& key )
            {
                item.hash = hash;
                item.key = key;
                item.list = &m_items.add( &item );
            }
            void free()
            {
                auto& last = m_last;
                while ( last )
                {
                    auto prev = last->prev;
                    if ( last->nodes )
                    {
                        m_pools.free( last->nodes, Size );
                    }
                    
                    m_nodes.free( *last );
                    last = prev;
                }
            }
            
            void remove( Item& item )
            {
                m_items.remove( *item.list );
                item.hash = 0;                
            }
            
            void nodes( Node& node )
            {
                bool reuse = false;
                node.nodes = m_pools.get( Size, &reuse );
                
                if ( !reuse )
                {
                    std::memset( node.nodes, 0, Size * sizeof ( Node* ) );
                }
            }
            
            Node& node( Node*& ref, const Hash& hash, const Key& key )
            {
                auto& node = m_nodes.get( ref );
                item( node.item, hash, key );
                node.prev = m_last;
                m_last = &node;
                return node;
            }

            Node& set( const Hash& hash, const Key& key, Node& parent )
            {   
                auto& item = parent.item;
                
                if ( ! ( ul ) item.hash )
                {
                    this->item( item, hash, key );
                    return parent;
                }
                
                if ( !parent.nodes )
                {
                    nodes( parent );
                }
                
                auto& node = parent[ hash ];

                node = &this->node( node, hash, key );
                
                return *node;
            }

        private:
            Node* m_node;
            Node* m_null;
            Pile< Node > m_nodes;
            si::mem::Bytes< Node* > m_pools;
            Items m_items;
            Node* m_last;
            ui m_id;
        };

        template< class Value > class Set : public Map< Value, Value >
        {
        public:
            virtual ~Set()
            {
                
            }

            void set( const Value& value )
            {
                ( *this )[ value ] = value;
            }            
        };

        typedef Set< Data > Strings;

        template< class Type > struct cycle
        {
            std::initializer_list< Type > list;

            cycle( const std::initializer_list< Type >& _list )
            : list( _list )
            {
            }

            template< class Cycle > void operator()( Cycle cycle )
            {
                for ( auto i = list.begin( ); i != list.end( ); i++ )
                {
                    cycle( *i );
                }
            }
        };
    }
}

#endif