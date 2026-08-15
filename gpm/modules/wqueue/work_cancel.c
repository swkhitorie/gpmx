
#include <gpmx/config.h>
#include <queue.h>
#include <wqueue.h>
#include <stdint.h>
#include <stddef.h>
#include <pthread.h>
#include "work_lock.h"

#ifdef CONFIG_SCHED_WORKQUEUE

int work_cancel(int qid, struct work_s *work)
{
	struct wqueue_s *wqueue = &g_work[qid];

	/* Cancelling the work is simply a matter of removing the work structure
	 * from the work queue.  This must be done with interrupts disabled because
	 * new work is typically added to the work queue from interrupt handlers.
	 */

	work_lock(qid);

	if (work->worker != NULL) {
		/* A little test of the integrity of the work queue */

		/* Remove the entry from the work queue and make sure that it is
		 * mark as availalbe (i.e., the worker field is nullified).
		 */

		dq_rem((dq_entry_t *)work, &wqueue->q);
		work->worker = NULL;
	}

	work_unlock(qid);
	return 0;
}

#endif
