#include "tau.h"

using namespace tau;

#include "../../src/trace.h"
#include "newmem.h"
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

class First: public tau::Listener
{
    public:
        First()
        {
        
        }
    
        virtual void result( Result* what )
        {
            TRACE( "0x%x", what );
        }
};



int main( )
{

    // {
  //       Test< int > test( 30 );
  //       test( [ ] ( ) { return si::random( INT_MAX ); }, [ & ] ( const Test< int >::Pair & pair )
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
  //      test( [ ] ( ) { return si::random( INT_MAX ); }, [ & ] ( const Test< int >::Pair & pair ) { } );
        

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
        
        First first;    
    
    
        li::Map< int, int > map;
    
    auto count = 3;
    
//    void* voids[ 100 ];
    

    
    for( ui i = 0; i < count; ++i )
    {
  //      auto s = Data::get( 10 );
        
//        ul hash = si::h::hash< ui >()( i );
        
                

                            map[ i ] = i;
    }

       STRACE( "%d", map.length() );
         //
  //       //map.all( [  ]( ul hash, int d ) { STRACE( "id: %d, d: %d", hash, d ); } );
  //
  //       for( ui i = 0; i < count; ++i )
  //       {
  //
  //
  //           ul hash = si::h::hash< ui >()( i );
  //
  //           STRACE( "hash: %u, d: %d", hash, i );
  //
  //           assert( map[ hash ] == i );
  //
  //
  //
  //
  //       }
  // //    
//        STRACE( "%d", map[ 3 ] );
    
        // tau::listen( &first );
 //         tau::start( { { "threads", "2" } } );
 //         //
 //         //
 //         //
 //         //
 //          tau::stop();
       
       li::Array< int > ints;

       
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
       for( auto i = 0; i < 20; ++i )
       {
           ints.add( i );
       }
// //
// //        firsts.all( [ & ]( First* f ){ mem.detype< First >( f ); } );
//
//        test = mem.type< First >();
//        mem.detype< First >( test );
       
           
       
       
       
       

  //      test.map.keys( [ & ] ( int key ) {  printf("key %d\n", key ); } );
        
    //     test( [ ] ( ) { return si::random( INT_MAX ); }, [ & ] ( const Test< int >::Pair & pair ) { assert( test.map.size() == test.count + 1 ); } );
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
  //       test( [ ] ( ) { return si::random( INT_MAX ); }, [ & ] ( const Test< int >::Pair & pair ) { } );
  //       test.map.clear();
  //       assert( !test.map.size() );
  //       test.list.all( [ & ] ( const Test< int >::Pair& pair ) {  assert( !test.map.remove( pair.key ) ); } );
  //   }
    
    

}


