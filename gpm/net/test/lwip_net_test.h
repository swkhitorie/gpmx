#ifndef LWIP_NET_TEST_H_
#define LWIP_NET_TEST_H_

#include <board_config.h>

#ifndef TEST_PRINTF
#define TEST_PRINTF    BOARD_PRINTF
#endif

#ifdef __cplusplus
extern "C" {
#endif

int lwip_udp_test(int argc, char **argv);
int lwip_tcp_server_test(int argc, char **argv);
int lwip_tcp_client_test(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif
