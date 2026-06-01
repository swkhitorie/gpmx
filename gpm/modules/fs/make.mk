TMP_PATH=gpm/modules
ifeq (${MK_USE_FS_FATFS},y)
PROJ_CDEFS += CONFIG_FATFS_ENABLE
PROJ_CINCDIRS += ${TMP_PATH}/fs/fat/
CSOURCES += ${TMP_PATH}/fs/fat/ff.c
CSOURCES += ${TMP_PATH}/fs/fat/diskio.c
CSOURCES += ${TMP_PATH}/fs/fat/ff_drv.c
CSOURCES += ${TMP_PATH}/fs/fat/ffsystem.c
CSOURCES += ${TMP_PATH}/fs/fat/ffunicode.c

ifeq (${MK_TEST_ENABLE},y)
PROJ_CINCDIRS += ${TMP_PATH}/fs/fat/test
CSOURCES += ${TMP_PATH}/fs/fat/test/fatfs_test.c
endif

endif # end with MK_USE_FS_FATFS

ifeq (${MK_USE_FS_LITTLEFS},y)
PROJ_CDEFS += CONFIG_LFS_ENABLE
PROJ_CINCDIRS += ${TMP_PATH}/fs/littlefs/
CSOURCES += ${TMP_PATH}/fs/littlefs/lfs_util.c
CSOURCES += ${TMP_PATH}/fs/littlefs/lfs.c
CSOURCES += ${TMP_PATH}/fs/littlefs/lfs_drv.c

ifeq (${MK_TEST_ENABLE},y)
PROJ_CINCDIRS += ${TMP_PATH}/fs/littlefs/test
CSOURCES += ${TMP_PATH}/fs/littlefs/test/lfs_test.c
endif

endif # end with MK_USE_FS_LITTLEFS

ifeq (${MK_USE_FS_ROMFS},y)
PROJ_CDEFS += CONFIG_ROMFS_ENABLE
PROJ_CINCDIRS += ${TMP_PATH}/fs/romfs/
CSOURCES += ${TMP_PATH}/fs/romfs/gromfs.c

ifeq (${MK_TEST_ENABLE},y)
PROJ_CINCDIRS += ${TMP_PATH}/fs/romfs/test
CSOURCES += ${TMP_PATH}/fs/romfs/test/romfs_test_data.c
CSOURCES += ${TMP_PATH}/fs/romfs/test/romfs_test.c
endif

endif # end with MK_USE_FS_ROMFS

