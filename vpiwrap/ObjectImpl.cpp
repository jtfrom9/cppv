#include <string>
#include <stdexcept>

#include "util.hpp"
#include "ObjectImpl.hpp"

#include "vpi_user.h"

namespace vpi {
namespace internal {

using std::string;
using std::runtime_error;

struct vpi_object_event_descriptor
{
    s_cb_data cbdata;
    s_vpi_time time;
    s_vpi_value value;
    ReadableSignalMixinBase* object;

    vpi_object_event_descriptor()
    {
        memset(&cbdata,0,sizeof(cbdata));
        memset(&time,0,sizeof(time));
        memset(&value,0,sizeof(value));
    }
};

// static
void ReadableSignalMixinBase::valueChanged( s_cb_data* pcbdata )
{
    vpi_object_event_descriptor* desc = (vpi_object_event_descriptor*)pcbdata->user_data;
    ReadableSignalMixinBase* object     = desc->object;

    delete desc;

    assert(!object->_callbacks.empty());

    if( vpi_remove_cb(object->_cbhandle)==0 ) {
        s_vpi_error_info error;
        if(vpi_chk_error(&error)) {
            throw SimulatorError(string(__func__) + ": fail to vpi_remove_cb: " + error.message);
        }
    }

    callback_container temp;
    // move to temp
    while(!object->_callbacks.empty()) {
        SimulatorCallback* cb = object->_callbacks.front();
        object->_callbacks.pop_front();
        temp.push_back( cb );
    }
    object->_callbacks.clear();

    // hook callback handlers. by this, object->_callbacks may be re-push_back-ed.
    LIST_FOREACH( SimulatorCallback*, cb, temp ) {
        (*cb).called();
    }
}

// public
template<typename T>
void ReadableSignalMixin<T>::setValueChangedCallback( SimulatorCallback* cb )
{
    if (_callbacks.empty()) {
        _callbacks.push_back( cb );

        vpi_object_event_descriptor* desc = new vpi_object_event_descriptor();
        desc->time             = { vpiSuppressTime };
        desc->value            = { vpiSuppressVal };
        desc->cbdata.reason    = cbValueChange;
        desc->cbdata.obj       = T::handle();
        desc->cbdata.cb_rtn    = (vpi_callback_handler_t*)valueChanged;
        desc->cbdata.user_data = (PLI_BYTE8*)desc;
        desc->cbdata.time      = &desc->time;
        desc->cbdata.value     = &desc->value;
        desc->object           = this;

        if((_cbhandle = vpi_register_cb( &desc->cbdata )) == NULL) {
            s_vpi_error_info error;
            if(vpi_chk_error(&error)) {
                throw SimulatorError(string(__func__) + ": fail to vpi_register_cb: " + error.message);
            }
        }
    } else {
        if (_find(_callbacks, cb)==0) {
            _callbacks.push_back( cb );
        }
    }
}


//static
void ModuleImpl::scanRegs( vector<RegImpl*>& regs, Object* const parent )
{
    vpiHandle iter = vpi_iterate(vpiReg, parent->handle());
    vpiHandle h;
    if(iter==NULL) 
        throw SimulatorError(string(__func__) + ": fail to scan vpiReg");
    while((h = vpi_scan(iter)) != NULL) {
        regs.push_back( new RegImpl(h,parent) );
    }
}

//static
void ModuleImpl::scanWires( vector<WireImpl*>& wires, Object* parent )
{
    vpiHandle iter = vpi_iterate(vpiNet, parent->handle());
    vpiHandle h;
    if(iter==NULL) 
        throw SimulatorError(string(__func__) + ": fail to scan vpiWire");
    while((h = vpi_scan(iter)) != NULL) {
        wires.push_back( new WireImpl(h,parent) );
    }
}

//static
void ModuleImpl::scanModules( vector<ModuleImpl*>& mods, Object* parent ) 
{
    vpiHandle mod_iter = vpi_iterate(vpiModule, (parent==0) ? NULL : parent->handle());
        
    if (mod_iter==NULL)
        throw SimulatorError(string(__func__) + ": fail to scan vpiModule");
    while(1) {
        vpiHandle h;
        if((h = vpi_scan(mod_iter)) == NULL)
            break;
        mods.push_back( new ModuleImpl(h,parent) );
    }
}

Reg* ModuleImpl::getReg_p( const string& name ) const {
    vector<RegImpl*>::const_iterator pr;
    if ((pr = find_if( _regs.begin(), _regs.end(), Object::predNameOf(name) )) == _regs.end()) {
        return 0;
    }
    return *pr;
}

Wire* ModuleImpl::getWire_p( const string& name ) const {
    vector<WireImpl*>::const_iterator pw;
    if ((pw = find_if( _wires.begin(), _wires.end(), Object::predNameOf(name) )) == _wires.end()) {
        return 0;
    }
    return *pw;
}

Module* ModuleImpl::getModule_p( const string& name ) const {
    vector<ModuleImpl*>::const_iterator pm;
    if ((pm = find_if( _modules.begin(), _modules.end(), Object::predNameOf(name) )) == _modules.end()) {
        return 0;
    }
    return *pm;
}


} //namespace internal
} //vpi

