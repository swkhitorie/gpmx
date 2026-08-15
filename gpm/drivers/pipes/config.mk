
# CONFIG_DEV_FIFO_SIZE=80
# CONFIG_DEV_PIPE_SIZE=80
# CONFIG_DEV_PIPE_VFS_PATH="/dev/pipes"
ifeq (${CONFIG_PIPES},y)

ifneq (${CONFIG_LIBC_VFS},y)
$(error CONFIG_PIPES depend on CONFIG_LIBC_VFS)
endif
CSOURCES += ${GPMPATH}/drivers/pipes/fifo.c
CSOURCES += ${GPMPATH}/drivers/pipes/pipe_common.c
CSOURCES += ${GPMPATH}/drivers/pipes/pipe.c
endif
