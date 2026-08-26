#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "conf_global.h"
#include "conf_tables.h"

/* If APP_CONFORMANCE_TEST is defined, the app is configured according to that ETSI test */
/* #define APP_CONFORMANCE_TEST */

/* Generic G3 IPv6 coordinator address */
#define APP_IPV6_GENERIC_COORDINATOR_ADDR "fe80::7455:ff:fe00:0000"

/* Generic G3 IPv6 local-link address */
#define APP_IPV6_GENERIC_LINK_LOCAL_ADDR "fe80::7455:ff:fe00:0001"

#define APP_IPV6_MULTICAST_ADDR_0 "ff02:0:0:0:0:0:0:1"
#define APP_IPV6_MULTICAST_ADDR_1 "ff12:30:1122:3344:5566:0:123:4567"

/* Port number for the application */
#define DLMS_SOCKET_PORT 0xF0B1

/* Port number for conformance UDP responder */
  #define CONFORMANCE_SOCKET_PORT 0xF0BF
/* Conformance Ping */
  #define CONFORMANCE_PING_TTL                 10

/* Default timeout for the UDP socket */
#define APP_SOCKET_TIMEOUT 5000

/* Coordinator short address */
#define CONF_SHORT_ADDRESS (const uint8_t *)"\x00\x00"

/* PSK / Network authentication Key (16 bytes) */
#define CONF_PSK_KEY (const uint8_t *)"\xAB\x10\x34\x11\x45\x11\x1B\xC3\xC1\x2D\xE8\xFF\x11\x14\x22\x04"

/* GMK (16 bytes) */
#define CONF_GMK_KEY (const uint8_t *)"\xAF\x4D\x6D\xCC\xF1\x4D\xE7\xC1\xC4\x23\x5E\x6F\xEF\x6C\x15\x1F"

/* Context information table: index 0 (Context 0 with value c_IPv6_PREFIX & x_PAN_ID (length = 80)) */
#define CONF_CONTEXT_INFORMATION_TABLE_0 (const uint8_t *)"\x02\x00" "\x01" "\x50" "\xFE\x80\x00\x00\x00\x00\x00\x00\x78\x1D"

/* Context information table: index 1 (Context 1 with value �112233445566� (length = 48)) */
#define CONF_CONTEXT_INFORMATION_TABLE_1 (const uint8_t *)"\x02\x00" "\x01" "\x30" "\x11\x22\x33\x44\x55\x66"

/* Duty Cycle Limit RF: 360 (10% out of 3600) */
#define CONF_DUTY_CYCLE_LIMIT_RF (const uint8_t *)"\x68\x01"
/* Duty Cycle Limit RF: 3600 (100% out of 3600) */
/* #define CONF_DUTY_CYCLE_LIMIT_RF (const uint8_t *)"\x10\x0E" */

#ifdef APP_CONFORMANCE_TEST
/* Routing table entry TTL (2 bytes, little endian) */
  #define CONF_ROUTING_TABLE_ENTRY_TTL (const uint8_t *)"\x05\x00"  /* 5 minutes */

/* Blacklist table entry TTL (2 bytes, little endian) (2 minutes) */
  #define CONF_BLACKLIST_TABLE_ENTRY_TTL (const uint8_t *)"\x02\x00"

/* Broadcast Log table entry TTL (2 bytes, little endian) (2 minutes) */
  #define CONF_BROADCAST_LOG_TABLE_ENTRY_TTL (const uint8_t *)"\x02\x00"

/* IDP (EAP's network access identifier) for ARIB */
  #define CONF_ARIB_IDP (const uint8_t *) \
	"\x48\x45\x4D\x53\xA3\xB2\x48\x45\x4D\x53\xA3\xB2\x48\x45\x4D\x53\xA3\xB2\x48\x45\x4D\x53\xA3\xB2\x48\x45\x4D\x53\xA3\xB2\x48\x45\x4D\x53\xA3\xB2"

/* Default routing (LoadNG) disable */
  #define CONF_DISABLE_LOADNG (const uint8_t *)"\x01"

/* GroupTable: index 0 (2 bytes, little endian) (0x8567 � note that the IPv6 layer must listen to ff02::1 in correspondence to this group) */
  #define CONF_GROUP_TABLE_0 (const uint8_t *)"\x01\x80"

/* GroupTable: index 1 (2 bytes, little endian) (0x8567 � note that the IPv6 layer must listen to ff12:30:1122:3344:5566:0:123:4567 in correspondence to this
 * group) */
  #define CONF_GROUP_TABLE_1 (const uint8_t *)"\x67\x85"

/* Max Join Time: 20 seconds */
  #define CONF_MAX_JOIN_WAIT_TIME (const uint8_t *)"\x14\x00"

/* Max Hops: 8 */
  #define CONF_MAX_HOPS (const uint8_t *)"\x08"

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
  #define CONF_MAX_CSMA_BACKOFFS_RF (const uint8_t *)"\x32"
  /* Max Frame Retries RF: 5 */
  #define CONF_MAX_FRAME_RETRIES_RF (const uint8_t *)"\x05"
  /* POS Table Entry TTL: 4 minutes */
  #define CONF_POS_TABLE_TTL (const uint8_t *)"\x04"
  /* POS Recent Entry Threshold: 3 minutes */
  #define CONF_POS_RECENT_THRESHOLD_CONFORMANCE (const uint8_t *)"\x03"
  /* Adaptive Power Low Bound: 74 */
  #define CONF_ADAPTIVE_POWER_LOW_BOUND_CONFORMANCE (const uint8_t *)"\x4A"

  /* Routing parameters */
  #define CONF_KR (const uint8_t *)"\x00"
  #define CONF_KM (const uint8_t *)"\x00"
  #define CONF_KC (const uint8_t *)"\x00"
  #define CONF_KQ (const uint8_t *)"\x0A"
  #define CONF_KH (const uint8_t *)"\x04"
  #define CONF_KRT (const uint8_t *)"\x00"
  #define CONF_KQ_RF (const uint8_t *)"\x05"
  #define CONF_KH_RF (const uint8_t *)"\x08"
  #define CONF_KRT_RF (const uint8_t *)"\x00"
  #define CONF_KDC_RF (const uint8_t *)"\x0A"
#else
/* Routing table entry TTL (2 bytes, little endian) */
  #define CONF_ROUTING_TABLE_ENTRY_TTL (const uint8_t *)"\xB4\x00"  /* 180 minutes */

/* Max Join Time: 90 seconds */
  #define CONF_MAX_JOIN_WAIT_TIME (const uint8_t *)"\x5A\x00"

/* Max Hops: 10 */
  #define CONF_MAX_HOPS (const uint8_t *)"\x0A"
#endif

/* Max Number of DLMS devices (used for G3 GW and DLMS Client) */
#ifndef CONF_DLMS_MAX_DEV_NUM
  #define CONF_DLMS_MAX_DEV_NUM   50
#endif

#define DLMS_MAX_DEV_NUM   CONF_DLMS_MAX_DEV_NUM

#endif
