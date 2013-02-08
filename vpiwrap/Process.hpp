#ifndef PROCESS_HPP
#define PROCESS_HPP

#include "boost/noncopyable.hpp"

#include "ProcessManager.hpp"

class Context;
class Command;

class ISignal {};

void delay(int cycle);

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

    // for global functions
    friend void delay(int cycle);

    // for inherited Process classes
    void delay(int cycle);
    void wait(ISignal signal);

public:
    // for ProcessManager
    void next();
    bool end();
    Command* receive();
};

#endif
