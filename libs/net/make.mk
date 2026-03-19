
MK_USE_NEW_LWIP_ICMP=y

ifeq (${MK_USE_NET_LWIP_CORE},y)
PROJ_CDEFS += CONFIG_NET_LWIP_ENABLE
PROJ_CDEFS += CONFIG_LWIP_TCP
PROJ_CDEFS += CONFIG_LWIP_UDP
PROJ_CINCDIRS += libs/net/lwip/src/include
PROJ_CINCDIRS += libs/net/lwip/src/include/netif
PROJ_CINCDIRS += libs/net/lwip/src/include/compat/posix

# ===========================================================================
# 0. lwip port layer
# ===========================================================================
PROJ_CINCDIRS += libs/net/port
CSOURCES += libs/net/port/sys_arch.c
CSOURCES += libs/net/port/ethernetif.c

PROJ_CINCDIRS += libs/net/test
CSOURCES += libs/net/test/lwip_udp_test.c
CSOURCES += libs/net/test/lwip_tcp_server_test.c
CSOURCES += libs/net/test/lwip_tcp_client_test.c

# ===========================================================================
# 1. core(mini system): ipv4 + tcp + udp + arp + mem
# ===========================================================================
CORE_SOURCES += libs/net/lwip/src/core/init.c
CORE_SOURCES += libs/net/lwip/src/core/def.c 
CORE_SOURCES += libs/net/lwip/src/core/inet_chksum.c
CORE_SOURCES += libs/net/lwip/src/core/ip.c
# CORE_SOURCES += libs/net/lwip/src/core/mem.c
CORE_SOURCES += libs/net/lwip/src/core/memp.c
CORE_SOURCES += libs/net/lwip/src/core/netif.c
CORE_SOURCES += libs/net/lwip/src/core/pbuf.c
CORE_SOURCES += libs/net/lwip/src/core/sys.c
CORE_SOURCES += libs/net/lwip/src/core/timeouts.c
CORE_SOURCES += libs/net/lwip/src/core/tcp.c
CORE_SOURCES += libs/net/lwip/src/core/tcp_in.c
CORE_SOURCES += libs/net/lwip/src/core/tcp_out.c
CORE_SOURCES += libs/net/lwip/src/core/udp.c
CORE_SOURCES += libs/net/lwip/src/core/ipv4/ip4.c
CORE_SOURCES += libs/net/lwip/src/core/ipv4/ip4_addr.c
CORE_SOURCES += libs/net/lwip/src/core/ipv4/etharp.c
CSOURCES += $(CORE_SOURCES)

# ===========================================================================
# 2. core module
# ===========================================================================
###### DNS
ifeq ($(MK_USE_NEW_LWIP_DNS),y)
PROJ_CDEFS += CONFIG_LWIP_DNS
CSOURCES += libs/net/lwip/src/core/dns.c
endif

###### RAW socket
ifeq ($(MK_USE_NEW_LWIP_RAW),y)
PROJ_CDEFS += CONFIG_LWIP_RAW
CSOURCES += libs/net/lwip/src/core/raw.c
endif

###### IGMP 
ifeq ($(MK_USE_NEW_LWIP_IGMP),y)
PROJ_CDEFS += CONFIG_LWIP_IGMP
CSOURCES += libs/net/lwip/src/core/ipv4/igmp.c
endif

###### ICMP 
ifeq ($(MK_USE_NEW_LWIP_ICMP),y)
PROJ_CDEFS += CONFIG_LWIP_ICMP
CSOURCES += libs/net/lwip/src/core/ipv4/icmp.c
endif

###### DHCP
ifeq ($(MK_USE_NEW_LWIP_DHCP),y)
PROJ_CDEFS += CONFIG_LWIP_DHCP
CSOURCES += libs/net/lwip/src/core/ipv4/dhcp.c
endif

###### AUTOIP
ifeq ($(MK_USE_NEW_LWIP_AUTOIP),y)
PROJ_CDEFS += CONFIG_LWIP_AUTOIP
CSOURCES += libs/net/lwip/src/core/ipv4/autoip.c
endif

