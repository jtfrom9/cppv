#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <cstring>
#include <string>

#include "boost/noncopyable.hpp"

#include "util.hpp"
#include "Object.hpp"
#include "Simulator.hpp"
#include "ProcessManager.hpp"

namespace vpi {

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
    IReadableSignal* _obj;

public:
    WaitValueChangeRequest( Process* proc, IReadableSignal* obj ):
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


class TerminateProcessRequest: public Request, public ProcessCallback
{
    Process* _termproc;
    bool _block;

public:
    TerminateProcessRequest( Process* self, Process* termproc, bool block ):
        Request( self ),
        _termproc( termproc )
    {}
    
    void execute()
    {
        if (_termproc->is_end() ) {
            // already end.
            _manager->run( _proc );
        } 
        else {
            _termproc->terminate();
            if( !_block ) {
                // immediately resume
                _manager->run( _proc ); 
            } else {
                // late resume
                _termproc->addEndCallback( this );
            }
        }
    }

    void onEnd()
    {
        // late resume
        _manager->raise( _proc );
    }

    const std::string str() const { 
        return std::string("TerminateProcessRequest: proc=") + _termproc->name(); 
    }
};


class FinishSimulationRequest: public Request
{
public:
    FinishSimulationRequest( Process* self ): Request( self )
    {}
    
    void execute()
    {
        cout << format("# %0d finish()") % sim_time() << endl;
        throw FinishSimulation();
    }

    const std::string str() const { 
        return std::string("FinishSimulationRequest");
    }
};

} // namespace vpi
#endif
