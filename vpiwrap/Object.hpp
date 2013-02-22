#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <string>
#include <stdexcept>

#include "boost/noncopyable.hpp"

#include "util.hpp"
#include "Simulator.hpp"

#include "vpi_user.h"

namespace vpi {

using std::string;
using std::invalid_argument;

class Object: public boost::noncopyable
{
protected:
    vpiHandle _handle;
    Object*   _parent;

public:
    Object():
        _handle(0),
        _parent(0)
    {}

    // dtor
    virtual ~Object() 
    {
        //vpi_release_handle( _handle );
    }

    const vpiHandle handle() const 
    { 
        return _handle; 
    }

    const Object* parent() const
    {
        return _parent;
    }


    // vpi property getter
    virtual const string vpi_type_str() const = 0;
    virtual int vpi_type() const              = 0;
    virtual const string vpi_name() const     = 0;
    virtual const string vpi_fullname() const = 0;

    
    const string name() const
    {
        return vpi_name();
    }

    const string fullname() const
    {
        // consist of parent fullname() recursively 
        throw not_implemented("ObjectImpl::fullname()");
    }

    virtual const string type_str() const = 0;
    virtual const string str() const
    {
        return fullname() + "@" + type_str() + "[" + vpi_type_str() + "]";
    }

protected:
    PLI_INT32 __vpi_get(PLI_INT32 prop, const char* fname) const
    {
        PLI_INT32 v = vpi_get(prop, handle());
        if(v==vpiUndefined) {
            throw SimulatorError(string(fname) + ": error vpi_get @" + fullname());
        }
        return v;
    }
#define _vpi_get(prop) __vpi_get(prop,__func__)


public:
    // functors
    class predNameOf {
    private:
        string _name;
    public:
        predNameOf(string name): _name(name) {}

        template<typename T>
        bool operator()( T* obj ) const {
            return obj->name() == _name;
        }
    };
};


class Reg;
class Wire;

class Module: public Object
{
public:
    // dtor
    virtual ~Module() 
    {}

    const string type_str() const {
        return "Module";
    };
    
    virtual Reg* getReg_p( const string& name ) const       = 0;
    virtual Wire* getWire_p( const string& name ) const     = 0;
    virtual Module* getModule_p( const string& name ) const = 0;

    Reg& getReg( const string& _name ) const {
        Reg* r = getReg_p(_name);
        if(r==0)
            throw invalid_argument((format("getReg: '%s' not found in '%s'") % _name % name()).str());
        return *r;
    }
    
    Wire& getWire( const string& _name ) const {
        Wire* w = getWire_p(_name);
        if(w==0)
            throw invalid_argument((format("getWire: wire '%s' not found in '%s'") % _name % name()).str());
        return *w;
    }

    Module& getModule( const string& _name ) const {
        Module* m = getModule_p(_name);
        if(m==0)
            throw invalid_argument((format("getModule: module '%s' not found in '%s'") % _name % name()).str());
        return *m;
    }
};


} // namespace vpi

#endif
