#####
# no use
#####

ifeq (${BOARD_SELECTION},)
ifeq (${MAKE_TARGET_CLEANS},n)
$(error Invalid empty BOARD_SELECTION)
endif
endif

ifeq (${BOARD_SELECTION},f103_eval)
MOD_ARCH = m3
include ${SDK_ROOTDIR}/boards/stm32/stm32f103_eval/make.mk
endif

ifeq (${BOARD_SELECTION},f407_eval)
MOD_ARCH = m4
include ${SDK_ROOTDIR}/boards/stm32/stm32f407_eval/make.mk
endif

ifeq (${BOARD_SELECTION},h743_eval)
MOD_ARCH = m7
include ${SDK_ROOTDIR}/boards/stm32/stm32h743_eval/make.mk
endif

ifeq (${BOARD_SELECTION},h7b0_eval)
MOD_ARCH = m7
include ${SDK_ROOTDIR}/boards/stm32/stm32h7b0_eval/make.mk
endif

ifeq (${BOARD_SELECTION},fmu_v2)
MOD_ARCH = m4
include ${SDK_ROOTDIR}/boards/stm32/stm32f427_fmuv2/make.mk
endif

ifeq (${BOARD_SELECTION},fmu_v6c)
MOD_ARCH = m7
include ${SDK_ROOTDIR}/boards/stm32/stm32h743_fmuv6/make.mk
endif

ifeq (${BOARD_SELECTION},wl55_eval)
MOD_ARCH = m4
include ${SDK_ROOTDIR}/boards/stm32/stm32wl55_eval/make.mk
endif

ifeq (${BOARD_SELECTION},wle5_eval)
MOD_ARCH = m4
include ${SDK_ROOTDIR}/boards/stm32/stm32wle5_eval/make.mk
endif

