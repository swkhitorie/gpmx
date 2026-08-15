ifeq (${CONFIG_LIBC_MQUEUE},y)

ifeq (${CONFIG_LIBC_TIME},n)
$(error CONFIG_LIBC_MQUEUE depend on CONFIG_LIBC_TIME)
endif

CSOURCES += ${GPMPATH}/libs/libc/mqueue/mq_close.c
CSOURCES += ${GPMPATH}/libs/libc/mqueue/mq_getattr.c
CSOURCES += ${GPMPATH}/libs/libc/mqueue/mq_open.c
CSOURCES += ${GPMPATH}/libs/libc/mqueue/mq_receive.c
CSOURCES += ${GPMPATH}/libs/libc/mqueue/mq_send.c
CSOURCES += ${GPMPATH}/libs/libc/mqueue/mq_setattr.c
CSOURCES += ${GPMPATH}/libs/libc/mqueue/mq_timedreceive.c
CSOURCES += ${GPMPATH}/libs/libc/mqueue/mq_timedsend.c
CSOURCES += ${GPMPATH}/libs/libc/mqueue/mq_unlink.c
CSOURCES += ${GPMPATH}/libs/libc/mqueue/prv_mqueue.c
endif
