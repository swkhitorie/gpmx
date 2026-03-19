#include <arch/sys_arch.h>
#include <lwip/sys.h>
#include <lwip/opt.h>
#include <lwip/stats.h>
#include <lwip/err.h>
#include <lwip/debug.h>
#include <lwip/netif.h>
#include <lwip/netifapi.h>
#include <lwip/tcpip.h>
#include <lwip/sio.h>
#include <lwip/init.h>
#include <lwip/dhcp.h>
#include <lwip/inet.h>
#include <netif/ethernetif.h>
#include <netif/etharp.h>

#include <stdio.h>
#include <string.h>

#if defined(CONFIG_RTTNANO_ENABLE)
#include <rtthread.h>
#include <rthw.h>
#elif defined(CONFIG_FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <task.h>
#include <portmacro.h>
#else
#include <board_config.h>
#endif

void sys_init(void)
{
    // do nothing
}

/****************************************************************************
 * Semaphore
 ****************************************************************************/

err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    *sem = xSemaphoreCreateCounting(UINT16_MAX, count);
    return (*sem != NULL) ? ERR_OK : ERR_MEM;
#elif defined(CONFIG_RTTNANO_ENABLE)

    static unsigned short counter = 0;
    char tname[RT_NAME_MAX];
    sys_sem_t tmpsem;

    RT_DEBUG_NOT_IN_INTERRUPT;

    rt_snprintf(tname, RT_NAME_MAX, "%s%d", SYS_LWIP_SEM_NAME, counter);
    counter++;

    tmpsem = rt_sem_create(tname, count, RT_IPC_FLAG_FIFO);
    if (tmpsem == RT_NULL) {
        return ERR_MEM;
    } else {
        *sem = tmpsem;
        return ERR_OK;
    }
#endif
}

void sys_sem_free(sys_sem_t *sem)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    if (sem && *sem) {
        vSemaphoreDelete(*sem);
        *sem = NULL;
    }
#elif defined(CONFIG_RTTNANO_ENABLE)

    RT_DEBUG_NOT_IN_INTERRUPT;
    rt_sem_delete(*sem);
#endif
}

void sys_sem_signal(sys_sem_t *sem)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    BaseType_t higher_woken = pdFALSE;
    if (sem && *sem) {
        xSemaphoreGive(*sem);
    }
#elif defined(CONFIG_RTTNANO_ENABLE)

    rt_sem_release(*sem);
#endif
}

u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    TickType_t start = xTaskGetTickCount();
    TickType_t wait_ticks = (timeout == 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout);
    BaseType_t ret;

    if (!sem || !*sem) {
        return SYS_ARCH_TIMEOUT;
    }

    ret = xSemaphoreTake(*sem, wait_ticks);

    if (ret == pdTRUE) {
        return (xTaskGetTickCount() - start) * portTICK_PERIOD_MS;
    } else {
        return SYS_ARCH_TIMEOUT;
    }
#elif defined(CONFIG_RTTNANO_ENABLE)

    rt_err_t ret;
    s32_t t;
    u32_t tick;

    RT_DEBUG_NOT_IN_INTERRUPT;

    /* get the begin tick */
    tick = rt_tick_get();
    if (timeout == 0) {
        t = RT_WAITING_FOREVER;
    } else {
        /* convert msecond to os tick */
        if (timeout < (1000 / RT_TICK_PER_SECOND)) {
            t = 1;
        } else {
            t = timeout / (1000 / RT_TICK_PER_SECOND);
        }
    }

    ret = rt_sem_take(*sem, t);

    if (ret == -RT_ETIMEOUT) {
        return SYS_ARCH_TIMEOUT;
    } else {
        if (ret == RT_EOK) {
            ret = 1;
        }
    }

    /* get elapse msecond */
    tick = rt_tick_get() - tick;

    /* convert tick to msecond */
    tick = tick * (1000 / RT_TICK_PER_SECOND);
    if (tick == 0) {
        tick = 1;
    }

    return tick;
#endif
}

#ifndef sys_sem_valid
/** Check if a semaphore is valid/allocated:
 *  return 1 for valid, 0 for invalid
 */
int sys_sem_valid(sys_sem_t *sem)
{
    int ret = 0;
    if (*sem) ret = 1;
    return ret;
}
#endif

#ifndef sys_sem_set_invalid
/** Set a semaphore invalid so that sys_sem_valid returns 0
 */
void sys_sem_set_invalid(sys_sem_t *sem)
{
    *sem = ((void *)0);
}
#endif

/****************************************************************************
 * Mutex
 ****************************************************************************/

