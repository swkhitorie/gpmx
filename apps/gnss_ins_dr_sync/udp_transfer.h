#ifndef _UDP_TRANSFER_H_
#define _UDP_TRANSFER_H_

#define UDP_TRANSFER_PORT 5001

#ifdef __cplusplus
extern "C" {
#endif

void udp_transfer_init(void);

int  udp_transfer_raw(const uint8_t *p, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif


