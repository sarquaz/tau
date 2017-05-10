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
                : os::Thread(), m_data( NULL )
            {
                ENTER();
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
            
            template < class Callback > void assign( action::Action action, Callback callback )
            {
                if ( action == action::Start )
                {
                    m_start = callback;
                }
                else
                {
                    m_stop = callback;
                }   
            }
        
        private:
            virtual void run()
            {
                ENTER();
                
                Main::instance().started( this );
               
                m_start( ); 
                
                m_loop.run( [ & ] ( ev::Loop::Event& event ) 
                    {
                        assert( event.request );
                        event.request->callback();
                     } );
                
                     m_stop( );
            }
    
        private:
            void* m_data;
            std::function< void() > m_start; 
            std::function< void() > m_stop; 
            
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
                thread->assign( action::Start, callback );
                
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
        
        template < class Callback > void stop( Callback callback )
        {
            ENTER();
            
            m_threads.all( [ & ] ( Thread* thread ) 
             {
                 thread->assign( action::Stop, callback );
                 thread->stop(); 
                 thread->join();
                 
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
    
    template < class Callback > inline void stop( Callback callback )
    {
        SENTER();
        
        Main::instance().stop( callback );
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
    
     
    
    
}

#endif  