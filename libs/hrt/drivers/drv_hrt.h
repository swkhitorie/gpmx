/**
 * @file drv_hrt.h
 *
 * High-resolution timer with callouts and timekeeping.
 */

#pragma once

#include <sys/types.h>
#include <stdbool.h>
#include <inttypes.h>
#include <sdqueue.h>

#ifndef __EXPORT
#define __EXPORT __attribute__ ((visibility ("default")))
#endif

#if defined(__cplusplus)
extern "C"{
#endif

/**
 * Absolute time, in microsecond units.
 *
 * Absolute time is measured from some arbitrary epoch shortly after
 * system startup.  It should never wrap or go backwards.
 */
typedef uint64_t	hrt_abstime;

/**
 * Callout function type.
 *
 * Note that callouts run in the timer interrupt context, so
 * they are serialised with respect to each other, and must not
 * block.
 */
typedef void	(* hrt_callout)(void *arg);

/**
 * Callout record.
 */
typedef struct hrt_call {
    struct sq_entry_s	link;

    hrt_abstime		deadline;
    hrt_abstime		period;
    hrt_callout		callout;
    void			*arg;
} *hrt_call_t;


#define LATENCY_BUCKET_COUNT 8
extern const uint16_t latency_bucket_count;
extern const uint16_t latency_buckets[LATENCY_BUCKET_COUNT];
extern uint32_t latency_counters[LATENCY_BUCKET_COUNT + 1];
typedef struct latency_info {
	uint16_t                bucket;
	uint32_t                counter;
} latency_info_t;

/**
 * Get absolute time in [us] (does not wrap).
 */
__EXPORT extern hrt_abstime hrt_absolute_time(void);

/**
 * Convert a timespec to absolute time.
 */
static inline hrt_abstime ts_to_abstime(const struct timespec *ts)
{
	hrt_abstime	result;

	result = (hrt_abstime)(ts->tv_sec) * 1000000;
	result += (hrt_abstime)(ts->tv_nsec / 1000);
	return result;
}
/**
 * Convert absolute time to a timespec.
 */
static inline void abstime_to_ts(struct timespec *ts, hrt_abstime abstime)
{
	ts->tv_sec = (typeof(ts->tv_sec))(abstime / 1000000);
	abstime -= (hrt_abstime)(ts->tv_sec) * 1000000;
	ts->tv_nsec = (typeof(ts->tv_nsec))(abstime * 1000);
}

/**
 * Compute the delta between a timestamp taken in the past
 * and now.
 *
 * This function is not interrupt save.
 */
static inline hrt_abstime hrt_elapsed_time(const hrt_abstime *then)
{
	hrt_abstime now = hrt_absolute_time();
	if (*then > now) {
		return 0;
	}
	return now - *then;
}

/**
 * Compute the delta between a timestamp taken in the past
 * and now.
 *
 * This function is safe to use even if the timestamp is updated
 * by an interrupt during execution.
 */
__EXPORT extern hrt_abstime hrt_elapsed_time_atomic(const volatile hrt_abstime *then);

/**
 * Store the absolute time in an interrupt-safe fashion.
 *
 * This function ensures that the timestamp cannot be seen half-written by an interrupt handler.
 */
__EXPORT extern void hrt_store_absolute_time(volatile hrt_abstime *time);

/**
 * Call callout(arg) after delay has elapsed.
 *
 * If callout is NULL, this can be used to implement a timeout by testing the call
 * with hrt_called().
 */
__EXPORT extern void	hrt_call_after(struct hrt_call *entry, hrt_abstime delay, hrt_callout callout, void *arg);

/**
 * Call callout(arg) at absolute time calltime.
 */
__EXPORT extern void	hrt_call_at(struct hrt_call *entry, hrt_abstime calltime, hrt_callout callout, void *arg);

/**
 * Call callout(arg) after delay, and then after every interval.
 *
 * Note thet the interval is timed between scheduled, not actual, call times, so the call rate may
 * jitter but should not drift.
 */
__EXPORT extern void	hrt_call_every(struct hrt_call *entry, hrt_abstime delay, hrt_abstime interval,
                    hrt_callout callout, void *arg);

/**
 * If this returns true, the entry has been invoked and removed from the callout list,
 * or it has never been entered.
 *
 * Always returns false for repeating callouts.
 */
__EXPORT extern bool	hrt_called(struct hrt_call *entry);

/**
 * Remove the entry from the callout list.
 */
__EXPORT extern void	hrt_cancel(struct hrt_call *entry);

/**
 * Initialise a hrt_call structure
 */
__EXPORT extern void	hrt_call_init(struct hrt_call *entry);

/*
* delay a hrt_call_every() periodic call by the given number of
* microseconds. This should be called from within the callout to
* cause the callout to be re-scheduled for a later time. The periodic
* callouts will then continue from that new base time at the
* previously specified period.
*/
__EXPORT extern void	hrt_call_delay(struct hrt_call *entry, hrt_abstime delay);

/*
* Initialise the HRT.
*/
__EXPORT extern void	hrt_init(void);

#if defined(__cplusplus)
static inline uint16_t get_latency_bucket_count(void) { return LATENCY_BUCKET_COUNT; }
static inline latency_info_t get_latency(uint16_t bucket_idx, uint16_t counter_idx)
{
	latency_info_t ret = {latency_buckets[bucket_idx], latency_counters[counter_idx]};
	return ret;
}
static inline void reset_latency_counters(void)
{
	for (int i = 0; i <= get_latency_bucket_count(); i++) {
		latency_counters[i] = 0;
	}
}
#endif

#ifdef	__cplusplus

namespace time_literals
{

// User-defined integer literals for different time units.
// The base unit is hrt_abstime in microseconds

constexpr hrt_abstime operator ""_s(unsigned long long seconds)
{
	return hrt_abstime(seconds * 1000000ULL);
}

constexpr hrt_abstime operator ""_ms(unsigned long long milliseconds)
{
	return hrt_abstime(milliseconds * 1000ULL);
}

constexpr hrt_abstime operator ""_us(unsigned long long microseconds)
{
	return hrt_abstime(microseconds);
}

} /* namespace time_literals */


#endif /* __cplusplus */
