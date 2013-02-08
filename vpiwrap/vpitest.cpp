#include<iostream>

#include "Simulator.hpp"
#include "Process.hpp"

using std::cout;
using std::endl;

long long get_time() {
    s_vpi_time time;
    std::memset(&time,0,sizeof(time));
    time.type = vpiSimTime;
    vpi_get_time(NULL,&time);
    return (long long)time.high << 32 | (long long)time.low;
}

void delay(int cycle)
{
    Process *currentProcess = ProcessManager::get().getCurrent();
    currentProcess->wait(cycle);
}

int vmain(int argc, char *argv[])
{
    const Simulator& sim = ProcessManager::get().getSimulator();
    Reg::ptr clk = sim.getModule(0).get_reg("clk");

    for(int i=0; i<10; i++) {
        cout << "time: " << get_time() << endl;
        clk->write(0);
        delay(10);
        clk->write(1);
        delay(10);
    }
    return 0;
}

