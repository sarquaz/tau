#ifndef _TAU_TH_H_
#define _TAU_TH_H_

namespace tau
{
    namespace th
    {
        class Pool
        {
            public:
                
                class Task: public newev::Event::Parent
                {
                    friend class Pool;
                        
                public:
                    virtual ~Task()
                    {
                        ENTER();                        
                    }
                    
                    virtual void operator()() = 0;
                    virtual void complete( newev::Event& )
                    {
                        ENTER();
                    }
                                        
                    virtual void event( ui, newev::Event& );                                        
                    
                protected:
                    Task(  )
                        : m_loop( &tau::loop() )
                    {
                        ENTER();
                    }
                    
                    
                private:
                    void dispatch( newev::Loop& );
                    newev::Loop* loop() const
                    {
                        return m_loop;
                    }
                    
                private:
                    newev::Loop* m_loop;
                    
                };
                
            private:
                class Thread: public os::Thread
                {
                public:
                    Thread( )
                        : os::Thread()
                    {
                        
                    }
                    
                    virtual ~Thread()
                    {
                        
                    }
                    
                    virtual void run();
                    void stop()
                    {
                        ENTER();
                        m_loop.stop();
                    }
                    
                    newev::Loop& loop()
                    {
                        return m_loop;
                    }
                    
                private:
                    newev::Loop m_loop;
                };
                
            public:
                Pool( const Options& options = {} )
                    : m_count( options.def( options::Threads, 5 ) )
                {
                }
                
                ~Pool()
                {
                    m_threads.all( [] ( Thread* thread ) { mem::mem().detype< Thread >( thread ); } );
                }
                                
                void add( Task& task );
                void stop();

            private:
                li::List< Thread* > m_threads;
                ui m_count;
        };
    };
    
    extern th::Pool& pool();
}

#endif