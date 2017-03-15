#ifndef TAU_IO_H
#define	TAU_IO_H

#include "in.h"
#include "ev.h"

#include "../../src/trace.h"

namespace tau
{
    class Event;
    
    namespace io
    {
        enum Type
        {
            In,
            Out,
            Inout
        };
        
        class Till : public in::Male, public in::Female
        {
        public:

#define EVENT_TILL 100000

            enum 
            {
                Write = EVENT_TILL
            };
            
            static Till& get( Type type = Inout, in::Female* female = NULL );
            Till(  );
            
            virtual ~Till( )
            {
            }
            
            Type type( ) const
            {
                return m_type;
            }
            const Data& data() const
            {
                return m_data;
            }
            void clear()
            {
                m_data.clear();
            }
            ui length() const
            {
                return data().length();
            }
            bool empty() const
            {
                return length() == 0;
            }
            
            void add( ui length )
            {
                add( Data( ( char* ) NULL, length ) );
            }
            void add( const Data& data );
            void add( Till& );
            
            Data read( ui length = 0 );
            
            operator const char*() const
            {
                return data();
            }
            
            operator const Data&() const
            {
                return data();
            }
            
            Data space( ui  length );
            bool next();
            
        private:
            void tillWrite( Grain& grain );
            
            void setType( Type type )
            {
                m_type = type;
            }
            void setTarget( Till* till )
            {
                m_target = till;
            }
            void setMapped( Till* );
            Till* mapped( ) const
            {
                return m_mapped;
            }
            Till* target( ) const
            {
                return m_target;
            }
            
            virtual void cleanup();
            virtual ui code() const
            {
                return typeid( *this ).hash_code();
            }
            ui last( ) const
            {
                return m_last;
            }
            void map( Till& till );
            
            
        private:
            Type m_type;
            ui m_last;
            Data m_data;
            Till* m_mapped;
            Till* m_target;
            li::List< Till* > m_tills;
        };
        
        class Set: public in::Male, public in::Female
        {
            friend class Client;
        public:
            virtual ~Set()
            {
            }
            
            struct Options: public li::Strings
            {                
                Options( const Options& options )
                {
                    li::Strings::operator =( options );
                }                
                Options()
                {
                }
                Options( std::initializer_list< std::pair< si::Data, si::Data > > list )
                {
                    for ( auto i = list.begin(); i != list.end(); i++ )
                    {
                        ( *this )[ i->first ] = i->second;
                    }
                }
                
                int number( const Pair& name ) const
                {
                    return ::atoi( ( *this )[ name ] );
                }
            };
            
#define EVENT_SET 9000
            enum 
            {
                End = EVENT_SET,
                Data,
                Wrote,
                Error,
                Write
            };

            void start( const Options& options )
            {
                m_options = options;
                onStart( options );
            }
            
            static const gen::Generators& populate();
            static ui type()
            {
                return typeid( Set ).hash_code();
            }
            static Set& get( const si::Data& name )
            {
                return dynamic_cast < Set& > ( gen::grain( type(), name ) );
            }
            
            const Options& options() const
            {
                return m_options;
            }
            
            const Till& in( ) const
            {
                return path( In );
            }

            Till& in( )
            {
                return path( In );
            }

            Till& out( )
            {
                return path( Out );
            }
            
            const tau::Error& error() const
            {
                return m_error;
            }
            void setError( const tau::Error& error )
            {
                if ( error.message.length( ) )
                {
                    m_error = error;
                    onError( );
                }
            }
            void onData()
            {
                send( Data );
            }
            void onWrote()
            {
                send( Wrote );
            }
        protected:
            Set(  );
            
            void send( ui type )
            {
                in::Male::dispatch( type );
            }
            
            void onError()
            {
                send( Error );
            }
            
            void onEnd()
            {
                send( End );
            }
            virtual void inWrite()
            {
                send( Write );
            }
            
                    
        private:
            virtual void onStart( const Options& ) = 0;
            
            virtual void cleanup()
            {
                in::Male::cleanup();
                after();
            }
            
            virtual void after() = 0;
            
            struct Path
            {
                Till* _till;
                Set* _set;
                Type type;
                
                Path()
                : _till( NULL ), _set( NULL )
                {
                    
                }
                
                void clear();   
                
                Till& till();
                const Till& till() const
                {
                    assert( _till );
                    return *_till;
                }
                
                void assign( Set& set, Type type )
                {
                    _set = &set;
                    this->type = type;
                }
                
                Set& set()
                {
                    assert( _set );
                    return *_set;
                }
                
                operator const Till&() const
                {
                    return till();
                }
                operator Till&()
                {
                    return till();
                }
                
            };
            
            Path& path( Type type )
            {
                return m_paths[ type ];
            }
            const Path& path( Type type ) const
            {
                return m_paths[ type ];
            }
            
            void tillWrite( Grain& )
            {
                inWrite();
            }
            
        private:
            Options m_options;
            Path m_paths[ 2 ];
            tau::Error m_error;
            
        };
        
