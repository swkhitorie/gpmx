#include <rthw.h>
#include <rtthread.h>
#include "rtt_test.h"

#define THREAD_PRIORITY      20
#define THREAD_STACK_SIZE    512
#define THREAD_TIMESLICE     5

static rt_uint32_t cnt;
void thread_entry(void *parameter)
{
    rt_uint32_t no;
    rt_uint32_t level;
    no = (rt_uint32_t) parameter;
    while (1)
    {
        level = rt_hw_interrupt_disable();
        cnt += no;
        rt_hw_interrupt_enable(level);
        rt_kprintf("protect thread[%d]'s counter is %d\n", no, cnt);
        rt_thread_mdelay(no * 10);
    }
}

int rtt_irq_test(int argc, char **argv)
{
    rt_thread_t thread;
    thread = rt_thread_create("thread1", thread_entry, (void *)10,
                                THREAD_STACK_SIZE,
                                THREAD_PRIORITY, THREAD_TIMESLICE);
    if (thread != RT_NULL)
        rt_thread_startup(thread);

    thread = rt_thread_create("thread2", thread_entry, (void *)20,
                                THREAD_STACK_SIZE,
                                THREAD_PRIORITY, THREAD_TIMESLICE);
    if (thread != RT_NULL)
        rt_thread_startup(thread);
    return 0;
}
