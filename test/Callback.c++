#include "tau.h"

using namespace tau;

namespace t
{
    template < class ...Args > class Call
    {
    public:
        Call()
        {
            
        }
        
        virtual ~Call()
        {
            
        }
        
        virtual void operator()( Args&& ... args ) const = 0;
        virtual void destroy() = 0;
        
    };
    
    template < class Callable, class ...Args > class Callback: public Call< Args...  >
    {
    public:
        Callback( Callable callable )
            : Call< Args...  >()
        {
            ENTER();
            auto size = align( sizeof( Callable ) );
            TRACE( "need %d bytes", size );
            m_store = mem::mem().get( size ); 
            std::memcpy( m_store, ( void* ) &callable, sizeof( Callable ) );
        }
        
        virtual ~Callback()
        {
            ENTER();
            mem::mem().free( m_store );
        }
        
        virtual void operator()( Args&& ... args ) const
        {
            ( *( static_cast< Callable* >( m_store ) ) )( std::forward< Args >( args ) ...  );
        }
        
        virtual void destroy()
        {
            ENTER();
            this->~Callback();
            mem::mem().free( this );
        }
        
        
    private:
        ui align( ui size ) const
        {
            auto rem = size % sizeof( void* );
            if ( rem )
            {
                size += sizeof( void* ) - rem;
            }
            
            return size;
        }
    private:
        void* m_store;
    };
}

class Thread: public os::Thread
{
    public:
        Thread()
            : os::Thread(), m_callback( NULL )
        {
            
        }
        
        virtual ~Thread()
        {
            ENTER();
            
            if ( m_callback )
            {
                m_callback->destroy();
            }
        }
        
        template < class Callback > void assign( Callback callback )
        {
            m_callback = mem::mem().type< t::Callback< Callback > >( callback );
            
        } 
        
        virtual void run()
        {
            ENTER();
            ::sleep( 2 );
            ( *m_callback )();
        }
        
    private:
        os::Condition m_condition;
        t::Call<>* m_callback;
            
        
};

template< class Callback > void test( Callback callback )
{
    
    
 //    printf("%d\n", sizeof( callback ) );
 //    t::lambda< void() > l;
 //
 //    
 //
 //    void* test;
 //    test =  new char[ sizeof( Callback ) ];
 //    std::memcpy( test, &callback, sizeof( Callback ) );
 //
 // //   std::type_info type = Callback;
 //
 //    
    
    //c( 5 );    
    
    
}

int main()
{
    0Thread thread;
    thread.assign( [](){ STRACE( "callback", "" ); } );
    thread.start();
    thread.join();
}   