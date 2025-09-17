#include <device/rbdrv.h>

void rbdrv_init(struct rbdrv *obj, uint8_t *p, uint16_t capacity)
{
    obj->buffer = p;
    obj->capacity = capacity;
    rbdrv_clear(obj);
}

void rbdrv_clear(struct rbdrv *obj)
{
    obj->in = 0;
    obj->out = 0;
    obj->size = 0;
}

uint16_t rbdrv_size(struct rbdrv *obj)
{
    uint16_t rt = 0;
    
    __disable_irq();

    rt = obj->size;

    __enable_irq();

    return rt;
}

int rbdrv_write(struct rbdrv *obj, const uint8_t *p, uint16_t len)
{
	uint16_t i;
	uint16_t wlen = 0;
	uint16_t rssize = 0;

    __disable_irq();

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

    __enable_irq();
	return wlen;
}

uint16_t rbdrv_read(struct rbdrv *obj, uint8_t *p, uint16_t len)
{
	uint16_t i;
	uint16_t rlen = 0;

    __disable_irq();

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

    __enable_irq();
	return rlen;
}

