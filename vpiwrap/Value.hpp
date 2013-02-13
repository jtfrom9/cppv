#ifndef VALUE_HPP
#define VALUE_HPP

#include <cassert>
#include <stdexcept>
#include <iostream>

#include "boost/noncopyable.hpp"
#include "boost/shared_ptr.hpp"
#include "vpi_user.h"

#include "util.hpp"

class ValueBase
{
public:
    virtual ~ValueBase() {}
};

class ScalarVal: public ValueBase, public boost::noncopyable
{
    char _bit;
public:
    ScalarVal(char v): _bit(v) {}

    operator char() const 
    { 
        return _bit; 
    }

    operator int() const;
    operator bool() const 
    {
        return static_cast<int>(*this);
    }
};
typedef ScalarVal scalarval;

const ScalarVal _1('1');
const ScalarVal _0('0');

#define b0 (_0)
#define b1 (_1)

const ScalarVal _x('x');
const ScalarVal _z('z');

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

private:
    boost::shared_ptr<vecvals_container> _vecvals;
    int _width;

    
    bool width_check( int num_vecvals, int width )
    {
        return num_vecvals >= ((width - 1) / 32) + 1;
    }

public:
    // ctor
    VectorValue( vecvals_container_p vecvals, int width ):
        _vecvals( vecvals ),
        _width( width )
    {
        if(!width_check(vecvals->size(), width))
            throw std::invalid_argument((format("%s: vecvals entry = %d is not enough for width = %d")
                                         % __func__ % vecvals->size() % width).str());
    }

    static VectorValue create( t_vpi_vecval* vpi_vecval_array, int width )
    {
        vecvals_container_p vecvals_p(new vecvals_container());
        for(int i=0; i< ((width -1) / 32 + 1); i++) {
            vecvals_p->push_back( vpi_vecval_array[i] );
        }
        return VectorValue(vecvals_p, width);
    }

    // copy ctor
    VectorValue( const VectorValue& rhs )
    {
        _vecvals = rhs._vecvals;
        _width   = rhs._width;
    }

    // assign copy
    VectorValue& operator=( const VectorValue& rhs )
    {
        _vecvals = rhs._vecvals;
        _width   = rhs._width;
        return *this;
    }

private:
    const scalarval& get_scalar( int index ) const
    {
        int div = index / 32;
        int mod = index % 32;
        int32_t a = (*_vecvals)[div].aval & (1 << mod);
        int32_t b = (*_vecvals)[div].bval & (1 << mod);
        if(!b) {
            return (a) ? _1 : _0;
        } else {
            return (a) ? _X : _Z;
        }
    }

public:
    const scalarval& operator[]( int index ) const 
    {
        if (index < 0 || index >= _width)
            throw std::invalid_argument((format("index=%d is larger than width=%d") % index % _width).str());
        return get_scalar( index );
    }
    
    int width() const { return _width; }

    bool has_x() const {
        for(int i=0; i<_width; i++) {
            if((*this)[i]==_X)
                return true;
        }
        return false;
    }

    bool has_z() const {
        for(int i=0; i<_width; i++) {
            if((*this)[i]==_Z)
                return true;
        }
        return false;
    }

    bool calculable() const { return !has_x() && !has_z(); }
};

typedef VectorValue vecval;

vecval make_vecval2(const char* str);

std::ostream& operator<<( std::ostream& os, const VectorValue& v );

#endif
