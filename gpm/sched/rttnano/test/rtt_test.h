#ifndef _RTTNANO_TEST_INCLUDE_H_
#define _RTTNANO_TEST_INCLUDE_H_

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

int rtt_event_test(int argc, char **argv);
int rtt_idlehook_test(int argc, char **argv);
int rtt_irq_test(int argc, char **argv);
int rtt_mailbox_test(int argc, char **argv);
int rtt_mem_test(int argc, char **argv);
int rtt_mempool_test(int argc, char **argv);
int rtt_messagequeue_test(int argc, char **argv);
int rtt_mutex_test(int argc, char **argv);
int rtt_priority_reverse_test(int argc, char **argv);
int rtt_producer_consumer_test(int argc, char **argv);
int rtt_schedulerhook_test(int argc, char **argv);
int rtt_sem_test(int argc, char **argv);
int rtt_signal_test(int argc, char **argv);
int rtt_thread_test(int argc, char **argv);
int rtt_timer_test(int argc, char **argv);
int rtt_timeslice_test(int argc, char **argv);

#ifdef __cplusplus
}
#endif


#endif

