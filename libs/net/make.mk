
ifeq (${MK_USE_NET_LWIP_CORE},y)
PROJ_CINCDIRS += libs/net/lwip/src/include
PROJ_CINCDIRS += libs/net/lwip/src/include/netif
PROJ_CINCDIRS += libs/net/lwip/src/include/compat/posix

# 1. The minimum set of files needed for lwIP.
CSOURCES += libs/net/lwip/src/core/init.c
CSOURCES += libs/net/lwip/src/core/def.c
CSOURCES += libs/net/lwip/src/core/dns.c
CSOURCES += libs/net/lwip/src/core/inet_chksum.c
CSOURCES += libs/net/lwip/src/core/ip.c
CSOURCES += libs/net/lwip/src/core/mem.c
CSOURCES += libs/net/lwip/src/core/memp.c
CSOURCES += libs/net/lwip/src/core/netif.c
CSOURCES += libs/net/lwip/src/core/pbuf.c
CSOURCES += libs/net/lwip/src/core/raw.c
CSOURCES += libs/net/lwip/src/core/stats.c
CSOURCES += libs/net/lwip/src/core/sys.c
CSOURCES += libs/net/lwip/src/core/tcp.c
CSOURCES += libs/net/lwip/src/core/tcp_in.c
CSOURCES += libs/net/lwip/src/core/tcp_out.c
CSOURCES += libs/net/lwip/src/core/timeouts.c
CSOURCES += libs/net/lwip/src/core/udp.c

# 1.1. Altcp
CSOURCES += libs/net/lwip/src/core/altcp.c
CSOURCES += libs/net/lwip/src/core/altcp_alloc.c
CSOURCES += libs/net/lwip/src/core/altcp_tcp.c

# 1.2. using ipv4 by default
CSOURCES += libs/net/lwip/src/core/ipv4/autoip.c
CSOURCES += libs/net/lwip/src/core/ipv4/dhcp.c
CSOURCES += libs/net/lwip/src/core/ipv4/etharp.c
CSOURCES += libs/net/lwip/src/core/ipv4/icmp.c
CSOURCES += libs/net/lwip/src/core/ipv4/igmp.c
CSOURCES += libs/net/lwip/src/core/ipv4/ip4_frag.c
CSOURCES += libs/net/lwip/src/core/ipv4/ip4.c
CSOURCES += libs/net/lwip/src/core/ipv4/ip4_addr.c

ifeq (${MK_USE_NET_LWIP_CORE6},y)
# 1.3. using ipv6
CSOURCES += libs/net/lwip/src/core/ipv6/dhcp6.c
CSOURCES += libs/net/lwip/src/core/ipv6/ethip6.c
CSOURCES += libs/net/lwip/src/core/ipv6/icmp6.c
CSOURCES += libs/net/lwip/src/core/ipv6/inet6.c
CSOURCES += libs/net/lwip/src/core/ipv6/ip6.c
CSOURCES += libs/net/lwip/src/core/ipv6/ip6_addr.c
CSOURCES += libs/net/lwip/src/core/ipv6/ip6_frag.c
CSOURCES += libs/net/lwip/src/core/ipv6/mld6.c
CSOURCES += libs/net/lwip/src/core/ipv6/nd6.c
endif

# 2. APIFILES: The files which implement the sequential and socket APIs.
CSOURCES += libs/net/lwip/src/api/api_lib.c
CSOURCES += libs/net/lwip/src/api/api_msg.c
CSOURCES += libs/net/lwip/src/api/err.c
CSOURCES += libs/net/lwip/src/api/if_api.c
CSOURCES += libs/net/lwip/src/api/netbuf.c
CSOURCES += libs/net/lwip/src/api/netdb.c
CSOURCES += libs/net/lwip/src/api/netifapi.c
CSOURCES += libs/net/lwip/src/api/sockets.c
CSOURCES += libs/net/lwip/src/api/tcpip.c

# 3. Files implementing various generic network interface functions
CSOURCES += libs/net/lwip/src/netif/ethernet.c

# 3.1 Files implementing an IEEE 802.1D bridge by using a multilayer netif approach
# CSOURCES += libs/net/lwip/src/netif/bridgeif.c
# CSOURCES += libs/net/lwip/src/netif/bridgeif_fdb.c

# 3.2 A generic implementation of the SLIP (Serial Line IP) protocol.
# CSOURCES += libs/net/lwip/src/netif/slipif.c

# 4. 6LoWPAN
CSOURCES += libs/net/lwip/src/netif/lowpan6.c

# 4.1 A 6LoWPAN over Bluetooth Low Energy (BLE) implementation as netif,
#           according to RFC-7668.
# CSOURCES += libs/net/lwip/src/netif/lowpan6_ble.c

# 4.2 Common 6LowPAN routines for IPv6.
# CSOURCES += libs/net/lwip/src/netif/lowpan6_common.c

# 4.3 A netif implementing the ZigBee Encapsulation Protocol (ZEP).
# CSOURCES += libs/net/lwip/src/netif/zepif.c

################################################
################  LwIP APP      ################
################################################

ifeq (${MK_USE_NET_LWIP_PPP},y)
# 5. PPP
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

ifeq (${MK_USE_NET_LWIP_SNMP},y)
# 6. SNMPv3 agent
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

# 7. HTTP server + client
# CSOURCES += libs/net/lwip/src/apps/http/altcp_proxyconnect.c
# CSOURCES += libs/net/lwip/src/apps/http/fs.c
# CSOURCES += libs/net/lwip/src/apps/http/http_client.c
# CSOURCES += libs/net/lwip/src/apps/http/httpd.c

# 8. MAKEFSDATA HTTP server host utility
# CSOURCES += libs/net/lwip/src/apps/http/makefsdata/makefsdata.c

# 9. IPERF server
# CSOURCES += libs/net/lwip/src/apps/lwiperf/lwiperf.c

# 10. SMTP client
# CSOURCES += libs/net/lwip/src/apps/smtp/smtp.c

# 11. SNTP client
# CSOURCES += libs/net/lwip/src/apps/sntp/sntp.c

# 12. MDNS responder
# CSOURCES += libs/net/lwip/src/apps/mdns/mdns.c

# 13. NetBIOS name server
# CSOURCES += libs/net/lwip/src/apps/netbiosns/netbiosns.c

# 14. TFTP server files
# CSOURCES += libs/net/lwip/src/apps/tftp/tftp_server.c
# CSOURCES += libs/net/lwip/src/apps/tftp/tftp_port.c

# 15. MQTT client files
# CSOURCES += libs/net/lwip/src/apps/mqtt/mqtt.c

# 16. ARM MBEDTLS related files of lwIP rep
# CSOURCES += libs/net/lwip/src/apps/altcp_tls/altcp_tls_mbedtls.c
# CSOURCES += libs/net/lwip/src/apps/altcp_tls/altcp_tls_mbedtls_mem.c
# CSOURCES += libs/net/lwip/src/apps/snmp/snmpv3_mbedtls.c

# 17. ping
ifeq (${MK_USE_NET_LWIP_PING},y)
CSOURCES += libs/net/lwip/src/apps/ping/ping.c
endif

################################################
################  LwIP Port     ################
################################################

PROJ_CINCDIRS += libs/net/port
CSOURCES += libs/net/port/sys_arch.c
CSOURCES += libs/net/port/ethernetif.c

endif

