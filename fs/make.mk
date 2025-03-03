
ifeq (${CONFIG_FR_FAT_FATFS},y)
FR_CINCDIRS += fs/fat/
FR_CSOURCES += fs/fat/ff.c
FR_CSOURCES += fs/fat/diskio.c
FR_CSOURCES += fs/fat/ff_drv.c
FR_CSOURCES += fs/fat/ffsystem.c
FR_CSOURCES += fs/fat/ffunicode.c
endif

ifeq (${CONFIG_FR_FAT_FATFS},l)
FR_CINCDIRS += fs/fat/legacy
FR_CSOURCES += fs/fat/legacy/ff.c
FR_CSOURCES += fs/fat/legacy/diskio.c
FR_CSOURCES += fs/fat/legacy/ff_drv.c
FR_CSOURCES += fs/fat/legacy/option/syscall.c
FR_CSOURCES += fs/fat/legacy/option/unicode.c
endif
