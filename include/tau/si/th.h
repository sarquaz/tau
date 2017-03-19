#ifndef _TAU_TH_H
#define _TAU_TH_H



namespace tau
{
    namespace si
    {
        namespace th
        {
            class Condition
            {
                typedef pthread_cond_t Cond;
                typedef pthread_mutex_t Mutex;
                
            public:
                Condition();
                ~Condition();
                
                void wait( const Time* time = NULL );
                void signal();
                                
            private:
                Cond m_cond;
                Mutex m_mutex;
            };
            
            class Thread
            {
                typedef pthread_t Handle;
                
            public:
                Thread( )
                {
                }

                virtual ~Thread( )
                {
                }

                void join( ) const;
                void start( );
                
                static ul id( );
            private:
                static void routine( void* data );
                virtual void run( ) = 0;
            
            private:
                Handle m_handle;
            };
            

        }
        
        
    }
}
#endif