err_t sys_mutex_new(sys_mutex_t *mutex)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    *mutex = xSemaphoreCreateRecursiveMutex();
    return (*mutex != NULL) ? ERR_OK : ERR_MEM;
#elif defined(CONFIG_RTTNANO_ENABLE)

    static unsigned short counter = 0;
    char tname[RT_NAME_MAX];
    sys_mutex_t tmpmutex;

    RT_DEBUG_NOT_IN_INTERRUPT;

    rt_snprintf(tname, RT_NAME_MAX, "%s%d", SYS_LWIP_MUTEX_NAME, counter);
    counter++;

    tmpmutex = rt_mutex_create(tname, RT_IPC_FLAG_PRIO);
    if (tmpmutex == RT_NULL) {
        return ERR_MEM;
    } else {
        *mutex = tmpmutex;
        return ERR_OK;
    }
#endif
}

void sys_mutex_free(sys_mutex_t *mutex)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    if (mutex && *mutex) {
        vSemaphoreDelete(*mutex);
        *mutex = NULL;
    }
#elif defined(CONFIG_RTTNANO_ENABLE)

    RT_DEBUG_NOT_IN_INTERRUPT;
    rt_mutex_delete(*mutex);
#endif
}

void sys_mutex_lock(sys_mutex_t *mutex)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    if (mutex && *mutex) {
        xSemaphoreTakeRecursive(*mutex, portMAX_DELAY);
    }
#elif defined(CONFIG_RTTNANO_ENABLE)

    RT_DEBUG_NOT_IN_INTERRUPT;
    rt_mutex_take(*mutex, RT_WAITING_FOREVER);
#endif
}

void sys_mutex_unlock(sys_mutex_t *mutex)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    if (mutex && *mutex) {
        xSemaphoreGiveRecursive(*mutex);
    }
#elif defined(CONFIG_RTTNANO_ENABLE)

    rt_mutex_release(*mutex);
#endif
}

#ifndef sys_mutex_valid
/** Check if a mutex is valid/allocated:
 *  return 1 for valid, 0 for invalid
 */
int sys_mutex_valid(sys_mutex_t *mutex)
{
    int ret = 0;

    if (*mutex) ret = 1;

    return ret;
}
#endif

#ifndef sys_mutex_set_invalid
/** Set a mutex invalid so that sys_mutex_valid returns 0
 */
void sys_mutex_set_invalid(sys_mutex_t *mutex)
{
    *mutex = ((void *)0);
}
#endif

/****************************************************************************
 * Mailbox
 ****************************************************************************/

err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    *mbox = xQueueCreate(size, sizeof(void*));
    return (*mbox != NULL) ? ERR_OK : ERR_MEM;
#elif defined(CONFIG_RTTNANO_ENABLE)

    static unsigned short counter = 0;
    char tname[RT_NAME_MAX];
    sys_mbox_t tmpmbox;

    RT_DEBUG_NOT_IN_INTERRUPT;

    rt_snprintf(tname, RT_NAME_MAX, "%s%d", SYS_LWIP_MBOX_NAME, counter);
    counter++;

    tmpmbox = rt_mb_create(tname, size, RT_IPC_FLAG_FIFO);
    if (tmpmbox != RT_NULL) {
        *mbox = tmpmbox;
        return ERR_OK;
    }

    return ERR_MEM;
#endif
}

void sys_mbox_free(sys_mbox_t *mbox)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    if (mbox && *mbox) {
        vQueueDelete(*mbox);
        *mbox = NULL;
    }
#elif defined(CONFIG_RTTNANO_ENABLE)

    RT_DEBUG_NOT_IN_INTERRUPT;
    rt_mb_delete(*mbox);
#endif
}

void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    if (mbox && *mbox) {
        xQueueSendToBack(*mbox, &msg, portMAX_DELAY);
    }
#elif defined(CONFIG_RTTNANO_ENABLE)

    RT_DEBUG_NOT_IN_INTERRUPT;
    rt_mb_send_wait(*mbox, (rt_ubase_t)msg, RT_WAITING_FOREVER);
#endif
}

err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    BaseType_t ret;

    if (!mbox || !*mbox) {
        return ERR_MEM;
    }

    ret = xQueueSendToBack(*mbox, &msg, 0);
    return (ret == pdTRUE) ? ERR_OK : ERR_MEM;
#elif defined(CONFIG_RTTNANO_ENABLE)

    if (rt_mb_send(*mbox, (rt_ubase_t)msg) == RT_EOK) {
        return ERR_OK;
    }

    return ERR_MEM;
#endif
}

