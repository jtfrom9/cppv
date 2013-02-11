#include<iostream>

#include "Simulator.hpp"
#include "Process.hpp"
#include "ProcessManager.hpp"

using std::cout;
using std::endl;

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
        Reg::ptr clk = sim.getModule(0).get_reg("clk");
        //Wire::ptr clko = sim.getModule(0).get_wire("clko");

        for(int i=0; i<10; i++) {
            //cout << "time: " << get_time() << endl;
            clk->write(0);
            //cout << "clko = " << clko->read() << endl;
            delay(10);
            clk->write(1);
            //cout << "clko = " << clko->read() << endl;
            delay(10);
        }
    }
};

int vmain(int argc, char *argv[])
{
    cout << "vmain start" << endl;
    Process* p = create(new clkgen("clkgen"));
    wait(p);
    cout << "vmain end" << endl;
    return 0;
}

