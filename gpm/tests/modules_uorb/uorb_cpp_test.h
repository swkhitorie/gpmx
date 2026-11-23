#ifndef UORB_CPP_TEST_H_
#define UORB_CPP_TEST_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int uorb_cpp_pubsub_test(int argc, char **argv);
int uorb_cpp_callback_test(int argc, char **argv);
int uorb_cpp_multi_test(int argc, char **argv);
int uorb_cpp_blocking_test(int argc, char **argv);
int uorb_cpp_all_test(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif
