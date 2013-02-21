#include<iostream>
using std::cout;
using std::endl;

#include "Process.hpp"
using namespace vpi;
using vpi::wait;

Reg* clk;
Reg* n_rst;
Reg* addr;
Reg* dwrite;
Wire* dread;
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

void posedge_clkmon() {
    while(true) {
        wait(posedge(clk));
        cout << format("%04d: clk=%0d") % sim_time() % clk->readv() << endl;
    }
}

void init()
{
    clk    = top().get_reg("clk");
    n_rst  = top().get_reg("n_rst");
    addr   = top().get_reg("addr");
    dwrite = top().get_reg("dwrite");
    dread  = top().get_wire("dread");
    req    = top().get_reg("req");
    wen    = top().get_reg("wen");
    ack    = top().get_wire("ack");

    clk->write(0);
    n_rst->write(1);
    addr->write(0);
    dwrite->write(0);
    req->write(0);
    wen->write(0);
}

void reset(int cycle)
{
    n_rst->write(0);
    delay(cycle);
    n_rst->write(1);
}

void write(int _addr, int _dat)
{
    addr->write(_addr * 2);
    dwrite->write(_dat);
    req->write(1);
    wen->write(1);
    
    while( ack->readi() == 0 )      // wait ack==1
        wait( posedge(clk) );
    
    req->write(0);

    while( ack->readi() == 1 )      // wait ack==0
        wait( posedge(clk) );
}

int read(int _addr)
{
    int ret;
    addr->write(_addr * 2);
    req->write(1);
    wen->write(0);
    
    while( ack->readi() == 0 )
        wait( posedge(clk) );

    ret = dread->readi();
    
    req->write(0);
    
    while( ack->readi() == 1 )      // wait ack==0
        wait( posedge(clk) );

    return ret;
}

int vmain(int argc, char *argv[])
{
    init();
    delay(100);
    reset(100);
    delay(100);

    //create("pos_clkmon", posedge_clkmon);
    create("clkgen",clkgen);

    delay(1000);

    for(int addr=0; addr<8*1024; addr++) {
        write(addr,addr);
        wait( posedge(clk) );
    }

    for(int addr=0; addr<8*1024; addr++) {
        cout << format("addr=%04x, data=%04x") % addr % read(addr) << endl;
        wait( posedge(clk) );
    }

    delay(1000);
    finish();
    return 0;
}

