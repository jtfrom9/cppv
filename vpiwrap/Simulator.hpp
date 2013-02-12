#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <vector>
#include <ostream>
#include <stdexcept>

#include "boost/noncopyable.hpp"

#include "objects.hpp"

#include "vpi_user.h"

class SimulatorError: public std::runtime_error
{
public:
    SimulatorError( const std::string& what_arg ): std::runtime_error( what_arg )
    {}
    SimulatorError( const char* what_arg ): std::runtime_error( what_arg )
    {}
};

class SimulatorCallback 
{
    vpiHandle _cbh;

public:
    virtual void called()      = 0;
    virtual const char* dump() = 0;
    int count;

    void setCbHandle( vpiHandle h ) {
        _cbh = h;
    }
    vpiHandle cbHandle() const {
        return _cbh;
    }
};

struct vpi_descriptor
{
    s_cb_data cbdata;
    s_vpi_time time;
    s_vpi_value value;
};

class Simulator: public boost::noncopyable
{
public:
    virtual VPIObject& getObject( const char* path ) const = 0;
    virtual int numOfModule() const                        = 0;
    virtual Module& getModule( int index ) const           = 0;
    virtual Module& getModule( const char* path ) const    = 0;

    virtual void registerCallback( SimulatorCallback* cb, vpi_descriptor *desc ) const = 0;
    virtual void unregisterCallback( SimulatorCallback* cb ) const = 0;

    static void scanRegs( std::vector<Reg::ptr>& regs, const VPIObject& vpiObj );
    //static void scanPorts( std::vector<Port::ptr>& ports, const VPIObject& vpiObj );
    static void scanWires( std::vector<Wire::ptr>& wires, const VPIObject& vpiObj );
    static void scanModules( std::vector<Module::ptr>& mods, const VPIObject* obj=0 );
    
    static Simulator* create();
};

void dumpTopology(std::ostream os, const Simulator& sim);

#endif
