#ifndef _TAU_SI_OS_H_
#define _TAU_SI_OS_H_

namespace tau
{
    namespace os
    {
        typedef int Handle;
        
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
    
        class Condition
        {
            typedef pthread_cond_t Cond;
            typedef pthread_mutex_t Mutex;
        
        public:
            Condition();
            ~Condition();
        
            void wait( const Time* time = NULL );
            void signal();
            ui count() const
            {
                return m_count;
            }
                        
        private:
            Cond m_cond;
            Mutex m_mutex;
            ui m_count;
        };
    
        class Thread
        {
            typedef pthread_t Handle;
        
        public:
            Thread( )
                : m_started( false )
            {
            }

            virtual ~Thread( )
            {
            }

            void join( ) const;
            void start( );
        
            ul id( );
            
            bool started() const
            {
                return m_started;
            }
            
        private:
            static void routine( void* data );
            virtual void run( ) = 0;
    
        private:
            Handle m_handle;
            bool m_started;
        };
        
        class Process
        {
        public:
            static ui id( )
            {
                return ::getpid();
            }

            Process();
            void start( const Data& );
            void stop( );

            struct Stream
            {
                int fds[2];
                ui type;
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

                Stream( ui _type = 0 )
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

            ui pid( ) const
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
            ui m_pid;
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
        
        /*`

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
        };*/
    }

}

#endif