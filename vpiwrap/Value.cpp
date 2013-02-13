#include <string>
#include <vector>

using std::string;
using std::vector;

#include "Value.hpp"

ScalarVal::operator int() const
{
    switch( _bit ) {
    case '1': return 1;
    case '0': return 0;
    default:
        throw ValueError(string(__func__) + ": fail to convert " + _bit);
    }
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
    for(int i=0; i<v.width(); i++) {
        os << static_cast<char>(v[i]);
    }
    return os;
}
