#include "Simulator.hpp"
#include "Process.hpp"
#include "ProcessManager.hpp"

#include "vpi_user.h"

extern int vmain(int argc, char* argv[]);

class MainProcess: public Process
{
public:
    MainProcess():
        Process("MainProcess")
    {}

protected:
    void main() {
        vmain(0,0);
    }
};

int startup(s_cb_data*cpb)
{
    // initialize Simulation & ProcessManager instance
    ProcessManager::create( Simulator::create() );
    
    ProcessManager& m = ProcessManager::get();
    
    // register Main
    m.regist(new MainProcess());
    
    // first schedule
    m.schedule(); 

    return 0;
}


#ifdef __cplusplus
extern "C" {
#endif

void initialCallback()
{
    static s_cb_data cbdata;

    cbdata.reason    = cbStartOfSimulation;
    cbdata.cb_rtn    = startup;
    cbdata.obj       = NULL;
    cbdata.time      = NULL;
    cbdata.value     = NULL;
    cbdata.user_data = NULL;
  
    if(vpi_register_cb(&cbdata) == NULL) {
        vpi_printf("initialCallback: fail to vpi_register_cb...");
    }
}

void (*vlog_startup_routines[])() = {
    initialCallback,
    0
};
#ifdef __cplusplus
}
#endif
