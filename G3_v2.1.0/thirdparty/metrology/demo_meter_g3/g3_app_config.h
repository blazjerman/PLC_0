#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "conf_global.h"
#include "conf_project.h"

/* If APP_CONFORMANCE_TEST is defined, the app is configured according to that test */
/* #define APP_CONFORMANCE_TEST */

/* Generic G3 IPv6 local-link address */
#define APP_IPV6_GENERIC_LINK_LOCAL_ADDR "fe80:0:0:0:781D:ff:fe00:0001"

/* Multicast Groups required in Conformance */
#define APP_IPV6_MULTICAST_ADDR_0 "ff02:0:0:0:0:0:0:1"
#define APP_IPV6_MULTICAST_ADDR_1 "ff12:30:1122:3344:5566:0:123:4567"

/* Network prefix for Unique local network */
#define G3_IF1_IPV6_NET_PREFIX   "FD00:0:2:781D:0:0:0:0"
#define G3_IF1_IPV6_NET_PREFIX_LEN  64

/* Port number for the application */
#define DLMS_SOCKET_PORT 0xF0B1

/* Port number for conformance */
#define CONFORMANCE_SOCKET_PORT 0xF0BF
/* Conformance Ping */
#define CONFORMANCE_PING_TTL        10

/* Default timeout for sockets */
#define APP_SOCKET_TIMEOUT 5000

/* Discovery / NetworkScan timeout in seconds */
#define CONF_DISCOVERY_TIMEOUT            12u

/* PSK / Network authentication Key (16 bytes) */
#define CONF_PSK_KEY (const uint8_t *)"\xAB\x10\x34\x11\x45\x11\x1B\xC3\xC1\x2D\xE8\xFF\x11\x14\x22\x04"

/* Context information table: index 0 (Context 0 with value c_IPv6_PREFIX & x_PAN_ID (length = 80)) */
#define CONF_CONTEXT_INFORMATION_TABLE_0 (const uint8_t *)"\x02\x00" "\x01" "\x50" "\xFE\x80\x00\x00\x00\x00\x00\x00\x78\x1D"

/* Context information  table: index 1 (Context 1 with value �112233445566� (length = 48)) */
#define CONF_CONTEXT_INFORMATION_TABLE_1 (const uint8_t *)"\x02\x00" "\x01" "\x30" "\x11\x22\x33\x44\x55\x66"

/* Default Route to Coord Enabled */
#define CONF_DEFAULT_COORD_ROUTE_ENABLED (const uint8_t *)"\x01"

/* Routing table entry TTL (2 bytes, little endian) */
#define CONF_ROUTING_TABLE_ENTRY_TTL (const uint8_t *)"\xB4\x00"  /* 180 minutes */

/* Max Join Time: 90 seconds */
#define CONF_MAX_JOIN_WAIT_TIME (const uint8_t *)"\x5A\x00"

/* Max Hops: 10 */
#define CONF_MAX_HOPS (const uint8_t *)"\x0A"

/* Routing table entry TTL for Conformance (2 bytes, little endian) */
#define CONF_ROUTING_TABLE_ENTRY_TTL_CONFORMANCE (const uint8_t *)"\x05\x00"  /* 5 minutes */

/* Blacklist table entry TTL (2 bytes, little endian) (2 minutes) */
#define CONF_BLACKLIST_TABLE_ENTRY_TTL (const uint8_t *)"\x02\x00"

/* Broadcast Log table entry TTL (2 bytes, little endian) (2 minutes) */
#define CONF_BROADCAST_LOG_TABLE_ENTRY_TTL (const uint8_t *)"\x02\x00"

/* IDP (EAP's network access identifier) for ARIB */
#define CONF_ARIB_IDP (const uint8_t *)	\
	"\x48\x45\x4D\x53\xA3\xB2\x48\x45\x4D\x53\xA3\xB2\x48\x45\x4D\x53\xA3\xB2\x48\x45\x4D\x53\xA3\xB2\x48\x45\x4D\x53\xA3\xB2\x48\x45\x4D\x53\xA3\xB2"

/* Default routing (LoadNG) disable */
#define CONF_DISABLE_LOADNG (const uint8_t *)"\x01"

/* GroupTable: index 0 (2 bytes, little endian) (0x8567 � note that the IPv6 layer must listen to ff02::1 in correspondence to this group) */
#define CONF_GROUP_TABLE_0 (const uint8_t *)"\x01\x80"

/* GroupTable: index 1 (2 bytes, little endian) (0x8567 � note that the IPv6 layer must listen to ff12:30:1122:3344:5566:0:123:4567 in correspondence to this
 * group) */
#define CONF_GROUP_TABLE_1 (const uint8_t *)"\x67\x85"

/* Max Join Time: 20 seconds for Conformance */
#define CONF_MAX_JOIN_WAIT_TIME_CONFORMANCE (const uint8_t *)"\x14\x00"

/* Max Hops: 8 for Conformance */
#define CONF_MAX_HOPS_CONFORMANCE (const uint8_t *)"\x08"

