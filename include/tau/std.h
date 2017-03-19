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

#endif 
