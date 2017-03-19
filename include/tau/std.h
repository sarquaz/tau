#ifndef _TAU_STD_H
#define _TAU_STD_H

#define TAU_NAME "tau"

#include <utility>
#include <initializer_list>
#include <random>
#include <typeinfo>


#include <cstring>
#include <cstdlib>

#ifndef assert
    #include "assert.h"
#endif
    
#include <stdarg.h>
#include <limits.h>
#include <time.h>


#define MAX( a, b ) ( a > b ) ? ( a ) : ( b )
#define MIN( a, b ) ( a < b ) ? ( a ) : ( b )
#define lengthof( a ) sizeof( a ) / sizeof( a[ 0 ] )


typedef unsigned short us;
typedef unsigned int ui;
typedef unsigned long ul;
typedef unsigned char uchar;

namespace tau
{
    namespace si 
    {
        class Hash
        {
        public:
            Hash( uchar* what, ui size = sizeof( ul ) )
                : m_what( what ), m_size( size )
            {
            }
        
            ul operator()() const;
        
            ~Hash ()
            {
            }
    
        private:
            unsigned char* m_what;
            ui m_size;
        };
        
        //
        //  Template specializations for integral types
        //
        namespace h
        {
            template < class What > struct hash
            {
                ul operator()( const What& what ) const;
            };
            
            //
            //  ushort
            //
            template <> struct hash < us >
            {
                ul operator()( const us& what ) const
                {
                    return si::Hash( ( uchar* ) &what, sizeof( us ) )();
                }
            }; 
            //
            //  uint
            //
            template <> struct hash < ui >
            {
                ul operator()( const ui& what ) const
                {
                    return si::Hash( ( uchar* ) &what, sizeof( ui ) )();
                }
            }; 
            //
            //  ulong
            //
            template <> struct hash < ul >
            {
                ul operator()( const ul& what ) const
                {
                    return si::Hash( ( uchar* ) &what )();
                }
            }; 
            //
            //  int
            //
            template <> struct hash < int >
            {
                ul operator()( const int& what ) const
                {
                    return si::Hash( ( uchar* ) &what, sizeof( int ) )();
                }
            };
            //
            //  long
            //
            template <> struct hash < long >
            {
                ul operator()( const long& what ) const
                {
                    return si::Hash( ( uchar* ) &what, sizeof( long ) )();
                }
            }; 
        }
    }
}

#endif 
