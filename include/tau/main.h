#ifndef _TAU_MAIN_H_
#define _TAU_MAIN_H_

#include "../../src/trace.h"

namespace tau
{
    
    class Main
    {
    public:
        class Thread: public os::Thread
        {
        public:
            Thread( )
                : os::Thread(), m_data( NULL ), m_callback( NULL )
            {
                ENTER();
            }
            
            virtual ~Thread ()
            {
                if ( m_callback )
                {
                    m_callback->destroy();
                }
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
            
            void stop()
            {
                ENTER();
                m_loop.stop();
                m_pool.stop();
            }
            
            ev::Loop& loop() 
            {
                return m_loop;
            }
            
            th::Pool& pool()
            {
                return m_pool;
            }
            
            template < class Callback > void assign( Callback callback )
            {
                m_callback = mem::mem().type< si::Callback < Callback, action::Action > >( callback );
            }
        
        private:
            virtual void run()
            {
                ENTER();
                
                Main::instance().started( this );
               
                assert( m_callback );   
                Main::instance().lock().with( [ & ]( ){ ( *m_callback ) ( action::Start ); } );
                
                m_loop.run( [ & ] ( ev::Loop::Event& event ) 
                    {
                        assert( event.request );
                        event.callback();
                     } );
                
                Main::instance().lock().with( [ & ]( ){ ( *m_callback )( action::Stop ); } );
            }
    
        private:
            void* m_data;
            si::Call< action::Action >* m_callback; 
            ev::Loop m_loop;
            th::Pool m_pool;
            
        };
        
        Main()
        {
        }
        
        ~Main ()
        {
            ENTER();
                                
            m_threads.all( [ & ] ( Thread* thread ) 
             {
                delete thread;
             } );
        }
        
        template < class Callback > void start( us threads, Callback callback )
        {
            ENTER();
        
            for ( auto i = 0; i < threads; i++ )
            {
                //
                //  create new thread
                //
                auto thread = new Thread();
                thread->assign( callback );
                
                m_lock.with( [ & ] ( ) { m_threads.add( thread ); } );
            
                //
                //  start the thread
                //
                thread->start();
            }                
            
            m_threads.all( [ & ] ( Thread* thread )
             {

                thread->join();
             } );
        }
        
        void stop()
        {
            ENTER();
            
        
            m_threads.all( [ & ] ( Thread* thread ) 
             {
                thread->stop(); 
            } );
        }
        
        static Thread& thread();
        
        void started( Thread* );

        os::Lock& lock()
        {
            return m_lock;
        }
        
        static Main& instance();
         
    
    private:
        li::Array< Thread* > m_threads; 
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
        SENTER();
        
        Main::instance().stop();
    }
    
    inline Main::Thread& thread()
    {
        return Main::thread();
    }
    
    inline ev::Loop& loop()
    {
        return thread().loop();
    }
    
    inline th::Pool& pool()
    {
        return thread().pool();
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
        
        virtual void configure( ev::Loop::Event& event )
        {
            event.time = m_time;  
            if ( m_repeat && m_time.value > 0 )
            {
                TRACE( "timer event", "" );
                event.type = ev::Loop::Event::Timer;        
            }
        }
        
        virtual void after()
        {
            ENTER();
            
            if ( !m_repeat )
            {
                deref();
            }
        }
        
        virtual void destroy()
        {
            ENTER();
            this->~Event();
            mem::mem().free( this );
        }
        
        
    private:
        bool m_repeat;
        Time m_time;
    };
    
    template < class Callback > Event& event( Callback callback, const Options& options = {}, ev::Request* request = NULL )
    {
        auto event = mem::mem().type< Event >( options );
        
        event->request( callback, request );
        return *event;
    } 
    
    
}

#endif  