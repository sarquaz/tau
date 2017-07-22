#include "tau.h"

using namespace tau;

newev::Loop* g_loop;


class Parent: public newev::Event::Parent
{
public:
    Parent()
    {
        
    }
    
    virtual ~Parent()
    {
        
    }
    
    virtual void destroy()
    {
        mem::mem().detype< Parent >( this );
    }
    
    virtual void event( ui operation, newev::Event& event )
    {
        TRACE( "%u 0x%x", operation, &event );
        event.deref();
        g_loop->stop();
    }
};


class Task: public th::Pool::Task
{
    public:
        Task()
            : th::Pool::Task()
        {
            
        }
        
        virtual ~Task()
        {
            
        }
        
        virtual void destroy()
        {
            mem::mem().detype< Task >( this );
        }
        
        virtual void operator()()
        {
            ENTER();
        }
        
        virtual void complete( newev::Event& event )
        {
            ENTER();
            tau::stop( [](){} );
        }
};


int main( )
{
    SENTER();
    
    
    tau::start( []()
        {
            STRACE( "started", "" );
            auto task = mem::mem().type< Task >();
            tau::pool().add( *task );
            
        } );
    
    
    
}   