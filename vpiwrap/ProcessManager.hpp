#ifndef PROCESSMANAGER_HPP
#define PROCESSMANAGER_HPP

#include "boost/noncopyable.hpp"

class Process;
class Simulator;

class ProcessManager: public boost::noncopyable
{
protected:
    virtual ~ProcessManager()
    {}

public:
    virtual Process* getCurrent() const = 0;
    virtual void add( Process* proc )   = 0;
    virtual void schedule()             = 0;
    virtual void raise( Process* proc ) = 0;

    virtual const Simulator& getSimulator() = 0;

    // singleton APIs
    static void create( Simulator* sim );
    static ProcessManager& get();
};

#endif
