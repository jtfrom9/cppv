#include <string>
#include <vector>
#include <sstream>

using std::string;
using std::vector;
using std::stringstream;

using std::hex;

#include "boost/lexical_cast.hpp"

#include "Value.hpp"


// ScalarValue

ScalarValue::operator int() const
{
    switch( _bit ) {
    case '1': return 1;
    case '0': return 0;
    default:
        throw ValueError(string(__func__) + ": fail to convert " + _bit);
    }
}


// VectorValue

// private
const scalarval& VectorValue::get_scalar( int index ) const
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

string VectorValue::_dump() const
{
    stringstream ss;
    for(unsigned int i=0; i<_vecvals->size(); i++) {
        // ss << format("[%d] aval=%08h, bval=%08h") 
        //     % i % (*_vecvals)[i].aval % (*_vecvals)[i].bval << endl;
        ss << "[" << i << "] aval=" << hex << (*_vecvals)[i].aval << 
            " , bval=" << (*_vecvals)[i].bval << endl;
    }
    return ss.str().c_str();
}

// public
const scalarval& VectorValue::bit( unsigned int index ) const 
{
    if (index < 0 || index >= _width)
        throw std::invalid_argument((format("index=%d is larger than width=%d") % index % _width).str());
        
    int reverse_index = width() - index - 1;
        
    return get_scalar( reverse_index );
}

// public 
std::string VectorValue::str() const
{
    std::stringstream ss;
    for(unsigned int i=0; i<_width; i++) {
        ss << static_cast<char>(bit(i));
    }
    return ss.str();
}

// public
unsigned int VectorValue::conv() const 
{
    return (*_vecvals)[0].aval;
}

// public
int VectorValue::to_int() const
{
    if (!calculable()) 
        throw ValueError((format("can't convert to int, not calculable: %s") 
                          % str()).str());
    if (_width > (sizeof(unsigned int)*8))
        throw ValueError((format("can't convert to int, %d width: %s")
                          % _width % str()).str());
    return static_cast<int>(conv());
}

// public
unsigned int VectorValue::to_uint() const
{
    if (!calculable()) 
        throw ValueError((format("can't convert to int, not calculable: %s") 
                          % str()).str());
    if (_width > (sizeof(unsigned int)*8))
        throw ValueError((format("can't convert to int, %d width: %s")
                          % _width % str()).str());
    return conv();
}

//public
const string VectorValue::to_int_str() const
{
    if (calculable())
        return boost::lexical_cast<string>(to_int());
    else if (has_x())
        return _X.str();
    else
        return _Z.str();
}

static void init_vecval_bit(t_vpi_vecval& vpi_vecval)
{
    vpi_vecval.aval = 0;
    vpi_vecval.bval = 0;
}

static bool set_vecval_bit(t_vpi_vecval& vpi_vecval, int index, char v)
{
    assert(0 <= index && index < 32);

//      PLI_INT32 aval, bval; /* ab encoding: 00=0, 10=1, 11=X, 01=Z */
    switch( v ){
    case '0': 
        break;
    case '1': 
        vpi_vecval.bval |= (1 << index);
       break;
    case 'z': case 'Z':
        vpi_vecval.aval |= (1 << index);
        break;
    case 'x':  case 'X':
        vpi_vecval.aval |= (1 << index);
        vpi_vecval.bval |= (1 << index);
        break;
    default:
        return false;
    }
    return true;
}

vecval make_vecval2(const char* str)
{
    int width = strlen(str);

    assert(str!=NULL && width!=0);

    VectorValue::vecvals_container_p vecvals_p(new VectorValue::vecvals_container());
    vecvals_p->resize( (width -1) / 32 + 1 );

    for(int i=0; i<width; i++) {
        int div = i / 32;
        int mod = i % 32;
        if (mod==0) 
            init_vecval_bit( (*vecvals_p)[div] );
        if (!set_vecval_bit( (*vecvals_p)[div], mod, str[i] ))
            throw std::invalid_argument((format("invalid str=%c") % str).str());
    }
    return vecval( vecvals_p, width );
}

std::ostream& operator<<( std::ostream& os, const vecval& v )
{
    os << v.str();
    return os;
}
