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

            class Stream: public fs::File
            {
                friend class Process;
            public:
                virtual ~Stream()
                {
                    close();
                }
                
                ui type() const
                {
                    return m_type;
                }
                
            private:
                Handle writeFd( bool child = false )
                {
                    return child ? m_fds[ 0 ] : m_fds[ 1 ];
                }

                Handle readFd( bool child = false )
                {
                    return child ? m_fds[ 1 ] : m_fds[ 0 ];
                }

                void init( bool child );
                void open( );
                void close( );

                Stream( ui type = 0 )
                : m_type( type )
                {
                }

            private:
                int m_fds[2];
                ui m_type;

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
            
            Stream& stream( ui type )
            {
                for ( auto i = 0; i < lengthof( m_streams ); i++ )
                {
                    auto& stream = m_streams[ i ];
                    if ( stream.type() == type )
                    {
                        return stream;
                    }
                }   
                
                throw Error();
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
            virtual void aborted( What ) = 0;
            virtual void terminated() = 0;
            
            class Terminate : public Thread
            {
            public:
                Terminate()
                {
                    start();
                }
                
                virtual ~Terminate( )
                {
                }
                
            private:
                virtual void run( )
                {
                    
                }

            };

            static void signal( int );

        private:
            static Signals* s_instance;
            static Terminate* s_terminate;
        };
    }

}

#endif