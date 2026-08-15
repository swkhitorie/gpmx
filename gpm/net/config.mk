
ifeq (${CONFIG_LWIP_CORE},y)
CONFIG_LWIP_ICMP=y

PROJ_CINCDIRS += ${GPMPATH}/net/lwip/src/include
PROJ_CINCDIRS += ${GPMPATH}/net/lwip/src/include/netif
PROJ_CINCDIRS += ${GPMPATH}/net/lwip/src/include/compat/posix

# ===========================================================================
# 0. lwip port layer
# ===========================================================================
PROJ_CINCDIRS += ${GPMPATH}/net/port
CSOURCES += ${GPMPATH}/net/port/sys_arch.c
CSOURCES += ${GPMPATH}/net/port/ethernetif.c

PROJ_CINCDIRS += ${GPMPATH}/net/test
CSOURCES += ${GPMPATH}/net/test/lwip_udp_test.c
CSOURCES += ${GPMPATH}/net/test/lwip_tcp_server_test.c
CSOURCES += ${GPMPATH}/net/test/lwip_tcp_client_test.c

# ===========================================================================
# 1. core(mini system): ipv4 + tcp + udp + arp + mem
# ===========================================================================
CORE_SOURCES += ${GPMPATH}/net/lwip/src/core/init.c
CORE_SOURCES += ${GPMPATH}/net/lwip/src/core/def.c 
CORE_SOURCES += ${GPMPATH}/net/lwip/src/core/inet_chksum.c
CORE_SOURCES += ${GPMPATH}/net/lwip/src/core/ip.c
# CORE_SOURCES += ${GPMPATH}/net/lwip/src/core/mem.c
CORE_SOURCES += ${GPMPATH}/net/lwip/src/core/memp.c
CORE_SOURCES += ${GPMPATH}/net/lwip/src/core/netif.c
CORE_SOURCES += ${GPMPATH}/net/lwip/src/core/pbuf.c
CORE_SOURCES += ${GPMPATH}/net/lwip/src/core/sys.c
CORE_SOURCES += ${GPMPATH}/net/lwip/src/core/timeouts.c
CORE_SOURCES += ${GPMPATH}/net/lwip/src/core/tcp.c
CORE_SOURCES += ${GPMPATH}/net/lwip/src/core/tcp_in.c
CORE_SOURCES += ${GPMPATH}/net/lwip/src/core/tcp_out.c
CORE_SOURCES += ${GPMPATH}/net/lwip/src/core/udp.c
CORE_SOURCES += ${GPMPATH}/net/lwip/src/core/ipv4/ip4.c
CORE_SOURCES += ${GPMPATH}/net/lwip/src/core/ipv4/ip4_addr.c
CORE_SOURCES += ${GPMPATH}/net/lwip/src/core/ipv4/etharp.c
CSOURCES += $(CORE_SOURCES)

# ===========================================================================
# 2. core module
# ===========================================================================
###### DNS
ifeq ($(CONFIG_LWIP_DNS),y)
CSOURCES += ${GPMPATH}/net/lwip/src/core/dns.c
endif

###### RAW socket
ifeq ($(CONFIG_LWIP_RAW),y)
CSOURCES += ${GPMPATH}/net/lwip/src/core/raw.c
endif

###### IGMP 
ifeq ($(CONFIG_LWIP_IGMP),y)
CSOURCES += ${GPMPATH}/net/lwip/src/core/ipv4/igmp.c
endif

###### ICMP 
ifeq ($(CONFIG_LWIP_ICMP),y)
CSOURCES += ${GPMPATH}/net/lwip/src/core/ipv4/icmp.c
endif

###### DHCP
ifeq ($(CONFIG_LWIP_DHCP),y)
CSOURCES += ${GPMPATH}/net/lwip/src/core/ipv4/dhcp.c
endif

###### AUTOIP
ifeq ($(CONFIG_LWIP_AUTOIP),y)
CSOURCES += ${GPMPATH}/net/lwip/src/core/ipv4/autoip.c
endif

###### IP fragmentation & reassembly
ifeq ($(CONFIG_LWIP_REASSEMBLY_FRAG),y)
CSOURCES += ${GPMPATH}/net/lwip/src/core/ipv4/ip4_frag.c
endif

###### statistics function
ifeq ($(CONFIG_LWIP_STATS),y)
CSOURCES += ${GPMPATH}/net/lwip/src/core/stats.c
endif

