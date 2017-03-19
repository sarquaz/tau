#ifndef TAU_TYPES_H
#define TAU_TYPES_H

#include "data.h"

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
    }
    
    typedef si::Data Data;
    typedef si::Interval Time;
    
#define EPRINT( data, format )\
    data("%s error: ", TAU_NAME );\
    PRINT( data, format );

    struct Error
    {
        Data message;
        
        Error(  )
        {
        }
        
        Error( const char* format, ... )
        {
            PRINT( message, format );
        }
        Error( const Data& error )
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
    
    struct Grain
    {
        virtual ~Grain( )
        {
        }
    };
    
    

}

#include "li.h"

namespace tau
{
    namespace ie
    {
        class Toker;
        
        class Tok: public Grain
        {
            friend class Toker;

        public:
            virtual ~Tok( )
            {
            }
            
            virtual void destroy();
            
            virtual ui code( ) const = 0;
            
        protected:
            Tok( )
            {
            }
            
        private:
            virtual void init( )
            {
            }

            virtual void cleanup( )
            {
            }
        };

        
        class Toker
        {
        public:
            virtual ~Toker( )
            {
                clear();
            }
            
            Toker()
            : m_max( 0 )
            {
                
            }

            template < class Allocate > Tok* tok( const std::type_info& type, Allocate allocate )
            {
                Tok* tok = get( type.hash_code( ) );
                
                if ( !tok )
                {
                    tok = allocate( );
                }
                
                tok->init();
                return tok;
            }
            
            void dispose( Tok& );
            void setMax( ui max )
            {
                m_max = max;
            }

        private:
            ui max() const
            {
                return m_max;
            }
            
            void clear() ;
            Tok* get( ul );
            
            typedef li::Mass< Tok* > Toks;
            
        private:
            
            li::Map< ul, Toks >  m_map;
            ui m_max;
        };
        
        Toker& toker();
        void cleanup();
    }
    
    template < class Type > struct tok
    {
        Type& operator()( )
        {
            return *dynamic_cast < Type* > ( ie::toker().tok( typeid ( Type ), [ ] ( ) { return new Type( ); } ) );
        }
    };
    
    class Rock: public ie::Tok
    {
    public:
        virtual ~Rock()
        {
        }
        
        void ref( );
        void deref( );
        
    protected:
        Rock( )
        {
            init();
        }
        
        virtual void init()
        {
            m_ref = 1;
        }
        
    private:
        ui m_ref;
    };
}

#endif