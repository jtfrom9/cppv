#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include <vector>
#include <ostream>
#include <stdexcept>

#include "util.hpp"

#include "vpi_user.h"

namespace vpi {

class SimulatorError: public std::runtime_error
{
public:
    SimulatorError( const std::string& what_arg ): std::runtime_error( what_arg )
    {}
    SimulatorError( const char* what_arg ): std::runtime_error( what_arg )
    {}
};

class FinishSimulation: public std::exception
{};

class SimulatorCallback 
{
public:
    virtual ~SimulatorCallback() {}

    virtual void called() = 0;
};

typedef PLI_INT32 vpi_callback_handler_t( s_cb_data* );

class Object;
class Module;
class Reg;
class Wire;

class Simulator: public noncopyable
{
public:
    virtual Object& getObject( const char* path ) const = 0;
    virtual int numOfModule() const                        = 0;
    virtual Module& getModule( int index ) const           = 0;
    virtual Module& getModule( const char* path ) const    = 0;

    virtual void setAfterDelayCallback( SimulatorCallback* cb, int delay ) const = 0;

    virtual long long sim_time() const = 0;
    virtual void finish( int arg = 0 ) const = 0;

    static Simulator* create();
};

void dumpTopology(std::ostream os, const Simulator& sim);

} // namespace vpi
#endif
