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

int board_set_rtc_time(int argc, char **argv)
{
    rclk_time_t timestamp = 0;
    struct rclk_tm input;

    if (argc != 7) {
        board_printf("cmd <yy> <mm> <dd> <hh> <mm> <ss>");
        return -1;
    }

    input._year = strtol(argv[1], NULL, 10) - 1900;
    input._mon = strtol(argv[2], NULL, 10) - 1;
    input._mday = strtol(argv[3], NULL, 10);
    input._hour = strtol(argv[4], NULL, 10);
    input._min = strtol(argv[5], NULL, 10);
    input._sec = strtol(argv[6], NULL, 10);

    if (!rclk_tm_to_timstamp(&input, &timestamp)) {
        board_printf("rtc time trans failed \r\n");
        return -1;
    }

    if (!board_rtc_set_timestamp(timestamp)) {
        board_printf("rtc time set failed \r\n");
        return -1;
    }

    board_printf("rtc set complete: %d/%02d/%02d %02d:%02d:%02d \r\n",
        input._year+1900, input._mon+1, input._mday,
        input._hour, input._min, input._sec);
    return 0;
}
MSH_FUNCTION_EXPORT_CMD(board_set_rtc_time, rtctimeset, set rtc time);

int board_get_rtc_time(int argc, char **argv)
{
    struct rclk_timeval now;
    rclk_time_t timestamp;

    struct rclk_tm output;
    timestamp = board_rtc_get_timestamp(&now);

    if (!rclk_timstamp_to_tm(timestamp, &output)) {
        board_printf("rtc time trans failed \r\n");
        return -1;
    }

    board_printf("rtc: %d/%02d/%02d %02d:%02d:%02d, timestamp:%d\r\n",
        output._year+1900, output._mon+1, output._mday,
        output._hour, output._min, output._sec, timestamp);
    return 0;
}
MSH_FUNCTION_EXPORT_CMD(board_get_rtc_time, rtctimeget, get rtc time);


int list_rtos(int argc, char **argv)
{
    TaskStatus_t *p_tasks;
    int sz, i;
    char status;

    uint32_t stack_base;
    uint32_t stack_hw;
    uint32_t stack_sz;
    uint32_t stack_used;

    uint32_t total_run = 0;
    uint32_t total_cnter = 0;
    uint32_t free_heap_sz = xPortGetFreeHeapSize();
    static uint32_t list_rtos_cnter = 0;

    sz = uxTaskGetNumberOfTasks();
    p_tasks = (TaskStatus_t *)pvPortMalloc(sz * sizeof(TaskStatus_t));
    if (!p_tasks) {
        return -1;
    }

    sz = uxTaskGetSystemState(p_tasks, sz, &total_run);
    if (total_run == 0) {
        total_run = 1;
    }

    for (i = 0; i < sz; i++) {
        TaskStatus_t *status = &p_tasks[i];
        if (strncmp(status->pcTaskName,"IDLE",4)) {
            total_cnter += status->ulRunTimeCounter;
        }
    }

    board_printf("[main]CPU usage: %3.2f - Clock freq %u\r\n",
        (float) total_cnter / total_run * 100.0f, configCPU_CLOCK_HZ);

    board_printf("[main]MEM usage: heap total size %8d, req %8d, free %8d\r\n",
        configTOTAL_HEAP_SIZE, configTOTAL_HEAP_SIZE - free_heap_sz, free_heap_sz);

    board_printf("[main]Timenow: %10u, delta: %4dmsec, RTOS tick %8d, RTOS delta %4d\r\n",
        xTaskGetTickCount()*portTICK_PERIOD_MS, 
        (xTaskGetTickCount() - list_rtos_cnter)*portTICK_PERIOD_MS, 
        xTaskGetTickCount(),
        (xTaskGetTickCount() - list_rtos_cnter));

    board_printf("[main]CS: %8d, IRQs num: %d, IRQs time: %d\r\n", 0, 0, 0);

/*
	eRunning = 0
	eReady = 1
	eBlocked = 2
	eSuspended = 3
	eDeleted = 4
*/
    for (i = 0; i < sz; i++) {
        TaskStatus_t *status = &p_tasks[i];
        TaskHandle_t ptcb = status->xHandle;

        uxTaskStatus(ptcb, &stack_base, &stack_hw, &stack_sz, &stack_used);

        float cpu_usage = (float)(status->ulRunTimeCounter) / (float)total_run * 100.0f;
        float stack_usage = (stack_sz > 0) ? ((float)stack_used / (float)stack_sz * 100.0f) : 0.0f;

        board_printf("[main][task] name %15s %x - prio :%2lu, bptr=%x, wptr=%x, size=%6lu, used=%6lu -> %5.1f - CPU usage :%.2f\r\n",
            status->pcTaskName,
            status->eCurrentState,
            (unsigned long) (status->uxCurrentPriority),
            stack_base,        // bptr
            stack_hw,   // wptr
            (unsigned long) stack_sz,
            (unsigned long) stack_used,
            (double) stack_usage,
            (double) cpu_usage
        );
    }

    vPortFree(p_tasks);

    list_rtos_cnter = xTaskGetTickCount();
    return 0;
}
MSH_FUNCTION_EXPORT_CMD(list_rtos, rtosstatus, get rtos status);

#ifdef __cplusplus
}
#endif
