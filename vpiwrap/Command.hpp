#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <cstring>
#include "boost/noncopyable.hpp"

#include "util.hpp"
#include "Simulator.hpp"
#include "ProcessManager.hpp"

class Process;

class Command: public SimulatorCallback, public boost::noncopyable
{
protected:
    Process* _proc;
    ProcessManager* _manager;

public:
    Command( Process* proc ):
        _proc(proc)
    {}

    virtual ~Command() 
    {}

    void setManager( ProcessManager* manager )
    {
        _manager = manager;
    }
    virtual void execute() = 0;
};


class VPICommand: public Command
{
protected:
    vpi_descriptor _desc;

public:
    VPICommand( Process* proc ):
        Command( proc )
    {
        memset(&_desc, 0, sizeof(_desc));
    }
};


class DelayCommand: public VPICommand
{
    int _cycle;

public:
    DelayCommand( Process* proc, int cycle ): 
        VPICommand( proc ),
        _cycle( cycle )
    {}

    // override
    void execute()
    {
        _desc.time.type = vpiSimTime;
        _desc.time.high = 0;
        _desc.time.low  = _cycle;

        _desc.cbdata.reason    = cbAfterDelay;
        //cbdata.cb_rtn       = ... set by Simulator ...
        _desc.cbdata.obj       = NULL;
        _desc.cbdata.time      = &_desc.time;
        _desc.cbdata.value     = NULL;
        //_desc.cbdata.user_data = ... set by Simulator ...

        _manager->getSimulator().registerCallback( this, &_desc );
    }

    void called()
    {
        _manager->raise( _proc );
    }
};


#endif
