ifeq (${MK_USE_KERNEL_POSIX_MQUEUE},y)
PROJ_CDEFS += CONFIG_MODULE_KPOSIX_MQUEUE
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

MK_USE_KERNEL_POSIX_TIME:=y
endif
