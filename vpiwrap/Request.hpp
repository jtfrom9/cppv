#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <cstring>
#include <string>

#include "boost/noncopyable.hpp"

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
    virtual const std::string str() const = 0;

    const char* dump()
    {
        static char msg[1024];
        strcpy(msg, str().c_str());
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

    const std::string str() const { 
        return std::string("DelayRequest: proc=") + _proc->name();
    }
};

class WaitValueChangeRequest: public Request, public SimulatorCallback
{
    VPIObject* _obj;

public:
    WaitValueChangeRequest( Process* proc, VPIObject* obj ):
        Request( proc ),
        _obj( obj )
    {}

    virtual ~WaitValueChangeRequest()
    {}

    void execute() 
    {
        _obj->setValueChangedCallback( this );
    }

    void called()
    {
        _manager->raise( _proc );
    }
    
    const std::string str() const { 
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

    const std::string str() const { 
        return std::string("CreateProcessRequest: proc=") + _newproc->name(); 
    }
};


class WaitProcessRequest: public Request, public ProcessCallback
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
        if ( _waitproc->is_end() ) {
            // already end.
            _manager->run( _proc );
        } else {
            _waitproc->addEndCallback( this );
        }
    }

    void onEnd()
    {
        _manager->raise( _proc );
    }

    const std::string str() const { 
        return std::string("WaitProcessRequest: proc=") + _waitproc->name(); 
    }
};

#endif
