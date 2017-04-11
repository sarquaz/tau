#include "newmem.h"

#include "tau.h"
#include "trace.h"

namespace tau
{
    namespace newmem
    {
        namespace nodes
        {
            enum Count
            {
                Max = 1000
            };

            __thread newmem::Node* t_node = NULL;
            __thread ui t_count = 0;


            Node& node()
            {
                if ( !t_node )
                {
                    void* data = NULL;
                    STRACE( "allocating memory for node", "" );
                    t_node = new Node( data );
                    t_count ++;
                }

                return *t_node;
            }

            Node& get( )
            {
                SENTER();
                //
                //  get next node in the list
                //
                auto& node = nodes::node();
                t_node = node.next();

                return node;
            }

            void free( Node& node )
            {
                SENTER();
                
                if ( t_count >= nodes::Max )
                {
                    //
                    //  if max count has been reached call delete
                    //
                    t_count --;
                    delete &node;
                    return;
                }

                //
                //  append it to list
                //
                node.next( t_node );
                t_node = &node;
            }
        }

        void* Allocator::allocate( ui size )
        {
            assert( size == sizeof( Node ) );
 
            return ( void* ) &( nodes::get() );
        }

        void Allocator::deallocate( void* what )
        {
            //
            //  only instances of Node should be deallocated
            //
            auto node = static_cast< Node* > ( what );
            nodes::free( *node );
        }
        
        struct Size
        {
            ul hash;
            ul size;
            
            Size()
            {
            }
            
            Size( ul _hash, ul _size )
                : hash( _hash ), size( _size )
            {
            }
            
            
        };
        
        void* Mem::get( ui size )
        {
            ENTER();
            
            TRACE( "need to get %d bytes", size );
            
            ul hash = box::h::hash< ui >()( size );
            void* data = NULL;
            
            //
            //  lookup list
            //
            auto& list = m_map[ hash ];
            
            TRACE( "hash %u", hash );
            
            if ( list.length() )
            {
                auto node = list.tail();
                
                data = node->data();
                node->remove();
            }    
            else
            {
                Size s( hash, size );
                
                TRACE( "used size %u of %u, using %u bytes", m_used, m_total, s.size );
                data = std::malloc( size + sizeof( s ) );
                std::memcpy( data, &s, sizeof( s ) );
            }
            
            data = ( uchar* ) data + sizeof( Size );
            
            return data;
            
        }
        
        void Mem::free( void* data )
        {
            Size s;
            data = ( uchar* ) data - sizeof( s );
            
            std::memcpy( &s, data, sizeof( s ) );
            
            auto& list = m_map[ s.hash ];
            
            TRACE( "used size %u of %u, need to free %u bytes", m_used, m_total, s.size );
            
            if ( m_used + s.size < m_total )
            {
                m_used += s.size;
                list.append( data );    
            }
            else
            {
                m_used -= s.size;
                std::free( data );
            }
        }
        
    }
}