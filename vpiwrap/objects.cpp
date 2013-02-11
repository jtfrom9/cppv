#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <stdexcept>

#include "objects.hpp"
#include "Simulator.hpp"

using std::vector;
using std::string;
using std::stringstream;
using std::runtime_error;

string Reg::to_str() const {
    stringstream ss;
    ss << name() << "["
       << ":width=" << width()
       << "]";
    return ss.str();
}

/*
string Port::to_str() const {
    stringstream ss;
    ss << name() << "["
       << "dir=" << direction()
       << ":width=" << width()
       << ":@" << index()
       << "]";
    return ss.str();
}
*/

// ctor
Module::Module(vpiHandle h): VPIObject(h)
{
    /*
    try {
        Simulator::scanPorts(_ports, *this);
    } 
    catch(const SimulatorError& e) {
        // Do Nothing
        }*/
    try {
        Simulator::scanRegs(_regs, *this);
    }
    catch(const SimulatorError& e) {
        // Do Nothing
    }
    try {
        Simulator::scanModules(_modules, this);
    }
    catch(const SimulatorError& e) {
        // Do Nothing
    }
}

string Module::to_str() const {
    stringstream ss;
    ss << "(";
    /*
    for(vector<Port::ptr>::const_iterator p = _ports.begin(); p!=_ports.end(); ++p) {
        ss << (*p)->to_str() << ", ";
        }*/
    for(vector<Reg::ptr>::const_iterator r = _regs.begin(); r!=_regs.end(); ++r) {
        ss << (*r)->to_str() << ", ";
    }
    ss << ")";
    return ss.str();
}

/*
Port::ptr Module::get_port(string name) const {
    vector<Port::ptr>::const_iterator pp;
    if ((pp = find_if( _ports.begin(), _ports.end(), VPIObject::predNameOf(name) )) == _ports.end()) {
        throw runtime_error("get_port: not found: " + name);
    }
    return *pp;
}
*/

Reg::ptr Module::get_reg(string name) const {
    vector<Reg::ptr>::const_iterator pp;
    if ((pp = find_if( _regs.begin(), _regs.end(), VPIObject::predNameOf(name) )) == _regs.end()) {
        throw runtime_error("get_reg: not found: " + name);
    }
    return *pp;
}
