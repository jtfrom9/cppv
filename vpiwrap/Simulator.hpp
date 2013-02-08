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
public:
    virtual void called() = 0;
};

struct vpi_descriptor
{
    s_cb_data cbdata;
    s_vpi_time time;
};

class Simulator: public boost::noncopyable
{
public:
    virtual VPIObject& getObject( const char* path ) const = 0;
    virtual int numOfModule() const                        = 0;
    virtual Module& getModule( int index ) const           = 0;
    virtual Module& getModule( const char* path ) const    = 0;

    virtual void registerCallback( const SimulatorCallback* cb, vpi_descriptor *desc ) const = 0;
};

void dumpTopology(std::ostream os, const Simulator& sim);

#endif
