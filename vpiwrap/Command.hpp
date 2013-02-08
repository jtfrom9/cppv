#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <cstring>

#include "Process.hpp"
#include "Simulator.hpp"
#include "util.hpp"

class Command: public SimulatorCallback
{
public:
    typedef enum { DELAY, VALUE_CHANGE, EXPR } type_t;

protected:
    type_t _type;
    Process* _proc;
    ProcessManager* _manager;

public:
    Command( type_t type, Process* proc ):
        _type(type),
        _proc(proc)
    {}

    virtual ~Command() {
        cout << "Command deleted" << endl;
    }

    type_t type() const { return _type; }
    
    virtual void execute( const Simulator& sim ) = 0;

    void setManager( ProcessManager* manager )
    {
        _manager = manager;
    }
};


class DelayCommand: public Command
{
    int _cycle;
    
public:
    DelayCommand( Process* proc, int cycle ): 
        Command( Command::DELAY, proc ),
        _cycle(cycle) 
    {}

    // override
    void execute( const Simulator& sim )
    {
        vpi_descriptor desc;

        memset(&desc,0,sizeof(desc));

        desc.time.type = vpiSimTime;
        desc.time.high = 0;
        desc.time.low  = _cycle;

        desc.cbdata.reason    = cbAfterDelay;
        //cbdata.cb_rtn       = ... set by Simulator ...
        desc.cbdata.obj       = NULL;
        desc.cbdata.time      = &desc.time;
        desc.cbdata.value     = NULL;
        //desc.cbdata.user_data = ... set by Simulator ...

        sim.registerCallback( this, &desc );
    }

    void called()
    {
        _manager->raise( _proc );
    }
};


#endif
