#include "board_usb_cdc.h"

#include <device/dnode.h>
#include <device/gringbuffer.h>
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <semphr.h>
#endif

/**
    PX4 Firmware:
    #define USBD_VID           0x3185
    #define USBD_PID           0x0038
    default:
    #define USBD_VID           0xFFFF
    #define USBD_PID           0xFFFF
*/

/*!< endpoint address */
#define CDC_IN_EP  0x81
#define CDC_OUT_EP 0x02
#define CDC_INT_EP 0x83

#define USBD_VID           0xFFFF
#define USBD_PID           0xFFFF
#define USBD_MAX_POWER     100
#define USBD_LANGID_STRING 1033

/*!< config descriptor size */
#define USB_CONFIG_SIZE (9 + CDC_ACM_DESCRIPTOR_LEN)

#ifdef CONFIG_USB_HS
#define CDC_MAX_MPS 512
#else
#define CDC_MAX_MPS 64
#endif

/*!< global descriptor */
static const uint8_t cdc_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0xEF, 0x02, 0x01, USBD_VID, USBD_PID, 0x0100, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x02, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    CDC_ACM_DESCRIPTOR_INIT(0x00, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, CDC_MAX_MPS, 0x02),
    ///////////////////////////////////////
    /// string0 descriptor
    ///////////////////////////////////////
    USB_LANGID_INIT(USBD_LANGID_STRING),
    ///////////////////////////////////////
    /// string1 descriptor
    ///////////////////////////////////////
    0x14,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    ///////////////////////////////////////
    /// string2 descriptor
    ///////////////////////////////////////
    0x26,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'C', 0x00,                  /* wcChar0 */
    'h', 0x00,                  /* wcChar1 */
    'e', 0x00,                  /* wcChar2 */
    'r', 0x00,                  /* wcChar3 */
    'r', 0x00,                  /* wcChar4 */
    'y', 0x00,                  /* wcChar5 */
    'U', 0x00,                  /* wcChar6 */
    'S', 0x00,                  /* wcChar7 */
    'B', 0x00,                  /* wcChar8 */
    ' ', 0x00,                  /* wcChar9 */
    'C', 0x00,                  /* wcChar10 */
    'D', 0x00,                  /* wcChar11 */
    'C', 0x00,                  /* wcChar12 */
    ' ', 0x00,                  /* wcChar13 */
    'D', 0x00,                  /* wcChar14 */
    'E', 0x00,                  /* wcChar15 */
    'M', 0x00,                  /* wcChar16 */
    'O', 0x00,                  /* wcChar17 */
    ///////////////////////////////////////
    /// string3 descriptor
    ///////////////////////////////////////
    0x16,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    '2', 0x00,                  /* wcChar0 */
    '0', 0x00,                  /* wcChar1 */
    '2', 0x00,                  /* wcChar2 */
    '2', 0x00,                  /* wcChar3 */
    '1', 0x00,                  /* wcChar4 */
    '2', 0x00,                  /* wcChar5 */
    '3', 0x00,                  /* wcChar6 */
    '4', 0x00,                  /* wcChar7 */
    '5', 0x00,                  /* wcChar8 */
    '6', 0x00,                  /* wcChar9 */
#ifdef CONFIG_USB_HS
    ///////////////////////////////////////
    /// device qualifier descriptor
    ///////////////////////////////////////
    0x0a,
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x02,
    0x02,
    0x01,
    0x40,
    0x00,
    0x00,
#endif
    0x00
};

static volatile bool ep_tx_busy_flag = false;
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t cdc_rbuf[CDC_MAX_MPS];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t cdc_wbuf[CDC_MAX_MPS];
static uint8_t rb_tx_buffer[512];
static uint8_t rb_rx_buffer[512];
static struct gringbuffer usb_rb_tx = {.capacity = 512, .buffer = rb_tx_buffer};
static struct gringbuffer usb_rb_rx = {.capacity = 512, .buffer = rb_rx_buffer};
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)
static SemaphoreHandle_t tx_sem;
#endif

static void usbd_event_handler(uint8_t busid, uint8_t event)
{
    int fsize = 0;

    switch (event) {
        case USBD_EVENT_RESET:
            break;
        case USBD_EVENT_CONNECTED:
            break;
        case USBD_EVENT_DISCONNECTED:
            break;
        case USBD_EVENT_RESUME:
            break;
        case USBD_EVENT_SUSPEND:
            break;
        case USBD_EVENT_CONFIGURED:
            /* setup first out ep read transfer */
            usbd_ep_start_read(busid, CDC_OUT_EP, cdc_rbuf, CDC_MAX_MPS);
            if (grb_size(&usb_rb_tx) > 0) {
                gpdrv_irq_disable();
                fsize = grb_read(&usb_rb_tx, &cdc_wbuf[0], CDC_MAX_MPS);
                gpdrv_irq_enable();
                usbd_ep_start_write(0, CDC_IN_EP, cdc_wbuf, fsize);
            } else {
                ep_tx_busy_flag = false;
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)
                BaseType_t h_pri;
                xSemaphoreGiveFromISR(tx_sem, &h_pri);
                portYIELD_FROM_ISR(h_pri);
#endif
            }
            break;
        case USBD_EVENT_SET_REMOTE_WAKEUP:
            break;
        case USBD_EVENT_CLR_REMOTE_WAKEUP:
            break;

        default:
            break;
    }
}