###### IP fragmentation & reassembly
ifeq ($(MK_USE_NEW_LWIP_REASSEMBLY_FRAG),y)
PROJ_CDEFS += CONFIG_LWIP_REASSEMBLY_FRAG
CSOURCES += libs/net/lwip/src/core/ipv4/ip4_frag.c
endif

###### statistics function
ifeq ($(MK_USE_NEW_LWIP_STATS),y)
PROJ_CDEFS += CONFIG_LWIP_STATS
CSOURCES += libs/net/lwip/src/core/stats.c
endif

###### ALTCP (available TLS layer)
ifeq ($(MK_USE_NEW_LWIP_ALTCP),y)
PROJ_CDEFS += CONFIG_LWIP_ALTCP
CSOURCES += libs/net/lwip/src/core/altcp.c
CSOURCES += libs/net/lwip/src/core/altcp_alloc.c
CSOURCES += libs/net/lwip/src/core/altcp_tcp.c
endif

# ===========================================================================
# 3. ipv6 support (master switch, submodules optional)
# ===========================================================================
ifeq (${MK_USE_NET_LWIP_CORE6},y)
PROJ_CDEFS += CONFIG_LWIP_IPV6
# Core IPv6 files (always needed when IPv6 is enabled)
CSOURCES += libs/net/lwip/src/core/ipv6/inet6.c
CSOURCES += libs/net/lwip/src/core/ipv6/ip6.c
CSOURCES += libs/net/lwip/src/core/ipv6/ip6_addr.c
CSOURCES += libs/net/lwip/src/core/ipv6/nd6.c          # Neighbor discovery
CSOURCES += libs/net/lwip/src/core/ipv6/icmp6.c        # ICMPv6 (required for ND)
# DHCPv6
CSOURCES += libs/net/lwip/src/core/ipv6/dhcp6.c
# MLDv6 (multicast listener discovery)
CSOURCES += libs/net/lwip/src/core/ipv6/mld6.c
# IPv6 fragmentation
CSOURCES += libs/net/lwip/src/core/ipv6/ip6_frag.c
# IPv6 over Ethernet (for ethernet interfaces)
CSOURCES += libs/net/lwip/src/core/ipv6/ethip6.c
endif

# ===========================================================================
# 4. Sequential and socket APIs (optional, enabled by default)
# ===========================================================================
ifeq ($(MK_USE_NET_LWIP_SOCKET),n)
# Socket API explicitly disabled
else
CSOURCES += libs/net/lwip/src/api/api_lib.c
CSOURCES += libs/net/lwip/src/api/api_msg.c
CSOURCES += libs/net/lwip/src/api/err.c
CSOURCES += libs/net/lwip/src/api/if_api.c
CSOURCES += libs/net/lwip/src/api/netbuf.c
CSOURCES += libs/net/lwip/src/api/netdb.c
CSOURCES += libs/net/lwip/src/api/netifapi.c
CSOURCES += libs/net/lwip/src/api/sockets.c
CSOURCES += libs/net/lwip/src/api/tcpip.c
endif

# ===========================================================================
# 5. Generic network interface (ethernet, always needed for Ethernet drivers)
# ===========================================================================
CSOURCES += libs/net/lwip/src/netif/ethernet.c

# ===========================================================================
# 6. Optional netif drivers (controlled by MK_USE_NET_LWIP_*)
# ===========================================================================
# IEEE 802.1D bridge
ifeq ($(MK_USE_NET_LWIP_BRIDGEIF),y)
CSOURCES += libs/net/lwip/src/netif/bridgeif.c
CSOURCES += libs/net/lwip/src/netif/bridgeif_fdb.c
endif

# SLIP (Serial Line IP)
ifeq ($(MK_USE_NET_LWIP_SLIPIF),y)
CSOURCES += libs/net/lwip/src/netif/slipif.c
endif

# 6LoWPAN (IPv6 over Low-Power Wireless Personal Area Networks)
ifeq ($(MK_USE_NET_LWIP_LOWPAN6),y)
CSOURCES += libs/net/lwip/src/netif/lowpan6.c
endif

