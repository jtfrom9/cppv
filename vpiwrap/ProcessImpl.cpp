#include "ProcessImpl.hpp"
#include "util.hpp"

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <string>

using std::vector;
using std::string;
using std::invalid_argument;
using std::runtime_error;

static ProcessManagerImpl* managerImpl = 0;

// singleton
void ProcessManager::create( Simulator* sim )
{
    if(managerImpl!=0) {
        throw std::runtime_error("ProcessManager already created");
    }
    managerImpl = new ProcessManagerImpl( sim );
}

// singleton
ProcessManager& ProcessManager::get()
{
    if(managerImpl==0) {
        throw std::runtime_error("ProcessManager Not created yet");
    }
    return *managerImpl;
}


// public override
void ProcessManagerImpl::add( Process* proc )
{
    if (proc==0)
        throw invalid_argument("process == null");

    if (std::find(_processes.begin(), _processes.end(), proc) != _processes.end())
        throw invalid_argument("already registered");

    ProcessImpl *procImpl = dynamic_cast<ProcessImpl*>( proc );
    if( procImpl == 0 )
        throw invalid_argument(string(__func__) + ": invalid process");
    _processes.push_back( procImpl );
}

// private
void ProcessManagerImpl::switch_to( ProcessImpl* proc )
{
    Command* command;
    
    _current = proc;
    try {
        proc->next(); // context switch
    } catch( const stop_iteration& e ) {
        cout << "Process End" << endl;
    }
    _current = 0;
    
    if(proc->receive( &command )) {
        command->setManager( this );
        // do request from Process by yield_send() methods
        command->execute( *_sim );
    }
}

// public override
void ProcessManagerImpl::schedule()
{
    process_container temp;
    ProcessImpl* procImpl;

    while(!_processes.empty()) {
        procImpl = _processes.front();
        _processes.pop_front();

        switch_to( procImpl );
        
        // if not end save to temp ( restore later... )
        if( !procImpl->end() ) 
            temp.push_back( procImpl );
        else
            _end_processes.push_back( procImpl );
    }

    // restore to _processes
    _processes.assign( temp.begin(), temp.end() );
}

// public override
void ProcessManagerImpl::raise( Process* proc )
{
    ProcessImpl* procImpl = dynamic_cast<ProcessImpl*>(proc);
    if (procImpl==0)
        throw invalid_argument(string(__func__) + ": invalid process");
    switch_to( procImpl );

    if( procImpl->end() ) {
        try {
            _processes.erase( std::find( _processes.begin(), _processes.end(), procImpl ) );
        } catch (const std::exception& e) {
            throw invalid_argument(string(__func__) + ": fail to erase.");
        }
        _end_processes.push_back( procImpl );
    }
}

/*
struct test_gen: public generic_generator<int>
{
    std::vector<Module::ptr> modules;

    void wait(int cycle) {
        yield_send(cycle);
    }
    
    void body() {
        Reg::ptr p = modules[0]->get_reg("clk");
        //Port::ptr p = modules[0]->_modules[0]->get_port("clk");
        p->write(1);
        wait(10);
    }
};

void test( const std::vector<Module::ptr>& modules )
{
    test_gen tg;
    tg.modules = modules;

    try {
        while(true) {
            int cycle;
            tg.next();
            if(tg.receive(&cycle)) {
                // sleep cycle time;
                std::cout << "sleep: " << cycle << std::endl;
                settimer( 100, 

            }
        } catch(const stop_iteration& e) {
            std::cout << "fin" << std::endl;
        }
    }
}
*/

