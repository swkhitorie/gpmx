#include "lwip_net_test.h"
#include "lwip/netif.h"
#include "lwip/ip.h"
#include "lwip/tcp.h"
#include "lwip/init.h"
#include "lwip/udp.h"
#include "lwip/pbuf.h"
#include "lwip/inet.h"
#include "netif/etharp.h"

#include <stdio.h>	
#include <string.h>

#if defined(CONFIG_FREERTOS_ENABLE)
#include <FreeRTOS.h>
#include <task.h>
#elif defined(CONFIG_RTTNANO_ENABLE)
#include <rthw.h>
#include <rtthread.h>
#endif

#ifndef UP_UDP_IP
#define UP_UDP_IP "192.168.1.130"
#endif

#ifndef UP_UDP_PORT
#define UP_UDP_PORT   6001
#endif

static struct udp_pcb *udp_tpcb;
static ip_addr_t target_ip;
static uint16_t target_port;
static bool is_bind_target = false;
static uint32_t request_tick = 0;

static void udp_empty_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    if (!strcmp((const char *)p->payload, "READY")) {
        TEST_PRINTF("[eth] success to link \r\n");
        is_bind_target = true;
    } else {
        TEST_PRINTF("[eth] ready msg error \r\n");
    }

    pbuf_free(p);
}

int udp_transfer_raw(const uint8_t *p, uint16_t len)
{
    int ret = 0;

    struct pbuf *q = NULL;

    q = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    if(!q) {
        TEST_PRINTF("[udp] out of PBUF_RAM\n");
        ret = 1;
        return ret;
    }

    memset(q->payload, 0 , q->len);
    memcpy(q->payload, p, len);

    udp_sendto(udp_tpcb, q, &target_ip, target_port);
    pbuf_free(q);

    return ret;
}

int udp_transfer_raw_control(const uint8_t *p, uint16_t len)
{
    int ret = 0;

    if (!is_bind_target) {
        return ret;
    }

    struct pbuf *q = NULL;

    q = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    if(!q) {
        TEST_PRINTF("[udp] out of PBUF_RAM\n");
        ret = 1;
        return ret;
    }

    memset(q->payload, 0 , q->len);
    memcpy(q->payload, p, len);

    udp_sendto(udp_tpcb, q, &target_ip, target_port);
    pbuf_free(q);

    return ret;
}

void udp_set_target_ip_port(ip_addr_t addr, uint16_t port)
{
    // is_bind_target = true;
    target_port = port;
    memcpy(&target_ip, &addr, sizeof(ip_addr_t));

    TEST_PRINTF("[eth] record host ip:%ld.%ld.%ld,%ld, port:%d\r\n",
        (target_ip.addr&0x000000ff),
        ((target_ip.addr&0x0000ff00)>>8),
        ((target_ip.addr&0x00ff0000)>>16),
        ((target_ip.addr&0xff000000)>>24), port);
}

void udp_transfer_init(void)
{
    udp_tpcb = udp_new();    

    udp_bind(udp_tpcb, IP_ADDR_ANY, UP_UDP_PORT);

    udp_recv(udp_tpcb, udp_empty_callback, NULL);
}

void udp_request(void *p)
{
    const char msg[] = "REQUEST";
    const char msg2[] = "TEST_UDP\r\n";

    while (1) {

        if (!is_bind_target) {
            TEST_PRINTF("[eth] request link\r\n");
            udp_transfer_raw((const uint8_t *)&msg[0], 7);
        } else {
            TEST_PRINTF("[eth] snd LwIP udp test \r\n");
            udp_transfer_raw((const uint8_t *)&msg2[0], 11);
        }

#if defined(CONFIG_FREERTOS_ENABLE)
        vTaskDelay(pdMS_TO_TICKS(100));
#elif defined(CONFIG_RTTNANO_ENABLE)
        rt_thread_mdelay(100);
#else
        board_delay(100);
#endif
    }
}


static ip_addr_t up_addr;
static uint16_t up_port;
int lwip_udp_test(int argc, char **argv)
{
    udp_transfer_init();

    up_addr.addr = inet_addr(UP_UDP_IP);
    up_port = UP_UDP_PORT;
    udp_set_target_ip_port(up_addr, up_port);


#if defined(CONFIG_FREERTOS_ENABLE)
    xTaskCreate(udp_request, "udp_req", 512, NULL, 3, NULL);
#elif defined(CONFIG_RTTNANO_ENABLE)
    rt_thread_t tid = rt_thread_create("udp_req", udp_request, RT_NULL, 1024, RT_THREAD_PRIORITY_MAX / 3, 20);
    if (tid != RT_NULL) {
        rt_thread_startup(tid);
    } else {
        TEST_PRINTF("[tcp] create thread failed\n");
        return -1;
    }
#else
    udp_request();
#endif

    return 0;
}

/*
    PC ETH (192.168.1.1)  sscom (192.168.1.130)
    TEST BOARD (192.168.1.34)(default)
        connect with port 6001

    1. TEST BOARD send REQUEST
    2. SSCOM send READY
    3. TEST BOARD received READY, change to send TEST_UDP
*/
