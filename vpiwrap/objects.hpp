#ifndef OBJECTS_HPP
#define OBJECTS_HPP

#include <string>
#include <cstring>
#include <vector>
#include <list>
#include <iostream>
#include <sstream>

#include "boost/noncopyable.hpp"
#include "boost/shared_ptr.hpp"

#include "vpi_user.h"


class SimulatorCallback;

class VPIObject: private boost::noncopyable
{
private:
    vpiHandle _cbhandle;
    typedef std::list<SimulatorCallback*> callbacks_container;
    callbacks_container _callbacks;
    
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

    static void valueChanged( s_cb_data* );
    void setValueChangedCallback( SimulatorCallback* cb );
    void unsetCallback( SimulatorCallback *cb );

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
        _time.low  = 0;
        _time.real = 0.0;
        //vpi_put_value(_handle, &_val, &_time, vpiInertialDelay);
        //vpi_put_value(_handle, &_val, &_time, vpiTransportDelay);
        vpi_put_value(_handle, &_val, NULL, vpiNoDelay);
    }

    // overrides
    std::string to_str() const;
};


class Wire: public VPIObject
{
private:
    s_vpi_value _val;
    s_vpi_time _time;

    // ctor
    Wire(vpiHandle h): VPIObject(h)
    {}

public:
    typedef boost::shared_ptr<Wire> ptr;
    
    static ptr create(vpiHandle h) {
        return ptr(new Wire(h));
    }

    unsigned int width() const {
        return (unsigned int)vpi_get(vpiSize, _handle);
    }

    std::string binstr()
    {
        std::memset(&_val,0,sizeof(_val));
        _val.format    = vpiBinStrVal;
        vpi_get_value(_handle, &_val);
        return _val.value.str;
    }

    std::string hexstr()
    {
        std::memset(&_val,0,sizeof(_val));
        _val.format    = vpiHexStrVal;
        vpi_get_value(_handle, &_val);
        return _val.value.str;
    }

    std::string scalar()
    {
        std::memset(&_val,0,sizeof(_val));
        _val.format    = vpiScalarVal;
        vpi_get_value(_handle, &_val);
        switch( _val.value.scalar ) {
        case vpi1: return "vpi1";
        case vpi0: return "vpi0";
        case vpiX: return "vpiX";
        case vpiZ: return "vpiZ";
        case vpiH: return "vpiH";
        case vpiL: return "vpiL";
        }
        return "??";
    }

    std::string integer()
    {
        std::memset(&_val,0,sizeof(_val));
        _val.format    = vpiIntVal;
        vpi_get_value(_handle, &_val);
        std::stringstream ss;
        ss << _val.value.integer;
        return ss.str();
    }

    std::string vector()
    {
        std::memset(&_val,0,sizeof(_val));
        _val.format    = vpiVectorVal;
        vpi_get_value(_handle, &_val);
        std::stringstream ss;
        int a = _val.value.vector[0].aval;
        int b = _val.value.vector[0].bval;
        for(unsigned int i=0; i<width(); i++) {
            if((b & 1)==0) {
                ss << ((a & 1) ? "1" : "0");
            } else {
                ss << ((a & 1) ? "x" : "z");
            }
            a >>= 1;
            b >>= 1;
        }
        return ss.str();
    }
    
    bool readb() {
        std::memset(&_val,0,sizeof(_val));
        std::memset(&_time,0,sizeof(_time));
        _val.format    = vpiBinStrVal;

        //_val.format    = vpiObjTypeVal;

        //_val.value.str = (char*)malloc(1024);
        //_val.value.str = const_cast<char*>((b) ? "1" : "0");
        // _val.format = vpiIntVal;
        // _val.value.integer = b;
        vpi_get_value(_handle, &_val);
        std::cout << "str=" << _val.value.str << std::endl;
        return (bool)_val.value.str;
    }

    std::string read() {
        return "bin: " + binstr() + 
            ", hex: " + hexstr() +
            ", scalar: " + scalar() +
            ", int: " + integer()+
            ", vec: " + vector();
    }

    std::string to_str() const
    {
        return "";
    }

};


/*
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
*/

class Module: public VPIObject
{
public:
    typedef boost::shared_ptr<Module> ptr;

    static ptr create(vpiHandle h) {
        return ptr(new Module(h));
    }

private:
    std::vector<Module::ptr> _modules;
    //std::vector<Port::ptr> _ports;
    std::vector<Reg::ptr> _regs;
    std::vector<Wire::ptr> _wires;

    //ctor
    // Module(vpiHandle h): VPIObject(h)
    // {}
    Module(vpiHandle h);

public:
    // overrides
    virtual std::string to_str() const;

    //Port::ptr get_port(std::string name) const;
    Reg::ptr get_reg(std::string name) const;
    Wire::ptr get_wire(std::string name) const;
    Module::ptr get_module(std::string name) const;
};

#endif

