#include<iostream>
using std::cout;
using std::endl;

#include "boost/format.hpp"
using boost::format;

#include "Simulator.hpp"
#include "Process.hpp"
#include "ProcessManager.hpp"

#include "Value.hpp"

long long get_time() {
    s_vpi_time time;
    std::memset(&time,0,sizeof(time));
    time.type = vpiSimTime;
    vpi_get_time(NULL,&time);
    return (long long)time.high << 32 | (long long)time.low;
}

class clkgen: public Process
{
public:
    clkgen( const char* name ): Process( name ) {}

protected:
    void main() {
        const Simulator& sim = ProcessManager::get().getSimulator();
        Reg* clk = sim.getModule(0).get_reg("clk");

        for(int i=0; i<5; i++) {
            delay(10);
            clk->write(0);
            delay(10);
            clk->write(1);
            delay(10);
        }
    }
};

class monitor: public Process
{
public:
    monitor(): Process("monitor") {}
protected:
    void main() {
        const Simulator& sim = ProcessManager::get().getSimulator();
        Wire* clko = sim.getModule(0).get_wire("clko");
        //for(int i=0; i<10; i++) {
        while(true) {
            wait(clko);
            cout << format("time: %4d, clko=%s") % get_time() % clko->readvec().to_int_str() << endl;
        }
    }
};

class monitor2: public Process
{
public:
    monitor2(): Process("monitor2") {}
protected:
    void main() {
        const Simulator& sim = ProcessManager::get().getSimulator();
        Wire* C = sim.getModule(0).get_wire("C");
        while(true) {
            wait(C);
            cout << format("time: %4d, C=%s") % get_time() % C->readvec().to_int_str() 
                 << endl;
        }
    }
};


int vmain(int argc, char *argv[])
{
    cout << "vmain start" << endl;

    vecval v = make_vecval2("0011zX10");
    cout << format("vecval = %s") % v  << endl;
    cout << "vecval = " << v << endl;

    Process* p = create(new clkgen("clkgen"));
    Process* p2 = create(new monitor());
    Process* p3 = create(new monitor2());
    wait(p);
    wait(p2);
    wait(p3);
    cout << "vmain end" << endl;
    return 0;
}

