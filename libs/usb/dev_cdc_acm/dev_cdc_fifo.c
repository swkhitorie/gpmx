#include "dev_cdc_fifo.h"
#include "dev_cdc_acm.h"

uint16_t dfifocdc_size(struct devfifo_cdc *obj) 
{ 
    return obj->size; 
}

uint16_t dfifocdc_write(struct devfifo_cdc *obj, const uint8_t *p, uint16_t len)
{ 
	uint16_t i;
	uint16_t rsize;
	uint16_t wlen;

	rsize = CONFIG_DEV_CDC_FIFO_BUFFER_LENGTH - dfifocdc_size(obj);
	wlen = (rsize >= len) ? len : rsize;

	for (i = 0; i < wlen; i++) {
		obj->buf[obj->in] = p[i];
		obj->in++;
		if (obj->in >= CONFIG_DEV_CDC_FIFO_BUFFER_LENGTH) {
			obj->in -= CONFIG_DEV_CDC_FIFO_BUFFER_LENGTH;
		}
        obj->size++;
	}

	return wlen;
}

uint16_t dfifocdc_read(struct devfifo_cdc *obj, uint8_t *p, uint16_t len)
{
	uint16_t i;
    uint16_t size;
	uint16_t rlen;

	size = dfifocdc_size(obj);
	rlen = (size > len) ? len : size;

	for (i = 0; i < rlen; i++) {
		p[i] = obj->buf[obj->out];
		obj->out++;
		if (obj->out >= CONFIG_DEV_CDC_FIFO_BUFFER_LENGTH) {
			obj->out -= CONFIG_DEV_CDC_FIFO_BUFFER_LENGTH;
		}
        obj->size--;
	}

	return rlen;
}
