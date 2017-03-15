#ifndef TAU_LINER_H
#define	TAU_LINER_H

#include "log.h"
#include "base.h"

namespace tau
{
    class Line;
    
    class Liner: public in::Female, public in::Male
    {
        friend class Line;
    public:
        
#define EVENT_LINER 3000
        enum 
        {
            Line = EVENT_LINER
        };
        
        virtual ~Liner();

        virtual void start( );
        virtual void stop();
        
        static tau::Line* pline();
        static Liner& instance( in::Female& female, ui threads = 1 );
        static tau::Line& line();
        
    private:
        static Liner& instance()
        {
            return *s_instance;
        }
        
        Liner( ui threads );
        
    private:
        void started( Grain& grain );
        void stopped();
        
        virtual ui code() const
        {
            return 0;
        }
        
    private:
        ui m_count;
        li::List< tau::Line* > m_lines;
        si::Lock m_lock;   
        ui m_stopped;
        si::Semaphore m_semaphore;
        static Liner* s_instance;
    };

    
    class Line : public si::Thread, public in::Male, public ev::Loop::Client
    {    
        friend class Liner;
        
    public:
#define EVENT_LINE 100

        enum 
        {
            Start = EVENT_LINE,
            Stop
        };
        
        virtual ~Line();

        virtual intptr_t id() const
        {
            return ( intptr_t ) m_id;
        }
        intptr_t threadId() const
        {
            return si::Thread::id();
        }
        
    protected:
        Line( ui id );
        
    private:
        virtual void routine( );
        virtual void stop();
        
        virtual void onEvent( ev::Loop::Event& );
        
        enum 
        {
            _Start = EVENT_LINE + 10
        };
        
        virtual void destroy()
        {
            delete this;
        }
        
        virtual ui code() const
        {
            return 0;
        }

    private:
        ui m_id;
        ev::Loop* m_loop;
    };
    
    inline Line& line()
    {
        return Liner::line();
    }
    
    class Event : public ev::Loop::Client, public in::Male
    {
    public:
        Event( )
        {
        }
        
        virtual ~Event( )
        {
        }

        void start( const Time& interval = Time() );

        static Event& get( in::Female* female = NULL )
        {
            auto& event = tok< Event >( )( );
            if ( female )
            {
                event.females( ).add( *female );
            }

            return event;
        }

#define EVENT_TIMER 200

        enum
        {
            Timeout = EVENT_TIMER
        };

        void setType( ui type )
        {
            m_type = type;
        }

        ui type( ) const
        {
            return m_type;
        }

        virtual ui code( ) const
        {
            return typeid ( *this ).hash_code( );
        }

        Grain* grain( ) const
        {
            return m_grain;
        }

        void setGrain( Grain* grain )
        {
            m_grain = grain;
        }

    private:
        virtual void onEvent( ev::Loop::Event& )
        {
            in::Male::dispatch( Timeout );
        }
        
        virtual void init()
        {
            m_grain = NULL;
            m_type = 0;
            Rock::init();
        }

        virtual void cleanup( )
        {
            ENTER();
            ev::Loop::Client::clear();
            in::Male::cleanup();
        }

    private:
        Time m_interval;
        Grain* m_grain;
        ui m_type;
    };
    
    struct event
    {
        Event& e;
        ui type;

        event( in::Female* female = NULL, ui _type = 0 )
        : e( Event::get( female ) ), type( _type )
        {
        }

        Event& operator()( const Time& interval = Time(), Grain* grain = NULL )
        {
            e.setGrain( grain );
            e.setType( type );
            
            e.start( interval );
            return e;
        }
    };
    
    
}

#endif	