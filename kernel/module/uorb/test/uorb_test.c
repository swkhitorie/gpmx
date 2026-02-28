#include "uorb_test.h"
#include "uorb_common.h"
#include "uorb_manager.h"
#include "workqueue_manager.h"
#include <uorb/topics/orb_test.h>
#include <uorb/topics/orb_test_medium.h>
#include <uorb/topics/orb_test_large.h>
#include <pthread.h>
#include <unistd.h>
#include "kmodule_defines.h"

orb_advert_t _pfd[4];
bool _thread_should_exit = false;

int uorb_unit_test_single()
{
	struct orb_test_s t;
	struct orb_test_s u;
	int sfd = -1;
	orb_advert_t ptopic;
	bool updated = false;

    KMINFO("============== uORB Node: try single-topic support\n");

	t.val = 0;
	ptopic = orb_advertise(ORB_ID(orb_test), &t);

	if (ptopic == NULL) {
        KMINFO("uORB Fail: advertise failed: %d\n", errno);
		goto unittest_single_exit;
	}

	KMINFO("uORB Node: publish handle %p\n", ptopic);
	sfd = orb_subscribe(ORB_ID(orb_test));
	if (sfd < 0) {
		KMINFO("uORB Fail: subscribe failed: %d\n", errno);
        goto unittest_single_exit;
	}

	KMINFO("uORB Node: subscribe fd %d\n", sfd);
	u.val = 1;

	if (0 != orb_copy(ORB_ID(orb_test), sfd, &u)) {
		KMINFO("uORB Fail: copy(1) failed: %d\n", errno);
        goto unittest_single_exit;
	}

	if (u.val != t.val) {
		KMINFO("uORB Fail: copy(1) mismatch: %d expected %d\n", u.val, t.val);
        goto unittest_single_exit;
	}

	if (0 != orb_check(sfd, &updated)) {
		KMINFO("uORB Fail: check(1) failed\n");
        goto unittest_single_exit;
	}

	if (updated) {
		KMINFO("uORB Fail: spurious updated flag\n");
        goto unittest_single_exit;
	}

	t.val = 2;
	KMINFO("uORB Node: try publish\n");

	if (0 != orb_publish(ORB_ID(orb_test), ptopic, &t)) {
        KMINFO("uORB Fail: publish failed\n");
		goto unittest_single_exit;
	}

	if (0 != orb_check(sfd, &updated)) {
        KMINFO("uORB Fail: check(2) failed\n");
		goto unittest_single_exit;
	}

	if (!updated) {
        KMINFO("uORB Fail: missing updated flag\n");
		goto unittest_single_exit;
	}

	if (0 != orb_copy(ORB_ID(orb_test), sfd, &u)) {
        KMINFO("uORB Fail: copy(2) failed: %d\n", errno);
		goto unittest_single_exit;
	}

	if (u.val != t.val) {
        KMINFO("uORB Fail: copy(2) mismatch: %d expected %d\n", u.val, t.val);
		goto unittest_single_exit;
	}

	orb_unsubscribe(sfd);

	int ret = orb_unadvertise(ptopic);

	if (ret != 0) {
        KMINFO("uORB Fail: orb_unadvertise failed: %i\n", ret);
		goto unittest_single_exit;
	}

    KMINFO("uORB Node: PASS single-topic test\n");
	return 0;
unittest_single_exit:
    return -1;
}

