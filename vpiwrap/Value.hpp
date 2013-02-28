#ifndef VALUE_HPP
#define VALUE_HPP

#include <cassert>
#include <stdexcept>
#include <iostream>
#include <sstream>

#include "util.hpp"
#include "vpi_user.h"

namespace vpi
{
class ValueBase
{
public:
    virtual ~ValueBase() {}
};

class ScalarValue: public ValueBase, public noncopyable
{
    char _bit;
public:
    ScalarValue(char v): _bit(v) {}

    bool operator==( const ScalarValue& rsh ) const
    {
        return _bit==rsh._bit;
    }

    operator char() const 
    { 
        return _bit; 
    }

    operator int() const;
    operator bool() const 
    {
        return static_cast<int>(*this);
    }

    std::string str() const { return std::string(&_bit); }
};

typedef ScalarValue scalarval;

const ScalarValue _1('1');
const ScalarValue _0('0');

#define b0 (_0)
#define b1 (_1)

const ScalarValue _x('x');
const ScalarValue _z('z');

#define _X (_x)
#define _Z (_z)
#define bX (_x)
#define bZ (_z)
#define bx (_x)
#define bz (_z)


class ValueError: public std::runtime_error
{
public:
    ValueError( const std::string& what_arg ): std::runtime_error( what_arg )
    {}
    ValueError( const char* what_arg ): std::runtime_error( what_arg )
    {}
};

class VectorValue: public ValueBase
{
public:
    typedef std::vector<t_vpi_vecval> vecvals_container;
    typedef boost::shared_ptr<vecvals_container> vecvals_container_p;

    static unsigned int num_of_vecvals( unsigned int width ) {
        return ((width - 1) / 32) + 1;
    }

private:
    boost::shared_ptr<vecvals_container> _vecvals;
    unsigned int _width;
    bool _has_x;
    bool _has_z;

    bool _width_check( unsigned int num_vecvals, unsigned int width )
    {
        return num_vecvals >= num_of_vecvals(width);
    }

    void _init_has_xz() {
        _has_x = false;
        _has_z = false;
        for(unsigned int i=0; i<_width; i++) {
            const ScalarValue& sv = bit(i);
            if(sv==_X) _has_x = true;
            if(sv==_Z) _has_z = true;
        }
    }

public:
    // ctor
    VectorValue( vecvals_container_p vecvals, unsigned int width ):
        _vecvals( vecvals ),
        _width( width )
    {
        if(!_width_check(vecvals->size(), width))
            throw std::invalid_argument((format("%s: vecvals entry = %d is not enough for width = %d")
                                         % __func__ % vecvals->size() % width).str());
        _init_has_xz();
    }

    // static  create from t_vpi_vecval array
    static VectorValue create( t_vpi_vecval* vpi_vecval_array, int width )
    {
        vecvals_container_p vecvals_p(new vecvals_container());
        for(unsigned int i=0; i< num_of_vecvals(width); i++) {
            vecvals_p->push_back( vpi_vecval_array[i] );
        }
        return VectorValue(vecvals_p, width);
    }

    // copy ctor
    // VectorValue( const VectorValue& rhs )
    // {
    //     _vecvals = rhs._vecvals;
    //     _width   = rhs._width;
    // }

    // assign copy
    // VectorValue& operator=( const VectorValue& rhs )
    // {
    //     _vecvals = rhs._vecvals;
    //     _width   = rhs._width;
    //     return *this;
    // }

private:
    const scalarval& get_scalar( int index ) const;
    unsigned int conv() const;
    const std::string _dump() const;
    const char* dump() const { return _dump().c_str(); }

public:
    // bit operations
    const scalarval& bit( unsigned int index ) const;
    const scalarval& operator[]( int index ) const { return bit( index ); }

    // properties
    int width() const { return _width; }
    bool has_x() const { return _has_x; }
    bool has_z() const { return _has_z; }
    bool calculable() const { return !has_x() && !has_z(); }

    const std::string bits_str() const;
    const std::string to_int_str() const;
    const std::string str() const { return to_int_str(); }

    // converter
    int to_int() const;
    unsigned int to_uint() const;

    operator int() const          { return to_int(); }
    operator unsigned int() const { return to_uint(); }
    operator bool() const         { return static_cast<bool>(to_int()); }

    unsigned int get_raw_vecval_size() const { return _vecvals->size(); }
    t_vpi_vecval get_raw_vecval(unsigned int index) const {
        return (*_vecvals)[ index ];
    }
};

typedef VectorValue vecval;

vecval make_vecval2(const char* str);

std::ostream& operator<<( std::ostream& os, const VectorValue& v );

} // namespace vpi

#endif
