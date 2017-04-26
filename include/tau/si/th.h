#ifndef _TAU_TH_H_
#define _TAU_TH_H_


namespace tau
{
    namespace th
    {
        class Pool
        {
            public:
                
                class Task: public ev::Request::Parent
                {
                    friend class Pool;
                        
                public:
                    virtual ~Task()
                    {
                        ENTER();
                        
                        if ( m_request )
                        {
                            m_request->deref();
                        }
                    }
                    
                    virtual void operator()() = 0;
                    
                    template < class Callback > void assign( Callback callback )
                    {
                       // m_callback = mem::mem().type< si::Callback< Callback > >( callback );
                        
                        request().assign( callback );
                    }
                    
                    void callback()
                    {
                        request().callback();
                    }
                    
                    virtual void destroy()
                    {
                        ENTER();
                        this->~Task();
                        mem::mem().free( this );
                    }
                    
                    ev::Request& request()
                    {
                        if ( !m_request )
                        {
                            m_request = mem::mem().type< ev::Request >( *this );
                        }
                        
                        return *m_request;
                    }
                    
                    ev::Loop& loop( ) const
                    {
                        assert( m_loop );
                        return *m_loop;
                    }
                    
                    virtual void configure( ev::Loop::Event& event )
                    {
                        event.type = ev::Loop::Event::Once;
                    }
                    
                protected:
                    Task()
                        : m_request( NULL ), m_loop( NULL ), m_callback( NULL )
                    {
                        ENTER();
                    }
                    
                private:
                    void execute()
                    {
                        
                    }                
                    
                    ev::Loop& loop( ev::Loop& loop )
                    {
                        m_loop = &loop;
                        return loop;
                    }        
                    
                private:
                    si::Call<>* m_callback;
                    ev::Request* m_request;
                    ev::Loop* m_loop;
                    
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
                    
                    void dispatch( Task& );
                    
                private:
                    ev::Loop m_loop;
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
                                
                void add( Task& task )
                {
                    ENTER();
                    
                    
                    TRACE( "setting task callback", "" );
                    //
                    //  set callback
                    //
                    ///task.request().assign( callback );
                    
                    
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
                    
                    thread->dispatch( task );
                }
                
                void stop();

            private:
                li::List< Thread* > m_threads;
                ui m_count;
        };
            

    }
    
        
    
}
#endif