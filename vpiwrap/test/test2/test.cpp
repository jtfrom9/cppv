#include<iostream>
using std::cout;
using std::endl;

#include "Process.hpp"
using namespace vpi;
using vpi::wait;

Reg* clk;
Reg* addr;
Reg* data;
Reg* req;
Reg* wen;
Wire* ack;

void clkgen() {
    while(true) {
        clk->write(0);
        delay(10);
        clk->write(1);
        delay(10);
    }
};

void wait_pos(IReadableSignal* sig)
{
    while(true) {
        wait(sig);
        sig->readvec()
    }
}

void write(int _addr, int _dat)
{
    addr.write(_addr);
    req.write(1);
    wait(clk);
    
}

int vmain(int argc, char *argv[])
{
    create("clkgen",clkgen);

    clk  = top().get_reg("clk");
    addr = top().get_reg("addr");
    data = top().get_reg("dwrite");
    req  = top().get_reg("req");
    wen  = top().get_reg("wen");
    ack = top().get_reg("ack");
    
    for(int addr=0; addr<16; addr++) {
        write(addr,addr);
    }

    return 0;
}

