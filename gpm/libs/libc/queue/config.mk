ifeq (${CONFIG_LIBC_QUEUE},y)
CSOURCES += ${GPMPATH}/libs/libc/queue/dq_addafter.c
CSOURCES += ${GPMPATH}/libs/libc/queue/dq_addbefore.c
CSOURCES += ${GPMPATH}/libs/libc/queue/dq_addfirst.c
CSOURCES += ${GPMPATH}/libs/libc/queue/dq_addlast.c
CSOURCES += ${GPMPATH}/libs/libc/queue/dq_cat.c
CSOURCES += ${GPMPATH}/libs/libc/queue/dq_count.c
CSOURCES += ${GPMPATH}/libs/libc/queue/dq_rem.c
CSOURCES += ${GPMPATH}/libs/libc/queue/dq_remfirst.c
CSOURCES += ${GPMPATH}/libs/libc/queue/dq_remlast.c
CSOURCES += ${GPMPATH}/libs/libc/queue/sq_addafter.c
CSOURCES += ${GPMPATH}/libs/libc/queue/sq_addfirst.c
CSOURCES += ${GPMPATH}/libs/libc/queue/sq_addlast.c
CSOURCES += ${GPMPATH}/libs/libc/queue/sq_cat.c
CSOURCES += ${GPMPATH}/libs/libc/queue/sq_count.c
CSOURCES += ${GPMPATH}/libs/libc/queue/sq_rem.c
CSOURCES += ${GPMPATH}/libs/libc/queue/sq_remafter.c
CSOURCES += ${GPMPATH}/libs/libc/queue/sq_remfirst.c
CSOURCES += ${GPMPATH}/libs/libc/queue/sq_remlast.c
endif
