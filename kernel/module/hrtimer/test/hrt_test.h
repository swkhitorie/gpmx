#ifndef HRTIMER_TEST_H_
#define HRTIMER_TEST_H_

#include <board_config.h>

#ifndef TEST_PRINTF
#define TEST_PRINTF    BOARD_PRINTF
#endif

#ifdef __cplusplus
extern "C" {
#endif

int drv_hrt_test(int argc, char **argv);

#ifdef __cplusplus
}
#endif


#endif
