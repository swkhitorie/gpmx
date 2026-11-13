#ifndef RTKCONFIG_H
#define RTKCONFIG_H

#include <gpmx/config.h>

#define NFREQ    3
#define NOCALLOC 0

#define ENACMP
#define ENAGAL
// #define ENAGLO
// #define ENAQZS
// #define ENAIRN
// #define ENALEO

// #define NEXOBS    3
// #define MAXOBS    64
// #define MAXSOLBUF 256
// #define MAXOBSBUF 10

#define CONFIG_RTKLIB_INPUT_STREAM     (2)
#define CONFIG_RTKLIB_OUTPUT_STREAM    (1)

#define CONFIG_RTKLIB_TIME_PORT

// #define CONFIG_RTKLIB_GIS_DATA
// #define CONFIG_RTKLIB_IONEX_TEC_MODEL_ENABLE
// #define CONFIG_RTKLIB_OPTIONS_EXTEND
// #define CONFIG_RTKLIB_OPTIONS_RINEX
// #define CONFIG_RTKLIB_DATUMTRANS
// #define CONFIG_RTKLIB_SBAS_CORR_ENABLE
// #define CONFIG_RTKLIB_SSR_ENABLE
// #define CONFIG_RTKLIB_PRECISE_EPH_CLK_ENABLE
// #define CONFIG_RTKLIB_POST_PROCESS_ENABLE
// #define CONFIG_RTKLIB_PPP_ENABLE
// #define CONFIG_RTKLIB_RINEX_ENABLE
// #define CONFIG_RTKLIB_PRECISE_EPH_CLK_ENABLE
// #define CONFIG_RTKLIB_RTCM2_ENABLE
// #define CONFIG_RTKLIB_RTCM3_OUTPUT_ENABLE
// #define CONFIG_RTKLIB_ALMANAC_ENABLE
// #define CONFIG_RTKLIB_DGPS_ENABLE
// #define CONFIG_RTKLIB_EPH_FNAV_ENABLE
// #define CONFIG_RTKLIB_LOCAL_TIME_SYNC_ENABLE
// #define CONFIG_RTKLIB_ANTENNA_PARAMETER_ENABLE
// #define CONFIG_RTKLIB_TLE_ENABLE
// #define CONFIG_RTKLIB_EARTH_TIDE_MODLE_ENABLE
// #define CONFIG_RTKLIB_SVR_EPH_CHECK1
// #define CONFIG_RTKLIB_RAW_FORMAT_ENABLE
// #define CONFIG_RTKLIB_STREAM_INNER_ENABLE
// #define CONFIG_RTKLIB_INNER_STREAM_SERIAL_ENABLE
// #define CONFIG_RTKLIB_INNER_STREAM_TCP_ENABLE
// #define CONFIG_RTKLIB_INNER_STREAM_NTRIP_ENABLE
// #define CONFIG_RTKLIB_INNER_STREAM_UDP_ENABLE
// #define CONFIG_RTKLIB_INNER_STREAM_MEMBUF_ENABLE
// #define CONFIG_RTKLIB_INNER_STREAM_FTP_ENABLE
// #define CONFIG_RTKLIB_DOWNLOADER_ENABLE
// #define CONFIG_RTKLIB_STREAM_SERVER_ENABLE
// #define CONFIG_RTKLIB_GIS_DATA_ENABLE
// #define CONFIG_RTKLIB_GOOGLE_EARTH_KML_CONVERTER_ENABLE
// #define CONFIG_RTKLIB_GPX_CONVERTER_ENABLE

#ifndef WIN32
#if defined(CONFIG_FREERTOS_ENABLE)
/**
 * Platform define
 */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#define CONFIG_RTK_TASK_STACK_SIZE        (30*1024)
#define CONFIG_RTK_TASK_PRIORITY          (16)

typedef struct {
    SemaphoreHandle_t sem;
} rtk_lock_t;

#define thread_t    void *
#define lock_t      rtk_lock_t
#define initlock(f) do {                                        \
    (f)->sem = xSemaphoreCreateMutex();                         \
    configASSERT((f)->sem != NULL);                             \
} while (0)
#define lock(f)     xSemaphoreTake((f)->sem, portMAX_DELAY)
#define unlock(f)   xSemaphoreGive((f)->sem)
#define FILEPATHSEP '\\'
#define platform_join(f,r)               vTaskDelete(f)
#define platform_create(p,m,func,param)  !xTaskCreate(func,"rtk",CONFIG_RTK_TASK_STACK_SIZE/4,\
                                        param,CONFIG_RTK_TASK_PRIORITY,(TaskHandle_t *)p)
#define PLATFORM_THREAD_RET            void
#else
#include <pthread.h>
#define thread_t    pthread_t
#define lock_t      pthread_mutex_t
#define initlock(f) pthread_mutex_init(f,NULL)
#define lock(f)     pthread_mutex_lock(f)
#define unlock(f)   pthread_mutex_unlock(f)
#define FILEPATHSEP '/'
#define platform_join(f,r)             pthread_join(f,NULL)
#define platform_create(p,func,param)  pthread_create(p,NULL,func,param)
#define PLATFORM_THREAD_RET            void*
#endif
#endif

#endif
