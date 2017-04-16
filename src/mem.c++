#include "tau.h"

namespace tau
{
    namespace mem
    {        
        Mem::Nodes::Nodes()
            : node( NULL )
        {
        }
        
        Mem::Nodes::~Nodes()
        {
            while ( node )
            {
                auto next = node->next();
                std::free( ( void* ) node );
                node = next;
            }
        }
        
        Node* Mem::Nodes::get()
        {
            Node* node;
            //
            //  get next node in the list
            //
            if ( this->node )
            {
                 node = this->node;
                 this->node = node->next();
            }
            else
            {
                node = ( Node* ) std::malloc( sizeof( Node ) );
            }
            
            
            return node;
        }
        
        void Mem::Nodes::free( Node* node )
        {
            //
            //  append it to list
            //
            node->next( this->node );
            this->node = node;
            return;
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
        
        __thread Mem* t_mem = NULL;
        
        Mem& mem()
        {
            if ( !t_mem )
            {
                t_mem = new Mem();
            }
            
            return *t_mem;
        }
                
        void disable()
        {
            if ( t_mem )
            {
                delete t_mem;
                t_mem = NULL;
            }
        }
        
        void* Allocator::allocate( ui size )
        {
            assert( size == sizeof( Node ) );
            
            return ( void* ) ( mem().nodes().get() );
            
        }

        void Allocator::deallocate( void* what )
        {
            //
            //  only instances of Node should be deallocated
            //            
            mem().nodes().free( static_cast< Node* > ( what ) );
        }
        
        Mem::Mem()
            : m_total( 1024 * 1024 * 10 ), m_used( 0 )
        {
        }
        
        Mem::~Mem()
        {
        }
        
        void* Mem::get( ui size )
        {
            ul hash = box::h::hash< ui >()( size );
            void* data = NULL;
            
            //
            //  lookup list
            //
            auto& list = m_map [ hash ];
            
            
            if ( list.length() )
            {
                auto node = list.tail();
                
                data = node->data();
                node->remove();
            }    
            else
            {
                Size s( hash, size );
                data = std::malloc( size + sizeof( s ) );
                std::memcpy( data, &s, sizeof( s ) );
            }
            
            data = ( uchar* ) data + sizeof( Size );
            
            return data;
            
        }
        
        void Mem::free( void* data )
        {
            data =  ( uchar* ) data - sizeof( Size );
            Size* s = ( Size* ) ( data );
                        
            auto& list = m_map[ s->hash ];
        
            if ( m_used + s->size < m_total )
            {
                m_used += s->size;
                list.append( data );    
            }
            else
            {
                m_used -= s->size;
                std::free( data );
            }
        }
    }
}