int uorb_unit_test_multi()
{
	/* this routine tests the multi-topic support */
	KMINFO("============== uORB Node: try multi-topic support\r\n");

	struct orb_test_s t;
	struct orb_test_s u;

	int instance0;
	_pfd[0] = orb_advertise_multi(ORB_ID(orb_multitest), &t, &instance0, ORB_PRIO_MAX);

	KMINFO("uORB Node: advertised %d \r\n", xPortGetFreeHeapSize());

	int instance1;
	_pfd[1] = orb_advertise_multi(ORB_ID(orb_multitest), &t, &instance1, ORB_PRIO_MIN);

	KMINFO("uORB Node: advertised2\r\n");
	if (instance0 != 0) {
		KMINFO("uORB Fail: mult. id0: %d\r\n", instance0);
		goto unittest_multi_exit;
	}

	if (instance1 != 1) {
		KMINFO("uORB Fail: mult. id1: %d\r\n", instance1);
		goto unittest_multi_exit; 
	}

	t.val = 103;

	if (0 != orb_publish(ORB_ID(orb_multitest), _pfd[0], &t)) {
		KMINFO("uORB Fail: mult. pub0 fail\r\n");
		goto unittest_multi_exit;
	}

	KMINFO("uORB Node: published\r\n");

	t.val = 203;

	if (0 != orb_publish(ORB_ID(orb_multitest), _pfd[1], &t)) {
		KMINFO("uORB Fail: mult. pub1 fail\r\n");
		goto unittest_multi_exit; 
	}

	/* subscribe to both topics and ensure valid data is received */
	int sfd0 = orb_subscribe_multi(ORB_ID(orb_multitest), 0);

	if (0 != orb_copy(ORB_ID(orb_multitest), sfd0, &u)) {
		KMINFO("uORB Fail: sub #0 copy failed: %d\r\n", errno);
		goto unittest_multi_exit; 
	}

	if (u.val != 103) {
		KMINFO("uORB Fail: sub #0 val. mismatch: %d\r\n", u.val);
		goto unittest_multi_exit; 
	}

	int sfd1 = orb_subscribe_multi(ORB_ID(orb_multitest), 1);

	if (0 != orb_copy(ORB_ID(orb_multitest), sfd1, &u)) {
		KMINFO("uORB Fail: sub #1 copy failed: %d\r\n", errno);
		goto unittest_multi_exit; 
	}

	if (u.val != 203) {
		KMINFO("uORB Fail: sub #1 val. mismatch: %d\r\n", u.val);
		goto unittest_multi_exit; 
	}

	/* test priorities */
	enum ORB_PRIO prio;

	if (0 != orb_priority(sfd0, &prio)) {
		KMINFO("uORB Fail: prio #0\r\n");
		goto unittest_multi_exit; 
	}

	if (prio != ORB_PRIO_MAX) {
		KMINFO("uORB Fail: prio: %d\r\n", prio);
		goto unittest_multi_exit; 
	}

	if (0 != orb_priority(sfd1, &prio)) {
		KMINFO("uORB Fail: prio #1\r\n");
		goto unittest_multi_exit;
	}

	if (prio != ORB_PRIO_MIN) {
		KMINFO("uORB Fail: prio: %d\r\n", prio);
		goto unittest_multi_exit; 
	}

	orb_unsubscribe(sfd0);
	orb_unsubscribe(sfd1);

	KMINFO("uORB Node: PASS multi-topic test\r\n");

	return 0;
unittest_multi_exit:
    return -1;
}


