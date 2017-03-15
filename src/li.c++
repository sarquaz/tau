//#include "types.h"
//
//#include "trace.h"
//
//namespace tau
//{
//    namespace li
//    {
////        namespace map
////        {
////            ul Bash::operator() ()
////            {
////                auto pos = 0;
////                auto length = sizeof( hash );
////                auto prev = value;
////                    
////                for ( auto i = 0; i < 2; i++ )
////                {
////                    if ( offset + pos <= length )
////                    {
////                        *( ( unsigned char* ) &value + pos ) = *( ( unsigned char* ) &hash + offset + pos );
////                    }
////
////                    pos++;
////                }
////                
////                if ( value != prev )
////                {
////                    value ^= prev >> round;
////                }
////                offset ++;
////                round ++;
////                
////                if ( offset == length )
////                {
////                    offset = 1;
////                }
////                
////                return value;
////            }
////        }
////        
//        void Strings::operator=( const Strings& strings )
//        {
//            m_map.clear( );
//            strings.all( [ & ] ( const Data& key, const Data& value ) 
//            {
//                ( *this ) [ key ] = value; 
//            } );
//        }
//
//        si::Store& Strings::operator[]( const Data& key )
//        {
//            auto hash = key.hash( );
//
//            auto& data = m_map[ hash ];
//            if ( !( data.key == key ) )
//            {
//                data.key = key;
//            }
//            
//            return data.value;
//        }
//
//        const Data& Strings::operator[]( const Data& key ) const
//        {
//            
//        }
//
//        Data Strings::get( const Data& key, const Data& def ) const
//        {
//            
//        }
//    }
//}
