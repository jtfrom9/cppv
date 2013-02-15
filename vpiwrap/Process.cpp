#include <string>
#include <stdexcept>

using std::string;
using std::invalid_argument;

#include "boost/shared_ptr.hpp"
#include "boost/foreach.hpp"
using boost::shared_ptr;

#include "util.hpp"
#include "Process.hpp"
#include "Request.hpp"
#include "generator.hpp"


class Context: public generic_generator<Request*>
{
private:
    Process* _proc;

public:
    Context( Process* proc ): _proc(proc)
    {}

protected:
    void body() throw(std::exception) 
    {
        _proc->main();
    }
    friend class Process;
}; // Context


// Process

// ctor
Process::Process( const char* name ): 
    _name(name),
    _context( new Context( this )),
    _status( INIT ),
    _sleep_reason( NONE ),
    _end_reason( NORMAL )
{}

// dtor
Process::~Process()
{
    delete _context;
}

// public for ProcessManager
void Process::resume()
{
    bool terminated, abort;
    _status = RUN;
    try {
        _context->next();
    } 
    catch(const stop_iteration& e) {
        terminated = e.terminated;
        abort      = e.abort;
    }
    if (_context->end()) {
        _status = END;
        if(abort)           _end_reason = ABORT;
        else if(terminated) _end_reason = TERMINATE;
        else                _end_reason = NORMAL;
        BOOST_FOREACH( ProcessCallback* cb, _callbacks ) {
            cb->onEnd();
        }
    } else {
        _status = SLEEP;
    }
}

// public for ProcessManager
Request* Process::receive()
{
    Request* pcom;
    return _context->receive( &pcom ) ? pcom : 0;
}

// public for Request(TerminateProcessRequest)
void Process::terminate()
{
    _context->terminate();
}

// public for Request(WaitProcessRequest)
void Process::addEndCallback( ProcessCallback* cb )
{
    if (_find(_callbacks, cb)!=0) 
        throw invalid_argument(string(__func__) + (format(": already added cb=%p") % cb).str());
    _callbacks.push_back( cb );
}

// protected
void Process::delay( int cycle ) {
    // RAII, DelayRequest must be deleted when returning this function
    shared_ptr<Request> p( new DelayRequest(this, cycle) );
    _sleep_reason = DELAY;
    _context->yield_send( p.get() );
}

// protected
void Process::wait( Process* proc ) {
    shared_ptr<Request> p( new WaitProcessRequest(this, proc) );
    _sleep_reason = WAIT_FOR_PROCESS;
    _context->yield_send( p.get() );
}

// protected
void Process::wait( VPIObject* obj ) {
    shared_ptr<Request> p( new WaitValueChangeRequest(this, obj) );
    _sleep_reason = WAIT_FOR_VALUECHANGE;
    _context->yield_send( p.get() );
}

// protected
Process* Process::create( Process* newproc ) {
    shared_ptr<Request> p( new CreateProcessRequest(this, newproc) );
    _sleep_reason = CREATE_PROCESS;
    _context->yield_send( p.get() );
    return newproc;
}

// protected
void Process::terminate( Process* proc, bool block ) {
    shared_ptr<Request> p( new TerminateProcessRequest(this, proc, block) );
    _sleep_reason = TERMINATE_PROCESS;
    _context->yield_send( p.get() );
}

// protected
void Process::finish() {
    shared_ptr<Request> p( new FinishSimulationRequest(this) );
    _sleep_reason = FINISH_SIMULATION;
    _context->yield_send( p.get() );
}

// global function
void delay( int cycle )
{
    Process *currentProcess = ProcessManager::get().getCurrent();
    currentProcess->delay( cycle );
}

// global function
void wait( Process* proc )
{
    Process *currentProcess = ProcessManager::get().getCurrent();
    return currentProcess->wait( proc );
}

// global function
void wait( VPIObject* obj )
{
    Process *currentProcess = ProcessManager::get().getCurrent();
    return currentProcess->wait( obj );
}

// global function
Process* create( Process* proc )
{
    Process *currentProcess = ProcessManager::get().getCurrent();
    return currentProcess->create( proc );
}

// global function
void terminate( Process* proc, bool block )
{
    Process *currentProcess = ProcessManager::get().getCurrent();
    return currentProcess->terminate( proc, block );
}

// global function
void finish()
{
    Process *currentProcess = ProcessManager::get().getCurrent();
    return currentProcess->finish();
}

class UserProcess: public Process
{
    boost::function<void()> _func;
public:
    UserProcess( const char* name, boost::function<void()> func ): 
        Process(name),
        _func(func)
    {}

    void main() {
        _func();
    }
};

Process* create( const char* name, boost::function<void()> func )
{
    Process *currentProcess = ProcessManager::get().getCurrent();
    return currentProcess->create(new UserProcess( name, func ));
}

long long sim_time() 
{
    return ProcessManager::get().getSimulator().sim_time();
}

Module& top()
{
    return ProcessManager::get().getSimulator().getModule(0);
}
