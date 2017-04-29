#ifndef _TAU_STD_H
#define _TAU_STD_H

#define TAU_NAME "tau"

#include <utility>
#include <initializer_list>
#include <random>
#include <typeinfo>
#include <functional>


#include <cstring>
#include <cstdlib>

#ifndef assert
    #include "assert.h"
#endif
    
#include <stdarg.h>
#include <limits.h>
#include <time.h>

typedef unsigned short us;
typedef unsigned int ui;
typedef unsigned long ul;
typedef unsigned char uchar;


#include "sys.h"

#ifdef __MACH__
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    #pragma clang diagnostic ignored "-Wformat"
    #pragma clang diagnostic ignored "-Wswitch"
#endif

#define MAX( a, b ) ( a > b ) ? ( a ) : ( b )
#define MIN( a, b ) ( a < b ) ? ( a ) : ( b )
#define lengthof( a ) sizeof( a ) / sizeof( a[ 0 ] )


namespace tau
{
    namespace r
    {
        class Random
        {
        public:
            Random( );
            ui operator()( ui max = 0 );

        private:
            std::default_random_engine m_random;
        };
        
        ul random( ui max = 0 );
    }
    
    struct Time
    {
        enum Type
        {
            Now = -2,
            Infinite = -1
    };
        
        long long value;
        
        Time( long millis = 0, long micros = 0 )
            : value( 0 )
        {
            
            switch ( millis )
            {
                case Now:
                    struct timeval time;
                    ::gettimeofday( &time, NULL );
                    value = time.tv_sec * 1000000 + time.tv_usec;
                    return;
                    
                    
                case Infinite:
                    value = millis;
                    return;
            }
                        
            value = millis;
            value *= 1000;
            value += micros;
        }
        
        

        // Time( const Time& time )
//         {
//             *this = time;
//         }
        void operator=( const Time& time )
        {
            value = time.value;
        }
        operator long long () const
        {
            return value;
        }

        bool infinite() const
        {
            return value == -1;
        }
        
        operator struct timeval () const
        {
            struct timeval time;
            time.tv_sec = value / 1000000;
            time.tv_usec = value - time.tv_sec  * 1000000 ;
            return time;
        }
        
        ul s() const
        {
            return value / 1000000;
        }
        
        ul ms() const
        {
            return value / 1000;
        }
        
        ul us() const
        {
            return value;
        }
    };
    
    inline Time time()
    {
        Time time( Time::Now );
        return time;
    }    
}

#endif 
