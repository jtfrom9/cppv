#include<iostream>
#include<map>
using std::cout;
using std::endl;
using std::map;

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

void write(int addr, int dat)
{
    sig->addr.write(addr * 2);
    sig->dwrite.write(dat);
    sig->req.write(1);
    sig->wen.write(1);

    while( sig->ack.readi() == 0 )      // wait ack==1
        wait( posedge(sig->clk) );

    sig->req.write(0);

    while( sig->ack.readi() == 1 )      // wait ack==0
        wait( posedge(sig->clk) );
}

int read(int addr)
{
    int ret;
    sig->addr.write(addr * 2);
    sig->req.write(1);
    sig->wen.write(0);

    while( sig->ack.readi() == 0 )
        wait( posedge(sig->clk) );

    ret = sig->dread.readi();

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

    map<int,int> memory;

    for(int i=0; i<100; i++) {
        int addr = rand() % (8*1024);
        int data = rand() % (8*1024);
        write( addr, data );
        memory[ addr ] = data;     // backup addr/data pair
        cout << format("write. addr:%04x, data:%04x") % addr % data << endl;
        wait( posedge(sig->clk) );
    }

    int check=0;
    int fail=0;
    for(map<int,int>::iterator iter=memory.begin(); iter!=memory.end(); iter++) {
        int addr  = iter->first;
        int rdata = read(addr);
        check++;
        if(rdata == memory[addr]) {
            cout << format("ok. addr:%04x, data:%04x") % addr % rdata << endl;
        } else {
            cout << format("ng. addr:%04x, data:%04x (expected:%04x)") % addr % rdata % memory[addr] << endl;
            fail++;
        }
        wait( posedge(sig->clk) );
    }

    cout << format(">> %0d fails / %0d test") % fail % check << endl;
    cout << ((fail==0) ? "OK" : "NG") << endl;

    delay(1000);
    finish();
    return 0;
}

