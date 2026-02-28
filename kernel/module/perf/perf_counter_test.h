#ifndef PERF_COUNTER_H_
#define PERF_COUNTER_H_

#include <board_config.h>

#ifndef TEST_PRINTF
#define TEST_PRINTF    BOARD_PRINTF
#endif

#ifdef __cplusplus
extern "C" {
#endif

int perf_counter_test(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif

