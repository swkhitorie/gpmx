#include <gpmx/config.h>

#include <driver/drv_hrt.h>
#include <mlog.h>
#include "hrt_test.h"

/**
 * drv_hrt support: f4, h7
 * this is debug example, do not call printf in hrt call, because it is in irq mode
 */
int call1_arg = 3;
struct hrt_call call1;
void hrt_call1(void *arg)
{
    float now = hrt_absolute_time()/1e6f;
    // KMRAW("%.6f call 1 run | iscalled?:%d | arg: %d | in irq?:%d \r\n", now,
    //      hrt_called(&call1), *(int *)arg, (__get_IPSR() != 0U));
    KMRAW("%.6f call 1 run | iscalled?:%d | arg: %d \r\n", now,
         hrt_called(&call1), *(int *)arg);
}

float call2_arg = 2.78f;
struct hrt_call call2;
void hrt_call2(void *arg)
{
    float now = hrt_absolute_time()/1e6f;
    KMRAW("%.6f call 2 run | iscalled?:%d | arg: %.5f \r\n", now, hrt_called(&call2), *(float *)arg);
}

double call3_arg = 3.141592678;
struct hrt_call call3;
void hrt_call3(void *arg)
{
    float now = hrt_absolute_time()/1e6f;
    KMRAW("%.6f call 3 run | iscalled?:%d | arg: %.5f \r\n", now, hrt_called(&call3), *(double *)arg);
}

#define ONE_S 1000000ULL
#define ONE_MS 1000ULL
int drv_hrt_test(int argc, char **argv)
{
    hrt_init();

    //call1: delay 2s, period 1s
    //call2: in abstime 6.2s
    //call3: after 1s
    hrt_call_every(&call1, 2*ONE_S, ONE_S, hrt_call1, &call1_arg);
    hrt_call_at(&call2, (6*ONE_S)+(200*ONE_MS), hrt_call2, &call2_arg);
    hrt_call_after(&call3, ONE_S, hrt_call3, &call3_arg);

    return 0;
}

