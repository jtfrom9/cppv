#ifndef SIGNAL_HPP
#define SIGNAL_HPP

#include <string>
#include <iostream>
#include <stdexcept>

#include "Object.hpp"
#include "Value.hpp"

namespace vpi {

using std::string;
using std::ostream;
using std::invalid_argument;


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

    operator vecval()       { return readv(); }
    operator int()          { return readi(); }
    operator unsigned int() { return readu(); }
    operator bool()         { return readb(); }

    virtual void setValueChangedCallback( SimulatorCallback* cb ) = 0;

    virtual ISignal& posedge() = 0;
};

ostream& operator<<( ostream& os, const ISignal& sig );

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
