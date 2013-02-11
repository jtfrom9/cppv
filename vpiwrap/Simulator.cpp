#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>

#include "util.hpp"
#include "objects.hpp"
#include "Simulator.hpp"

using std::string;
using std::vector;
using std::invalid_argument;
using std::runtime_error;

typedef PLI_INT32 handler_type( s_cb_data* );

static void __handler( s_cb_data* pcbdata )
{
    SimulatorCallback* cb = (SimulatorCallback*)pcbdata->user_data;
    cb->called();
}

class SimulatorImpl: public Simulator
{
private:
    vector<Module::ptr> _modules;

public:
    // ctor
    SimulatorImpl() 
    {
        Simulator::scanModules(_modules);
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
        vector<Module::ptr>::const_iterator pm;
        if((pm = std::find_if(_modules.begin(), _modules.end(), VPIObject::predNameOf(path))) == _modules.end()) {
            throw invalid_argument(string(__func__) + ": not found: " + path);
        }
        return **pm;
    }

    void registerCallback( const SimulatorCallback* cb, vpi_descriptor *desc ) const
    {
        desc->cbdata.cb_rtn    = (handler_type*)__handler;
        desc->cbdata.user_data = (PLI_BYTE8*)cb;
        desc->cbdata.time      = &desc->time;
        vpi_register_cb( &desc->cbdata );
    }
};


// static
void Simulator::scanRegs( vector<Reg::ptr>& regs, const VPIObject& vpiObj )
{
    vpiHandle iter = vpi_iterate(vpiReg, vpiObj.handle());
    vpiHandle ph;
    if(iter==NULL) 
        throw SimulatorError(string(__func__) + ": fail to scan vpiReg");
    while((ph = vpi_scan(iter)) != NULL) {
        regs.push_back( Reg::create(ph) );
    }
}

// static
void Simulator::scanWires( vector<Wire::ptr>& wires, const VPIObject& vpiObj )
{
    vpiHandle iter = vpi_iterate(vpiNet, vpiObj.handle());
    vpiHandle ph;
    if(iter==NULL) 
        throw SimulatorError(string(__func__) + ": fail to scan vpiWire");
    while((ph = vpi_scan(iter)) != NULL) {
        wires.push_back( Wire::create(ph) );
    }
}

// static
/*
void Simulator::scanPorts( vector<Port::ptr>& ports, const VPIObject& vpiObj )
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
void Simulator::scanModules( vector<Module::ptr>& mods, const VPIObject* obj ) 
{
    vpiHandle mod_iter = vpi_iterate(vpiModule, (obj==0) ? NULL : obj->handle());
        
    if (mod_iter==NULL)
        throw SimulatorError(string(__func__) + ": fail to scan vpiModule");

    while(1) {
        vpiHandle modh;
        if((modh = vpi_scan(mod_iter)) == NULL)
            break;
        mods.push_back(Module::create(modh));
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
