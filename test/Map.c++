#include "types.h"

using namespace tau;

#include "../../src/trace.h"

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

int main( )
{
    SENTER();

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
        

        
        li::Map< int, int > map;
        
        printf(" size %d\n", map.size() );
        map[ 4 ] = 4;
        printf( "%d\n", map[ 4 ] );
        
        map[ 5 ] = 5;
        printf( "%d\n", map[ 5 ] );
        
        map[ 6 ] = 6;
        printf( "%d\n", map[ 6 ] );
        
        // map.set( 5, 5  );
        // map.set( 6, 6 );
        
        map.all( [ ] ( int key, int value ) { printf(" key %d value %d\n", key, value ); } );

        
        
        
        
        
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


