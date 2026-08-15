#include "./prv_mqueue.h"
#include <string.h>
#include <fcntl.h>
#include <gpmx/config.h>
#include "utils.h"

#if defined(CONFIG_FREERTOS_ENABLE)

/**< Maximum number of bytes in a filename (not including terminating null). */
#ifndef NAME_MAX
#define NAME_MAX                         64
#endif

static Link_t            queue_listhead  = {0};
static StaticSemaphore_t queue_listmutex = { {0}, .u = {0} };

void queuelist_lock()
{
    xSemaphoreTake((SemaphoreHandle_t)&queue_listmutex, portMAX_DELAY);
}

void queuelist_unlock()
{
    xSemaphoreGive((SemaphoreHandle_t)&queue_listmutex);
}

void init_queuelist()
{
    static BaseType_t queuelist_initialized = pdFALSE;
    Link_t *head = &queue_listhead;
    StaticSemaphore_t *mutex = &queue_listmutex;

    if (queuelist_initialized == pdFALSE) {
        taskENTER_CRITICAL();
        if (queuelist_initialized == pdFALSE) {
            (void)xSemaphoreCreateMutexStatic(mutex);
            listINIT_HEAD(head);
            queuelist_initialized = pdTRUE;
        }
        taskEXIT_CRITICAL();
    }
}

int find_queue_inlist(queuelist_element_t **p, const char *name, mqd_t descriptor)
{
    Link_t * queue_listlink = NULL;
    queuelist_element_t *msg_queue = NULL;
    int found = -1;
    Link_t *head = &queue_listhead;

    /* Iterate through the list of queues. */
    listFOR_EACH(queue_listlink, head) {
        msg_queue = listCONTAINER(queue_listlink, queuelist_element_t, link);
        if (name != NULL && strcmp(msg_queue->name, name) == 0) {
            found = 0;
            break;
        } else {
            if ((mqd_t)msg_queue == descriptor) {
                found = 0;
                break;
            }
        }
    }

    if (found == 0 && p != NULL){
        *p = msg_queue;
    }

    return found;
}

int create_new_messagequeue(queuelist_element_t **p,
    const struct mq_attr *pattr, const char *name, size_t len)
{
    int ret = 0;
    Link_t *head = &queue_listhead;

    *p = pvPortMalloc(sizeof(queuelist_element_t));
    if (*p == NULL) {
        ret = -1;
    }

    if (ret == 0) {
        (*p)->queue = xQueueCreate(pattr->mq_maxmsg, sizeof(queue_element_t));
        if ((*p)->queue == NULL) {
            vPortFree(*p);
            ret = -1;
        }
    }

    if (ret == 0) {
        (*p)->name = pvPortMalloc(len + 1);
        if ((*p)->name == NULL) {
            vQueueDelete((*p)->queue);
            vPortFree(*p);
            ret = -1;
        } else {
            (void)strncpy((*p)->name, name, len + 1);
        }
    }

    if (ret == 0) {
        (*p)->attr = *pattr;
        (*p)->open_descriptors = 1;
        (*p)->pending_unlink = pdFALSE;
        listADD(head, &(*p)->link);
    }

    return ret;
}

void delete_messagequeue(const queuelist_element_t *p)
{
    queue_element_t element = {0};
    while (xQueueReceive(p->queue, (void *)&element, 0) == pdTRUE) {
        vPortFree(element.p);
    }

    vQueueDelete(p->queue);
    vPortFree((void *)p->name);
    vPortFree((void *)p);
}

int cal_ticktimeout(long flag, const struct timespec *ptimeout, TickType_t *ptimeout_ticks)
{
    int ret = 0;

    if (flag & O_NONBLOCK) {
        *ptimeout_ticks = 0;
    } else {
        if (ptimeout == NULL) {
            *ptimeout_ticks = portMAX_DELAY;
        } else {
            struct timespec cur = {0};
            if (utils_validtimespec(ptimeout) == false) {
                ret = EINVAL;
            }
            if (ret == 0 && clock_gettime(CLOCK_REALTIME, &cur) != 0) {
                ret = EINVAL;
            }
            if ( ret == 0 && utils_timespec_todeltaticks(ptimeout, &cur, ptimeout_ticks) != 0) {
                ret = ETIMEDOUT;
            }
        }
    }

    return ret;
}

int validate_queuename(const char *name, size_t *len)
{
    int ret = 0;
    size_t name_len = 0;

    /* All message queue names must start with '/'. */
    if (name[0] != '/') {
        ret = -1;
    } else {
        /* Get the length of pcName, excluding the first '/' and null-terminator. */
        name_len = utils_strlen(name, NAME_MAX + 2);
        if (name_len == NAME_MAX + 2) {
            ret = -1;
        } else {
            *len = name_len;
        }
    }

    return ret;
}

#endif
