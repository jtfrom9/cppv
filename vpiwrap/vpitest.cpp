#include<iostream>
using std::cout;
using std::endl;

#include "boost/format.hpp"
using boost::format;

#include "Simulator.hpp"
#include "Process.hpp"
#include "ProcessManager.hpp"

#include "Value.hpp"

void clkgen() {
    Reg* clk = top().get_reg("clk");

    for(int i=0; i<5; i++) {
        delay(10);
        clk->write(0);
        delay(10);
        clk->write(1);
        delay(10);
    }
};

void monitor() {
    Wire* clko = top().get_wire("clko");
    //for(int i=0; i<10; i++) {
    while(true) {
        wait(clko);
        cout << format("time: %4d, clko=%s") % sim_time() % clko->readvec() 
             << endl;
    }
};

void monitor2() {
    Wire* C = top().get_wire("C");
    while(true) {
        wait(C);
        cout << format("time: %4d, C=%s") % sim_time() % C->readvec()
             << endl;
    }
};

int vmain(int argc, char *argv[])
{
    cout << "vmain start" << endl;

    vecval v = make_vecval2("0011zX10");
    cout << format("vecval = %s") % v  << endl;
    cout << "vecval = " << v << endl;

    Process* p = create("clkgen",clkgen);
    Process* p2 = create("monitor", monitor);
    Process* p3 = create("monitor2", monitor2);
    wait(p);
    wait(p2);
    wait(p3);
    cout << "vmain end" << endl;
    return 0;
}

