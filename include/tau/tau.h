#ifndef _TAU_TAU_H
#define _TAU_TAU_H

#include "std.h"

namespace tau
{
    namespace si 
    {
        struct Interval
        {
            struct timeval time;
             
            long long value;
            
            Interval( long millis = 0, long micros = 0 )
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

            Interval( const Interval& interval )
            {
                *this = interval;
            }
            void operator=( const Interval& interval )
            {
                value = interval.value;
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
        
        class Random
        {
        public:
            Random( );
            ui operator()( ui max = 0 );

        private:
            std::default_random_engine m_random;
        };

        ul random( ui max = 0 );        
        
        void* swap( void** target, void* value );
        ui inc( ui* target );
        ui dec( ui* target );
        
        enum Stream
        {
            In = STDIN_FILENO,
            Out = STDOUT_FILENO,
            Err = STDERR_FILENO
        };
    
        void backtrace( ui length, Stream stream );
        
                
        Interval time();

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
    }
}

#include "data.h"

namespace tau
{
    
#define EPRINT( data, format )\
    data("%s error: ", TAU_NAME );\
    PRINT( data, format );

    struct Error
    {
        si::Data message;
        
        Error(  )
        {
        }
        
        Error( const char* format, ... )
        {
            PRINT( message, format );
        }
        Error( const si::Data& error )
        : message( error )
        {
            
        }
        Error( const Error& error )
        {
            *this = error;
        }
        virtual ~Error()
        {
            
        }
        void operator = ( const Error& error )
        {
            message.clear();
            message.add( error.message );
        }
        operator const char* () const
        {
            return message;
        }
        
        static Error Tau( const char* format, ... )
        {
            Error error;
            EPRINT( error.message, format );
            return error;   
        }
    };
    
    namespace si
    {
        void out( const Data& data, Stream stream  );
        
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
            //
            //  Data
            //   
            template <> struct hash< Data >
            {
                ul operator()( const Data& data ) const
                {
                    return data.hash();
                }
            };
        }        
    }
    
    //
    //  Template specializations for integral types
    //
    
    typedef si::Interval Time;
    typedef si::Data Data;
}

#include "si.h"
#include "li.h"

#endif