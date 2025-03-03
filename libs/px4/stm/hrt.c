/****************************************************************************
 *
 *   Copyright (c) 2012, 2013 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#include <sys/types.h>
#include <stdbool.h>

#include <assert.h>
#include <time.h>
#include <sdqueue.h>
#include <errno.h>
#include <string.h>

#include <drivers/drv_hrt.h>

#include "./hrt_stm32h7xx.h"

#ifdef CONFIG_DEBUG_HRT
#define hrtinfo _info
#else
#define hrtinfo(x...)
#endif

#ifdef HRT_TIMER

static void hrt_tim_isr(void);

/* HRT configuration */
#if   HRT_TIMER == 1
# define HRT_TIMER_BASE		STM32_TIM1_BASE
# define HRT_TIMER_POWER_REG	STM32_RCC_APB2ENR
# define HRT_TIMER_POWER_BIT	RCC_APB2ENR_TIM1EN
# define HRT_TIMER_VECTOR	STM32_IRQ_TIM1CC
# define HRT_TIMER_CLOCK	STM32_APB2_TIM1_CLKIN
void TIM1_CC_IRQHandler() { hrt_tim_isr(); }
#elif HRT_TIMER == 2
# define HRT_TIMER_BASE		STM32_TIM2_BASE
# define HRT_TIMER_POWER_REG	STM32_RCC_APB1ENR
# define HRT_TIMER_POWER_BIT	RCC_APB1ENR_TIM2EN
# define HRT_TIMER_VECTOR	STM32_IRQ_TIM2
# define HRT_TIMER_CLOCK	STM32_APB1_TIM2_CLKIN
void TIM2_IRQHandler() { hrt_tim_isr(); }
#elif HRT_TIMER == 3
# define HRT_TIMER_BASE		STM32_TIM3_BASE
# define HRT_TIMER_POWER_REG	STM32_RCC_APB1ENR
# define HRT_TIMER_POWER_BIT	RCC_APB1ENR_TIM3EN
# define HRT_TIMER_VECTOR	STM32_IRQ_TIM3
# define HRT_TIMER_CLOCK	STM32_APB1_TIM3_CLKIN
void TIM3_IRQHandler() { hrt_tim_isr(); }
#elif HRT_TIMER == 4
# define HRT_TIMER_BASE		STM32_TIM4_BASE
# define HRT_TIMER_POWER_REG	STM32_RCC_APB1ENR
# define HRT_TIMER_POWER_BIT	RCC_APB1ENR_TIM4EN
# define HRT_TIMER_VECTOR	STM32_IRQ_TIM4
# define HRT_TIMER_CLOCK	STM32_APB1_TIM4_CLKIN
void TIM4_IRQHandler() { hrt_tim_isr(); }
#elif HRT_TIMER == 5
# define HRT_TIMER_BASE		STM32_TIM5_BASE
# define HRT_TIMER_POWER_REG	STM32_RCC_APB1ENR
# define HRT_TIMER_POWER_BIT	RCC_APB1ENR_TIM5EN
# define HRT_TIMER_VECTOR	STM32_IRQ_TIM5
# define HRT_TIMER_CLOCK	STM32_APB1_TIM5_CLKIN
void TIM5_IRQHandler() { hrt_tim_isr(); }
#elif HRT_TIMER == 8
# define HRT_TIMER_BASE		STM32_TIM8_BASE
# define HRT_TIMER_POWER_REG	STM32_RCC_APB2ENR
# define HRT_TIMER_POWER_BIT	RCC_APB2ENR_TIM8EN
# define HRT_TIMER_VECTOR	STM32_IRQ_TIM8CC
# define HRT_TIMER_CLOCK	STM32_APB2_TIM8_CLKIN
void TIM8_CC_IRQHandler() { hrt_tim_isr(); }
#elif HRT_TIMER == 9
# error not support HRT TIMER 9
#elif HRT_TIMER == 10
# error not support HRT TIMER 10
#elif HRT_TIMER == 11
# error not support HRT TIMER 11
#else
# error not support HRT TIMER 12
#endif

/*
 * HRT clock must be a multiple of 1MHz greater than 1MHz
 */
#if (HRT_TIMER_CLOCK % 1000000) != 0
# error HRT_TIMER_CLOCK must be a multiple of 1MHz
#endif
#if ((HRT_TIMER_CLOCK) <= 1000000)
# error HRT_TIMER_CLOCK must be greater than 1MHz
#endif

/*
 * HRT clock must be a multiple of 1MHz greater than 1MHz
 */
/**
 * Minimum/maximum deadlines.
 *
 * These are suitable for use with a 16-bit timer/counter clocked
 * at 1MHz.  The high-resolution timer need only guarantee that it
 * not wrap more than once in the 50ms period for absolute time to
 * be consistently maintained.
 *
 * The minimum deadline must be such that the time taken between
 * reading a time and writing a deadline to the timer cannot
 * result in missing the deadline.
 */