#if (LWIP_VERSION_MAJOR * 100 + LWIP_VERSION_MINOR) >= 201 /* >= v2.1.0 */
err_t sys_mbox_trypost_fromisr(sys_mbox_t *q, void *msg)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    BaseType_t higher_woken = pdFALSE;
    BaseType_t ret = xQueueSendToBackFromISR(*q, &msg, &higher_woken);
    return (ret == pdTRUE) ? ERR_OK : ERR_MEM;
#elif defined(CONFIG_RTTNANO_ENABLE)

    return sys_mbox_trypost(q, msg);
#endif
}
#endif

u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    TickType_t start = xTaskGetTickCount();
    TickType_t wait_ticks = (timeout == 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout);
    BaseType_t ret;

    if (!mbox || !*mbox || !msg) {
        return SYS_ARCH_TIMEOUT;
    }

    ret = xQueueReceive(*mbox, msg, wait_ticks);

    if (ret == pdTRUE) {
        return (xTaskGetTickCount() - start) * portTICK_PERIOD_MS;
    } else {
        return SYS_ARCH_TIMEOUT;
    }
#elif defined(CONFIG_RTTNANO_ENABLE)

    rt_err_t ret;
    s32_t t;
    u32_t tick;

    RT_DEBUG_NOT_IN_INTERRUPT;

    tick = rt_tick_get();

    if(timeout == 0) {
        t = RT_WAITING_FOREVER;
    } else {
        /* convirt msecond to os tick */
        if (timeout < (1000 / RT_TICK_PER_SECOND)) {
            t = 1;
        } else {
            t = timeout / (1000 / RT_TICK_PER_SECOND);
        }
    }

    /*When the waiting msg is generated by the application through signaling mechanisms,
    only by using interruptible mode can the program be made runnable again*/
    ret = rt_mb_recv_interruptible(*mbox, (rt_ubase_t *)msg, t);
    if(ret != RT_EOK) {
        return SYS_ARCH_TIMEOUT;
    }

    tick = rt_tick_get() - tick;

    tick = tick * (1000 / RT_TICK_PER_SECOND);
    if (tick == 0) {
        tick = 1;
    }

    return tick;
#endif
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    BaseType_t ret;

    if (!mbox || !*mbox || !msg) {
        return SYS_ARCH_TIMEOUT;
    }

    ret = xQueueReceive(*mbox, msg, 0);
    return (ret == pdTRUE) ? 0 : SYS_MBOX_EMPTY;
#elif defined(CONFIG_RTTNANO_ENABLE)

    int ret;

    ret = rt_mb_recv(*mbox, (rt_ubase_t *)msg, 0);
    if(ret == -RT_ETIMEOUT) {
        return SYS_ARCH_TIMEOUT;
    } else {
        if (ret == RT_EOK) {
            ret = 0;
        }
    }

    return ret;
#endif
}

#ifndef sys_mbox_valid
/** Check if an mbox is valid/allocated:
 *  return 1 for valid, 0 for invalid
 */
int sys_mbox_valid(sys_mbox_t *mbox)
{
    int ret = 0;
    if (*mbox) ret = 1;
    return ret;
}
#endif

#ifndef sys_mbox_set_invalid
/** Set an mbox invalid so that sys_mbox_valid returns 0
 */
void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
    *mbox = ((void *)0);
}
#endif

/****************************************************************************
 * System
 ****************************************************************************/

sys_thread_t sys_thread_new(const char    *name,
                            lwip_thread_fn thread,
                            void          *arg,
                            int            stacksize,
                            int            prio)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    TaskHandle_t created_task = NULL;
    stacksize /= sizeof(StackType_t);

    if (xTaskCreate(thread, name, stacksize, arg, prio, &created_task) != pdPASS) {
        created_task = NULL;
    }

    return created_task;
#elif defined(CONFIG_RTTNANO_ENABLE)

    rt_thread_t t;

    RT_DEBUG_NOT_IN_INTERRUPT;

    t = rt_thread_create(name, thread, arg, stacksize, prio, 20);
    RT_ASSERT(t != RT_NULL);

    rt_thread_startup(t);
    return t;
#endif
}

sys_prot_t sys_arch_protect(void)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    return (sys_prot_t)ulPortRaiseBASEPRI();
#elif defined(CONFIG_RTTNANO_ENABLE)

    return (sys_prot_t)rt_hw_interrupt_disable();
#endif
}

void sys_arch_unprotect(sys_prot_t pval)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    vPortSetBASEPRI(pval);
#elif defined(CONFIG_RTTNANO_ENABLE)

    rt_hw_interrupt_enable((rt_base_t)pval);
#endif
}

void sys_arch_assert(const char *file, int line)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    board_printf("\nAssertion: %d in %s, thread \n",
                line, file);
