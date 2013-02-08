#include <vector>

#include "objects.hpp"
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
    m.add(new MainProcess());
    
    // first schedule
    m.schedule(); 

    return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

void initialCallback()
{
  vpiHandle href;
  static s_cb_data cbrec;
  p_cb_data p_cb;
  p_cb = &cbrec;
  p_cb->reason    = cbStartOfSimulation;
  p_cb->cb_rtn    = startup;
  p_cb->obj       = NULL;
  p_cb->time      = NULL;
  p_cb->value     = NULL;
  p_cb->user_data = NULL;
  
  if((href = vpi_register_cb(p_cb)) == NULL){
    vpi_printf("ERROR: VPI NOT HOOKED.");
  }

  //registerSysTask();
}

void (*vlog_startup_routines[])() = {
    initialCallback,
    0
};


PLI_INT32 load_veriepy_script(PLI_BYTE8 *arg)
//PLI_INT32 load_veriepy_script()
{
/*
  vpiHandle systf = vpi_handle(vpiSysTfCall,NULL);
  vpiHandle iter;
  //int count = 0;
  
  if((iter = vpi_iterate(vpiArgument, systf)) != NULL) {
    vpiHandle arg;
    
    // pybench
    if((arg = vpi_scan(iter))!=NULL) {
      char* script = vpiObjValueString(arg);
      if(script!=NULL) {
        vpi_printf("$load_veriepy_script: %s\n",script);
        importUserScript(script);
        free(script);
      }
    }
    }*/

    vpi_printf((char*)"systask!!!!\n");
    startup(0);
  return 0;
}

// void registerSysTask()
// {
//     typedef PLI_INT32 func_t();

//     static s_vpi_systf_data systf_;
//     p_vpi_systf_data systf = &systf_;
//     systf->type        = vpiSysTask;
//     systf->tfname      = (char*)"$load_veriepy_script";
//     systf->compiletf   = NULL;
//     systf->sizetf      = NULL;
//     //systf->calltf      = (func_t*)load_veriepy_script;
//     systf->calltf      = load_veriepy_script;
//     vpi_register_systf(systf);
// }


#ifdef __cplusplus
}
#endif
