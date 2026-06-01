#include <board_config.h>

#include "gmsh.h"
#include "shell.h"
#include "rtx_mem.h"
#include "rtklib.h"

#if !defined(BOARD_PXBOARD_EBFV2)
#error rtklib_test example only support pxboard_ebfv2 
#endif

#define RTKLIB_MEMPOOL_TOTAL_SIZE          (1023*1024)
/**
 * stm32f4 -> 1hz single cpu usage 17%
 *         -> 5hz single cpu usage 80%
 * use rtknavi.exe to replot 1hz~5hz rover rtcm3 data
 * 1): input command "rtctimeset 2024 11 04 09 46 52<cr><lf>" by begin of data time (GPST)
 * 2): input command "rtkstart"
 */

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((section(".fmcsram"))) static uint8_t mempool[RTKLIB_MEMPOOL_TOTAL_SIZE];

gtime_t timeget()
{
    gtime_t time = {0};
    double ep[6] = {0};
    struct rclk_timeval now;
    rclk_time_t timestamp;
    struct rclk_tm output;
    timestamp = board_rtc_get_timestamp(&now);
    rclk_timstamp_to_tm(timestamp, &output);

    ep[0] = output._year+1900;
    ep[1] = output._mon+1;
    ep[2] = output._mday;
    ep[3] = output._hour;
    ep[4] = output._min;
    ep[5] = output._sec;

    time = epoch2time(ep);
#ifdef CPUTIME_IN_GPST /* cputime operated in gpst */
    time = gpst2utc(time);
#endif

    return timeadd(time, timeoffset_);
}

uint32_t tickget()
{
    return board_get_time();
}

void sleepms(int ms)
{
    vTaskDelay(ms);
}

void *rtkmalloc(size_t sz)
{
    if (sz == 0) {
        return (void*)0;
    }

    return rtx_mem_alloc(mempool, sz, 1);
}

void *rtkrealloc(void *p, size_t sz)
{
    void *tmp = rtkmalloc(sz);
    if (!tmp) {
        return tmp;
    }

    memcpy(tmp, p, sz);
    rtkfree(p);
    return tmp;
}

void *rtkcalloc(size_t num, size_t sz)
{
    return rtkmalloc(num*sz);
}

void rtkfree(void *p)
{
    rtx_mem_free(mempool, p);
}

void main_root(void *p)
{
    rtx_mem_init(&mempool[0], RTKLIB_MEMPOOL_TOTAL_SIZE);

    board_printf("main root start %d \r\n",xPortGetFreeHeapSize());
    gmsh_system_init();

    extern int rtklib_main(int argc, char **argv);
    rtklib_main(0, NULL);

    vTaskDelete(NULL);
}

int rtklibmem_free(int argc, char **argv)
{
    int i;

    board_printf("rtklib mempool total: %d, free: %d\r\n", RTKLIB_MEMPOOL_TOTAL_SIZE, 
        RTKLIB_MEMPOOL_TOTAL_SIZE - rtx_mem_used(mempool));

    return 0;
}
MSH_FUNCTION_EXPORT_CMD(rtklibmem_free, rtkmem, rtklib memory free display);

#ifdef __cplusplus
}
#endif
