#ifndef TAU_EV_H  
#define	TAU_EV_H

#include "si.h"
#include "../../src/trace.h"

namespace tau
{    
    namespace ev
    {
        class Loop;
    }
    
    ev::Loop& loop();
    
    namespace ev
    {
        class Loop: public si::Queue
        {
            friend class Event;
            friend struct event;
            friend struct timer;
            friend struct file;
            
        public:
            class Client;
                        
            virtual ~Loop()
            {
                
            }
                    
            class Event: public ie::Tok
            {
                friend class Loop;
                
            public:      
                virtual ~Event()
                {
                    ENTER();
                }
                    
                Loop& loop()
                {
                    assert( m_loop );
                    return *m_loop;    
                }
                
                Client& client()
                {
                    assert( m_client );
                    return *m_client;    
                }
                
                virtual void destroy( );
                
                virtual ui code() const
                {
                    return typeid( *this ).hash_code();
                }
                
                si::Queue::Type type() const
                {
                    return set().type;
                }
                
                si::Handle fd() const
                {
                    return set().fd;
                }
                
                Grain* grain() const
                {
                    return m_grain;
                }
                
                const si::Queue::Set& set() const
                {
                    return m_set;
                }
                
                
                
            protected:
                void assign( si::Handle fd, Type type );
z                void assign( const Time& );
                           
                virtual void init()
                {
                    m_set = si::Queue::Set();
                    m_client = NULL;
                    m_grain = NULL;
                }
                void fire();
                
                si::Queue::Set& set()
                {
                    return m_set;
                }
                

            private:
                virtual void remove()
                {
                    
                }
                void setClient( Client& client )
                {
                    m_client = &client;
                }
                void setGrain( Grain* grain )
                {
                    m_grain = grain;
                }
                void start( Loop& loop )
                {
                    ENTER();
                    m_loop = &loop;
                    client().add( *this );
                }
                
                static Event& instance( void* data )
                {
                    assert( data );
                    return *( ( Event* ) data );
                }
                
                operator Set&()
                {
                    set().data = this;
                    return set();
                }
                
                operator const Set&() const
                {
                    return set();
                }
                
                long time() const
                {
                    return m_timer.time();
                }
                                
            private:
                Loop* m_loop;
                Client* m_client;
                Grain* m_grain;
                Set m_set;
                si::Timer m_timer;
                
            };
                        
            
            struct event
            {
                Client& client;
                
                event( Client& _client )
                : client( _client )
                {
                }
                
                void operator()( const si::File& file, Type type );
                void operator()( const Time& time = Time( ), Loop* loop = NULL, Grain* grain = NULL );
                void assign( Event& event, Loop* loop = NULL );
            };
            
            class Client
            {
                friend class Event;
                
            public:
                virtual ~Client( )
                {
                    clear();
                }
                            
                void clear();
                
            protected:
                void timer( const Time& time = Time(), Loop* loop = NULL, Grain* grain = NULL )
                {
                    Loop::event( *this )( time, loop, grain );
                }
                
                void write( const si::File& file )
                {
                    Loop::event( *this )( file, Write );
                }
                void read( const si::File& file )
                {
                    Loop::event( *this )( file, Read );
                }
                
            private:
                virtual void onEvent( Event& event ) = 0;
                
                void add( Event& event )
                {
                    TRACE( "adding event 0x%x", &event );
                    
                    m_lock.with( [ & ] ( ) { m_events[ &event ] = &event; } );
                }
                void remove( Event& event )
                {
                    ENTER();
                    m_lock.with( [ & ] ( ) { m_events.remove( &event ); } );
                }
                
            private: 
                li::Map< Event* > m_events;
                si::Lock m_lock;
            };
            
            
            
            void stop()
            {
                ENTER();
                new Stop( *this );
                
            }
            
            void run()
            {
                si::Queue::setHandler( ( si::Queue::Handler ) &Loop::onEvent );
                si::Queue::run();
            }
            
        private:
            struct Stop: Client
            {
                Loop& loop;

                Stop( Loop& _loop )
                : loop( _loop )
                {
                    Client::timer( Time(), &loop );
                }

                virtual void onEvent( Event& event );

            };
            
            ui size() const
            {
                return m_events.size();
            }
            
            
            void clear();
            void remove( const Event& event );
            void add( Event& event );
            virtual void onEvent( const Set& set );
            virtual ui length()
            {
                return m_events.size();
            }
            
            
            
        private:
            li::Map< Event* > m_events; 
            si::Lock m_lock;
        };
        
        class File
        {
            
        public:
            struct Chunk: Rock
            {
                Data data;
                
                virtual ui code() const
                {
                    return typeid( *this ).hash_code();
                }
                
                virtual void cleanup()
                {
                    data.clear();
                }
            };
            
            class Client: Loop::Client
            {
                friend class File;
                
            public:
                virtual ~Client()
                {
                }
                
                void read()
                {
                    Loop::Client::read( file() );
                }
                void write()
                {
                    Loop::Client::write( file() );
                }
                
            protected:
                Client()
                : m_file( NULL )
                {
                }
                
                si::File& file() 
                {
                    assert( m_file );
                    return *m_file;
                }

                
            private:
                void setFile( si::File& file )
                {
                    m_file = &file;
                }
                                
                virtual void onEvent( Loop::Event& event );
                
                virtual void onWrite( )
                {
                }
                
                virtual void onRead( )
                {
                }
                
                virtual ui doWrite() = 0;
                virtual ui doRead( ) = 0;
                
            private:
                si::File* m_file;
            };
            
            File( Client& client, si::File* target = NULL )
            : m_client( client ),  m_target( target ? *target : m_file )
            {
                client.setFile( m_target );
            }
            
            void close()
            {
                m_client.clear();
                target().close();
            }
            
        protected:
            Client& client( )
            {
                return m_client;
            }
            
            si::File& target()
            {
                return m_target;
            }
            
            
        private:
            si::File& m_target;
            si::File m_file;
            Client& m_client;
        };
        
        class Link: public File
        {
        public:
            Link( File::Client& client )
            : File( client, &m_link )
            {
            }
            
        protected:
            si::Link& link()
            {
                return dynamic_cast< si::Link& >( File::target() );
            }
            
        private:
            si::Link m_link;
        };
        
    }
}
#endif
