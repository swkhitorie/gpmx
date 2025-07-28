#include "./prv_mqueue.h"

/**< Maximum number of bytes in a filename (not including terminating null). */
#define NAME_MAX                         64

static StaticSemaphore_t queue_listmutex = { { 0 }, .u = { 0 } };

static Link_t queue_listhead = { 0 };

StaticSemaphore_t *get_queue_listmutex()
{
    return &queue_listmutex;
}

Link_t *get_queue_listhead()
{
    return &queue_listhead;
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

BaseType_t create_new_messagequeue(queuelist_element_t **p,
    const struct mq_attr *pattr, const char *name, size_t len)
{
    BaseType_t ret = pdTRUE;
    Link_t *head = get_queue_listhead();

    *p = pvPortMalloc(sizeof(queuelist_element_t));
    if (*p == NULL) {
        ret = pdFALSE;
    }

    if (ret == pdTRUE) {
        (*p)->queue = xQueueCreate(pattr->mq_maxmsg, sizeof(queue_element_t));
        if ((*p)->queue == NULL) {
            vPortFree(*p);
            ret = pdFALSE;
        }
    }

    if (ret == pdTRUE) {
        (*p)->name = pvPortMalloc(len + 1);
        if ((*p)->name == NULL) {
            vQueueDelete((*p)->queue);
            vPortFree(*p);
            ret = pdFALSE;
        } else {
            (void)strncpy((*p)->name, name, len + 1);
        }
    }

    if (ret == pdTRUE) {
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

BaseType_t find_queue_inlist(queuelist_element_t **p, const char *name, mqd_t descriptor)
{
    Link_t * queue_listlink = NULL;
    queuelist_element_t *msg_queue = NULL;
    BaseType_t found = pdFALSE;
    Link_t *head = get_queue_listhead();

    /* Iterate through the list of queues. */
    listFOR_EACH(queue_listlink, head) {
        msg_queue = listCONTAINER(queue_listlink, queuelist_element_t, link);

        if ( name != NULL && strcmp(msg_queue->name, name) == 0) {
            found = pdTRUE;
            break;
        } else {
            if ((mqd_t)msg_queue == descriptor) {
                found = pdTRUE;
                break;
            }
        }
    }

    if (found == pdTRUE && p != NULL){
        *p = msg_queue;
    }
    return found;
}

void init_queuelist( void )
{
    static BaseType_t queuelist_initialized = pdFALSE;
    Link_t *head = get_queue_listhead();
    StaticSemaphore_t *mutex = get_queue_listmutex();
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

BaseType_t validate_queuename(const char *name, size_t *len)
{
    BaseType_t ret = pdTRUE;
    size_t name_len = 0;

    /* All message queue names must start with '/'. */
    if (name[0] != '/') {
        ret = pdFALSE;
    } else {
        /* Get the length of pcName, excluding the first '/' and null-terminator. */
        name_len = utils_strlen(name, NAME_MAX + 2);
        if (name_len == NAME_MAX + 2) {
            ret = pdFALSE;
        } else {
            *len = name_len;
        }
    }
    return ret;
}
