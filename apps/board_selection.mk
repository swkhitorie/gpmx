
# 1. manufacturer board
STM32_BOARDS   := pxboard_ebfv2 fanke_tech_h7 pixhawk_fmuv2 pixhawk_fmuv6x
N32_BOARDS     := n32g430_evk

SUPPORTED_BOARDS := $(STM32_BOARDS) $(N32_BOARDS)

ifeq (${BUILD_USR_BOARD},)
$(error BUILD_USR_BOARD is not defined. Usage: make ... BUILD_USR_BOARD=<board_name>)
endif

BOARD_CHECK := $(filter $(BUILD_USR_BOARD), $(SUPPORTED_BOARDS))
ifeq ($(BOARD_CHECK),)
$(error Unsupported board type: "$(BUILD_USR_BOARD)". Available: $(SUPPORTED_BOARDS))
endif

ifeq ($(filter $(BUILD_USR_BOARD), $(STM32_BOARDS)), $(BUILD_USR_BOARD))
    BOARD_SERIES := stm32
else ifeq ($(filter $(BUILD_USR_BOARD), $(N32_BOARDS)), $(BUILD_USR_BOARD))
    BOARD_SERIES := n32
else
    $(error Cannot determine chip series for board: $(BUILD_USR_BOARD))
endif

BOARD_MAKEFILE := ${SDK_ROOTDIR}/boards/${BOARD_SERIES}/${BUILD_USR_BOARD}/make.mk

ifeq ($(wildcard $(BOARD_MAKEFILE)),)
$(error Board makefile not found: $(BOARD_MAKEFILE))
endif

include $(BOARD_MAKEFILE)
