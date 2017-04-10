// #include "tau.h"
// #include "trace.h"
//
// namespace tau
// {
//     namespace mem
//     {
//         namespace nodes
//         {
//             enum Count
//             {
//                 Max = 1000
//             };
//
//             __thread Node* t_node = NULL;
//             __thread ui t_count = 0;
//
//
//             Node& node()
//             {
//                 if ( !t_node )
//                 {
//                     t_node = new Node( );
//                     t_count ++;
//                 }
//
//                 return *t_node;
//             }
//
//             Node& get( )
//             {
//                 //
//                 //  get next node in the list
//                 //
//                 auto& node = nodes::node();
//                 t_node = node.next();
//
//                 return node;
//             }
//
//             void free( Node& node )
//             {
//                 if ( t_count >= nodes::Count )
//                 {
//                     //
//                     //  if max count has been reached call delete
//                     //
//                     t_count --;
//                     delete &node;
//                     return;
//                 }
//
//                 //
//                 //  append it to list
//                 //
//                 node.next( t_node );
//                 t_node = &node;
//             }
//         }
//
//         void* Allocator::allocate( ui size )
//         {
//             assert( size == sizeof( Node ) );
//
//             return ( void* ) &( nodes::get() );
//         }
//
//         void Allocator::deallocate( void* what )
//         {
//             //
//             //  only instances of Node should be deallocated
//             //
//             auto node = static_cast< Node* > ( what );
//             nodes::free( *node );
//         }
//     }
// }