#include <board_config.h>

#include "gmsh.h"
#include "shell.h"
#include "rtxmem.h"
#include "rtklib.h"

// #define CONFIG_RTKLIB_TIME_RTC

#if !defined(BOARD_PXBOARD_EBFV2)
// https://doc.embedfire.com/products/link/zh/latest/mcu/stm32/stm32f407_batianhu.html
#error rtklib_test example only support pxboard_ebfv2 
#endif

/**
 * stm32f4 -> 1hz single cpu usage 17%
 *         -> 5hz single cpu usage 80%
 * 
 * RealTime Mode (memory usage: ~290k)
 * use rtknavi.exe to replot 1hz~5hz rover rtcm3 data with time tag
 * 1): input command "system_time set [timestamp]" by begin of data time (GPST)
 * 2): input command "rtkstart"
 * 3): see NMEA Output
 * 
 * Replay mode (memory usage: ~300k (Fatfs use)):
 * 2): input command "rtkstart"
 * 3): see NMEA Output
 */

#ifdef __cplusplus
extern "C" {
#endif

#define RTKLIB_MEMPOOL_TOTAL_SIZE          (1023*1024)
__attribute__((section(".fmcsram"))) static uint8_t mempool[RTKLIB_MEMPOOL_TOTAL_SIZE];
uint32_t rtkport_tickget() { return board_get_time(); }
void     rtkport_sleepms(int ms) { vTaskDelay(ms); }
void *rtkport_malloc(size_t sz) { if (sz == 0) return NULL; return rtx_mem_alloc(mempool, sz, 1); }
void *rtkport_calloc(size_t num, size_t sz) { return rtkport_malloc(num*sz); }
void  rtkport_free(void *p) { rtx_mem_free(mempool, p); }
void *rtkport_realloc(void *p, size_t sz)
{
    void *tmp = rtkport_malloc(sz);
    if (!tmp) {
        return tmp;
    }

    memcpy(tmp, p, sz);
    rtkport_free(p);
    return tmp;
}

gtime_t rtkport_timeget()
{
    gtime_t time = {0};
#if !defined(CONFIG_RTKLIB_TIME_RTC)
    double ep[6] = {2024,12,04,8,0,0};
#else
    double ep[6] = {0};
    struct rtc_timeval now;
    rtc_time_t timestamp;
    struct rtc_tm output;
    timestamp = board_rtc_get_timestamp(&now);
    rtc_timstamp_to_tm(timestamp, &output);

    ep[0] = output._year+1900;
    ep[1] = output._mon+1;
    ep[2] = output._mday;
    ep[3] = output._hour;
    ep[4] = output._min;
    ep[5] = output._sec;
#endif

    time = epoch2time(ep);
#ifdef CPUTIME_IN_GPST 
    /* cputime operated in gpst */
    time = gpst2utc(time);
#endif

    return timeadd(time, timeoffset_);
}

void main_root(void *p)
{
    rtx_mem_init(&mempool[0], RTKLIB_MEMPOOL_TOTAL_SIZE);

    extern int rtklib_main(int argc, char **argv);
    rtklib_main(0, NULL);

    vTaskDelete(NULL);
}

int rtklibmem_free(int argc, char **argv)
{
    int i;

    board_printf("rtklib mempool total: %d, free: %d, used: %d\r\n", RTKLIB_MEMPOOL_TOTAL_SIZE, 
        RTKLIB_MEMPOOL_TOTAL_SIZE - rtx_mem_used(mempool), rtx_mem_used(mempool));

    return 0;
}
MSH_FUNCTION_EXPORT_CMD(rtklibmem_free, rtkmem, rtklib memory free display);

#ifdef __cplusplus
}
#endif
