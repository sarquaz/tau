#ifndef TAU_LI_H
#define TAU_LI_H

#include "../../src/trace.h"
#include "mem.h"

namespace tau
{
    namespace li
     {

        template < class Data, unsigned int Size = 16 > class Mass
        {
        public:
            Mass( )
            : m_data( m_default ), m_size( Size ), m_length( 0 )
            {

            }

            unsigned int length( ) const
            {
                return m_length;
            }

            void add( const Data& value )
            {
                check( );
                m_data[ m_length ] = value;
                m_length ++;
            }

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

            void clear( )
            {
                m_length = 0;
            }
 
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
                auto size = m_size * 2;

                if ( m_length == m_size )
                {
                    if ( m_data == m_default )
                    {
                        m_data = ( Data* ) std::malloc( size * sizeof ( Data ) );
                        std::memcpy( m_data, m_default, sizeof ( m_default ) );
                    }
                    else
                    {
                        m_data = ( Data* ) std::realloc( m_data, size );
                    }

                    m_size = size;
                }
            }

            unsigned int size( ) const
            {
                return m_size;
            }

        private:
            Data m_default[ Size ];
            Data* m_data;
            unsigned int m_size;
            unsigned int m_length;
        };

        template < class Data > class List
        {
        public:
            typedef si::mem::list::Node< Data > Item;

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

            Item& add( const Data& value )
            {
                return add( m_items.get( value ) );
            }

            // Item& add( )
            // {
            //     return add( m_items.get( ) );
            // }

            void remove( Item& item )
            {                
                if ( m_list == &item )
                {
                    m_list = item.next;
                }

                item.remove();

                m_items.free( item );
            }
            
            Item& head( )
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

            const Item& head( ) const
            {
                return const_cast< List* >( this )->head();
            }

            Data get( )
            {
                auto& item = head( );
                auto data = item.v;

                //remove( item );
                return data;
            }

            template< class Logic > List< Data > filter( Logic logic ) const
            {
                List< Data > list;
                all( [ & ]( const Data & data )
                {
                    if ( logic( data ) )
                    {
                        list.add( data );
                    }
                } );

                return list;
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

            unsigned int size( ) const
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
            si::mem::Pile< Item > m_items;
        };

        template < class Key, class Value, unsigned int Size = 64 > class Map
        {
            template < unsigned int S > struct _Hash
            {
                unsigned long value;
                unsigned long mod;
                unsigned int id;
                
                _Hash( unsigned long _value = 0, unsigned int _id = 0 )
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
                
                operator unsigned long() const
                {
                    return value;
                }
                
                bool operator ==( const _Hash& hash ) const
                {
                    return value == hash.value && id == hash.id;
                }
            };
            
            template < class K, class V, unsigned int S > struct _Item
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
            
            template < class K, class V, unsigned int S > struct _Node 
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
                
                _Node( unsigned long hash, const K& key, _Node*& _ref )
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
                Hash hash( std::hash< Key >( )( key ), m_id );

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
                Hash hash( std::hash< Key >( )( key ), m_id );
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
                Hash hash( std::hash< Key >( )( key ), m_id );

                auto item = m_node->get( hash );

                if ( item )
                {
                    remove( *item );
                    return true;
                }

                return false;
            }

            unsigned int size( ) const
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
                
                if ( ! ( unsigned long ) item.hash )
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
            si::mem::Pile< Node > m_nodes;
            si::mem::Bytes< Node* > m_pools;
            Items m_items;
            Node* m_last;
            unsigned int m_id;
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