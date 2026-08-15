#include <rtthread.h>
#include "rtt_test.h"

#define THREAD_PRIORITY      25
#define THREAD_STACK_SIZE    512
#define THREAD_TIMESLICE     5

void thread1_entry(void *parameter)
{
    int i;
    char *ptr = RT_NULL;
    for (i = 0; ; i++)
    {
        ptr = rt_malloc(1 << i);

        if (ptr != RT_NULL)
        {
            rt_kprintf("get memory :%d byte\n", (1 << i));
            rt_free(ptr);
            rt_kprintf("free memory :%d byte\n", (1 << i));
            ptr = RT_NULL;
        }
        else
        {
            rt_kprintf("try to get %d byte memory failed!\n", (1 << i));
            return;
        }
    }
}

int rtt_mem_test(int argc, char **argv)
{
    rt_thread_t tid = RT_NULL;

    tid = rt_thread_create("thread1",
                            thread1_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY,
                            THREAD_TIMESLICE);

    if (tid != RT_NULL)
        rt_thread_startup(tid);
    return 0;
}
