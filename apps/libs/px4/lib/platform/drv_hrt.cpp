#include <gpmx/config.h>

#include <errno.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
#include <drivers/drv_hrt.h>

#include <wqueue.h>

#include "hrt_work.h"

extern "C" {
void hrt_work_cancel(struct work_s *work);
int  hrt_work_queue(struct work_s *work, worker_t worker, void *arg, uint32_t usdelay);
void hrt_work_queue_init(void);
}

#include <platform/common/time.h>
#include <platform/common/defines.h>

#if defined(ENABLE_LOCKSTEP_SCHEDULER)
#include <lockstep_scheduler/lockstep_scheduler.h>
#include <platform/common/defines.h>
static LockstepScheduler lockstep_scheduler {true};
#endif

// Intervals in usec
static constexpr unsigned HRT_INTERVAL_MIN = 50;
static constexpr unsigned HRT_INTERVAL_MAX = 50000000;

/*
 * Queue of callout entries.
 */
static struct sq_queue_s	callout_queue;

/* latency baseline (last compare value applied) */
static uint64_t			latency_baseline;

/* timer count at interrupt (for latency purposes) */
static uint64_t			latency_actual;

/* latency histogram */
const uint16_t latency_bucket_count = LATENCY_BUCKET_COUNT;
const uint16_t latency_buckets[LATENCY_BUCKET_COUNT] = { 1, 2, 5, 10, 20, 50, 100, 1000 };
uint32_t latency_counters[LATENCY_BUCKET_COUNT + 1];

static sem_t 	        _hrt_lock;
static struct work_s	_hrt_work;
static bool             _hrt_initialized = false;

static void hrt_latency_update();

static void hrt_call_reschedule();
static void hrt_call_invoke();

static void hrt_lock()
{
	// loop as the wait may be interrupted by a signal
	do {} while (sem_wait(&_hrt_lock) != 0);
}

static void hrt_unlock()
{
	sem_post(&_hrt_lock);
}

hrt_abstime hrt_absolute_time()
{
#if defined(ENABLE_LOCKSTEP_SCHEDULER)
	// optimized case (avoid ts_to_abstime) if lockstep scheduler is used
	return lockstep_scheduler.get_absolute_time();

#else // defined(ENABLE_LOCKSTEP_SCHEDULER)
	struct timespec ts;
	px4_clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts_to_abstime(&ts);
#endif // defined(ENABLE_LOCKSTEP_SCHEDULER)
}

void hrt_store_absolute_time(volatile hrt_abstime *t)
{
	*t = hrt_absolute_time();
}

bool hrt_called(struct hrt_call *entry)
{
	return (entry->deadline == 0);
}

void hrt_cancel(struct hrt_call *entry)
{
	hrt_lock();
	sq_rem(&entry->link, &callout_queue);
	entry->deadline = 0;

	/* if this is a periodic call being removed by the callout, prevent it from
	 * being re-entered when the callout returns.
	 */
	entry->period = 0;

	hrt_unlock();
}

static void hrt_latency_update()
{
	uint16_t latency = latency_actual - latency_baseline;
	unsigned	index;

	/* bounded buckets */
	for (index = 0; index < LATENCY_BUCKET_COUNT; index++) {
		if (latency <= latency_buckets[index]) {
			latency_counters[index]++;
			return;
		}
	}

	/* catch-all at the end */
	latency_counters[index]++;
}

void hrt_call_init(struct hrt_call *entry)
{
	memset(entry, 0, sizeof(*entry));
}

void hrt_call_delay(struct hrt_call *entry, hrt_abstime delay)
{
	entry->deadline = hrt_absolute_time() + delay;
}

void hrt_init()
{
	if (!_hrt_initialized) {
		_hrt_initialized = true;
	} else {
		return;
	}

	sq_init(&callout_queue);

	int sem_ret = sem_init(&_hrt_lock, 0, 1);

	memset(&_hrt_work, 0, sizeof(_hrt_work));

	hrt_work_queue_init();
}

