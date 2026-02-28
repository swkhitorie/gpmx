#ifndef FAULT_TEST_CMBACKTRACE_H_
#define FAULT_TEST_CMBACKTRACE_H_

#include <board_config.h>

#ifndef TEST_PRINTF
#define TEST_PRINTF    BOARD_PRINTF
#endif

#ifdef __cplusplus
extern "C" {
#endif

int fault_test_by_unalign(int argc, char **argv);
int fault_test_by_div0(int argc, char **argv);

#ifdef __cplusplus
}
#endif


#endif
