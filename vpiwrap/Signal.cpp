#include "Signal.hpp"

namespace vpi {

ostream& operator<<( ostream& os, const ISignal& sig ) {
    os << sig.readv();
    return os;
}

} // namespace vpi
