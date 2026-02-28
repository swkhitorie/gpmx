#include "workqueue_manager.h"
#include "workqueue.h"

#include "container/blocking_list.h"
#include "container/blocking_queue.h"
#include "container/atomic.h"

#include <string.h>

#include <pthread.h>
#include <unistd.h>

#include "kmodule_defines.h"

const struct wq_config_t wq_config_rate_ctrl = {"wq:rate_ctrl", 1664, 0};

const struct wq_config_t wq_config_spi0 = {"wq:SPI0", 2336, -1};
const struct wq_config_t wq_config_spi1 = {"wq:SPI1", 2336, -2};
const struct wq_config_t wq_config_spi2 = {"wq:SPI2", 2336, -3};
const struct wq_config_t wq_config_spi3 = {"wq:SPI3", 2336, -4};
const struct wq_config_t wq_config_spi4 = {"wq:SPI4", 2336, -5};
const struct wq_config_t wq_config_spi5 = {"wq:SPI5", 2336, -6};
const struct wq_config_t wq_config_spi6 = {"wq:SPI6", 2336, -7};

const struct wq_config_t wq_config_i2c0 = {"wq:I2C0", 1472, -8};
const struct wq_config_t wq_config_i2c1 = {"wq:I2C1", 1472, -9};
const struct wq_config_t wq_config_i2c2 = {"wq:I2C2", 1472, -10};
const struct wq_config_t wq_config_i2c3 = {"wq:I2C3", 1472, -11};
const struct wq_config_t wq_config_i2c4 = {"wq:I2C4", 1472, -12};

const struct wq_config_t wq_config_attitude_ctrl = {"wq:attitude_ctrl", 1672, -13};
const struct wq_config_t wq_config_nav_and_controllers = {"wq:nav_and_controllers", 7200, -14};

const struct wq_config_t wq_config_hp_default = {"wq:hp_default", 1900, -15};

const struct wq_config_t wq_config_uart0 = {"wq:UART0", 1400, -17};
const struct wq_config_t wq_config_uart1 = {"wq:UART1", 1400, -18};
const struct wq_config_t wq_config_uart2 = {"wq:UART2", 1400, -19};
const struct wq_config_t wq_config_uart3 = {"wq:UART3", 1400, -20};
const struct wq_config_t wq_config_uart4 = {"wq:UART4", 1400, -21};
const struct wq_config_t wq_config_uart5 = {"wq:UART5", 1400, -22};
const struct wq_config_t wq_config_uart6 = {"wq:UART6", 1400, -23};
const struct wq_config_t wq_config_uart7 = {"wq:UART7", 1400, -24};
const struct wq_config_t wq_config_uart8 = {"wq:UART8", 1400, -25};
const struct wq_config_t wq_config_uart_unknown = {"wq:UART_UNKNOWN", 1400, -26};

const struct wq_config_t wq_config_lp_default = {"wq:lp_default", 1700, -50};

const struct wq_config_t wq_config_test1 = {"wq:test1", 2000, 0};
const struct wq_config_t wq_config_test2 = {"wq:test2", 2000, 0};

DEFINE_BLOCKING_QUEUE(const struct wq_config_t *, 1, wq_config_queue_t)

// queue of WorkQueues to be created (as threads in the wq manager task)
static wq_config_queue_t *_wq_manager_create_queue;

// list of current work queues
static struct blocking_list *_wq_manager_wqs_list;

static atomic_bool_t _wq_manager_should_exit = { .val = true, };

static struct __workqueue *find_workqueue_by_name(const char *name)
{
    struct intrusive_node *iterator = NULL;
    struct __workqueue *queue = NULL;

    if (_wq_manager_wqs_list == NULL) {
        KMERROR("not running\r\n");
        return NULL;
    }

    pthread_mutex_lock(&_wq_manager_wqs_list->_mutex);

    // search list
    for (iterator = _wq_manager_wqs_list->list.head; iterator != NULL; iterator = iterator->next) {
        queue = container_of(iterator, struct __workqueue, node);

        if (strcmp(workqueue_getname(queue), name) == 0) {
            pthread_mutex_unlock(&_wq_manager_wqs_list->_mutex);
            return queue;
        }
    }

