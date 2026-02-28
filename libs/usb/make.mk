

ifeq (${MK_USE_CRUSB},y)
PROJ_CDEFS += CONFIG_CRUSB_DEVICE_ENABLE
PROJ_CINCDIRS += libs/usb/CherryUSB/common
PROJ_CINCDIRS += libs/usb/CherryUSB/core
CSOURCES += libs/usb/CherryUSB/core/usbd_core.c

ifeq (${MK_USE_CRUSB_CLASS},cdc_acm)
PROJ_CDEFS += CONFIG_CRUSB_DEVICE_CDC_ACM_ENABLE
PROJ_CINCDIRS += libs/usb/CherryUSB/class/cdc
PROJ_CINCDIRS += libs/usb/CherryUSB/class/hub
CSOURCES += libs/usb/CherryUSB/class/cdc/usbd_cdc_acm.c
endif  # end with MK_USE_CRUSB_CLASS

ifeq (${MK_USE_CRUSB_IP},dwc2_st)
PROJ_CINCDIRS += libs/usb/CherryUSB/port/dwc2
CSOURCES += libs/usb/CherryUSB/port/dwc2/usb_dc_dwc2.c
CSOURCES += libs/usb/CherryUSB/port/dwc2/usb_glue_st.c
endif  # end with MK_USE_CRUSB_IP

endif  # end with MK_USE_CRUSB
