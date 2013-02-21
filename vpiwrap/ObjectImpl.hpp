#ifndef OBJECTIMPL_HPP
#define OBJECTIMPL_HPP

#include <string>
#include <cstring>
#include <vector>
#include <list>
#include <iostream>
#include <sstream>

#include "boost/noncopyable.hpp"

#include "util.hpp"
#include "Object.hpp"
#include "Simulator.hpp"
#include "Value.hpp"

namespace vpi {

using std::string;
using std::vector;
using std::list;
using std::invalid_argument;

namespace internal {

template<typename T>
class ObjectMixin: virtual public T
{
public:
    const string vpi_type_str() const
    {
        return string(vpi_get_str(vpiType, T::handle()));
    }

    int vpi_type() const
    {
        return (int)T::_vpi_get(vpiType);
    }

    const string vpi_name() const
    {
        return string(vpi_get_str(vpiName, T::handle()));
    }

    const string vpi_fullname() const
    {
        return string(vpi_get_str(vpiFullName, T::handle()));
    }
};


class ReadableSignalMixinBase
{
protected:
    typedef list<SimulatorCallback*> callback_container;
    callback_container _callbacks;
    vpiHandle _cbhandle;

    static void valueChanged( s_cb_data* pcbdata );

public:
    // dtor
    virtual ~ReadableSignalMixinBase()
    {}
};


template<typename T>
class ReadableSignalMixin: public ReadableSignalMixinBase,
                           virtual public T
{
public:
    int width() const {
        return (int)T::_vpi_get(vpiSize);
    }

    void setValueChangedCallback( SimulatorCallback* cb );

    vecval readv() const {
        s_vpi_value val;
        val.format  = vpiVectorVal;
        vpi_get_value( T::handle(), &val );
        return VectorValue::create(val.value.vector, width());
    }
    
    int readi() const {
        s_vpi_value val;
        val.format  = vpiIntVal;
        vpi_get_value( T::handle(), &val );
        return val.value.integer;
    }
};


template<typename T>
class WritableSignalMixin: virtual public T
{
private:
    t_vpi_vecval* __write_buf;

public:
    // ctor
    WritableSignalMixin():
        __write_buf(0)
    {}

    // dtor
    virtual ~WritableSignalMixin()
    {
        delete[] __write_buf;
    }

private:
    t_vpi_vecval* _write_buf() {
        if(__write_buf==0) {
            __write_buf = new t_vpi_vecval[ VectorValue::num_of_vecvals( T::width() ) ];
        }
        return __write_buf;
    }

    void _put_no_delay( s_vpi_value& val ) {
        vpi_put_value(T::handle(), &val, NULL, vpiNoDelay);
    }
    void _put_inertial_delay( s_vpi_value& val, s_vpi_time& _time ) {
        vpi_put_value(T::handle(), &val, &_time, vpiInertialDelay);
    }
    void _put_transport_delay( s_vpi_value& val, s_vpi_time& _time ) {
        vpi_put_value(T::handle(), &val, &_time, vpiTransportDelay);
    }

public:
    void write_no_delay( bool b ) {
        s_vpi_value val;
        val.format    = vpiBinStrVal;
        val.value.str = const_cast<char*>((b) ? "1" : "0");
        _put_no_delay( val );
    }

    void write_no_delay( int i ) {
        s_vpi_value val;
        val.format        = vpiIntVal;
        val.value.integer = i;
        _put_no_delay( val );
    }

    void write_no_delay( const vecval& v ) {
        s_vpi_value val;
        
        if( T::width()!=v.width() )
            throw invalid_argument((format("diffrent vecval width. %s is %d bits") 
                                    % T::fullname() % T::width()).str());
        val.format = vpiVectorVal;
        val.value.vector = _write_buf();;
        for(unsigned int i=0; i<VectorValue::num_of_vecvals(T::width()); i++) {
            val.value.vector[i] = v.get_raw_vecval(i);
        }
        _put_no_delay( val );
    }
};


class ModuleImpl;
class RegImpl;
class WireImpl;

class ModuleImpl: public ObjectMixin<Module>
{
private:
    vector<ModuleImpl*> _modules;
    vector<RegImpl*>    _regs;
    vector<WireImpl*>   _wires;

public:
    // ctor
    ModuleImpl(vpiHandle h, Object* parent)
    {
        _handle = h;
        _parent = parent;

        try {
            scanRegs(_regs, this);
        }
        catch(const SimulatorError& e) {
            // Do Nothing
        }
        try {
            scanWires(_wires, this);
        }
        catch(const SimulatorError& e) {
            // Do Nothing
        }
        try {
            scanModules(_modules, this);
        }
        catch(const SimulatorError& e) {
            // Do Nothing
        }
    }

    //
    // object getter
    //
    virtual Reg* get_reg(string name) const;
    virtual Wire* get_wire(string name) const;
    virtual Module* get_module(string name) const;

    //
    // static
    //
    static void scanRegs( vector<RegImpl*>& regs, Object* const parent );
    static void scanWires( vector<WireImpl*>& wires, Object* parent );
    static void scanModules( vector<ModuleImpl*>& mods, Object* parent );
};


class WireImpl: public ObjectMixin<Wire>,
                public ReadableSignalMixin<Wire>
{
public:
    // ctor
    WireImpl(vpiHandle h, Object* parent)
    {
        _handle = h;
        _parent = parent;
    }
    
    const vpiHandle handle() const { 
        return _handle;
    }
};


class RegImpl: public ObjectMixin<Reg>,
               public WritableSignalMixin< ReadableSignalMixin<Reg> >
{
public:
    // ctor
    RegImpl(vpiHandle h, Object* parent)
    {
        _handle = h;
        _parent = parent;
    }

    const vpiHandle handle() const { 
        return _handle;
    }
};


} //namespace internal
} //namespace vpi

#endif
