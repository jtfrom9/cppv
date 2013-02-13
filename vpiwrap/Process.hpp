#ifndef PROCESS_HPP
#define PROCESS_HPP

#include "boost/noncopyable.hpp"
#include "boost/function.hpp"

#include "objects.hpp"

class Context;
class Request;

class ISignal {};

class Process: public boost::noncopyable
{
private:
    const char* _name;
    Context* _context;

public:
    Process( const char* name );
    virtual ~Process();

    const char* name() const { return _name; }

protected:
    friend class Context;
    virtual void main() = 0;

    void delay( int cycle );
    void wait( Process* proc );
    void wait( VPIObject* obj );
    Process* create(Process* proc);

public:
    // for ProcessManager
    void next();
    bool end();
    Request* receive();

    // for global functions
    friend void delay( int cycle );
    friend void wait( Process* proc );
    friend void wait( VPIObject* obj );
    friend Process* create( Process* proc );
    friend Process* create( const char* name, boost::function<void()> func );
};


// global functions
void delay( int cycle );
void wait( Process* proc );
void wait( VPIObject* proc );
Process* create( Process* proc );
Process* create( const char* name, boost::function<void()> func );
long long sim_time();
Module& top();

#endif
