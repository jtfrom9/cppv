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
    process_container _registed_procs;
    process_container _run_list;
    process_container _active_list;
    process_container _end_list;

    typedef map<Process*, hook_handler_t> hook_container;
    typedef hook_container::value_type hook_entry;
    hook_container _end_hooks;

    // dtor
    ~ProcessManagerImpl()
    {}

public:
    // ctor
    ProcessManagerImpl( const Simulator* sim ): 
        _sim(sim), 
        _current(0)
    {}

    Process* getCurrent() const 
    {
        if (_current==0)
            throw std::runtime_error(std::string(__func__) + ": _current==0");
        return _current;
    }

    void regist( Process* proc )
    {
        if (proc==0)
            throw invalid_argument("process == null");
        if ( _find(_registed_procs, proc) != 0 )
            throw invalid_argument(string("already registered: ") + proc->name());
        _registed_procs.push_back( proc );

        run( proc ); 
    }

    void run( Process* proc )
    {
        if (proc==0)
            throw invalid_argument("process == null");
        if ( _find(_registed_procs, proc) == 0 )
            throw invalid_argument(string("must be registered before run(): ") + proc->name());
        if ( _find(_run_list, proc) != 0 )
            throw invalid_argument(string("already run: ") + proc->name());
        _run_list.push_back( proc );
    }

private:
    void switch_to( Process* proc )
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

public:
    void schedule()
    {
        //cout << __func__ << ": " << "entered." << endl;
        //cout << dump() << endl;

        while(!_run_list.empty()) {
            
            // all entries in _run_list are moved to _active_list
            while(!_run_list.empty()) {
                Process* proc = _run_list.front();
                _run_list.pop_front();
                _active_list.push_back( proc );
            }
            
            // run all processes
            while(!_active_list.empty()) {
                Process* proc = _active_list.front();
                _active_list.pop_front();

                switch_to( proc );
        
                if( proc->end() ) {
                    _end_list.push_back( proc ); // to end list
                    call_hook( proc );
                }
            }
        }

        //cout << __func__ << ": " << "exit." << endl;
    }

private:
    void call_hook( Process* proc ) 
    {
        assert(proc!=0);

        hook_container::iterator i = _end_hooks.find( proc );

        // call hook
        if(i!=_end_hooks.end()) {
            _end_hooks[ proc ](); 
        }
    }

public:
    void raise( Process* proc ) {
        run( proc );
        schedule();
    }

    void addHook( Process* waitproc, hook_handler_t cb )
    {
        hook_container::iterator i = _end_hooks.find( waitproc );
        if(i==_end_hooks.end()) { 
            _end_hooks.insert(hook_entry(waitproc,cb));
        }
    }

    const Simulator& getSimulator() 
    {
        return *_sim;
    }

private:
    const char* dump() const
    {
        stringstream ss;
        ss << "cur: " << ((_current!=0) ? _current->name() : "-");
        ss << ", active: {";
        BOOST_FOREACH( Process* p, _active_list ) { ss << p->name() << ","; }
        ss << "} run: { ";
        BOOST_FOREACH( Process* p, _run_list ) { ss << p->name() << ","; }
        ss << "} end: { ";
        BOOST_FOREACH( Process* p, _end_list ) { ss << p->name() << ","; }
        ss << "}";
        return ss.str().c_str();
    }

}; // ProcessManagerImpl


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


