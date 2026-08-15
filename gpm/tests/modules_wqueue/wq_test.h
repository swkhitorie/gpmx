#ifndef WQ_TEST_H_
#define WQ_TEST_H_

#include <wqueue.h>
#include <string.h>

class WQueueTest
{
public:
	WQueueTest() = default;
	~WQueueTest() = default;

	int main();

	static int appState;

private:
	static void hp_worker_cb(void *p);
	static void lp_worker_cb(void *p);

	void do_lp_work(void);
	void do_hp_work(void);

	bool _lpwork_done {false};
	bool _hpwork_done {false};
	work_s _lpwork {};
	work_s _hpwork {};
};

extern "C" {
	int wq_test(int argc, char **argv);
}

#endif
