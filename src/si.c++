#include "tau.h"
#include "trace.h"  

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
        // void Tok::destroy( )
        // {
        //     //toker( ).dispose( *this );
        // }
        
        // void Toker::dispose( Tok& tok )
//         {
//             tok.cleanup();
//
//             if ( !m_max )
//             {
//                 delete &tok;
//                 return;
//             }
//
//
//             // auto& toks =  m_map[ ( ul ) tok.code( ) ];
// //
// //             if ( toks.length() < max() )
// //             {
// //                 toks.add( &tok );
// //             }
// //             else
// //             {
// //                 delete &tok;
// //             }
//         }
        
        // Tok* Toker::get( ul hash )
        // {
        //     // Tok* tok = NULL;
        //     //
        //     // try
        //     // {
        //     //     tok = m_map.get( hash ).get( );
        //     // }
        //     // catch( ... )
        //     // {
        //     // }
        //     //
        //     // return tok;
        //
        //     return NULL;
        // }
        //
        // void Toker::clear()
        // {
        //     // m_max = 0;
        //     //
        //     // m_map.values( [] ( const Toks& toks )
        //     // {
        //     //     toks.all( [ ] ( Tok* tok )
        //     //     {
        //     //         delete tok;
        //     //     } );
        //     // } );
        //     //
        //     // m_map.clear( );
        // }
        
        // __thread Toker* s_toker = NULL;
        //
        // Toker& toker( )
        // {
        //     if ( !s_toker )
        //     {
        //         s_toker = new Toker( );
        //     }
        //
        //     return *s_toker;
        // }

        // void cleanup( )
        // {
        //     if ( s_toker )
        //     {
        //         delete s_toker;
        //         s_toker = NULL;
        //     }
        // }
    }
    
    namespace r
    {
        __thread Random* t_random = NULL;
        
        Random::Random( )
        : m_random( time().ms() /* + Thread::id( )*/ )
        {
        }
        
        ui Random::operator()( ui max )
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
        
        ul random( ui max )
        {
            if ( !t_random )
            {
                t_random = new Random( );
            }

            return( *t_random ) ( max );
        }
            
    }
    

    namespace si
    {
        void* swap( void** target, void* value )
        {
            void* oldValue = __sync_fetch_and_add( target, 0 );
            return __sync_val_compare_and_swap( target, oldValue, value );
        }
        
        ui inc( ui* target )
        {
            ui result = __sync_fetch_and_add( target, 1 );
            return result + 1;
        }

        ui dec( ui* target )
        {
            ui result = __sync_fetch_and_sub( target, 1 );
            return result - 1;
        }
        
        // void backtrace( ui length, Stream stream )
        // {
        //
        // }
        
        
        
        // void out( const Data& data, Stream stream  )
        // {
        //     auto wrote = ::write( stream, data, data.length() );
        // }
        
        
        
        

        
        
        
        
    }
    
    
        
             
        // namespace map
        // {
        //     ul Hash::operator()() const
        //     {
        //         ul hash = 0;
        //         auto pos = 0;
        //
        //         auto step = sizeof( hash );
        //
        //         if ( m_size <= step )
        //         {
        //             step = m_size / 4;
        //         }
        //
        //         ul last = 1;
        //         ul* next = NULL;
        //
        //         for ( ;; )
        //         {
        //             auto length = 0;
        //             ul number = 1;
        //
        //             if ( pos + step > m_size )
        //             {
        //                  next = ( ul* ) ( m_what + pos );
        //             }
        //             else
        //             {
        //                 for ( ;; )
        //                 {
        //                     auto index = pos + step - length;
        //                     if ( index >= m_size )
        //                     {
        //                         index = index - m_size;
        //                     }
        //
        //                     ( ( char* ) &number )[ step - length ] = m_what[ index ];
        //
        //                     length ++;
        //                     if ( length >= step )
        //                     {
        //                         break;
        //                     }
        //                 }
        //
        //                 next = &number;
        //             }
        //
        //             hash ^= *next * ( ( last + 1 ) ) ;
        //             last = *next ^ m_what[ pos ];
        //
        //             pos += MIN( step, m_size - pos );
        //
        //             if ( pos >= m_size )
        //             {
        //                 break;
        //             }
        //         }
        //
        //         return hash;
        //     }
        // }
        
    namespace si
    {
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
    
    namespace box
    {
        ul Hash::operator()() const
        {
            ul hash = 100253314709;
            ul last = 179426173;
            ul store = hash ^ m_size;
            auto pos = 0;
            ul* next;
            
            auto step = sizeof( next );
            
            ul number = hash ^ m_size;
            
            for ( ;; )
            {
                
                auto length = 0;
                auto rem = m_size - pos;

                if ( rem >= sizeof( store ) )
                {
                    next =  ( ul* ) ( m_what + pos ); 

                }
                else
                {
                    next = &store;
                    for ( auto i = 0; i < rem; i++ )
                    {

                        ( ( uchar* ) next )[ i ] = m_what[ pos + i ];
                    }
                    
                }
                
                hash ^= ( *next * last );
                last *=  m_what[ pos ];
                
                pos += MIN( step, m_size - pos );

                if ( pos >= m_size )
                {
                    break;
                }
            }

            return hash;
        }
        
    }
}
