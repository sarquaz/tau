#ifndef _TAU_LI_H_
#define _TAU_LI_H_

#include "../../src/trace.h"
#include "mem.h"

namespace tau
{
    namespace li
     {
         /**
          * Array wrapper
         **/
        template < class Data, ui Size = 16 > class Array
        {
        public:
            Array( )
                : m_data( m_default ), m_size( Size ), m_length( 0 )
            {
            }
            
            ~Array()
            {
                free();
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
            void free()
            {
                if ( m_data != m_default )
                {
                    mem::mem().free( m_data );    
                }
            }
            
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
                    auto data = mem::mem().get( size * sizeof( Data ) );
                    //
                    //  copy data
                    //
                    std::memcpy( data, m_data, m_size * sizeof( Data )  );
                
                    m_size = size;
                    //
                    //  free old space if needed
                    //
                    free();
                    m_data = ( Data* ) data;
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
        };
        
        struct Allocator
        {
            static void* allocate( ui size )
            {
                return mem::mem().get( size );
            }
    
            static void deallocate( void* data )
            {
                mem::mem().free( data );
            }
        };

        /**
         * List class (std::list replacement)
        **/
        template < class Value > class List: public box::list::List< Value, Allocator >
        {
        public:
            enum Direction
            {
                Head = box::list::Head,
                Tail = box::list::Tail
            };
            
            List( )
                : box::list::List< Value, Allocator >()
            {
            }

            List( std::initializer_list< Value > list )
                : box::list::List< Value, Allocator >()
            {
                for ( auto i = list.begin( ); i != list.end( ); i++ )
                {
                    append( *i );
                }
            }

            List( const List& list )
                : box::list::List< Value, Allocator >( list )
            {
            }

            virtual ~List( )
            {
            }

            Value pop( Direction direction = Tail)
            {
                auto node = direction == Tail ? this->tail( ) : this->head();
                if ( node )
                {
                    auto data = node->data();
                    node->remove();
                    return data;    
                }
                
                throw Error();
            }

            template < class Logic > List< Data > filter( Logic logic ) const
            {
                List< Data > list;
                all( [ & ]( const Value& value )
                {
                    if ( logic( value ) )
                    {
                        list.append( value );
                    }
                } );

                return list;
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
        
        /**
         * Map class (std::map replacement)
        **/
        template < class Key, class Value, ui Size = 64 > class Map: public box::map::Map< Data< Key, Value >, Size, Allocator > 
        {
        public:
            Map()
                : box::map::Map< Data< Key, Value >, Size, Allocator > ()
            {
            }
            virtual ~Map()
            {
                
            }
            
            
            Value& operator[]( const Key& key )
            {
                ENTER();
                
                ul hash = box::h::hash< Key >()( key );    
                
                TRACE( "hash %u", hash );
                auto& data = box::map::Map< Data< Key, Value >, Size, Allocator >::operator[]( hash );
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
             
            bool exists( const Key& key ) const
            {
                auto hash = box::h::hash< Key >()( key );
                return ( const_cast< Map* >( this ) )->find( hash ) ? true : false;    
            }    
            
            bool remove( const Key& key )
            {
                auto hash = box::h::hash< Key >()( key );
                return box::map::Map< Data< Key, Value >, Size, Allocator >::remove( hash );
            }
            
            bool empty() const
            {
                return this->length() == 0;
            }
            
            template < class Callback > void keys( Callback callback )
            {
                this->all( [ & ] ( ul hash, Data< Key, Value >& data ) 
                    {
                        callback( data.key );
                    });
            }
            
            template < class Callback > void values( Callback callback )
            {
                this->all( [ & ] ( ul hash, Data< Key, Value >& data ) 
                    {
                        callback( data.value );
                    });
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
            
            Set( const std::initializer_list< std::pair< const Value, Value > >& list )
                : Map< Value, Value >()
            {
                for ( auto i = list.begin(); i != list.end(); i++ )
                {
                    ( *this )[ i->first ] = i->second;
                }
            }
            
            
            virtual ~Set()
            {
                
            }

            void set( const Value& value )
            {
                ( *this )[ value ] = value;
            }       
            
            Value def( const Value& key, const Value& value ) const
            {
                Value ret;
        
                try
                {
                    ret = this->get( key );
                }
                catch ( ... )
                {
                    ret = value;
                }
        
                return ret;
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