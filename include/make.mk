
FR_CINCDIRS += include/fr
FR_CINCDIRS += include/dev

ifeq (${CONFIG_FR_LIB_POSIX},y)
FR_CINCDIRS += include
endif
