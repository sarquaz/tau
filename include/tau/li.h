#ifndef _TAU_LI_H_
#define _TAU_LI_H_

#include "../../src/trace.h"

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
        
        // template < class Value > struct _Node
        // {
        //     Value v;
        //     _Node* next;
        //     _Node* prev;
        //
        //     _Node( const Value& value )
        //         : next( NULL ), prev( NULL ), v( value )
        //     {
        //     }
        //
        //     _Node( )
        //         : next( NULL ), prev( NULL )
        //     {
        //     }
        //
        //     void remove( )
        //     {
        //         if ( prev )
        //         {
        //             prev->next = next;
        //         }
        //
        //         if ( next )
        //         {
        //             next->prev = prev;
        //         }
        //     }
        //
        //     void after( _Node& node )
        //     {
        //         this->next = &node;
        //         node.prev = this;
        //
        //     }
        // };
        //
        // template < class Type > class Pile
        // {
        //     struct Chunk
        //     {
        //         ui got;
        //         ui count;
        //         ui freed;
        //         void* start;
        //
        //         Chunk( ui _count = 0 )
        //             : count( _count ), got( 0 ), freed( 0 )
        //         {
        //         }
        //     };
        //
        //     typedef _Node< Chunk > Node;
        //
        //     template < class _Type > struct _Item
        //     {
        //         Node* node;
        //         _Type type;
        //     };
        //
        //     typedef _Item< Type > Item;
        //
        //     enum
        //     {
        //         Count = 10
        //     };
        //
        // public:
        //     ui count;
        //
        //     Pile( ui _count = Count )
        //         : count( _count ), m_list( NULL ), m_size( 0 )
        //     {
        //     }
        //
        //     ~Pile()
        //     {
        //         clear();
        //     }
        //
        //     template < class ... Args > Type& get( Args&&... args )
        //     {
        //         //
        //         //  check if the space was allocated
        //         //
        //         if ( m_list )
        //         {
        //             auto& count = m_list->v.got;
        //             //
        //             //  if the new item fits in the allocated storage
        //             //
        //             if ( count < m_list->v.count )
        //             {
        //
        //                 //
        //                 //  get the pointer
        //                 //
        //                 auto item = ( Item* ) ( ( ul ) m_list->v.start + count * sizeof( Item ) );
        //                 auto type = &item->type;
        //                 item->node = m_list;
        //                 //
        //                 //  increase count
        //                 //
        //                 count ++;
        //                 //
        //                 //  increase size
        //                 //
        //                 set( [ & ] ( ) { m_size ++; } );
        //                 //
        //                 //  call item constructor
        //                 //
        //                 new ( type ) Type( std::forward< Args >( args ) ... );
        //                 return *type;
        //             }
        //         }
        //
        //         //
        //         //  allocate space
        //         //
        //         auto& node = this->node( );
        //         if ( m_list )
        //         {
        //             m_list->after( node );
        //         }
        //         m_list = &node;
        //         //
        //         //  call itself again (with enough space)
        //         //
        //         return get( std::forward< Args >( args ) ... );
        //     }
        //
        //     void free( Type& type )
        //     {
        //         //
        //         //  call desctuctor
        //         //
        //         type.~Type();
        //
        //         //
        //         //  get pointer to item
        //         //
        //         auto item = ( Item* ) ( ( ul ) &type - sizeof( Node* ) );
        //         auto node = item->node;
        //         auto& freed = node->v.freed;
        //         freed ++;
        //         //
        //         //  decrease size
        //         //
        //         set( [ & ] ( ) { m_size --; } );
        //
        //         //
        //         //  if all items contained in this node were freed
        //         //
        //         if ( freed == node->v.count )
        //         {
        //             if ( m_list == node )
        //             {
        //                 m_list = dynamic_cast< Node* > ( node->prev );
        //             }
        //
        //             free( *node );
        //         }
        //     }
        //
        //     void free( Node& node )
        //     {
        //         //
        //         //  remove node
        //         //
        //         node.remove();
        //         //
        //         //  free allocated space
        //         //
        //         m_items.free( ( Item* ) ( node.v.start ), node.v.count );
        //         m_nodes.free( node );
        //     }
        //
        //     void clear()
        //     {
        //         auto& node = m_list;
        //         while ( node )
        //         {
        //             auto prev = node->prev;
        //             free( *node );
        //             node = prev;
        //         }
        //
        //         m_size = 0;
        //         count = Count;
        //     }
        //
        //     ui size() const
        //     {
        //         return m_size;
        //     }
        //
        // private:
        //     template < class Op > void set( Op op )
        //     {
        //         auto size = m_size;
        //         op( );
        //
        //         if ( !( m_size % Count ) )
        //         {
        //             auto& count = this->count;
        //             auto change = m_size / count;
        //             count = ( m_size > size ? +change : -change ) + count;
        //         }
        //     }
        //
        //     Node& node()
        //     {
        //         auto& node = m_nodes.get();
        //
        //         node.v.start = m_items.get( count );
        //         node.v.count = count;
        //
        //         return node;
        //     }
        //
        // private:
        //     si::mem::Types< Node > m_nodes;
        //     si::mem::Bytes< Item > m_items;
        //     Node* m_list;
        //     ui m_size;
        // };

        /**
         * List class (std::list replacement)
        **/
        template < class Value > class List: public box::list::List< Value >
        {
        public:
            List( )
                : box::list::List< Value >()
            {
            }

            List( std::initializer_list< Value > list )
                : box::list::List< Value >()
            {
                for ( auto i = list.begin( ); i != list.end( ); i++ )
                {
                    append( *i );
                }
            }

            List( const List& list )
                : box::list::List< Value >( list )
            {
            }

            virtual ~List( )
            {
            }

            Value pop()
            {
                auto node = this->tail( );
                if ( node )
                {
                    auto data = node->data();
                    node->remove();
                    return data;    
                }
                
                throw Error();
            }

            template < class Logic > Mass< Data > filter( Logic logic ) const
            {
                Mass< Data > mass;
                all( [ & ]( const Value& value )
                {
                    if ( logic( value ) )
                    {
                        mass.add( value );
                    }
                } );

                return mass;
            }

            bool empty( ) const
            {
                return !( this->length() );
            }
        };
        
        
        template < class Key, class Value > struct Data
        {
            Key key;
            Value value;
            
            Data(  )
            {
            }
            
            Data( const Data& data )
                : key( data.key ), value( data.value )
            {
            }
        };
        
        /**retu
         * Map class (std::map replacement)
        **/
        template < class Key, class Value, ui Size = 64 > class Map: public box::map::Map< Data< Key, Value >, Size > 
        {
        public:
            Map()
                : box::map::Map< Data< Key, Value >, Size > ()
            {
            }
            virtual ~Map()
            {
                
            }
            
            
            Value& operator[]( const Key& key )
            {
                ENTER();
                
                ul hash = box::h::hash< Key >()( key );    
                auto& data = box::map::Map< Data< Key, Value >, Size >::operator[]( hash );
                data.key = key;
                return data.value;
            }
            
            const Value& get( const Key& key ) const
            {
                ul hash = box::h::hash< Key >()( key );    
                auto node = ( const_cast< Map* >( this ) )->find( hash );
                
                if ( node )
                {
                    return node->data().value;
                }
                 
                throw Error( "value not found" );    
            }
        };

        template< class Value > class Set : public Map< Value, Value >
        {
        public:
            Set()
                : Map< Value, Value >()
            {
                
            }
            Set( const Set& set )
                : Map< Value, Value >()
            {
                operator =( set );
            }
            
            
            virtual ~Set()
            {
                
            }

            void set( const Value& value )
            {
                ( *this )[ value ] = value;
            }       
        };

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