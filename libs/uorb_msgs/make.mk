
UORB_MSG_DIR=libs/uorb_msgs/uorb_build

ifneq ("$(wildcard ${SDK_ROOTDIR}/$(UORB_MSG_DIR))","")

FR_CINCDIRS += ${UORB_MSG_DIR}/
FR_CPPSOURCES += $(subst ${SDK_ROOTDIR}/,,$(wildcard ${SDK_ROOTDIR}/${UORB_MSG_DIR}/msg/topics_sources/*cpp))

endif
