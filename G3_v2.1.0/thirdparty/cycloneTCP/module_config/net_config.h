/**
 * @file tcp_ip_stack_config.h
 * @brief CycloneTCP configuration file
 *
 * @section License
 *
 * Copyright (C) 2010-2019 Oryx Embedded SARL. All rights reserved.
 *
 * This file is part of CycloneTCP Eval.
 *
 * This software is provided in source form for a short-term evaluation only. The
 * evaluation license expires 90 days after the date you first download the software.
 *
 * If you plan to use this software in a commercial product, you are required to
 * purchase a commercial license from Oryx Embedded SARL.
 *
 * After the 90-day evaluation period, you agree to either purchase a commercial
 * license or delete all copies of this software. If you wish to extend the
 * evaluation period, you must contact sales@oryx-embedded.com.
 *
 * This evaluation software is provided "as is" without warranty of any kind.
 * Technical support is available as an option during the evaluation period.
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.9.4
 **/

#ifndef _TCP_IP_STACK_CONFIG_H
#define _TCP_IP_STACK_CONFIG_H

#include "os_port_config.h"
#if defined(__G3_STACK__)
#include "conf_tables.h"
#include "conf_project.h"
#endif

#ifdef USE_NO_RTOS
/* RTOS-less working mode */
#define NET_RTOS_SUPPORT DISABLED
#endif

/* Trace level for TCP/IP stack debugging */
#define MEM_TRACE_LEVEL          4
#define NIC_TRACE_LEVEL          4
#define ETH_TRACE_LEVEL          2
#define ARP_TRACE_LEVEL          2
#define IP_TRACE_LEVEL           2
#define IPV4_TRACE_LEVEL         2
#define IPV6_TRACE_LEVEL         2
#define ICMP_TRACE_LEVEL         2
#define IGMP_TRACE_LEVEL         4
#define ICMPV6_TRACE_LEVEL       2
#define MLD_TRACE_LEVEL          4
#define NDP_TRACE_LEVEL          4
#define UDP_TRACE_LEVEL          2
#define TCP_TRACE_LEVEL          2
#define SOCKET_TRACE_LEVEL       2
#define RAW_SOCKET_TRACE_LEVEL   2
#define BSD_SOCKET_TRACE_LEVEL   2
#define SLAAC_TRACE_LEVEL        4
#define DHCP_TRACE_LEVEL         4
#define DHCPV6_TRACE_LEVEL       4
#define DNS_TRACE_LEVEL          4
#define DNS_SD_TRACE_LEVEL       4
#define MDNS_TRACE_LEVEL         4
#define NBNS_TRACE_LEVEL         4
#define LLMNR_TRACE_LEVEL        4
#define FTP_TRACE_LEVEL          5
#define HTTP_TRACE_LEVEL         4
#define SMTP_TRACE_LEVEL         5
#define SNTP_TRACE_LEVEL         4
#define SNMP_TRACE_LEVEL         4
#define STD_SERVICES_TRACE_LEVEL 5
#define PPP_TRACE_LEVEL          4
#define PING_TRACE_LEVEL         2

/* Number of network adapters */
#if defined(__G3_GATEWAY__) || defined(__PRIME_GATEWAY__)
#define NET_INTERFACE_COUNT 2
#else
#define NET_INTERFACE_COUNT 1
#endif

/* IPv4 support */
#define IPV4_SUPPORT DISABLED

/* Ethernet support (only if gateway)* / */
#if (defined(__G3_GATEWAY__) && defined(CONF_USE_ETHERNET)) || defined(__PRIME_GATEWAY__)
#define ETH_SUPPORT ENABLED
#endif

#if defined(__G3_GATEWAY__) && defined(CONF_USE_PPP)
#define NIC_TICK_INTERVAL 50
#define PPP_SUPPORT ENABLED
#define PAP_SUPPORT ENABLED
#else
#define PPP_SUPPORT DISABLED
#endif

/* IPv4 fragmentation support */
#define IPV4_FRAG_SUPPORT ENABLED
/* Maximum number of fragmented packets the host will accept */
/* and hold in the reassembly queue simultaneously */
#define IPV4_MAX_FRAG_DATAGRAMS 4
/* Maximum datagram size the host will accept when reassembling fragments */
#define IPV4_MAX_FRAG_DATAGRAM_SIZE 8192