# 6LoWPAN over BLE (RFC 7668)
ifeq ($(MK_USE_NET_LWIP_LOWPAN6_BLE),y)
CSOURCES += libs/net/lwip/src/netif/lowpan6_ble.c
CSOURCES += libs/net/lwip/src/netif/lowpan6_common.c
endif

# ZigBee Encapsulation Protocol (ZEP)
ifeq ($(MK_USE_NET_LWIP_ZEPIF),y)
CSOURCES += libs/net/lwip/src/netif/zepif.c
endif

# ===========================================================================
# 7. LwIP applications (optional)
# ===========================================================================
# PPP
ifeq (${MK_USE_NET_LWIP_PPP},y)
PROJ_CDEFS += CONFIG_LWIP_PPP
PROJ_CDEFS += CONFIG_LWIP_PPPOE
PROJ_CDEFS += CONFIG_LWIP_PPPOS
PROJ_CINCDIRS += libs/net/lwip/src/netif/ppp
CSOURCES += libs/net/lwip/src/netif/ppp/auth.c
CSOURCES += libs/net/lwip/src/netif/ppp/ccp.c
CSOURCES += libs/net/lwip/src/netif/ppp/chap-md5.c
CSOURCES += libs/net/lwip/src/netif/ppp/chap_ms.c
CSOURCES += libs/net/lwip/src/netif/ppp/chap-new.c
CSOURCES += libs/net/lwip/src/netif/ppp/demand.c
CSOURCES += libs/net/lwip/src/netif/ppp/eap.c
CSOURCES += libs/net/lwip/src/netif/ppp/ecp.c
CSOURCES += libs/net/lwip/src/netif/ppp/eui64.c
CSOURCES += libs/net/lwip/src/netif/ppp/fsm.c
CSOURCES += libs/net/lwip/src/netif/ppp/ipcp.c
CSOURCES += libs/net/lwip/src/netif/ppp/ipv6cp.c
CSOURCES += libs/net/lwip/src/netif/ppp/lcp.c
CSOURCES += libs/net/lwip/src/netif/ppp/magic.c
CSOURCES += libs/net/lwip/src/netif/ppp/mppe.c
CSOURCES += libs/net/lwip/src/netif/ppp/multilink.c
CSOURCES += libs/net/lwip/src/netif/ppp/ppp.c
CSOURCES += libs/net/lwip/src/netif/ppp/pppapi.c
CSOURCES += libs/net/lwip/src/netif/ppp/pppcrypt.c
CSOURCES += libs/net/lwip/src/netif/ppp/pppoe.c
CSOURCES += libs/net/lwip/src/netif/ppp/pppol2tp.c
CSOURCES += libs/net/lwip/src/netif/ppp/pppos.c
CSOURCES += libs/net/lwip/src/netif/ppp/upap.c
CSOURCES += libs/net/lwip/src/netif/ppp/utils.c
CSOURCES += libs/net/lwip/src/netif/ppp/vj.c
CSOURCES += libs/net/lwip/src/netif/ppp/polarssl/arc4.c
CSOURCES += libs/net/lwip/src/netif/ppp/polarssl/des.c
CSOURCES += libs/net/lwip/src/netif/ppp/polarssl/md4.c
CSOURCES += libs/net/lwip/src/netif/ppp/polarssl/md5.c
CSOURCES += libs/net/lwip/src/netif/ppp/polarssl/sha1.c
endif

