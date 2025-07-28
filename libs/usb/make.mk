

ifeq (${MK_USE_CRUSB},y)
PROJ_CINCDIRS += libs/usb/CherryUSB/common
PROJ_CINCDIRS += libs/usb/CherryUSB/core
CSOURCES += libs/usb/CherryUSB/core/usbd_core.c

ifeq (${MK_CRUSB_IP},dwc2_st)
PROJ_CINCDIRS += libs/usb/CherryUSB/port/dwc2
CSOURCES += libs/usb/CherryUSB/port/dwc2/usb_dc_dwc2.c
CSOURCES += libs/usb/CherryUSB/port/dwc2/usb_glue_st.c
endif  # end with MK_CRUSB_IP

ifeq (${MK_USE_CRUSB_CDC},y)
PROJ_CINCDIRS += libs/usb/CherryUSB/class/cdc
PROJ_CINCDIRS += libs/usb/CherryUSB/class/hub
CSOURCES += libs/usb/CherryUSB/class/cdc/usbd_cdc_acm.c
CSOURCES += libs/usb/CherryUSB/class/cdc/usbd_cdc_ecm.c

PROJ_CINCDIRS += libs/usb/dev_cdc_acm
CSOURCES += libs/usb/dev_cdc_acm/dev_cdc_acm.c
CSOURCES += libs/usb/dev_cdc_acm/dev_cdc_fifo.c
endif  # end with MK_USE_CRUSB_CDC

endif  # end with MK_USE_CRUSB
