#ifndef OBJECTS_HPP
#define OBJECTS_HPP

#include <string>
#include <cstring>
#include <vector>
#include <iostream>

#include "boost/noncopyable.hpp"
#include "boost/shared_ptr.hpp"

#include "vpi_user.h"

class VPIObject: private boost::noncopyable
{
protected:
    vpiHandle _handle;
    
    // ctor
    VPIObject(vpiHandle h): _handle(h) 
    {}

    // dtor
    virtual ~VPIObject() {
        //vpi_release_handle( _handle );
    }

public:

    // VPIType type() {
    //     vpi_get(vpiType,_handle);
    // }
    const vpiHandle handle() const { return _handle; }

    const char* type_str() const {
        return vpi_get_str(vpiType, _handle);
    }
    int type() const {
        return vpi_get(vpiType, _handle);
    }

    const char* name() const {
        return vpi_get_str(vpiName, _handle);
    }

    const char* fullname() const {
        return vpi_get_str(vpiFullName, _handle);
    }
    
    virtual std::string to_str() const = 0;

    class predNameOf {
    private:
        std::string _name;
    public:
        predNameOf(std::string name): _name(name) {}

        template<typename T>
        bool operator()( T obj ) const {
            return obj->name() == _name;
        }
    };

    typedef boost::shared_ptr<VPIObject> ptr;
};


class Reg: public VPIObject
{
private:
    s_vpi_value _val;
    s_vpi_time _time;

    // ctor
    Reg(vpiHandle h): VPIObject(h)
    {}

public:
    typedef boost::shared_ptr<Reg> ptr;

    static ptr create(vpiHandle h) {
        return ptr(new Reg(h));
    }

    unsigned int width() const {
        return (unsigned int)vpi_get(vpiSize, _handle);
    }

    void write(bool b) {
        std::memset(&_val,0,sizeof(_val));
        std::memset(&_time,0,sizeof(_time));
        _val.format    = vpiBinStrVal;
        _val.value.str = const_cast<char*>((b) ? "1" : "0");
        // _val.format = vpiIntVal;
        // _val.value.integer = b;
        _time.type = vpiSimTime;
        _time.high = 0;
        _time.low  = 1;
        _time.real = 0.0;
        //vpi_put_value(_handle, &_val, &_time, vpiInertialDelay);
        //vpi_put_value(_handle, &_val, &_time, vpiTransportDelay);
        vpi_put_value(_handle, &_val, NULL, vpiNoDelay);
    }

    // overrides
    std::string to_str() const;
};


class Port: public VPIObject
{
private:
    s_vpi_value _val;
    s_vpi_time _time;

    // ctor
    Port(vpiHandle h): VPIObject(h)
    {}

public:
    typedef boost::shared_ptr<Port> ptr;

    static ptr create(vpiHandle h) {
        return ptr(new Port(h));
    }

    int index() const { 
        return vpi_get(vpiPortIndex, _handle); 
    }

    // const char* port_type_str() const {
    //     return vpi_get_str(vpiPortType, _handle);
    // }

    int direction() const {
        return vpi_get(vpiDirection, _handle);
    }

    // bool is_scalar() const {
    //     return (bool)vpi_get(vpiScalar, _handle);
    // }

    // bool is_vector() const {
    //     return (bool)vpi_get(vpiVector, _handle);
    // }

    unsigned int width() const {
        return (unsigned int)vpi_get(vpiSize, _handle);
    }

    void write(bool b) {
        std::memset(&_val,0,sizeof(_val));
        std::memset(&_time,0,sizeof(_time));
        // _val.format    = vpiBinStrVal;
        // _val.value.str = const_cast<char*>((b) ? "1" : "0");
        _val.format = vpiIntVal;
        _val.value.integer = b;
        _time.type = vpiSimTime;
        _time.high = 0;
        _time.low  = 1;
        _time.real = 0.0;
        std::cout << "before" << std::endl;
        //vpi_put_value(_handle, &_val, &_time, vpiInertialDelay);
        vpi_put_value(_handle, &_val, &_time, vpiTransportDelay);
        //vpi_put_value(_handle, &_val, NULL, vpiNoDelay);
        std::cout << "after" << std::endl;
    }

    // overrides
    std::string to_str() const;
};

class Module: public VPIObject
{
public:
    typedef boost::shared_ptr<Module> ptr;

    static ptr create(vpiHandle h) {
        return ptr(new Module(h));
    }

private:
    std::vector<Module::ptr> _modules;
    std::vector<Port::ptr> _ports;
    std::vector<Reg::ptr> _regs;

    //ctor
    // Module(vpiHandle h): VPIObject(h)
    // {}
    Module(vpiHandle h);

public:
    // overrides
    virtual std::string to_str() const;

    Port::ptr get_port(std::string name) const;
    Reg::ptr get_reg(std::string name) const;
};


#endif

