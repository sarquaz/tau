#ifndef _TAU_SI_FS_H_
#define _TAU_SI_FS_H_

namespace tau
{
    namespace fs
    {
        typedef int Handle;
        
        typedef struct stat Stat;
        
        namespace type
        {
            enum Type
            {
                File,
                Dir,
                Fifo,
                Link,
                Socket
            };
        }
        
        struct Time
        {
            long access;
            long mod;
            long status;
            
            Time( long _access, long _mod, long _status )
            : access( _access ), mod( _mod ), status( _status )
            {
                
            }
            
        };
        
        struct Info
        {
            Stat stat;

            operator Stat*( )
            {
                return &stat;
            }

            unsigned long size( ) const
            {
                return stat.st_size;
            }
            
            Time time() const
            {
                return Time( stat.st_atime, stat.st_mtime, stat.st_ctime );
            }
            
            type::Type type() const;
            
        };
        
        Info info( const Data& );
        bool exists( const Data& );
        
        void chdir( const Data& );
        Data cwd( );
        std::pair< Data, Data > split( const Data& );
        
        
        class File
        { 
        public:            
            static void remove( const Data& name )
            {
                ::unlink( name );
            }
            
            virtual ~File()
            {
                close();
            }
            void operator = ( const File& file )
            {
                m_fd = file.fd();
            }
            
            static File open( const Data& );
            virtual void close( );
            
            operator Handle() const
            {
                return fd();
            }
            
            Handle fd() const
            {
                return m_fd;
            }
            
            unsigned int available( ) const;
            
            virtual unsigned long write( const Data& data, ul offset = 0 ) const;            
            virtual unsigned long read( Data& data, ul length = 0, ul offset = 0 ) const;
            
            File( const File& file )
            {
                operator =( file );
            }
            
            const Data& path() const
            {
                return m_path;
            }
            
            void assign( Handle fd, bool nb = true );
            void nb() const;
            
        protected:
            File( Handle fd = 0 )
            : m_fd( fd )
            {
            }
            
            
                        
        private:            
            void seek( ul offset ) const;
            
        private:
            Handle m_fd;
            Data m_path;
        };
        
        class Link: public File
        {
        public:

            enum Type
            {
                Udp,
                Tcp,
                Local
            };

            static Type type( const Data& );
            static Data stype( Type );

            typedef struct addrinfo Info;
            typedef struct sockaddr Peer;
            typedef struct sockaddr_in Ip;
            typedef struct sockaddr_in6 Ip6;

            struct Address
            {
                ui family;
                struct sockaddr_storage a;
                struct sockaddr_un un;
                socklen_t length;
                Type type;
                Data host;
                ui _port;

                void parse( const Info& );
                void parse( );
                void local( const Data& );

                Address( const Info* info = NULL );
                Address( Type type, const Data& host, ui port = 0 );

                Ip* ip4( ) const
                {
                    return ( Ip* ) &a;
                }

                Ip6* ip6( ) const
                {
                    return ( Ip6* ) &a;
                }

                ui port( ui );
                ui port() const
                {
                    return _port;
                }
                
                operator Peer*( ) const;
                void operator = ( const Address& );

                Data tostring( ) const;
            };

            struct Lookup
            {
                Info info;

                Lookup( Type type = Tcp );

                operator Info*( )
                {
                    return &info;
                }

                Address operator()( const Data& );
            };

            virtual ~Link()
            {

            }

            void operator = ( const Link& link )
            {
                m_address = link.address();
                File::operator =( link );
            }

            void open( const Address& );

            int error( );

            struct Accept
            {
                Handle fd;
                Address address;
            };

            Accept accept( );
            void connect(  );
            void listen( );

            virtual ul write( const Data& data, ul offset = 0 ) const;
            virtual ul read( Data& data, ul length = 0, ul offset = 0 ) const;

            const Address& address() const
            {
                return m_address;
            }
            Address& address()
            {
                return m_address;
            }
            bool local() const
            {
                return type() == Local;
            }
            bool udp() const
            {
                return type() == Udp;
            }
            bool tcp() const
            {
                return type() == Tcp;
            }
            
            void bind( );
            void shutdown();

        private:
            Type type( ) const
            {
                return m_address.type;
            }

        private:
            Address m_address;
        };
        
    }
}

#endif