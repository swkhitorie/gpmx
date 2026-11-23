#include <gpmx/config.h>

#include "uorb_cpp_test.h"

#include "Publication.hpp"
#include "PublicationMulti.hpp"
#include "Subscription.hpp"
#include "SubscriptionCallback.hpp"
#include "SubscriptionBlocking.hpp"
#include "SubscriptionInterval.hpp"

#include "workqueue/WorkItem.hpp"

#include "uorb_manager.h"
#include "uorb_device_node.h"

#include <mlog.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>

#include <uorb/topics/orb_test.h>
//=============================================================================
// -DUORB_TEST_ORB_ID=ORB_ID(xxx) -DUORB_TEST_MSG_TYPE=xxx_s
//=============================================================================
#ifndef UORB_TEST_ORB_ID
#define UORB_TEST_ORB_ID	ORB_ID(orb_test)
#endif

#ifndef UORB_TEST_MSG_TYPE
#define UORB_TEST_MSG_TYPE	orb_test_s
#endif

namespace
{

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

//-----------------------------------------------------------------------------
// 1) Publication / Subscription / PublicationData / SubscriptionData
//-----------------------------------------------------------------------------
int uorb_cpp_pubsub_main()
{
	uORB::Publication<UORB_TEST_MSG_TYPE> pub{UORB_TEST_ORB_ID};
	TEST_CHECK(pub.advertise());

	UORB_TEST_MSG_TYPE data{};
	data.timestamp = 12345;
	TEST_CHECK(pub.publish(data));

	uORB::Subscription sub{UORB_TEST_ORB_ID};
	TEST_CHECK(sub.subscribe() || sub.advertised());
	TEST_CHECK(sub.advertised());
	TEST_CHECK(sub.valid());
	TEST_CHECK(sub.updated());

	UORB_TEST_MSG_TYPE out{};
	TEST_CHECK(sub.copy(&out));
	TEST_CHECK(out.timestamp == 12345);
	TEST_CHECK(!sub.updated());

	data.timestamp = 22222;
	TEST_CHECK(pub.publish(data));
	TEST_CHECK(sub.update(&out));
	TEST_CHECK(out.timestamp == 22222);

	// --- SubscriptionInterval ---
	// this section must be in front of PublicationData
	// PublicationBase deconstruction() will call unadvertise(),
    // and c layer will set flag, it will unregister same topic exsiting publicatoin
	{
		uORB::SubscriptionInterval si{UORB_TEST_ORB_ID, 0};

		TEST_CHECK(si.get_interval_us() == 0);
		si.set_interval_ms(50);
		TEST_CHECK(si.get_interval_us() == 50000);
		si.set_interval_us(0);
		TEST_CHECK(si.get_interval_us() == 0);

		data.timestamp = 55555;
		TEST_CHECK(pub.publish(data));
		TEST_CHECK(si.advertised());
		TEST_CHECK(si.updated());
		TEST_CHECK(si.update(&out));
		TEST_CHECK(out.timestamp == 55555);
	}

	// --- PublicationData / SubscriptionData ---
	{
		uORB::PublicationData<UORB_TEST_MSG_TYPE> pdata{UORB_TEST_ORB_ID};
		pdata.get().timestamp = 33333;
		TEST_CHECK(pdata.update());

		uORB::SubscriptionData<UORB_TEST_MSG_TYPE> sdata{UORB_TEST_ORB_ID};
		TEST_CHECK(sdata.get().timestamp == 33333);

		pdata.get().timestamp = 44444;
		TEST_CHECK(pdata.update());
		TEST_CHECK(sdata.update());
		TEST_CHECK(sdata.get().timestamp == 44444);
	}

	return g_failures;
}

//-----------------------------------------------------------------------------
// 2) SubscriptionCallback / SubscriptionCallbackWorkItem
//-----------------------------------------------------------------------------
class TestSubscriptionCallback : public uORB::SubscriptionCallback
{
public:
	explicit TestSubscriptionCallback(const orb_metadata *meta) :
		SubscriptionCallback(meta)
	{
	}

	void call() override { ++_count; }

	unsigned count() const { return _count; }

private:
	volatile unsigned _count{0};
};

class TestCallbackWorkItem : public px4::WorkItem
{
public:
	TestCallbackWorkItem() : WorkItem("uorb_cpp_test", px4::wq_configurations::test2) {}

