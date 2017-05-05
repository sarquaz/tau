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
                
                template < class Callback > Event& request( Callback callback, ev::Request* request = NULL )
                {   
                    ref();
                    
                    if ( !request )
                    {
                        request = mem::mem().type< ev::Request >( *this );
                    }
                    
                    request->assign( [ & ] ( ev::Request& request ) 
                        { 
                            TRACE( "event callback", "" );
                            
                            
                            before( request );
                            callback( request );
                            after();
                            deref();
                        } );
                        
                    m_requests.append( request );

                    loop().add( *request );
                    return *this;
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
            
            template < class Callback > File& read( Callback callback, ul length = 0, ul offset = 0 )
            {
                auto task = mem::mem().type< Read >( *this, length, offset );
                task->request().assign( callback );
                task->reel( this->reel() );
                pool().add( *task );
                return *this;
            }
            
            template < class Callback > File& write( Callback callback, const Data& data, ul offset = 0 )
            {
                auto task = mem::mem().type< Write >( *this, data, offset );
                task->request().assign( callback );
                task->reel( this->reel() );
                pool().add( *task );
                return *this;
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
        
        class Net: public Reel
        {
        public:
            Net( const Options& options, const Data& host = Data() )
                : m_host( host.empty() ? "localhost" : host ), m_server( options.def( options::Server, false ) ), 
                m_port( options.def( options::Port, 0 ) ), m_type( ( fs::Link::Type ) options.def( options::Type, fs::Link::Tcp ) )
            {
                ENTER();            
            }
            
            virtual ~Net()
            {
                ENTER();
            }    
            
            
            fs::Link& link()
            {
                return m_link;
            }
            
            ui port() const
            {
                return m_port;
            }
            
            fs::Link::Type type() const
            {
                return m_type;
            }
            
            virtual void destroy()
            {
                mem::mem().detype< Net >( this );
            }
            
        private:
            
            struct Context: Reel
            {
                Context( Net& _net )
                    : net( _net ), address( NULL ), _first( NULL ), _second( NULL )
                {
                    ENTER();
                    net.ref();
                }
                
                virtual ~Context()
                {
                    ENTER(); 
                    net.deref();
                    mem::mem().detype< fs::Link::Address >( address );
                    
                    if ( _first )
                    {
                        _first->destroy();
                    }
                    
                    if ( _second )
                    {
                        _second->destroy();
                    }
                    
                    
                }
                
                virtual void destroy()
                {
                    mem::mem().detype< Context >( this );
                }
                
                template < class Callback > void setfirst( Callback callback )
                {
                    _first  = mem::mem().type< si::Callback< Callback, ev::Request& > >( callback );
                }
                
                si::Call< ev::Request& >& first()
                {
                    assert( _first );
                    return *_first;
                }
                
                
                template < class Callback > void setsecond( Callback callback )
                {
                    _second  = mem::mem().type< si::Callback< Callback, ev::Request& > >( callback );
                }
                
                si::Call< ev::Request& >& second()
                {
                    assert( _second );
                    return *_second;
                }
            
                si::Call< ev::Request& >* _first;
                si::Call< ev::Request& >* _second;
                
                Data write;
                fs::Link::Address* address;
                Net& net;
            };
            
            class Event: public io::Event
            {
            public:
                
                virtual ~Event()
                {
                    ENTER();
                    m_context.deref();
                }
                
            protected:
                Event( Context& context )
                    : m_context( context )
                {
                }
                
                Net& net()
                {
                    return m_context.net;
                }
                
                Context& context()
                {
                    return m_context;
                }
                
            private:
                Context& m_context;
            };
            
            class Read: public Event
            {
            public:
                Read( Context& context )
                    : Event( context )
                {
                    ENTER();
                }
                
                virtual ~Read()
                {
                    ENTER();
                }
                
                virtual void destroy()
                {
                    mem::mem().detype< Read >( this );
                }
                
                virtual void before( ev::Request& request )
                {
                    try
                    {
                        net().link().read( request.data() );
                    }
                    catch ( Error* e )
                    {
                        request.error( e );
                    }
                    
                }
            };
            
            class Write: public Event
            {
            public:
                Write( Context& context )
                    : Event( context )
                {
                    ENTER();
                }
                
                virtual ~Write()
                {
                    ENTER();
                }
                
                virtual void destroy()
                {
                    mem::mem().detype< Write >( this );
                }
                
                virtual void before( ev::Request& request )
                {
                    try
                    {
                        net().link().write( context().write );    
                    }
                    catch ( Error* e )
                    {
                        request.error( e );
                    }
                }
            };
            
        
            class Lookup: public th::Pool::Task
            {
            public:
                Lookup( const Data& host )
                    : m_host( host ), m_address( NULL )
                {
                    ENTER();
                }
                
                virtual ~Lookup()
                {
                    ENTER();
                }
                
                virtual void operator()()
                {
                    ENTER();
                    
                    try
                    {
                        auto context = reinterpret_cast< Context* >( this->reel() );
                        context->address = fs::Link::Lookup()( m_host );    
                    }
                    catch ( Error* error )
                    {
                        request().error( error );
                    }
                }
                
                virtual void destroy()
                {
                    mem::mem().detype< Lookup >( this );
                }
                
            private:
                Data m_host;    
                fs::Link::Address* m_address;
            };
            
        public:
            template < class Callback > Net& read( Callback callback )
            {
                ENTER();
                auto context = mem::mem().type< Context >( *this );
                return connect< Read >( callback, *context );
            }
            
            template < class Callback > Net& write( Callback callback, const Data& what )
            {
                ENTER();
                auto context = mem::mem().type< Context >( *this );
                context->write = what;
                return connect< Write >( callback, *context );
            }
            
        private:
            
            template < class Callback > Lookup* lookup( Callback callback, Context& context )
            {
                ENTER();
                
                auto lookup = mem::mem().type< Lookup >( m_host );
                lookup->reel( &context );
                context.setfirst( callback );
                
                lookup->request().assign( [] ( ev::Request& request ) 
                    {
                        auto context = reinterpret_cast< Context* >( request.parent().reel() );
                        context->first()( request );
                    } );
                    
                pool().add( *lookup );
                return lookup;
            }
            
            template < class What, class Callback  > Net& connect( Callback callback, Context& context )
            {
                ENTER();
                
                if ( !m_link.fd() )
                {
                    context.setsecond( callback );
                    
                    lookup( []( ev::Request& request ) 
                        {
                            auto context = reinterpret_cast< Context* >( request.parent().reel() );
                            
                            if ( !request.error() )
                            {
                                context->address->port( context->net.port() );
                                context->address->type = context->net.type(); 
                                                                
                                context->net.link().open( *( context->address ) );        
                                
                                auto what = mem::mem().type< What >( *context );
                                what->reel( &context->net );
                                what->request( []( ev::Request& request ) 
                                    {
                                        auto context = reinterpret_cast< Context* >( request.parent().reel() );
                                        context->second()( request );
                                    } ).deref();
                            }
                            else
                            {
                                context->second()( request );
                            }
                            
                        }, context );
                }
                else
                {
                    auto what = mem::mem().type< What >( context );
                    what->reel( &context.net );
                    what->request( callback ).deref();
                }
                
                return *this;
            }
            
        private:
            Data m_host; 
            ui m_port;
            bool m_server;
            fs::Link::Type m_type;
            fs::Link m_link;
        };
        
        inline Net& net( const Options& options )
        {
            auto net = mem::mem().type< Net >( options );
            return *net;
        }
    }
}

#endif  