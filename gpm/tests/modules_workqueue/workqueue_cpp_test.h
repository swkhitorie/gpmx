#ifndef WQUEUE_CPP_TEST_H_
#define WQUEUE_CPP_TEST_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int wqueue_cpp_workitem_test(int argc, char **argv);
int wqueue_cpp_scheduled_test(int argc, char **argv);
int wqueue_cpp_singleshot_test(int argc, char **argv);
int wqueue_cpp_all_test(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif
