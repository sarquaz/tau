#include "base.h"
#include "liner.h"
#include "trace.h"

namespace tau
{
    namespace io
    {
        Till& Till::get( Type type, in::Female* female )
        {
            Till& till =  tok< Till >()();
            till.setType( type );
            if ( female )
            {
                till.females().add( *female );
            }
            
            return till;
        }
        
        Till::Till( )
        : m_mapped( NULL ), m_type( In ), m_last( 0 ), m_target( NULL )
        {
            in::Female::handler( Till::Write, ( Till::Handler ) &Till::tillWrite );
        }
        void Till::map( Till& till )
        {
            if ( m_data.length() || m_mapped )
            {
                throw Error( "cannot map not empty buffer" );
            }
            
            setMapped( &till );
            till.setTarget( this );
            
            if ( till.data().length() )
            {
                till.add( 0 );
            }
        }
        
        void Till::cleanup()
        {
            m_data.clear();
            in::Female::clear();
            setMapped( NULL );
            setTarget( NULL );
            in::Male::females().clear();
        }
        
        void Till::add( const Data& data )
        {
            m_last = data.length();
            m_data.add( data );
            
            if ( target() || type() == Out )
            {
                in::Male::dispatch( Write, *this );
            }
        }
        void Till::add( Till& till )
        {
            ENTER();
            
            if ( !m_mapped )
            {
                map( till );
            }
            else
            {
                m_tills.add( &till );
            }
        }
        bool Till::next()
        {
            if ( mapped( ) && mapped( )->type( ) == Inout )
            {
                setMapped( NULL );
            }
            
            try
            {
                map( *m_tills.get() );
                return true;
            }
            catch ( ... )
            {
            }
            
            return false;
        }
        
        Data Till::space( unsigned int  length )
        {
            m_data.space( length );
            return Data( m_data.target(), length );
        }
        
        Data Till::read( unsigned int length )
        {
            const char* last = data();
            if ( !length )
            {
                length = data().length();
            }
            
            m_data.setOffset( data().offset() + length );
            return Data( last, length );
        }
        
        void Till::setMapped( Till* till )
        {
            if ( !till )
            {
                if ( m_mapped )
                {
                    m_mapped->setTarget( NULL );
                    in::Female::unmale( *m_mapped );

                    if ( m_data.external( ) )
                    {
                        m_data.clear( );
                    }
                }
            }
            else
            {
                in::Female::male( *till );
            }                
            m_mapped = till;
        }
        
        void Till::tillWrite( Grain& grain )
        {
            
            auto& till = dynamic_cast< Till& >( grain );
            
            if ( !m_data.length() )
            {
                m_data = till.data();
            }
            else
            {
                m_data.inc( till.last() );
            }
            
            dispatch( Write, *this );
        }
        
        void Set::Path::clear(  )
        {
            if ( _till )
            {
                till().females().remove( set() );
                till().deref();
                _till = NULL;
            }
        }
        
        Till& Set::Path::till( )
        {
            if ( !_till )
            {
                _till = &Till::get( type, &set() );
            }
            
            return *_till;
        }
  
        Set::Set()
        {
            ENTER();
            
            li::cycle< Type >( { In, Out  } )( [ & ] ( Type type ) { m_paths[ type ].assign( *this, type ); } );
            in::Female::handler( Till::Write, ( Set::Handler ) &Set::tillWrite );
        }
        
        const gen::Generators& Set::populate( )
        {
            gen::add( type( ), ( gen::Generator ) & Bot::create, "process" );
            gen::add( type( ), ( gen::Generator ) & Net::create, "net" );
        //    gen::add( type( ), ( gen::Generator ) & File::create, "file" );

            return gen::generators( type( ) );
        }
        
        unsigned int Client::doRead()
        {
            ENTER();
            
            unsigned int read = 0;
                
            auto data = set().out( ).space( file().available( ) );
            if ( data.length( ) )
            {
                read = file().read( data );
                set().onData( );
            }

            return read;
        }
        
        unsigned int Client::doWrite()
        {
            ENTER();
            auto wrote = file().write( set().in().data() );
            set().in().next();
        
            return wrote;
        }
        
        Net::Net( )
        : m_parent( NULL ), ev::Link( dynamic_cast< ev::File::Client& > ( *this ) ), m_write( NULL ), m_timeout( 10000 )
        {
            in::Female::handler( tau::Event::Timeout, ( in::Female::Handler ) &Net::timer );
            io::Client::setSet( *this );
        }
        
        void Net::onStart( const Set::Options& options )
        {
            ENTER();
            
            auto& host = options[ "host" ];
            auto type = si::Link::type( options[ "type" ] );
            auto port = 0;
            
            if ( type != si::Link::Local )
            {
                port = options.number( "port" );
            }
            
            m_server = options.get( "mode", "client" ) == "server";
            
            TRACE("server: %d, host: %s, port: %d, type %d", server(), ( const char* ) host, port, type );
            
            if ( type == si::Link::Local )
            {
                m_link.open( si::Link::Address( type, host ) );
            }
            else
            {
                auto address = si::Link::Lookup( type )( host );
                address.setPort( port );
                link().open( address );                
            }
            
            open();
        }
        
        void Net::open( si::Link::Accept* accept )
        {
            ENTER();
            if ( accept )
            {
                link().assign( accept->fd );
                link().address() = accept->address;
                return;
            }
            
            if ( server() )
            {
                io::Client::read();
                link().listen();
            }
            else
            {
                io::Client::write();
                if ( !link().udp() )
                {
                    link().connect();
                }
            }
        }
        
        
        void Net::onError( const tau::Error& error, bool sync )
        {
            ENTER( );
            if ( sync )
            {
                throw error;
            }
            else
            {
                Set::setError( error );
            }
        }
        
