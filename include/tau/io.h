#ifndef _TAU_IO_H_
#define _TAU_IO_H_

#include "../../src/trace.h"

namespace tau
{
    extern ev::Loop& loop();
    extern th::Pool& pool();
    
        
    namespace io
    {
        class Event: public ev::Request::Parent
        {
            public:
                Event()
                    : m_reel( NULL )
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
                
                Reel* reel() const
                {
                    return m_reel;
                }
            
                Reel* reel( Reel* reel ) 
                {
                    m_reel = reel;
                    return m_reel;
                } 
                

            private:
                li::List< ev::Request* > m_requests;
                Reel* m_reel;
        };
        
        class Process: public Reel
        {
        public:
            class Stream: public Event, public fs::File 
            {
                friend class Process;
                
            public:
                Stream( out::Stream type, ev::Handle fd, Process& process )
                    : m_type( type ), m_process( process ), fs::File( fd )
                {
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
                
                const Process& process() const
                {
                    return m_process;
                }
                                
            private:
                out::Stream m_type;
                Process& m_process;
            };
            
            Process( const Data& command )
                : m_command( command )
            {
            }
            
            virtual ~Process()
            {
                ENTER();
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
                auto event = mem::mem().type< Stream >( type, type == out::In ? stream.writeFd() : stream.readFd(), *this );
                
                ev::Request* request = NULL;
                if ( data )
                {
                    request = mem::mem().type< ev::Request >( *event );
                    request->data() = *data;
                }
                
                event->request( callback, request );
                m_streams.append( event );
            }
            
            
            
        private:
            os::Process m_process;
            Data m_command;
            li::List< Stream* > m_streams;
            Reel* m_reel;
        };
        
        inline Process& process( const Data& command )
        {
            auto process = mem::mem().type< Process >( command );         
            return *process;
        }
        
        class File: public Event
        {
        public:
            
            class Task: public th::Pool::Task
            {
            public:
                virtual ~Task()
                {
                    m_file.deref();
                }
                
                File& file()
                {
                    return m_file;
                }
                
            protected:
                Task( File& file, ul offset )
                    : m_file( file ), m_offset( offset )
                {
                    file.ref();    
                }
                    
                
                
                ul offset() const
                {
                    return m_offset;
                }
                
            private:
                File& m_file;
                ul m_offset;
            };
            
            class Read: public Task
            {
                public:
                    Read( File& file, ul length, ul offset )
                        : Task( file, offset ), m_length( length )
                    {
                    }
                    
                    virtual ~Read()
                    {
                        ENTER();
                    }
                    
                    virtual void operator()( )
                    {
                        ENTER();
                        
                        try
                        {
                            if ( !m_length )
                            {
                                auto info = fs::info( file().f().path() );
                                m_length = info.size();    
                            }
                            
                            file().f().read( request().data(), m_length, offset() );
                        }
                        catch ( Error* e )
                        {
                            request().error( e );
                        }
                    }
                    
                    virtual void destroy()
                    {
                        mem::mem().detype< Read >( this );            
                    }
                    
                    
                    
                private:
                    ul m_length;
     
            };
            
            class Write: public Task
            {
                public:
                    Write( File& file, const Data& data, ul offset )
                        : Task( file, offset ), m_data( data )
                    {
                    }
                    
                    virtual ~Write()
                    {
                        ENTER();
                    }
                    
                    virtual void operator()( )
                    {
                        ENTER();
                        
                        try
                        {
                            file().f().write( m_data, offset() );
                        }
                        catch ( Error* e )
                        {
                            request().error( e );
                        }
                    }
                    
                    virtual void destroy()
                    {
                        mem::mem().detype< Write >( this );            
                    }
                    
                private:
                    Data m_data;
            };
            
            
            File( const Data& path )
                : m_file( fs::File::open( path ) )
            {
                
            }
            
            virtual ~File()
            {
                ENTER();
            }
            
            template < class Callback > void read( Callback callback, ul length = 0, ul offset = 0 )
            {
                auto task = mem::mem().type< Read >( *this, length, offset );
                task->request().assign( callback );
                task->reel( this->reel() );
                pool().add( *task );
            }
            
            template < class Callback > void write( Callback callback, const Data& data, ul offset = 0 )
            {
                auto task = mem::mem().type< Write >( *this, data, offset );
                task->request().assign( callback );
                task->reel( this->reel() );
                pool().add( *task );
            }
            
            virtual void destroy()
            {
                mem::mem().detype< File >( this );            
            }
            
            const fs::File& f() const 
            {
                return m_file;
            }
            
            
            
        private:
            fs::File m_file;
        };
        
        inline File& file( const Data& path )
        {
            auto file = mem::mem().type< File >( path );
            return *file;
        }
    }
}

#endif