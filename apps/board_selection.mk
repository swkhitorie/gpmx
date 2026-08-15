
ifeq (${BUILD_USR_BOARD},)
$(error BUILD_USR_BOARD is not defined. Usage: make ... BUILD_USR_BOARD=<board_name>)
endif

ifneq ($(BUILD_USR_BOARD),unix)

SUPPORTED_BOARDS += pxboard_ebfv2
SUPPORTED_BOARDS += fanke_tech_h7
SUPPORTED_BOARDS += pixhawk_fmuv2
SUPPORTED_BOARDS += pixhawk_fmuv6x
SUPPORTED_BOARDS += e77_900mbl
SUPPORTED_BOARDS += nucleo_wl55jc
SUPPORTED_BOARDS += n32g430_evk

BOARD_CHECK := $(filter $(BUILD_USR_BOARD), $(SUPPORTED_BOARDS))
ifeq ($(BOARD_CHECK),)
$(error Unsupported board type: "$(BUILD_USR_BOARD)". Available: $(SUPPORTED_BOARDS))
endif

BOARD_MAKEFILE := ${SDK_ROOTDIR}/boards/${BUILD_USR_BOARD}/make.mk
ifeq ($(wildcard $(BOARD_MAKEFILE)),)
$(error Board makefile not found: $(BOARD_MAKEFILE))
endif

include $(BOARD_MAKEFILE)

endif # end with ifneq ($(BUILD_USR_BOARD),unix)
