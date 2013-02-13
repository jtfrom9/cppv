#include<iostream>
using std::cout;
using std::endl;

#include "boost/format.hpp"
using boost::format;

#include "Simulator.hpp"
#include "Process.hpp"
#include "ProcessManager.hpp"


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
        // for(int i=0; i<10; i++) {
        while(true) {
            wait(clko);
            cout << format("time: %4d, clko=%s") % get_time() % clko->readb() << endl;
        }
    }
};

int vmain(int argc, char *argv[])
{
    cout << "vmain start" << endl;
    Process* p = create(new clkgen("clkgen"));
    Process* p2 = create(new monitor());
    wait(p);
    wait(p2);
    cout << "vmain end" << endl;
    return 0;
}

