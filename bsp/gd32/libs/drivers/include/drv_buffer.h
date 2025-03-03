#ifndef DRV_BUFFER_H_
#define DRV_BUFFER_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct __devbuf {
	uint8_t *buf;
	uint16_t capacity;
	uint16_t size;
	uint16_t idx_out;
	uint16_t idx_in;
	uint16_t overflow;
} devbuf_t;

#ifdef __cplusplus
extern "C" {
#endif

uint16_t devbuf_size(devbuf_t *obj);
uint16_t devbuf_free(devbuf_t *obj);
uint16_t devbuf_overflow(devbuf_t *obj);
bool devbuf_is_empty(devbuf_t *obj);
bool devbuf_is_full(devbuf_t *obj);
void devbuf_reset(devbuf_t *obj);
void devbuf_init(devbuf_t *obj, uint8_t *p, uint16_t capacity);

uint16_t devbuf_write(devbuf_t *obj, const uint8_t *p, uint16_t len);
uint16_t devbuf_read(devbuf_t *obj, uint8_t *p, uint16_t len);
uint16_t devbuf_query(devbuf_t *obj, uint16_t offset, uint8_t *p, uint16_t len);

#ifdef __cplusplus
}
#endif


#endif
