#ifndef PROCESS_HPP
#define PROCESS_HPP

#include "boost/shared_ptr.hpp"

class ISignal {};

class Process
{
public:
    class Command
    {
    };

private:
    const char* _name;

public:
    Process( const char* name ): _name(name)
    {}

    const char* name() const { return _name; }

    virtual void wait(int cycle) = 0;
    virtual void wait(ISignal signal) = 0;

    typedef boost::shared_ptr<Process> ptr;
};


class ProcessManager
{
public:
    virtual Process::ptr add(Process* process) = 0;
    virtual void start() = 0;
};


#endif