    pthread_mutex_unlock(&_wq_manager_wqs_list->_mutex);

    return NULL;
}

struct __workqueue *workqueue_find_or_create(const struct wq_config_t *new_wq)
{
    uint64_t t = 0;
    struct __workqueue *wq = NULL;

    if (_wq_manager_create_queue == NULL) {
        KMERROR("not running\r\n");
        return NULL;
    }

    // search list for existing work queue
    wq = find_workqueue_by_name(new_wq->name);

    // create work queue if it doesn't already exist
    if (wq == NULL) {
        // add WQ config to list
        //  main thread wakes up, creates the thread
        wq_config_queue_t_push(_wq_manager_create_queue, new_wq);
        // _wq_manager_create_queue->push(&new_wq);

        // we wait until new wq is created, then return
        t = 0;

        while (wq == NULL && t < 10*1000*1000) {
            // Wait up to 10 seconds, checking every 1 ms
            t += 1*1000;
            usleep(1000);

            wq = find_workqueue_by_name(new_wq->name);
        }

        if (wq == NULL) {
            KMERROR("failed to create %s\r\n", new_wq->name);
        }
    }

    return wq;
}

static void *workqueue_runner(void *context)
{
    struct wq_config_t *config = (struct wq_config_t *)(context);
    struct __workqueue wq;

    workqueue_init(&wq, config);

    // add to work queue list
    blocking_list_add(_wq_manager_wqs_list, &wq.node);

    workqueue_run(&wq);

    // remove from work queue list
    blocking_list_remove(_wq_manager_wqs_list, &wq.node);

    workqueue_deinit(&wq);

    return NULL;
}

static void* workqueue_manager_run(void *p)
{
    pthread_setname_np(pthread_self(), "wq:manager");

	_wq_manager_wqs_list = pvPortMalloc(sizeof(struct blocking_list));
	_wq_manager_create_queue = pvPortMalloc(sizeof(wq_config_queue_t));

    blocking_list_init(_wq_manager_wqs_list, workqueue_cmp_method);
    wq_config_queue_t_init(_wq_manager_create_queue);

	while (!atomic_bool_load(&_wq_manager_should_exit)) {
		// create new work queues as needed
		const struct wq_config_t *wq = wq_config_queue_t_pop(_wq_manager_create_queue);

		if (wq != NULL) {
			// create new work queue
			pthread_attr_t attr;
			int ret_attr_init = pthread_attr_init(&attr);

			if (ret_attr_init != 0) {
				KMERROR("attr init for %s failed (%i)\r\n", wq->name, ret_attr_init);
			}

			struct sched_param param;
			int ret_getschedparam = pthread_attr_getschedparam(&attr, &param);

			if (ret_getschedparam != 0) {
				KMERROR("getting sched param for %s failed (%i)\r\n", wq->name, ret_getschedparam);
			}

			// stack size
			const size_t stacksize = ((uint16_t)PTHREAD_STACK_MIN > wq->stacksize) ? (uint16_t)PTHREAD_STACK_MIN : wq->stacksize;
			int ret_setstacksize = pthread_attr_setstacksize(&attr, stacksize);

			if (ret_setstacksize != 0) {
				KMERROR("setting stack size for %s failed (%i)\r\n", wq->name, ret_setstacksize);
			}

			// schedule policy FIFO
			int ret_setschedpolicy = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

			if (ret_setschedpolicy != 0) {
				KMERROR("failed to set sched policy SCHED_FIFO (%i)\r\n", ret_setschedpolicy);
			}

			// priority
			param.sched_priority = sched_get_priority_max(SCHED_FIFO) + wq->relative_priority;
			int ret_setschedparam = pthread_attr_setschedparam(&attr, &param);

			if (ret_setschedparam != 0) {
				KMERROR("setting sched params for %s failed (%i)\r\n", wq->name, ret_setschedparam);
			}

			// create thread
			pthread_t thread;
			int ret_create = pthread_create(&thread, &attr, workqueue_runner, (void *)wq);

			if (ret_create == 0) {
				KMDEBUG("starting: %s, priority: %d, stack: %zu bytes\r\n", wq->name, param.sched_priority, stacksize);

			} else {
				KMERROR("failed to create thread for %s (%i): %s\r\n", wq->name, ret_create, strerror(ret_create));
			}

			// destroy thread attributes
			int ret_destroy = pthread_attr_destroy(&attr);

			if (ret_destroy != 0) {
				KMERROR("failed to destroy thread attributes for %s (%i)\r\n", wq->name, ret_create);
			}
		}
	}

}

