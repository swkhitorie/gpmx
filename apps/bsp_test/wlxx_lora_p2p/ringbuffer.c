#include "ringbuffer.h"
#include <stddef.h>

uint16_t rb_size(ringbuffer_t *obj) { return obj->size; }
bool rb_is_empty(ringbuffer_t *obj) { return (obj->size == 0); }
bool rb_is_full(ringbuffer_t *obj) { return (obj->size == obj->capacity); }
void rb_reset(ringbuffer_t *obj) { obj->size = obj->out = obj->in = 0; } 
void rb_init(ringbuffer_t *obj, uint8_t *p, uint16_t capacity)
{
    if (p != NULL && capacity != 0) {
        obj->buf = p;
        obj->capacity = capacity;
    }
	rb_reset(obj);
}

uint16_t rb_write(ringbuffer_t *obj, const uint8_t *p, uint16_t len)
{
	uint16_t i;
	uint16_t wlen = 0;
	uint16_t rssize = 0;

	rssize = obj->capacity - obj->size;
	if (rssize >= len) {
		wlen = len;
	} else {
		wlen = rssize;
	}

	for (i = 0; i < wlen; i++) {
		obj->buf[obj->in] = p[i];
		obj->in++;
		if (obj->in >= obj->capacity) {
			obj->in -= obj->capacity;
		}
        obj->size++;
	}

	return wlen;
}

uint16_t rb_read(ringbuffer_t *obj, uint8_t *p, uint16_t len)
{
	uint16_t i;
	uint16_t rlen = 0;

	if (obj->size >= len) {
		rlen = len;
	} else {
		rlen = obj->size;
	}

	for (i = 0; i < rlen; i++) {
		p[i] = obj->buf[obj->out];
		obj->out++;
		if (obj->out >= obj->capacity) {
			obj->out -= obj->capacity;
		}
        obj->size--;
	}

	return rlen;
}

