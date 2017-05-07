#ifndef _TAU_IO_H_
#define _TAU_IO_H_

#include "../../src/trace.h"

namespace tau
{
    extern ev::Loop& loop();
    extern th::Pool& pool();
    
    
    namespace io
    {
        class Result: public Reel
        {
        public:
            virtual ~Result()
            {
                
            }
            
            virtual void operator()( ui, ev::Request& ) = 0;
            
        protected:
            Result()
            {
                
            }
        };
        
        class Event: public ev::Request::Parent
        {
            public:
                Event( Result& result )
                    : m_result( result )
                {
                    m_result.ref();
                }

                virtual ~Event()
                {
                    ENTER();
                    m_requests.all( [ ] ( ev::Request* request ) { request->deref(); } );
                    m_result.deref();
                }

                virtual void destroy()
                {
                    ENTER();
                    mem::mem().detype< Event >( this );
                }
                
                void request( ev::Request* request = NULL )
                {
                    ENTER();
                    
                    if ( !request )
                    {
                        request = mem::mem().type< ev::Request >( *this );
                    }
                    
                    m_requests.append( request );
                    loop().add( *request );
                }
                
                virtual void callback( ev::Request& request )
                {
                    ENTER();

                    ref();
                    
                    on( request );
                    after();
                    deref();
                }
                    

                virtual void on( ev::Request&  )
                {
                    
                }
                
                virtual void after(  )
                {
                    
                }
                
                
            protected:
                Result& result( ) 
                {
                    return m_result;
                }
                
            private:
                li::List< ev::Request* > m_requests;
                Result& m_result;
                
        };
        
        class Timer: public Event
        {
        public:    
            enum
            {
                Fire        
            };
                
            Timer( Result& result, const Options& options )
                : Event( result ), m_time( options.def( options::Msec, 0 ), options.def( options::Usec, 0 ) ),
                m_repeat( options.def( options::Repeat, false ) )
            {
                ENTER();    
            }
        
            virtual ~Timer()
            {
                ENTER();
            }
        
            virtual void before( ev::Request& request )
            {
                ENTER();
                request.event().time = m_time;  
                if ( m_repeat && m_time.value > 0 )
                {
                    request.event().type = ev::Loop::Event::Timer;        
                }
            }
        
            virtual void after()
            {
                ENTER();
            
                if ( !m_repeat )
                {
                    deref();
                }
            }
        
            virtual void destroy()
            {
                ENTER();
                mem::mem().detype< Timer >( this ); 
            }
            
            virtual void on( ev::Request& request )
            {
                result()( Fire, request );
            }
        
        
        private:
            Time m_time;
            bool m_repeat;
        };
    
        inline Timer& timer( Result& result, const Options& options = {} )
        {
            auto timer = mem::mem().type< Timer >( result, options );
            timer->request();        
            return *timer;
        }
        
        class Process: public Event
        {
        public:
            enum
            {
                Read,
                Write,
                Error
            };
                        
            Process( Result& result, const Data& command ) 
                : Event( result ), m_command( command )
            {
                ENTER();
                
                m_process.start( m_command );            
                m_process.streams([ & ] ( os::Process::Stream& s )
                    {
                        if ( s.type() != out::In )
                        {
                            event( s );
                        }
                    } );
            }
            
            virtual ~Process()
            {
                ENTER();
            }
            
            const os::Process& process() const
            {
                return m_process;
            }
            
            Process& write( const Data& data )
            {
                event( m_process.stream( out::In ), &data );
                return *this;
            }
                        
            virtual void destroy()
            {
                mem::mem().detype< Process >( this );            
            }
            
            virtual void on( ev::Request& request )
            {
                ENTER();
                
                auto& stream = reinterpret_cast< os::Process::Stream& > ( *request.file() );
                auto event = Read;
                
                if ( request.event().type == ev::Loop::Event::Read )
                {
                    try
                    {
                        stream.read( request.data() );
                    }
                    catch ( tau::Error* e )
                    {
                        request.error( e );
                    }
                    
                    if ( stream.type() == out::Err )
                    {
                        event = Error;
                    }
                }
                else
                {
                    try
                    {
                        stream.write( request.data() );    
                    }
                    catch ( tau::Error* e )
                    {
                        request.error( e );
                    }
                    
                    event = Write;
                }
                
                result()( event, request );
            }
            
        private:
            void event( os::Process::Stream& stream, const Data* data = NULL )
            {
                
                auto request = mem::mem().type< ev::Request >( *this );
                request->event().type = ( stream.type() != out::In ) ? ev::Loop::Event::Read : ev::Loop::Event::Write;
                request->event().fd = stream.fd();
                request->file() = &stream;
                
                if ( data )
                {   
                    request->data() = *data;
                }
                
                this->request( request );                
            }
            
            
            
