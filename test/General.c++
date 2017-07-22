#include "Test.h"


class General: public Test
{
public:
    General()
        : Test( 2, Time( 3000 ) )
    {
        start();
    }
    
private:
    virtual void run()
    {
        ENTER();
    }
    
    virtual void check()
    {
        assert( true );
    }
    
    
};

struct Result: public io::Result
{
    virtual void event( ui event, ev::Request& parent )
    {
        ENTER();
//        parent.deref();
        
  //      Result::timer();
        
        
    }
    
    virtual void destroy()
    {
        mem::mem().detype< Result >( this );
    }
    
    static void timer()
    {
        SENTER();
        auto result = mem::mem().type< Result >();
        io::timer( *result, {{ options::Repeat, true}, {options::Msec, 100 }} );
        result->deref();        
    }
    
};


int main()
{
    new General();
    
    
    
    // tau::start( [ ] ( )
    //     {
    //         Result::timer();
    //     });
    return 0;
}


