#include <string>
#include <sstream>
#include <list>
#include <map>
#include <stdexcept>
#include <algorithm>

using std::string;
using std::stringstream;
using std::list;
using std::map;
using std::invalid_argument;
using std::runtime_error;

#include "boost/thread.hpp"
using boost::thread;

#include "util.hpp"
#include "Process.hpp"
#include "ProcessManager.hpp"
#include "Request.hpp"
#include "generator.hpp"

class ProcessManagerImpl: public ProcessManager
{
private:
    const Simulator *_sim;
    Process* _current;

    typedef list<Process*> process_container;
    process_container _all_processes;
    process_container _run_processes;
    process_container _end_processes;

    typedef map<Process*, hook_handler_t> hook_container;
    typedef hook_container::value_type hook_entry;
    hook_container _end_hooks;

    const char* dump() const;

    void switch_to( Process* proc );
    void add_end( Process* proc );

    // dtor
    ~ProcessManagerImpl()
    {}

public:
    // ctor
    ProcessManagerImpl( const Simulator* sim ): 
        _sim(sim), 
        _current(0)
    {}

    Process* getCurrent() const;
    void add( Process* proc );
    void add_run( Process* proc );
    void schedule();
    void raise( Process* proc ) {
        add_run( proc );
        schedule();
    }
    void addWaitHook( Process* waitproc, hook_handler_t cb );

    const Simulator& getSimulator() 
    {
        return *_sim;
    }
};


// process operation
Process* ProcessManagerImpl::getCurrent() const {
    if (_current==0)
        throw std::runtime_error(std::string(__func__) + ": _current==0");
    return _current;
}

// public override
void ProcessManagerImpl::add( Process* proc ) {
    if (proc==0)
        throw invalid_argument("process == null");
    if ( _find(_all_processes, proc) != 0 )
        throw invalid_argument("already registered: _all_processes");
    _all_processes.push_back( proc );

    add_run( proc ); 
}

// public override
void ProcessManagerImpl::add_run( Process* proc )
{
    if (proc==0)
        throw invalid_argument("process == null");
    if ( _find(_run_processes, proc) != 0 )
        throw invalid_argument("already registered: _run_processes");
    _run_processes.push_back( proc );
}

// private 
void ProcessManagerImpl::add_end( Process* proc )
{
    if (proc==0)
        throw invalid_argument("process == null");
    _end_processes.push_back( proc );
    hook_container::iterator i = _end_hooks.find( proc );
    if(i!=_end_hooks.end()) {
        _end_hooks[ proc ](); // call hook
    }
}

// private
void ProcessManagerImpl::switch_to( Process* proc )
{
    Request* req;

    //cout << __func__ << ": " << proc->name() << " entered." << endl;
    
    _current = proc;
    try {
        proc->next();// context switch
    } catch( const stop_iteration& e ) {
        cout << "Process End" << endl;
    }
    _current = 0;
    
    //cout << __func__ << ": " << proc->name() << " yield." << endl;

    if((req = proc->receive()) !=0 ) {
        //cout << __func__ << ": " << proc->name() << " recv=" << req->to_str() << endl;

        req->setManager( this );
        // do request from Process by yield_send() methods
        req->execute();
    }
}

// public override
void ProcessManagerImpl::schedule()
{
    process_container temp;

    //cout << __func__ << ": " << "entered." << endl;

    while(!_run_processes.empty()) {
            
        // all entries in _run_processes are moved to temp
        while(!_run_processes.empty()) {
            Process* proc = _run_processes.front();
            _run_processes.pop_front();
            temp.push_back( proc );
        }
            
        // run all processes
        while(!temp.empty()) {
            Process* proc = temp.front();
            temp.pop_front();

            switch_to( proc );
        
            if( proc->end() ) 
                add_end( proc );
        }
    }

    //cout << __func__ << ": " << "exit." << endl;
}
    
// public override
void ProcessManagerImpl::addWaitHook( Process* waitproc, hook_handler_t cb )
{
    hook_container::iterator i = _end_hooks.find( waitproc );
    if(i==_end_hooks.end()) { 
        _end_hooks.insert(hook_entry(waitproc,cb));
    }
}

const char* ProcessManagerImpl::dump() const
{
    stringstream ss;
    ss << "prcesses: { ";
    for(process_container::const_iterator i=_run_processes.begin(); i!=_run_processes.end(); ++i) {
        ss << (*i)->name() << ",";
    }
    ss << "}, end_run_processes: { ";
    for(process_container::const_iterator i=_end_processes.begin(); i!=_end_processes.end(); ++i) {
        ss << (*i)->name() << ",";
    }
    ss << "}";
    return ss.str().c_str();
}

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


