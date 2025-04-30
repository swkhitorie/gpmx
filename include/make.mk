
FR_CINCDIRS += include/

ifeq (${CONFIG_MK_USE_FREERTOS},y)
FR_CINCDIRS += include/fr
endif

ifeq (${CONFIG_MK_USE_FR_POSIX},y)
FR_CINCDIRS += include/posix
endif