int workqueue_manager_start()
{
    int task_id;
    pthread_attr_t attr;
    pthread_t id;

    pthread_attr_init(&attr);
    attr.schedparam.sched_priority = sched_get_priority_max(0);
    attr.schedpolicy = PTHREAD_CREATE_JOINABLE;
    attr.stacksize = 1280 * sizeof(StackType_t);

    if (atomic_bool_load(&_wq_manager_should_exit) && (_wq_manager_create_queue == NULL)) {

        atomic_bool_store(&_wq_manager_should_exit, false);

        task_id = pthread_create(&id, &attr, &workqueue_manager_run, NULL);

        if (task_id != 0) {
            atomic_bool_store(&_wq_manager_should_exit, true);
            KMERROR("task start failed (%i)\r\n", task_id);
            return -1;
        }

    } else {
        KMWARN("already running\r\n");
        return -2;
    }

    return 0;
}

int workqueue_manager_stop()
{
    struct intrusive_node *iterator = NULL;
    struct __workqueue *wq = NULL;

    if (!atomic_bool_load(&_wq_manager_should_exit)) {

        // error can't shutdown until all WorkItems are removed/stopped
        if ((_wq_manager_wqs_list != NULL) && (blocking_list_size(_wq_manager_wqs_list) > 0)) {
            KMERROR("can't shutdown with active WQs\r\n");
            workqueue_manager_status();
            return -1;
        }

        // first ask all WQs to stop
        if (_wq_manager_wqs_list != NULL) {

            pthread_mutex_lock(&_wq_manager_wqs_list->_mutex);

            // ask all work queues (threads) to stop
            // NOTE: not currently safe without all WorkItems stopping first
            for (iterator = _wq_manager_wqs_list->list.head; iterator != NULL; iterator = iterator->next) {
                wq = container_of(iterator, struct __workqueue, node);

                workqueue_request_stop(wq);
            }

            pthread_mutex_unlock(&_wq_manager_wqs_list->_mutex);

            // wait until they're all stopped (empty list)
            while (blocking_list_size(_wq_manager_wqs_list) > 0) {
                usleep(1000);
            }

            blocking_list_deinit(_wq_manager_wqs_list);
            vPortFree(_wq_manager_wqs_list);
        }

        atomic_bool_store(&_wq_manager_should_exit, true);

        if (_wq_manager_create_queue != NULL) {
            // push nullptr to wake the wq manager task
            wq_config_queue_t_push(_wq_manager_create_queue, NULL);

            usleep(10000);

            wq_config_queue_t_destroy(_wq_manager_create_queue);
            vPortFree(_wq_manager_create_queue);
        }

    } else {
        KMWARN("manager not running\r\n");
        return -2;
    }

    return 0;
}

int workqueue_manager_status()
{
    struct intrusive_node *iterator = NULL;
    struct __workqueue *wq = NULL;

    if (!atomic_bool_load(&_wq_manager_should_exit) && (_wq_manager_wqs_list != NULL)) {

        const size_t num_wqs = blocking_list_size(_wq_manager_wqs_list);

        KMINFO("\nWork Queue: %-1zu threads                        RATE        INTERVAL\n", num_wqs);

        pthread_mutex_lock(&_wq_manager_wqs_list->_mutex);

        size_t i = 0;

        for (iterator = _wq_manager_wqs_list->list.head; iterator != NULL; iterator = iterator->next) {
            wq = container_of(iterator, struct __workqueue, node);
            i++;

            const bool last_wq = (i >= num_wqs);

            if (!last_wq) {
                KMINFO("|__ %zu) ", i);

            } else {
                KMINFO("\\__ %zu) ", i);
            }

            workqueue_print_status(wq, last_wq);
        }

        pthread_mutex_unlock(&_wq_manager_wqs_list->_mutex);
    } else {
        KMERROR("manager not running\r\n");
    }

    return 0;
}
