#ifndef OBJECTS_HPP
#define OBJECTS_HPP

#include <string>
#include <cstring>
#include <vector>
#include <list>
#include <iostream>
#include <sstream>

#include "boost/noncopyable.hpp"

#include "Simulator.hpp"
#include "vpi_user.h"

class VPIObject: private boost::noncopyable
{
private:
    vpiHandle _cbhandle;

    typedef std::list<SimulatorCallback*> callbacks_container;
    callbacks_container _callbacks;

private:
    s_vpi_value _val;
    s_vpi_time  _time;

protected:
    vpiHandle _handle;
    
    // ctor
    VPIObject(vpiHandle h): 
        _handle(h) 
    {}

    // dtor
    virtual ~VPIObject() 
    {
        //vpi_release_handle( _handle );
    }

    PLI_INT32 __vpi_get(PLI_INT32 prop, const char* fname) const
    {
        PLI_INT32 v = vpi_get(prop, _handle);
        if(v==vpiUndefined) {
            throw SimulatorError(std::string(fname) + ": error vpi_get @" + name());
        }
        return v;
    }
#define _vpi_get(prop) __vpi_get(prop,__func__)

public:
    const vpiHandle handle() const 
    { 
        return _handle; 
    }
    
    virtual const char* type_cstr() const = 0;

    const std::string vpi_type_str() const 
    {
        return std::string(vpi_get_str(vpiType, _handle));
    }

    const char* vpi_type_cstr() const
    {
        return vpi_type_str().c_str();
    }

    int vpi_type() const 
    {
        return (int)_vpi_get(vpiType);
    }

    std::string name() const 
    {
        return std::string(vpi_get_str(vpiName, _handle));
    }

    const char* namec() const
    {
        return name().c_str();
    }

    std::string fullname() const 
    {
        return std::string(vpi_get_str(vpiFullName, _handle));
    }

    const char* fullnamec() const
    {
        return fullname().c_str();
    }

    int width() const {
        return (int)_vpi_get(vpiSize);
    }

    void write_nodely( bool b ) {
        std::memset(&_val,0,sizeof(_val));
        _val.format    = vpiBinStrVal;
        _val.value.str = const_cast<char*>((b) ? "1" : "0");

        std::memset(&_time,0,sizeof(_time));
        _time.type = vpiSimTime;

        //vpi_put_value(_handle, &_val, &_time, vpiInertialDelay);
        //vpi_put_value(_handle, &_val, &_time, vpiTransportDelay);
        vpi_put_value(_handle, &_val, NULL, vpiNoDelay);
    }

    void write( bool b ) { write_nodely( b ); }

    bool readb() {
        std::memset(&_val,0,sizeof(_val));
        _val.format    = vpiBinStrVal;
        vpi_get_value(_handle, &_val);
        std::cout << "str=" << _val.value.str << std::endl;
        return (bool)_val.value.str;
    }

    virtual std::string to_str() const = 0;

private:
    static void valueChanged( s_cb_data* );

public:
    void setValueChangedCallback( SimulatorCallback* cb );
    void unsetCallback( SimulatorCallback *cb );

public:
    // functors
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
};


class Reg: public VPIObject
{
public:
    // ctor
    Reg(vpiHandle h): VPIObject(h)
    {}

    const char* type_cstr() const {
        return "Reg";
    }

    // overrides
    std::string to_str() const;
};


class Wire: public VPIObject
{
private:
    s_vpi_value _val;
    s_vpi_time _time;

public:
    // ctor
    Wire(vpiHandle h): VPIObject(h)
    {}

    const char* type_cstr() const {
        return "Wire";
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
        for(int i=0; i<width(); i++) {
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

public:
    // ctor
    Port(vpiHandle h): VPIObject(h)
    {}

    int index() const { 
        return vpi_get(vpiPortIndex, _handle); 
    }

    // const char* port_type_cstr() const {
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
private:
    std::vector<Module*> _modules;
    //std::vector<Port*> _ports;
    std::vector<Reg*> _regs;
    std::vector<Wire*> _wires;

public:
    //ctor
    // Module(vpiHandle h): VPIObject(h)
    // {}
    Module(vpiHandle h);

    const char* type_cstr() const 
    {
        return "Module";
    }

    // overrides
    virtual std::string to_str() const;

    //Port* get_port(std::string name) const;
    Reg* get_reg(std::string name) const;
    Wire* get_wire(std::string name) const;
    Module* get_module(std::string name) const;
};

#endif

