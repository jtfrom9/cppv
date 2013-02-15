#include <string>
#include <sstream>
#include <list>
#include <stdexcept>
#include <algorithm>

using std::string;
using std::stringstream;
using std::list;
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

    bool _finished;

    // dtor
    ~ProcessManagerImpl()
    {}

public:
    // ctor
    ProcessManagerImpl( const Simulator* sim ): 
        _sim(sim), 
        _current(0),
        _finished(false)
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
        //cout << dump() << endl;
        proc->resume();// context switch
        _current = 0;
    
        //cout << __func__ << ": " << proc->name() << " yield." << endl;

        if((req = proc->receive()) !=0 ) {
            //cout << __func__ << ": " << proc->name() << " recv=" << req->str() << endl;

            req->setManager( this );
            // do request from Process by yield_send() methods
            req->execute();
        }
    }

public:
    void schedule()
    {
        if (_finished) return;

        //cout << __func__ << ": " << "entered." << endl;

        while(!_run_list.empty()) {
            
            // all entries in _run_list are moved to _active_list
            while(!_run_list.empty()) {
                Process* proc = _run_list.front();
                _run_list.pop_front();
                _active_list.push_back( proc );
            }

            process_container temp_end_list;

            // run all processes
            while(!_active_list.empty()) {
                Process* proc = _active_list.front();
                _active_list.pop_front();

                try 
                {
                    switch_to( proc );
                } 
                catch(const FinishSimulation& e) 
                {
                    // finish() called.
                    _sim->finish();
                    _finished = true;
                    return;
                }
        
                if( proc->is_end() ) {
                    temp_end_list.push_back( proc ); // to end list
                }
            }

            while(!temp_end_list.empty()) {
                Process* proc = temp_end_list.front();
                temp_end_list.pop_front();
                _end_list.push_back( proc );
            }
        }

        //cout << __func__ << ": " << "exit." << endl;
    }

public:
    void raise( Process* proc ) 
    {
        if (_finished) return;

        run( proc );
        schedule();
    }

    const Simulator& getSimulator() 
    {
        return *_sim;
    }

private:
    const char* dump() const;

}; // ProcessManagerImpl

const char* ProcessManagerImpl::dump() const
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


