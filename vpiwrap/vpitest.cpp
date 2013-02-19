#include<iostream>
using std::cout;
using std::endl;

#include "Process.hpp"
using namespace vpi;
using vpi::wait;

void clkgen() {
    Reg* clk = top().get_reg("clk");
    while(true) {
        clk->write(0);
        delay(10);
        clk->write(1);
        delay(10);
    }
};

void datagen() {
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

void outmon() {
    Wire* C = top().get_wire("C");
    IReadableSignal* A = top().get_reg("A");
    IReadableSignal* B = top().get_reg("B");
    while(true) {
        wait(C);
        cout << format("time: %4d, A=%s, B=%s, C=%s") 
            % sim_time() 
            % A->readvec()
            % B->readvec()
            % C->readvec()
             << endl;
    }
};

int vmain(int argc, char *argv[])
{
    cout << "vmain start" << endl;

    vecval v = make_vecval2("0011zX10");
    cout << format("vecval = %s") % v  << endl;
    cout << "vecval = " << v.bits_str() << endl;

    Process* clkgen_p = create("clkgen",clkgen);
    Process* dgen_p   = create("datagen", datagen);
    Process* omon_p   = create("outmon", outmon);

    wait(dgen_p);
    
    delay(1000);
    finish();

    cout << "vmain end" << endl;
    return 0;
}