int uorb_unit_test_multi_reversed()
{
	KMINFO("============== uORB Node: try multi-topic support subscribing before publishing\r\n");

	/* For these tests 0 and 1 instances are taken from before, therefore continue with 2 and 3. */

	/* Subscribe first and advertise afterwards. */
	int sfd2 = orb_subscribe_multi(ORB_ID(orb_multitest), 2);

	KMINFO("sfd2:%d \r\n", sfd2);
	if (sfd2 < 0) {
		KMINFO("uORB Fail: sub. id2: ret: %d\r\n", sfd2);
		goto unittest_multi_reserved_exit; 
	}

	struct orb_test_s t;
	struct orb_test_s u;

	t.val = 0;

	int instance2;

	_pfd[2] = orb_advertise_multi(ORB_ID(orb_multitest), &t, &instance2, ORB_PRIO_MAX);

	KMINFO("uORB Node: advertised 1\r\n");

	int instance3;

	_pfd[3] = orb_advertise_multi(ORB_ID(orb_multitest), &t, &instance3, ORB_PRIO_MIN);

	KMINFO("uORB Node: advertised\r\n");

	if (instance2 != 2) {
		KMINFO("uORB Fail: mult. id2: %d\r\n", instance2);
		goto unittest_multi_reserved_exit;  
	}

	if (instance3 != 3) {
		KMINFO("uORB Fail: mult. id3: %d\r\n", instance3);
		goto unittest_multi_reserved_exit;  
	}

	t.val = 204;

	if (0 != orb_publish(ORB_ID(orb_multitest), _pfd[2], &t)) {
		KMINFO("uORB Fail: mult. pub0 fail\r\n");
		goto unittest_multi_reserved_exit; 
	}

	t.val = 304;

	if (0 != orb_publish(ORB_ID(orb_multitest), _pfd[3], &t)) {
		KMINFO("uORB Fail: mult. pub1 fail\r\n");
		goto unittest_multi_reserved_exit; 
	}

	KMINFO("uORB Node: published\r\n");

	if (0 != orb_copy(ORB_ID(orb_multitest), sfd2, &u)) {
		KMINFO("uORB Fail: sub #2 copy failed: %d\r\n", errno);
		goto unittest_multi_reserved_exit; 
	}

	if (u.val != 204) {
		KMINFO("uORB Fail: sub #3 val. mismatch: %d %d\r\n", u.val, sfd2);
		goto unittest_multi_reserved_exit; 
	}

	int sfd3 = orb_subscribe_multi(ORB_ID(orb_multitest), 3);

	if (0 != orb_copy(ORB_ID(orb_multitest), sfd3, &u)) {
		KMINFO("uORB Fail: sub #3 copy failed: %d\r\n", errno);
		goto unittest_multi_reserved_exit; 
	}

	if (u.val != 304) {
		KMINFO("uORB Fail: sub #3 val. mismatch: %d\r\n", u.val);
		goto unittest_multi_reserved_exit; 
	}

	KMINFO("uORB Node: PASS multi-topic reversed\r\n");
	return 0;
unittest_multi_reserved_exit:
    return -1;
}

int uorb_unit_test_unadvertise()
{
	KMINFO("============== uORB Node: Testing unadvertise");

	//we still have the advertisements from the previous test_multi calls.
	for (int i = 0; i < 4; ++i) {
		int ret = orb_unadvertise(_pfd[i]);

		if (ret != 0) {
			KMINFO("uORB Fail: orb_unadvertise failed (%i)", ret);
			return -1;
		}
	}

	//try to advertise and see whether we get the right instance
	int instance_test[4] = {0};
	struct orb_test_s t;

	for (int i = 0; i < 4; ++i) {
		_pfd[i] = orb_advertise_multi(ORB_ID(orb_multitest), &t, &instance_test[i], ORB_PRIO_MAX);

		if (instance_test[i] != i) {
			KMINFO("uORB Fail: got wrong instance (should be %i, is %i)", i, instance_test[i]);
			return -1;
		}
	}

	for (int i = 0; i < 4; ++i) {
		orb_unadvertise(_pfd[i]);
	}

	KMINFO("uORB Node: PASS unadvertise\r\n");
	return 0;
}

void *pub_test_multi2_entry(void *arg)
{
	int data_next_idx = 0;
	const int num_instances = 3;
	orb_advert_t orb_pub[num_instances];
	struct orb_test_medium_s data_topic = {0};

	pthread_setname_np(pthread_self(), "uorb_test_multi");

	for (int i = 0; i < num_instances; ++i) {
		orb_advert_t *pub = &orb_pub[i];
		int idx = i;
		KMINFO("advertise %i, t=%" PRIu64"\r\n", i, hrt_absolute_time());
		KMINFO("before advertise %x \r\n", &data_topic);
		*pub = orb_advertise_multi(ORB_ID(orb_test_medium_multi), &data_topic, &idx, ORB_PRIO_DEFAULT);

		if (idx != i) {
			_thread_should_exit = true;
			KMINFO("uORB Fail: Got wrong instance! should be: %i, but is %i\r\n", i, idx);
			return NULL;
		}
	}

	usleep(100 * 1000);

	int message_counter = 0;
	int num_messages = 50 * num_instances;

	while (message_counter++ < num_messages) {
		usleep(2); //make sure the timestamps are different
		orb_advert_t *pub = &orb_pub[data_next_idx];

		data_topic.timestamp = hrt_absolute_time();
		data_topic.val = data_next_idx;

		orb_publish(ORB_ID(orb_test_medium_multi), *pub, &data_topic);
		KMINFO("publishing %d msg (idx=%i, t=%" PRIu64 ")\r\n", message_counter, data_next_idx, data_topic.timestamp);

		data_next_idx = (data_next_idx + 1) % num_instances;

		if (data_next_idx == 0) {
			usleep(50 * 1000);
		}
	}

	KMINFO("publishing msg end\r\n");

	usleep(100 * 1000);
	_thread_should_exit = true;

	for (int i = 0; i < num_instances; ++i) {
		orb_unadvertise(orb_pub[i]);
	}

	return NULL;
}


