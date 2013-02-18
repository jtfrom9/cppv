#include<iostream>
using std::cout;
using std::endl;

#include "vpi.hpp"

void clkgen() {
    Reg* clk = top().get_reg("clk");
    while(true) {
        clk->write(0);
        delay(10);
        clk->write(1);
        delay(10);
    }
};

void monitor() {
    Wire* clko = top().get_wire("clko");
    while(true) {
        wait(clko);
        cout << format("time: %4d, clko=%s") % sim_time() % clko->readvec() 
             << endl;
    }
};

void geninput() {
    Reg* A = top().get_reg("A");
    Reg* B = top().get_reg("B");

    A->write(0);
    B->write(0);

    for(int a=0; a<16; a++) {
        A->write(a);
        delay(20);
        for(int b=0; b<16; b++) {
            B->write(b);
            delay(3);
        }
    }
}

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

    Process* clkgen_p = create("clkgen",clkgen);
    Process* mon_p    = create("monitor", monitor);
    Process* mon2_p   = create("monitor2", monitor2);

    delay(1000);
    //terminate(p);
    //terminate(p2);
    
    delay(1000);
    finish();

    cout << "vmain end" << endl;
    return 0;
}

