#ifndef OBJECTS_HPP
#define OBJECTS_HPP

#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <tr1/memory>

#include "vpi_user.h"

class NonCopyable
{
  protected:
    NonCopyable () {}
    ~NonCopyable () {} 

  private: 
    NonCopyable(const NonCopyable &);
    NonCopyable& operator=(const NonCopyable &);
};

class VPIObject: private NonCopyable
{
protected:
    vpiHandle _handle;
    VPIObject(vpiHandle h): _handle(h) 
    {}

public:
    virtual ~VPIObject() {
#if 0        
        vpi_release_handle( _handle );
#endif
    }

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
    
    virtual std::string to_str() = 0;

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
private:
    s_vpi_value _val;
    s_vpi_time _time;

    Reg(vpiHandle h): VPIObject(h)
    {}

public:
    typedef std::shared_ptr<Reg> ptr;

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
    virtual std::string to_str() {
        std::stringstream ss;
        ss << name() << "["
           << ":width=" << width()
           << "]";
        return ss.str();
    }
};

void scanRegs( std::vector<Reg::ptr>& regs, const VPIObject& vpiObj )
{
    vpiHandle iter = vpi_iterate(vpiReg, vpiObj.handle());
    vpiHandle ph;
    if(iter!=NULL) {
        while((ph = vpi_scan(iter)) != NULL) {
            regs.push_back( Reg::create(ph) );
        }
    }
}

class Port: public VPIObject
{
private:
    s_vpi_value _val;
    s_vpi_time _time;

    Port(vpiHandle h): VPIObject(h)
    {}

public:
    typedef std::shared_ptr<Port> ptr;

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
    virtual std::string to_str() {
        std::stringstream ss;
        ss << name() << "["
           << "dir=" << direction()
           << ":width=" << width()
           << ":@" << index()
           << "]";
        return ss.str();
    }
};

void scanPorts( std::vector<Port::ptr>& ports, const VPIObject& vpiObj )
{
    vpiHandle iter = vpi_iterate(vpiPort, vpiObj.handle());
    vpiHandle ph;
    if(iter!=NULL) {
        while((ph = vpi_scan(iter)) != NULL) {
            ports.push_back( Port::create(ph) );
        }
    }
}


class Module: public VPIObject
{
public:
    typedef std::shared_ptr<Module> ptr;

    static ptr create(vpiHandle h) {
        return ptr(new Module(h));
    }

    std::vector<Module::ptr> _modules;

private:
    std::vector<Port::ptr> _ports;
    std::vector<Reg::ptr> _regs;

    Module(vpiHandle h): VPIObject(h)
    {
        scanPorts(_ports, *this);
        scanRegs(_regs, *this);
        scanModules(_modules, this);
    }


public:
    // overrides
    virtual std::string to_str() {
        std::stringstream ss;
        ss << "(";
        for(std::vector<Port::ptr>::iterator p = _ports.begin(); p!=_ports.end(); ++p) {
            ss << (*p)->to_str() << ", ";
        }
        for(std::vector<Reg::ptr>::iterator r = _regs.begin(); r!=_regs.end(); ++r) {
            ss << (*r)->to_str() << ", ";
        }
        ss << ")";
        return ss.str();
    }

    Port::ptr get_port(std::string name) {
        std::vector<Port::ptr>::iterator pp;
        if ((pp = find_if( _ports.begin(), _ports.end(), VPIObject::predNameOf(name) )) == _ports.end()) {
            throw std::runtime_error("get_port: not found: " + name);
        }
        return *pp;
    }

    Reg::ptr get_reg(std::string name) {
        std::vector<Reg::ptr>::iterator pp;
        if ((pp = find_if( _regs.begin(), _regs.end(), VPIObject::predNameOf(name) )) == _regs.end()) {
            throw std::runtime_error("get_reg: not found: " + name);
        }
        return *pp;
    }

    static void scanModules( std::vector<Module::ptr>& mods, const VPIObject* obj=0 ) 
    {
        vpiHandle mod_iter = vpi_iterate(vpiModule, (obj==0) ? NULL : obj->handle());
        
        if (mod_iter==NULL)
            return;

        while(1) {
            vpiHandle modh;
            if((modh = vpi_scan(mod_iter)) == NULL)
                break;
            mods.push_back(Module::create(modh));
        }
    }

};
/*
class Global {

private:

public:
    static Global *instance;
    std::vector<Module::ptr> modules;

    static Global& get() { 
        return *instance;
    }

    static void initialize()
    {
        Global::instance = new Global();

        Module::scanModules(instance->modules);

        for(unsigned int i=0; i<instance->modules.size(); i++) {
            std::cout << "[" << i << "] " 
                      << instance->modules[i]->type_str() 
                      << ": " << instance->modules[i]->name()
                      << ": " << instance->modules[i]->fullname()
                      << ": " << instance->modules[i]->to_str()
                      << std::endl;
        }

        for(unsigned int i=0; i<instance->modules[0]->_modules.size(); i++) {
            std::cout << "[" << i << "] " 
                      << instance->modules[0]->_modules[i]->type_str() 
                      << ": " << instance->modules[0]->_modules[i]->name()
                      << ": " << instance->modules[0]->_modules[i]->fullname()
                      << ": " << instance->modules[0]->_modules[i]->to_str()
                      << std::endl;
        }
    }
};
*/
std::vector<Module::ptr> modules;
void initialize()
{
    Module::scanModules(modules);

    for(unsigned int i=0; i<modules.size(); i++) {
        std::cout << "[" << i << "] " 
                  << modules[i]->type_str() 
                  << ": " << modules[i]->name()
                  << ": " << modules[i]->fullname()
                  << ": " << modules[i]->to_str()
                  << std::endl;
    }

    for(unsigned int i=0; i<modules[0]->_modules.size(); i++) {
        std::cout << "[" << i << "] " 
                  << modules[0]->_modules[i]->type_str() 
                  << ": " << modules[0]->_modules[i]->name()
                  << ": " << modules[0]->_modules[i]->fullname()
                  << ": " << modules[0]->_modules[i]->to_str()
                  << std::endl;
    }
}

#endif

