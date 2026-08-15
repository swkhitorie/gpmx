
include ${GPMPATH}/libs/libc/pthread/config.mk
include ${GPMPATH}/libs/libc/sched/config.mk
include ${GPMPATH}/libs/libc/mqueue/config.mk
include ${GPMPATH}/libs/libc/queue/config.mk
include ${GPMPATH}/libs/libc/semaphore/config.mk
include ${GPMPATH}/libs/libc/timer/config.mk
include ${GPMPATH}/libs/libc/time/config.mk
include ${GPMPATH}/libs/libc/stdio/config.mk
include ${GPMPATH}/libs/libc/string/config.mk
include ${GPMPATH}/libs/libc/libgen/config.mk
include ${GPMPATH}/libs/libc/lzf/config.mk
include ${GPMPATH}/libs/libc/dirent/config.mk
include ${GPMPATH}/libs/libc/unistd/config.mk

ifneq (${CONFIG_POSIXRUN_ENABLE},y)
CSOURCES += ${GPMPATH}/libs/libc/utils/compile_utils.c
endif
