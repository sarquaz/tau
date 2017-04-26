#include "tau.h"
#include "../trace.h"

namespace tau
{
    namespace th
    {
        void Pool::Thread::dispatch( Task& task )
        {
            ENTER();
            
            task.loop( tau::loop() );
            
            m_loop.add( task.request() );
        }
        
        void Pool::Thread::run()
        {
            ENTER();
            
            m_loop.run( [ & ] ( ev::Loop::Event& event ) 
                {
                    assert( event.request );
                    
                    auto& task = reinterpret_cast< Task& >( event.request->parent() );
                    task();

                    task.loop().add( task.request() );
                 } );
        }
        
        void Pool::stop()
        {
            ENTER();
            
            m_threads.all( [] ( Thread* thread ) 
                {
                     thread->stop();
                     thread->join();
                } );
        }
        
    }
    
}   