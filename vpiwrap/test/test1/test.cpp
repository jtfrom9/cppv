#include<iostream>
using std::cout;
using std::endl;

#include "Process.hpp"
using namespace vpi;
using vpi::wait;

void clkgen(void* arg) {
    Reg* clk = top().getReg_p("clk");
    while(true) {
        clk->write(0);
        delay(10);
        clk->write(1);
        delay(10);
    }
};

void datagen(void* arg) {
    Reg* A = top().getReg_p("A");
    Reg* B = top().getReg_p("B");

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

void outmon(void* arg) {
    Wire* C = top().getWire_p("C");
    ISignal* A = top().getReg_p("A");
    ISignal* B = top().getReg_p("B");
    while(true) {
        wait(C);
        cout << format("time: %4d, A=%s, B=%s, C=%s") 
            % sim_time() 
            % A->readv()
            % B->readv()
            % C->readv()
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

