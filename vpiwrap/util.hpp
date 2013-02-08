#ifndef UTIL_HPP
#define UTIL_HPP

#include <stdexcept>
#include <iostream>

using std::cout;
using std::endl;

class not_implemented: public std::runtime_error
{
public:
    not_implemented( const std::string& what_arg ): std::runtime_error( what_arg )
    {}
    not_implemented( const char* what_arg ): std::runtime_error( what_arg )
    {}
};

#endif
