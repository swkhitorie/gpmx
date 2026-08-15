#include <rtthread.h>
#include "rtt_test.h"

#define THREAD_STACK_SIZE   1024
#define THREAD_PRIORITY     20
#define THREAD_TIMESLICE    10

volatile rt_uint32_t count[2];

static void thread_entry(void *parameter)
{
    rt_uint32_t value;
    value = (rt_uint32_t)parameter;
    while (1)
    {
        rt_kprintf("thread %d is running\n", value);
        rt_thread_mdelay(1000);
    }
}

static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;
static void hook_of_scheduler(struct rt_thread *from, struct rt_thread *to)
{
    rt_kprintf("from: %s -->  to: %s \n", from->name, to->name);
}

int rtt_schedulerhook_test(int argc, char **argv)
{
    rt_scheduler_sethook(hook_of_scheduler);

    tid1 = rt_thread_create("thread1",
                            thread_entry, (void *)1,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);

    tid2 = rt_thread_create("thread2",
                            thread_entry, (void *)2,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE - 5);
    if (tid2 != RT_NULL)
        rt_thread_startup(tid2);

    return 0;
}
