#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <stdexcept>

using std::vector;
using std::string;
using std::stringstream;
using std::runtime_error;

#include "util.hpp"
#include "Simulator.hpp"
#include "objects.hpp"

string Reg::str() const {
    stringstream ss;
    ss << name() << "["
       << ":width=" << width()
       << "]";
    return ss.str();
}

/*
string Port::str() const {
    stringstream ss;
    ss << name() << "["
       << "dir=" << direction()
       << ":width=" << width()
       << ":@" << index()
       << "]";
    return ss.str();
}
*/

// ctor
Module::Module(vpiHandle h): VPIObject(h)
{
    /*
    try {
        Simulator::scanPorts(_ports, *this);
    } 
    catch(const SimulatorError& e) {
        // Do Nothing
        }*/
    try {
        Simulator::scanRegs(_regs, *this);
    }
    catch(const SimulatorError& e) {
        // Do Nothing
    }
    try {
        Simulator::scanWires(_wires, *this);
    }
    catch(const SimulatorError& e) {
        // Do Nothing
    }
    try {
        Simulator::scanModules(_modules, this);
    }
    catch(const SimulatorError& e) {
        // Do Nothing
    }
}

string Module::str() const {
    stringstream ss;
    ss << "(";
    /*
    for(vector<Port*>::const_iterator p = _ports.begin(); p!=_ports.end(); ++p) {
        ss << (*p)->str() << ", ";
        }*/
    for(vector<Reg*>::const_iterator r = _regs.begin(); r!=_regs.end(); ++r) {
        ss << (*r)->str() << ", ";
    }
    ss << ")";
    return ss.str();
}

/*
Port* Module::get_port(string name) const {
    vector<Port*>::const_iterator pp;
    if ((pp = find_if( _ports.begin(), _ports.end(), VPIObject::predNameOf(name) )) == _ports.end()) {
        throw runtime_error("get_port: not found: " + name);
    }
    return *pp;
}
*/

Reg* Module::get_reg(string name) const {
    vector<Reg*>::const_iterator pr;
    if ((pr = find_if( _regs.begin(), _regs.end(), VPIObject::predNameOf(name) )) == _regs.end()) {
        throw runtime_error("get_reg: not found: " + name);
    }
    return *pr;
}

Wire* Module::get_wire(std::string name) const {
    vector<Wire*>::const_iterator pw;
    if ((pw = find_if( _wires.begin(), _wires.end(), VPIObject::predNameOf(name) )) == _wires.end()) {
        throw runtime_error("get_wire: not found: " + name);
    }
    return *pw;
}

Module* Module::get_module(std::string name) const {
    vector<Module*>::const_iterator pm;
    if ((pm = find_if( _modules.begin(), _modules.end(), VPIObject::predNameOf(name) )) == _modules.end()) {
        throw runtime_error("get_module: not found: " + name);
    }
    return *pm;

}


struct vpi_object_event_descriptor
{
    s_cb_data cbdata;
    s_vpi_time time;
    s_vpi_value value;
    VPIObject* object;

    vpi_object_event_descriptor()
    {
        memset(&cbdata,0,sizeof(cbdata));
        memset(&time,0,sizeof(time));
        memset(&value,0,sizeof(value));
    }
};

// static
void VPIObject::valueChanged( s_cb_data* pcbdata )
{
    vpi_object_event_descriptor* desc = (vpi_object_event_descriptor*)pcbdata->user_data;
    VPIObject* object                 = desc->object;

    delete desc;

    assert(!object->_callbacks.empty());

    if( vpi_remove_cb(object->_cbhandle)==0 ) {
        s_vpi_error_info error;
        if(vpi_chk_error(&error)) {
            throw SimulatorError(string(__func__) + ": fail to vpi_remove_cb: " + error.message);
        }
    }

    callbacks_container temp;
    // move to temp
    while(!object->_callbacks.empty()) {
        SimulatorCallback* cb = object->_callbacks.front();
        object->_callbacks.pop_front();
        temp.push_back( cb );
    }
    object->_callbacks.clear();

    // hook callback handlers. by this, object->_callbacks may be re-push_back-ed.
    BOOST_FOREACH( SimulatorCallback* cb, temp ) {
        cb->called();
    }
}

// public
void VPIObject::setValueChangedCallback( SimulatorCallback* cb )
{
    if (_callbacks.empty()) {
        _callbacks.push_back( cb );

        vpi_object_event_descriptor* desc = new vpi_object_event_descriptor();
        desc->time             = { vpiSuppressTime };
        desc->value            = { vpiSuppressVal };
        desc->cbdata.reason    = cbValueChange;
        desc->cbdata.obj       = handle();
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

// public
/*
void VPIObject::unsetCallback( SimulatorCallback *cb )
{
    callbacks_container::iterator p = std::find( _callbacks.begin(), _callbacks.end(), cb );
    if (p!=_callbacks.end()) {
        _callbacks.erase( p );
        if (_callbacks.empty()) {
            if( vpi_remove_cb(_cbhandle)==0 ) {
                s_vpi_error_info error;
                if(vpi_chk_error(&error)) {
                    throw SimulatorError(string(__func__) + ": fail to vpi_remove_cb: " + error.message);
                }
            }
        }
    }
}
*/
