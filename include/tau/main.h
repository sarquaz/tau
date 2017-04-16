#ifndef _TAU_MAIN_H_
#define _TAU_MAIN_H_

namespace tau
{
    /**
     * Strings map class (equivalent to std::map< std::string, std::string >)
    **/
    class Strings: public li::Set< Data >
    {
    public:
        Strings()
            : li::Set< Data >()
        {
        }
        Strings( const Strings& strings )
            : li::Set< Data >()
        {
            operator =( strings );
        }
        Strings( std::initializer_list< std::pair< const Data, Data > > list )
            : li::Set< Data >()
        {
            for ( auto i = list.begin(); i != list.end(); i++ )
            {
                ( *this )[ i->first ] = i->second;
            }
        }
        
        virtual ~Strings()
        {
        }
        
        Data def( const Data& key, const Data& value ) const;
        int number( const Data& name ) const
        {
            return ::atoi( def( name, "0" ) );
        }
    };
    
    enum Action
    {
        Init,
        Begin,
        Complete
    };
    
    class Result;
    
    class Listener
    {
    public:
        virtual void result( Action, Result* ) = 0;
    };
    
    class Result
    {
    public:
        
        
        virtual ~Result ()
        {
            
        }
        
        void listener( Listener* listener )
        {
            m_listeners.append( listener );
        }
        
    protected:
        void dispatch( Action, Result* ) const;
        
    private:
        li::List< Listener* > m_listeners;
    };
      
    void start( );
    void start( const Strings& options );
    void stop();
    void listen( Listener* );
    
    class Main: public Result
    {
    public:
        class Thread: public si::os::Thread, public Result
        {
        public:
            Thread()
                : si::os::Thread(), m_data( NULL )
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
            virtual void run();
    
        private:
            void* m_data;
        };
        
        Main()
        {
        }
        
        ~Main ()
        {
        }
        
        void start( us threads );
        void stop();
        
        Thread* thread();
    
    private:
        li::Array< Thread* > m_threads; 
        si::os::Lock m_lock;
    };
    
}

#endif