int uorn_unit_test_multi2()
{
    pthread_attr_t attr;
    pthread_t id;

	#define num_instances 3
	KMINFO("============== uORB Node: Testing multi-topic 2 test (queue simulation)\r\n");
	//test: first subscribe, then advertise

	_thread_should_exit = false;
	int orb_data_fd[num_instances] = {-1, -1, -1};
	int orb_data_next = 0;

	for (int i = 0; i < num_instances; ++i) {
		orb_data_fd[i] = orb_subscribe_multi(ORB_ID(orb_test_medium_multi), i);
	}

	char *args = NULL;

    pthread_attr_init(&attr);
    attr.schedparam.sched_priority = sched_get_priority_max(0) - 5;
    attr.schedpolicy = PTHREAD_CREATE_JOINABLE;
    attr.stacksize = 3000 * sizeof(StackType_t);

    int pubsub_task = pthread_create(&id, &attr, &pub_test_multi2_entry, args);

	if (pubsub_task < 0) {
		KMINFO("uORB Fail: launching task\r\n");
		return -1;
	}

	hrt_abstime last_time = 0;
	struct orb_test_medium_s msg = {0};
	while (!_thread_should_exit) {

		usleep(1000);

		bool updated = false;
		int orb_data_cur_fd = orb_data_fd[orb_data_next];
		orb_check(orb_data_cur_fd, &updated);

		if (updated) {

			orb_copy(ORB_ID(orb_test_medium_multi), orb_data_cur_fd, &msg);

			if (last_time >= msg.timestamp && last_time != 0) {
				KMINFO("uORB Fail: Timestamp not increasing! (%" PRIu64 " >= %" PRIu64 ")\r\n", last_time, msg.timestamp);
				return -1;
			}

			last_time = msg.timestamp;

			KMINFO("got message (val=%i, idx=%i, t=%" PRIu64 ")\r\n", msg.val, orb_data_next, msg.timestamp);
			orb_data_next = (orb_data_next + 1) % num_instances;
		}
	}

	for (int i = 0; i < num_instances; ++i) {
		orb_unsubscribe(orb_data_fd[i]);
	}

	KMINFO("uORB Node: PASS multi-topic 2 test (queue simulation)\r\n");
	return 0;
}

