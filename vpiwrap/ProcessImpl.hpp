#ifndef PROCESSIMPL_HPP
#define PROCESSIMPL_HPP

#include <list>
#include <string>

#include <boost/shared_ptr.hpp>
using boost::shared_ptr;

#include "Process.hpp"
#include "generator.hpp"
#include "Command.hpp"

class ProcessImpl: public Process, public generic_generator<Command*>
{
private:
    typedef shared_ptr<Command> res;

public:
    ProcessImpl(const char* name):
        Process(name)
    {}

    void wait( int cycle ) {
        yield_send( res(new DelayCommand(this, cycle)).get() );
    }

    void wait(ISignal signal) {
        yield();
    }
};


class ProcessManagerImpl: public ProcessManager
{
private:
    const Simulator *_sim;
    ProcessImpl* _current;

    typedef std::list<ProcessImpl*> process_container;
    process_container _processes;
    process_container _end_processes;

    void switch_to( ProcessImpl* proc );

    // cannnot be deleted temporary
    virtual ~ProcessManagerImpl()
    {}

public:
    ProcessManagerImpl( const Simulator* sim ): 
        _sim(sim), 
        _current(0)
    {}

    Process* getCurrent() const {
        if (_current==0)
            throw std::runtime_error(std::string(__func__) + ": _current==0");
        return _current;
    }

    void add( Process* proc );
    void schedule();
    void raise( Process* proc );

    const Simulator& getSimulator() { return *_sim; }
};



int vmain(int argc, char* argv[]);

class MainProcess: public ProcessImpl
{
public:
    MainProcess():
        ProcessImpl("MainProcess")
    {}

protected:
    void body() {
        vmain(0,0);
    }
};

#endif