static void
hrt_call_enter(struct hrt_call *entry)
{
	struct hrt_call	*call, *next;

	call = (struct hrt_call *)sq_peek(&callout_queue);


	if ((call == nullptr) || (entry->deadline < call->deadline)) {
		sq_addfirst(&entry->link, &callout_queue);
		//if (call != nullptr) PX4_INFO("call enter at head, reschedule (%lu %lu)", entry->deadline, call->deadline);
		/* we changed the next deadline, reschedule the timer event */
		hrt_call_reschedule();

	} else {
		do {
			next = (struct hrt_call *)sq_next(&call->link);

			if ((next == nullptr) || (entry->deadline < next->deadline)) {
				//lldbg("call enter after head\n");
				sq_addafter(&call->link, &entry->link, &callout_queue);
				break;
			}
		} while ((call = next) != nullptr);
	}
}

/**
 * Timer interrupt handler
 *
 * This routine simulates a timer interrupt handler
 */
static void
hrt_tim_isr(void *p)
{
	/* grab the timer for latency tracking purposes */
	latency_actual = hrt_absolute_time();

	/* do latency calculations */
	hrt_latency_update();

	/* run any callouts that have met their deadline */
	hrt_call_invoke();

	hrt_lock();

	/* and schedule the next interrupt */
	hrt_call_reschedule();

	hrt_unlock();
}


/**
 * Reschedule the next timer interrupt.
 *
 * This routine must be called with interrupts disabled.
 */
static void
hrt_call_reschedule()
{
	hrt_abstime	now = hrt_absolute_time();
	hrt_abstime	delay = HRT_INTERVAL_MAX;
	struct hrt_call	*next = (struct hrt_call *)sq_peek(&callout_queue);
	hrt_abstime	deadline = now + HRT_INTERVAL_MAX;

	/*
	 * Determine what the next deadline will be.
	 *
	 * Note that we ensure that this will be within the counter
	 * period, so that when we truncate all but the low 16 bits
	 * the next time the compare matches it will be the deadline
	 * we want.
	 *
	 * It is important for accurate timekeeping that the compare
	 * interrupt fires sufficiently often that the base_time update in
	 * hrt_absolute_time runs at least once per timer period.
	 */
	if (next != nullptr) {
		//lldbg("entry in queue\n");
		if (next->deadline <= (now + HRT_INTERVAL_MIN)) {
			//lldbg("pre-expired\n");
			/* set a minimal deadline so that we call ASAP */
			delay = HRT_INTERVAL_MIN;

		} else if (next->deadline < deadline) {
			//lldbg("due soon\n");
			delay = next->deadline - now;
		}
	}

	/* set the new compare value and remember it for latency tracking */
	latency_baseline = now + delay;

	// There is no timer ISR, so simulate one by putting an event on the
	// high priority work queue

	// Remove the existing expiry and update with the new expiry
	hrt_work_cancel(&_hrt_work);

	hrt_work_queue(&_hrt_work, &hrt_tim_isr, nullptr, delay);
}

static void
hrt_call_internal(struct hrt_call *entry, hrt_abstime deadline, hrt_abstime interval, hrt_callout callout, void *arg)
{
	hrt_lock();

	//PX4_INFO("hrt_call_internal after lock");
	/* if the entry is currently queued, remove it */
	/* note that we are using a potentially uninitialised
	   entry->link here, but it is safe as sq_rem() doesn't
	   dereference the passed node unless it is found in the
	   list. So we potentially waste a bit of time searching the
	   queue for the uninitialised entry->link but we don't do
	   anything actually unsafe.
	*/
	if (entry->deadline != 0) {
		sq_rem(&entry->link, &callout_queue);
	}

#if 1

	// Use this to debug busy CPU that keeps rescheduling with 0 period time
	/*if (interval < HRT_INTERVAL_MIN) {*/
	/*PX4_ERR("hrt_call_internal interval too short: %" PRIu64, interval);*/
	/*abort();*/
	/*}*/

#endif
	entry->deadline = deadline;
	entry->period = interval;
	entry->callout = callout;
	entry->arg = arg;

	hrt_call_enter(entry);
	hrt_unlock();
}

/*
 * Call callout(arg) after delay has elapsed.
 *
 * If callout is nullptr, this can be used to implement a timeout by testing the call
 * with hrt_called().
 */
void	hrt_call_after(struct hrt_call *entry, hrt_abstime delay, hrt_callout callout, void *arg)
{
	// printf("hrt_call_after\n");
	hrt_call_internal(entry,
			  hrt_absolute_time() + delay,
			  0,
			  callout,
			  arg);
}

