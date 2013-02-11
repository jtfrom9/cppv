#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <cstring>
#include "boost/noncopyable.hpp"
#include "boost/bind.hpp"

#include "util.hpp"
#include "Simulator.hpp"
#include "ProcessManager.hpp"

class Process;

class Command: public boost::noncopyable
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


class VPICommand: public Command, public SimulatorCallback
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

    // override (Command)
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

    // override (SimulatorCallback)
    void called()
    {
        _manager->raise( _proc );
    }
};


class CreateProcessCommand: public Command
{
    Process* _newproc;

public:
    CreateProcessCommand( Process* self, Process* newproc ):
        Command( self ), 
        _newproc( newproc )
    {}

    // override (Command)
    void execute()
    {
        _manager->add_run( _proc );
        _manager->add( _newproc );
    }
};


class WaitProcessCommand: public Command
{
    Process* _waitproc;

public:
    WaitProcessCommand( Process* self, Process* waitproc ):
        Command( self ),
        _waitproc( waitproc )
    {}

    // override (Command)
    void execute()
    {
        ProcessManager::hook_handler_t f = boost::bind(&WaitProcessCommand::waithook, this);
        _manager->addWaitHook( _waitproc,  f);
    }

    void waithook()
    {
        _manager->raise( _proc );
    }
};

#endif
