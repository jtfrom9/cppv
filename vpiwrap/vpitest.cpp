#include<iostream>

#include "scheduler.hpp"
#include "Simulator.hpp"

using std::cout;
using std::endl;

long long get_time() {
    s_vpi_time time;
    std::memset(&time,0,sizeof(time));
    time.type = vpiSimTime;
    vpi_get_time(NULL,&time);
    return (long long)time.high << 32 | (long long)time.low;
}

class TimerHandler1: public ITimerEventObserver
{
public:
    void onTimer() {
        static int v = 0;
        std::cout << "onTimer:" << get_time() << std::endl;

        Simulator& sim = Simulator::getSimulator();
        Reg::ptr r = sim.getModule(0).get_reg("clk");
        v = (v) ? 0 : 1;
        r->write(v);
    }
};

int vmain(int argc, char *argv[])
{
    Simulator& sim = Simulator::getSimulator();

    cout << "main" << endl;

    for(int i=0; i<sim.numOfModule(); i++) {
        Module& m = sim.getModule(i);
        std::cout << "[" << i << "] " 
                  << m.type_str() 
                  << ": " << m.name()
                  << ": " << m.fullname()
                  << ": " << m.to_str()
                  << std::endl;
    }

    // for(unsigned int i=0; i<modules[0]->_modules.size(); i++) {
    //     std::cout << "[" << i << "] " 
    //               << modules[0]->_modules[i]->type_str() 
    //               << ": " << modules[0]->_modules[i]->name()
    //               << ": " << modules[0]->_modules[i]->fullname()
    //               << ": " << modules[0]->_modules[i]->to_str()
    //               << std::endl;
    // }

    TimerEvent *te = new TimerEvent();
    te->add(shared_ptr<TimerHandler1>(new TimerHandler1()));
    te->set(10);
    te->set(30);
    te->set(50);
    te->set(70);
    te->set(90);
    return(0);

}

