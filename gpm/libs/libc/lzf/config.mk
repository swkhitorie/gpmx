
ifeq (${CONFIG_LIBC_LZF},y)
CSOURCES += ${GPMPATH}/libs/libc/lzf/lzf_c.c
CSOURCES += ${GPMPATH}/libs/libc/lzf/lzf_d.c
endif
