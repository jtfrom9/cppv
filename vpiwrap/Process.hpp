#ifndef PROCESS_HPP
#define PROCESS_HPP

#include "boost/noncopyable.hpp"

#include "Simulator.hpp"

class ISignal {};

class Process: public boost::noncopyable
{
private:
    const char* _name;

public:
    Process( const char* name ): _name(name)
    {}
    virtual ~Process() {} 

    const char* name() const { return _name; }

    virtual void wait(int cycle) = 0;
    virtual void wait(ISignal signal) = 0;
};


class ProcessManager: public boost::noncopyable
{
public:
    virtual ~ProcessManager() {}

    // process operation
    virtual Process* getCurrent() const = 0;
    virtual void add( Process* proc )   = 0;
    virtual void schedule()             = 0;
    virtual void raise( Process* proc ) = 0;

    // 
    virtual const Simulator& getSimulator()  = 0;

    // singleton APIs
    static void create( Simulator* sim );
    static ProcessManager& get();
};


class UserProcess: public Process
{
};

#endif
