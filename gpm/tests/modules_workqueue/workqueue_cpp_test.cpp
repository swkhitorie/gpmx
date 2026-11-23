#include <gpmx/config.h>

#include "workqueue_cpp_test.h"

#include "WorkItem.hpp"
#include "ScheduledWorkItem.hpp"
#include "WorkItemSingleShot.hpp"

#include "workqueue_manager.h"

#include <drivers/drv_hrt.h>
#include <mlog/mlog.h>
#include <unistd.h>
#include <stdint.h>

int g_failures = 0;

#define TEST_CHECK(cond) \
	do { \
		if (cond) { \
			KMRAW("  [PASS] %s:%d  %s\r\n", __FILE__, __LINE__, #cond); \
		} else { \
			KMRAW("  [FAIL] %s:%d  %s\r\n", __FILE__, __LINE__, #cond); \
			++g_failures; \
		} \
	} while (0)

//=============================================================================
// 1) WorkItem
//    对应 wqueue_test.c：Run() 里不断把自身重新入队，跑到上限为止
//=============================================================================

constexpr uint32_t kWorkItemMaxRuns = 200000;

volatile uint32_t g_workitem_run_cnt = 0;
volatile bool     g_workitem_done    = false;

class TestWorkItem : public px4::WorkItem
{
public:
	TestWorkItem() : px4::WorkItem("wqueue_cpp_test", px4::wq_configurations::test1) {}

protected:
	void Run() override
	{
		if (g_workitem_run_cnt > kWorkItemMaxRuns) {
			g_workitem_done = true;

		} else {
			ScheduleNow();
		}

		++g_workitem_run_cnt;
	}
};

TestWorkItem *g_workitem = nullptr;

void wqueue_cpp_workitem_test_init()
{
	if (g_workitem == nullptr) {
		g_workitem = new TestWorkItem();

		if (g_workitem == nullptr) {
			KMRAW("new TestWorkItem failed\r\n");
		}
	}
}

int wqueue_cpp_workitem_main()
{
	if (g_workitem == nullptr) {
		TEST_CHECK(false);
		return g_failures;
	}

	g_workitem_run_cnt = 0;
	g_workitem_done = false;

	g_workitem->ScheduleNow();

	// 用循环次数做超时：本移植的 hrt 可能由 lockstep 驱动，不能拿它当墙钟
	for (uint32_t i = 0; (!g_workitem_done) && (i < 20000); ++i) {
		usleep(5000);
	}

	TEST_CHECK(g_workitem_done);
	TEST_CHECK(g_workitem_run_cnt > kWorkItemMaxRuns);
	TEST_CHECK(g_workitem->ItemName() != nullptr);
	TEST_CHECK(strcmp(g_workitem->ItemName(), "wqueue_cpp_test") == 0);

	KMRAW("WorkItem test: runs=%u\r\n", (unsigned)g_workitem_run_cnt);

	sleep(1);
	return g_failures;
}

//=============================================================================
// 2) ScheduledWorkItem
//    对应 wqueue_scheduled_test.c：4ms 周期跑 1000 次；额外验证
//    ScheduleDelayed / Scheduled() / ScheduleClear()
//=============================================================================

constexpr uint32_t kSchedIntervalUs = 4000;
constexpr uint32_t kSchedExpectedRuns = 1000;

volatile uint32_t g_sched_run_cnt = 0;
volatile bool     g_sched_done    = false;

class TestScheduledWorkItem : public px4::ScheduledWorkItem
{
public:
	TestScheduledWorkItem() : px4::ScheduledWorkItem("wqueue_cpp_sched_test", px4::wq_configurations::test2) {}

	void start_delayed(uint32_t delay_us, uint32_t expected_runs)
	{
		_expected_runs = expected_runs;
		_stop_on_done = false;
		ScheduleDelayed(delay_us);
	}

	void start_interval(uint32_t interval_us, uint32_t expected_runs)
	{
		_expected_runs = expected_runs;
		_stop_on_done = true;
		ScheduleOnInterval(interval_us);
	}

protected:
	void Run() override
	{
		++g_sched_run_cnt;

		if (g_sched_run_cnt >= _expected_runs) {
			g_sched_done = true;

			if (_stop_on_done) {
				ScheduleClear();
			}
		}
	}

private:
	uint32_t _expected_runs{1};
	bool     _stop_on_done{false};
};

TestScheduledWorkItem *g_sched = nullptr;

void wqueue_cpp_scheduled_test_init()
{
	if (g_sched == nullptr) {
		g_sched = new TestScheduledWorkItem();

		if (g_sched == nullptr) {
			KMRAW("new TestScheduledWorkItem failed\r\n");
		}
	}
}

int wqueue_cpp_scheduled_main()
{
	if (g_sched == nullptr) {
		TEST_CHECK(false);
		return g_failures;
	}

	// --- ScheduleDelayed：只跑一次 ---
	g_sched_run_cnt = 0;
	g_sched_done = false;

	g_sched->start_delayed(10000, 1);	// 10 ms

	TEST_CHECK(g_sched->Scheduled());


	for (uint32_t i = 0; (!g_sched_done) && (i < 4000); ++i) {
		usleep(5000);
	}

	TEST_CHECK(g_sched_done);
	TEST_CHECK(g_sched_run_cnt == 1);
	TEST_CHECK(!g_sched->Scheduled());	// hrt 到期后 deadline 归零

	// --- ScheduleOnInterval：4 ms 周期 ---
	g_sched_run_cnt = 0;
	g_sched_done = false;

	g_sched->start_interval(kSchedIntervalUs, kSchedExpectedRuns);

	for (uint32_t i = 0; (!g_sched_done) && (i < 20000); ++i) {
		usleep(5000);
	}

	TEST_CHECK(g_sched_done);
	TEST_CHECK(g_sched_run_cnt >= kSchedExpectedRuns);
	TEST_CHECK(!g_sched->Scheduled());	// Run() 里已 ScheduleClear()

	KMRAW("ScheduledWorkItem test: runs=%u\r\n", (unsigned)g_sched_run_cnt);

	sleep(1);
	return g_failures;
}

//=============================================================================
// 3) WorkItemSingleShot
//    两种构造：直接给 wq_config、或复用已有 WorkItem 的队列
//=============================================================================

volatile uint32_t g_shot_run_cnt = 0;
volatile void    *g_shot_arg     = nullptr;

void wqueue_cpp_shot_method(void *arg)
{
	++g_shot_run_cnt;
	g_shot_arg = arg;
}

TestWorkItem *g_anchor = nullptr;	// 仅作为“已有 WorkItem”给第二种构造用

void wqueue_cpp_singleshot_test_init()
{
	if (g_anchor == nullptr) {
		g_anchor = new TestWorkItem();
	}
}

int wqueue_cpp_singleshot_main()
{
	if (g_anchor == nullptr) {
		TEST_CHECK(false);
		return g_failures;
	}

	// 1) 直接指定 work queue
	{
		int token = 0x11;

		g_shot_run_cnt = 0;
		g_shot_arg = nullptr;

		px4::WorkItemSingleShot shot(px4::wq_configurations::test2, wqueue_cpp_shot_method, &token);
		shot.ScheduleNow();
		shot.wait();

		TEST_CHECK(g_shot_run_cnt == 1);
		TEST_CHECK(g_shot_arg == (void *)&token);
	}

	// 2) 复用已有 WorkItem 的队列
	{
		int token = 0x22;

		g_shot_run_cnt = 0;
		g_shot_arg = nullptr;

		px4::WorkItemSingleShot shot(*g_anchor, wqueue_cpp_shot_method, &token);
		shot.ScheduleNow();
		shot.wait();

		TEST_CHECK(g_shot_run_cnt == 1);
		TEST_CHECK(g_shot_arg == (void *)&token);
	}

	KMRAW("WorkItemSingleShot test: runs=%u\r\n", (unsigned)g_shot_run_cnt);

	return g_failures;
}

void wqueue_cpp_cleanup()
{
	delete g_workitem;
	g_workitem = nullptr;

	delete g_sched;
	g_sched = nullptr;

	delete g_anchor;
	g_anchor = nullptr;
}

//=============================================================================
// 对外测试入口（与 wqueue_test()/wqueue_scheduled_test()/wqueue_all_test() 对齐）
//=============================================================================

extern "C" int wqueue_cpp_workitem_test(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	hrt_init();
	workqueue_manager_start();

	wqueue_cpp_workitem_test_init();
	wqueue_cpp_workitem_main();

	workqueue_manager_status();
	wqueue_cpp_cleanup();

	KMRAW("wqueue_cpp_workitem_test: %s (%d failures)\r\n",
	      (g_failures == 0) ? "PASS" : "FAIL", g_failures);

	return (g_failures == 0) ? 0 : -1;
}

extern "C" int wqueue_cpp_scheduled_test(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	hrt_init();
	workqueue_manager_start();

	wqueue_cpp_scheduled_test_init();
	wqueue_cpp_scheduled_main();

	workqueue_manager_status();
	wqueue_cpp_cleanup();

	KMRAW("wqueue_cpp_scheduled_test: %s (%d failures)\r\n",
	      (g_failures == 0) ? "PASS" : "FAIL", g_failures);

	return (g_failures == 0) ? 0 : -1;
}

extern "C" int wqueue_cpp_singleshot_test(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	hrt_init();
	workqueue_manager_start();

	wqueue_cpp_singleshot_test_init();
	wqueue_cpp_singleshot_main();

	workqueue_manager_status();
	wqueue_cpp_cleanup();

	KMRAW("wqueue_cpp_singleshot_test: %s (%d failures)\r\n",
	      (g_failures == 0) ? "PASS" : "FAIL", g_failures);

	return (g_failures == 0) ? 0 : -1;
}

extern "C" int wqueue_cpp_all_test(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	hrt_init();
	workqueue_manager_start();

	sleep(1);

	wqueue_cpp_workitem_test_init();
	wqueue_cpp_scheduled_test_init();
	wqueue_cpp_singleshot_test_init();

	wqueue_cpp_workitem_main();
	wqueue_cpp_scheduled_main();
	wqueue_cpp_singleshot_main();

	workqueue_manager_status();
	wqueue_cpp_cleanup();

	KMRAW("wqueue_cpp_all_test: %s (%d failures)\r\n",
	      (g_failures == 0) ? "PASS" : "FAIL", g_failures);

	return (g_failures == 0) ? 0 : -1;
}
