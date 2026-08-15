#include <rtthread.h>
#include <rthw.h>
#include "rtt_test.h"

#define THREAD_PRIORITY      20
#define THREAD_STACK_SIZE    1024
#define THREAD_TIMESLICE     5

static rt_thread_t tid = RT_NULL;

volatile static int hook_times = 0;

static void idle_hook()
{
    if (0 == (hook_times % 10000))
    {
        rt_kprintf("enter idle hook %d times.\n", hook_times);
    }
    rt_enter_critical();
    hook_times++;
    rt_exit_critical();
}

static void thread_entry(void *parameter)
{
    int i = 5;
    while (i--)
    {
        rt_kprintf("enter thread1.\n");
        rt_enter_critical();
        hook_times = 0;
        rt_exit_critical();
        rt_kprintf("thread1 delay 50 OS Tick.\n", hook_times);
        rt_thread_mdelay(500);
    }
    rt_kprintf("delete idle hook.\n");
    rt_thread_idle_delhook(idle_hook);
    rt_kprintf("thread1 finish.\n");
}

int rtt_idlehook_test(int argc, char **argv)
{

    rt_thread_idle_sethook(idle_hook);

    tid = rt_thread_create("thread1",
                            thread_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}
