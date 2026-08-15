
#include <gpmx/config.h>
#include <stdint.h>
#include <stdio.h>
#include <queue.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>
#include "work_lock.h"
#include <wqueue.h>

#ifdef CONFIG_SCHED_WORKQUEUE

int work_queue(int qid, struct work_s *work, worker_t worker, void *arg, uint32_t delay)
{
	struct wqueue_s *wqueue = &g_work[qid];

	/* First, initialize the work structure */
	work->worker = worker;           /* Work callback */
	work->arg    = arg;              /* Callback argument */
	work->delay  = delay;            /* Delay until work performed */

	/* Now, time-tag that entry and put it in the work queue.  This must be
	 * done with interrupts disabled.  This permits this function to be called
	 * from with task logic or interrupt handlers.
	 */

	work_lock(qid);
	work->qtime  = clock_systimer(); /* Time work queued */
	dq_addlast((dq_entry_t *)work, &wqueue->q);
    pthread_kill(wqueue->pid, SIGCONT);      /* Wake up the worker thread */

	work_unlock(qid);
	return 0;
}

#endif
