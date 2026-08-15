
#include <gpmx/config.h>
#include <stdint.h>
#include <stdio.h>
#include <queue.h>
#include <signal.h>
#include <semaphore.h>
#include <driver/drv_hrt.h>

#include "hrt_work.h"
#include <wqueue.h>

#include <sys/types.h>
#include <unistd.h>

int hrt_work_queue(struct work_s *work, worker_t worker, void *arg, uint32_t delay)
{
	struct wqueue_s *wqueue = &g_hrt_work;

	/* First, initialize the work structure */

	work->worker = worker;           /* Work callback */
	work->arg    = arg;              /* Callback argument */
	work->delay  = delay;            /* Delay until work performed */

	/* Now, time-tag that entry and put it in the work queue.  This must be
	 * done with interrupts disabled.  This permits this function to be called
	 * from with task logic or interrupt handlers.
	 */

	hrt_work_lock();
	work->qtime  = hrt_absolute_time(); /* Time work queued */

	dq_addlast(&work->dq, &wqueue->q);

	if (getpid() != wqueue->pid) {
        /* only need to wake up if called from a different thread */
		//wqueue->pid == own task? -> don't signal
		pthread_kill(wqueue->pid, SIGCONT);      /* Wake up the worker thread */
	}

	hrt_work_unlock();
	return 0;
}
