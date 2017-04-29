#include "tau.h"

using namespace tau;
using namespace si;

#include "../../src/trace.h"
#include <map>

template < class T > struct Test
{    
    template< class _T > struct _Pair
    {
        _T key;
        _T value;
        
        _Pair( const _T& _key, const T& _value )
        : key( _key ), value( _value )
        {
            
        }
        
        _Pair( const _Pair& pair )
        : key( pair.key ), value( pair.value )
        {
            
        }
    };

    typedef li::Map< T, T > Map;
    typedef _Pair< T > Pair;
    typedef li::List< Pair > List;
    
    Map map;
    List list;
    ui size;
    ui count;
    
    Test( ui _size )
    : size( _size ), count( 0 )
    {
        
    }
    
    template < class Gen > Pair pair( Gen gen )
    {        
        Pair pair( gen(), gen() );
        list.add( pair );
        return pair;
    }
    
    template< class Gen, class Run > void operator()( Gen gen, Run run )
    {
        ENTER();
        
        for (  ; count < size; count ++ )
        {
            Pair pair = this->pair( gen );
            map[ pair.key ] = pair.value;
            run( pair );
        }
    }
};

// class First
// {
//     public:
//         First()
//         {
//
//         }
//
//         virtual void result(  )
//         {
//             TRACE( "0x%x", what );
//         }
// };

class Task: public th::Pool::Task
{
public:
    Task()
        : th::Pool::Task()
    {
        
    }
    
    virtual ~Task()
    {
        ENTER();
    }
    
    virtual void operator()( )
    {
        ENTER();
        
        request().data().add( Data::get() );
    }
    
};


int main( )
{

    // {
  //       Test< int > test( 30 );
  //       test( [ ] ( ) { return random( INT_MAX ); }, [ & ] ( const Test< int >::Pair & pair )
  //       {
  //           test.map.remove( pair.key );
  //
  //           try
  //           {
  //               test.map.get( pair.key );
  //               assert( false );
  //           }
  //           catch( ... )
  //           {
  //
  //           }
  //       } );
  //   }
  //  
//    {
//        Test< int > test( 30 );
  //      test( [ ] ( ) { return random( INT_MAX ); }, [ & ] ( const Test< int >::Pair & pair ) { } );
        

        // Some test;
        // li::Map< int, int > map;
        //
        // for ( int i = 0; i < 10; ++i )
        // {
        //     map[ i ] = i;
        // }
        //
        // printf( "pthread_self: %f\n", ::pthread_self( ) );
        //
        // test.start();
        // test.join();
        
    //    First first;    
    
    
  //       li::Map< int, int > map;
//
//     auto count = 3;
//
// //    void* voids[ 100 ];
//
//     {
//         li::List< int >  ints;
//
//         for( auto i = 0; i < 2; ++i )
//         {
//             ints.append( i );
//         }
//     }
        
        
       

       
       // map.remove( 2 );
//        STRACE( "%d", map.length() );
//
//
//        li::List< Data > list;
//        list.append( "test" );
//        list.append( "test1" );
//        list.clear();
//        STRACE( "%d", list.length() );
//
//        newmem::Mem mem;
//        auto test = mem.type< First >();
//        mem.detype< First >( test );
//
//
//        // li::List< First* > firsts;
// //
    
// //
// //        firsts.all( [ & ]( First* f ){ mem.detype< First >( f ); } );
//
//        test = mem.type< First >();
//        mem.detype< First >( test );
    
//        mem::disable();
        // auto event = ev::Loop::Event::get( Time( 1000 ) );
        // ev::Loop loop;
        // loop.add( *event );
        // loop.run( [ & ]( ev::Loop::Event& event )
        //     {
        //         STRACE( "event 0x%x", &event );
        //         //event.deref();
        //         loop.stop();
        //     } );
        
        
                
        
       
        tau::start( [ ] ( action::Action action )
        { 
            STRACE( "action %s", action == tau::action::Start ? "start" : "stop" );
            
            if ( action == action::Start )
            {
                STRACE( "thread %u 0x%x started", tau::thread().id(), &tau::thread() ); 
                
                Task* task = mem::mem().type< Task >();

                task->request().assign( [ & ]( ev::Request& request ) {
                    STRACE( "callback", "" );
                    STRACE( "%s", request.data().c() );
                    STRACE( "0x%x", task );
                    request.parent().deref();
                    tau::stop();    
                });

               tau::thread().pool().add( *task );
            }
        } );
        
        
       

  //      test.map.keys( [ & ] ( int key ) {  printf("key %d\n", key ); } );
        
    //     test( [ ] ( ) { return random( INT_MAX ); }, [ & ] ( const Test< int >::Pair & pair ) { assert( test.map.size() == test.count + 1 ); } );
//         test.list.all( [ & ] ( const Test< int >::Pair& pair )
//             {
//
//
// //                assert( test.map.remove( pair.key ) );
//             } );
//     }
//
        
        
    // {
  //       Test< int > test( 30 );
  //       test( [ ] ( ) { return random( INT_MAX ); }, [ & ] ( const Test< int >::Pair & pair ) { } );
  //       test.map.clear();
  //       assert( !test.map.size() );
  //       test.list.all( [ & ] ( const Test< int >::Pair& pair ) {  assert( !test.map.remove( pair.key ) ); } );
  //   }
    
    

}