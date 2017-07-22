#include "tau.h"
#include "../trace.h"

namespace tau
{
    namespace th
    {
        void Pool::add( Task& task )
        {
            ENTER();
            
            //
            //  dispatch to thread
            //
            Thread* thread;
            
            TRACE( "have %d threads count %d", m_threads.length(), m_count );
            
            if ( m_threads.length() <= m_count )
            {
                thread = mem::mem().type< Thread >( );
                thread->start();
            }
            else
            {
                thread = m_threads.pop();
            }
            
            m_threads.prepend( thread );
            
            task.dispatch( thread->loop() );
        }
        
        void Pool::Task::dispatch( newev::Loop& loop )
        {
            ENTER();
            auto event = newev::Event::get( this );
            event->schedule( newev::Event::Start, &loop );
        }
        
        void Pool::Task::event( ui, newev::Event& event )
        {
            ENTER();
            
            if ( m_loop != event.processor() )
            {
                ( *this )();
                event.schedule( newev::Event::Start, m_loop );
            }
            else
            {
                complete( event );
                deref();
            }
        }
        
        void Pool::Thread::run()
        {
            ENTER();
            
            m_loop.run();
        }
        
        void Pool::stop()
        {
            ENTER();
            
            m_threads.all( [] ( Thread* thread ) 
                {
                     thread->stop();
//                     thread->join();
                } );
        }
        
    }
    
}   