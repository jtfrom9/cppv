#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <iostream>

#include <ucontext.h>

#include "boost/format.hpp"

struct stop_iteration: std::exception 
{
public:
    bool ended;
    bool terminated;

    stop_iteration(bool _ended, bool _terminated):
        ended(_ended),
        terminated(_terminated)
    {}

    const char* what() const throw() {
        static char msg[256];
        std::strncpy(msg, 
                     (boost::format("stop_iteration: ended=%d, terminated=%d") 
                      % ended % terminated).str().c_str(),
                     sizeof(msg));
        return msg;
    }
};

std::ostream& operator<<( std::ostream& os, const stop_iteration& e )
{
    os << e.what() << ": end=" << e.ended << ", terminated=" << e.terminated;
    return os;
}

class generator {
private:
    ucontext_t _context_child;
    ucontext_t _context_parent;
    char *_stack;
    int   _stack_size;
    
    bool  _end;
    bool  _terminate;

    static void callChild(generator* gen) {
        gen->body();
        gen->_end = true;
    }

    typedef void func_t();

protected:
    generator( int stack_size=16*1024 ): 
        _end(false),
        _terminate(false)
    {
        if(getcontext(&_context_child)==-1)
            throw std::runtime_error("generator: fail @ getcontext");
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

    virtual void body() = 0;

public:
    void next() {
        if(_end || _terminate)
            throw stop_iteration(_end,_terminate);
        if(swapcontext(&_context_parent, &_context_child) == -1)
            throw std::runtime_error("next: fail @ swapcontext");
    }
    bool end() {
        return _end;
    }
    void terminate() {
        _terminate = true;
    }
    
protected:
    void _yield() {
        if(swapcontext(&_context_child, &_context_parent) == -1)
            throw std::runtime_error("yield: fail @ swapcontext");
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
    generic_generator( int stack_size=16*1024 ):
        generator( stack_size ),
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