        class Client: public ev::File::Client
        {
         public:
            void setSet( Set& set )
            {
                m_set = &set;
            }
            
            virtual ui doWrite();
            virtual ui doRead( );
            
        protected:
            Client( )
            : m_set( NULL )
            {
                
            }
            
        private:
            Set& set()
            {
                assert( m_set );
                return *m_set;
            }
            
        private:
            Set* m_set;
        };

        class Net: public Set, public io::Client, public ev::Link
        {
        public:

            enum 
            {
                Accept = Set::End + 100
            };
            
            static Grain* create( )
            {
                return dynamic_cast < Grain* > ( &tok< Net >()() );
            }
            Net( );
            virtual ~Net( )
            {
                
            }

            bool remote()
            {
                return parent();
            }
            Net* parent() const
            {
                return m_parent;
            }
            
            void enable()
            {
                Rock::ref();
                io::Client::read();
            }
            
            bool server() const
            {
                return m_server;
            }
            
        protected:            
            void open( si::Link::Accept* accept = NULL );
            
        private:
            
            virtual void onStart( const Set::Options& options );
            
            virtual void after( );
            
            void accept( );
            
            void setParent( Net* parent )
            {
                m_parent = parent; 
            }
            
            virtual ui code( ) const
            {
                return typeid ( *this ).hash_code( );
            }
            
            virtual void inWrite();
            
            void resetTimer();
            
            void timer( Grain& );
            void onError( const tau::Error& error, bool sync = false );
            
            
            virtual ui doRead( );
            virtual void onWrite(); 
            

        private:            
            Net* m_parent;
            tau::Event* m_write;
            Time m_timeout;
            si::Link m_link;
            bool m_server;
        };

        class Bot : public Set
        {
        public:
            Bot( );
            virtual ~Bot()
            {
                
            }
            static Grain* create( )
            {
                return dynamic_cast < Grain* > ( &tok< Bot >()() );
            }

            void send( ui signal ) const
            {
                m_process.signal( signal );
            }
            void terminate( )
            {
                m_process.kill( );
            }
            int status( ) const;
            ui pid( ) const
            {
                return m_process.pid( );
            }
            
            const si::Data& command() const
            {
                return options()[ "command" ];
            }

        private:
            virtual void onStart( const Set::Options& options );
            
            virtual ui code( ) const
            {
                return typeid ( *this ).hash_code( );
            }
            
            virtual void inWrite( )
            {
                writeStream().write();
            }
            
            class Stream : public io::Client, public ev::File
            {
            public:
                enum Type
                {
                    In = si::File::In,
                    Out,
                    Err
                };

                virtual ~Stream( )
                {
                }

                int type( ) const
                {
                    return m_type;
                }
                
                Stream( )
                : m_type( 0 ), m_bot( NULL ), ev::File( dynamic_cast< ev::File::Client& > ( *this ) )
                {
                }
                
                void assign( Bot& bot, const si::Process::Stream& stream );
                
                
                virtual void onRead( );
                virtual void onWrite( );
                
                
            private:
                Bot& process()
                {
                    assert( m_bot );
                    return *m_bot;
                }
                
            private:
                int m_type;
                Bot* m_bot;
            };

            void timer( Grain& );
            Stream& writeStream( )
            {
                return m_streams[ Stream::In ];
            }
            Stream& readStream( )
            {
                return m_streams[ Stream::Out ];
            }
            const Stream& readStream( ) const
            {
                return m_streams[ Stream::Out ];
            }
            Stream& errorStream( )
            {
                return m_streams[ Stream::Err ];
            }
            
            virtual void after( );
            
        private:
            Stream m_streams[ 3 ];
            int m_code;
            si::Process m_process;
            Event* m_timer;
        };
        
//        class File : public Set, public ev::File
//        {
//        public:
//            enum
//            {
//                Read = Set::End + 200
//            };
//            
//            File( );
//            
//            static Grain* create( )
//            {
//                return dynamic_cast < Grain* > ( &tok< File > ()() );
//            }
//            static File& get( const si::Data& name );
//
//            virtual ~File( )
//            {
//            }
//            
//            virtual void onStart( const Set::Options& options );
//
//            const si::Data& name( ) const
//            {
//                return options( )[ "name" ];
//            }
//
//            void read( ui length = 0 );
//
//            virtual ui code( ) const
//            {
//                return typeid ( *this ).hash_code( );
//            }
//            virtual void after( )
//            {
//                ev::File::reset();
//            }
//            
//            virtual void onProcess( ev::Event& event )
//            {
//                event.setInterval( Ticks( 0 ) );
//            }
//
//            virtual void init( )
//            {
//                m_size = 0;
//                m_read = 0;
//                ev::File::setup( );
//                Set::init( );
//            }
//            
//        private:
//            virtual ui doRead( );
//            virtual ui available();
//            
//            void onTimer( Grain& );            
//            
//        private:
//            ui m_read;
//            ui m_length;
//            ui m_size;
//            
//        };
    }
}

#endif	
