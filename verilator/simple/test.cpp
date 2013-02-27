#include "Vtest.h"
#include "verilated.h"

#include <iostream>
#include <cstdio>
using namespace std;

int main(int argc, char **argv, char **env) 
{
    Verilated::commandArgs(argc, argv);
    Vtest* top = new Vtest;

    vluint64_t _time = 0;
   
    while (!Verilated::gotFinish()) { 
        if(top->clk==0) {
            cout << "clk=" << (int)top->clk << ", count=" << top->count << endl;
            //printf("clk=%d\n",top->clk);
        }
        
        top->clk = (_time % 2 == 0);

        //cout << top->A << endl;
        cout << "A=" << (int)top->v__DOT__A << endl;

        top->eval(); 
        _time++;
    }
    delete top;
    exit(0);
}
