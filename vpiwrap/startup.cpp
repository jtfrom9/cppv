#include <cstring>
#include <sys/time.h>

#include "Simulator.hpp"
#include "Process.hpp"
#include "ProcessManager.hpp"

#include "vpi_user.h"

using namespace vpi;

extern int vmain(int argc, char* argv[]);

class MainProcess: public Process
{
public:
    MainProcess():
        Process("MainProcess")
    {}

protected:

    void main() {
        s_vpi_vlog_info info;
        vpi_get_vlog_info(&info);

        char* argv[1024];
        int argc=1;
        argv[0] = info.argv[0];
        for(int i=1; i<info.argc; i++) {
            if(info.argv[i][0]=='+') {
                char* buf = (char*)malloc(strlen(info.argv[i]) + 2);
                buf[0] = '-';
                buf[1] = '-';
                strcpy(buf+2, &info.argv[i][1]);
                argv[argc] = buf;
                argc++;
            }
        }
        vmain(argc,argv);
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

static struct timeval tv_start;
static struct timeval tv_end;

static PLI_INT32 onStartSim( s_cb_data* cbdata )
{
    //vpi_printf("onStartSim\n");
    gettimeofday(&tv_start,NULL);
    return 0;
}

static PLI_INT32 onEndSim( s_cb_data* cbdata )
{
    struct timeval tv;


    gettimeofday(&tv_end,NULL);

    timersub(&tv_end,&tv_start,&tv);
    
    vpi_printf("elapsed: %lf (sec)\n", tv.tv_sec + (tv.tv_usec * 1e-6));
    return 0;
}

static void registerCb( PLI_INT32 reason, PLI_INT32 (*cb_rtn)(struct t_cb_data *) )
{
    s_cb_data cbdata;
    
    cbdata.reason    = reason;
    cbdata.cb_rtn    = cb_rtn;
    cbdata.obj       = NULL;
    cbdata.time      = NULL;
    cbdata.value     = NULL;
    cbdata.user_data = NULL;
  
    if(vpi_register_cb(&cbdata) == NULL) {
        vpi_printf("initialCallback: fail to vpi_register_cb...");
    }
}

static void registerEvalCallback()
{
    registerCb( cbStartOfSimulation, onStartSim );
    registerCb( cbEndOfSimulation,   onEndSim );
}

void initialCallback()
{
    //static s_cb_data cbdata;
    s_cb_data cbdata;
    
    cbdata.reason    = cbStartOfSimulation;
    cbdata.cb_rtn    = startup;
    cbdata.obj       = NULL;
    cbdata.time      = NULL;
    cbdata.value     = NULL;
    cbdata.user_data = NULL;
  
    if(vpi_register_cb(&cbdata) == NULL) {
        vpi_printf("initialCallback: fail to vpi_register_cb...");
    }
    
    registerEvalCallback();
}

void (*vlog_startup_routines[])() = {
    initialCallback,
    //registerSysTask,
    0
};
#ifdef __cplusplus
}
#endif
