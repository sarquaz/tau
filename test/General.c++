#include "Test.h"

using namespace tau;
#include <forward_list>
#include <vector>


TEST();

class General: public Test
{
public:
    General()
    : m_timer( false )
    {
        setInterval( Time( Time::Infinite ) );
    }
    
private:
    virtual void onTimer( Event& timer )
    {
        ENTER();
       
        m_timer = true;
        timer.deref();
        
        //li::List< int > list;
        //std::forward_list< int > list;
        //std::vector< int > list;
        li::Mass< int > list;
        for ( auto i = 0; i < 100; i++ )
        {
            //list.push_front( i );
            list.add( i );
        }
//        
        start();
    }
    
    void start()
    {
        ENTER();
        
        tau::event( this )( Time( 1 ) );
    }
    
    virtual void run( )
    {
        ENTER();
        start();
        
        out( "line %d", line().id() );
    }
    
    virtual void check()
    {
        ENTER();
        assert( m_timer );
    }
    
    bool m_timer;   
    
};


int main()
{
    return General()();
}


