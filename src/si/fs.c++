#include "tau.h"

namespace tau
{
    namespace fs
    {
        void chdir( const Data& path )
        {
            STRACE( "%s", path.c() );
            ::chdir( path );
        }
        
        Data cwd( )
        {
            Data data;
            data.space( 0x200 );
            ::getcwd( data, data.size() );
            
            return data;
        }
        
        std::pair< Data, Data > split( const Data& path )
        {
            std::pair< Data, Data > pair;
            auto offset = 0;
            
            for ( ;; )
            {
                auto found = path.find( "/", offset );
                
                if ( found == -1 )
                {
                    break;
                }
                
                offset = found + 1;
            }
            
            if ( offset )
            {
                pair.first = Data( path, offset - 1 );
                pair.second = Data( path + offset );
            }
            else
            {
                pair.first = path;
            }
             
            return pair;
        }
        
        Info info( const Data& name )
        {
            Info info;
            si::check( ::stat( name, info ) )( "stat" );
            return info;
        }
        
        type::Type Info::type() const
        {
            ENTER();
            
            switch( stat.st_mode & S_IFMT )
            {
                case S_IFDIR:
                    return type::Dir;
                    
                case S_IFREG:
                    return type::File;
                    
            }
            
            return type::File;
        }
        
        bool exists( const Data& name )
        {
            bool exists = false;
            
            try
            {
                info( name );
                exists = true;
            }
            catch( ... )
            {

            }

            return exists;
        }
        
        class File File::open( const Data& name )
        {
            int flags = 0;
            int mode = 0;
            
            
            STRACE( "opening %s", name.c() );
            
            if ( !exists( name ) )
            {
                flags |= O_CREAT;
                mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
                STRACE( "will create new file", "" );
            }
            else
            {
                flags |= O_APPEND;
            }

            flags |= O_RDWR;


            File file( si::check(
                    ::open( name, flags, mode )
                    )( "file open, name %s, flags %d, mode %d", name.c(), flags, mode ) );

            file.m_path = name;
            return file;
        }
        
        unsigned int File::available( ) const
        {
            unsigned int available = 0;
            ::ioctl( fd( ), FIONREAD, &available );

            return available;
        }
        
        unsigned long File::write( const Data& data, long offset ) const
        {
            seek( offset );
            auto wrote = si::check( ::write( fd( ), data, data.length( ) ) )( "write" );
            TRACE( "wrote %d bytes", wrote );
            
            return wrote;
        }
        unsigned long File::read( Data& data, ul length, long offset ) const
        {
            seek( offset );
            
            
            if ( !length ) 
            {
                length = available();
            }

            data.space( length );
            auto read = si::check( ::read( fd( ), data, length ) )( "read" );
            data.length( read );
            
            TRACE( "read %d bytes", read );
            
            return read;
        }   
        
        void File::assign( Handle fd, bool nb  )
        {
            m_fd = fd;
            if ( nb )
            {
                this->nb();
            }
        }
        
        void File::nb() const
        {
            si::check( ::fcntl( m_fd, F_SETFL, ::fcntl( m_fd, F_GETFL, 0 ) | O_NONBLOCK ) )( "fcntl" );
        }

        void File::close( )
        {
            ENTER();
        
            if ( fd( ) )
            {
                ::close( fd( ) );
                m_fd = 0;
            }
        }
        
        void File::seek( long offset ) const
        {
            ENTER()
            TRACE( "offset %u", offset );
                    
            if ( offset != -1 )
            {
                si::check( ::lseek( fd( ), offset, SEEK_SET ) )( "seek" );    
            }    
            
        }
        
        Link::Type Link::type( const Data& key )
        {
            if ( key == "local" )
            {
                return Local;
            }
            
            return key == "tcp" ? Tcp : Udp;
        }
        
        Data Link::stype( Type type )
        {
            Data data;
            
            switch ( type )
            {
                case Local:
                    data.add( "local" );
                    break;
                    
                case Udp:
                    data.add( "udp" );
                    break;
                    
                case Tcp:
                    data.add( "tcp" );
                    break;
            }
            
            return data;
        }
        
        Link::Lookup::Lookup( Type type )
        {
            std::memset( &info, 0, sizeof( info ) );
            info.ai_socktype = type == Link::Tcp ? SOCK_STREAM : SOCK_DGRAM;
            info.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
        }
        
        Link::Address Link::Lookup::operator()( const Data& name )
        {
            Info* info;
            auto result = ::getaddrinfo( name, NULL, *this, &info );
            if ( !result )
            {
                Address address( info );
                 ::freeaddrinfo( info );
                 return address;
            }
            else
            {
                throw mem::mem().type< Error >( "lookup error: %s", ::gai_strerror( result ) ); 
            }
        }
        
        void Link::Address::parse()
        {
            ENTER();
            auto peer = ( Peer* ) &a;

            family = peer->sa_family;
            const void* target = NULL;
            
            if ( family == AF_INET )
            {
                Ip* s = ( Ip* ) peer;
                target = &s->sin_addr;
                _port = s->sin_port;
            }
            else
            {
                Ip6* s = ( Ip6* ) peer;
                target = &s->sin6_addr;
                _port = s->sin6_port;
            }
            
            host.space( 0x200 );
            ::inet_ntop( family, target, host, host.size() );
        }
        
