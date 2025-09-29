#ifndef _GRING_BUFFER_H_
#define _GRING_BUFFER_H_

#include <stdint.h>

struct gringbuffer {
    volatile uint16_t in;   /* Index to the head [IN] index in the buffer */
    volatile uint16_t out;   /* Index to the tail [OUT] index in the buffer */
    uint16_t          size;
    uint16_t          capacity;
    uint8_t           *buffer;
};

#ifdef __cplusplus
extern "C" {
#endif

void grb_init(struct gringbuffer *obj, uint8_t *p, uint16_t capacity);
void grb_clear(struct gringbuffer *obj);
uint16_t grb_size(struct gringbuffer *obj);
int grb_write(struct gringbuffer *obj, const uint8_t *p, uint16_t len);
uint16_t grb_read(struct gringbuffer *obj, uint8_t *p, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif
