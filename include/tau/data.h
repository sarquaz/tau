#ifndef _TAU_DATA_H
#define _TAU_DATA_H

#include "std.h"    

namespace tau
{
    namespace si
    {
        /**
         * General purpose binary safe buffer class (std::string replacement)
        **/
        class Data
        {
        public:
            Data( )
            {
                setup( );
            }
            Data( const char* data, ui length = 0 )
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
            ui offset( ) const
            {
                return m_window.offset;
            }
            ui offset( ui offset )
            {
                m_window.offset = offset;
            
                if ( !m_external && size( ) < offset )
                {
                    space( size( ) + offset );
                }
                
                return 0;
            }
            ui length( ) const
            {
                return m_window.length;
            }
            ui length( ui length )
            {
                m_window.length = length;
                return 0;
            }
            bool empty( ) const
            {
                return length( ) == 0;
            }
            void move( ui offset )
            {
                this->offset( this->offset( ) + offset );
            }
            void inc( ui length = 1 )
            {
                this->length( this->length() + length );
            }
            ui size( ) const
            {
                return ( ui ) m_data;
            }
            ui available( ) const
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
            char operator []( ui offset ) const;
            char& operator []( ui offset );

            Data& add( char data );
            Data& add( const char* data, ui length = 0 );
            Data& add( const Data& data )
            {
                return add( data.c(), data.length() );
            }
            void clear( )
            {
                m_external = NULL;
                m_window.clear();
            }
            int find( const Data&, ui offset = 0 ) const;
            
            ul hash() const;

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

            static Data get( ui length = 10 );

            void space( ui length );
                    
            void assign( const char* data, ui length = 0 )
            {
                m_external = data;
                this->length( length ? length : std::strlen( data ) );
            }
            
        private:
            void setup( const char* data = NULL, ui length = 0 );
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
                
                Piece( char* _ptr, ui _size )
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
                operator ui() const
                {
                    return m_size;
                }
                
                ul operator()(   ) const;
                
                void get( ui length );
                
            private:
                char* m_data;
                ui m_size;
            };
            
            struct Window
            {
                ui length;
                ui offset;
                
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
        
        //
        //  hash template specialization
        //
        namespace h
        {
            template <> struct hash< tau::si::Data >
            {
                ul operator()( const tau::si::Data& data ) const
                {
                    return data.hash();
                }
            };
        }
    }
}

#endif