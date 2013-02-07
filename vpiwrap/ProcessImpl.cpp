#include "ProcessImpl.hpp"

#include <vector>
#include <algorithm>
#include <stdexcept>

using std::vector;
using std::invalid_argument;

Process::ptr ProcessManagerImpl::add(Process* process)
{
    if (process==0)
        throw invalid_argument("process == null");

    ProcessImpl::ptr ptr( dynamic_cast<ProcessImpl*>(process) );

    if (std::find(_processes.begin(), _processes.end(), ptr) != _processes.end())
        throw invalid_argument("already registered");

    ptr->setManager(this);
    _processes.push_back(ptr);
    return ptr;
}

void ProcessManagerImpl::start()
{
    for(vector<ProcessImpl::ptr>::iterator p=_processes.begin();
        p != _processes.end(); ++p) 
    {
        int value;

        (*p)->next(); // 実行(ここでコンテキストスイッチ)
            
        if((*p)->receive(&value)) {
            // yeild commandの処理(タイマー、信号変化コールバック)
        }
        if((*p)->end()) {
            // Processの終了。_processesから外すか、別のリストに入れるか
        }
    }
    // 全部終った終了。start()自体がVPIコールバック内で呼ばれるので
    // VPIコールバックが終了し、シミュレータの動作に遷移
}

/*
struct test_gen: public generic_generator<int>
{
    std::vector<Module::ptr> modules;

    void wait(int cycle) {
        yield_send(cycle);
    }
    
    void body() {
        Reg::ptr p = modules[0]->get_reg("clk");
        //Port::ptr p = modules[0]->_modules[0]->get_port("clk");
        p->write(1);
        wait(10);
    }
};

void test( const std::vector<Module::ptr>& modules )
{
    test_gen tg;
    tg.modules = modules;

    try {
        while(true) {
            int cycle;
            tg.next();
            if(tg.receive(&cycle)) {
                // sleep cycle time;
                std::cout << "sleep: " << cycle << std::endl;
                settimer( 100, 

            }
        } catch(const stop_iteration& e) {
            std::cout << "fin" << std::endl;
        }
    }
}
*/

