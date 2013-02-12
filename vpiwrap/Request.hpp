#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <cstring>
#include <string>

#include "boost/noncopyable.hpp"
#include "boost/bind.hpp"

#include "util.hpp"
#include "objects.hpp"
#include "Simulator.hpp"
#include "ProcessManager.hpp"

class Process;

class Request: public boost::noncopyable
{
protected:
    Process* _proc;
    ProcessManager* _manager;

public:
    Request( Process* proc ):
        _proc(proc)
    {}

    virtual ~Request() 
    {}

    void setManager( ProcessManager* manager )
    {
        _manager = manager;
    }
    virtual void execute() = 0;
    virtual std::string to_str() = 0;
};


class VPIRequest: public Request, public SimulatorCallback
{
protected:
    vpi_descriptor _desc;

public:
    VPIRequest( Process* proc ):
        Request( proc )
    {
        memset(&_desc, 0, sizeof(_desc));
    }
};


class DelayRequest: public VPIRequest
{
    int _cycle;

public:
    DelayRequest( Process* proc, int cycle ): 
        VPIRequest( proc ),
        _cycle( cycle )
    {}

    // override (Request)
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
        return (std::string("DelayRequest: proc=") + _proc->name()).c_str();
    }

    std::string to_str() { return std::string(dump()); }
};

class WaitValueChangeRequest: public VPIRequest
{
    VPIObject::ptr _obj;

public:
    WaitValueChangeRequest( Process* proc, VPIObject::ptr obj ):
        VPIRequest( proc ),
        _obj( obj )
    {}

    void execute() 
    {
        _desc.time          = { vpiSimTime };
        _desc.value         = { vpiBinStrVal };
        _desc.cbdata.reason = cbValueChange;
        _desc.cbdata.obj    = _obj->handle();
        _desc.cbdata.value  = NULL;
        _manager->getSimulator().registerCallback( this, &_desc );
    }

    void called()
    {
        _manager->getSimulator().unregisterCallback( this );
        _manager->raise( _proc );
    }
    
    const char* dump() {
        return (std::string("WaitValueChangeRequest: proc=") + _proc->name()).c_str();
    }

    std::string to_str() { return std::string(dump()); }
};


class CreateProcessRequest: public Request
{
    Process* _newproc;

public:
    CreateProcessRequest( Process* self, Process* newproc ):
        Request( self ), 
        _newproc( newproc )
    {}

    // override (Request)
    void execute()
    {
        _manager->add_run( _proc );
        _manager->add( _newproc );
    }

    std::string to_str() { return std::string("CreateProcessRequest: proc=") + _newproc->name(); }
};


class WaitProcessRequest: public Request
{
    Process* _waitproc;

public:
    WaitProcessRequest( Process* self, Process* waitproc ):
        Request( self ),
        _waitproc( waitproc )
    {}

    // override (Request)
    void execute()
    {
        ProcessManager::hook_handler_t f = boost::bind(&WaitProcessRequest::waithook, this);
        _manager->addWaitHook( _waitproc,  f);
    }

    void waithook()
    {
        _manager->raise( _proc );
    }

    std::string to_str() { return std::string("WaitProcessRequest: proc=") + _waitproc->name(); }
};

#endif
