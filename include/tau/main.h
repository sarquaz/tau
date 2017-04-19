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
            Repeat
        };
    }
    
    typedef li::Set< ui > Options;
    
    class Main
    {
    public:
        template < class Callback > class Thread: public si::os::Thread
        {
        public:
            Thread( Callback callback )
                :si::os::Thread(), m_data( NULL ), m_callback( callback )
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
        
        private:
            virtual void run()
            {
                ENTER();
                Main::instance().started( this );
               
                Main::instance().lock().with( [ & ]( ){ m_callback(); } );
            }
    
        private:
            void* m_data;
            std::function< void() > m_callback;
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
            m_threads.all( [ & ] ( si::os::Thread* thread ) 
             { 
                thread->join();
                delete thread;
             } );
        }
        
        static si::os::Thread& thread();
        
        void started( si::os::Thread* );

        si::os::Lock& lock()
        {
            return m_lock;
        }
        
        static Main& instance();
         
    
    private:
        li::Array< si::os::Thread* > m_threads; 
        si::os::Lock m_lock;
    };
    
    template < class Callback > inline void start( Callback callback )
    {
        start( {}, callback );
    }
    
    template < class Callback > inline void start( const Options& options, Callback callback )
    {
        auto threads = options.def( options::Threads, 1 );
        STRACE( "need to start %d threads", threads );        
        
        Main::instance().start( threads, callback );
    }
    
    inline void stop()
    {
        Main::instance().stop();
    }
    
    inline si::os::Thread& thread()
    {
        return Main::thread();
    }
}

#endif  