#include <rtthread.h>
#include "rtt_test.h"

#define THREAD_PRIORITY      10
#define THREAD_TIMESLICE     5

static struct rt_mailbox mb;

static char mb_pool[128];
static char mb_str1[] = "I'm a mail!";
static char mb_str2[] = "this is another mail!";
static char mb_str3[] = "over";
ALIGN(RT_ALIGN_SIZE)
static char thread1_stack[1024];
static struct rt_thread thread1;

static void thread1_entry(void *parameter)
{
    char *str;
    while (1)
    {
        rt_kprintf("thread1: try to recv a mail\n");
        if (rt_mb_recv(&mb, (rt_ubase_t *)&str, RT_WAITING_FOREVER) == RT_EOK)
        {
            rt_kprintf("thread1: get a mail from mailbox, the content:%s\n", str);
            if (str == mb_str3)
                break;
            rt_thread_mdelay(100);
        }
    }

    rt_mb_detach(&mb);
}

ALIGN(RT_ALIGN_SIZE)
static char thread2_stack[1024];
static struct rt_thread thread2;

static void thread2_entry(void *parameter)
{
    rt_uint8_t count;
    count = 0;
    while (count < 10)
    {
        count ++;
        if (count & 0x1)
        {
            rt_mb_send(&mb, (rt_uint32_t)&mb_str1);
        }
        else
        {
            rt_mb_send(&mb, (rt_uint32_t)&mb_str2);
        }

        rt_thread_mdelay(200);
    }

    rt_mb_send(&mb, (rt_uint32_t)&mb_str3);
}

int rtt_mailbox_test(int argc, char **argv)
{
    rt_err_t result;

    result = rt_mb_init(&mb,
                        "mbt",
                        &mb_pool[0],
                        sizeof(mb_pool) / 4,
                        RT_IPC_FLAG_FIFO);

    if (result != RT_EOK)
    {
        rt_kprintf("init mailbox failed.\n");
        return -1;
    }

    rt_thread_init(&thread1,
                    "thread1",
                    thread1_entry,
                    RT_NULL,
                    &thread1_stack[0],
                    sizeof(thread1_stack),
                    THREAD_PRIORITY, THREAD_TIMESLICE);
    rt_thread_startup(&thread1);

    rt_thread_init(&thread2,
                    "thread2",
                    thread2_entry,
                    RT_NULL,
                    &thread2_stack[0],
                    sizeof(thread2_stack),
                    THREAD_PRIORITY, THREAD_TIMESLICE);
    rt_thread_startup(&thread2);
    return 0;
}
