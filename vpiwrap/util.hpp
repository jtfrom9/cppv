#ifndef UTIL_HPP
#define UTIL_HPP

#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <cassert>

using std::cout;
using std::endl;

#include "boost/format.hpp"
#include "boost/foreach.hpp"
using boost::format;

class not_implemented: public std::runtime_error
{
public:
    not_implemented( const std::string& what_arg ): std::runtime_error( what_arg )
    {}
    not_implemented( const char* what_arg ): std::runtime_error( what_arg )
    {}
};

template<typename Container>
typename Container::value_type _find( Container& c, 
                                      typename Container::value_type v ) 
{
    typename Container::iterator i = std::find(c.begin(), c.end(), v);
    return (i==c.end()) ? 0 : (*i);
}



#endif