/* Size of ARP cache */
#define ARP_CACHE_SIZE 8
/* Maximum number of packets waiting for address resolution to complete */
#define ARP_MAX_PENDING_PACKETS 2

/* IGMP support */
#define IGMP_SUPPORT DISABLED

/* IPv6 support */
#define IPV6_SUPPORT ENABLED

/* Enable routing for gateways */
#define IPV6_ROUTING_SUPPORT ENABLED

#ifdef __G3_GATEWAY__
  #define NDP_ROUTER_ADV_SUPPORT ENABLED
#endif

/* IPv6 fragmentation support */
#define IPV6_FRAG_SUPPORT ENABLED
/* Maximum number of fragmented packets the host will accept */
/* and hold in the reassembly queue simultaneously */
#define IPV6_MAX_FRAG_DATAGRAMS 4
/* Maximum datagram size the host will accept when reassembling fragments */
#define IPV6_MAX_FRAG_DATAGRAM_SIZE 8192

/* MLD support */
#define MLD_SUPPORT DISABLED

/* Neighbor cache size */
#define NDP_CACHE_SIZE 8
/* Maximum number of packets waiting for address resolution to complete */
#define NDP_MAX_PENDING_PACKETS 2

/* G3-specific NDP settings */
/* Maximum number of Neighbor Solicitation messages sent while performing DAD (0) */
#define G3_PLC_DUP_ADDR_DETECT_TRANSMITS 0
/* Time interval between retransmissions of RS messages (10s) */
#define G3_PLC_RS_RETRY_WAIT_TIME 10000
/* Number of retransmissions for RS messages (3) */
#define G3_PLC_RS_MAX_RETRY 3

/* Some NDP parameters are G3-PLC specific and should be overriden... */
#define NDP_DUP_ADDR_DETECT_TRANSMITS G3_PLC_DUP_ADDR_DETECT_TRANSMITS
#define NDP_MIN_RTR_SOLICITATION_DELAY G3_PLC_RS_RETRY_WAIT_TIME
#define NDP_MAX_RTR_SOLICITATION_DELAY G3_PLC_RS_RETRY_WAIT_TIME
#define NDP_RTR_SOLICITATION_INTERVAL G3_PLC_RS_RETRY_WAIT_TIME
#define NDP_MAX_RTR_SOLICITATIONS G3_PLC_RS_MAX_RETRY

/* TCP support */
#define TCP_SUPPORT DISABLED
/* Default buffer size for transmission */
#define TCP_DEFAULT_TX_BUFFER_SIZE (1430 * 2)
/* Default buffer size for reception */
#define TCP_DEFAULT_RX_BUFFER_SIZE (1430 * 2)
/* Default SYN queue size for listening sockets */
#define TCP_DEFAULT_SYN_QUEUE_SIZE 4
/* Maximum number of retransmissions */
#define TCP_MAX_RETRIES 5
/* Selective acknowledgment support */
#define TCP_SACK_SUPPORT DISABLED

/* UDP support */
#define UDP_SUPPORT ENABLED
/* Receive queue depth for connectionless sockets */
#define UDP_RX_QUEUE_SIZE 4

/* Raw socket support */
#define RAW_SOCKET_SUPPORT ENABLED
/* Receive queue depth for raw sockets */
#define RAW_SOCKET_RX_QUEUE_SIZE 4

/* Number of sockets that can be opened simultaneously */
#define SOCKET_MAX_COUNT 10

/* Disable MDNS support */
#define MDNS_RESPONDER_SUPPORT DISABLED
#define MDNS_CLIENT_SUPPORT DISABLED

/* Buffer size for PLC application */
#define NET_MEM_POOL_BUFFER_SIZE 1536

/* Num of buffers for PLC application */
#ifndef CONF_NET_MEM_POOL_BUFFER_COUNT
  #define CONF_NET_MEM_POOL_BUFFER_COUNT   4
#endif

#define NET_MEM_POOL_BUFFER_COUNT   CONF_NET_MEM_POOL_BUFFER_COUNT

/* Enable Static Memory Pool for PLC application */
#define NET_MEM_POOL_SUPPORT  ENABLED

/* Disable WEB SOCKET support */
#define WEB_SOCKET_SUPPORT DISABLED
#endif
