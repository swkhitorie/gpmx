ifeq (${MK_TEST_ENABLE},y)
ifeq (${MK_RTOS},frtos)
CSOURCES += ${GPMPATH}/tests/freertos/frtos_mutex_test.c
CSOURCES += ${GPMPATH}/tests/freertos/frtos_priority_reverse_test.c
CSOURCES += ${GPMPATH}/tests/freertos/frtos_queue_test.c
CSOURCES += ${GPMPATH}/tests/freertos/frtos_sem_test.c
CSOURCES += ${GPMPATH}/tests/freertos/frtos_tasks_test.c
CSOURCES += ${GPMPATH}/tests/freertos/frtos_timer_test.c
CSOURCES += ${GPMPATH}/tests/freertos/frtos_status_test.c
CSOURCES += ${GPMPATH}/tests/freertos/frtos_eventgroup_test.c
endif
endif
