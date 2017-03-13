#ifndef TAU_TYPES_H
#define TAU_TYPES_H

#include "data.h"

namespace tau
{
    namespace si
    {
        struct Interval
        {
            long long value;
            
            Interval( long millis = 0, long micros = 0 )
            {
                if ( millis == Infinite )
                {
                    value = millis;
                    return;
                }

                value = millis * 1000;
                value += micros;
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
        };

        class Random
        {
        public:
            Random( );
            unsigned int operator()( unsigned int max = 0 );

        private:
            std::default_random_engine m_random;
        };

        unsigned long random( unsigned int max = 0 );
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
            
            virtual unsigned int code( ) const = 0;
            
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
            void setMax( unsigned int max )
            {
                m_max = max;
            }

        private:
            unsigned int max() const
            {
                return m_max;
            }
            
            void clear() ;
            Tok* get( unsigned long );
            
            typedef li::Mass< Tok* > Toks;
            
        private:
            
            li::Map< unsigned long, Toks >  m_map;
            unsigned int m_max;
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
        unsigned int m_ref;
    };
}



#endif