#define HRT_INTERVAL_MIN	50
#define HRT_INTERVAL_MAX	50000

/*
 * Period of the free-running counter, in microseconds.
 */
#define HRT_COUNTER_PERIOD	65536

/*
 * Scaling factor(s) for the free-running counter; convert an input
 * in counts to a time in microseconds.
 */
#define HRT_COUNTER_SCALE(_c)	(_c)

/*
 * Timer register accessors
 */
#define REG(_reg)	(*(volatile uint32_t *)(HRT_TIMER_BASE + _reg))

#define rCR1     	REG(STM32_GTIM_CR1_OFFSET)
#define rCR2     	REG(STM32_GTIM_CR2_OFFSET)
#define rSMCR    	REG(STM32_GTIM_SMCR_OFFSET)
#define rDIER    	REG(STM32_GTIM_DIER_OFFSET)
#define rSR      	REG(STM32_GTIM_SR_OFFSET)
#define rEGR     	REG(STM32_GTIM_EGR_OFFSET)
#define rCCMR1   	REG(STM32_GTIM_CCMR1_OFFSET)
#define rCCMR2   	REG(STM32_GTIM_CCMR2_OFFSET)
#define rCCER    	REG(STM32_GTIM_CCER_OFFSET)
#define rCNT     	REG(STM32_GTIM_CNT_OFFSET)
#define rPSC     	REG(STM32_GTIM_PSC_OFFSET)
#define rARR     	REG(STM32_GTIM_ARR_OFFSET)
#define rCCR1    	REG(STM32_GTIM_CCR1_OFFSET)
#define rCCR2    	REG(STM32_GTIM_CCR2_OFFSET)
#define rCCR3    	REG(STM32_GTIM_CCR3_OFFSET)
#define rCCR4    	REG(STM32_GTIM_CCR4_OFFSET)
#define rDCR     	REG(STM32_GTIM_DCR_OFFSET)
#define rDMAR    	REG(STM32_GTIM_DMAR_OFFSET)

/*
 * Specific registers and bits used by HRT sub-functions
 */
/* FIXME! There is an interaction in the CCMR registers that prevents using Chan 1 as the timer and chan 2 as the PPM*/
#if HRT_TIMER_CHANNEL == 1
# define rCCR_HRT	rCCR1			/* compare register for HRT */
# define DIER_HRT	GTIM_DIER_CC1IE		/* interrupt enable for HRT */
# define SR_INT_HRT	GTIM_SR_CC1IF		/* interrupt status for HRT */
#elif HRT_TIMER_CHANNEL == 2
# define rCCR_HRT	rCCR2			/* compare register for HRT */
# define DIER_HRT	GTIM_DIER_CC2IE		/* interrupt enable for HRT */
# define SR_INT_HRT	GTIM_SR_CC2IF		/* interrupt status for HRT */
#elif HRT_TIMER_CHANNEL == 3
# define rCCR_HRT	rCCR3			/* compare register for HRT */
# define DIER_HRT	GTIM_DIER_CC3IE		/* interrupt enable for HRT */
# define SR_INT_HRT	GTIM_SR_CC3IF		/* interrupt status for HRT */
#elif HRT_TIMER_CHANNEL == 4
# define rCCR_HRT	rCCR4			/* compare register for HRT */
# define DIER_HRT	GTIM_DIER_CC4IE		/* interrupt enable for HRT */
# define SR_INT_HRT	GTIM_SR_CC4IF		/* interrupt status for HRT */
#else
# error HRT_TIMER_CHANNEL must be a value between 1 and 4
#endif

/*
 * Queue of callout entries.
 */
static struct sq_queue_s	callout_queue;

/* latency baseline (last compare value applied) */
static uint16_t			latency_baseline;

/* timer count at interrupt (for latency purposes) */
static uint16_t			latency_actual;

/* latency histogram */
const uint16_t latency_bucket_count = LATENCY_BUCKET_COUNT;
const uint16_t latency_buckets[LATENCY_BUCKET_COUNT] = { 1, 2, 5, 10, 20, 50, 100, 1000 };
__EXPORT uint32_t latency_counters[LATENCY_BUCKET_COUNT + 1];

/* timer-specific functions */
static void		hrt_tim_init(void);
static void		hrt_latency_update(void);

/* callout list manipulation */
static void		hrt_call_internal(struct hrt_call *entry,
		hrt_abstime deadline,
		hrt_abstime interval,
		hrt_callout callout,
		void *arg);
static void		hrt_call_enter(struct hrt_call *entry);
static void		hrt_call_reschedule(void);
static void		hrt_call_invoke(void);

