#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#include <stdint.h>
#include <stdbool.h>

#define RBUFFER_LENGTH (3*1024)


struct __prbuf {
	uint8_t buf[RBUFFER_LENGTH];
	uint16_t size;
	uint16_t out;
	uint16_t in;
};

#ifdef __cplusplus
extern "C" {
#endif

uint16_t prb_size(struct __prbuf *obj);
bool prb_is_empty(struct __prbuf *obj);
bool prb_is_full(struct __prbuf *obj);
void prb_reset(struct __prbuf *obj);

uint16_t prb_write(struct __prbuf *obj, const uint8_t *p, uint16_t len);
uint16_t prb_read(struct __prbuf *obj, uint8_t *p, uint16_t len);

#ifdef __cplusplus
}
#endif


#endif
