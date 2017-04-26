#ifndef TAU_TYPES_H_
#define TAU_TYPES_H_

namespace tau
{

    struct Grain
    {
        virtual ~Grain( )
        {
        }
    };
}

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

        /*
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
        */
        void cleanup();
    }
    /*
    template < class Type > struct tok
    {
        Type& operator()( )
        {
            return *dynamic_cast < Type* > ( ie::toker().tok( typeid ( Type ), [ ] ( ) { return new Type( ); } ) );
        }
    };
    */
    class Reel
    {
    public:
        virtual ~Reel()
        {
        }
        
        void ref( );
        void deref( );
        
    protected:
        Reel( )
            : m_ref( 1 )
        {
        }
        
    private:
        virtual void destroy() = 0;
        
    private:
        ui m_ref;
    };
}

#endif