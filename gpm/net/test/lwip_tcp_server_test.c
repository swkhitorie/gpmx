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

#ifndef TCP_SERVER_PORT
#define TCP_SERVER_PORT   6001
#endif

#ifndef TCP_PC_IP
#define TCP_PC_IP         "192.168.1.130"
#endif

static struct tcp_pcb *tcp_server_pcb;    /* listen PCB */
static struct tcp_pcb *tcp_client_pcb;    /* client connected PCB */
static bool is_connected = false;
static bool is_ready = false;
static ip_addr_t pc_ip;

static err_t tcp_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (p == NULL) {
        TEST_PRINTF("[tcp] client closed connection\n");
        is_connected = false;
        is_ready = false;
        tcp_client_pcb = NULL;
        tcp_close(tpcb);
        return ERR_OK;
    }

    if (p->len > 0) {
        if (strncmp((const char *)p->payload, "READY", 5) == 0) {
            TEST_PRINTF("[tcp] received READY, target connected\n");
            is_ready = true;
        } else {
            TEST_PRINTF("[tcp] received unknown data: %.*s\n", p->len, (char *)p->payload);
        }
    }

    tcp_recved(tpcb, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}

static void tcp_err_callback(void *arg, err_t err)
{
    TEST_PRINTF("[tcp] connection error: %d\n", err);
    is_connected = false;
    is_ready = false;
    tcp_client_pcb = NULL;
}

static err_t tcp_accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    tcp_recv(newpcb, tcp_recv_callback);
    tcp_err(newpcb, tcp_err_callback);
    tcp_nagle_disable(newpcb);

    tcp_client_pcb = newpcb;
    is_connected = true;
    is_ready = false;

    pc_ip = newpcb->remote_ip;
    TEST_PRINTF("[tcp] client connected from %s:%d\n",
        ipaddr_ntoa(&pc_ip), newpcb->remote_port);

    return ERR_OK;
}

void tcp_server_init(void)
{
    tcp_server_pcb = tcp_new();
    if (tcp_server_pcb == NULL) {
        TEST_PRINTF("[tcp] failed to create server PCB\n");
        return;
    }

    err_t err = tcp_bind(tcp_server_pcb, IP_ADDR_ANY, TCP_SERVER_PORT);
    if (err != ERR_OK) {
        TEST_PRINTF("[tcp] bind failed: %d\n", err);
        return;
    }

    tcp_server_pcb = tcp_listen(tcp_server_pcb);
    if (tcp_server_pcb == NULL) {
        TEST_PRINTF("[tcp] listen failed\n");
        return;
    }

    tcp_accept(tcp_server_pcb, tcp_accept_callback);
    TEST_PRINTF("[tcp] server listening on port %d\n", TCP_SERVER_PORT);
}

void tcp_send_test_data(void)
{
    if (!is_connected || !is_ready || tcp_client_pcb == NULL) {
        return;
    }

    const char test_msg[] = "TCP_TEST\r\n";
    err_t wr_err = tcp_write(tcp_client_pcb, test_msg, sizeof(test_msg)-1, TCP_WRITE_FLAG_COPY);
    if (wr_err == ERR_OK) {
        tcp_output(tcp_client_pcb);
        TEST_PRINTF("[tcp] sent test data\n");
    } else {
        TEST_PRINTF("[tcp] tcp_write error: %d\n", wr_err);
    }
}

void tcp_request(void *p)
{
    const char req_msg[] = "REQUEST";

    while (1) {
        if (!is_connected) {
        } else if (!is_ready) {
        } else {
            tcp_send_test_data();
        }

#if defined(CONFIG_FREERTOS_ENABLE)
        vTaskDelay(pdMS_TO_TICKS(1000));
#elif defined(CONFIG_RTTNANO_ENABLE)
        rt_thread_mdelay(1000);
#else
        board_delay(1000);
#endif
    }
}

int lwip_tcp_server_test(int argc, char **argv)
{
    tcp_server_init();

#if defined(CONFIG_FREERTOS_ENABLE)
    xTaskCreate(tcp_request, "tcp_req", 512, NULL, 3, NULL);
#elif defined(CONFIG_RTTNANO_ENABLE)
    rt_thread_t tid = rt_thread_create("tcp_req", tcp_request, RT_NULL, 1024, RT_THREAD_PRIORITY_MAX / 3, 20);
    if (tid != RT_NULL) {
        rt_thread_startup(tid);
    } else {
        TEST_PRINTF("[tcp] create thread failed\n");
        return -1;
    }
#else
    tcp_request(NULL);
#endif

    return 0;
}

/*
    PC ETH (192.168.1.1)  
    sscom (192.168.1.130)
    TEST BOARD (192.168.1.34)(default) start listen port 6001

    sscom (192.168.1.130) request link (192.168.1.34)(6001)

    sscom request success
    TEST BOARD send REQUEST
    SSCOM receive REQUEST, send READY

    TEST BOARD received READY, change to send TEST_UDP
*/