        private:
            os::Process m_process;
            Data m_command;
        };
        
        inline Process& process( Result& result, const Data& command )
        {
            auto process = mem::mem().type< Process >( result, command );         
            return *process;
        }
        
        /*       
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
                    
                    virtual void after()
                    {
                        ENTER();
                        deref();
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
        
        class Net: public Event 
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
            
            const Data& host() const
            {
                return m_host;
            }
            
            fs::Link::Type type() const
            {
                return m_type;
            }
            
            bool server() const
            {
                return m_server;
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
                    TRACE( "net 0x%x", &net );
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
                    TRACE( "context 0x%x", &context );
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
                    ENTER();
                    
                    try
                    {
                        net().link().write( context().write );    
                    }
                    catch ( Error* e )
                    {
                        TRACE( "error 0x%x", e );
                        request.error( e );
                    }
                }
                
                virtual void configure( ev::Loop::Event& event )
                {
                    ENTER();
                    
                    TRACE( "net 0x%x", &net() );
                    
                    
                    event.type = ev::Loop::Event::Write;
                    event.fd = net().link().fd();    
                }
            };
            
        
            class Connect: public th::Pool::Task
            {
            public:
                Connect( Net& net )
                    : m_net( net )
                {
                    ENTER();
                    TRACE( "net 0x%x", &m_net );
                    
                    m_net.ref();
                }
                
                virtual ~Connect()
                {
                    ENTER();
                    m_net.deref();
                }
                
                virtual void operator()()
                {
                    ENTER();
                    
                    try
                    {
                        //
                        //  lookup hostname
                        //
                        auto address = fs::Link::Lookup()( m_net.host() );
                        //
                        //  set port and type
                        //  
                        address.port( m_net.port() );
                        address.type = m_net.type(); 
                        //                             
                        //  create socket
                        //                                
                        m_net.link().open( address );        
                        //
                        //  call bind or connect
                        //
                        if ( !m_net.server() )
                        {
                            m_net.link().connect();    
                        }
                        else
                        {
                            m_net.link().bind();
                        }
                        
                        //
                        //  set non blocking
                        //
                        m_net.link().nb();

                    }
                    catch ( Error* error )
                    {
                        request().error( error );
                    }
                }
                
                virtual void destroy()
                {
                    mem::mem().detype< Connect >( this );
                }
                
                Net& net()
                {
                    return m_net;
                }
                
            private:
                Net& m_net;
            };
            
        public:
            template < class Callback > Net& read( Callback callback )
            {
                ENTER();
                auto context = mem::mem().type< Context >( *this );
                return start< Read >( callback, *context );
            }
            
            template < class Callback > Net& write( Callback callback, const Data& what )
            {
                ENTER();
                
                auto context = mem::mem().type< Context >( *this );
                context->write = what;
                return start< Write >( callback, *context );
            }
            
        private:
            
            template < class Callback > Connect& connect( Callback callback, Context& context )
            {
                ENTER();
                
                auto connect = mem::mem().type< Connect >( *this );
                connect->reel( &context );
                //context.setfirst( callback );
                
                connect->request().assign( [ & ] ( ev::Request& request ) 
                        {
                        callback( request );    
                    } );
                    
                pool().add( *connect );
                return *connect;
            }
            
            template < class What, class Callback  > Net& start( Callback callback, Context& context )
            {
                ENTER();
                
                TRACE( "callback 0x%x", &callback ); 
                
                if ( !m_link.fd() )
                {
                    //context.setsecond( callback );
                    
                    // lookup( []( ev::Request& request )
                    //     {
                    //         auto context = reinterpret_cast< Context* >( request.parent().reel() );
                    //
                    //         if ( !request.error() )
                    //         {
                    //
                    //
                    //             auto what = mem::mem().type< What >( *context );
                    //             what->reel( &context->net );
                    //             what->request( []( ev::Request& request )
                    //                 {
                    //                     
                    //                     context->second()( request );
                    //                 } ).deref();
                    //         }
                    //         else
                    //         {
                    //             context->second()( request );
                    //         }
                    //
                    //     }, context );
                    
                    connect( [ & ] ( ev::Request& request ) 
                        {
                             TRACE( " connect callback, request 0x%x", &request ); 
                             auto context = reinterpret_cast< Context* >( request.parent().reel() );
                             auto& connect = reinterpret_cast< Connect& >( request.parent() );
                             auto what = mem::mem().type< What >( *context );
                             TRACE( "callback 0x%x", &callback ); 
                             what->request( callback, NULL, &connect.net() );
                        }, context );
                }
                else
                {
                    // auto what = mem::mem().type< What >( context );
                    // what->reel( &context.net );
                    // 
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
        */
    }
}

#endif  