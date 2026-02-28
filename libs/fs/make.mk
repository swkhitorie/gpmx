
ifeq (${MK_USE_FS_FATFS},y)
PROJ_CDEFS += CONFIG_FATFS_ENABLE
PROJ_CINCDIRS += libs/fs/fat/
CSOURCES += libs/fs/fat/ff.c
CSOURCES += libs/fs/fat/diskio.c
CSOURCES += libs/fs/fat/ff_drv.c
CSOURCES += libs/fs/fat/ffsystem.c
CSOURCES += libs/fs/fat/ffunicode.c

PROJ_CINCDIRS += libs/fs/fat/test
CSOURCES += libs/fs/fat/test/fatfs_test.c
endif

ifeq (${MK_USE_FS_LITTLEFS},y)
PROJ_CDEFS += CONFIG_LFS_ENABLE
PROJ_CINCDIRS += libs/fs/littlefs/
CSOURCES += libs/fs/littlefs/lfs_util.c
CSOURCES += libs/fs/littlefs/lfs.c
CSOURCES += libs/fs/littlefs/lfs_drv.c

PROJ_CINCDIRS += libs/fs/littlefs/test
CSOURCES += libs/fs/littlefs/test/lfs_test.c
endif

ifeq (${MK_USE_FS_ROMFS},y)
PROJ_CDEFS += CONFIG_ROMFS_ENABLE
PROJ_CINCDIRS += libs/fs/romfs/
CSOURCES += libs/fs/romfs/gromfs.c

PROJ_CINCDIRS += libs/fs/romfs/test
CSOURCES += libs/fs/romfs/test/romfs_test_data.c
CSOURCES += libs/fs/romfs/test/romfs_test.c
endif

