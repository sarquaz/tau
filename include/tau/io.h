#ifndef _TAU_IO_H_
#define _TAU_IO_H_

#include "../../src/trace.h"

namespace tau
{
    extern ev::Loop& loop();
    extern th::Pool& pool();
    
        
    namespace io
    { 
        class Thread: public os::Thread, public Reel
        {
        protected:
            Thread()
                : os::Thread()
            {
                
            }
            
            virtual ~Thread()
            {
                
            }
        };
    }
    
    extern io::Thread& thread();
    
    namespace io
    {
        class Result: public Reel
        {
        public:
            virtual ~Result()
            {
                
            }
            
            virtual void event( ui, ev::Request& ) = 0;
            
        protected:
            Result()
            {
                
            }
        };
        
        class Event: public ev::Request::Parent
        {
            public:
                enum
                {
                    Error = __LINE__
                };
                
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
                    
                    loop().add( *request );
                }
                
                virtual void callback( ev::Request& request );
                
                virtual void on( ev::Request& request )
                {
                    try
                    {
                        perform( request );
                    }
                    catch ( tau::Error* e )
                    {
                        request.error( e );
                        result().event( Error, request );
                        request.deref();
                    }
                        
                }
                
                virtual void after(  )
                {
                    
                }
                
                Result& result( ) 
                {
                    return m_result;
                }
                
            private:
                virtual void perform( ev::Request& request ) 
                {
                    
                }
                
            private:
                Result& m_result;
                
        };
        
        class Timer: public Event
        {
        public:    
            enum 
            {
                Fire = __LINE__        
            };
                
            Timer( Result& result, const Options& options )
                : Event( result ), m_time( options.def( options::Msec, 0 ), options.def( options::Usec, 0 ) ),
                m_repeat( options.def( options::Repeat, false ) ), m_custom( NULL )
            {
                ENTER();    
            }
            
            Timer( Result& result, const Time& time )
                : Event( result ), m_time( time ), m_repeat( false ), m_custom( NULL )
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
        
            virtual void destroy()
            {
                ENTER();
                mem::mem().detype< Timer >( this ); 
            }
            
            virtual void on( ev::Request& request )
            {
                result().event( Fire, request );
            }
            
            bool repeat() const
            {
                return m_repeat;
            }
            
            void* custom() const
            {
                return m_custom;
            }
            
            void* custom( void* custom )
            {
                m_custom = custom;
                return m_custom;
            }
        
        
        private:
            Time m_time;
            bool m_repeat;
            void* m_custom;
        };
    
        inline Timer& timer( Result& result, const Options& options = {} )
        {
            auto timer = mem::mem().type< Timer >( result, options );
            timer->request();        
            return *timer;
        }
        
        inline Timer& timer( Result& result, const Time& time )
        {
            auto timer = mem::mem().type< Timer >( result, time );
            timer->request();        
            return *timer;
        }
        
        class Process: public Event
        {
        public:
            enum 
            {
                Read = __LINE__,
                Write,
                Exit,
                Error
            };
                        
            Process( Result& result, const Data& command ); 
            
            virtual ~Process()
            {
                ENTER();
                ev::Request::Parent::clear();
            }
            
            const os::Process& process() const
            {
                return m_process;
            }
            
            Process& write( const Data& data )
            {
                event( &m_process.stream( sys::In ), &data );
                return *this;
            }
            
            
                        
            virtual void destroy()
            {
                mem::mem().detype< Process >( this );            
            }
            
            virtual void perform( ev::Request& );
                
            
        private:
            Process& read();
            void event( os::Process::Stream* stream = NULL, const Data* data = NULL );            

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
                Read = __LINE__,
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
                    
                virtual void operator()( );
                virtual void complete( ev::Request& request )
                {
                    m_file.result().event( m_type, request );
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
            class Lookup: public th::Pool::Task
            {
            public:
                Lookup( Net& net )
                    : m_net( net )
                {
                    ENTER();
                    
                    m_net.ref();
                }
                
                virtual ~Lookup()
                {
                    ENTER();
                    m_net.deref();
                }
                
                virtual void operator()();

                virtual void destroy()
                {
                    mem::mem().detype< Lookup >( this );
                }
                
                virtual void complete( ev::Request& );
                                
            private:
                Net& m_net;
            };
            
            friend class Lookup;
            
        public:
            enum
            {
                Error = __LINE__,
                Accept,
                Close,
                Read,
                Write,
                Listen
            };
            
            Net( Result& result, const Options& options, const Data& host );
            Net( Result& result, fs::Link::Accept& accept );
            
            virtual ~Net()
            {
                ENTER();
                ev::Request::Parent::clear();
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
            
            bool remote() const
            {
                return m_remote;
            }
            
        private:
            
            void start();
            
            virtual void perform( ev::Request& );
            
            ev::Request& event( ui what, tau::Error* error = NULL );
            
            
        public:            
            Net& write( const Data& what = Data() );
            
        private:
            Data m_host; 
            ui m_port;
            fs::Link::Type m_type;
            fs::Link m_link;
            Data m_write;
            bool m_connected;
            bool m_server;
            bool m_remote;
        };
        
        inline Net& net( Result& result, const Options& options, const Data& host = Data() )
        {
            auto net = mem::mem().type< Net >( result, options, host );
            return *net;
        }
        
    }
}

#endif  