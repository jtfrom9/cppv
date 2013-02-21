#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <string>
#include <stdexcept>

#include "boost/noncopyable.hpp"

#include "Simulator.hpp"
#include "Value.hpp"

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


class ISignal
{
public:
    // dtor
    virtual ~ISignal()
    {}

    virtual int width() const = 0;

    virtual vecval readv() const       = 0;
    virtual int readi() const          = 0;

    bool readb() const {
        return static_cast<bool>(readi());
    }

    unsigned int readu() const {
        return static_cast<unsigned int>(readi());
    }

    virtual void setValueChangedCallback( SimulatorCallback* cb ) = 0;

    virtual ISignal& posedge() = 0;
};


class IWritableSignal: public ISignal
{
public:
    // dtor
    virtual ~IWritableSignal()
    {}

    virtual void write_no_delay( bool b )   = 0;
    virtual void write_no_delay( int i )    = 0;
    virtual void write_no_delay( const vecval& v ) = 0;

    void write( bool b ) throw(std::exception) try {
        write_no_delay( b ); 
    } catch (const std::exception& e) {
        throw invalid_argument((format("write(bool): %s") % e.what()).str());
    }

    void write( int i ) throw(std::exception) try {
         write_no_delay( i ); 
    } catch (const std::exception& e) {
        throw invalid_argument((format("write(int): %s") % e.what()).str());
    }

    void write( const vecval& v ) throw(std::exception) try {
        write_no_delay( v ); 
    } catch (const std::exception& e) {
        throw invalid_argument((format("write(vecval): %s") % e.what()).str());
    } 
};


class Module: public Object
{
public:
    // dtor
    virtual ~Module() 
    {}

    const string type_str() const {
        return "Module";
    };
    
    virtual Reg* get_reg(std::string name) const       = 0;
    virtual Wire* get_wire(std::string name) const     = 0;
    virtual Module* get_module(std::string name) const = 0;
};


class Wire: public Object,
            public ISignal
{
public:
    // dtor
    virtual ~Wire()
    {}

    const string type_str() const {
        return "Wire";
    };
};


class Reg: public Object,
           public IWritableSignal
{
public:
    //dtor
    virtual ~Reg() 
    {}

    const string type_str() const {
        return "Reg";
    };
};

} // namespace vpi

#endif
