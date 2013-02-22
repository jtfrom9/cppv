#include <cstring>
#include <exception>
#include <stdexcept>
#include <string>

#include <ucontext.h>

#include "boost/format.hpp"

namespace vpi {
namespace internal {

using std::exception;
using std::string;
using std::runtime_error;
using std::strncpy;
using boost::format;

class stop_iteration: exception 
{
    string _from;
    bool   _terminated;

public:
    stop_iteration(string from, bool terminated):
        _from(from),
        _terminated(terminated)
    {}

    virtual ~stop_iteration() throw()
    {}

    const char* what() const throw() 
    {
        static char msg[512];
        strncpy(msg,
                (format("%s %s.") % _from % ((_terminated) ? "terminated" : "stop"))
                .str().c_str(),
                sizeof(msg));
        return msg;
    }
    
    bool terminated() const { return _terminated; }
    string from() const     { return _from; }
};


class caught_exception: exception
{
    string _from;
    string _what;
    
public:
    caught_exception( string from, string what ):
        _from(from),
        _what(what)
    {}

    virtual ~caught_exception() throw()
    {}

    const char* what() const throw()
    {
        static char msg[512];
        strncpy(msg,
                (format("caught_exception: %s from %s") % _what % _from).str().c_str(),
                sizeof(msg));
        return msg;
    }

    const string from() const        { return _from; }
    const string caught_what() const { return _what; }
};


class generator {
private:
    string _name;

    ucontext_t _context_child;
    ucontext_t _context_parent;
    char *_stack;
    int   _stack_size;
    
    bool  _end;
    bool  _terminated;
    bool  _abort;
    
    string _except_msg;

    static void callChild(generator* gen) {
        try {
            gen->body();
        }
        catch(const exception& e) {
            gen->_except_msg = e.what();
            gen->_abort = true;
        }
        gen->_end = true;
    }

    typedef void func_t();

protected:
    generator( string name, int stack_size=16*1024 ): 
        _name(name),
        _end(false),
        _terminated(false),
        _abort(false)
    {
        if(getcontext(&_context_child)==-1)
            throw runtime_error("generator: fail @ getcontext");
        _stack = (char*)malloc(stack_size);
        _stack_size = stack_size;
        _context_child.uc_stack.ss_sp   = _stack;
        _context_child.uc_stack.ss_size = _stack_size;
        _context_child.uc_link          = &_context_parent;
        makecontext(&_context_child, (func_t*)callChild, 1, this);
    }

    virtual ~generator() {
        free(_stack);
    }

    virtual void body() throw( exception ) = 0;

public:
    void next()
    {
        if(_end || _terminated) {
            _end = true;
            throw stop_iteration(_name, _terminated);
        }
        if(swapcontext(&_context_parent, &_context_child) == -1)
            throw runtime_error("next: fail @ swapcontext");
        if(_abort) {
            throw caught_exception(_name, _except_msg);
        }
    }
    bool end() {
        return _end;
    }
    void terminate() {
        _terminated = true;
    }
    
protected:
    void _yield() {
        if(swapcontext(&_context_child, &_context_parent) == -1)
            throw runtime_error("yield: fail @ swapcontext");
    }
    void yield() {
        _yield();
    }
};


template<typename T>
class generic_generator: public generator
{
private:
    bool  _has_data;
    T     _data;
    
public:
    generic_generator( string name, int stack_size=16*1024 ):
        generator( name, stack_size ),
        _has_data( false )
    {}

    bool receive(T* pdata) {
        if(end())
            return false;
        if(_has_data && pdata!= 0 )
            *pdata = _data;
        return _has_data;
    }

protected:
    void yield() {
        _has_data = false;
        _yield();
    }
    void yield_send(T data) {
        _has_data = true;
        _data     = data;
        _yield();
    }
};

} // namespace internal
} // namespace vpi
