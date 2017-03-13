#include "si.h"

namespace si
{
    namespace os
    {
        class Lock
        {
        public:
            Lock( );
            ~Lock( );

            template < class Logic > void with( Logic logic )
            {
                lock( );
                logic( );
                unlock( );
            }

        private:
            typedef pthread_mutex_t Handle;

            void lock( );
            void unlock( );

        private:
            Handle m_lock;
        };
        
        class Semaphore
        {
        public:
            Semaphore( );
            ~Semaphore( );

            void post( );
            void wait( );

        private:
#ifdef __MACH__
            typedef sem_t* Handle;
#else
            typedef sem_t Handle;
#endif

        private:
            Handle m_handle;
        };

        class Thread
        {
        public:
            static unsigned int id()
            {
                return ( unsigned long ) ::pthread_self();
            }
            
            Thread( )
            {
            }

            virtual ~Thread( )
            {
            }

            void join( ) const;
            void start( );
            
        private:
            static void routineStatic( void* data );
            virtual void routine( ) = 0;

            typedef pthread_t Handle;

        private:
            Handle m_handle;
        };

        class Process
        {
        public:
            static unsigned int id( )
            {
                return ::getpid();
            }
        
            Process();
            void start( const Data& );
            void stop( );

            struct Stream
            {
                int fds[2];
                unsigned int type;
                Handle fd;

                Handle writeFd( bool child = false )
                {
                    return child ? fds[ 0 ] : fds[ 1 ];
                }

                Handle readFd( bool child = false )
                {
                    return child ? fds[ 1 ] : fds[ 0 ];
                }

                void init( bool child );
                void open( );
                void close( );

                Stream( unsigned int _type = 0 )
                : type( _type ), fd( 0 )
                {
                }
                ~Stream( )
                {
                    close( );
                }
                                
                void operator = ( const Stream& stream )
                {
                    type = stream.type;
                }

            };

            unsigned int pid( ) const
            {
                return m_pid;
            }

            int code( );
            void signal( int signal ) const;

            void kill( ) const
            {
                signal( SIGKILL );
            }

            const Data& command( ) const
            {
                return m_command;
            }

            template < class Logic > void streams( Logic logic )
            {
                for ( auto i = 0; i < lengthof( m_streams ); i++ )
                {
                    logic( m_streams[ i ] );
                }
            }

        private:
            unsigned int m_pid;
            Stream m_streams[ 3 ];
            Data m_command;
        };

        class Module
        {
        public:
            Module( const Data& name, const Data& entry = "" );
            ~Module( );

            void* symbol( const Data& name ) const;
            void* entry( ) const
            {
                return m_entry;
            }

        private:
            void* m_handle;
            void* m_entry;
        };

        class Signals
        {
        public:

            virtual ~Signals( )
            {
            }

            static void assign( Signals* signals );
            static Signals& instance( )
            {
                assert( s_instance );
                return *s_instance;
            }

            enum What
            {
                Error = SIGSEGV,
                Abort = SIGABRT,
                Exit = SIGTERM
            };

        protected:
            Signals( );

        private:

            virtual void onAbort( What ) = 0;
            virtual void onTerminate( ) = 0;

            class Terminate : public Thread
            {
            public:

                Terminate( )
                {
                    start( );
                }

            private:
                virtual void routine( );

            };

            static void signal( int );

        private:
            static Signals* s_instance;
            static Terminate* s_terminate;
        };
    }
}