        void Net::after( )
        {
            ENTER();
            
            m_parent = NULL;
            resetTimer();
            
            if ( server() && link().local() )
            {
                si::File::remove( link().address().host );
            }
            
            ev::Link::close();
        }

        void Net::timer( Grain& )
        {
            ENTER();
            setError( Error::Tau( "write timed out" ) );
        }
        
        void Net::resetTimer()
        {
            ENTER();
            if ( m_write )
            {
                m_write->deref();
                m_write = NULL;
            }
        }
        
        unsigned int Net::doRead( )
        {
            ENTER( );
                        
            if ( !server() )
            {                
                return io::Client::doRead();
            }
            else
            {
                accept();
                return 1;
            }
        }
        
        void Net::accept( )
        {
            ENTER();

            auto accept = link().accept( );
            auto& net = *dynamic_cast < Net* > ( Net::create( ) );
            
            net.open( &accept );
            net.setParent( this );
            
            in::Male::dispatch( Accept, net );
            net.deref();
        }
        
        void Net::inWrite(  )
        {
            ENTER();
            
            if ( !m_write  )
            {
                m_write =  &tau::event( this )( m_timeout );      
            }
            
            Set::inWrite( );
        }
        
        void Net::onWrite()
        {
            ENTER();
            resetTimer();
            Set::onWrote();
        }

        void Bot::Stream::onRead( )
        {
            ENTER( );            

            if ( type() == Err )
            {
                tau::Error error;
                error.message.add( process().in().data() );
                process().setError( error );
            }
            else
            {
                process().onData();
            }
        }

        void Bot::Stream::onWrite( )
        {
            ENTER( );
            ev::File::close();
            process().onWrote();
        }
        
        void Bot::Stream::assign( Bot& bot, const si::Process::Stream& stream )
        {
            m_bot = &bot;
            m_type = stream.type;
            ev::File::target().assign( stream.fd );
            io::Client::setSet( bot );
        }

        Bot::Bot( )
        : m_code( 0 ), m_timer( NULL )
        {
            in::Female::handler( Event::Timeout, ( Bot::Handler ) &Bot::timer );
        }

        void Bot::onStart( const Set::Options& options )
        {
            ENTER();
            
            m_process.start( command() );
            m_process.streams( [ & ] ( const si::Process::Stream& s ) 
            {
                auto type = s.type;
                auto& stream = m_streams[ type ];
                
                stream.assign( *this, s );
                if ( type > Stream::In )
                {
                    stream.read();
                }
            } );
            
            m_timer = &tau::event( this )( Time( 0, 1000 ) );
        }
        
        void Bot::timer( Grain& grain )
        {
            auto& timer = dynamic_cast< Event& >( grain );

            try
            {
                m_code = m_process.code();
                TRACE( "child process exited with code %d", m_code );
                timer.deref();
                m_timer = NULL;
                
                Set::send( Set::End );
            }

            catch ( ... )
            {
            }
        }
        
        int Bot::status( ) const
        {
            if ( m_timer )
            {
                throw tau::Error();
            }
            
            return m_code;
        }
        void Bot::after( )
        {
            ENTER( );
            
            if ( m_timer )
            {
                m_timer->deref();
                m_timer = NULL;
            }
                       
            m_process.stop();   
            for ( auto i = 0; i < 0; i++ )
            {
                m_streams[ i ].close();
            }
            
            m_code = 0;
        }

        
//        File& File::get( const si::Data& name )
//        {
//            File& file = *dynamic_cast < File* > ( create( ) );
//            file.start( { { "name", name } } );
//            return file;
//        }
//        
//        void File::onStart( const Set::Options& options )
//        {
//            ENTER();
//            
//            ev::File::open( si::File::open( name() ) );
//            try
//            {
//                m_size = si::File::size( name() );
//            }
//            catch ( ... )
//            {
//                
//            }
//            
//            TRACE( "file name %s size %d", ( const char* ) name(),  m_size );
//        }
        
//        File::File( )
//        : ev::File( dynamic_cast < Set* > ( this ) )
//        {
//            in::Female::handler( Event::Timeout, ( in::Female::Handler ) &File::onTimer );
//        }
//        
//        unsigned int File::doRead( )
//        {
//            auto read = ev::File::doRead( );
//            m_read += read;
//            if ( m_read == m_length )
//            {
//                ev::File::current().event().disable();
//                Set::send( Read );
//                m_read = 0;
//            }
//            else
//            {
//                event( this )( Ticks( 0, 1000 ) );
//            }
//            
//            return read;
//        }
//        void File::read( unsigned int length )
//        {
//            if ( length > m_size )
//            {
//                throw tau::Error( "cannot read %d bytes", length );
//            }
//            
//            m_length = length;
//            ev::File::process( ev::In );
//        }
//        
//        unsigned int File::available( )
//        {
//            auto max = ev::File::available( );
//            auto length = m_length - m_read;
//            if ( !length )
//            {
//                length = max;
//            }
//            
//            TRACE( "need to read %d bytes", max );
//
//            return MIN( max, length );
//        }
//        
//        void File::onTimer( Grain& grain )
//        {
//            ( dynamic_cast< Event& >( grain ) ).deref();
//            read( m_length );
//        }
        
        
    }
} 