###### ALTCP (available TLS layer)
ifeq ($(CONFIG_LWIP_ALTCP),y)
CSOURCES += ${GPMPATH}/net/lwip/src/core/altcp.c
CSOURCES += ${GPMPATH}/net/lwip/src/core/altcp_alloc.c
CSOURCES += ${GPMPATH}/net/lwip/src/core/altcp_tcp.c
endif

# ===========================================================================
# 3. ipv6 support (master switch, submodules optional)
# ===========================================================================
ifeq (${CONFIG_LWIP_CORE6},y)
# Core IPv6 files (always needed when IPv6 is enabled)
CSOURCES += ${GPMPATH}/net/lwip/src/core/ipv6/inet6.c
CSOURCES += ${GPMPATH}/net/lwip/src/core/ipv6/ip6.c
CSOURCES += ${GPMPATH}/net/lwip/src/core/ipv6/ip6_addr.c
CSOURCES += ${GPMPATH}/net/lwip/src/core/ipv6/nd6.c          # Neighbor discovery
CSOURCES += ${GPMPATH}/net/lwip/src/core/ipv6/icmp6.c        # ICMPv6 (required for ND)
# DHCPv6
CSOURCES += ${GPMPATH}/net/lwip/src/core/ipv6/dhcp6.c
# MLDv6 (multicast listener discovery)
CSOURCES += ${GPMPATH}/net/lwip/src/core/ipv6/mld6.c
# IPv6 fragmentation
CSOURCES += ${GPMPATH}/net/lwip/src/core/ipv6/ip6_frag.c
# IPv6 over Ethernet (for ethernet interfaces)
CSOURCES += ${GPMPATH}/net/lwip/src/core/ipv6/ethip6.c
endif

# ===========================================================================
# 4. Sequential and socket APIs (optional, enabled by default)
# ===========================================================================
ifeq ($(CONFIG_LWIP_SOCKET),n)
# Socket API explicitly disabled
else
CSOURCES += ${GPMPATH}/net/lwip/src/api/api_lib.c
CSOURCES += ${GPMPATH}/net/lwip/src/api/api_msg.c
CSOURCES += ${GPMPATH}/net/lwip/src/api/err.c
CSOURCES += ${GPMPATH}/net/lwip/src/api/if_api.c
CSOURCES += ${GPMPATH}/net/lwip/src/api/netbuf.c
CSOURCES += ${GPMPATH}/net/lwip/src/api/netdb.c
CSOURCES += ${GPMPATH}/net/lwip/src/api/netifapi.c
CSOURCES += ${GPMPATH}/net/lwip/src/api/sockets.c
CSOURCES += ${GPMPATH}/net/lwip/src/api/tcpip.c
endif

# ===========================================================================
# 5. Generic network interface (ethernet, always needed for Ethernet drivers)
# ===========================================================================
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ethernet.c

# ===========================================================================
# 6. Optional netif drivers (controlled by CONFIG_LWIP_*)
# ===========================================================================
# IEEE 802.1D bridge
ifeq ($(CONFIG_LWIP_BRIDGEIF),y)
CSOURCES += ${GPMPATH}/net/lwip/src/netif/bridgeif.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/bridgeif_fdb.c
endif

# SLIP (Serial Line IP)
ifeq ($(CONFIG_LWIP_SLIPIF),y)
CSOURCES += ${GPMPATH}/net/lwip/src/netif/slipif.c
endif

# 6LoWPAN (IPv6 over Low-Power Wireless Personal Area Networks)
ifeq ($(CONFIG_LWIP_LOWPAN6),y)
CSOURCES += ${GPMPATH}/net/lwip/src/netif/lowpan6.c
endif

# 6LoWPAN over BLE (RFC 7668)
ifeq ($(CONFIG_LWIP_LOWPAN6_BLE),y)
CSOURCES += ${GPMPATH}/net/lwip/src/netif/lowpan6_ble.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/lowpan6_common.c
endif

# ZigBee Encapsulation Protocol (ZEP)
ifeq ($(CONFIG_LWIP_ZEPIF),y)
CSOURCES += ${GPMPATH}/net/lwip/src/netif/zepif.c
endif

