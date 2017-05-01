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
                            before( request );
                            callback( request );
                            after();
                            deref();
                        } );
                        
                    m_requests.append( request );

                    loop().add( *request );
                }

                virtual void before( ev::Request&  )
                {
                    
                }
                
                virtual void after(  )
                {
                    
                }
                

            private:
                li::List< ev::Request* > m_requests;
        };
        
        class Process: public Reel
        {
        public:
            Process( const Data& command )
                : m_command( command )
            {
                
            }
            
            virtual ~Process()
            {
                m_streams.all( []( Stream* stream ){ stream->deref(); } );
            }
            
            const os::Process& process() const
            {
                return m_process;
            }
            
            template < class Callback > void read( Callback callback )
            {
                event( callback, out::Out );
            }
            
            template < class Callback > void write( Callback callback, const Data& data )
            {
                event( callback, out::In, &data );
            }
            
            template < class Callback > void error( Callback callback )
            {
                event( callback, out::Err );
            }
            
            virtual void destroy()
            {
                mem::mem().detype< Process >( this );            
            }
            
        private:
            
            
            
            template < class Callback > void event( Callback callback, out::Stream type, const Data* data = NULL )
            {
                if ( !m_process.pid() )
                {
                    m_process.start( m_command );            
                }
                
                auto& stream  = m_process.stream( type );
                auto event = mem::mem().type< Stream >( type, type == out::In ? stream.writeFd() : stream.readFd() );
                
                ev::Request* request = NULL;
                if ( data )
                {
                    request = mem::mem().type< ev::Request >( *event );
                    request->data() = *data;
                }
                
                event->request( callback, request );
                m_streams.append( event );
            }
            
            class Stream: public Event, public fs::File 
            {
            public:
                Stream( out::Stream type, ev::Handle fd )
                    : m_type( type )
                {
                    fs::File::assign( fd );
                }
                
                virtual ~Stream()
                {
                    
                }
                
                virtual void configure( ev::Loop::Event& event )
                {
                    ENTER();
                    event.fd = fd();
                    event.type = m_type == out::In ? ev::Loop::Event::Write : ev::Loop::Event::Read;
                }
                
                virtual void before( ev::Request& request )
                {
                    ENTER();
                    
                    if ( m_type == out::In )
                    {
                        fs::File::write( request.data() );        
                    }
                    else
                    {
                        auto read = fs::File::read( request.data() );        
                        TRACE( "read %d bytes", read );
                    }                
                }
                
                virtual void destroy()
                {
                    mem::mem().detype< Stream >( this );            
                }
                
                
            private:
                out::Stream m_type;
            };
            
        private:
            os::Process m_process;
            Data m_command;
            li::List< Stream* > m_streams;
        };
        
        inline Process& process( const Data& command )
        {
            auto process = mem::mem().type< Process >( command );         
            return *process;
        }
    }
}

#endif