#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include "util.hpp"
#include "ObjectImpl.hpp"
#include "Simulator.hpp"

namespace vpi {

using std::string;
using std::vector;
using std::stringstream;
using std::invalid_argument;
using std::runtime_error;

namespace internal {

class Request;

class SimulatorImpl: public Simulator
{
private:
    vector<ModuleImpl*> _modules;
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
        ModuleImpl::scanModules(_modules,0);
        count = 0;
    }

    Object& getObject(const char* path) const
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
        vector<ModuleImpl*>::const_iterator pm;
        if((pm = std::find_if(_modules.begin(), _modules.end(), Object::predNameOf(path))) == _modules.end()) {
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

} // namespace internal


// static
static internal::SimulatorImpl* simInstance = 0;

Simulator* Simulator::create()
{
    if( simInstance == 0 ) {
        simInstance = new internal::SimulatorImpl();
    }
    return simInstance;
}

} // namespace vpi
