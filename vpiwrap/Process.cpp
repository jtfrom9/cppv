#include <list>
#include <string>

#include "boost/shared_ptr.hpp"
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
    void body() {
        _proc->main();
    }
    friend class Process;
};


Process::Process( const char* name ): _name(name)
{
    _context = new Context( this );
}

Process::~Process()
{
    delete _context;
}

void Process::next()
{
    _context->next();
}

bool Process::end()
{
    return _context->end();
}

Request* Process::receive()
{
    Request* pcom;
    return _context->receive( &pcom ) ? pcom : 0;
}

// protected
void Process::delay( int cycle ) {
    // RAII, DelayRequest must be deleted when returning this function
    shared_ptr<Request> p( new DelayRequest(this, cycle) );
    _context->yield_send( p.get() );
}

// protected
void Process::wait( Process* proc ) {
    shared_ptr<Request> p( new WaitProcessRequest(this, proc) );
    _context->yield_send( p.get() );
}

// protected
void Process::wait( VPIObject* obj ) {
    shared_ptr<Request> p( new WaitValueChangeRequest(this, obj) );
    _context->yield_send( p.get() );
}

// protected
Process* Process::create( Process* newproc ) {
    shared_ptr<Request> p( new CreateProcessRequest(this, newproc) );
    _context->yield_send( p.get() );
    return newproc;
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

void finish()
{
    return ProcessManager::get().getSimulator().finish();
}

Module& top()
{
    return ProcessManager::get().getSimulator().getModule(0);
}
