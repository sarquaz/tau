#include "tau.h"

namespace tau
{
    namespace io
    {
    //     void Event::callback( ev::Request& request )
    //     {
    //         ENTER();
    //         ref();
    //         on( request );
    //         after( );
    //         deref();
    //     }
    //
    //     Process::Process( Result& result, const Data& command )
    //         : Event( result ), m_command( command )
    //     {
    //         ENTER();
    //
    //         m_process.start( m_command );
    //         read();
    //
    //         //
    //         //  wait for process exit
    //         //
    //         event();
    //     }
    //
    //     void Process::perform( ev::Request& request )
    //     {
    //         ENTER();
    //
    //         auto& stream = reinterpret_cast< os::Process::Stream& > ( *request.file() );
    //         auto event = Read;
    //
    //         if ( request.event().type == ev::Loop::Event::Read )
    //         {
    //             stream.read( request.data() );
    //             request.custom( ( void* ) stream.type() );
    //         }
    //
    //         if ( request.event().type == ev::Loop::Event::Write )
    //         {
    //             stream.write( request.data() );
    //             event = Write;
    //         }
    //
    //         if ( request.event().type == ev::Loop::Event::Process )
    //         {
    //             request.custom( ( void* ) m_process.code() );
    //             event = Exit;
    //         }
    //
    //         result().event( event, request );
    //     }
    //
    //     Process& Process::read()
    //     {
    //         m_process.streams([ & ] ( os::Process::Stream& s )
    //             {
    //                 if ( s.type() != sys::In )
    //                 {
    //                     event( &s );
    //                 }
    //             } );
    //
    //         return *this;
    //     }
    //
    //     void Process::event( os::Process::Stream* stream, const Data* data )
    //     {
    //         auto request = mem::mem().type< ev::Request >( *this );
    //
    //         if ( stream )
    //         {
    //             request->event().type = ( stream->type() != sys::In ) ? ev::Loop::Event::Read : ev::Loop::Event::Write;
    //             request->event().fd = stream->fd();
    //             request->file() = stream;
    //         }
    //         else
    //         {
    //             request->event().type = ev::Loop::Event::Process;
    //             request->event().fd = m_process.pid();
    //         }
    //
    //         if ( data )
    //         {
    //             TRACE( "need to write %s", data->c() );
    //             request->data() = *data;
    //         }
    //
    //         this->request( request );
    //     }
    //
    //     void File::Task::operator()( )
    //     {
    //         ENTER();
    //
    //         auto& file = m_file.f();
    //
    //         try
    //         {
    //             if ( m_type == File::Read )
    //             {
    //                 if ( !m_length )
    //                 {
    //                     auto info = fs::info( file.path() );
    //                     m_length = info.size();
    //                 }
    //
    //                 TRACE( "need to read %u bytes at offset %u", m_length, m_offset );
    //
    //                 file.read( request().data(), m_length, m_offset );
    //             }
    //             else
    //             {
    //                 file.write( request().data(), m_offset );
    //             }
    //
    //         }
    //         catch ( tau::Error* e )
    //         {
    //             request().error( e );
    //         }
    //     }
    //
    //     void Net::Lookup::operator()()
    //     {
    //         ENTER();
    //
    //         try
    //         {
    //             //
    //             //  lookup hostname
    //             //
    //             auto address = fs::Link::Lookup()( m_net.host() );
    //             TRACE( "looked up %s", m_net.host().c() );
    //             //
    //             //  set port and type
    //             //
    //             address.port( m_net.port() );
    //             address.type = m_net.type();
    //             address.host = m_net.host();
    //
    //             //
    //             //  create socket
    //             //
    //             m_net.link().open( address );
    //         }
    //         catch ( tau::Error* error )
    //         {
    //             request().error( error );
    //         }
    //     }
    //
    //     void Net::Lookup::complete( ev::Request& request )
    //     {
    //         if ( !request.error() )
    //         {
    //             m_net.start();
    //         }
    //         else
    //         {
    //             m_net.result().event( Error, request );
    //         }
    //     }
    //
    //     Net::Net( Result& result, const Options& options, const Data& host )
    //         : Event( result ), m_host( host.empty() ? "localhost" : host ), m_server( options.def( options::Server, false ) ),
    //         m_port( options.def( options::Port, 0 ) ), m_type( ( fs::Link::Type ) options.def( options::Type, fs::Link::Tcp ) ), m_connected( false ), m_remote( false )
    //     {
    //         ENTER();
    //
    //         TRACE( "host %s port %d type %s server %d", m_host.c(), m_port, fs::Link::stype( m_type ).c(), m_server );
    //
    //         if ( m_type != fs::Link::Local )
    //         {
    //             //
    //             //  lookup hostname
    //             //
    //             pool().add( *( mem::mem().type< Lookup >( *this ) ) );
    //         }
    //         else
    //         {
    //             start();
    //         }
    //
    //     }
    //
    //     Net::Net( Result& result, fs::Link::Accept& accept )
    //         : Event( result ),  m_server( false ), m_port( accept.address.port() ), m_host( accept.address.host ), m_type( accept.address.type ), m_connected( true ), m_remote( true )
    //     {
    //         ENTER();
    //
    //         m_link.assign( accept.fd );
    //         m_link.address() = accept.address;
    //
    //
    //         start();
    //     }
    //
    //     void Net::start()
    //     {
    //         ENTER();
    //
    //         try
    //         {
    //             if ( type() == fs::Link::Local && !m_remote )
    //             {
    //                 m_link.open( fs::Link::Address( m_type, m_host ) );
    //             }
    //
    //             if ( !m_server )
    //             {
    //                 if ( !m_connected  )
    //                 {
    //                     if ( m_type != fs::Link::Udp )
    //                     {
    //                         m_link.connect();
    //                     }
    //                     else
    //                     {
    //                         m_connected = true;
    //                     }
    //                 }
    //             }
    //             else
    //             {
    //                 m_link.listen();
    //                 auto& request = event( Listen );
    //                 result().event( Listen, request );
    //                 request.deref();
    //             }
    //         }
    //         catch ( tau::Error* e )
    //         {
    //             auto& request = event( Error, e );
    //             result().event( Error, request );
    //             request.deref();
    //             return;
    //         }
    //
    //         //
    //         //  check for readability
    //         //
    //
    //         request( &event( Read ) );
    //
    //         if ( !m_server && !m_remote )
    //         {
    //             //
    //             //  check for writeability
    //             //
    //             write();
    //         }
    //     }
    //
    //     ev::Request& Net::event( ui what, tau::Error* error )
    //     {
    //         auto request = mem::mem().type< ev::Request >( *this );
    //
    //         if ( what == Read || what == Write )
    //         {
    //             request->event().type = what == Read ? ev::Loop::Event::Read : ev::Loop::Event::Write;
    //             request->event().fd = m_link.fd();
    //             request->file() = &m_link;
    //         }
    //
    //         if ( error )
    //         {
    //             request->error( error );
    //         }
    //
    //         return *request;
    //     }
    //
    //     void Net::perform( ev::Request& request )
    //     {
    //         ENTER();
    //         TRACE( "server: %d, remote %d connected: %d event %s", m_server, m_remote, m_connected, request.event().type == ev::Loop::Event::Read ? "read" : "write" );
    //
    //         if ( request.event().type == ev::Loop::Event::Read )
    //         {
    //             auto event = Read;
    //             if ( !m_server || m_type == fs::Link::Udp )
    //             {
    //                 if ( !m_link.read( request.data() ) )
    //                 {
    //                     event = Close;
    //                 }
    //
    //                 result().event( event, request );
    //             }
    //             else
    //             {
    //                 if ( m_type != fs::Link::Udp )
    //                 {
    //                     auto accept = m_link.accept();
    //                     TRACE( "accepted type %d", accept.address.type );
    //                     auto net = mem::mem().type< Net >( result(), accept );
    //                     request.custom( &accept.address );
    //                     result().event( Accept, request );
    //                 }
    //             }
    //
    //             if ( event == Close )
    //             {
    //                 request.deref();
    //             }
    //         }
    //
    //         if ( request.event().type == ev::Loop::Event::Write )
    //         {
    //             if ( !m_connected )
    //             {
    //                 m_connected = true;
    //             }
    //
    //             TRACE( "request 0x%x connected %d write length %u request data length %u", &request, m_connected, m_write.length(), request.data().length() );
    //
    //             if ( !m_write.empty() )
    //             {
    //                 request.data() = m_write;
    //                 m_write.clear();
    //             }
    //
    //             if ( !request.data().empty() )
    //             {
    //                 m_link.write( request.data() );
    //                 result().event( Write, request );
    //             }
    //
    //             request.deref();
    //         }
    //     }
    //
    //     Net& Net::write( const Data& what )
    //     {
    //         ENTER();
    //
    //         if ( !m_connected && !what.empty() )
    //         {
    //             TRACE( "adding to write", "" );
    //             m_write.add( what );
    //             return *this;
    //         }
    //
    //         auto& request = event( Write );
    //
    //         if ( !what.empty() )
    //         {
    //             request.data() = what;
    //         }
    //
    //         this->request( &request );
    //         return *this;
    //     }
    }
}
    