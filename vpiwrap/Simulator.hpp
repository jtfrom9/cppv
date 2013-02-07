#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <vector>
#include <ostream>
#include <stdexcept>

#include "boost/noncopyable.hpp"

#include "objects.hpp"

class SimulatorError: public std::runtime_error
{
public:
    SimulatorError( const std::string& what_arg ): std::runtime_error( what_arg )
    {}
    SimulatorError( const char* what_arg ): std::runtime_error( what_arg )
    {}
};

class Simulator: public boost::noncopyable
{
public:
    virtual VPIObject& getObject( const char* path ) const = 0;
    virtual int numOfModule() const                        = 0;
    virtual Module& getModule( int index ) const           = 0;
    virtual Module& getModule( const char* path ) const    = 0;

    static Simulator& getSimulator();
    //static Simulator getSimulatorProxy();
};

void dumpTopology(std::ostream os, const Simulator& sim);

#endif
