#include "tau.h"

namespace tau
{
    namespace data
    {
        int Data::find( const Data& data, ui offset ) const
        {
            auto found = ( char* ) ::memmem( *this + offset, length(), data, data.length() );
            
            if ( found )
            {
                auto pos = found - *this;
                return ( pos < length() ) ? pos : -1;
            }
            
            return -1;
        }
        
        void Data::setup( const char* data, ui length )
        {
            m_external = data;
            
            if ( !length && data )
            {
                length = std::strlen( data );
            };
            
            this->length( length );
        }
        
        Data& Data::add( char data )
        {
            if ( !data )
            {
                return *this;
            }
            
           if ( !m_external )
           {
               space( 1 );
                *( target() ) = data;
                inc( );
            }
            
            return *this;
        }
        
        Data& Data::add( const char* data, ui length )
        {
            if ( !data )
            {
                return *this;
            }
            
            if ( m_external )
            {
                 assert( false );
            }
            else
            {
                if ( !length )
                {
                    length = std::strlen( data );
                }
                
                
                
                space( offset() + length + 1 );
               
                std::memcpy( target( ), data, length );
                *( target() + length ) = 0;
            }
            
            inc( length );
            return *this;
        }
        
        void Data::space( ui length )
        {
            if ( m_external )
            {
                return;
            }
            
            if ( available() > length )
            {
                return;
            }
                        
            m_data.get( length );
        }
        
                        
        char& Data::operator[]( ui offset )
        {
            if ( !data( ) || offset > ( ui ) m_data )
            {
                throw Error();
            }

            return *( data( ) + offset );
        }

        char Data::operator[]( ui offset ) const
        {
            if ( !data( ) || offset > ( ui ) m_data )
            {
                return 0;
            }

            return *( data( ) + offset );
        }

        
        Data Data::get( ui length )
        {
            Data data;
            
            auto start = 33;
            auto range = 126 - start;
            
            ui number = 0;
            auto pos = 0;
            
            while ( data.length() < length )
            {
                if ( !number )
                {
                    number = r::random( UINT_MAX );
                }
                
                unsigned char current = ( ( char* ) &number )[ pos ];
                                
                data.add( start + ( current % range ) );
                pos ++;
                
                if ( pos == sizeof( number ) )
                {
                    pos = 0;
                    number = 0;
                }
            }
                            
            return data;
        }
        
        ul Data::hash( ) const
        {
            auto hash = Piece( ( char* ) ( const char *) m_data, length() )();

            return hash;
        }
        
        struct Chunk
        {
            enum
            {

                Size = 0x40
            };

            static ui bytes( ui size )
            {
                return chunks( size ) * Size;
            }

            static ui chunks( ui length )
            {
                auto count = length / Size;

                if ( count * Size < length )
                {
                    count += 1;
                }

                return count;
            }
        };
        
        void Data::Piece::clear( )
        {
            if ( m_data )
            {
                mem::mem().free( m_data );
                m_size = 0;
                m_data = NULL;
            }
        }
         
        void Data::Piece::get( ui length )
        {
            Piece old = *this;
            
            auto chunks = Chunk::chunks( m_size + length );
                        
            m_size += chunks * Chunk::Size;
            m_data = ( char* ) mem::mem().get( m_size );
                        
            if ( old.m_data )
            {
                std::memcpy( m_data, old.m_data, old.m_size );
                old.clear( );
            }
        }
        
        ul Data::Piece::operator()( ) const
        {            
            return box::Hash( ( uchar* ) m_data, m_size )();
        }
    }
}