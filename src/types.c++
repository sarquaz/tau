#include "si.h"
#include "trace.h"  
#include "mem.h"

namespace tau
{
    void Rock::ref( )
    {
        if ( !m_ref )
        {
            assert( false );
            return;
        }

        m_ref++;
        
        TRACE( "%d", m_ref );
    }

    void Rock::deref( )
    {
        if ( !m_ref )
        {
            assert( false );
            return;
        }

        m_ref--;
        TRACE( "%d", m_ref );

        if ( !m_ref )
        {
            destroy( );
        }
    }
    
    namespace ie
    {
        
        void Tok::destroy( )
        {
            toker( ).dispose( *this );            
        }
        
        void Toker::dispose( Tok& tok )
        {
            tok.cleanup();

            if ( !m_max )
            {
                delete &tok;
                return;
            }

             
            auto& toks =  m_map[ ( unsigned long ) tok.code( ) ];
            
            if ( toks.length() < max() )
            {
                toks.add( &tok );
            }
            else
            {
                delete &tok;
            }
        }
        
        Tok* Toker::get( unsigned long hash )
        {
            Tok* tok = NULL;
            
            try
            {
                tok = m_map.get( hash ).get( );
            }
            catch( ... )
            {
            }

            return tok;
        }
        
        void Toker::clear()
        {
            m_max = 0;
            
            m_map.values( [] ( const Toks& toks ) 
            { 
                toks.all( [ ] ( Tok* tok ) 
                {
                    delete tok;
                } );
            } );
            
            m_map.clear( );
        }
        
        __thread Toker* s_toker = NULL;
        
        Toker& toker( )
        {
            if ( !s_toker )
            {
                s_toker = new Toker( );
            }

            return *s_toker;
        }

        void cleanup( )
        {
            if ( s_toker )
            {
                delete s_toker;
                s_toker = NULL;
            }
        }
    }
    

    namespace si
    {
        int Data::find( const Data& data, unsigned int offset ) const
        {
            auto found = ( char* ) ::memmem( *this + offset, length(), data, data.length() );
            
            if ( found )
            {
                auto pos = found - *this;
                return ( pos < length() ) ? pos : -1;
            }
            
            return -1;
        }
        
        void Data::setup( const char* data, unsigned int length )
        {
            m_external = data;
            
            if ( !length && data )
            {
                length = std::strlen( data );
            };
            
            setLength( length );
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
        
        Data& Data::add( const char* data, unsigned int length )
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
               
                ::memcpy( target( ), data, length );
                *( target() + length ) = 0;
            }
            
            inc( length );
            return *this;
        }
        
        void Data::space( unsigned int length )
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
        
                        
        char& Data::operator[]( unsigned int offset )
        {
            if ( !data( ) || offset > ( unsigned int ) m_data )
            {
                throw Error();
            }

            return *( data( ) + offset );
        }

        char Data::operator[]( unsigned int offset ) const
        {
            if ( !data( ) || offset > ( unsigned int ) m_data )
            {
                return 0;
            }

            return *( data( ) + offset );
        }

        
        Data Data::get( unsigned int length )
        {
            Data data;
            
            auto start = 33;
            auto range = 126 - start;
            
            unsigned int number = 0;
            auto pos = 0;
            
            while ( data.length() < length )
            {
                if ( !number )
                {
                    number = random( UINT_MAX );
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
        
        unsigned long Data::hash( ) const
        {
            auto hash = Piece( ( char* ) ( const char *) m_data, length() )();

            return hash;
        }
        
        __thread mem::Char* t_mem = NULL;  
        
        struct Chunk
        {
            enum
            {

                Size = 0x40
            };

            static unsigned int bytes( unsigned int size )
            {
                return chunks( size ) * Size;
            }

            static unsigned int chunks( unsigned int length )
            {
                auto size = length / Size;

                if ( size * Size < length )
                {
                    size += 1;
                }

                return size;
            }
        };
        
        mem::Char& chars()
        {
            if ( !t_mem )
            {
                t_mem = new mem::Char( Chunk::Size );
            }
            
            return *t_mem;
        }
        
        void Data::Piece::clear( )
        {
            if ( m_data )
            {
                chars().free( m_data, m_size / Chunk::Size );
                m_size = 0;
            }
        }
         
        void Data::Piece::get( unsigned int length )
        {
            Piece old = *this;
            
            auto chunks = Chunk::chunks( m_size + length );
            m_size += chunks * Chunk::Size;
            m_data = chars().get( m_size );
            
            if ( old.m_data )
            {
                std::memcpy( m_data, old.m_data, old.m_size );
                old.clear( );
            }
        }
        
        unsigned long Data::Piece::operator()( ) const
        {            
            unsigned long hash = 0;
            auto pos = 0;
            
            auto step = sizeof( hash );
            
            if ( m_size < step )
            {
                step = 1;
            }
            
            unsigned long last = 0;
            unsigned long* next = NULL;
            
            for ( ;; )
            {
                auto length = 0;
                unsigned long number = 0;
                
                if ( pos + step >= m_size )
                {
                     next = ( unsigned long* ) ( m_data + pos ); 
                }
                else
                {
                    for ( ;; )
                    {
                        auto index = pos + step - length;
                        if ( index >= m_size )
                        {
                            index = index - m_size;
                        }

                        ( ( char* ) &number )[ step - length ] = m_data[ index ];

                        length ++;
                        if ( length >= step )
                        {
                            break;
                        }
                    }
                    
                    next = &number;
                }
                
                
                hash ^= *next * ( ( last + 1 ) ) ;
                last = *next ^ m_data[ pos ];
                
                pos += MIN( step, m_size - pos );
                
                if ( pos >= m_size )
                {
                    break;
                }
            }
            
            return hash;
        }
        
        __thread Random* t_random = NULL;

        unsigned long random( unsigned int max )
        {
            if ( !t_random )
            {
                t_random = new Random( );
            }

            return( *t_random ) ( max );
        }
        
        Random::Random( )
        : m_random( millis( ) /* + Thread::id( )*/ )
        {
        }
        
        unsigned int Random::operator()( unsigned int max )
        {
            auto random = m_random( );

            if ( max )
            {
                if ( max )
                {
                    random = random % max;
                }
            }

            return random;
        }
        
        namespace mem
        {
            namespace nodes
            {
                __thread Node* t_node = NULL;
                
                Node& node()
                {
                    if ( !t_node )
                    {
                        t_node = new Node( );
                    }
                    
                    return *t_node;
                }

                Node& get( )
                {
                    auto& node = nodes::node();
                    t_node = node.next;
                    node.clear();
                                        
                    return node;
                }
                
                void free( Node& node )
                {
                    node.next = t_node;
                    t_node = &node;
                }
            }

            Node& Node::get( void* data )
            {
                auto& node = nodes::get();
                                
                node.data = data;
                return node;
            }

            void Node::free( )
            {
                nodes::free( *this );
            }
            
            __thread Sizes* t_sizes = NULL;
            
            Sizes& sizes()
            {
                if ( !t_sizes )
                {
                    t_sizes = new Sizes();
                }
                
                return *t_sizes;
            }
        }
    }
}