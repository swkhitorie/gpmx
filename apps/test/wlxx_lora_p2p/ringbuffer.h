#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct __ringbuf {
	uint8_t *buf;
	uint16_t capacity;
	uint16_t size;
	uint16_t out;
	uint16_t in;
} ringbuffer_t;

#ifdef __cplusplus
extern "C" {
#endif

uint16_t rb_size(ringbuffer_t *obj);
bool rb_is_empty(ringbuffer_t *obj);
bool rb_is_full(ringbuffer_t *obj);
void rb_reset(ringbuffer_t *obj);
void rb_init(ringbuffer_t *obj, uint8_t *p, uint16_t capacity);

uint16_t rb_write(ringbuffer_t *obj, const uint8_t *p, uint16_t len);
uint16_t rb_read(ringbuffer_t *obj, uint8_t *p, uint16_t len);

#ifdef __cplusplus
}
#endif


#endif
