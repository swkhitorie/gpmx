/**
 * @file perf_counter.h
 * Performance measuring tools.
 */

#ifndef PERF_COUNTER_H
#define PERF_COUNTER_H

#include <stdint.h>

enum perf_counter_type {
	PC_COUNT,		/**< count the number of times an event occurs */
	PC_ELAPSED,		/**< measure the time elapsed performing an event */
	PC_INTERVAL		/**< measure the interval between instances of an event */
};

struct perf_ctr_header;
typedef struct perf_ctr_header *perf_counter_t;

#ifdef __cplusplus
extern "C" {
#endif

perf_counter_t perf_alloc(enum perf_counter_type type, const char *name);
perf_counter_t perf_alloc_once(enum perf_counter_type type, const char *name);
void perf_free(perf_counter_t handle);
void perf_count(perf_counter_t handle);
void perf_begin(perf_counter_t handle);
void perf_end(perf_counter_t handle);
void perf_set_elapsed(perf_counter_t handle, int64_t elapsed);
void perf_count_interval(perf_counter_t handle, uint64_t time);
void perf_set_count(perf_counter_t handle, uint64_t count);
void perf_cancel(perf_counter_t handle);
void perf_reset(perf_counter_t handle);
void perf_print_counter(perf_counter_t handle);
int perf_print_counter_buffer(char *buffer, int length, perf_counter_t handle);
void perf_print_all(void);

typedef void (*perf_callback)(perf_counter_t handle, void *user);
void perf_iterate_all(perf_callback cb, void *user);

void perf_print_latency(void);
void perf_reset_all(void);
uint64_t perf_event_count(perf_counter_t handle);
float perf_mean(perf_counter_t handle);

#ifdef __cplusplus
}
#endif

#endif