#elif defined(CONFIG_RTTNANO_ENABLE)

    board_printf("\nAssertion: %d in %s, thread %s\n",
                line, file, rt_thread_self()->parent.name);

    RT_ASSERT(0);
#endif
}

u32_t sys_jiffies(void)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    return xTaskGetTickCount();
#elif defined(CONFIG_RTTNANO_ENABLE)

    return rt_tick_get();
#endif
}

u32_t sys_now(void)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    return (xTaskGetTickCount() * portTICK_RATE_MS);
#elif defined(CONFIG_RTTNANO_ENABLE)

    return rt_tick_get_millisecond();
#else

    return board_get_time();
#endif
}

void mem_init(void)
{

}

void *mem_calloc(mem_size_t count, mem_size_t size)
{
    void *ptr;
    mem_size_t total = count * size;

#if defined(CONFIG_FREERTOS_ENABLE)

    ptr = pvPortMalloc(total);
    if (ptr != NULL) {
        memset(ptr, 0, total);
    }
#elif defined(CONFIG_RTTNANO_ENABLE)

    ptr = rt_calloc(count, size);
#endif
    return ptr;
}

void *mem_trim(void *mem, mem_size_t size)
{
    // return rt_realloc(mem, size);
    /* not support trim yet */
    return mem;
}

void *mem_malloc(mem_size_t size)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    return pvPortMalloc(size);
#elif defined(CONFIG_RTTNANO_ENABLE)

    return rt_malloc(size);
#endif
}

void  mem_free(void *mem)
{
#if defined(CONFIG_FREERTOS_ENABLE)

    vPortFree(mem);
#elif defined(CONFIG_RTTNANO_ENABLE)

    rt_free(mem);
#endif
}

#if LWIP_VERSION_MAJOR >= 2 /* >= v2.x */
#if MEM_OVERFLOW_CHECK || MEMP_OVERFLOW_CHECK

/**
 * Check if a mep element was victim of an overflow or underflow
 * (e.g. the restricted area after/before it has been altered)
 *
 * @param p the mem element to check
 * @param size allocated size of the element
 * @param descr1 description of the element source shown on error
 * @param descr2 description of the element source shown on error
 */
void mem_overflow_check_raw(void *p, size_t size, const char *descr1, const char *descr2)
{
#if MEM_SANITY_REGION_AFTER_ALIGNED || MEM_SANITY_REGION_BEFORE_ALIGNED
    u16_t k;
    u8_t *m;

#if MEM_SANITY_REGION_AFTER_ALIGNED > 0
    m = (u8_t *)p + size;
    for (k = 0; k < MEM_SANITY_REGION_AFTER_ALIGNED; k++) {
        if (m[k] != 0xcd) {
            char errstr[128];
            snprintf(errstr, sizeof(errstr), "detected mem overflow in %s%s", descr1, descr2);
            LWIP_ASSERT(errstr, 0);
        }
    }
#endif /* MEM_SANITY_REGION_AFTER_ALIGNED > 0 */

#if MEM_SANITY_REGION_BEFORE_ALIGNED > 0
    m = (u8_t *)p - MEM_SANITY_REGION_BEFORE_ALIGNED;
    for (k = 0; k < MEM_SANITY_REGION_BEFORE_ALIGNED; k++) {
        if (m[k] != 0xcd) {
            char errstr[128];
            snprintf(errstr, sizeof(errstr), "detected mem underflow in %s%s", descr1, descr2);
            LWIP_ASSERT(errstr, 0);
        }
    }
#endif
#else

    LWIP_UNUSED_ARG(p);
    LWIP_UNUSED_ARG(descr1);
    LWIP_UNUSED_ARG(descr2);
#endif
}

/**
 * Initialize the restricted area of a mem element.
 */
void mem_overflow_init_raw(void *p, size_t size)
{
#if MEM_SANITY_REGION_BEFORE_ALIGNED > 0 || MEM_SANITY_REGION_AFTER_ALIGNED > 0
    u8_t *m;

#if MEM_SANITY_REGION_BEFORE_ALIGNED > 0
    m = (u8_t *)p - MEM_SANITY_REGION_BEFORE_ALIGNED;
    memset(m, 0xcd, MEM_SANITY_REGION_BEFORE_ALIGNED);
#endif

#if MEM_SANITY_REGION_AFTER_ALIGNED > 0
    m = (u8_t *)p + size;
    memset(m, 0xcd, MEM_SANITY_REGION_AFTER_ALIGNED);
#endif

#else

    LWIP_UNUSED_ARG(p);
    LWIP_UNUSED_ARG(size);
#endif
}
#endif /* MEM_OVERFLOW_CHECK || MEMP_OVERFLOW_CHECK */
#endif /*LWIP_VERSION_MAJOR >= 2 */
