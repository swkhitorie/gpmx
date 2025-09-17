#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#include <stdint.h>
#if defined (__CC_ARM)
#include "cmsis_armcc.h"
#elif defined(__clang__) && (__GNUC__)
#include "cmsis_armclang.h"
#elif defined(__GNUC__)
#include "cmsis_gcc.h"
#endif


struct rbdrv {
    volatile uint16_t in;   /* Index to the head [IN] index in the buffer */
    volatile uint16_t out;   /* Index to the tail [OUT] index in the buffer */
    uint16_t          size;
    uint16_t          capacity;
    uint8_t      *buffer;
};

#ifdef __cplusplus
extern "C" {
#endif

void rbdrv_init(struct rbdrv *obj, uint8_t *p, uint16_t capacity);
void rbdrv_clear(struct rbdrv *obj);
uint16_t rbdrv_size(struct rbdrv *obj);
int rbdrv_write(struct rbdrv *obj, const uint8_t *p, uint16_t len);
uint16_t rbdrv_read(struct rbdrv *obj, uint8_t *p, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif
