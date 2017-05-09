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
                    
                    ev::Request::Parent::add( *request );
                    loop().add( *request );
                }
                
                virtual void callback( ev::Request& request )
                {
                    ENTER();

                    ref();
                    
                    on( request );
                    after( );
                    deref();
                }
                    

                virtual void on( ev::Request&  )
                {
                    
                }
                
                virtual void after(  )
                {
                    
                }
                
                Result& result( ) 
                {
                    return m_result;
                }
                
            private:
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
        
        class File: public Event
        {
        public:
            
            enum
            {
                Read,
                Write
            };
            
            class Task: public th::Pool::Task
            {
            public:
                virtual ~Task()
                {
                    ENTER();
                    m_file.deref();
                }
                
                Task( File& file, const Data data, ul offset  )
                    : th::Pool::Task( &file ), m_file( file ), m_offset( offset ), m_length( 0 ), m_type( File::Write )
                {
                    file.ref();    
                    request().data() = data;
                }
                
                Task( File& file, ul length, ul offset  )
                    : th::Pool::Task( &file ), m_file( file ), m_offset( offset ), m_length( length ), m_type( File::Read )
                {
                    file.ref();    
                }
                    
                virtual void operator()( )
                {
                    ENTER();
                    
                    try
                    {
                        if ( m_type == File::Read )
                        {
                            if ( !m_length )
                            {
                                auto info = fs::info( m_file.f().path() );
                                m_length = info.size();    
                            }
                        
                            m_file.f().read( request().data(), m_length, m_offset );    
                        }
                        else
                        {
                            m_file.f().write( request().data(), m_offset );    
                        }
                        
                    }
                    catch ( Error* e )
                    {
                        request().error( e );
                    }
                }
                
                
                virtual void complete( ev::Request& request )
                {
                    ENTER();
                    
                    m_file.result()( m_type, request );
                }
                
                virtual void destroy()
                {
                    mem::mem().detype< Task >( this );
                }
                
            private:
                File& m_file;
                ul m_offset;
                ul m_length;
                ui m_type;
            };
            
            File( Result& result, const Data& path )
                : Event( result ), m_file( fs::File::open( path ) )
            {
                ENTER();
            }
            
            virtual ~File()
            {
                ENTER();
            }
            
            File& read( ul length = 0, ul offset = 0 )
            {
                auto task = mem::mem().type< Task >( *this, length, offset );
                pool().add( *task );
                return *this;
            }
            
            File& write( const Data& data, ul offset = 0 )
            {
                auto task = mem::mem().type< Task >( *this, data, offset );
                pool().add( *task );
                return *this;
            }
            
            virtual void destroy()
            {
                mem::mem().detype< File >( this );            
            }
            
            fs::File& f() 
            {
                return m_file;
            }
            
        private:
            fs::File m_file;
        };
        
        inline File& file( Result& result, const Data& path )
        {
            auto file = mem::mem().type< File >( result, path );
            return *file;
        }
        
        
        class Net: public Event 
        {
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
                    }
                    catch ( tau::Error* error )
                    {
                        request().error( error );
                    }
                }
                
                virtual void destroy()
                {
                    mem::mem().detype< Connect >( this );
                }
                
                virtual void complete( ev::Request& request )
                {
                    if ( !request.error() )
                    {
                        m_net.start();
                    }
                    else
                    {
                        m_net.error( request );
                    }
                }
                
                Net& net()
                {
                    return m_net;
                }
                
            private:
                Net& m_net;
            };
            
            friend class Connect;
            
        public:
            enum
            {
                Error,
                Accept,
                Read,
                Write
            };
            
            Net( Result& result, const Options& options, const Data& host = Data() )
                : Event( result ), m_host( host.empty() ? "localhost" : host ), m_server( options.def( options::Server, false ) ), 
                m_port( options.def( options::Port, 0 ) ), m_type( ( fs::Link::Type ) options.def( options::Type, fs::Link::Tcp ) ), m_connected( false )
            {
                ENTER();         
                TRACE( "host %s", host.c() );
                    
                //
                //  connect
                //         
                auto connect = mem::mem().type< Connect >( *this );
                pool().add( *connect );
            }
            
            Net( Result& result, fs::Link::Accept& accept )
                : Event( result ),  m_server( false ), m_port( accept.address.port() ), m_host( accept.address.host ), m_type( accept.address.type ), m_connected( true )
            {
                ENTER();
                m_link.assign( accept.fd );
                m_link.address() = accept.address;
                
                start();
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
            
            void start()
            {
                ENTER();
                
                if ( !m_server ) 
                {
                    if ( !m_connected )
                    {
                        m_link.connect();    
                    }
                }
                else
                {
                    m_link.listen();
                }
                
                TRACE( "link error %d link fd %d", m_link.error(), m_link.fd() );
                
                TRACE( "creating request", "" );
                
                //
                //  check for readability
                //  
                auto request = mem::mem().type< ev::Request >( *this );
                request->event().type = ev::Loop::Event::Read;
                request->event().fd = m_link.fd();
                request->file() = &m_link;
                
                this->request( request );
                
                //
                //  check for writeability
                //
                write();
            }
            
            virtual void on( ev::Request& request )
            {
                ENTER();
                TRACE( "server: %d, connected: %d event %s", m_server, m_connected, request.event().type == ev::Loop::Event::Read ? "read" : "write" );
                
                
                if ( request.event().type == ev::Loop::Event::Read )
                {
                    if ( !m_server )
                    {
                        m_link.read( request.data() );
                        result()( Read, request );
                    }
                    else
                    {
                        auto accept = m_link.accept();
                        auto net = mem::mem().type< Net >( result(), accept );
                        request.custom( &accept.address ); 
                        result()( Accept, request );
                    }
                }
                
                if ( request.event().type == ev::Loop::Event::Write )
                {
                    if ( !m_connected )
                    {
                        m_connected = true;
                        if ( !m_write.empty() )
                        {
                            write( m_write );
                            m_write.clear();
                        }
                    }
                    
                    if ( !request.data().empty() )
                    {
                        m_link.write( request.data() );
                        result()( Write, request );
                    }
                    
                    request.deref();
                    
                }
            }
            
            void error( ev::Request& request )
            {
                ENTER();
                result()( Error, request );    
            }            
            
        public:            
            Net& write( const Data& what = Data() )
            {
                ENTER();
                
                if ( !m_connected && !what.empty() )
                {
                    m_write.add( what );
                    return *this;
                }

                auto request = mem::mem().type< ev::Request >( *this );
                
                if ( !what.empty() )
                {
                    request->data() = what;
                }
            
                
                request->event().type = ev::Loop::Event::Write;
                request->event().fd = m_link.fd();
                
                this->request( request );
                return *this;
            }
                        
        private:
            Data m_host; 
            ui m_port;
            bool m_server;
            fs::Link::Type m_type;
            fs::Link m_link;
            bool m_connected;
            Data m_write;
        };
        
        inline Net& net( Result& result, const Options& options )
        {
            auto net = mem::mem().type< Net >( result, options );
            return *net;
        }
        
    }
}

#endif  