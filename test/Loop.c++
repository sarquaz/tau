#include "types.h"
#include "unix.h"
#include "../src/trace.h"
#include <unordered_map>


//
using namespace tau;
//using namespace si;

struct Clock
{
    struct itimerspec time;
    int fd;
    
    operator int()
    {
        return fd;
    }
    
    Clock()
    {
        fd = ::timerfd_create( CLOCK_MONOTONIC, 0 );
        
        ::memset( &time, 0, sizeof( time ) );
//        time.it_interval.tv_sec = 1;
  //      time.it_value.tv_sec = 2;
        //time_t
        
        time.it_interval.tv_nsec = 1000 * 1000;
        time.it_value.tv_nsec = 1000 * 1000;
        
        assert( ::timerfd_settime( fd, 0, &time, NULL ) != -1 );
    }
    
    ~Clock()
    {
        ::close( fd );
    }
};

struct Epoll
{
    int fd;
    
    typedef struct epoll_event Event;
    
    enum Action
    {
        Add,
        Remove
    };
    
    
    Epoll()
    {
        ENTER();
        fd = ::epoll_create1( 0 );
    }
    
    void add( int fd, void* data )
    {
        ENTER();
        action( fd, Add, data );
    }
    
    void remove( int fd )
    {
        action( fd, Remove, NULL );
    }
    
    void action( int _fd, Action action, void* data )
    {
        
        Event set;
        set.events = EPOLLIN;
        set.data.ptr = data;
        
        assert( ::epoll_ctl( fd, action == Add ? EPOLL_CTL_ADD : EPOLL_CTL_DEL, _fd, &set ) != -1 );
    }
    
    void event( void* data )
    {
        auto clock = ( Clock* ) data;
        long long value;
        auto read = ::read( *clock, &value, sizeof( value ) );
        
//        remove( *clock );
//        delete clock;
//        
//        clock = new Clock();
//        add( *clock, clock );;
        
//        li::Map< Data, Data > map;
        //std::unordered_map< ui, Data > map;
//        for ( auto i = 0; i < 1; i++ )
//        {
//            
//            
//            map[ "a" ] = "b";
//            
//         //   list.add ( random( 1000000 ) );
//        }
    }
    
    void run()
    {
        
        for ( ;; )
        {
            Event events[ 10 ];
            auto changes = ::epoll_wait( fd, events, lengthof( events ), -1 );

            if ( changes >= -1 )
            {
                for ( auto i = 0; i < changes; i++ )
                {
                    event( events[ i ].data.ptr );
                }
            }
        }
        
    }
};

int main()
{

    ie::toker().setMax( 100 );
    
    Epoll epoll;
    auto clock = new Clock();
    epoll.add( *clock, clock );
    
    epoll.run();
    
    

    
    
    
    //
    
    
    
    
    
//    File file( test );
//    file.open( "test1" );
//    
    
    return 0;
}

