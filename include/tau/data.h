#ifndef TAU_DATA_H
#define TAU_DATA_H

#include "std.h"    

namespace tau
{
    namespace si
    {
        class Data
        {
        public:
            Data( )
            {
                setup( );
            }
            Data( const char* data, unsigned int length = 0 )
            {
                setup( data, length );
            }
            Data( const Data& data )
            {
                setup( );
                add( data );
            }
            ~Data( )
            {
                m_data.clear();
            }
        
            unsigned int offset( ) const
            {
                return m_window.offset;
            }

            unsigned int length( ) const
            {
                return m_window.length;
            }

            bool empty( ) const
            {
                return length( ) == 0;
            }
            void setOffset( unsigned int offset )
            {
                m_window.offset = offset;
            
                if ( !m_external && size( ) < offset )
                {
                    space( size( ) + offset );
                }
            }
            void move( unsigned int offset )
            {
                setOffset( this->offset( ) + offset );
            }
            void setLength( unsigned int length )
            {
                m_window.length = length;
            }
            void inc( unsigned int length = 1 )
            {
                setLength( this->length() + length );
            }
            unsigned int size( ) const
            {
                return ( unsigned int ) m_data;
            }
            unsigned int available( ) const
            {
                if ( !m_data )
                {
                    return 0;
                }

                return size() - length();
            }

            operator char*( )
            {
                return data( ) + offset();
            }
            operator const char*( ) const
            {
                return data( ) + offset();
            }
            char* c( )
            {
                return ( char* ) * this;
            }
            const char* c( ) const
            {
                return ( const char* ) * this;
            }
            
            void operator =( const Data& data )
            {
                clear();
                add( data );
            }
            bool operator ==( const Data& data ) const
            {
                return length() == data.length() && find( data ) == 0;
            }
            bool operator ==( const char* data ) const
            {
                return *this == Data( data );
            }
            char operator []( unsigned int offset ) const;
            char& operator []( unsigned int offset );

            Data& add( char data );
            Data& add( const char* data, unsigned int length = 0 );
            Data& add( const Data& data )
            {
                return add( data.c(), data.length() );
            }
            void clear( )
            {
                m_external = NULL;
                m_window.clear();
            }
            int find( const Data&, unsigned int offset = 0 ) const;
            
            unsigned long hash() const;

            void vprint( const char* format, va_list args )
            {
                char data[ 0x200 ];
                auto count = ::vsnprintf( data, sizeof( data ), format, args );
                
                add( data, count );
            }

#define PRINT( data, format )\
            va_list args;\
            va_start( args, format );\
            data.vprint ( format, args );\
            va_end( args );        
            
            Data& operator()( const char* format, ... )
            {
                PRINT( ( *this ), format );
                return *this;
            }

            static Data get( unsigned int length = 10 );

            void space( unsigned int length );
                    
            void assign( const char* data, unsigned int length = 0 )
            {
                m_external = data;
                setLength( length ? length : std::strlen( data ) );
            }
            
        private:
            void setup( const char* data = NULL, unsigned int length = 0 );
            
            const char* data( ) const
            {
                return m_external ? m_external : m_data;
            }

            char* data( )
            {
                return m_data;
            }

            char* target( )
            {
                return *this + length( );
            }
            
            class Piece
            {
            
            public:
                Piece( )
                : m_data( NULL ), m_size( 0 )
                {

                }
                
                Piece( char* _ptr, unsigned int _size )
                : m_data( _ptr ), m_size( _size )
                {

                }
                
                Piece( const Piece& piece )
                : m_data( piece.m_data ), m_size( piece.m_size )
                {
                    
                }

                void clear( );
                    
                operator bool( ) const
                {
                    return m_data;
                }
                operator char*( )
                {
                    return m_data;
                }
                operator const char*( ) const
                {
                    return m_data;
                }
                operator unsigned int() const
                {
                    return m_size;
                }
                
                unsigned long operator()(   ) const;
                
                void get( unsigned int length );
                
            private:
                char* m_data;
                unsigned int m_size;
            };
            
            struct Window
            {
                unsigned int length;
                unsigned int offset;
                
                Window()
                : length( 0 ), offset( 0 )
                {
                }
                
                void operator = ( const Window& window )
                {
                    length = window.length;
                    offset = window.offset;
                }
                
                void clear()
                {
                    length = 0;
                    offset = 0;
                }
            };
            
        private:
            const char* m_external;
            Piece m_data;
            Window m_window;
        };
    }
}

namespace std
{
    template<> struct hash< tau::si::Data >
    {
        unsigned long operator()( const tau::si::Data& data ) const
        {
            return data.hash();
        }
    };
}

#endif