# ===========================================================================
# 7. LwIP applications (optional)
# ===========================================================================
# PPP
ifeq (${CONFIG_LWIP_PPP},y)
PROJ_CINCDIRS += ${GPMPATH}/net/lwip/src/netif/ppp
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/auth.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/ccp.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/chap-md5.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/chap_ms.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/chap-new.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/demand.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/eap.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/ecp.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/eui64.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/fsm.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/ipcp.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/ipv6cp.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/lcp.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/magic.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/mppe.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/multilink.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/ppp.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/pppapi.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/pppcrypt.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/pppoe.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/pppol2tp.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/pppos.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/upap.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/utils.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/vj.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/polarssl/arc4.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/polarssl/des.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/polarssl/md4.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/polarssl/md5.c
CSOURCES += ${GPMPATH}/net/lwip/src/netif/ppp/polarssl/sha1.c
endif

# SNMPv3 agent
ifeq (${CONFIG_LWIP_SNMP},y)
PROJ_CINCDIRS += ${GPMPATH}/net/lwip/src/apps/snmp
CSOURCES += ${GPMPATH}/net/lwip/src/apps/snmp/snmp_asn1.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/snmp/snmp_core.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/snmp/snmp_mib2.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/snmp/snmp_mib2_icmp.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/snmp/snmp_mib2_interfaces.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/snmp/snmp_mib2_ip.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/snmp/snmp_mib2_snmp.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/snmp/snmp_mib2_system.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/snmp/snmp_mib2_tcp.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/snmp/snmp_mib2_udp.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/snmp/snmp_snmpv2_framework.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/snmp/snmp_snmpv2_usm.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/snmp/snmp_msg.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/snmp/snmpv3.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/snmp/snmp_netconn.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/snmp/snmp_pbuf_stream.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/snmp/snmp_raw.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/snmp/snmp_scalar.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/snmp/snmp_table.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/snmp/snmp_threadsync.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/snmp/snmp_traps.c
endif

# HTTP server + client
ifeq ($(CONFIG_LWIP_HTTP),y)
CSOURCES += ${GPMPATH}/net/lwip/src/apps/http/altcp_proxyconnect.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/http/fs.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/http/http_client.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/http/httpd.c
endif

# IPERF server
ifeq ($(CONFIG_LWIP_IPERF),y)
CSOURCES += ${GPMPATH}/net/lwip/src/apps/lwiperf/lwiperf.c
endif

# SMTP client
ifeq ($(CONFIG_LWIP_SMTP),y)
CSOURCES += ${GPMPATH}/net/lwip/src/apps/smtp/smtp.c
endif

# SNTP client
ifeq ($(CONFIG_LWIP_SNTP),y)
CSOURCES += ${GPMPATH}/net/lwip/src/apps/sntp/sntp.c
endif

# MDNS responder
ifeq ($(CONFIG_LWIP_MDNS),y)
CSOURCES += ${GPMPATH}/net/lwip/src/apps/mdns/mdns.c
endif

# NetBIOS name server
ifeq ($(CONFIG_LWIP_NETBIOSNS),y)
CSOURCES += ${GPMPATH}/net/lwip/src/apps/netbiosns/netbiosns.c
endif

# TFTP server
ifeq ($(CONFIG_LWIP_TFTP),y)
CSOURCES += ${GPMPATH}/net/lwip/src/apps/tftp/tftp_server.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/tftp/tftp_port.c
endif

# MQTT client
ifeq ($(CONFIG_LWIP_MQTT),y)
CSOURCES += ${GPMPATH}/net/lwip/src/apps/mqtt/mqtt.c
endif

# ALTCP with mbedTLS
ifeq ($(CONFIG_LWIP_ALTCP_TLS),y)
CSOURCES += ${GPMPATH}/net/lwip/src/apps/altcp_tls/altcp_tls_mbedtls.c
CSOURCES += ${GPMPATH}/net/lwip/src/apps/altcp_tls/altcp_tls_mbedtls_mem.c
endif

# SNMPv3 with mbedTLS
ifeq ($(CONFIG_LWIP_SNMP_MBEDTLS),y)
CSOURCES += ${GPMPATH}/net/lwip/src/apps/snmp/snmpv3_mbedtls.c
endif

# Ping utility
ifeq ($(CONFIG_LWIP_PING),y)
CSOURCES += ${GPMPATH}/net/lwip/src/apps/ping/ping.c
endif

endif #### end with CONFIG_LWIP_CORE
