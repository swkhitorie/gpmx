
#include <gpmx/config.h>

#include "hrt_work.h"
#include <wqueue.h>
#include <stddef.h>

void hrt_work_cancel(struct work_s *work)
{
	struct wqueue_s *wqueue = &g_hrt_work;

	/* Cancelling the work is simply a matter of removing the work structure
	 * from the work queue.  This must be done with interrupts disabled because
	 * new work is typically added to the work queue from interrupt handlers.
	 */

	hrt_work_lock();

	if (work->worker != NULL) {
		/* A little test of the integrity of the work queue */

		/* Remove the entry from the work queue and make sure that it is
		 * mark as availalbe (i.e., the worker field is nullified).
		 */

		dq_rem(&work->dq, &wqueue->q);
		work->worker = NULL;
	}

	hrt_work_unlock();
}