/* Weak LQI: 60 for Conformance */
#define CONF_WEAK_LQI_CONFORMANCE (const uint8_t *)"\x3C"
/* RF Weak LQI: 100 for Conformance */
#define CONF_WEAK_LQI_RF_CONFORMANCE (const uint8_t *)"\x64"
/* Trickle Enabled for Conformance */
#define CONF_TRICKLE_DATA_ENABLED_CONFORMANCE (const uint8_t *)"\x01"
/* Trickle Adaptive Ki: True for Conformance */
#define CONF_TRICKLE_ADAPTIVE_KI_CONFORMANCE (const uint8_t *)"\x01"
/* Jitter delays for Conformance 1500ms and 100ms */
#define CONF_DELAY_LOW_LQI_CONFORMANCE (const uint8_t *)"\xDC\x05"
#define CONF_DELAY_HIGH_LQI_CONFORMANCE (const uint8_t *)"\x64\x00"
/* Cluster Trickle Enabled for Conformance */
#define CONF_CLUSTER_TRICKLE_ENABLED_CONFORMANCE (const uint8_t *)"\x01"
/* Cluster Trickle I for Conformance: 1400ms */
#define CONF_CLUSTER_TRICKLE_I_CONFORMANCE (const uint8_t *)"\x78\x05"
/* RF Trickle LQI Thresholds for Conformance 100 and 120 */
#define CONF_TRICKLE_LQI_THRESHOLD_LOW_RF_CONFORMANCE (const uint8_t *)"\x64"
#define CONF_TRICKLE_LQI_THRESHOLD_HIGH_RF_CONFORMANCE (const uint8_t *)"\x78"
/* Jitter LQI thresholds for Conformance: 100 and 170 */
#define CONF_JITTER_LOW_LQI_RF_CONFORMANCE (const uint8_t *)"\x64"
#define CONF_JITTER_HIGH_LQI_RF_CONFORMANCE (const uint8_t *)"\xAA"
/* Cluster Min LQI for Conformance: 110 */
#define CONF_CLUSTER_MIN_LQI_RF_CONFORMANCE (const uint8_t *)"\x6E"

/* Tone-Map Response TTL (1 byte) (3 minutes) */
#define CONF_TMR_TTL (const uint8_t *)"\x03"
/* Destination Address Set: index 0 address added (2 bytes, little endian) */
#define CONF_DEST_ADDR_SET_0 (const uint8_t *)"\xFF\x7F"
/* Media Probing Interval (1 byte) (2 minutes) */
#define CONF_PROBING_INTERVAL (const uint8_t *)"\x02"

/* Max CSMA Backoffs: 5 */
#define CONF_MAX_CSMA_BACKOFFS_CONFORMANCE (const uint8_t *)"\x05"
/* Max CSMA Backoffs RF: 50 */
#define CONF_MAX_CSMA_BACKOFFS_RF_CONFORMANCE (const uint8_t *)"\x32"
/* Max Frame Retries RF: 5 */
#define CONF_MAX_FRAME_RETRIES_RF_CONFORMANCE (const uint8_t *)"\x05"
/* POS Table Entry TTL: 4 minutes */
#define CONF_POS_TABLE_TTL_CONFORMANCE (const uint8_t *)"\x04"
/* POS Recent Entry Threshold: 3 minutes */
#define CONF_POS_RECENT_THRESHOLD_CONFORMANCE (const uint8_t *)"\x03"
/* Adaptive Power Low Bound: 74 */
#define CONF_ADAPTIVE_POWER_LOW_BOUND_CONFORMANCE (const uint8_t *)"\x4A"

/* Routing parameters */
#define CONF_KR_CONFORMANCE (const uint8_t *)"\x00"
#define CONF_KM_CONFORMANCE (const uint8_t *)"\x00"
#define CONF_KC_CONFORMANCE (const uint8_t *)"\x00"
#define CONF_KQ_CONFORMANCE (const uint8_t *)"\x0A"
#define CONF_KH_CONFORMANCE (const uint8_t *)"\x04"
#define CONF_KRT_CONFORMANCE (const uint8_t *)"\x00"
#define CONF_KQ_RF_CONFORMANCE (const uint8_t *)"\x05"
#define CONF_KH_RF_CONFORMANCE (const uint8_t *)"\x08"
#define CONF_KRT_RF_CONFORMANCE (const uint8_t *)"\x00"
#define CONF_KDC_RF_CONFORMANCE (const uint8_t *)"\x0A"
/* Duty Cycle Limit RF: 90 (2.5% out of 3600) */
#define CONF_DUTY_CYCLE_LIMIT_RF (const uint8_t *)"\x5A\x00"
/* Duty Cycle Limit RF: 3600 (100% out of 3600) */
/* #define CONF_DUTY_CYCLE_LIMIT_RF (const uint8_t *)"\x10\x0E" */
/* Use backup Media. True */
#define CONF_USE_BACKUP_MEDIA (const uint8_t *)"\x01"

/* LQI min to consider a link for Joining */
#define CONF_LQI_MIN_PLC         53
#define CONF_LQI_MIN_RF          95

/* Network In Progress timeout in milliseconds */
#define CONF_NETWORK_IN_PROGRESS_TIMEOUT_MIN        3000u
#define CONF_NETWORK_IN_PROGRESS_TIMEOUT_MAX       10000u
#define CONF_NETWORK_IN_PROGRESS_LIMIT             30000u
/* Start Up timeout max in milliseconds */
#define CONF_STARTUP_TIMEOUT_MAX                    5000u
#endif /* ifdef __CONFIG_H__ */
