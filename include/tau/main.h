#ifndef _TAU_MAIN_H_
#define _TAU_MAIN_H_

#include "../../src/trace.h"

namespace tau
{
    namespace options
    {
        enum 
        {
            Threads,
            Repeat,
            Msec,
            Usec
        };
    }
    
    namespace action
    {
        enum Action
        {
            Start,
            Stop
        };
    }
    
    typedef li::Set< ui > Options;
    
    class Main
    {
    public:
        
        template < class Callback > class Thread: public os::Thread
        {
        public:
            Thread( Callback callback )
                : os::Thread(), m_data( NULL ), m_callback( callback )
            {
            }
            
            virtual ~Thread ()
            {
            }
            
            void* data() const
            {
                return m_data;
            }
            
            void* data( void* data )
            {
                m_data = data;
                return data;
            }
            
            virtual void stop()
            {
                m_loop.stop();
            }
            
            const ev::Loop& loop() const
            {
                return m_loop;
            }
        
        private:
            virtual void run()
            {
                ENTER();
                Main::instance().started( this, &m_loop );
               
                Main::instance().lock().with( [ & ]( ){ m_callback( action::Start ); } );
                
                m_loop.run( [ & ] ( ev::Loop::Event& event ) 
                    {
                        assert( event.request );
                        event.callback();
                     } );
                
                Main::instance().lock().with( [ & ]( ){ m_callback( action::Stop ); } );
            }
    
        private:
            void* m_data;
            std::function< void( action::Action ) > m_callback;
            ev::Loop m_loop;
        };
        
        Main()
        {
        }
        
        ~Main ()
        {
            stop();
        }
        
        template < class Callback > void start( us threads, Callback callback )
        {
            ENTER();
        
            for ( auto i = 0; i < threads; i++ )
            {
                //
                //  create new thread
                //
                auto thread = new Thread< Callback >( callback );
                
                m_lock.with( [ & ] ( ) { m_threads.add( thread ); } );
            
                //
                //  start the thread
                //
                thread->start();
            }                
            
        }
        void stop()
        {
            ENTER();
            
        
            m_threads.all( [ & ] ( os::Thread* thread ) 
             {
                thread->stop(); 
                thread->join();
                delete thread;
             } );
        }
        
        static os::Thread& thread();
        static ev::Loop& loop();
        
        void started( os::Thread*, ev::Loop* );

        os::Lock& lock()
        {
            return m_lock;
        }
        
        static Main& instance();
         
    
    private:
        li::Array< os::Thread* > m_threads; 
        os::Lock m_lock;
    };
    
    template < class Callback > inline void start( const Options& options, Callback callback )
    {
        auto threads = options.def( options::Threads, 1 );
        STRACE( "need to start %d threads", threads );        
        
        Main::instance().start( threads, callback );
    }
    
    template < class Callback > inline void start( Callback callback )
    {
        start( {}, callback );
    }
    
    inline void stop()
    {
        Main::instance().stop();
    }
    
    inline os::Thread& thread()
    {
        return Main::thread();
    }
    
    inline ev::Loop& loop()
    {
        return Main::loop();
    }
    
    
    
    class Event: public io::Event
    {
    public:        
        Event( const Options& options )
            : io::Event(), m_time( options.def( options::Msec, 0 ), options.def( options::Usec, 0 ) )
        {
            ENTER();    
            m_repeat = options.def( options::Repeat, ( ui ) false );
        }
        
        virtual ~Event()
        {
            ENTER();
        }
        
        virtual ev::Loop::Event& event()
        {
            auto& event = io::Event::event();
            event.time = m_time;
            return event; 
        }
        
        virtual void callback()
        {
            ENTER();
            
            if ( !m_repeat )
            {
                deref();
            }
        }
        
        virtual void destroy()
        {
            this->~Event();
            mem::mem().free( this );
        }
        
        
    private:
        bool m_repeat;
        Time m_time;
    };
    
    template < class Callback > void event( Callback callback, const Options& options = {} )
    {
        auto event = mem::mem().type< Event >( options );
        
        event->request( callback );
    } 
    
}

#endif  