# SNMPv3 agent
ifeq (${MK_USE_NET_LWIP_SNMP},y)
PROJ_CDEFS += CONFIG_LWIP_SNMP
PROJ_CINCDIRS += libs/net/lwip/src/apps/snmp
CSOURCES += libs/net/lwip/src/apps/snmp/snmp_asn1.c
CSOURCES += libs/net/lwip/src/apps/snmp/snmp_core.c
CSOURCES += libs/net/lwip/src/apps/snmp/snmp_mib2.c
CSOURCES += libs/net/lwip/src/apps/snmp/snmp_mib2_icmp.c
CSOURCES += libs/net/lwip/src/apps/snmp/snmp_mib2_interfaces.c
CSOURCES += libs/net/lwip/src/apps/snmp/snmp_mib2_ip.c
CSOURCES += libs/net/lwip/src/apps/snmp/snmp_mib2_snmp.c
CSOURCES += libs/net/lwip/src/apps/snmp/snmp_mib2_system.c
CSOURCES += libs/net/lwip/src/apps/snmp/snmp_mib2_tcp.c
CSOURCES += libs/net/lwip/src/apps/snmp/snmp_mib2_udp.c
CSOURCES += libs/net/lwip/src/apps/snmp/snmp_snmpv2_framework.c
CSOURCES += libs/net/lwip/src/apps/snmp/snmp_snmpv2_usm.c
CSOURCES += libs/net/lwip/src/apps/snmp/snmp_msg.c
CSOURCES += libs/net/lwip/src/apps/snmp/snmpv3.c
CSOURCES += libs/net/lwip/src/apps/snmp/snmp_netconn.c
CSOURCES += libs/net/lwip/src/apps/snmp/snmp_pbuf_stream.c
CSOURCES += libs/net/lwip/src/apps/snmp/snmp_raw.c
CSOURCES += libs/net/lwip/src/apps/snmp/snmp_scalar.c
CSOURCES += libs/net/lwip/src/apps/snmp/snmp_table.c
CSOURCES += libs/net/lwip/src/apps/snmp/snmp_threadsync.c
CSOURCES += libs/net/lwip/src/apps/snmp/snmp_traps.c
endif

# HTTP server + client
ifeq ($(MK_USE_NET_LWIP_HTTP),y)
CSOURCES += libs/net/lwip/src/apps/http/altcp_proxyconnect.c
CSOURCES += libs/net/lwip/src/apps/http/fs.c
CSOURCES += libs/net/lwip/src/apps/http/http_client.c
CSOURCES += libs/net/lwip/src/apps/http/httpd.c
endif

# IPERF server
ifeq ($(MK_USE_NET_LWIP_IPERF),y)
CSOURCES += libs/net/lwip/src/apps/lwiperf/lwiperf.c
endif

# SMTP client
ifeq ($(MK_USE_NET_LWIP_SMTP),y)
CSOURCES += libs/net/lwip/src/apps/smtp/smtp.c
endif

# SNTP client
ifeq ($(MK_USE_NET_LWIP_SNTP),y)
CSOURCES += libs/net/lwip/src/apps/sntp/sntp.c
endif

# MDNS responder
ifeq ($(MK_USE_NET_LWIP_MDNS),y)
CSOURCES += libs/net/lwip/src/apps/mdns/mdns.c
endif

# NetBIOS name server
ifeq ($(MK_USE_NET_LWIP_NETBIOSNS),y)
CSOURCES += libs/net/lwip/src/apps/netbiosns/netbiosns.c
endif

# TFTP server
ifeq ($(MK_USE_NET_LWIP_TFTP),y)
CSOURCES += libs/net/lwip/src/apps/tftp/tftp_server.c
CSOURCES += libs/net/lwip/src/apps/tftp/tftp_port.c
endif

# MQTT client
ifeq ($(MK_USE_NET_LWIP_MQTT),y)
CSOURCES += libs/net/lwip/src/apps/mqtt/mqtt.c
endif

# ALTCP with mbedTLS
ifeq ($(MK_USE_NET_LWIP_ALTCP_TLS),y)
CSOURCES += libs/net/lwip/src/apps/altcp_tls/altcp_tls_mbedtls.c
CSOURCES += libs/net/lwip/src/apps/altcp_tls/altcp_tls_mbedtls_mem.c
endif

# SNMPv3 with mbedTLS
ifeq ($(MK_USE_NET_LWIP_SNMP_MBEDTLS),y)
CSOURCES += libs/net/lwip/src/apps/snmp/snmpv3_mbedtls.c
endif

# Ping utility
ifeq ($(MK_USE_NET_LWIP_PING),y)
CSOURCES += libs/net/lwip/src/apps/ping/ping.c
endif

endif
