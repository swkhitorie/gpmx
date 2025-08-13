#include "lwip/netif.h"
#include "lwip/ip.h"
#include "lwip/tcp.h"
#include "lwip/init.h"
#include "lwip/udp.h"
#include "lwip/pbuf.h"
#include "netif/etharp.h"

#include "udp_transfer.h"

#include <stdio.h>	
#include <string.h>	

static struct udp_pcb *udp_tpcb;
static ip_addr_t target_ip;
static uint16_t target_port;
static bool is_bind_target = false;

static void udp_empty_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    pbuf_free(p);

    is_bind_target = true;
    target_port = port;
    memcpy(&target_ip, addr, sizeof(ip_addr_t));

    printf("[udp] record host ip:%ld.%ld.%ld,%ld, port:%d\r\n",
        (target_ip.addr&0x000000ff),
        ((target_ip.addr&0x0000ff00)>>8),
        ((target_ip.addr&0x00ff0000)>>16),
        ((target_ip.addr&0xff000000)>>24), port);
}

void udp_transfer_init(void)
{
    udp_tpcb = udp_new();    

    udp_bind(udp_tpcb, IP_ADDR_ANY, UDP_TRANSFER_PORT);

    udp_recv(udp_tpcb, udp_empty_callback, NULL);
}

int  udp_transfer_raw(const uint8_t *p, uint16_t len)
{
    int ret = 0;

    if (!is_bind_target) {
        return ret;
    }

    struct pbuf *q = NULL;

    q = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    if(!q) {
        printf("[udp] out of PBUF_RAM\n");
        ret = 1;
        return ret;
    }

    memset(q->payload, 0 , q->len);
    memcpy(q->payload, p, len);

    udp_sendto(udp_tpcb, q, &target_ip, target_port);
    pbuf_free(q);

    return ret;
}
