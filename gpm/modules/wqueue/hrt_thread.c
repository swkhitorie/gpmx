#include <gpmx/config.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <queue.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <wqueue.h>
#include "hrt_work.h"
#include <driver/drv_hrt.h>

/* The state of each work queue. */
struct wqueue_s g_hrt_work;

sem_t _hrt_work_lock;

static void hrt_work_process(void);

static void _sighandler(int sig_num)
{
	// PX4_DEBUG("RECEIVED SIGNAL %d", sig_num);
}

static void hrt_work_process()
{
	struct wqueue_s *wqueue = &g_hrt_work;
	volatile struct work_s *work;
	worker_t  worker;
	void *arg;
	uint64_t elapsed;
	uint32_t remaining;
	uint32_t next;

	pthread_setname_np(pthread_self(), "HRT");

	/* Then process queued work.  We need to keep interrupts disabled while
	 * we process items in the work list.
	 */

	/* Default to sleeping for 1 sec */
	next  = 1000000;

	hrt_work_lock();

	work  = (struct work_s *)wqueue->q.head;

	while (work) {
		/* Is this work ready?  It is ready if there is no delay or if
		 * the delay has elapsed. qtime is the time that the work was added
		 * to the work queue.  It will always be greater than or equal to
		 * zero.  Therefore a delay of zero will always execute immediately.
		 */

		elapsed = hrt_absolute_time() - work->qtime;

		//PX4_INFO("hrt work_process: in usec elapsed=%lu delay=%u work=%p", elapsed, work->delay, work);
		if (elapsed >= work->delay) {
			/* Remove the ready-to-execute work from the list */

			(void)dq_rem((dq_entry_t *)&work->dq, &wqueue->q);
			//PX4_INFO("Dequeued work=%p", work);

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

			hrt_work_unlock();

			if (!worker) {
				// ERROR("MESSED UP: worker = 0");

			} else {
				worker(arg);
			}

			/* Now, unfortunately, since we re-enabled interrupts we don't
			 * know the state of the work list and we will have to start
			 * back at the head of the list.
			 */

			hrt_work_lock();
			work  = (struct work_s *)wqueue->q.head;

		} else {
			/* This one is not ready.. will it be ready before the next
			 * scheduled wakeup interval?
			 */

			/* Here: elapsed < work->delay */
			remaining = work->delay - elapsed;

			//PX4_INFO("remaining=%u delay=%u elapsed=%lu", remaining, work->delay, elapsed);
			if (remaining < next) {
				/* Yes.. Then schedule to wake up when the work is ready */

				next = remaining;
			}

			/* Then try the next in the list. */

			work = (struct work_s *)work->dq.flink;
			//PX4_INFO("next %u work %p", next, work);
		}
	}

	/* Wait awhile to check the work list.  We will wait here until either
	 * the time elapses or until we are awakened by a signal.
	 */
	hrt_work_unlock();

	/* might sleep less if a signal received and new item was queued */
	usleep(next);
}

static void *work_hrtthread(void *arg)
{
	pthread_setname_np(pthread_self(), "wkr_hrt");

	for (;;) {
		/* First, perform garbage collection.  This cleans-up memory de-allocations
		 * that were queued because they could not be freed in that execution
		 * context (for example, if the memory was freed from an interrupt handler).
		 * NOTE: If the work thread is disabled, this clean-up is performed by
		 * the IDLE thread (at a very, very low priority).
		 */

		/* Then process queued work.  We need to keep interrupts disabled while
		 * we process items in the work list.
		 */

		hrt_work_process();
	}

	return NULL;
}

void hrt_work_queue_init(void)
{
	sem_init(&_hrt_work_lock, 0, 1);
	memset(&g_hrt_work, 0, sizeof(g_hrt_work));

	{
		pthread_attr_t attr;
		pthread_t id;
		pthread_attr_init(&attr);
		struct sched_param attr_param;
		attr_param.sched_priority = sched_get_priority_max(SCHED_FIFO);
		pthread_attr_setschedparam(&attr, &attr_param);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		pthread_attr_setstacksize(&attr, 2000);
		g_hrt_work.pid = pthread_create(&id, &attr, &work_hrtthread, NULL);
	}

	// Create high priority worker thread
	// g_hrt_work.pid = px4_task_spawn_cmd("wkr_hrt",
	// 				    SCHED_DEFAULT,
	// 				    SCHED_PRIORITY_MAX,
	// 				    2000,
	// 				    work_hrtthread,
	// 				    (char *const *)NULL);


	signal(SIGCONT, _sighandler);
}
