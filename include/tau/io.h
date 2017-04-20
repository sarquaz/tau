#ifndef _TAU_IO_H_
#define _TAU_IO_H_

namespace tau
{
    extern ev::Loop& loop();
        
    namespace io
    {
        // class Event;
        //
        // class Request: public ev::Request
        // {
        //     friend class Event;
        //
        // public:
        //     Request( ev::Request::Type type = ev::Request::Default )
        //         : ev::Request( type ), m_callback( callback ), m_event( NULL )
        //     {
        //
        //     }
        //
        //     virtual ~Request()
        //     {
        //
        //     }
        //
        //     virtual void callback()
        //     {
        //         m_callback( this );
        //     }
        //
        //     virtual void destroy()
        //     {
        //         this->~Request();
        //         mem::mem().free( this );
        //     }
        //
        // private:
        //
        //
        //
        // private:
        //     std::function< void( ev::Request* ) > m_callback;
        //     Event* m_event;
        // };
        //
        class Event: public Reel
        {
            public:
                Event()
                {

                }

                virtual ~Event()
                {
                    m_requests.all( [ ] ( ev::Request* request ) { request->deref(); } );
                }

                virtual void destroy()
                {
                    this->~Event();
                    mem::mem().free( this );
                }
                
                virtual ev::Loop::Event& event()
                {
                    return *( ev::Loop::Event::get() );
                }

                template < class Callback > void request( Callback callback, ev::Request* request = NULL )
                {   
                    ev::Loop::Event* event;
                    
                    if ( !request )
                    {
                        event = &( this->event() );
                        request = mem::mem().type< ev::Request >( *event );
                    }
                    else
                    {
                        event = &( request->event() );
                    }
                    
                    request->assign( [ & ] ( ev::Request* request ) 
                        { 
                            callback( request );
                            this->callback();                
                        } );

                    event->request = request;
                    event->parent = this;
                    m_requests.append( request );

                    tau::loop().add( *event );
                }

                virtual void callback() = 0;

            private:
                li::List< ev::Request* > m_requests;
        };
        //  
    }
}

#endif