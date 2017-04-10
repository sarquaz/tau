// #ifndef _TAU_MEM_H_
// #define _TAU_MEM_H_
//
// #include "box.h"
//
// namespace tau
// {
//     namespace mem
//     {
//         struct Allocator
//         {
//             static void* allocate( ui size );
//
//             static void deallocate( void* data );
//         };
//
//         typedef box::Node< box::Type, Allocator > Node;
//
//         class Mem: box::Map< 0x100, Allocator >
//         {
//
//         };
//     }
// }
//
// #endif