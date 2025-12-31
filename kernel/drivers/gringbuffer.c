#include <device/gringbuffer.h>

void grb_init(struct gringbuffer *obj, uint8_t *p, uint16_t capacity)
{
    obj->buffer = p;
    obj->capacity = capacity;
    grb_clear(obj);
}

void grb_clear(struct gringbuffer *obj)
{
    obj->in = 0;
    obj->out = 0;
    obj->size = 0;
}

uint16_t grb_size(struct gringbuffer *obj)
{
    uint16_t rt = 0;

    rt = obj->size;

    return rt;
}

int grb_write(struct gringbuffer *obj, const uint8_t *p, uint16_t len)
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
		obj->buffer[obj->in] = p[i];
		obj->in++;
		if (obj->in >= obj->capacity) {
			obj->in -= obj->capacity;
		}
        obj->size++;
	}

	return wlen;
}

uint16_t grb_read(struct gringbuffer *obj, uint8_t *p, uint16_t len)
{
	uint16_t i;
	uint16_t rlen = 0;

	if (obj->size >= len) {
		rlen = len;
	} else {
		rlen = obj->size;
	}

	for (i = 0; i < rlen; i++) {
		p[i] = obj->buffer[obj->out];
		obj->out++;
		if (obj->out >= obj->capacity) {
			obj->out -= obj->capacity;
		}
        obj->size--;
	}

	return rlen;
}

