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
            
            void clear()
            {
                ENTER();
                
                box::list::List< Value >::clear();
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
            
            bool remove( const Key& key )
            {
                ENTER();
                
                return box::map::Map< Data< Key, Value >, Size >::remove( box::h::hash< Key >()( key ) );
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