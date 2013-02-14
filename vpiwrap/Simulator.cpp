#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include "util.hpp"
#include "objects.hpp"
#include "Simulator.hpp"

using std::string;
using std::vector;
using std::stringstream;
using std::invalid_argument;
using std::runtime_error;


class Request;

class SimulatorImpl: public Simulator
{
private:
    vector<Module*> _modules;
    mutable int count;

    struct vpi_timer_event_descriptor
    {
        s_cb_data cbdata;
        s_vpi_time time;
        s_vpi_value value;
        vpiHandle handle;
        SimulatorCallback* cb;

        vpi_timer_event_descriptor() 
        {
            memset(&cbdata,0,sizeof(cbdata));
            memset(&time,0,sizeof(time));
            memset(&value,0,sizeof(value));
            time.type    = vpiSuppressTime;
            value.format = vpiSuppressVal;
        }
    };

public:
    // ctor
    SimulatorImpl() 
    {
        Simulator::scanModules(_modules);
        count = 0;
    }

    VPIObject& getObject(const char* path) const
    {
        throw not_implemented(string(__func__));
    }

    int numOfModule() const 
    { 
        return _modules.size(); 
    }
    
    Module& getModule( int index ) const
    {
        return (*_modules[index]);
    }

    Module& getModule( const char* path ) const
    {
        vector<Module*>::const_iterator pm;
        if((pm = std::find_if(_modules.begin(), _modules.end(), VPIObject::predNameOf(path))) == _modules.end()) {
            throw invalid_argument(string(__func__) + ": not found: " + path);
        }
        return **pm;
    }

private:
    static void timeElapsed( s_cb_data* pcbdata )
    {
        vpi_timer_event_descriptor* desc = (vpi_timer_event_descriptor*)pcbdata->user_data;
        SimulatorCallback *cb            = desc->cb;
        vpiHandle handle                 = desc->handle;

        delete desc;

        if( vpi_remove_cb(handle)==0 ) {
            s_vpi_error_info error;
            if(vpi_chk_error(&error)) {
                throw SimulatorError(string(__func__) + ": fail to vpi_remove_cb: " + error.message);
            }
        }
        cb->called();
    }

    void setCallback( SimulatorCallback* cb, vpi_timer_event_descriptor* desc ) const
    {
        desc->cbdata.cb_rtn    = (vpi_callback_handler_t*)timeElapsed;
        desc->cbdata.user_data = (PLI_BYTE8*)desc;
        desc->cbdata.time      = &desc->time;
        desc->cbdata.value     = &desc->value;
        desc->cb               = cb;

        if((desc->handle = vpi_register_cb( &desc->cbdata )) == NULL) {
            s_vpi_error_info error;
            if(vpi_chk_error(&error)) {
                throw SimulatorError(string(__func__) + ": fail to vpi_register_cb: " + error.message);
            }
        }
    }

public:
    void setAfterDelayCallback( SimulatorCallback* cb, int delay ) const
    {
        vpi_timer_event_descriptor* desc = new vpi_timer_event_descriptor();

        desc->time.type = vpiSimTime;
        desc->time.high = 0;
        desc->time.low  = delay; // currently low time only

        desc->cbdata.reason    = cbAfterDelay;
        desc->cbdata.obj       = NULL;

        setCallback( cb, desc );
    }

    long long sim_time() const
    {
        s_vpi_time time;
        std::memset(&time,0,sizeof(time));
        time.type = vpiSimTime;
        vpi_get_time(NULL,&time);
        return (long long)time.high << 32 | (long long)time.low;
    }

    void finish( int arg ) const 
    {
        vpi_control(vpiFinish, arg);
        /*
        if(vpi_control(vpiFinish, arg)==0) {
            throw SimulatorError(string(__func__) + (format(": fail to vpi_control. arg=%d") % arg).str());
            }*/
    }
}; // SimulatorImpl


// static
void Simulator::scanRegs( vector<Reg*>& regs, const VPIObject& vpiObj )
{
    vpiHandle iter = vpi_iterate(vpiReg, vpiObj.handle());
    vpiHandle ph;
    if(iter==NULL) 
        throw SimulatorError(string(__func__) + ": fail to scan vpiReg");
    while((ph = vpi_scan(iter)) != NULL) {
        regs.push_back( new Reg(ph) );
    }
}

// static
void Simulator::scanWires( vector<Wire*>& wires, const VPIObject& vpiObj )
{
    vpiHandle iter = vpi_iterate(vpiNet, vpiObj.handle());
    vpiHandle ph;
    if(iter==NULL) 
        throw SimulatorError(string(__func__) + ": fail to scan vpiWire");
    while((ph = vpi_scan(iter)) != NULL) {
        wires.push_back( new Wire(ph) );
    }
}

// static
/*
void Simulator::scanPorts( vector<Port*>& ports, const VPIObject& vpiObj )
{
    vpiHandle iter = vpi_iterate(vpiPort, vpiObj.handle());
    vpiHandle ph;
    if(iter==NULL) 
        throw SimulatorError(string(__func__) + ": fail to scan vpiPort");
    while((ph = vpi_scan(iter)) != NULL) {
        ports.push_back( Port::create(ph) );
    }
}
*/

// static
void Simulator::scanModules( vector<Module*>& mods, const VPIObject* obj ) 
{
    vpiHandle mod_iter = vpi_iterate(vpiModule, (obj==0) ? NULL : obj->handle());
        
    if (mod_iter==NULL)
        throw SimulatorError(string(__func__) + ": fail to scan vpiModule");

    while(1) {
        vpiHandle modh;
        if((modh = vpi_scan(mod_iter)) == NULL)
            break;
        mods.push_back( new Module(modh) );
    }
}

// static
static SimulatorImpl* simInstance = 0;

Simulator* Simulator::create()
{
    if( simInstance == 0 ) {
        simInstance = new SimulatorImpl();
    }
    return simInstance;
}


