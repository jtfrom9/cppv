#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>

#include "SimulatorImpl.hpp"

using std::string;
using std::vector;

static SimulatorImpl* simImpl = 0;

Simulator& Simulator::getSimulator()
{
    if(simImpl==0) {
        simImpl = new SimulatorImpl();
    }
    return *simImpl;
}

// ctor
SimulatorImpl::SimulatorImpl()
{
    scanModules(_modules);
}

void SimulatorImpl::scanRegs( vector<Reg::ptr>& regs, const VPIObject& vpiObj )
{
    vpiHandle iter = vpi_iterate(vpiReg, vpiObj.handle());
    vpiHandle ph;
    if(iter==NULL) 
        throw SimulatorError(string(__func__) + ": fail to scan vpiReg");
    while((ph = vpi_scan(iter)) != NULL) {
        regs.push_back( Reg::create(ph) );
    }
}

void SimulatorImpl::scanPorts( vector<Port::ptr>& ports, const VPIObject& vpiObj )
{
    vpiHandle iter = vpi_iterate(vpiPort, vpiObj.handle());
    vpiHandle ph;
    if(iter==NULL) 
        throw SimulatorError(string(__func__) + ": fail to scan vpiPort");
    while((ph = vpi_scan(iter)) != NULL) {
        ports.push_back( Port::create(ph) );
    }
}

void SimulatorImpl::scanModules( vector<Module::ptr>& mods, const VPIObject* obj ) 
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

VPIObject& SimulatorImpl::getObject(const char* path) const
{
    return (*_modules[0]);
}

Module& SimulatorImpl::getModule( int index ) const
{
    return (*_modules[index]);
}

Module& SimulatorImpl::getModule( const char* path ) const
{
    vector<Module::ptr>::const_iterator pm;
    if((pm = std::find_if(_modules.begin(), _modules.end(), VPIObject::predNameOf(path))) == _modules.end()) {
        throw std::invalid_argument(string(__func__) + ": not found: " + path);
    }
    return **pm;
}
