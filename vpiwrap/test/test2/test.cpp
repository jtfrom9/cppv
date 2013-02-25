#include<iostream>
using std::cout;
using std::endl;

#include "Process.hpp"
using namespace vpi;
using vpi::wait;

struct signals {
    Reg& clk;
    Reg& n_rst;
    Reg& addr;
    Reg& dwrite;
    Wire& dread;
    Reg& req;
    Reg& wen;
    Wire& ack;

    signals():
        clk    (top().getReg("clk")),
        n_rst  (top().getReg("n_rst")),
        addr   (top().getReg("addr")),
        dwrite (top().getReg("dwrite")),
        dread  (top().getWire("dread")),
        req    (top().getReg("req")),
        wen    (top().getReg("wen")),
        ack    (top().getWire("ack"))
    {}
};
signals* sig;


void clkgen() {
    while(true) {
        sig->clk.write(0);
        delay(10);
        sig->clk.write(1);
        delay(10);
    }
};

void posedge_clkmon() {
    while(true) {
        wait(posedge(sig->clk));
        cout << format("%04d: clk=%0d") % sim_time() % sig->clk << endl;
    }
}

void init()
{
    sig = new signals();
    //Module& m = top().getModule("hoge");

    sig->clk.write(0);
    sig->n_rst.write(1);
    sig->addr.write(0);
    sig->dwrite.write(0);
    sig->req.write(0);
    sig->wen.write(0);
}

void reset(int cycle)
{
    sig->n_rst.write(0);
    delay(cycle);
    sig->n_rst.write(1);
}

void write(int _addr, int _dat)
{
    sig->addr.write(_addr * 2);
    sig->dwrite.write(_dat);
    sig->req.write(1);
    sig->wen.write(1);
    
    while( sig->ack.readi() == 0 )      // wait ack==1
        wait( posedge(sig->clk) );
    
    sig->req.write(0);

    while( sig->ack.readi() == 1 )      // wait ack==0
        wait( posedge(sig->clk) );
}

int read(int _addr)
{
    int ret;
    sig->addr.write(_addr * 2);
    sig->req.write(1);
    sig->wen.write(0);
    
    while( sig->ack.readi() == 0 )
        wait( posedge(sig->clk) );

    ret = sig->dread;
    
    sig->req.write(0);
    
    while( sig->ack.readi() == 1 )      // wait ack==0
        wait( posedge(sig->clk) );

    return ret;
}

int vmain(int argc, char *argv[])
{
    init();
    delay(100);
    reset(100);
    delay(100);

    //create("pos_clkmon", posedge_sig->clkmon);
    create("clkgen",clkgen);

    delay(1000);

    for(int addr=0; addr<8*1024; addr++) {
        write(addr,addr);
        wait( posedge(sig->clk) );
    }

    for(int addr=0; addr<8*1024; addr++) {
        cout << format("addr=%04x, data=%04x") % addr % read(addr) << endl;
        wait( posedge(sig->clk) );
    }

    delay(1000);
    finish();
    return 0;
}

