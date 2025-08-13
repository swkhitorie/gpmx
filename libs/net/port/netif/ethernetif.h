#ifndef __NETIF_ETHERNETIF_H__
#define __NETIF_ETHERNETIF_H__

#include "lwip/netif.h"

#ifndef LWIP_ETHERNET_DEFAULT_IP
#define LWIP_ETHERNET_DEFAULT_IP          "192.168.75.34"
#endif

#ifndef LWIP_ETHERNET_DEFAULT_GATEWAY
#define LWIP_ETHERNET_DEFAULT_GATEWAY     "192.168.75.1"
#endif

#ifndef LWIP_ETHERNET_DEFAULT_MASK
#define LWIP_ETHERNET_DEFAULT_MASK        "255.255.255.0"
#endif

#define NETIF_LOG(...)    printf(__VA_ARGS__)

#define NIOCTL_GADDR        0x01
#define ETHERNET_MTU        1500

/* eth flag with auto_linkup or phy_linkup */
#define ETHIF_LINK_AUTOUP   0x0000
#define ETHIF_LINK_PHYUP    0x0100

struct net_device {
    int  (*init)   (void);
    int  (*control)(int cmd, void *args);
};

struct eth_device {

    struct net_device ops;

    /* network interface for lwip */
    struct netif *netif;

    uint16_t flags;
    uint8_t  link_changed;
    uint8_t  link_status;
    uint8_t  rx_notice;

    /* eth device interface */
    struct pbuf* (*eth_rx)(void);
    int (*eth_tx)(struct pbuf* p);
};

#ifdef __cplusplus
extern "C" {
#endif

int  eth_system_device_init(void);
void eth_device_deinit(struct eth_device *dev);
int  eth_device_ready(struct eth_device* dev);
int  eth_device_init(struct eth_device * dev, const char *name);
int  eth_device_init_with_flag(struct eth_device *dev, const char *name, uint16_t flag);
int  eth_device_linkchange(struct eth_device* dev, int up);

#ifdef __cplusplus
}
#endif

#endif 