int uorb_unit_test_queue()
{
	KMINFO("============== uORB Node: Testing orb queuing\r\n");

	struct orb_test_medium_s t = {0};
	struct orb_test_medium_s u = {0};
	orb_advert_t ptopic = NULL;
	bool updated = false;

	int sfd = orb_subscribe(ORB_ID(orb_test_medium_queue));

	if (sfd < 0) {
		KMINFO("uORB Fail: subscribe failed: %d\r\n", errno);
		goto unittest_queue_exit;
	}

	const int queue_size = 11;
	t.val = 0;
	ptopic = orb_advertise_queue(ORB_ID(orb_test_medium_queue), &t, queue_size);

	if (ptopic == NULL) {
		KMINFO("uORB Fail: advertise failed: %d\r\n", errno);
		goto unittest_queue_exit;
	}

	orb_check(sfd, &updated);

	if (!updated) {
		KMINFO("uORB Fail: update flag not set\r\n");
		goto unittest_queue_exit;
	}

	if (0 != orb_copy(ORB_ID(orb_test_medium_queue), sfd, &u)) {
		KMINFO("uORB Fail: copy(1) failed: %d\r\n", errno);
		goto unittest_queue_exit;
	}

	if (u.val != t.val) {
		KMINFO("uORB Fail: copy(1) mismatch: %d expected %d\r\n", u.val, t.val);
		goto unittest_queue_exit;
	}

	orb_check(sfd, &updated);

	if (updated) {
		KMINFO("uORB Fail: spurious updated flag\r\n");
		goto unittest_queue_exit;
	}

#define CHECK_UPDATED(element) \
	orb_check(sfd, &updated); \
	if (!updated) { \
		KMINFO("uORB Fail: update flag not set, element %i\r\n", element); \
		goto unittest_queue_exit; \
	}
#define CHECK_NOT_UPDATED(element) \
	orb_check(sfd, &updated); \
	if (updated) { \
		KMINFO("uORB Fail: update flag set, element %i\r\n", element); \
		goto unittest_queue_exit; \
	}
#define CHECK_COPY(i_got, i_correct) \
	orb_copy(ORB_ID(orb_test_medium_queue), sfd, &u); \
	if (i_got != i_correct) { \
		KMINFO("uORB Fail: got wrong element from the queue (got %i, should be %i)\r\n", i_got, i_correct); \
		goto unittest_queue_exit; \
	}

	//no messages in the queue anymore

	KMINFO("uORB Node:   Testing to write some elements...\r\n");

	for (int i = 0; i < queue_size - 2; ++i) {
		t.val = i;
		orb_publish(ORB_ID(orb_test_medium_queue), ptopic, &t);
	}

	for (int i = 0; i < queue_size - 2; ++i) {
		CHECK_UPDATED(i);
		CHECK_COPY(u.val, i);
	}

	CHECK_NOT_UPDATED(queue_size);

	KMINFO("uORB Node:  Testing overflow...\r\n");
	int overflow_by = 3;

	for (int i = 0; i < queue_size + overflow_by; ++i) {
		t.val = i;
		orb_publish(ORB_ID(orb_test_medium_queue), ptopic, &t);
	}

	for (int i = 0; i < queue_size; ++i) {
		CHECK_UPDATED(i);
		CHECK_COPY(u.val, i + overflow_by);
	}

	CHECK_NOT_UPDATED(queue_size);

	KMINFO("uORB Node:  Testing underflow...\r\n");

	for (int i = 0; i < queue_size; ++i) {
		CHECK_NOT_UPDATED(i);
		CHECK_COPY(u.val, queue_size + overflow_by - 1);
	}

	t.val = 943;
	orb_publish(ORB_ID(orb_test_medium_queue), ptopic, &t);
	CHECK_UPDATED(-1);
	CHECK_COPY(u.val, t.val);

#undef CHECK_COPY
#undef CHECK_UPDATED
#undef CHECK_NOT_UPDATED

	orb_unadvertise(ptopic);

	KMINFO("uORB Node: PASS orb queuing\r\n");
	return 0;
unittest_queue_exit:
    return -1;
}

int uorb_unit_test_entry()
{
	KMINFO("\r\n");
	int ret = uorb_unit_test_single();
	if (ret != 0) {
		return ret;
	}

	KMINFO("\r\n");
	ret = uorb_unit_test_multi();
	if (ret != 0) {
		return ret;
	}

	KMINFO("\r\n");
	ret = uorb_unit_test_multi_reversed();
	if (ret != 0) {
		return ret;
	}

	KMINFO("\r\n");
	ret = uorb_unit_test_unadvertise();
	if (ret != 0) {
		return ret;
	}

	KMINFO("\r\n");
	ret = uorn_unit_test_multi2();
	if (ret != 0) {
		return ret;
	}

	KMINFO("\r\n");
	ret = uorb_unit_test_queue();
	if (ret != 0) {
		return ret;
	}

	KMINFO("\r\n");
	// test_queue_poll_notify();
	return 0;
}

int uorb_test(int argc, char **argv)
{
    hrt_init();
    workqueue_manager_start();

    uorb_manager_initialize();

    uorb_unit_test_entry();

	uorb_device_master_print_statistics(uorb_manager_get_device_master(uorb_manager_instance()));
	uorb_device_master_showtop(uorb_manager_get_device_master(uorb_manager_instance()), NULL, 0);

    return 0;
}