/* disable the PPM configuration */
# define rCCR_PPM	0
# define DIER_PPM	0
# define SR_INT_PPM	0
# define SR_OVF_PPM	0
# define CCMR1_PPM	0
# define CCMR2_PPM	0
# define CCER_PPM	0

static void hrt_tim_init(void)
{
	/* clock/power on our timer */
	modifyreg32(HRT_TIMER_POWER_REG, 0, HRT_TIMER_POWER_BIT);

	/* disable and configure the timer */
	rCR1 = 0;
	rCR2 = 0;
	rSMCR = 0;
	rDIER = DIER_HRT | DIER_PPM;
	rCCER = 0;		/* unlock CCMR* registers */
	rCCMR1 = CCMR1_PPM;
	rCCMR2 = CCMR2_PPM;
	rCCER = CCER_PPM;
	rDCR = 0;

	/* configure the timer to free-run at 1MHz */
	rPSC = (HRT_TIMER_CLOCK / 1000000) - 1;	/* this really only works for whole-MHz clocks */

	/* run the full span of the counter */
	rARR = 0xffff;

	/* set an initial capture a little ways off */
	rCCR_HRT = 1000;

	/* generate an update event; reloads the counter, all registers */
	rEGR = GTIM_EGR_UG;

	/* enable the timer */
	rCR1 = GTIM_CR1_CEN;

	/* enable interrupts */
	up_enable_irq(HRT_TIMER_VECTOR);
}

static void hrt_tim_isr()
{
	uint32_t status;

	/* grab the timer for latency tracking purposes */
	latency_actual = rCNT;

	/* copy interrupt status */
	status = rSR;

	/* ack the interrupts we just read */
	rSR = ~status;

	/* was this a timer tick? */
	if (status & SR_INT_HRT) {

		/* do latency calculations */
		hrt_latency_update();

		/* run any callouts that have met their deadline */
		hrt_call_invoke();

		/* and schedule the next interrupt */
		hrt_call_reschedule();
	}
}

/**
 * Fetch a never-wrapping absolute time value in microseconds from
 * some arbitrary epoch shortly after system start.
 */
hrt_abstime hrt_absolute_time(void)
{
	hrt_abstime	abstime;
	uint32_t	count;
	UBaseType_t	flags;

	/*
	 * Counter state.  Marked volatile as they may change
	 * inside this routine but outside the irqsave/restore
	 * pair.  Discourage the compiler from moving loads/stores
	 * to these outside of the protected range.
	 */
	static volatile hrt_abstime base_time;
	static volatile uint32_t last_count;

	/* prevent re-entry */
	flags = portSET_INTERRUPT_MASK_FROM_ISR();

	/* get the current counter value */
	count = rCNT;

	/*
	 * Determine whether the counter has wrapped since the
	 * last time we're called.
	 *
	 * This simple test is sufficient due to the guarantee that
	 * we are always called at least once per counter period.
	 */
	if (count < last_count) {
		base_time += HRT_COUNTER_PERIOD;
	}

	/* save the count for next time */
	last_count = count;

	/* compute the current time */
	abstime = HRT_COUNTER_SCALE(base_time + count);

	portCLEAR_INTERRUPT_MASK_FROM_ISR(flags);

	return abstime;
}

hrt_abstime ts_to_abstime(const struct timespec *ts)
{
	hrt_abstime	result;

	result = (hrt_abstime)(ts->tv_sec) * 1000000;
	result += ts->tv_nsec / 1000;

	return result;
}

void abstime_to_ts(struct timespec *ts, hrt_abstime abstime)
{
	ts->tv_sec = abstime / 1000000;
	abstime -= ts->tv_sec * 1000000;
	ts->tv_nsec = abstime * 1000;
}

hrt_abstime hrt_elapsed_time_atomic(const volatile hrt_abstime *then)
{
	UBaseType_t flags = portSET_INTERRUPT_MASK_FROM_ISR();

    hrt_abstime delta = hrt_absolute_time() - *then;

	portCLEAR_INTERRUPT_MASK_FROM_ISR(flags);

	return delta;
}

hrt_abstime hrt_store_absolute_time(volatile hrt_abstime *now)
{
	UBaseType_t flags = portSET_INTERRUPT_MASK_FROM_ISR();

	hrt_abstime ts = hrt_absolute_time();

	portCLEAR_INTERRUPT_MASK_FROM_ISR(flags);

	return ts;
}

void hrt_init(void)
{
	sq_init(&callout_queue);
	hrt_tim_init();
}

void hrt_call_after(struct hrt_call *entry, hrt_abstime delay, hrt_callout callout, void *arg)
{
	hrt_call_internal(entry,
                hrt_absolute_time() + delay,
                0,
                callout,
                arg);
}

