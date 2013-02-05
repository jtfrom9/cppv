#include <vector>

#include <objects.hpp>
#include <scheduler.hpp>

#include <generator.hpp>

#include "vpi_user.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL 0
#endif
/*
// vpi::Port wrapper
class Signal {
public:
    Signal& operator=(int v) {
        return *this;
    }
    Signal& operator=(bool b) {
        return *this;
    }
};

class SignalValue {
};

// interface in SystemVerilog
class SignalGroup {
};

// drive signals
class Driver {
};

namespace System {
void finish() {}
void stop() {}
}
*/

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

int startup(s_cb_data*cpb)
{
    ::initialize();
    
    //test( modules );

    TimerEvent *te = new TimerEvent();
    te->add(shared_ptr<TimerHandler1>(new TimerHandler1()));
    te->set(10);
    te->set(30);
    te->set(50);
    te->set(70);
    te->set(90);
    return(0);
}

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


#ifdef __cplusplus
}
#endif
