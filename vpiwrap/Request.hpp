#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <cstring>
#include <string>

#include "boost/noncopyable.hpp"
#include "boost/bind.hpp"
#include "boost/function.hpp"

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
    // ctor
    Request( Process* proc ):
        _proc(proc)
    {}

    // dtor
    virtual ~Request() 
    {}

    void setManager( ProcessManager* manager )
    {
        _manager = manager;
    }
    virtual void execute() = 0;
    virtual std::string to_str() = 0;

    const char* dump()
    {
        static char msg[1024];
        strcpy(msg, to_str().c_str());
        return msg;
    }
};


class DelayRequest: public Request, public SimulatorCallback
{
    int _cycle;

public:
    // ctor
    DelayRequest( Process* proc, int cycle ): 
        Request( proc ),
        _cycle( cycle )
    {}

    // override (Request)
    void execute()
    {
        _manager->getSimulator().setAfterDelayCallback( this, _cycle );
    }

    // override (SimulatorCallback)
    void called()
    {
        _manager->raise( _proc );
    }

    std::string to_str() { 
        return std::string("DelayRequest: proc=") + _proc->name();
    }
};

class WaitValueChangeRequest: public Request, public SimulatorCallback
{
    VPIObject::ptr _obj;

public:
    WaitValueChangeRequest( Process* proc, VPIObject::ptr obj ):
        Request( proc ),
        _obj( obj )
    {}

    virtual ~WaitValueChangeRequest()
    {
        _obj->unsetCallback( this );
    }

    void execute() 
    {
        _obj->setValueChangedCallback( this );
    }

    void called()
    {
        _manager->raise( _proc );
    }
    
    std::string to_str() { 
        return std::string("WaitValueChangeRequest: proc=") + _proc->name();
    }
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
        _manager->run( _proc );
        _manager->regist( _newproc );
    }

    std::string to_str() { 
        return std::string("CreateProcessRequest: proc=") + _newproc->name(); 
    }
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
        _manager->addHook( _waitproc, 
                           boost::bind( &WaitProcessRequest::waithook, this ) );
    }

    void waithook()
    {
        _manager->raise( _proc );
    }

    std::string to_str() { 
        return std::string("WaitProcessRequest: proc=") + _waitproc->name(); 
    }
};

#endif
