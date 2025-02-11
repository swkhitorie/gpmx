/*
 * Copyright (c) 2024, sakumisu
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "dev_cdc_acm.h"

/*!< endpoint address */
#define CDC_IN_EP  0x81
#define CDC_OUT_EP 0x02
#define CDC_INT_EP 0x83

/**
    PX4 Firmware:
    #define USBD_VID           0x3185
    #define USBD_PID           0x0038
    default:
    #define USBD_VID           0xFFFF
    #define USBD_PID           0xFFFF
*/
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

/* 2048 is only for test speed , please use CDC_MAX_MPS for common*/
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t read_buffer[CDC_MAX_MPS];
USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t write_buffer[CDC_MAX_MPS];

struct devfifo_cdc {
    uint8_t buf[1024];
    uint16_t size;
    uint16_t in;
    uint16_t out;
};
struct devfifo_cdc devbuf_tx = {.in = 0, .out = 0, .size = 0};
struct devfifo_cdc devbuf_rx = {.in = 0, .out = 0, .size = 0};


volatile bool ep_tx_busy_flag = false;

static void usbd_event_handler(uint8_t busid, uint8_t event)
{
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
            ep_tx_busy_flag = false;
            /* setup first out ep read transfer */
            usbd_ep_start_read(busid, CDC_OUT_EP, read_buffer, CDC_MAX_MPS);
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
    // USB_LOG_RAW("actual out len:%d\r\n", nbytes);
    /* setup next out ep read transfer */
    if ((1024 - devbuf_rx.size) >= nbytes) {
        for (int i = 0; i < nbytes; i++) {
            devbuf_rx.buf[devbuf_rx.in] = read_buffer[i];
            devbuf_rx.in++;
            if (devbuf_rx.in == 1024) {
                devbuf_rx.in = 0;
            }
            devbuf_rx.size++;
        }
    }
    usbd_ep_start_read(busid, CDC_OUT_EP, read_buffer, CDC_MAX_MPS);
}

void usbd_cdc_acm_bulk_in(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    int fsize = 0;
    // USB_LOG_RAW("actual in len:%d\r\n", nbytes);

    if ((nbytes % usbd_get_ep_mps(busid, ep)) == 0 && nbytes) {
        /* send zlp */
        usbd_ep_start_write(busid, CDC_IN_EP, NULL, 0);
    } else {
        if (devbuf_tx.size <= 0) {
            ep_tx_busy_flag = false;
            return;
        } else {
            fsize = (devbuf_tx.size > CDC_MAX_MPS) ? CDC_MAX_MPS : devbuf_tx.size;
            for (int i = 0; i < fsize; i++) {
                write_buffer[i] = devbuf_tx.buf[devbuf_tx.out];
                devbuf_tx.out++;
                if (devbuf_tx.out == 1024) {
                    devbuf_tx.out = 0;
                }
                devbuf_tx.size--;
            }
            usbd_ep_start_write(0, CDC_IN_EP, write_buffer, fsize);
        }
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

void cdc_acm_init(uint8_t busid, uintptr_t reg_base)
{
    dev_cdc_acm_init(busid, reg_base);
}

void dev_cdc_acm_init(uint8_t busid, uintptr_t reg_base)
{
    usbd_desc_register(busid, cdc_descriptor);
    usbd_add_interface(busid, usbd_cdc_acm_init_intf(busid, &intf0));
    usbd_add_interface(busid, usbd_cdc_acm_init_intf(busid, &intf1));
    usbd_add_endpoint(busid, &cdc_out_ep);
    usbd_add_endpoint(busid, &cdc_in_ep);
    usbd_initialize(busid, reg_base, usbd_event_handler);
}

int dev_cdc_acm_rsize(uint8_t busid)
{
    (void)busid;
    return devbuf_rx.size;
}

int dev_cdc_acm_send(uint8_t busid, const uint8_t *p, uint16_t len)
{
    uint16_t fsize = 0;
    (void)busid;
    if ((1024 - devbuf_tx.size) < len) return 0;
    for (int i = 0; i < len; i++) {
        devbuf_tx.buf[devbuf_tx.in] = p[i];
        devbuf_tx.in++;
        if (devbuf_tx.in == 1024) {
            devbuf_tx.in = 0;
        }
        devbuf_tx.size++;
    }

    if (ep_tx_busy_flag) return 0;
    ep_tx_busy_flag = true;

    fsize = (devbuf_tx.size > CDC_MAX_MPS) ? CDC_MAX_MPS : devbuf_tx.size;
    for (int i = 0; i < fsize; i++) {
        write_buffer[i] = devbuf_tx.buf[devbuf_tx.out];
        devbuf_tx.out++;
        if (devbuf_tx.out == 1024) {
            devbuf_tx.out = 0;
        }
        devbuf_tx.size--;
    }

    usbd_ep_start_write(0, CDC_IN_EP, write_buffer, fsize);
    return fsize;
}

int dev_cdc_acm_read(uint8_t busid, uint8_t *p, uint16_t len)
{
    int fsize = (devbuf_rx.size > len) ? len : devbuf_rx.size;

    for (int i = 0; i < fsize; i++) {
        p[i] = devbuf_rx.buf[devbuf_rx.out];
        devbuf_rx.out++;
        if (devbuf_rx.out == 1024) {
            devbuf_rx.out = 0;
        }
        devbuf_rx.size--;
    }
    return fsize;
}

#ifdef CRUSB_PRINTF_ENABLE
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
FILE __stdin, __stdout, __stderr;

size_t fwrite(const void *ptr, size_t size, size_t n_items, FILE *stream)
{
    return _write(stream->_file, ptr, size*n_items);
}

int _write(int file, char *ptr, int len)
{
    const int stdin_fileno = 0;
    const int stdout_fileno = 1;
    const int stderr_fileno = 2;
    if (file == stdout_fileno) {
        dev_cdc_acm_send(0, ptr, len);
    }
    return len;
}

size_t fread(void *ptr, size_t size, size_t n_items, FILE *stream)
{
    return _read(stream->_file, ptr, size*n_items);
}

// nonblock
int _read(int file, char *ptr, int len)
{
    const int stdin_fileno = 0;
    const int stdout_fileno = 1;
    const int stderr_fileno = 2;
    size_t rcv_size = dev_cdc_acm_rsize(0);
    size_t sld_size = (len >= rcv_size) ? rcv_size: len;
    size_t ret_size = 0;
    if (file == stdin_fileno) {
        ret_size = dev_cdc_acm_read(0, ptr, sld_size);
    }
    return ret_size;
}

#endif