        void Link::Address::parse( const Info& address )
        {
            family = address.ai_family;
            type = address.ai_socktype == SOCK_STREAM ? Tcp : Udp;

            if ( address.ai_family == AF_INET6 )
            {
                length = sizeof( Ip6 );
                Ip6* addr = ip6( );
                addr->sin6_addr = ( ( Ip6* ) address.ai_addr )->sin6_addr;
                addr->sin6_family = address.ai_family;
            }
            else
            {
                length = sizeof( Ip );
                Ip* addr = ip4( );
                addr->sin_addr = ( ( Ip* ) address.ai_addr )->sin_addr;
                addr->sin_family = address.ai_family;
            }
        }

        Link::Address::Address( const Info* info )
        : family( AF_INET ), type( Tcp ), _port( 0 ), length( sizeof( a ) )
        {
            std::memset( &a, 0, length );
            
            if ( info )
            {
                parse( *info );
            }
        }
        
        Link::Address::Address( Type _type,  const Data& _host, ui port  )
        : type( _type ), host( _host ), _port( port ), length( sizeof( a ) ), family( AF_INET )
        {
            std::memset( &a, 0, length );
            if ( type != Local )
            {
                this->port( port );
            }
            else
            {
                local( host );
            }
        }

        void Link::Address::local( const Data& name )
        {
            length = sizeof( un );
            std::memset( &un, 0, length );
            family = AF_LOCAL;
            std::strcpy( un.sun_path, name );
            un.sun_family = family;
        }
        
        Link::Address::operator Peer*( ) const
        {
            if ( type == Local )
            {
                return ( Peer* ) &un;
            }
            
            return ( Peer* ) &a;
        }
        
        Data Link::Address::tostring() const
        {
            Data data;
            data( "%s://%s:%d", ( const char* ) stype( type ), host.c(), _port );
            return data;
        }
        
        void Link::Address::operator = ( const Link::Address& address )
        {
            type = address.type;
            
            void* source =  address.type == Local ? ( void* ) &address.un : ( void* ) &address.a ;
            length =  address.type == Local ? sizeof( address.un ) : sizeof( address.a );
            
            std::memcpy( *this, source, length );
            
            length = address.length;
            host = address.host;
            family = address.family;
            
            port( address._port );
        }
        
        ui Link::Address::port( ui port )
        {
            if ( family == AF_INET )
            {
                ip4( )->sin_port = htons( port );
            }
            else
            {
                ip6( )->sin6_port = htons( port );
            }

            this->_port = port;
            return port;        
        }
        
        void Link::open( const Address& address )
        {
            ENTER();
            
            m_address = address;

            TRACE( "%d", address.family );
            auto fd = si::check(
                    ::socket( address.family, address.type == Udp ? SOCK_DGRAM : SOCK_STREAM, 0 )
                    )( "socket" );

            int set = 1;
#ifdef __MACH__
            ::setsockopt( fd, SOL_SOCKET, SO_NOSIGPIPE, ( void * ) &set, sizeof( set ) );
#else
            ::setsockopt( fd, SOL_SOCKET, MSG_NOSIGNAL, ( void * ) &set, sizeof( set ) );
            ::setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &set, sizeof( set ) );
#endif      

            TRACE( "opened fd %d for type %d", fd, address.type );
            
            
            File::assign( fd );
        }
        
        int Link::error()
        {
            int error = 0;
            unsigned int length = sizeof( error );

            ::getsockopt( fd( ), SOL_SOCKET, SO_ERROR, &error, &length );

            return error;
        }
        
        Link::Accept Link::accept()
        {
            Accept accept;
            
            accept.fd =  si::check( ::accept( fd( ), accept.address, &accept.address.length ) )( "accept" );
            if ( !local() )
            {
                accept.address.parse( );    
            }
            else
            {
                accept.address = Address( m_address.type, m_address.host );
            }
            
            
            TRACE( "accepted address type %u", accept.address.type );
            

            return accept;
        }
        
        void Link::connect()
        {
            ENTER( );
            si::check( ::connect( fd( ), address( ), address( ).length ), EINPROGRESS )( "connect" );
        }
         
        void Link::listen()
        {
            ENTER( );

            bind();
            
            if ( udp() )
            {
                return;
            }
            
            si::check( ::listen( fd(), SOMAXCONN ) )( "listen" );
        }
        
        void Link::bind()
        {
            STRACE( "binding to %s:%d", address().host.c(), address()._port );
            si::check( ::bind( fd(), address(), address().length ) )( "bind to %s", address().tostring().c() );
        }
        
        unsigned long Link::write( const Data& data, ul offset ) const
        {
            ENTER( );

            if ( local() )
            {
                return File::write( data );
            }

            long result;

            if ( udp() )
            {
                result = ::sendto( fd(), data, data.length( ), 0, address( ), address( ).length );
            }
            else
            {
                result = ::send( fd(), data, data.length( ), 0 );
            }
            
            TRACE( "wrote %u bytes", result );

            return si::check( result )( "send" );
        }
        
        unsigned long Link::read( Data& data, ul length, ul offset ) const
        {
            ENTER();
            
            TRACE( "local: %d, address type %u", local(), m_address.type );
            
            if ( local() )
            {
               return File::read( data );
            }

            long result;           
            
            length = File::available();
            data.space( length );
 
            if ( udp() )
            {
                Address peer;
                result = ::recvfrom( fd(), data, length, 0, peer, &peer.length );
                peer.parse( );
                TRACE( "read %d bytes", result );
                ( const_cast< Link* >( this ) )->address() = peer;
            }
            else
            {
                result = ::recv( fd(), data, length, 0 );
            }
            
            data.length( result );
            
            return si::check( result )( "receive" );

            return result;
        }
        
        void Link::shutdown()
        {
            si::check( ::shutdown( fd(), SHUT_RDWR ) )( "shutdow" );
        }
    }
}