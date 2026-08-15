
ifeq (${CONFIG_LIBC_VFS},y)
CSOURCES += ${GPMPATH}/drivers/bch/bchdev_driver.c
CSOURCES += ${GPMPATH}/drivers/bch/bchdev_register.c
CSOURCES += ${GPMPATH}/drivers/bch/bchdev_unregister.c
CSOURCES += ${GPMPATH}/drivers/bch/bchlib_cache.c
CSOURCES += ${GPMPATH}/drivers/bch/bchlib_read.c
CSOURCES += ${GPMPATH}/drivers/bch/bchlib_sem.c
CSOURCES += ${GPMPATH}/drivers/bch/bchlib_setup.c
CSOURCES += ${GPMPATH}/drivers/bch/bchlib_teardown.c
CSOURCES += ${GPMPATH}/drivers/bch/bchlib_write.c
endif
