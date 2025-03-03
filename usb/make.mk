


ifeq (${CONFIG_CRUSB},y)
FR_CINCDIRS += usb/CherryUSB/common
FR_CINCDIRS += usb/CherryUSB/core
FR_CSOURCES += usb/CherryUSB/core/usbd_core.c

FR_CINCDIRS += usb/CherryUSB/class/cdc
FR_CSOURCES += usb/CherryUSB/class/cdc/usbd_cdc_acm.c
FR_CSOURCES += usb/CherryUSB/class/cdc/usbd_cdc_ecm.c


FR_CINCDIRS += usb/CherryUSB/port/dwc2
FR_CSOURCES += usb/CherryUSB/port/dwc2/usb_dc_dwc2.c
FR_CSOURCES += usb/CherryUSB/port/dwc2/usb_glue_st.c

FR_CINCDIRS += usb/dev_cdc_acm
FR_CSOURCES += usb/dev_cdc_acm/dev_cdc_acm.c
FR_CSOURCES += usb/dev_cdc_acm/dev_cdc_fifo.c
endif

# CONFIG_CRUSB_CDC_TX_FIFO_ENABLE
# CONFIG_CRUSB_CDC_STD_INOUT_ENABLE