void usbd_cdc_acm_bulk_out(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    gpdrv_irq_disable();
    grb_write(&usb_rb_rx, &cdc_rbuf[0], nbytes);
    gpdrv_irq_enable();

    usbd_ep_start_read(busid, CDC_OUT_EP, cdc_rbuf, CDC_MAX_MPS);
}

void usbd_cdc_acm_bulk_in(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    int fsize = 0;

    if (grb_size(&usb_rb_tx) > 0) {
        gpdrv_irq_disable();
        fsize = grb_read(&usb_rb_tx, &cdc_wbuf[0], CDC_MAX_MPS);
        gpdrv_irq_enable();
        usbd_ep_start_write(0, CDC_IN_EP, cdc_wbuf, fsize);
    } else {
        ep_tx_busy_flag = false;
        if ((nbytes % usbd_get_ep_mps(busid, ep)) == 0 && nbytes) {
            /* send zlp */
            usbd_ep_start_write(busid, CDC_IN_EP, NULL, 0);
        }
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)
        BaseType_t h_pri;
        xSemaphoreGiveFromISR(tx_sem, &h_pri);
        portYIELD_FROM_ISR(h_pri);
#endif
    }
}

/*!< endpoint call back */
struct usbd_endpoint cdc_out_ep = {
    .ep_addr = CDC_OUT_EP,
    .ep_cb = usbd_cdc_acm_bulk_out
};

struct usbd_endpoint cdc_in_ep = {
    .ep_addr = CDC_IN_EP,
    .ep_cb = usbd_cdc_acm_bulk_in
};

static struct usbd_interface intf0;
static struct usbd_interface intf1;

void board_cdc_acm_init(uint8_t busid, uintptr_t reg_base)
{
    ep_tx_busy_flag = true;

#if defined(CONFIG_BOARD_FREERTOS_ENABLE)
    tx_sem = xSemaphoreCreateBinary();
    xSemaphoreGive(tx_sem);
#endif

    usbd_desc_register(busid, cdc_descriptor);
    usbd_add_interface(busid, usbd_cdc_acm_init_intf(busid, &intf0));
    usbd_add_interface(busid, usbd_cdc_acm_init_intf(busid, &intf1));
    usbd_add_endpoint(busid, &cdc_out_ep);
    usbd_add_endpoint(busid, &cdc_in_ep);
    usbd_initialize(busid, reg_base, usbd_event_handler);

    ep_tx_busy_flag = false;
}

int board_cdc_acm_read(uint8_t busid, uint8_t *p, uint16_t len)
{
    int sz=0;
    gpdrv_irq_disable();
    sz = grb_read(&usb_rb_rx, p, len);
    gpdrv_irq_enable();
    return sz;
}

int board_cdc_acm_send(uint8_t busid, const uint8_t *p, uint16_t len, uint8_t way)
{
    uint16_t fsize = 0;
    uint16_t rsize = 0;
    int pidx = 0;
    (void)busid;

    if (way == 0) {
        for (fsize = len; fsize > 0;) {
            ep_tx_busy_flag = true;

            if (fsize > CDC_MAX_MPS) {
                for (int i = 0; i < CDC_MAX_MPS; i++) {
                    cdc_wbuf[i] = p[pidx];
                    pidx++;
                }
                usbd_ep_start_write(0, CDC_IN_EP, cdc_wbuf, CDC_MAX_MPS);
                fsize -= CDC_MAX_MPS;
            } else {
                for (int i = 0; i < fsize; i++) {
                    cdc_wbuf[i] = p[i];
                }
                usbd_ep_start_write(0, CDC_IN_EP, cdc_wbuf, fsize);
                fsize = 0;
            }
            while (ep_tx_busy_flag);
        }
    } else if (way == 1) {
#if defined(CONFIG_BOARD_FREERTOS_ENABLE)
        if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
            grb_write(&usb_rb_tx, &p[0], len);
            return 0;
        } else {
            if (xSemaphoreTake(tx_sem, 10) == pdTRUE) {

                gpdrv_irq_disable();
                rsize = grb_write(&usb_rb_tx, &p[0], len);
                gpdrv_irq_enable();

                if (rsize == 0 || ep_tx_busy_flag) {
                    xSemaphoreGive(tx_sem);
                    return 0;
                }

                ep_tx_busy_flag = true;

                gpdrv_irq_disable();
                fsize = grb_read(&usb_rb_tx, &cdc_wbuf[0], CDC_MAX_MPS);
                gpdrv_irq_enable();

                usbd_ep_start_write(0, CDC_IN_EP, cdc_wbuf, fsize);
            }
        }
#else
        gpdrv_irq_disable();
        rsize = grb_write(&usb_rb_tx, &p[0], len);
        gpdrv_irq_enable();

        if (rsize == 0 || ep_tx_busy_flag) {
            return 0;
        }

        ep_tx_busy_flag = true;

        gpdrv_irq_disable();
        fsize = grb_read(&usb_rb_tx, &cdc_wbuf[0], CDC_MAX_MPS);
        gpdrv_irq_enable();

        usbd_ep_start_write(0, CDC_IN_EP, cdc_wbuf, fsize);
#endif
    }

    return fsize;
}


