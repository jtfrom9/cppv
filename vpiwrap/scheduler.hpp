#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include<set>
#include<stdexcept>
#include<tr1/memory>

using std::tr1::shared_ptr;

class ITimerEventObserver
{
public:
    virtual void onTimer() = 0;
    
    typedef shared_ptr<ITimerEventObserver> ptr;
};


class TimerEvent
{
private:
    std::set<ITimerEventObserver::ptr> _observers;

    typedef std::set<ITimerEventObserver::ptr>::iterator iterator;

    static void handler( s_cb_data* pcbdata )
    {
        TimerEvent* event = (TimerEvent*)pcbdata->user_data;
        event->raise();
    }

    void raise()
    {
        for(iterator i=_observers.begin(); i!=_observers.end(); i++) {
            (*i)->onTimer();
        }
    }


public:
    void add( ITimerEventObserver::ptr o )
    {
        _observers.insert( o );
    }

    // void delete( ITimerEventObserver::ptr o )
    // {
    //     _observers.erase(o);
    // }

    void set( int cycle )
    {
        s_cb_data cbdata;
        s_vpi_time time;
        typedef PLI_INT32 handler_type( s_cb_data* );

        if (_observers.empty())
            throw std::runtime_error("TimerEvent: no observers added");

        time.type = vpiSimTime;
        time.high = 0;
        time.low  = cycle;

        cbdata.reason    = cbAfterDelay;
        cbdata.cb_rtn    = (handler_type*)handler;
        cbdata.obj       = NULL;
        cbdata.time      = &time;
        cbdata.value     = NULL;
        cbdata.user_data = (PLI_BYTE8*)this;

        vpi_register_cb(&cbdata);
    }
};

long long get_time() {
    s_vpi_time time;
    memset(&time,0,sizeof(time));
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

        Reg::ptr r = ::modules[0]->get_reg("clk");
        v = (v) ? 0 : 1;
        r->write(v);

    }
};


#endif
