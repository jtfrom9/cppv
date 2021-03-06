#ifndef PROCESSMANAGER_HPP
#define PROCESSMANAGER_HPP

#include "util.hpp"
#include "Process.hpp"

namespace vpi {

class Process;
class Simulator;

class ProcessManager: public noncopyable
{
protected:
    virtual ~ProcessManager()
    {}

public:
    // system functions
    virtual void schedule()              = 0;
    virtual void raise( Process* proc )  = 0;
    virtual void run( Process* proc )    = 0;

    // user functions
    virtual Process* getCurrent() const  = 0;
    virtual void regist( Process* proc ) = 0;

    virtual const Simulator& getSimulator() = 0;

    // singleton APIs
    static void create( Simulator* sim );
    static ProcessManager& get();

};

} // namespace vpi
#endif
