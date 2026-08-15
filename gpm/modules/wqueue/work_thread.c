#define _GNU_SOURCE
#include <gpmx/config.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <queue.h>

#include "work_lock.h"
#include "wqueue.h"

#include <driver/drv_hrt.h>

#ifdef	__cplusplus
extern "C" {
#endif

#if defined(CONFIG_POSIXRUN_ENABLE)
long SYS_TICKS_PER_SEC = 0; //sysconf(_SC_CLK_TCK);
#else
#include <time.h>
long SYS_TICKS_PER_SEC = CLOCKS_PER_SEC;
#endif

#ifdef	__cplusplus
}
#endif

#ifdef CONFIG_SCHED_WORKQUEUE

/* The state of each work queue. */
struct wqueue_s g_work[NWORKERS];

sem_t _work_lock[NWORKERS];

static void work_process(struct wqueue_s *wqueue, int lock_id)
{
	volatile struct work_s *work;
	worker_t  worker;
	void *arg;
	uint64_t elapsed;
	uint32_t remaining;
	uint32_t next;

	/* Then process queued work.  We need to keep interrupts disabled while
	 * we process items in the work list.
	 */

	next  = CONFIG_SCHED_WORKPERIOD;

	work_lock(lock_id);

	work  = (struct work_s *)wqueue->q.head;

	while (work) {
		/* Is this work ready?  It is ready if there is no delay or if
		 * the delay has elapsed. qtime is the time that the work was added
		 * to the work queue.  It will always be greater than or equal to
		 * zero.  Therefore a delay of zero will always execute immediately.
		 */

		elapsed = USEC2TICK(clock_systimer() - work->qtime);

		//printf("work_process: in ticks elapsed=%lu delay=%u\n", elapsed, work->delay);
		if (elapsed >= work->delay) {
			/* Remove the ready-to-execute work from the list */

			(void)dq_rem((struct dq_entry_s *)work, &wqueue->q);

			/* Extract the work description from the entry (in case the work
			 * instance by the re-used after it has been de-queued).
			 */

			worker = work->worker;
			arg    = work->arg;

			/* Mark the work as no longer being queued */

			work->worker = NULL;

			/* Do the work.  Re-enable interrupts while the work is being
			 * performed... we don't have any idea how long that will take!
			 */

			work_unlock(lock_id);

			if (!worker) {
                // WARN("MESSED UP: worker = 0\n")

			} else {
				worker(arg);
			}

			/* Now, unfortunately, since we re-enabled interrupts we don't
			 * know the state of the work list and we will have to start
			 * back at the head of the list.
			 */

			work_lock(lock_id);

			work  = (struct work_s *)wqueue->q.head;
		} else {
			/* This one is not ready.. will it be ready before the next
			 * scheduled wakeup interval?
			 */

			/* Here: elapsed < work->delay */
			remaining = USEC_PER_TICK * (work->delay - elapsed);

			if (remaining < next) {
				/* Yes.. Then schedule to wake up when the work is ready */
				next = remaining;
			}

			/* Then try the next in the list. */
			work = (struct work_s *)work->dq.flink;
		}
	}

	/* Wait awhile to check the work list.  We will wait here until either
	 * the time elapses or until we are awakened by a signal.
	 */
	work_unlock(lock_id);

	usleep(next);
}

void work_queues_init(void)
{
	sem_init(&_work_lock[HPWORK], 0, 1);
	sem_init(&_work_lock[LPWORK], 0, 1);
#ifdef CONFIG_SCHED_USRWORK
	sem_init(&_work_lock[USRWORK], 0, 1);
#endif

#ifdef CONFIG_POSIXRUN_ENABLE
	SYS_TICKS_PER_SEC = sysconf(_SC_CLK_TCK);
#endif

	// Create high priority worker thread
	{
		pthread_attr_t attr;
		pthread_t id;
		pthread_attr_init(&attr);
		struct sched_param attr_param;
		attr_param.sched_priority = sched_get_priority_max(SCHED_FIFO)-1;
		pthread_attr_setschedparam(&attr, &attr_param);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		pthread_attr_setstacksize(&attr, 2000);
		// g_work[HPWORK].pid = pthread_create(&id, &attr, &work_hpthread, NULL);
		pthread_create(&g_work[HPWORK].pid, &attr, &work_hpthread, NULL);
	}

	// Create low priority worker thread
	{
		pthread_attr_t attr;
		pthread_t id;
		pthread_attr_init(&attr);
		struct sched_param attr_param;
		attr_param.sched_priority = sched_get_priority_min(SCHED_FIFO);
		pthread_attr_setschedparam(&attr, &attr_param);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		pthread_attr_setstacksize(&attr, 2000);
		// g_work[LPWORK].pid = pthread_create(&id, &attr, &work_lpthread, NULL);
		pthread_create(&g_work[LPWORK].pid , &attr, &work_lpthread, NULL);
	}
}

#ifdef CONFIG_SCHED_HPWORK
void *work_hpthread(void *arg)
{
    pthread_setname_np(pthread_self(), "hpwork");

	for (;;) {
		/* First, perform garbage collection.  This cleans-up memory de-allocations
		 * that were queued because they could not be freed in that execution
		 * context (for example, if the memory was freed from an interrupt handler).
		 * NOTE: If the work thread is disabled, this clean-up is performed by
		 * the IDLE thread (at a very, very low priority).
		 */

#ifndef CONFIG_SCHED_LPWORK
		// sched_garbagecollection();
#endif

		/* Then process queued work.  We need to keep interrupts disabled while
		 * we process items in the work list.
		 */

		work_process(&g_work[HPWORK], HPWORK);
	}

	return NULL;
}

#ifdef CONFIG_SCHED_LPWORK
void *work_lpthread(void *arg)
{
    pthread_setname_np(pthread_self(), "lpwork");

	for (;;) {

		/* Then process queued work.  We need to keep interrupts disabled while
		 * we process items in the work list.
		 */

		work_process(&g_work[LPWORK], LPWORK);
	}

	return NULL;
}

#endif /* CONFIG_SCHED_LPWORK */
#endif /* CONFIG_SCHED_HPWORK */

#ifdef CONFIG_SCHED_USRWORK
int work_usrthread(int argc, char *argv[])
{
	pthread_setname_np(pthread_self(), "usrwork");

	for (;;) {
		/* Then process queued work.  We need to keep interrupts disabled while
		 * we process items in the work list.
		 */

		work_process(&g_work[USRWORK], USRWORK);
	}

	return 0;
}
#endif

uint32_t clock_systimer()
{
	return (0x00000000ffffffff & hrt_absolute_time());
}

#endif /* CONFIG_SCHED_WORKQUEUE */
