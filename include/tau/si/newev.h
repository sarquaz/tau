#ifndef _TAU_SI_NEWEV_H_
#define _TAU_SI_NEWEV_H_

#include "../../src/trace.h"

namespace tau
{
    namespace newev
    {
        typedef int Handle;
        
        class Loop;
        
        class Event: public Reel
        {
            friend class Loop;
            
        public:
            enum Type
            {
                Default,
                Timer,
                File,
                Process,
                Error,
                Stop
            };
            
            class Parent: public Reel
            {
                friend class Event;
                
            public:
                typedef li::Set< Event* > Events;
                
                Parent()
                {
                    ENTER();
                }
                
                virtual ~Parent()
                {
                    ENTER();
                    cancel();
                }
                
            protected:
                Events& events()
                {
                    return m_events;
                }
                
                Event& last();
                
                Event* event( Event::Type );
                
                void cancel();
                
                virtual void event( ui operation, Event& )
                {
                    TRACE( "%u", operation );
                }
                
            private:
                Events m_events;
            };
            
            
            enum 
            {
                Cancel = __LINE__,
                Start
            };
            
            Event( Parent* parent, Type type = Default )
                : m_parent( parent ), m_type( type ), m_loop( NULL ), m_processor( NULL )
#ifdef __linux__
                    , m_value( 1 )
#endif
            {
            ENTER();
            }
            
            Event()
                : m_parent( NULL ), m_type( Stop ), m_loop( NULL ), m_processor( NULL )
#ifdef __linux__
                    , m_value( 1 )
#endif
            {
                ENTER();
            }
            
            virtual ~Event();
            
            virtual void destroy()
            {
                mem::mem().detype< Event >( this );
            }
            
            template < class ... Args > static Event* get ( Args&&... args )
            {
                return mem::mem().type< Event >( std::forward< Args >( args ) ... );
            }
            
            Type type() const
            {
                return m_type;
            }
            
            virtual const fs::File& file() const
            {
                return m_file;
            }
            
            virtual ui id();
            
            void schedule( ui what = Start, Loop* loop = NULL );
            void cancel();
            
            bool assigned( Loop& ) const;
            Loop* processor() const
            {
                return m_processor;
            }
            
        private:
            bool assign( Loop& );
            
#ifdef __linux__
            virtual void before();
            virtual void after();
#endif            
            
            void on ( ui );            
            Loop* processor( Loop& loop )
            {
                m_processor = &loop;    
                return m_processor;
            }
            
            ui smac();
            
#ifdef __linux__            
            virtual ui slinux();
#endif
            
            

            
        private:
            Type m_type;
            fs::File m_file; 
            Parent* m_parent;
            li::Set< Loop* > m_loops;
            Loop* m_loop;
            Loop* m_processor;
            
#ifdef __linux__
            long long m_value;
#endif
        };
        
        class Timer: public Event
        {
        public:
            Timer( Parent& parent, const Time& timeout )
                : Event( &parent, Event::Timer ), m_timeout( timeout )
            {
                
            }
            
            virtual ~Timer()
            {
                
            }
            
            virtual void destroy()
            {
                mem::mem().detype< Timer >( this );
            }
            
            const Time& timeout() const
            {
                return m_timeout;
            }
            
        private:
            
#ifdef __linux__
            virtual void before();
            virtual void after();
            virtual ui slinux();
#endif            
            
            
        private:
            Time m_timeout;
            
        };
        
        class Loop
        {
            friend class Event;
                        
            struct Request: Reel
            {
                ui operation;
                Event& event;
                
                Request( ui _operation, Event& _event )
                    : operation( _operation ), event( _event )
                {
                    TRACE( "operation %u event 0x%x", operation, &event );
                    event.ref();
                }
                
                virtual ~Request()
                {
                    ENTER();
                    event.deref();
                }
                
                virtual void destroy()
                {
                    mem::mem().detype< Request >( this );
                }
            };
            
            typedef li::Map< fs::Handle, fs::Handle > Ids;
            
#ifdef __MACH__
            typedef struct kevent Hevent;
#else
            typedef struct epoll_event Hevent;
#endif
            enum
            {
                Stop = __LINE__
            };
            
        public:
            Loop();
            ~Loop();
            
            void stop( ) 
            {
                ENTER();
                m_stop = op( Stop, *Event::get() );
            }
            
            void run();
            
        private:
            Request* op( ui, Event& );
            

            Ids& ids()
            {
                return m_ids;
            }
            
            void stopped();    

        private:
            Handle m_handle;
            Hevent m_events[ 128 ];
            Request* m_stop;
            Ids m_ids;
        };
    }
    
    extern newev::Loop& loop();
}

#endif  