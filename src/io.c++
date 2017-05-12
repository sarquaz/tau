#include "tau.h"

namespace tau
{
    namespace io
    {
        void Event::callback( ev::Request& request )
        {
            ENTER();
            ref();
            on( request );
            after( );
            deref();
        }
        
        Process::Process( Result& result, const Data& command ) 
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
                        
        void Process::on( ev::Request& request )
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
            
            result().event( event, request );
            if ( event == Write )
            {
                request.deref();
            }
        }
        
        void Process::event( os::Process::Stream& stream, const Data* data )
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
        
        void File::Task::operator()( )
        {
            ENTER();
            
            auto& file = m_file.f();
            
            try
            {
                if ( m_type == File::Read )
                {
                    if ( !m_length )
                    {
                        auto info = fs::info( file.path() );
                        m_length = info.size();    
                    }
                
                    file.read( request().data(), m_length, m_offset );    
                }
                else
                {
                    file.write( request().data(), m_offset );    
                }
                
            }
            catch ( Error* e )
            {
                request().error( e );
            }
        }
        
        void Net::Lookup::operator()()
        {
            ENTER();
            
            try
            {
                //
                //  lookup hostname
                //
                auto address = fs::Link::Lookup()( m_net.host() );
                TRACE( "looked up %s", m_net.host().c() );
                //
                //  set port and type
                //  
                address.port( m_net.port() );
                address.type = m_net.type(); 
                address.host = m_net.host();
                    
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
        
        void Net::Lookup::complete( ev::Request& request )
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
        
        Net::Net( Result& result, const Options& options, const Data& host )
            : Event( result ), m_host( host.empty() ? "localhost" : host ), m_server( options.def( options::Server, false ) ), 
            m_port( options.def( options::Port, 0 ) ), m_type( ( fs::Link::Type ) options.def( options::Type, fs::Link::Tcp ) ), m_connected( false )
        {
            ENTER();         
            TRACE( "host %s", m_host.c() );
                
            //
            //  lookup hostname 
            //         
            pool().add( *( mem::mem().type< Lookup >( *this ) ) );
        }
        
        Net::Net( Result& result, fs::Link::Accept& accept )
            : Event( result ),  m_server( false ), m_port( accept.address.port() ), m_host( accept.address.host ), m_type( accept.address.type ), m_connected( true )
        {
            ENTER();
            
            m_link.assign( accept.fd );
            m_link.address() = accept.address;
            
            start();
        }
        
        void Net::start()
        {
            ENTER();
            
            auto request = mem::mem().type< ev::Request >( *this );
            request->event().type = ev::Loop::Event::Read;
            request->event().fd = m_link.fd();
            request->file() = &m_link;
            
            try
            {
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
            }
            catch ( tau::Error* e )
            {
                request->error( e );
                result().event( Error, *request );
                request->deref();
                return;
            }
                        
            //
            //  check for readability
            //  
            this->request( request );
            
            //
            //  check for writeability
            //
            write();
        }
        
        void Net::on( ev::Request& request )
        {
            ENTER();
            TRACE( "server: %d, connected: %d event %s", m_server, m_connected, request.event().type == ev::Loop::Event::Read ? "read" : "write" );
            
            try
            {
                perform( request );
            }
            catch ( tau::Error* e )
            {
                request.error( e );
                result().event( Error, request );
            }
            
        }
        
        void Net::perform( ev::Request& request )
        {
            if ( request.event().type == ev::Loop::Event::Read )
            {
                auto event = Read;
                if ( !m_server )
                {
                    if ( !m_link.read( request.data() ) )
                    {
                        event = Close;
                    }
                    
                    result().event( event, request );
                }
                else
                {
                    auto accept = m_link.accept();
                    auto net = mem::mem().type< Net >( result(), accept );
                    request.custom( &accept.address ); 
                    result().event( Accept, request );
                }
                
                
                if ( event == Close )
                {
                    request.deref();
                }
            }
            
            if ( request.event().type == ev::Loop::Event::Write )
            {
                if ( !m_connected )
                {
                    m_connected = true;
                }
                
                if ( request.custom() )
                {
                    TRACE( "closing connection", "" );
                    m_link.shutdown();
                    result().event( Close, request );
                }
                
                TRACE( "connected %d write length %u", m_connected, m_write.length() );
                
                if ( !m_write.empty() )
                {
                    request.data() = m_write;
                    m_write.clear();
                }
                
                if ( !request.data().empty() )
                {
                    m_link.write( request.data() );
                    result().event( Write, request );
                }
                
                request.deref();
            }
        }
        
        Net& Net::write( const Data& what )
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
        
        Net& Net::close( )
        {
            ENTER();
            
            if ( !m_connected || m_server )
            {
                return *this;
            }

            auto request = mem::mem().type< ev::Request >( *this );
            
            request->event().type = ev::Loop::Event::Write;
            request->event().fd = m_link.fd();
            request->custom( ( void* ) Close  );
            
            this->request( request );
            return *this;
        }
    }
}
    