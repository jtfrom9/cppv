#ifndef PROCESSIMPL_HPP
#define PROCESSIMPL_HPP

#include <vector>

#include "boost/shared_ptr.hpp"

#include "Process.hpp"
#include "generator.hpp"

class ProcessImpl: public Process, generic_generator<int>
{
private:
    ProcessManager* _manager;

protected:
    void wait(int cycle) {
        yield();
    }
    void wait(ISignal signal) {
        yield();
    }

private:
    void setManager(ProcessManager* manager) {
        _manager = manager;
    }

public:
    ProcessImpl(const char* name):
        Process(name)
    {}

    typedef boost::shared_ptr<ProcessImpl> ptr;
    friend class ProcessManagerImpl;
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


class ProcessManagerImpl: public ProcessManager
{
private:
    std::vector<ProcessImpl::ptr> _processes;

public:
    Process::ptr add(Process* process);
    void start();
};


#endif
