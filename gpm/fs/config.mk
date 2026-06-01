
#PROJ_CINCDIRS += ${GPMPATH}/sched/freertos/include

PROJ_CINCDIRS += ${GPMPATH}/fs
CSOURCES += ${GPMPATH}/fs/fs_initialize.c

include ${GPMPATH}/fs/driver/config.mk
include ${GPMPATH}/fs/inode/config.mk
include ${GPMPATH}/fs/vfs/config.mk