void hrt_call_at(struct hrt_call *entry, hrt_abstime calltime, hrt_callout callout, void *arg)
{
	hrt_call_internal(entry, calltime, 0, callout, arg);
}

void hrt_call_every(struct hrt_call *entry, hrt_abstime delay, hrt_abstime interval, hrt_callout callout, void *arg)
{
	hrt_call_internal(entry,
                hrt_absolute_time() + delay,
                interval,
                callout,
                arg);
}

static void hrt_call_internal(struct hrt_call *entry, hrt_abstime deadline, hrt_abstime interval, hrt_callout callout, void *arg)
{
    UBaseType_t flags = portSET_INTERRUPT_MASK_FROM_ISR();

	/* if the entry is currently queued, remove it */
	/*  note that we are using a potentially uninitialised
        entry->link here, but it is safe as sq_rem() doesn't
        dereference the passed node unless it is found in the
        list. So we potentially waste a bit of time searching the
        queue for the uninitialised entry->link but we don't do
        anything actually unsafe.
	*/
	if (entry->deadline != 0) {
		sq_rem(&entry->link, &callout_queue);
	}

	entry->deadline = deadline;
	entry->period = interval;
	entry->callout = callout;
	entry->arg = arg;

	hrt_call_enter(entry);

	portCLEAR_INTERRUPT_MASK_FROM_ISR(flags);
}

bool hrt_called(struct hrt_call *entry)
{
	return (entry->deadline == 0);
}

void hrt_cancel(struct hrt_call *entry)
{
    UBaseType_t flags = portSET_INTERRUPT_MASK_FROM_ISR();

	sq_rem(&entry->link, &callout_queue);
	entry->deadline = 0;

	/* if this is a periodic call being removed by the callout, prevent it from
	 * being re-entered when the callout returns.
	 */
	entry->period = 0;

	portCLEAR_INTERRUPT_MASK_FROM_ISR(flags);
}

static void hrt_call_enter(struct hrt_call *entry)
{
	struct hrt_call	*call, *next;

	call = (struct hrt_call *)sq_peek(&callout_queue);

	if ((call == NULL) || (entry->deadline < call->deadline)) {
		sq_addfirst(&entry->link, &callout_queue);
		hrtinfo("call enter at head, reschedule\n");
		/* we changed the next deadline, reschedule the timer event */
		hrt_call_reschedule();

	} else {
		do {
			next = (struct hrt_call *)sq_next(&call->link);

			if ((next == NULL) || (entry->deadline < next->deadline)) {
				hrtinfo("call enter after head\n");
				sq_addafter(&call->link, &entry->link, &callout_queue);
				break;
			}
		} while ((call = next) != NULL);
	}

	hrtinfo("scheduled\n");
}

static void hrt_call_invoke(void)
{
	struct hrt_call	*call;
	hrt_abstime deadline;

	while (true) {
		/* get the current time */
		hrt_abstime now = hrt_absolute_time();

		call = (struct hrt_call *)sq_peek(&callout_queue);

		if (call == NULL) {
			break;
		}

		if (call->deadline > now) {
			break;
		}

		sq_rem(&call->link, &callout_queue);
		hrtinfo("call pop\n");

		/* save the intended deadline for periodic calls */
		deadline = call->deadline;

		/* zero the deadline, as the call has occurred */
		call->deadline = 0;

		/* invoke the callout (if there is one) */
		if (call->callout) {
			hrtinfo("call %p: %p(%p)\n", call, call->callout, call->arg);
			call->callout(call->arg);
		}

		/* if the callout has a non-zero period, it has to be re-entered */
		if (call->period != 0) {
			// re-check call->deadline to allow for
			// callouts to re-schedule themselves
			// using hrt_call_delay()
			if (call->deadline <= now) {
				call->deadline = deadline + call->period;
			}

			hrt_call_enter(call);
		}
	}
}

static void hrt_call_reschedule()
{
	hrt_abstime	now = hrt_absolute_time();
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
	if (next != NULL) {
		hrtinfo("entry in queue\n");

		if (next->deadline <= (now + HRT_INTERVAL_MIN)) {
			hrtinfo("pre-expired\n");
			/* set a minimal deadline so that we call ASAP */
			deadline = now + HRT_INTERVAL_MIN;

		} else if (next->deadline < deadline) {
			hrtinfo("due soon\n");
			deadline = next->deadline;
		}
	}

	hrtinfo("schedule for %u at %u\n", (unsigned)(deadline & 0xffffffff), (unsigned)(now & 0xffffffff));

	/* set the new compare value and remember it for latency tracking */
	rCCR_HRT = latency_baseline = deadline & 0xffff;
}

static void hrt_latency_update(void)
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

#endif /* HRT_TIMER */
