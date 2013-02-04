#ifdef __cplusplus
extern "C" {
#endif

#include "vpi_user.h"

#ifndef NULL
#define NULL 0
#endif


int startup(s_cb_data*cpb)
{
vpi_printf("=== processing startup ===\n");
  //scanModules();
  return(0);
}

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
}

void (*vlog_startup_routines[])() = {
    initialCallback,
    0
};


#ifdef __cplusplus
}
#endif
