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

#include "sys.h"



#ifdef __MACH__
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    #pragma clang diagnostic ignored "-Wformat"
#endif

#define MAX( a, b ) ( a > b ) ? ( a ) : ( b )
#define MIN( a, b ) ( a < b ) ? ( a ) : ( b )
#define lengthof( a ) sizeof( a ) / sizeof( a[ 0 ] )

typedef unsigned short us;
typedef unsigned int ui;
typedef unsigned long ul;
typedef unsigned char uchar;

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
        struct timeval time;
         
        long long value;
        
        Time( long millis = 0, long micros = 0 )
        {
            if ( !millis && !micros )
            {
                ::gettimeofday( &time, NULL );
                return;
            }
            
            if ( millis == Infinite )
            {
                value = millis;
                return;
            }

            value = millis * 1000;
            
            time.tv_sec = value;
            value += micros;
            time.tv_usec = micros;
        }

        Time( const Time& time )
        {
            *this = time;
        }
        void operator=( const Time& time )
        {
            value = time.value;
        }
        operator long long () const
        {
            return value;
        }
        enum Type
        {
            Infinite = -1
        };
        
        bool infinite() const
        {
            return value == -1;
        }
        
        operator const struct timespec* () const
        {
            return ( struct timespec* ) &time;
        }
        
        operator struct timeval* ()  
        {
            return &time;
        }
        
        ul s() const
        {
            return time.tv_sec;
        }
        
        ul ms() const
        {
            return time.tv_usec;
        }
    };
    
    inline Time time()
    {
        Time time;
        return time;
    }    
}

#endif 
