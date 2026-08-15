#include <rtthread.h>
#include "rtt_test.h"

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

static rt_thread_t tid1 = RT_NULL;

void thread1_signal_handler(int sig)
{
    rt_kprintf("thread1 received signal %d\n", sig);
}

static void thread1_entry(void *parameter)
{
    int cnt = 0;

    rt_signal_install(SIGUSR1, thread1_signal_handler);
    rt_signal_unmask(SIGUSR1);

    while (cnt < 10)
    {
        rt_kprintf("thread1 count : %d\n", cnt);
        cnt++;
        rt_thread_mdelay(100);
    }
}

int rtt_signal_test(int argc, char **argv)
{

    tid1 = rt_thread_create("thread1",
                            thread1_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);
    rt_thread_mdelay(300);

    rt_thread_kill(tid1, SIGUSR1);
    return 0;
}

