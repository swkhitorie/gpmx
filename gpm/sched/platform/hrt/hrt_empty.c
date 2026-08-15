#include <errno.h>
#include <string.h>
#include <time.h>
#include <driver/drv_hrt.h>

/* latency baseline (last compare value applied) */
static uint64_t			latency_baseline;

/* timer count at interrupt (for latency purposes) */
static uint64_t			latency_actual;

/* latency histogram */
const uint16_t latency_bucket_count = LATENCY_BUCKET_COUNT;
const uint16_t latency_buckets[LATENCY_BUCKET_COUNT] = { 1, 2, 5, 10, 20, 50, 100, 1000 };
uint32_t latency_counters[LATENCY_BUCKET_COUNT + 1];

hrt_abstime hrt_absolute_time()
{

	return 0;
}

void hrt_store_absolute_time(volatile hrt_abstime *t)
{
	*t = hrt_absolute_time();
}

bool hrt_called(struct hrt_call *entry)
{

}

void hrt_cancel(struct hrt_call *entry)
{

}

void hrt_call_init(struct hrt_call *entry)
{

}

void hrt_call_delay(struct hrt_call *entry, hrt_abstime delay)
{

}

void hrt_init()
{

}

void	hrt_call_after(struct hrt_call *entry, hrt_abstime delay, hrt_callout callout, void *arg)
{
}

void	hrt_call_every(struct hrt_call *entry, hrt_abstime delay, hrt_abstime interval, hrt_callout callout, void *arg)
{
}

void	hrt_call_at(struct hrt_call *entry, hrt_abstime calltime, hrt_callout callout, void *arg)
{

}

