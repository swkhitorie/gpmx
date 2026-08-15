#include <rtthread.h>
#include "rtt_test.h"

static rt_timer_t timer1;
static rt_timer_t timer2;
static int cnt = 0;

static void timeout1(void *parameter)
{
    rt_kprintf("periodic timer is timeout %d\n", cnt);
    if (cnt++ >= 9)
    {
        rt_timer_stop(timer1);
        rt_kprintf("periodic timer was stopped! \n");
    }
}

static void timeout2(void *parameter)
{
    rt_kprintf("one shot timer is timeout\n");
}

int rtt_timer_test(int argc, char **argv)
{
    timer1 = rt_timer_create("timer1", timeout1,
                            RT_NULL, 10,
                            RT_TIMER_FLAG_PERIODIC);
    if (timer1 != RT_NULL)
        rt_timer_start(timer1);

    timer2 = rt_timer_create("timer2", timeout2,
                            RT_NULL,  30,
                            RT_TIMER_FLAG_ONE_SHOT);

    if (timer2 != RT_NULL)
        rt_timer_start(timer2);
    return 0;
}
