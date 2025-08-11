#include <lwip/init.h>
#include <lwip/opt.h>
#include <lwip/debug.h>
#include <lwip/def.h>
#include <lwip/mem.h>
#include <lwip/pbuf.h>
#include <lwip/sys.h>
#include <lwip/netif.h>
#include <lwip/stats.h>
#include <lwip/tcpip.h>
#include <lwip/dhcp.h>
#include <lwip/netifapi.h>
#include <lwip/inet.h>
#include <netif/etharp.h>

#include <netif/ethernetif.h>
#include <string.h>

#if LWIP_IPV6
#include "lwip/ethip6.h"
#endif

#if LWIP_NETIF_HOSTNAME
#define LWIP_HOSTNAME_LEN 16
#endif

#define netifapi_netif_set_link_up(n)      netifapi_netif_common(n, netif_set_link_up, NULL)
#define netifapi_netif_set_link_down(n)    netifapi_netif_common(n, netif_set_link_down, NULL)

static struct netif _g_netif;

static err_t ethernetif_linkoutput(struct netif *netif, struct pbuf *p)
{
    struct eth_device* enetif;

    enetif = (struct eth_device*)netif->state;

    if (enetif->eth_tx(p) != 0) {
        return -1;
    }

    return 0;
}

static err_t eth_netif_device_init(struct netif *netif)
{
    struct eth_device *ethif;

    ethif = (struct eth_device*)netif->state;

    if (ethif != NULL) {

        if (ethif->ops.init() != 0) {
            return ERR_IF;
        }

        /* copy device flags to netif flags */
        netif->flags = (ethif->flags & 0xff);
        netif->mtu = ETHERNET_MTU;

        /* set output */
        netif->output       = etharp_output;

#if LWIP_IPV6
        netif->output_ip6 = ethip6_output;
        netif->ip6_autoconfig_enabled = 1;
        netif_create_ip6_linklocal_address(netif, 1);

#if LWIP_IPV6_MLD
        netif->flags |= NETIF_FLAG_MLD6;

        /*
        * For hardware/netifs that implement MAC filtering.
        * All-nodes link-local is handled by default, so we must let the hardware know
        * to allow multicast packets in.
        * Should set mld_mac_filter previously. */
        if (netif->mld_mac_filter != NULL)
        {
            ip6_addr_t ip6_allnodes_ll;
            ip6_addr_set_allnodes_linklocal(&ip6_allnodes_ll);
            netif->mld_mac_filter(netif, &ip6_allnodes_ll, NETIF_ADD_MAC_FILTER);
        }
#endif /* LWIP_IPV6_MLD */

#endif /* LWIP_IPV6 */

        /* set default netif */
        if (netif_default == NULL) {
            netif_set_default(netif);
        }

        /* set interface up */
        netif_set_up(netif);

#if LWIP_DHCP
        /* if this interface uses DHCP, start the DHCP client */
        dhcp_start(netif);
#endif

        if (ethif->flags & ETHIF_LINK_PHYUP)
        {
            /* set link_up for this netif */
            netif_set_link_up(netif);
        }

        return ERR_OK;
    }

    return ERR_IF;
}

int eth_device_ready(struct eth_device* dev)
{
    err_t err;
    struct pbuf *p;
    struct netif* netif = dev->netif;

    if (netif) {
        /* move received packet into a new pbuf */
        p = dev->eth_rx();

        if (p == NULL) {
            return -2; 
        }

        /* entry point to the LwIP stack */
        err = netif->input(p, netif);

        if (err != ERR_OK) {
            pbuf_free(p);
            p = NULL;    
        }

    } else {
        /* netif is not initialized yet, just return. */
        return -1; 
    }
}

/* Keep old drivers compatible in RT-Thread */
int eth_device_init_with_flag(struct eth_device *dev, const char *name, uint16_t flags)
{
    struct netif* netif = &_g_netif;

// #if LWIP_NETIF_HOSTNAME
//     char *hostname = RT_NULL;
//     netif = (struct netif*) rt_calloc (1, sizeof(struct netif) + LWIP_HOSTNAME_LEN);
// #else
//     netif = (struct netif*) rt_calloc (1, sizeof(struct netif));
// #endif

    netif = &_g_netif;

    /* set netif */
    dev->netif = netif;
    dev->flags = flags;
    /* link changed status of device */
    dev->link_changed = 0x00;
    /* avoid send the same mail to mailbox */
    dev->rx_notice = 0x00;

    /* set name */
    strncpy(netif->name, name, NETIF_NAMESIZE);

    /* set hw address to 6 */
    netif->hwaddr_len   = 6;
    /* maximum transfer unit */
    netif->mtu          = ETHERNET_MTU;

    /* set linkoutput */
    netif->linkoutput   = ethernetif_linkoutput;

    /* get hardware MAC address */
    dev->ops.control(NIOCTL_GADDR, netif->hwaddr);

// #if LWIP_NETIF_HOSTNAME
//     /* Initialize interface hostname */
//     hostname = (char *)netif + sizeof(struct netif);
//     sprintf(hostname, "rtthread_%02x%02x", name[0], name[1]);
//     netif->hostname = hostname;
// #endif

    /* if tcp thread has been started up, we add this netif to the system */
    if (1 /* rt_thread_find("tcpip") != RT_NULL */)
    {
        ip4_addr_t ipaddr, netmask, gw;

#if !LWIP_DHCP
        ipaddr.addr = inet_addr("192.168.75.34");
        gw.addr = inet_addr("192.168.75.1");
        netmask.addr = inet_addr("255.255.255.0");
#else
        IP4_ADDR(&ipaddr, 0, 0, 0, 0);
        IP4_ADDR(&gw, 0, 0, 0, 0);
        IP4_ADDR(&netmask, 0, 0, 0, 0);
#endif
        netif_add(netif, &ipaddr, &netmask, &gw, dev, eth_netif_device_init, ethernet_input);

        printf("localhost ip:%ld.%ld.%ld.%ld\n\n",  \
            ((netif->ip_addr.addr)&0x000000ff),       \
            (((netif->ip_addr.addr)&0x0000ff00)>>8),  \
            (((netif->ip_addr.addr)&0x00ff0000)>>16), \
            ((netif->ip_addr.addr)&0xff000000)>>24);

    }

    return 0;
}

int eth_device_init(struct eth_device * dev, const char *name)
{
    uint16_t flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

#if LWIP_IGMP
    /* IGMP support */
    flags |= NETIF_FLAG_IGMP;
#endif

    lwip_init();

    return eth_device_init_with_flag(dev, name, flags);
}

void eth_device_deinit(struct eth_device *dev)
{
    struct netif* netif = dev->netif;

#if LWIP_DHCP
    dhcp_stop(netif);
    dhcp_cleanup(netif);
#endif
    netif_set_down(netif);
    netif_remove(netif);
}


/* this function does not need,
 * use eth_system_device_init_private()
 * call by lwip_system_init().
 */
int eth_system_device_init(void)
{
    return 0;
}

int eth_system_device_init_private(void)
{
    int result = 0;

    return (int)result;
}

static void arp_timer(void *arg)
{
    etharp_tmr();
    sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);
}

uint32_t sys_now(void)
{
    return HAL_GetTick();
}
