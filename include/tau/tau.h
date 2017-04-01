#ifndef _TAU_TAU_H
#define _TAU_TAU_H

#include "std.h"

namespace tau
{
    namespace si 
    {
        void* swap( void** target, void* value );
        ui inc( ui* target );
        ui dec( ui* target );
    }
}

#include "out.h"
#include "mem.h"
#include "box.h"
#include "data.h"

namespace tau
{
    namespace si
    {
//        void out( const Data& data, Stream stream  );
        
        struct check
        {
            ui skip;
            long result;

            check( long _result, ui _skip = EWOULDBLOCK )
            : skip( _skip ), result( _result )
            {
            }

            ul operator()( const char* format, ... )
            {
                if ( errno == skip || result != -1 )
                {
                    return result;
                }

                Error error;
                EPRINT( error.message, format );
                error.message( ", errno %d", errno );
                throw error;
            }
        };
        
        namespace h
        {
            template < class What > struct hash
            {
                ul operator()( const What& what ) const;
            };
            
            //
            //  Template specializations for integral types
            //
    
        
            //
            //  ushort
            //
            template <> struct hash < us >
            {
                ul operator()( const us& what ) const
                {
                    return box::Hash( ( uchar* ) &what, sizeof( us ) )();
                }
            }; 
            //
            //  uint
            //
            template <> struct hash < ui >
            {
                ul operator()( const ui& what ) const
                {
                    return box::Hash( ( uchar* ) &what, sizeof( ui ) )();
                }
            }; 
            //
            //  ulong
            //
            template <> struct hash < ul >
            {
                ul operator()( const ul& what ) const
                {
                    return box::Hash( ( uchar* ) &what )();
                }
            }; 
            //
            //  int
            //
            template <> struct hash < int >
            {
                ul operator()( const int& what ) const
                {
                    return box::Hash( ( uchar* ) &what, sizeof( int ) )();
                }
            };
            //
            //  long
            //
            template <> struct hash < long >
            {
                ul operator()( const long& what ) const
                {
                    return box::Hash( ( uchar* ) &what, sizeof( long ) )();
                }
            };
            //
            //  Data
            //   
            template <> struct hash< data::Data >
            {
                ul operator()( const data::Data& data ) const
                {
                    return data.hash();
                }
            };
        }        
    }
    
    typedef data::Data Data;
}

#include "si.h"
#include "li.h"
#include "main.h"


#endif