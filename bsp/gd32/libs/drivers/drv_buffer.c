
#include "drv_buffer.h"
#include <stddef.h>

uint16_t devbuf_size(devbuf_t *obj) { return obj->size; }
uint16_t devbuf_free(devbuf_t *obj) { return obj->capacity - obj->size; }
uint16_t devbuf_overflow(devbuf_t *obj) { return obj->overflow; }
bool devbuf_is_empty(devbuf_t *obj) { return (obj->size == 0); }
bool devbuf_is_full(devbuf_t *obj) { return (obj->size == obj->capacity); }
void devbuf_reset(devbuf_t *obj) { obj->size = obj->idx_out = obj->idx_in = obj->overflow = 0; } 
void devbuf_init(devbuf_t *obj, uint8_t *p, uint16_t capacity)
{
	if (p == NULL) return;
	obj->buf = p;
	obj->capacity = capacity;
	devbuf_reset(obj);
}

uint16_t devbuf_write(devbuf_t *obj, const uint8_t *p, uint16_t len)
{
	uint16_t i;
	uint16_t write_len = 0;
	uint16_t fifo_res_size = 0;

	fifo_res_size = obj->capacity - devbuf_size(obj);
	if (fifo_res_size >= len) {
		write_len = len;
	} else {
		write_len = fifo_res_size;
		obj->overflow = len - fifo_res_size;
	}

	for (i = 0; i < write_len; i++) {
		obj->buf[obj->idx_in] = p[i];
		obj->idx_in++;
		if (obj->idx_in >= obj->capacity) {
			obj->idx_in -= obj->capacity;
		}
	}

	obj->size += write_len;
	return write_len;
}

uint16_t devbuf_read(devbuf_t *obj, uint8_t *p, uint16_t len)
{
	uint16_t i;
	uint16_t read_len = 0;

	if (devbuf_size(obj) >= len) {
		read_len = len;
	} else {
		read_len = devbuf_size(obj);
	}

	for (i = 0; i < read_len; i++) {
		p[i] = obj->buf[obj->idx_out];
		obj->idx_out++;
		if (obj->idx_out >= obj->capacity) {
			obj->idx_out -= obj->capacity;
		}
	}

	obj->size -= read_len;
	return read_len;
}

uint16_t devbuf_query(devbuf_t *obj, uint16_t offset, uint8_t *p, uint16_t len)
{
	uint16_t i;
	uint16_t query_len = 0;
	uint16_t query_idx = obj->idx_out + offset;

	if (devbuf_size(obj) >= (offset + len)) {
		query_len = len;
	} else {
		query_len = devbuf_size(obj);
	}

	for (i = 0; i < query_len; i++) {
		p[i] = obj->buf[query_idx];
		query_idx++;
		if (query_idx >= obj->capacity) {
			query_idx -= obj->capacity;
		}
	}

	return query_len;
}