	unsigned count() const { return _count; }

protected:
	void Run() override { ++_count; }

private:
	volatile unsigned _count{0};
};

int uorb_cpp_callback_main()
{
	uORB::Publication<UORB_TEST_MSG_TYPE> pub{UORB_TEST_ORB_ID};
	TEST_CHECK(pub.advertise());

	UORB_TEST_MSG_TYPE data{};
	data.timestamp = 1;

	// --- pure callback : publish sync trigger call() ---
	{
		TestSubscriptionCallback cb{UORB_TEST_ORB_ID};
		TEST_CHECK(cb.registerCallback());

		pub.publish(data);
		TEST_CHECK(cb.count() == 1);

		pub.publish(data);
		TEST_CHECK(cb.count() == 2);

		cb.unregisterCallback();
		pub.publish(data);
		TEST_CHECK(cb.count() == 2);		// will not trigger after unregister
	}

	// --- callback + WorkItem : publish trigger WorkItem::Run() ---
	{
		TestCallbackWorkItem wi;
		uORB::SubscriptionCallbackWorkItem sub{&wi, UORB_TEST_ORB_ID};

		TEST_CHECK(sub.registerCallback());

		pub.publish(data);

		for (int i = 0; (i < 2000) && (wi.count() == 0); ++i) {
			usleep(1000);
		}

		TEST_CHECK(wi.count() == 1);

		pub.publish(data);

		for (int i = 0; (i < 2000) && (wi.count() < 2); ++i) {
			usleep(1000);
		}

		TEST_CHECK(wi.count() == 2);
	}

	return g_failures;
}

//-----------------------------------------------------------------------------
// 3) PublicationMulti
//-----------------------------------------------------------------------------
int uorb_cpp_multi_main()
{
	uORB::PublicationMulti<UORB_TEST_MSG_TYPE> pub0{UORB_TEST_ORB_ID};
	uORB::PublicationMulti<UORB_TEST_MSG_TYPE> pub1{UORB_TEST_ORB_ID};

	TEST_CHECK(pub0.advertise());
	TEST_CHECK(pub1.advertise());

	const int inst0 = pub0.get_instance();
	const int inst1 = pub1.get_instance();

	TEST_CHECK(inst0 >= 0);
	TEST_CHECK(inst1 >= 0);
	TEST_CHECK(inst0 != inst1);

	UORB_TEST_MSG_TYPE d0{};
	d0.timestamp = 100;
	TEST_CHECK(pub0.publish(d0));

	UORB_TEST_MSG_TYPE d1{};
	d1.timestamp = 200;
	TEST_CHECK(pub1.publish(d1));

	uORB::Subscription sub0{UORB_TEST_ORB_ID, (uint8_t)inst0};
	uORB::Subscription sub1{UORB_TEST_ORB_ID, (uint8_t)inst1};

	TEST_CHECK(sub0.advertised());
	TEST_CHECK(sub1.advertised());

	UORB_TEST_MSG_TYPE out{};
	TEST_CHECK(sub0.copy(&out));
	TEST_CHECK(out.timestamp == 100);
	TEST_CHECK(sub1.copy(&out));
	TEST_CHECK(out.timestamp == 200);

	return g_failures;
}

//-----------------------------------------------------------------------------
// 4) SubscriptionBlocking
//-----------------------------------------------------------------------------
struct BlockingPublisherCtx {
	uORB::Publication<UORB_TEST_MSG_TYPE> *pub;
	volatile bool stop;
};

void *blocking_publisher(void *arg)
{
	BlockingPublisherCtx *ctx = (BlockingPublisherCtx *)arg;

	for (int i = 0; (i < 100) && !ctx->stop; ++i) {
		UORB_TEST_MSG_TYPE d{};
		d.timestamp = 1000 + i;
		ctx->pub->publish(d);
		usleep(10000);	// 10 ms
	}

	return nullptr;
}

int uorb_cpp_blocking_main()
{
	uORB::Publication<UORB_TEST_MSG_TYPE> pub{UORB_TEST_ORB_ID};
	TEST_CHECK(pub.advertise());

	BlockingPublisherCtx ctx{&pub, false};

	pthread_t th;

	if (pthread_create(&th, nullptr, blocking_publisher, &ctx) != 0) {
		TEST_CHECK(false);
		return g_failures;
	}

	uORB::SubscriptionBlocking<UORB_TEST_MSG_TYPE> bsub{UORB_TEST_ORB_ID};

	// wait the new publisher data
	UORB_TEST_MSG_TYPE out{};
	bool got = false;

	for (int i = 0; (i < 20) && !got; ++i) {
		UORB_TEST_MSG_TYPE tmp{};

		if (bsub.updateBlocking(tmp, 100000) && (tmp.timestamp >= 1000)) {
			out = tmp;
			got = true;
		}
	}

	ctx.stop = true;
	pthread_join(th, nullptr);

	TEST_CHECK(got);
	TEST_CHECK(out.timestamp >= 1000);

	return g_failures;
}

void uorb_cpp_test_init()
{
	uorb_manager_initialize();

	hrt_init();
	workqueue_manager_start();
}

} // namespace

extern "C" int uorb_cpp_pubsub_test(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	uorb_cpp_test_init();

	uorb_cpp_pubsub_main();

	KMRAW("uorb_cpp_pubsub_test: %s (%d failures)\r\n",
	      (g_failures == 0) ? "PASS" : "FAIL", g_failures);

	return (g_failures == 0) ? 0 : -1;
}

extern "C" int uorb_cpp_callback_test(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	uorb_cpp_test_init();

	uorb_cpp_callback_main();

	KMRAW("uorb_cpp_callback_test: %s (%d failures)\r\n",
	      (g_failures == 0) ? "PASS" : "FAIL", g_failures);

	return (g_failures == 0) ? 0 : -1;
}

extern "C" int uorb_cpp_multi_test(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	uorb_cpp_test_init();

	uorb_cpp_multi_main();

	KMRAW("uorb_cpp_multi_test: %s (%d failures)\r\n",
	      (g_failures == 0) ? "PASS" : "FAIL", g_failures);

	return (g_failures == 0) ? 0 : -1;
}

extern "C" int uorb_cpp_blocking_test(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	uorb_cpp_test_init();

	uorb_cpp_blocking_main();

	KMRAW("uorb_cpp_blocking_test: %s (%d failures)\r\n",
	      (g_failures == 0) ? "PASS" : "FAIL", g_failures);

	return (g_failures == 0) ? 0 : -1;
}

extern "C" int uorb_cpp_all_test(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	uorb_cpp_test_init();

	uorb_cpp_pubsub_main();
	uorb_cpp_callback_main();
	uorb_cpp_multi_main();
	uorb_cpp_blocking_main();

	KMRAW("uorb_cpp_all_test: %s (%d failures)\r\n",
	      (g_failures == 0) ? "PASS" : "FAIL", g_failures);

	return (g_failures == 0) ? 0 : -1;
}
