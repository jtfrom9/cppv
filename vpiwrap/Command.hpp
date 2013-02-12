#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <cstring>
#include <string>

#include "boost/noncopyable.hpp"
#include "boost/bind.hpp"

#include "util.hpp"
#include "objects.hpp"
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
    virtual std::string to_str() = 0;
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

    const char* dump() {
        return (std::string("DelayCommand: proc=") + _proc->name()).c_str();
    }

    std::string to_str() { return std::string(dump()); }
};

class WaitValueChangeCommand: public VPICommand
{
    VPIObject::ptr _obj;

public:
    WaitValueChangeCommand( Process* proc, VPIObject::ptr obj ):
        VPICommand( proc ),
        _obj( obj )
    {}

    void execute() 
    {
        _desc.time.type = vpiSimTime;
        _desc.time.high = 0;
        _desc.time.low  = 10;
        //_desc.time = {vpiSimTime};

        // _desc.value.format = vpiSimTime;
        _desc.value = {vpiBinStrVal};

        _desc.cbdata.reason = cbValueChange;
        _desc.cbdata.obj    = _obj->handle();
        //_desc.cbdata.time   = &_desc.time;
        //_desc.cbdata.value  = &_desc.value;
        _desc.cbdata.value  = NULL;
        _manager->getSimulator().registerCallback( this, &_desc );
    }

    void called()
    {
        _manager->getSimulator().unregisterCallback( this );
        _manager->raise( _proc );
    }
    
    const char* dump() {
        return (std::string("WaitValueChangeCommand: proc=") + _proc->name()).c_str();
    }

    std::string to_str() { return std::string(dump()); }
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

    std::string to_str() { return std::string("CreateProcessCommand: proc=") + _newproc->name(); }
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

    std::string to_str() { return std::string("WaitProcessCommand: proc=") + _waitproc->name(); }
};

#endif
