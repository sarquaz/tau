#ifndef _TAU_IO_H_
#define _TAU_IO_H_

#include "../../src/trace.h"

namespace tau
{
    extern ev::Loop& loop();
        
    namespace io
    {
        class Event: public ev::Request::Parent
        {
            public:
                Event()
                {

                }

                virtual ~Event()
                {
                    m_requests.all( [ ] ( ev::Request* request ) { request->deref(); } );
                }

                virtual void destroy()
                {
                    ENTER();
                    mem::mem().detype< Event >( this );
                }
                
                template < class Callback > void request( Callback callback, ev::Request* request = NULL )
                {   
                    if ( !request )
                    {
                        request = mem::mem().type< ev::Request >( *this );
                    }
                    
                    request->assign( [ & ] ( ev::Request& request ) 
                        { 
                            TRACE( "event callback", "" );
                            
                            ref();
                            callback( request );
                            this->callback();                
                            deref();
                        } );
                        
                    m_requests.append( request );

                    loop().add( *request );
                }

                virtual void callback() = 0;

            private:
                li::List< ev::Request* > m_requests;
        };
    }
}

#endif