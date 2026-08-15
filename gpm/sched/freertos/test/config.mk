
ifeq (${CONFIG_TEST},y)
PROJ_CINCDIRS += ${GPMPATH}/sched/freertos/test
CSOURCES += ${GPMPATH}/sched/freertos/test/frtos_mutex_test.c
CSOURCES += ${GPMPATH}/sched/freertos/test/frtos_priority_reverse_test.c
CSOURCES += ${GPMPATH}/sched/freertos/test/frtos_queue_test.c
CSOURCES += ${GPMPATH}/sched/freertos/test/frtos_sem_test.c
CSOURCES += ${GPMPATH}/sched/freertos/test/frtos_tasks_test.c
CSOURCES += ${GPMPATH}/sched/freertos/test/frtos_timer_test.c
CSOURCES += ${GPMPATH}/sched/freertos/test/frtos_status_test.c
CSOURCES += ${GPMPATH}/sched/freertos/test/frtos_eventgroup_test.c
CSOURCES += ${GPMPATH}/sched/freertos/test/frtos_streambuffer_test.c
CSOURCES += ${GPMPATH}/sched/freertos/test/frtos_messagebuffer_test.c
endif
