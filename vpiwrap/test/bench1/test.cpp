#include <iostream>
#include <string>
#include<iostream>

using std::cout;
using std::endl;
using std::string;

#include <boost/program_options.hpp>
using namespace boost::program_options;

#include "Process.hpp"
using namespace vpi;
using vpi::wait;


void clkgen()
{
    Reg& clk = top().getReg("clk");
    while(true) {
        clk.write(0);
        delay(10);
        clk.write(1);
        delay(10);
    }
}

int vmain(int argc, char *argv[])
{
    unsigned int cycle = 1<<22;

    options_description opt("options");
    opt.add_options()
        ("cycle" , value<unsigned int>( &cycle ),  "# of cycle")
        ("vclkgen",                                "clock genarated by verilog")
        ;

    variables_map vm;
    store(parse_command_line(argc, argv, opt), vm);
    notify(vm);

    // if( vm.count("help") || !vm.count("op") )
    //     cout << opt << endl;
    // else
    // {
    //     unsigned int cycle = vm["cycle"];
    // }

    if(!vm.count("vclkgen")) {
        cout << "vmain: cycle = " << cycle << endl;
        create("clkgen",clkgen);
    }

    return 0;
}


