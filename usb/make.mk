

ifeq (${CONFIG_MK_USE_CRUSB},y)
FR_CINCDIRS += usb/CherryUSB/common
FR_CINCDIRS += usb/CherryUSB/core
FR_CSOURCES += usb/CherryUSB/core/usbd_core.c
endif

ifeq (${CONFIG_MK_CRUSB_IP},dwc2_st)
FR_CINCDIRS += usb/CherryUSB/port/dwc2
FR_CSOURCES += usb/CherryUSB/port/dwc2/usb_dc_dwc2.c
FR_CSOURCES += usb/CherryUSB/port/dwc2/usb_glue_st.c
endif

ifeq (${CONFIG_MK_USE_CRUSB_CDC},y)
FR_CINCDIRS += usb/CherryUSB/class/cdc
FR_CSOURCES += usb/CherryUSB/class/cdc/usbd_cdc_acm.c
FR_CSOURCES += usb/CherryUSB/class/cdc/usbd_cdc_ecm.c

FR_CINCDIRS += usb/dev_cdc_acm
FR_CSOURCES += usb/dev_cdc_acm/dev_cdc_acm.c
FR_CSOURCES += usb/dev_cdc_acm/dev_cdc_fifo.c
endif
