#ifndef PRV_PTHREAD_H_
#define PRV_PTHREAD_H_

#include <gpmx/config.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/time.h>
#include <gpm/fs/fs.h>

#if defined(CONFIG_FREERTOS_ENABLE)

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

typedef struct __pthread_obj
{
    pthread_attr_t attr;
    void *(*start)(void *);
    void *arg;
    void *ret;
    int pterrno;

    struct filelist tg_filelist;

    TaskHandle_t handle;             /**< FreeRTOS task handle. */
    StaticSemaphore_t join_barrier;  /**< Synchronizes the two callers of pthread_join. */
    StaticSemaphore_t join_mutex;    /**< Ensures that only one other thread may join this thread. */
} pthread_obj_t;
#elif defined(CONFIG_RTTNANO_ENABLE)
#include <rthw.h>
#include <rtthread.h>

#ifndef PTHREAD_NUM_MAX
#define PTHREAD_NUM_MAX 32
#endif

#define PTHREAD_MAGIC   0x70746873

typedef struct _pthread_cleanup {
    void (*cleanup_func)(void *parameter);
    void *parameter;

    struct _pthread_cleanup *next;
} _pthread_cleanup_t;

typedef struct _pthread_key_data {
    int is_used;
    void (*destructor)(void *parameter);
} _pthread_key_data_t;

typedef struct _pthread_data {
    rt_uint32_t magic;
    pthread_attr_t attr;
    rt_thread_t tid;

    void* (*thread_entry)(void *parameter);
    void *thread_parameter;

    /* return value */
    void *return_value;

    /* semaphore for joinable thread */
    rt_sem_t joinable_sem;

    /* cancel state and type */
    rt_uint8_t cancelstate;
    volatile rt_uint8_t canceltype;
    volatile rt_uint8_t canceled;

    _pthread_cleanup_t *cleanup;
    void** tls; /* thread-local storage area */
    struct filelist tg_filelist;
} _pthread_data_t;
#endif


#ifdef __cplusplus
extern "C" {
#endif

#if defined(CONFIG_FREERTOS_ENABLE)

void prv_exit_thread();
void prv_run_thread(void *xarg);
#elif defined(CONFIG_RTTNANO_ENABLE)

_pthread_data_t *_pthread_get_data(pthread_t thread);
pthread_t _pthread_data_get_pth(_pthread_data_t *ptd);
pthread_t _pthread_data_create(void);
void _pthread_data_destroy(_pthread_data_t *ptd);
void _pthread_cleanup(rt_thread_t tid);
void pthread_entry_stub(void *parameter);
#endif

#ifdef __cplusplus
}
#endif

#endif
