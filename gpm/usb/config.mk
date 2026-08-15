
ifeq (${CONFIG_CRUSB_DEVICE_ENABLE},y)
PROJ_CINCDIRS += ${GPMPATH}/usb/CherryUSB/common
PROJ_CINCDIRS += ${GPMPATH}/usb/CherryUSB/core
CSOURCES += ${GPMPATH}/usb/CherryUSB/core/usbd_core.c

ifeq (${CONFIG_CRUSB_IP},"dwc2_st")
PROJ_CINCDIRS += ${GPMPATH}/usb/CherryUSB/port/dwc2
CSOURCES += ${GPMPATH}/usb/CherryUSB/port/dwc2/usb_dc_dwc2.c
CSOURCES += ${GPMPATH}/usb/CherryUSB/port/dwc2/usb_glue_st.c
endif

ifeq ($(CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE),y)
PROJ_CINCDIRS += ${GPMPATH}/usb/CherryUSB/class/cdc
PROJ_CINCDIRS += ${GPMPATH}/usb/CherryUSB/class/hub
CSOURCES += ${GPMPATH}/usb/CherryUSB/class/cdc/usbd_cdc_acm.c
endif

ifeq ($(CONFIG_CRUSB_DEVICE_MSC_ENABLE),y)
PROJ_CINCDIRS += ${GPMPATH}/usb/CherryUSB/class/msc
CSOURCES += ${GPMPATH}/usb/CherryUSB/class/msc/usbd_msc.c
endif

endif  # end with CONFIG_CRUSB_DEVICE_ENABLE
