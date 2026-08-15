ifeq (${CONFIG_TEST},y)
PROJ_CINCDIRS += ${GPMPATH}/sched/rttnano/test
CSOURCES += ${GPMPATH}/sched/rttnano/test/rtt_thread_test.c
CSOURCES += ${GPMPATH}/sched/rttnano/test/rtt_timer_test.c
CSOURCES += ${GPMPATH}/sched/rttnano/test/rtt_priority_reverse_test.c
CSOURCES += ${GPMPATH}/sched/rttnano/test/rtt_sem_test.c
CSOURCES += ${GPMPATH}/sched/rttnano/test/rtt_mutex_test.c
CSOURCES += ${GPMPATH}/sched/rttnano/test/rtt_event_test.c
CSOURCES += ${GPMPATH}/sched/rttnano/test/rtt_mailbox_test.c
CSOURCES += ${GPMPATH}/sched/rttnano/test/rtt_messagequeue_test.c
CSOURCES += ${GPMPATH}/sched/rttnano/test/rtt_signal_test.c
CSOURCES += ${GPMPATH}/sched/rttnano/test/rtt_mem_test.c
CSOURCES += ${GPMPATH}/sched/rttnano/test/rtt_timeslice_test.c
CSOURCES += ${GPMPATH}/sched/rttnano/test/rtt_irq_test.c
CSOURCES += ${GPMPATH}/sched/rttnano/test/rtt_producer_consumer_test.c
CSOURCES += ${GPMPATH}/sched/rttnano/test/rtt_schedulerhook_test.c
CSOURCES += ${GPMPATH}/sched/rttnano/test/rtt_idlehook_test.c
CSOURCES += ${GPMPATH}/sched/rttnano/test/rtt_mempool_test.c
endif
