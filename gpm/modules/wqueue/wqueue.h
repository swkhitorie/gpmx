#ifndef __WORK_QUEUE_H_
#define __WORK_QUEUE_H_

#include <gpmx/config.h>

#include <stdint.h>
#include <pthread.h>
#include <queue.h>
#include <sys/types.h>

#define HPWORK 0
#define LPWORK 1
#define NWORKERS 2

#define CONFIG_SCHED_WORKQUEUE 1
#define CONFIG_SCHED_HPWORK 1
#define CONFIG_SCHED_LPWORK 1

/** time in ms between checks for work in work queues **/
#define CONFIG_SCHED_WORKPERIOD 50000

#if defined(CONFIG_POSIXRUN_ENABLE)
#define USEC_PER_TICK (1000000/SYS_TICKS_PER_SEC)
#endif

#define USEC2TICK(x) (((x)+(USEC_PER_TICK/2))/USEC_PER_TICK)

struct wqueue_s {
	pthread_t         pid; /* The task ID of the worker thread */
	struct dq_queue_s q;   /* The queue of pending work */
};

typedef void (*worker_t)(void *arg);

struct work_s {
	struct dq_entry_s dq;  /* Implements a doubly linked list */
	worker_t  worker;      /* Work callback */
	void *arg;             /* Callback argument */
	uint64_t  qtime;       /* Time work queued */
	uint32_t  delay;       /* Delay until work performed */
};

#ifdef __cplusplus
extern "C" {
#endif

extern long SYS_TICKS_PER_SEC;

extern struct wqueue_s g_work[NWORKERS];

void work_queues_init(void);

int work_queue(int qid, struct work_s *work, worker_t worker, void *arg, uint32_t delay);

int work_cancel(int qid, struct work_s *work);

uint32_t clock_systimer(void);

void *work_hpthread(void* arg);
void *work_lpthread(void* arg);

#ifdef __cplusplus
}
#endif

#endif

