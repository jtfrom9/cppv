#include<iostream>
#include<map>
using std::cout;
using std::endl;
using std::map;

#include "Process.hpp"
using namespace vpi;
using vpi::wait;

class DutSignalDriver {
    Reg& sig_clk;
    Reg& sig_n_rst;
    Reg& sig_addr;
    Reg& sig_dwrite;
    Wire& sig_dread;
    Reg& sig_req;
    Reg& sig_wen;
    Wire& sig_ack;

    static void clkgen(void* arg) {
        DutSignalDriver* dut = (DutSignalDriver*)arg;
        while(true) {
            dut->sig_clk.write(0);
            delay(10);
            dut->sig_clk.write(1);
            delay(10);
        }
    };

public:
    DutSignalDriver():
        sig_clk    (top().getReg("clk")),
        sig_n_rst  (top().getReg("n_rst")),
        sig_addr   (top().getReg("addr")),
        sig_dwrite (top().getReg("dwrite")),
        sig_dread  (top().getWire("dread")),
        sig_req    (top().getReg("req")),
        sig_wen    (top().getReg("wen")),
        sig_ack    (top().getWire("ack"))
    {}

    void init() {
        sig_clk.write(0);
        sig_n_rst.write(1);
        sig_addr.write(0);
        sig_dwrite.write(0);
        sig_req.write(0);
        sig_wen.write(0);
    }

    void start_clock() {
        create("clkgen",clkgen,this);
    }

    void wait_pos_clk() {
        wait(posedge(sig_clk));
    }

    void reset(int cycle)
    {
        sig_n_rst.write(0);
        delay(cycle);
        sig_n_rst.write(1);
    }

    void write(int addr, int dat)
    {
        sig_addr.write(addr * 2);
        sig_dwrite.write(dat);
        sig_req.write(1);
        sig_wen.write(1);

        while( sig_ack.readi() == 0 )      // wait ack==1
            wait( posedge(sig_clk) );

        sig_req.write(0);

        while( sig_ack.readi() == 1 )      // wait ack==0
            wait( posedge(sig_clk) );
    }

    int read(int addr)
    {
        int ret;
        sig_addr.write(addr * 2);
        sig_req.write(1);
        sig_wen.write(0);

        while( sig_ack.readi() == 0 )
            wait( posedge(sig_clk) );

        ret = sig_dread.readi();

        sig_req.write(0);

        while( sig_ack.readi() == 1 )      // wait ack==0
            wait( posedge(sig_clk) );

        return ret;
    }
};


int vmain(int argc, char *argv[])
{
    DutSignalDriver dut;

    dut.init();
    dut.start_clock();

    delay(100);
    dut.reset(100);

    delay(1000);

    map<int,int> memory;

    for(int i=0; i<100; i++) {
        int addr = rand() % (8*1024);
        int data = rand() % (8*1024);
        dut.write( addr, data );
        memory[ addr ] = data;     // backup addr/data pair
        cout << format("write. addr:%04x, data:%04x") % addr % data << endl;
        dut.wait_pos_clk();
    }

    int check=0;
    int fail=0;
    for(map<int,int>::iterator iter=memory.begin(); iter!=memory.end(); iter++) {
        int addr  = iter->first;
        int rdata = dut.read(addr);
        check++;
        if(rdata == memory[addr]) {
            cout << format("ok. addr:%04x, data:%04x") % addr % rdata << endl;
        } else {
            cout << format("ng. addr:%04x, data:%04x (expected:%04x)") % addr % rdata % memory[addr] << endl;
            fail++;
        }
        dut.wait_pos_clk();
    }

    cout << format(">> %0d fails / %0d test") % fail % check << endl;
    cout << ((fail==0) ? "OK" : "NG") << endl;

    delay(1000);
    finish();
    return 0;
}

