#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <list>

#include "boost/noncopyable.hpp"
#include "boost/function.hpp"

#include "objects.hpp"

class Context;
class Request;

class ISignal {};

class ProcessCallback
{
public:
    virtual void onEnd() = 0;
};

class Process: public boost::noncopyable
{
public:
    typedef enum { INIT, RUN, SLEEP, END } status_t;
    typedef enum {
        NONE,
        DELAY,
        WAIT_FOR_PROCESS,
        WAIT_FOR_VALUECHANGE,
        CREATE_PROCESS,
        TERMINATE_PROCESS,
        FINISH_SIMULATION
    } sleep_reason_t;

    typedef enum {
        NORMAL,
        TERMINATE,
        ABORT
    } end_reason_t;

private:
    const char*    _name;
    Context*       _context;
    status_t       _status;
    sleep_reason_t _sleep_reason;
    end_reason_t   _end_reason;

    typedef std::list<ProcessCallback*> callback_container;
    callback_container _callbacks;

public:
    Process( const char* name ); // ctor
    virtual ~Process();          // dtor

    const char* name() const { return _name; }

protected:
    friend class Context;
    virtual void main() = 0;

    void delay( int cycle );
    void wait( Process* proc );
    void wait( VPIObject* obj );
    Process* create(Process* proc);
    void terminate( Process* proc, bool block = true );
    void finish();

public:
    // for ProcessManager
    void resume();
    Request* receive();
    void terminate();

    status_t status() const { return _status; }
    bool is_end() const     { return _status==END; }
    bool is_run() const     { return _status==RUN; }
    bool is_sleep() const   { return _status==SLEEP; }

    sleep_reason_t sleep_reason() const { return _sleep_reason; }
    end_reason_t end_reason() const     { return _end_reason; }

    void addEndCallback( ProcessCallback* cb );

    
    // friend functions
    friend void delay( int cycle );
    friend void wait( Process* proc );
    friend void wait( VPIObject* obj );
    friend Process* create( Process* proc );
    friend Process* create( const char* name, boost::function<void()> func );
    friend void terminate( Process* proc, bool block );
    friend void finish();
};



// global APIs
void delay( int cycle );
void wait( Process* proc );
void wait( VPIObject* proc );
Process* create( Process* proc );
Process* create( const char* name, boost::function<void()> func );
void terminate( Process* proc, bool block=true );
void finish();
long long sim_time();
Module& top();

#endif
