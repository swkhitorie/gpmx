#include "p2p_rbuffer.h"
#include <stddef.h>

uint16_t prb_size(struct __prbuf *obj) { return obj->size; }
bool prb_is_empty(struct __prbuf *obj) { return (obj->size == 0); }
bool prb_is_full(struct __prbuf *obj) { return (obj->size == P_RBUFFER_LENGTH); }
void prb_reset(struct __prbuf *obj) { obj->size = obj->out = obj->in = 0; } 

uint16_t prb_write(struct __prbuf *obj, const uint8_t *p, uint16_t len)
{
	uint16_t i;
	uint16_t wlen = 0;
	uint16_t rssize = 0;

    P2P_ENTER_CRITICAL();

	rssize = P_RBUFFER_LENGTH - obj->size;
	if (rssize >= len) {
		wlen = len;
	} else {
        wlen = rssize;
	}

	for (i = 0; i < wlen; i++) {
		obj->buf[obj->in] = p[i];
		obj->in++;
		if (obj->in >= P_RBUFFER_LENGTH) {
			obj->in -= P_RBUFFER_LENGTH;
		}
        obj->size++;
	}

    P2P_EXIT_CRITICAL();

	return wlen;
}

uint16_t prb_read(struct __prbuf *obj, uint8_t *p, uint16_t len)
{
	uint16_t i;
	uint16_t rlen = 0;

    P2P_ENTER_CRITICAL();

	if (obj->size >= len) {
		rlen = len;
	} else {
		rlen = obj->size;
	}

	for (i = 0; i < rlen; i++) {
		p[i] = obj->buf[obj->out];
		obj->out++;
		if (obj->out >= P_RBUFFER_LENGTH) {
			obj->out -= P_RBUFFER_LENGTH;
		}
        obj->size--;
	}

    P2P_EXIT_CRITICAL();

	return rlen;
}
