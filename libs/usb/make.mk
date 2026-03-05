

ifeq (${MK_USE_CRUSB},y)
PROJ_CDEFS += CONFIG_CRUSB_DEVICE_ENABLE
PROJ_CINCDIRS += libs/usb/CherryUSB/common
PROJ_CINCDIRS += libs/usb/CherryUSB/core
CSOURCES += libs/usb/CherryUSB/core/usbd_core.c

ifeq (${MK_USE_CRUSB_IP},dwc2_st)
PROJ_CINCDIRS += libs/usb/CherryUSB/port/dwc2
CSOURCES += libs/usb/CherryUSB/port/dwc2/usb_dc_dwc2.c
CSOURCES += libs/usb/CherryUSB/port/dwc2/usb_glue_st.c
endif  # end with MK_USE_CRUSB_IP

ifneq ($(filter cdc_acm, $(MK_USE_CRUSB_CLASS)),)
PROJ_CDEFS += CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE
PROJ_CINCDIRS += libs/usb/CherryUSB/class/cdc
PROJ_CINCDIRS += libs/usb/CherryUSB/class/hub
CSOURCES += libs/usb/CherryUSB/class/cdc/usbd_cdc_acm.c
endif

ifneq ($(filter msc, $(MK_USE_CRUSB_CLASS)),)
PROJ_CDEFS += CONFIG_CRUSB_DEVICE_MSC_ENABLE
PROJ_CINCDIRS += libs/usb/CherryUSB/class/msc
CSOURCES += libs/usb/CherryUSB/class/msc/usbd_msc.c
endif

endif  # end with MK_USE_CRUSB
