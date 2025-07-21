

ifeq (${MK_USE_CRUSB},y)
PROJ_CINCDIRS += usb/CherryUSB/common
PROJ_CINCDIRS += usb/CherryUSB/core
CSOURCES += usb/CherryUSB/core/usbd_core.c
endif

ifeq (${MK_CRUSB_IP},dwc2_st)
PROJ_CINCDIRS += usb/CherryUSB/port/dwc2
CSOURCES += usb/CherryUSB/port/dwc2/usb_dc_dwc2.c
CSOURCES += usb/CherryUSB/port/dwc2/usb_glue_st.c
endif

ifeq (${MK_USE_CRUSB_CDC},y)
PROJ_CINCDIRS += usb/CherryUSB/class/cdc
CSOURCES += usb/CherryUSB/class/cdc/usbd_cdc_acm.c
CSOURCES += usb/CherryUSB/class/cdc/usbd_cdc_ecm.c

PROJ_CINCDIRS += usb/dev_cdc_acm
CSOURCES += usb/dev_cdc_acm/dev_cdc_acm.c
CSOURCES += usb/dev_cdc_acm/dev_cdc_fifo.c
endif
