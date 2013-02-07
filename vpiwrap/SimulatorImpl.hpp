#ifndef SIMULATORIMPL_HPP
#define SIMULATORIMPL_HPP

#include "objects.hpp"
#include "Simulator.hpp"

#include "vpi_user.h"

class SimulatorImpl: public Simulator
{
private:
    std::vector<Module::ptr> _modules;

public:
    // ctor
    SimulatorImpl();

    VPIObject& getObject(const char* path) const;

    int numOfModule() const { return _modules.size(); }
    
    Module& getModule( int index ) const;
    Module& getModule( const char* path ) const;

    static void scanRegs( std::vector<Reg::ptr>& regs, const VPIObject& vpiObj );
    static void scanPorts( std::vector<Port::ptr>& ports, const VPIObject& vpiObj );
    static void scanModules( std::vector<Module::ptr>& mods, const VPIObject* obj=0 );
};

#endif
