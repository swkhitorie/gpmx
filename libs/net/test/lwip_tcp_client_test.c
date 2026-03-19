#include "lwip_net_test.h"
#include "lwip/netif.h"
#include "lwip/ip.h"
#include "lwip/tcp.h"
#include "lwip/init.h"
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

#ifndef TCP_SERVER_IP
#define TCP_SERVER_IP     "192.168.1.130"
#endif

#ifndef TCP_SERVER_PORT
#define TCP_SERVER_PORT   6001
#endif

static struct tcp_pcb *tcp_client_pcb = NULL;
static ip_addr_t server_ip;
static uint16_t server_port;
static bool is_connected = false;
static bool is_ready = false;
static bool should_reconnect = false;

static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (p == NULL) {
        TEST_PRINTF("[tcp] server closed connection\n");
        is_connected = false;
        is_ready = false;
        should_reconnect = true;
        return ERR_OK;
    }

    if (p->len > 0) {
        if (strncmp((const char *)p->payload, "READY", 5) == 0) {
            TEST_PRINTF("[tcp] received READY, target ready\n");
            is_ready = true;
        } else {
            TEST_PRINTF("[tcp] received: %.*s\n", p->len, (char *)p->payload);
        }
    }

    tcp_recved(tpcb, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}

static void tcp_client_err(void *arg, err_t err)
{
    TEST_PRINTF("[tcp] connection error: %d\n", err);
    is_connected = false;
    is_ready = false;
    tcp_client_pcb = NULL;
    should_reconnect = true;
}

static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    if (err != ERR_OK) {
        TEST_PRINTF("[tcp] connect failed: %d\n", err);
        tcp_client_pcb = NULL;
        should_reconnect = true;
        return err;
    }

    TEST_PRINTF("[tcp] connected to server\n");
    is_connected = true;
    should_reconnect = false;

    tcp_nagle_disable(tpcb);

    const char *req_msg = "REQUEST";
    err_t wr_err = tcp_write(tpcb, req_msg, strlen(req_msg), TCP_WRITE_FLAG_COPY);
    if (wr_err == ERR_OK) {
        tcp_output(tpcb);
        TEST_PRINTF("[tcp] sent REQUEST\n");
    } else {
        TEST_PRINTF("[tcp] tcp_write error: %d\n", wr_err);
    }

    return ERR_OK;
}

static void tcp_client_connect(void)
{
    if (tcp_client_pcb != NULL) {
        return;
    }

    tcp_client_pcb = tcp_new();
    if (tcp_client_pcb == NULL) {
        TEST_PRINTF("[tcp] failed to create PCB\n");
        return;
    }

    tcp_recv(tcp_client_pcb, tcp_client_recv);
    tcp_err(tcp_client_pcb, tcp_client_err);

    err_t err = tcp_connect(tcp_client_pcb, &server_ip, server_port, tcp_client_connected);
    if (err != ERR_OK) {
        TEST_PRINTF("[tcp] tcp_connect error: %d\n", err);
        tcp_close(tcp_client_pcb);
        tcp_client_pcb = NULL;
        should_reconnect = true;
    } else {
        TEST_PRINTF("[tcp] connecting to %s:%d...\n", ipaddr_ntoa(&server_ip), server_port);
    }
}

void tcp_client_task(void *p)
{
    const char test_msg[] = "TCP_TEST\r\n";

    while (1) {
        if (tcp_client_pcb == NULL && should_reconnect) {
            tcp_client_connect();
        } else if (tcp_client_pcb == NULL) {
            should_reconnect = true;
        }

        if (is_connected && is_ready && tcp_client_pcb != NULL) {
            err_t wr_err = tcp_write(tcp_client_pcb, test_msg, sizeof(test_msg)-1, TCP_WRITE_FLAG_COPY);
            if (wr_err == ERR_OK) {
                tcp_output(tcp_client_pcb);
                TEST_PRINTF("[tcp] sent test data\n");
            } else {
                TEST_PRINTF("[tcp] tcp_write error: %d\n", wr_err);
            }
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

int lwip_tcp_client_test(int argc, char **argv)
{
    server_ip.addr = inet_addr(TCP_SERVER_IP);
    server_port = TCP_SERVER_PORT;

    TEST_PRINTF("[tcp] target server: %s:%d\n", ipaddr_ntoa(&server_ip), server_port);

    should_reconnect = true;

#if defined(CONFIG_FREERTOS_ENABLE)
    xTaskCreate(tcp_client_task, "tcp_cli", 512, NULL, 3, NULL);
#elif defined(CONFIG_RTTNANO_ENABLE)
    rt_thread_t tid = rt_thread_create("tcp_cli", tcp_client_task, RT_NULL, 1024, RT_THREAD_PRIORITY_MAX / 3, 20);
    if (tid != RT_NULL) {
        rt_thread_startup(tid);
    } else {
        TEST_PRINTF("[tcp] create thread failed\n");
        return -1;
    }
#else
    tcp_client_task(NULL);
#endif

    return 0;
}

/*
    PC ETH (192.168.1.1)  
    sscom (192.168.1.130) start listen port 6004
    TEST BOARD (192.168.1.34)(default) request (192.168.1.130)(6001)

    TEST BOARD request success, send REQUEST
    SSCOM receive REQUEST, send READY

    TEST BOARD received READY, change to send TEST_UDP
*/