/*
 * Call callout(arg) after delay, and then after every interval.
 *
 * Note thet the interval is timed between scheduled, not actual, call times, so the call rate may
 * jitter but should not drift.
 */
void	hrt_call_every(struct hrt_call *entry, hrt_abstime delay, hrt_abstime interval, hrt_callout callout, void *arg)
{
	hrt_call_internal(entry,
			  hrt_absolute_time() + delay,
			  interval,
			  callout,
			  arg);
}

/*
 * Call callout(arg) at absolute time calltime.
 */
void	hrt_call_at(struct hrt_call *entry, hrt_abstime calltime, hrt_callout callout, void *arg)
{
	hrt_call_internal(entry, calltime, 0, callout, arg);
}

static void
hrt_call_invoke()
{
	struct hrt_call	*call;
	hrt_abstime deadline;

	hrt_lock();

	while (true) {
		/* get the current time */
		hrt_abstime now = hrt_absolute_time();

		call = (struct hrt_call *)sq_peek(&callout_queue);

		if (call == nullptr) {
			break;
		}

		if (call->deadline > now) {
			break;
		}

		sq_rem(&call->link, &callout_queue);
		//PX4_INFO("call pop");

		/* save the intended deadline for periodic calls */
		deadline = call->deadline;

		/* zero the deadline, as the call has occurred */
		call->deadline = 0;

		/* invoke the callout (if there is one) */
		if (call->callout) {
			// Unlock so we don't deadlock in callback
			hrt_unlock();

			//PX4_INFO("call %p: %p(%p)", call, call->callout, call->arg);
			call->callout(call->arg);

			hrt_lock();
		}

		/* if the callout has a non-zero period, it has to be re-entered */
		if (call->period != 0) {
			// re-check call->deadline to allow for
			// callouts to re-schedule themselves
			// using hrt_call_delay()
			if (call->deadline <= now) {
				call->deadline = deadline + call->period;
				//PX4_INFO("call deadline set to %lu now=%lu", call->deadline,  now);
			}

			hrt_call_enter(call);
		}
	}

	hrt_unlock();
}

int px4_clock_gettime(clockid_t clk_id, struct timespec *tp)
{
#if defined(ENABLE_LOCKSTEP_SCHEDULER)

	if (clk_id == CLOCK_MONOTONIC) {
		abstime_to_ts(tp, lockstep_scheduler.get_absolute_time());
		return 0;
	}

#endif
	return system_clock_gettime(clk_id, tp);
}

#if defined(ENABLE_LOCKSTEP_SCHEDULER)
int px4_clock_settime(clockid_t clk_id, const struct timespec *ts)
{
	if (clk_id == CLOCK_REALTIME) {
		return system_clock_settime(clk_id, ts);

	} else {
		lockstep_scheduler.set_absolute_time(ts_to_abstime(ts));
		return 0;
	}
}

int px4_usleep(useconds_t usec)
{
	if (lockstep_scheduler.get_absolute_time() == 0) {
		// Until the time is set by the simulator, we fallback to the normal usleep
		return system_usleep(usec);
	}

	const uint64_t time_finished = lockstep_scheduler.get_absolute_time() + usec;

	return lockstep_scheduler.usleep_until(time_finished);
}

unsigned int px4_sleep(unsigned int seconds)
{
	if (lockstep_scheduler.get_absolute_time() == 0) {
		// Until the time is set by the simulator, we fallback to the normal sleep
		return system_sleep(seconds);
	}

	const uint64_t time_finished = lockstep_scheduler.get_absolute_time() + ((uint64_t)seconds * 1000000);

	return lockstep_scheduler.usleep_until(time_finished);
}

int px4_pthread_cond_timedwait(pthread_cond_t *cond,
			       pthread_mutex_t *mutex,
			       const struct timespec *ts)
{
	const uint64_t scheduled = ts_to_abstime(ts);
	return lockstep_scheduler.cond_timedwait(cond, mutex, scheduled);
}

int px4_lockstep_register_component()
{
	return lockstep_scheduler.components().register_component();
}

void px4_lockstep_unregister_component(int component)
{
	lockstep_scheduler.components().unregister_component(component);
}

void px4_lockstep_progress(int component)
{
	lockstep_scheduler.components().lockstep_progress(component);
}

void px4_lockstep_wait_for_components()
{
	lockstep_scheduler.components().wait_for_components();
}
#endif

