#include <string>
#include <vector>
#include <list>
#include <stdexcept>
#include <algorithm>

using std::vector;
using std::string;
using std::invalid_argument;
using std::runtime_error;

#include "util.hpp"
#include "Process.hpp"
#include "ProcessManager.hpp"
#include "Command.hpp"
#include "generator.hpp"

class ProcessManagerImpl: public ProcessManager
{
private:
    const Simulator *_sim;
    Process* _current;

    typedef std::list<Process*> process_container;
    process_container _processes;
    process_container _end_processes;

    
    // private
    void switch_to( Process* proc )
    {
        Command* command;
    
        _current = proc;
        try {
            proc->next();// context switch
        } catch( const stop_iteration& e ) {
            cout << "Process End" << endl;
        }
        _current = 0;
    
        if((command = proc->receive()) !=0 ) {
            command->setManager( this );
            // do request from Process by yield_send() methods
            command->execute();
        }
    }

    // cannnot be deleted temporary
    virtual ~ProcessManagerImpl()
    {}

public:
    // ctor
    ProcessManagerImpl( const Simulator* sim ): 
        _sim(sim), 
        _current(0)
    {}

    // process operation
    Process* getCurrent() const {
        if (_current==0)
            throw std::runtime_error(std::string(__func__) + ": _current==0");
        return _current;
    }

    // public override
    void add( Process* proc ) {
        if (proc==0)
            throw invalid_argument("process == null");

        if (std::find(_processes.begin(), _processes.end(), proc) != _processes.end())
            throw invalid_argument("already registered");

        _processes.push_back( proc );
    }

    // public override
    void schedule()
    {
        process_container temp;
        Process* proc;

        while(!_processes.empty()) {
            proc = _processes.front();
            _processes.pop_front();

            switch_to( proc );
        
            // if not end save to temp ( restore later... )
            if( !proc->end() ) 
                temp.push_back( proc );
            else
                _end_processes.push_back( proc );
        }

        // restore to _processes
        _processes.assign( temp.begin(), temp.end() );
    }
    
    // public override
    void raise( Process* proc )
    {
        switch_to( proc );

        if( proc->end() ) {
            try {
                _processes.erase( std::find( _processes.begin(), _processes.end(), proc ) );
            } catch (const std::exception& e) {
                throw invalid_argument(string(__func__) + ": fail to erase.");
            }
            _end_processes.push_back( proc );
        }
    }

    // 
    const Simulator& getSimulator() 
    {
        return *_sim;
    }
};




static ProcessManagerImpl* managerInst = 0;

// singleton
void ProcessManager::create( Simulator* sim )
{
    if(managerInst!=0) {
        throw std::runtime_error("ProcessManager already created");
    }
    managerInst = new ProcessManagerImpl( sim );
}

// singleton
ProcessManager& ProcessManager::get()
{
    if(managerInst==0) {
        throw std::runtime_error("ProcessManager Not created yet");
    }
    return *managerInst;
}


