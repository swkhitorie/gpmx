#include <gpmx/config.h>

#include "wq_test.h"

#include <time.h>
#include <wqueue.h>
#include <unistd.h>
#include <stdio.h>

#include <mlog.h>

int WQueueTest::appState;

void WQueueTest::hp_worker_cb(void *p)
{
	WQueueTest *wqep = (WQueueTest *)p;

	wqep->do_hp_work();
}

void WQueueTest::lp_worker_cb(void *p)
{
	WQueueTest *wqep = (WQueueTest *)p;

	wqep->do_lp_work();
}

void WQueueTest::do_lp_work()
{
	static int iter = 0;
	KMRAW("done lp work %d\n", iter);

	if (iter > 5) {
		_lpwork_done = true;
	}

	++iter;

	work_queue(LPWORK, &_lpwork, (worker_t)&lp_worker_cb, this, 1000);
}

void WQueueTest::do_hp_work()
{
	static int iter = 0;
	KMRAW("done hp work %d\n", iter);

	if (iter > 5) {
		_hpwork_done = true;
	}

	++iter;

	// requeue
	work_queue(HPWORK, &_hpwork, (worker_t)&hp_worker_cb, this, 1000);
}

int WQueueTest::main()
{
	appState = 0xF1; // 0xF1:Running State, 0xF2:Exit Requested

	//Put work on HP work queue
	work_queue(HPWORK, &_hpwork, (worker_t)&hp_worker_cb, this, 1000);


	//Put work on LP work queue
	work_queue(LPWORK, &_lpwork, (worker_t)&lp_worker_cb, this, 1000);

	// Wait for work to finsh
	while (!(appState == 0xF2) && !(_hpwork_done && _lpwork_done)) {
		KMRAW("  Sleeping for 2 sec...\n");
		sleep(2);
	}

	return 0;
}

WQueueTest wq;
extern "C" int wq_test(int argc, char **argv)
{
	work_queues_init();
	wq.main();
    return 0;
}
