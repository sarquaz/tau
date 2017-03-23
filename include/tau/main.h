#ifndef _TAU_MAIN_H
#define _TAU_MAIN_H

namespace tau
{
    class Strings: public li::Set< Data >
    {
    public:
        Data def( const Data& key, const Data& value ) const;
        int number( const Data& name ) const
        {
            return ::atoi( def( name, "0" ) );
        }
    };
      
    void start( );
    void start( const Strings& options );
    void stop();
    
    class Main
    {
    public:
        class Thread: public si::os::Thread
        {
        public:
            Thread ()
            {
            
            }
        
        
            virtual ~Thread ()
            {
            
            }
        
        private:
            virtual void run();
    
        private:
            /* data */
        };
        
        Main()
        {
        }
        
        ~Main ()
        {
        }
        
        void start();
        
        Thread* thread();
    
    private:
        li::Mass< Thread > m_threads; 
        si::os::Lock m_lock;
    };
    
}

#endif