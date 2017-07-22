#ifndef _TAU_LI_H_
#define _TAU_LI_H_

#include "mem.h"
#include "../../src/trace.h"

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
                if ( m_data != m_default && m_data )
                {
                    mem::mem().free( m_data );    
                    m_data = NULL;
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
            typedef typename box::list::List< Value, Allocator >::Node Node;
            
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
            
            Value& last()
            {
                auto node = this->tail( );
                if ( node )
                {
                    return node->data();
                }
                
                throw Error();
            }
            
            Value& first()
            {
                auto node = this->head( );
                if ( node )
                {
                    return node->data();
                }
                
                throw Error();
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
            
            Map( const std::initializer_list< std::pair< const Key&, Value& > >& list )
                : box::map::Map< Data< Key, Value >, Size, Allocator > ()
            {
                for ( auto i = list.begin(); i != list.end(); i++ )
                {
                    ( *this )[ i->first ] = i->second;
                }
            }
            
            virtual ~Map()
            {
                
            }
            
            
            Value& operator[]( const Key& key )
            {
                auto hash = box::h::hash< Key >()( key );    
                auto h = hash;
                
                
                auto& node = box::map::Map< Data< Key, Value >, Size, Allocator >::operator[]( hash );
                
                node.key = key;
                return node.value;
            }
            
            Value& get( const Key& key ) 
            {
                ul hash = box::h::hash< Key >()( key );    
                auto node = this->find( hash );
                
                if ( node )
                {
                    return node->data().value;
                }
                 
                throw Error( "value not found" );    
            }
            
            const Value& get( const Key& key ) const
            {
                return ( const_cast< Map* >( this ) )->get( key );
            }
             
            bool contains( const Key& key ) const
            {
                auto hash = box::h::hash< Key >()( key );
                auto node = ( const_cast< Map* >( this ) )->find( hash );
                return  node ? true : false;    
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
            
            template < class Callback > void entries( Callback callback )
            {
                this->all( [ & ] ( ul hash, Data< Key, Value >& data ) 
                    {
                        callback( data.key, data.value );
                    });
            }
        };
        
        
        template < class Value > struct Entry
        {
            Value value;
            typename List< Entry < Value >* >::Node* node;
            
            Entry( const Entry& entry )
                : value( entry.value ), node( entry.node )
            {
                 
            }
            
            Entry()
            {
                
            }
        };
        

        template< class Value > class Set : public Map< Value, Entry< Value > >
        {
        public:
            Set()
                : Map< Value, Entry< Value > >()
            {
                
            }
            Set( const Set& set )
                : Map< Value, Entry< Value > >()
            {
                operator =( set );
            }
            
            Value& operator[]( const Value& key )
            {
                
                auto length = this->length();
                auto& entry = Map< Value, Entry< Value > >::operator[]( key );
                
                auto& value = entry.value;
                
                if ( this->length() != length )
                {
                    entry.node = m_list.append( &entry );
                }
                

                return value;
            }
            
            
            virtual ~Set()
            {
                
            }

            void set( const Value& value )
            {
                ( *this )[ value ] = value;
            }       
            
            Value def( const Value& key, const Value& def ) const
            {
                Entry< Value > entry;
                Value value;
        
                try
                {
                    entry = this->get( key );
                    value = entry.value;
                }
                catch ( ... )
                {
                    value = def;
                }
        
                return value;
            }
            
            void clear()
            {
                box::map::Sizeable::dec( m_list.length() );
                values( [ & ] ( Value& value ) 
                {
                    remove( value ); 
                } );
                
                m_list.clear();
                
                
            }
            
            Value& first()
            {
                if ( m_list.length() )
                {
                    return m_list.first()->value;
                }
                
                throw Error();
            }
            
            Value& last()
            {
                if ( m_list.length() )
                {
                    return m_list.last()->value;
                }
                
                throw Error();
            }
            
            bool remove( const Value& key )
            {
                auto length = this->length();
                try
                {
                    auto& entry = this->get( key );    
                    entry.node->remove();
                    Map< Value, Entry< Value > >::remove( key );
                    return true;
                }
                catch ( const Error& )
                {
                    return false;
                }                
            }
            
            template < class Callback > void values( Callback callback )
            {
                m_list.all( [ & ]( Entry< Value >* entry ) 
                    {
                        callback( entry->value );
                    });
            }
            
        private:
            List< Entry< Value >* > m_list;
            
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