#ifndef _UDP_TRANSFER_H_
#define _UDP_TRANSFER_H_

#include "lwip/netif.h"
#include "lwip/ip.h"
#include "lwip/tcp.h"
#include "lwip/init.h"
#include "lwip/udp.h"
#include "lwip/pbuf.h"
#include "lwip/inet.h"
#include "netif/etharp.h"

#define UDP_TRANSFER_PORT 5001


#include "board_config.h"
#define UDP_TRANSFER_TIMESTAMP    HAL_GetTick

#ifdef __cplusplus
extern "C" {
#endif

void udp_set_target_ip_port(ip_addr_t addr, uint16_t port);

void udp_transfer_init();

int  udp_transfer_raw(const uint8_t *p, uint16_t len);

int  udp_transfer_raw_control(const uint8_t *p, uint16_t len);

void udp_request();

#ifdef __cplusplus
}
#endif

#endif


