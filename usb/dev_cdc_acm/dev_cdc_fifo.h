#ifndef DEV_CDC_FIFO_H_
#define DEV_CDC_FIFO_H_

#include <stdint.h>

#ifndef CONFIG_DEV_CDC_FIFO_BUFFER_LENGTH
#define CONFIG_DEV_CDC_FIFO_BUFFER_LENGTH  (512)
#endif

struct devfifo_cdc {
    uint8_t buf[CONFIG_DEV_CDC_FIFO_BUFFER_LENGTH];
    uint16_t size;
    uint16_t in;
    uint16_t out;
};

#ifdef __cplusplus
extern "C" {
#endif

uint16_t dfifocdc_size(struct devfifo_cdc *obj);

uint16_t dfifocdc_write(struct devfifo_cdc *obj, const uint8_t *p, uint16_t len);

uint16_t dfifocdc_read(struct devfifo_cdc *obj, uint8_t *p, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif
