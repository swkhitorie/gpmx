#ifndef WQUEUE_TEST_H_
#define WQUEUE_TEST_H_

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

void wqueue_test_init();
int wqueue_main();

int wqueue_test(int argc, char **argv);

#ifdef __cplusplus
}
#endif



#endif
