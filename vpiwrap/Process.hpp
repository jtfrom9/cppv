#ifndef PROCESS_HPP
#define PROCESS_HPP

#include "boost/noncopyable.hpp"
#include "boost/shared_ptr.hpp"

#include "objects.hpp"

class Context;
class Command;

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
    void wait( VPIObject::ptr obj );
    Process* create(Process* proc);

public:
    // for ProcessManager
    void next();
    bool end();
    Command* receive();

    // for global functions
    friend void delay( int cycle );
    friend void wait( Process* proc );
    friend void wait( VPIObject::ptr obj );
    friend Process* create( Process* proc );
};


// global functions
void delay( int cycle );
void wait( Process* proc );
void wait( VPIObject::ptr proc );
Process* create( Process* proc );

#endif
