
USB_CINCDIRS += CherryUSB/common
USB_CINCDIRS += CherryUSB/core
USB_CSOURCES += CherryUSB/core/usbd_core.c

USB_CINCDIRS += CherryUSB/class/cdc
USB_CSOURCES += CherryUSB/class/cdc/usbd_cdc_acm.c
USB_CSOURCES += CherryUSB/class/cdc/usbd_cdc_ecm.c


USB_CINCDIRS += CherryUSB/port/dwc2
USB_CSOURCES += CherryUSB/port/dwc2/usb_dc_dwc2.c
USB_CSOURCES += CherryUSB/port/dwc2/usb_glue_st.c

USB_CINCDIRS += cdc_acm_user
USB_CSOURCES += cdc_acm_user/cdc_acm_user.c

