/**
 * \file
 *
 * \brief G3 Example Application : ADP Manager
 *
 * Copyright (c) 2014 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "conf_global.h"
#include "g3_app_config.h"
#include "conf_project.h"

#include <hal/hal.h>
#include <storage/storage.h>
#include "compiler.h"
#include "AdpApi.h"
#include "ProcessLbpDev.h"
#include "LbpDefs.h"
#include "drivers/g3/network_adapter_g3.h"
#include "app_adp_mng.h"
#include "app_dispatcher.h"
#ifdef DLMS_MGMT
#include "dlms_mgmt.h"
#endif /* #ifdef DLMS_MGMT */
#include "ipv6_mng.h"
#include "oss_if.h"

/* MAC include */
#include "mac_wrapper.h"

#ifndef DLMS_APP_PING_ALARM_TIMER_INTERVAL
	#define DLMS_APP_PING_ALARM_TIMER_INTERVAL 120
#endif

#ifdef APP_DEBUG_CONSOLE
#       define LOG_APP_DEBUG(a)   printf a
#else
#       define LOG_APP_DEBUG(a)   (void)0
#endif

#define NETWORK_DISCOVERY_CHANNEL_CHECK_TIME_MS    10000
#define NETWORK_DISCOVERY_MIN_BACKOFF_TIME_MS      10000
#define NETWORK_DISCOVERY_MAX_BACKOFF_TIME_MS     100000
#define NETWORK_JOIN_CHANNEL_CHECK_TIME_MS          2000
#define NETWORK_JOIN_MIN_BACKOFF_TIME_MS             500
#define NETWORK_JOIN_MAX_BACKOFF_TIME_MS            1000

/* Timers */
static uint32_t u32DelayStartUp;
static uint32_t u32BackOffTimer;
static uint32_t u32BackOffDiscoveryCW;
static uint32_t u32BackOffJoinCW;

static uint32_t u32ChannelCheckTimer;
static uint8_t u8JoinTries;

static bool bLoadPersistentInfo = true;
static bool bConformanceMode = false;

#if defined(__RF_MAC__)
static uint8_t savedRFParams[5];
static uint16_t savedRFParamLength = 0xFFFF;
#endif

/* MIB types and definitions */
enum MibType {
	MIB_MAC = 0,
	MIB_ADP
};

struct MibData {
	uint8_t m_u8Type; /* MibType */
	const char *m_szName;
	uint32_t m_u32Id;
	uint16_t m_u16Index;
	uint8_t m_u8ValueLength;
	const uint8_t *m_pu8Value;
};

extern uint8_t g_auc_ext_address[8]; /* EUI64 */

#ifdef DLMS_MGMT
#define APP_MIB_TABLE_SIZE 0
/* In DLMS management, params are obtained through DLMS from the METER (DLMS server) */
struct MibData *g_MibSettings;
#else /* SIMPLE_MGMT */
#define APP_MIB_TABLE_SIZE (sizeof(g_MibSettings) / sizeof(struct MibData))

struct MibData g_MibSettings[] = {
	{ MIB_MAC, "MAC_WRP_PIB_MANUF_EXTENDED_ADDRESS", MAC_WRP_PIB_MANUF_EXTENDED_ADDRESS, 0, 8, g_auc_ext_address },
#if defined(__RF_MAC__)
	{ MIB_MAC, "MAC_WRP_PIB_DUTY_CYCLE_LIMIT_RF", MAC_WRP_PIB_DUTY_CYCLE_LIMIT_RF, 0, 2, CONF_DUTY_CYCLE_LIMIT_RF },
#endif
#if defined(__PLC_MAC__) && defined(__RF_MAC__)
	{ MIB_ADP, "ADP_IB_USE_BACKUP_MEDIA", ADP_IB_USE_BACKUP_MEDIA, 0, 1, CONF_USE_BACKUP_MEDIA },
#endif
	{ MIB_ADP, "ADP_IB_CONTEXT_INFORMATION_TABLE", ADP_IB_CONTEXT_INFORMATION_TABLE, 0, 14, CONF_CONTEXT_INFORMATION_TABLE_0 },
	{ MIB_ADP, "ADP_IB_CONTEXT_INFORMATION_TABLE", ADP_IB_CONTEXT_INFORMATION_TABLE, 1, 10, CONF_CONTEXT_INFORMATION_TABLE_1 },
#if (ENABLE_ROUTING == 1)
	{ MIB_ADP, "ADP_IB_ROUTING_TABLE_ENTRY_TTL", ADP_IB_ROUTING_TABLE_ENTRY_TTL, 0, 2, CONF_ROUTING_TABLE_ENTRY_TTL },
	{ MIB_ADP, "ADP_IB_DEFAULT_COORD_ROUTE_ENABLED", ADP_IB_DEFAULT_COORD_ROUTE_ENABLED, 0, 1, CONF_DEFAULT_COORD_ROUTE_ENABLED },
#endif
	{ MIB_ADP, "ADP_IB_MAX_JOIN_WAIT_TIME", ADP_IB_MAX_JOIN_WAIT_TIME, 0, 2, CONF_MAX_JOIN_WAIT_TIME },
	{ MIB_ADP, "ADP_IB_MAX_HOPS", ADP_IB_MAX_HOPS, 0, 1, CONF_MAX_HOPS }
};
#endif /* DLMS_MGMT */

static uint8_t g_u8MibInitIndex;

static struct TAdpPanDescriptor g_BestNetwork;
static bool g_bFindValidNetwork;
static uint16_t g_u16ShortAddress;
static uint16_t g_u16PanId;
static Ipv6Addr ipv6Addr_plc_Prefix;

uint8_t g_u8NetworkJoinStatus;
/* Extern vars */
extern bool g_bHasMeterId;

static void LogJoinStatus(void)
{
	LOG_APP_DEBUG(("g_u8NetworkJoinStatus() Set join state to %s\r\n",
			g_u8NetworkJoinStatus == NJS_INIT_DELAY ? "NJS_INIT_DELAY" :
			g_u8NetworkJoinStatus == NJS_NOT_JOINED ? "NJS_NOT_JOINED" :
			g_u8NetworkJoinStatus == NJS_CHECK_FOR_BEACONS ? "NJS_CHECK_FOR_BEACONS" :
			g_u8NetworkJoinStatus == NJS_SCAN_BACKOFF ? "NJS_SCAN_BACKOFF" :
			g_u8NetworkJoinStatus == NJS_SCANNING ? "NJS_SCANNING" :
			g_u8NetworkJoinStatus == NJS_CHECK_FOR_LBP_LNG ? "NJS_CHECK_FOR_LBP_LNG" :
			g_u8NetworkJoinStatus == NJS_JOINNING_BACKOFF ? "NJS_JOINNING_BACKOFF" :
			g_u8NetworkJoinStatus == NJS_JOINNING ? "NJS_JOINNING" :
			g_u8NetworkJoinStatus == NJS_JOINED ? "NJS_JOINED" :
			g_u8NetworkJoinStatus == NJS_CHECK_FOR_BEACONS ? "NJS_CHECK_FOR_BEACONS" : "?????"
			));
}

static uint32_t GetRandomDelay(uint16_t us_min, uint16_t us_max)
{
	uint32_t ul_random;

	ul_random = platform_random_32();

	if (us_min < us_max) {
		return ((ul_random % (us_max - us_min)) + us_min);
	} else {
		return us_min;
	}
}

static void ResetChannelStatusIndicators(void)
{
	struct TAdpMacSetConfirm setConfirm;
	uint8_t u8null = 0;

	AdpMacSetRequestSync(MAC_WRP_PIB_MANUF_LBP_FRAME_RECEIVED, 0, 1, (const uint8_t *)&u8null, &setConfirm);
	AdpMacSetRequestSync(MAC_WRP_PIB_MANUF_LNG_FRAME_RECEIVED, 0, 1, (const uint8_t *)&u8null, &setConfirm);
	AdpMacSetRequestSync(MAC_WRP_PIB_MANUF_BCN_FRAME_RECEIVED, 0, 1, (const uint8_t *)&u8null, &setConfirm);
	AdpMacSetRequestSync(MAC_WRP_PIB_MANUF_LBP_FRAME_RECEIVED_RF, 0, 1, (const uint8_t *)&u8null, &setConfirm);
	AdpMacSetRequestSync(MAC_WRP_PIB_MANUF_LNG_FRAME_RECEIVED_RF, 0, 1, (const uint8_t *)&u8null, &setConfirm);
	AdpMacSetRequestSync(MAC_WRP_PIB_MANUF_BCN_FRAME_RECEIVED_RF, 0, 1, (const uint8_t *)&u8null, &setConfirm);
}

static bool CheckBeaconFramesReceived(void)
{
	struct TAdpMacGetConfirm getConfirm;
	uint8_t u8BCNFramesReceived = 0;
	enum EMacWrpPibAttribute attribute;

	if (g_BestNetwork.m_u8MediaType == MAC_WRP_MEDIA_TYPE_IND_RF) {
		attribute = MAC_WRP_PIB_MANUF_BCN_FRAME_RECEIVED_RF;
	}
	else {
		attribute = MAC_WRP_PIB_MANUF_BCN_FRAME_RECEIVED;
	}

	/* Get beacons frames indication */
	AdpMacGetRequestSync(attribute, 0, &getConfirm);
	if (getConfirm.m_u8Status == MAC_WRP_STATUS_SUCCESS) {
		u8BCNFramesReceived = getConfirm.m_au8AttributeValue[0];
	}

	if (u8BCNFramesReceived) {
		return true;
	} else {
		return false;
	}
}

static bool CheckLBPFramesReceived(void)
{
	struct TAdpMacGetConfirm getConfirm;
	uint8_t u8LBPFramesReceived = 0;
	enum EMacWrpPibAttribute attribute;

	if (g_BestNetwork.m_u8MediaType == MAC_WRP_MEDIA_TYPE_IND_RF) {
		attribute = MAC_WRP_PIB_MANUF_LBP_FRAME_RECEIVED_RF;
	}
	else {
		attribute = MAC_WRP_PIB_MANUF_LBP_FRAME_RECEIVED;
	}

	/* Get beacons frames indication */
	AdpMacGetRequestSync(attribute, 0, &getConfirm);
	if (getConfirm.m_u8Status == MAC_WRP_STATUS_SUCCESS) {
		u8LBPFramesReceived = getConfirm.m_au8AttributeValue[0];
	}

	if (u8LBPFramesReceived) {
		return true;
	} else {
		return false;
	}
}

static bool CheckLNGFramesReceived(void)
{
	struct TAdpMacGetConfirm getConfirm;
	uint8_t u8LNGFramesReceived = 0;
	enum EMacWrpPibAttribute attribute;

	if (g_BestNetwork.m_u8MediaType == MAC_WRP_MEDIA_TYPE_IND_RF) {
		attribute = MAC_WRP_PIB_MANUF_LNG_FRAME_RECEIVED_RF;
	}
	else {
		attribute = MAC_WRP_PIB_MANUF_LNG_FRAME_RECEIVED;
	}

	/* Get beacons frames indication */
	AdpMacGetRequestSync(attribute, 0, &getConfirm);
	if (getConfirm.m_u8Status == MAC_WRP_STATUS_SUCCESS) {
		u8LNGFramesReceived = getConfirm.m_au8AttributeValue[0];
	}

	if (u8LNGFramesReceived) {
		return true;
	} else {
		return false;
	}
}

static void SetConfirm(uint8_t u8Status, uint32_t u32AttributeId, uint16_t u16AttributeIndex)
{
	if (u8Status == G3_SUCCESS) {
		/* initialization phase? */
		if (g_u8MibInitIndex < APP_MIB_TABLE_SIZE) {
			if ((g_MibSettings[g_u8MibInitIndex].m_u32Id == u32AttributeId) && (g_MibSettings[g_u8MibInitIndex].m_u16Index == u16AttributeIndex)) {
				g_u8MibInitIndex++;
				if (g_u8MibInitIndex == APP_MIB_TABLE_SIZE) {
					LOG_APP_DEBUG(("Modem fully initialized.\r\n"));
				}
			} else {
				LOG_APP_DEBUG((
							"ERR[AppAdpSetConfirm] Invalid SetConfirm received during initialization. Expecting 0x%08X/%hu but received 0x%08X/%hu\r\n",
							g_MibSettings[g_u8MibInitIndex].m_u32Id, g_MibSettings[g_u8MibInitIndex].m_u16Index,
							u32AttributeId, u16AttributeIndex));
			}
		}
	}
	else if (u8Status == MAC_WRP_STATUS_INVALID_PARAMETER) {
		/* In case parameter is invalid, jump to next IB to avoid getting stuck in initialization */
		LOG_APP_DEBUG(("ERR[AppAdpSetConfirm] Invalid parameter for IB index %u", g_u8MibInitIndex));
		g_u8MibInitIndex++;
	}
	else {
		LOG_APP_DEBUG(("ERR[AppAdpSetConfirm] status: %u\r\n", u8Status));
	}
}

static void AppAdpDataConfirm(struct TAdpDataConfirm *pDataConfirm)
{
#ifdef DLMS_MGMT
	send_counters();
#endif /* #ifdef DLMS_MGMT */
	if (pDataConfirm->m_u8Status != G3_SUCCESS) {
		LOG_APP_DEBUG(("ERR[AppAdpDataConfirm] NsduHandle: %hu Status: %hu\r\n", pDataConfirm->m_u8NsduHandle, pDataConfirm->m_u8Status));
	}
}

static void AppAdpDataIndication(struct TAdpDataIndication *pDataIndication)
{
	if (pDataIndication->m_u16NsduLength) {
#ifdef DLMS_MGMT
		send_counters();
#endif /* #ifdef DLMS_MGMT */
		ipv6_receive_packet(pDataIndication);
		LOG_APP_DEBUG(("ERR[AppAdpDataIndication] Len: %hu LQI: %hu\r\n", pDataIndication->m_u16NsduLength, pDataIndication->m_u8LinkQualityIndicator));
	}
}

static void AppAdpDiscoveryIndication(struct TAdpPanDescriptor *pPanDescriptor)
{
	LOG_APP_DEBUG(("-- Discovered PanId: 0x%04X LBA: 0x%04X RcCoord: %hu LinkQuality: %hu MediaType: %hu\r\n",
			pPanDescriptor->m_u16PanId, pPanDescriptor->m_u16LbaAddress, pPanDescriptor->m_u16RcCoord,
			pPanDescriptor->m_u8LinkQuality, pPanDescriptor->m_u8MediaType));

	/* Check LQI min */
	uint8_t u8MinLQI;
	if (pPanDescriptor->m_u8MediaType == MAC_WRP_MEDIA_TYPE_IND_PLC) {
		u8MinLQI = CONF_LQI_MIN_PLC;
	}
	else {
		u8MinLQI = CONF_LQI_MIN_RF;
	}
	if ((pPanDescriptor->m_u8LinkQuality >= u8MinLQI) && (pPanDescriptor->m_u16RcCoord < 0x7FFF)) {
		g_bFindValidNetwork = true;
		/* Update Best Network : min Root Cost || best LQI with same Root Cost */
		if ((pPanDescriptor->m_u16RcCoord < g_BestNetwork.m_u16RcCoord) ||
				((pPanDescriptor->m_u16RcCoord == g_BestNetwork.m_u16RcCoord) &&
				(pPanDescriptor->m_u8LinkQuality > g_BestNetwork.m_u8LinkQuality))) {
			g_BestNetwork = *pPanDescriptor;
		}
	} else {
		return;
	}
}

static void AppAdpDiscoveryConfirm(uint8_t u8Status)
{
	if ((u8Status == G3_SUCCESS) && (g_BestNetwork.m_u16PanId != 0xFFFF) && (g_BestNetwork.m_u16LbaAddress != 0xFFFF)) {
		/* Check whether LBP frames are seeing in the network */
		LOG_APP_DEBUG(("Discovery Confirm Success.\r\n"));
		/* Check Valid network */
		if (g_bFindValidNetwork) {
			LOG_APP_DEBUG(("Found a valid network. Check network in process.\r\n"));
			g_u8NetworkJoinStatus = NJS_CHECK_FOR_LBP_LNG;
			ResetChannelStatusIndicators();
			u32BackOffTimer = 0;
			u32ChannelCheckTimer = NETWORK_JOIN_CHANNEL_CHECK_TIME_MS;
		} else {
			LOG_APP_DEBUG(("Not found valid network. Restart Discovery process.\r\n"));
			g_u8NetworkJoinStatus = NJS_NOT_JOINED;
			u32BackOffTimer = NETWORK_DISCOVERY_MIN_BACKOFF_TIME_MS;
		}
	} else {
		LOG_APP_DEBUG(("ERR[AppAdpDiscoveryConfirm] Status %hhu BestPan: 0x%04X\r\n", u8Status, g_BestNetwork.m_u16PanId));
		g_u8NetworkJoinStatus = NJS_NOT_JOINED;
		u32BackOffTimer = NETWORK_DISCOVERY_MIN_BACKOFF_TIME_MS;
	}

	LogJoinStatus();
}

static void AppAdpNetworkStartConfirm(struct TAdpNetworkStartConfirm *pNetworkStartConfirm)
{
	(void)pNetworkStartConfirm;
}

static void AppAdpNetworkJoinConfirm(struct TAdpNetworkJoinConfirm *pNetworkJoinConfirm)
{
	struct TAdpGetConfirm getConfirm;
	uint16_t u16CoordShorAddress;
	uint8_t u8NumHops;

	if (pNetworkJoinConfirm->m_u8Status == G3_SUCCESS) {
		g_u8NetworkJoinStatus = NJS_JOINED;
		LogJoinStatus();

		g_u16ShortAddress = pNetworkJoinConfirm->m_u16NetworkAddress;
		g_u16PanId = pNetworkJoinConfirm->m_u16PanId;

		LOG_APP_DEBUG(("Network successfully joined: PAN-ID: 0x%04X ShortAddress: 0x%04X\r\n", g_u16PanId, g_u16ShortAddress));

		/* Update CONTEXT INFORMATION : Only Update PAND_ID */
		/* u16ValidTimeMinutes :  bValidForCompression :  u8BitsContextLength : Context Information Table: Zeros */
		AdpGetRequestSync(ADP_IB_CONTEXT_INFORMATION_TABLE, 0, &getConfirm);
		getConfirm.m_au8AttributeValue[12] = (uint8_t)(g_u16PanId >> 8);
		getConfirm.m_au8AttributeValue[13] = (uint8_t)(g_u16PanId & 0xFF);
		AdpSetRequest(ADP_IB_CONTEXT_INFORMATION_TABLE, 0, getConfirm.m_u8AttributeLength, getConfirm.m_au8AttributeValue);

		/* Start a route request to coordinator */
		AdpGetRequestSync(ADP_IB_COORD_SHORT_ADDRESS, 0, &getConfirm);
		if (getConfirm.m_u8Status == G3_SUCCESS) {
			memcpy(&u16CoordShorAddress, &getConfirm.m_au8AttributeValue[0], sizeof(u16CoordShorAddress));
			u8NumHops = (*(CONF_MAX_HOPS));
			AdpRouteDiscoveryRequest(u16CoordShorAddress, u8NumHops);
			/* Report to dispatcher module */
			ipv6_mng_upd_link(g_u16PanId, g_u16ShortAddress);
		}

		/* On network join, add Local Link address  based on PanId and ShortAddress*/
		Ipv6Addr ipv6_addr;

		/* Set link-local address, based on the PAN_ID and the short address */
		ipv6StringToAddr(APP_IPV6_GENERIC_LINK_LOCAL_ADDR, &ipv6_addr);
		/* Adapt the IPv6 address to the G3 connection data */
		ipv6_addr.b[8] = (uint8_t)(g_u16PanId >> 8);
		ipv6_addr.b[9] = (uint8_t)(g_u16PanId & 0xFF);
		ipv6_addr.b[10] = 0x0;
		ipv6_addr.b[11] = 0xFF;
		ipv6_addr.b[12] = 0xFE;
		ipv6_addr.b[13] = 0x0;
		ipv6_addr.b[14] = (uint8_t)(g_u16ShortAddress >> 8);
		ipv6_addr.b[15] = (uint8_t)(g_u16ShortAddress & 0xFF);

		ipv6SetLinkLocalAddr(&netInterface[0], &ipv6_addr);

		/* Also, On network join, add Unique Local Link address */
		Ipv6Addr ipv6Addr;
		uint8_t puc_prefix_data[27];
		uint32_t ui_infinite = 0x20C000; /* MAX VALUE? */

		ipv6StringToAddr(G3_IF1_IPV6_NET_PREFIX, &ipv6Addr);
		ipv6Addr.b[6] =  (uint8_t)(g_u16PanId >> 8);
		ipv6Addr.b[7] =  (uint8_t)(g_u16PanId & 0xFF);

		ipv6Addr.b[8] =  (uint8_t)g_auc_ext_address[7];
		ipv6Addr.b[9] =  (uint8_t)g_auc_ext_address[6];
		ipv6Addr.b[10] =  (uint8_t)g_auc_ext_address[5];
		ipv6Addr.b[11] =  (uint8_t)g_auc_ext_address[4];
		ipv6Addr.b[12] =  (uint8_t)g_auc_ext_address[3];
		ipv6Addr.b[13] =  (uint8_t)g_auc_ext_address[2];
		ipv6Addr.b[14] =  (uint8_t)g_auc_ext_address[1];
		ipv6Addr.b[15] =  (uint8_t)g_auc_ext_address[0];

		/* ipv6SetLinkLocalAddr(&netInterface[0], &ipv6Addr); */
		ipv6SetGlobalAddr(&netInterface[0], 0, &ipv6Addr);

		/* Configure new Network Prefix */
		ipv6StringToAddr(G3_IF1_IPV6_NET_PREFIX, &ipv6Addr_plc_Prefix);
		ipv6Addr_plc_Prefix.b[6] = (uint8_t)(g_u16PanId >> 8);
		ipv6Addr_plc_Prefix.b[7] = (uint8_t)(g_u16PanId & 0xFF);
		ipv6SetPrefix(&netInterface[0], 0, &ipv6Addr_plc_Prefix, G3_IF1_IPV6_NET_PREFIX_LEN);

		puc_prefix_data[0] = G3_IF1_IPV6_NET_PREFIX_LEN; /* Prefix lenght */
		puc_prefix_data[1] = 1;   /* OnLinkFlag */
		puc_prefix_data[2] = 1;   /* AutonomuosConfiguration flag */
		memcpy(&puc_prefix_data[3], (uint8_t *)&ui_infinite, 4); /* valid lifetime */
		memcpy(&puc_prefix_data[7], (uint8_t *)&ui_infinite, 4); /* preferred lifetime */
		memcpy(&puc_prefix_data[11], &ipv6Addr_plc_Prefix, 16); /* network prefix */

		/* Configure ADP */
		AdpSetRequest(ADP_IB_PREFIX_TABLE, 0, 27, (const uint8_t *)puc_prefix_data);
	} else {
		if (g_u8NetworkJoinStatus != NJS_JOINED) {
			LOG_APP_DEBUG(("ERR[AppAdpNetworkJoinConfirm]: Status %hu ; g_u8NetworkJoinStatus %hhu \r\n", pNetworkJoinConfirm->m_u8Status,
					g_u8NetworkJoinStatus));

			if (++u8JoinTries > 2) { /* 3 tries */
				g_u8NetworkJoinStatus = NJS_NOT_JOINED;
				u32BackOffTimer = NETWORK_DISCOVERY_MIN_BACKOFF_TIME_MS;
				LogJoinStatus();
				u8JoinTries = 0;
			} else {
				g_u8NetworkJoinStatus = NJS_CHECK_FOR_LBP_LNG;
				LogJoinStatus();
				ResetChannelStatusIndicators();
				u32ChannelCheckTimer = NETWORK_JOIN_CHANNEL_CHECK_TIME_MS;
				u32BackOffJoinCW = NETWORK_JOIN_MIN_BACKOFF_TIME_MS;
				LogJoinStatus();
			}
		} else {
			LOG_APP_DEBUG(("ERR[AppAdpNetworkJoinConfirm]: Relayed NetworkJoinConfirm triggered. Ignored.\r\n"));
		}
	}
}

static void AppAdpNetworkLeaveConfirm(struct TAdpNetworkLeaveConfirm *pLeaveConfirm)
{
	if (pLeaveConfirm->m_u8Status == G3_SUCCESS) {
		LOG_APP_DEBUG(("Network left: no longer connected to the network.\r\n"));
		/* reinit the modem as Kick is followed internally by MAC/ADP reset */
		g_u8MibInitIndex = 0;
		g_u8NetworkJoinStatus = NJS_INIT_DELAY;
		u32DelayStartUp = GetRandomDelay(0, CONF_STARTUP_TIMEOUT_MAX);
		LogJoinStatus();
	} else {
		LOG_APP_DEBUG(("ERR[AppAdpNetworkLeaveConfirm]: Status %hu, Network left anyway.\r\n", pLeaveConfirm->m_u8Status));
		/* reinit the modem as Kick is followed internally by MAC/ADP reset */
		g_u8MibInitIndex = 0;
		g_u8NetworkJoinStatus = NJS_INIT_DELAY;
		u32DelayStartUp = GetRandomDelay(0, CONF_STARTUP_TIMEOUT_MAX);
		LogJoinStatus();
	}
}

static void AppAdpNetworkLeaveIndication(void)
{
	LOG_APP_DEBUG(("Kicked off the network: no longer connected to the network.\r\n"));
	/* reinit the modem as Kick is followed internally by MAC/ADP reset */
	g_u8MibInitIndex = 0;
	g_u8NetworkJoinStatus = NJS_INIT_DELAY;
	u32DelayStartUp = GetRandomDelay(0, CONF_STARTUP_TIMEOUT_MAX);
	LogJoinStatus();
}

static void AppAdpResetConfirm(struct TAdpResetConfirm *pResetConfirm)
{
	(void)pResetConfirm;
}

static void AppAdpGetConfirm(struct TAdpGetConfirm *pGetConfirm)
{
	(void)pGetConfirm;
}

static void AppAdpMacGetConfirm(struct TAdpMacGetConfirm *pGetConfirm)
{
	(void)pGetConfirm;
}

static void AppAdpSetConfirm(struct TAdpSetConfirm *pSetConfirm)
{
	SetConfirm(pSetConfirm->m_u8Status, pSetConfirm->m_u32AttributeId, pSetConfirm->m_u16AttributeIndex);
}

static void AppAdpMacSetConfirm(struct TAdpMacSetConfirm *pSetConfirm)
{
	SetConfirm(pSetConfirm->m_u8Status, pSetConfirm->m_u32AttributeId, pSetConfirm->m_u16AttributeIndex);
}

static void AppAdpNetworkStatusIndication(struct TAdpNetworkStatusIndication *pNetworkStatusIndication)
{
	LOG_APP_DEBUG(("NetworkStatusIndication: %hu\r\n", pNetworkStatusIndication->m_u8Status));
	(void)pNetworkStatusIndication;
}

static void AppAdpRouteDiscoveryConfirm(struct TAdpRouteDiscoveryConfirm *pRouteDiscoveryConfirm)
{
	(void)pRouteDiscoveryConfirm;
}

static void AppAdpPathDiscoveryConfirm(struct TAdpPathDiscoveryConfirm *pPathDiscoveryConfirm)
{
	(void)pPathDiscoveryConfirm;
}

static void AppAdpNotification_BufferIndication(TAdpBufferIndication *pBufferIndication)
{
	UNUSED(pBufferIndication);
}

static void AppAdpNotification_PREQIndication(void)
{
}

static void AppAdpNotification_UpdNonVolatileDataIndication(struct TAdpNonVolatileData *pNonVolatileData)
{
#ifdef ENABLE_PIB_RESTORE
	store_persistent_data_GPBR(pNonVolatileData);
#endif
}

static void InitializeStack(void)
{
	struct TAdpNotifications notifications;
	struct TLbpNotificationsDev lbpDevNotifications;

	notifications.fnctAdpDataConfirm = AppAdpDataConfirm;
	notifications.fnctAdpDataIndication = AppAdpDataIndication;
	notifications.fnctAdpDiscoveryConfirm = AppAdpDiscoveryConfirm;
	notifications.fnctAdpDiscoveryIndication = AppAdpDiscoveryIndication;
	notifications.fnctAdpNetworkStartConfirm = AppAdpNetworkStartConfirm;
	notifications.fnctAdpResetConfirm = AppAdpResetConfirm;
	notifications.fnctAdpSetConfirm = AppAdpSetConfirm;
	notifications.fnctAdpMacSetConfirm = AppAdpMacSetConfirm;
	notifications.fnctAdpGetConfirm = AppAdpGetConfirm;
	notifications.fnctAdpMacGetConfirm = AppAdpMacGetConfirm;
	notifications.fnctAdpRouteDiscoveryConfirm = AppAdpRouteDiscoveryConfirm;
	notifications.fnctAdpPathDiscoveryConfirm = AppAdpPathDiscoveryConfirm;
	notifications.fnctAdpNetworkStatusIndication = AppAdpNetworkStatusIndication;
	notifications.fnctAdpBufferIndication = AppAdpNotification_BufferIndication;
	notifications.fnctAdpPREQIndication = AppAdpNotification_PREQIndication;
	notifications.fnctAdpUpdNonVolatileDataIndication = AppAdpNotification_UpdNonVolatileDataIndication;
	notifications.fnctAdpRouteNotFoundIndication = NULL;

#if defined (CONF_BAND_CENELEC_A)
	AdpInitialize(&notifications, ADP_BAND_CENELEC_A);
#elif defined (CONF_BAND_CENELEC_B)
	AdpInitialize(&notifications, ADP_BAND_CENELEC_B);
#elif defined (CONF_BAND_FCC)
	AdpInitialize(&notifications, ADP_BAND_FCC);
#elif defined (CONF_BAND_ARIB)
	AdpInitialize(&notifications, ADP_BAND_ARIB);
#else
	AdpInitialize(&notifications, ADP_BAND_CENELEC_A);
#endif

	LBP_InitDev();
	lbpDevNotifications.fnctJoinConfirm = AppAdpNetworkJoinConfirm;
	lbpDevNotifications.fnctLeaveIndication = AppAdpNetworkLeaveIndication;
	lbpDevNotifications.fnctLeaveConfirm = AppAdpNetworkLeaveConfirm;
	LBP_SetNotificationsDev(&lbpDevNotifications);
}

static void SetConformanceModemParameters(void)
{
#ifdef CONF_BAND_ARIB
	struct TLbpSetParamConfirm lbpSetConfirm;
	LBP_SetParamDev(LBP_IB_IDP, 0, 36, CONF_ARIB_IDP, &lbpSetConfirm);
#if (ENABLE_ROUTING == 1)
	AdpSetRequest(ADP_IB_DISABLE_DEFAULT_ROUTING, 0, 1, CONF_DISABLE_LOADNG);
#endif
#endif
	AdpSetRequest(ADP_IB_BLACKLIST_TABLE_ENTRY_TTL, 0, 2, CONF_BLACKLIST_TABLE_ENTRY_TTL);
	AdpSetRequest(ADP_IB_BROADCAST_LOG_TABLE_ENTRY_TTL, 0, 2, CONF_BROADCAST_LOG_TABLE_ENTRY_TTL);
	AdpSetRequest(ADP_IB_GROUP_TABLE, 0, 2, CONF_GROUP_TABLE_0);
	AdpSetRequest(ADP_IB_GROUP_TABLE, 1, 2, CONF_GROUP_TABLE_1);
	AdpSetRequest(ADP_IB_ROUTING_TABLE_ENTRY_TTL, 0, 2, CONF_ROUTING_TABLE_ENTRY_TTL_CONFORMANCE);
	AdpSetRequest(ADP_IB_MAX_JOIN_WAIT_TIME, 0, 2, CONF_MAX_JOIN_WAIT_TIME_CONFORMANCE);
	AdpSetRequest(ADP_IB_MAX_HOPS, 0, 1, CONF_MAX_HOPS_CONFORMANCE);
	AdpSetRequest(ADP_IB_WEAK_LQI_VALUE, 0, 1, CONF_WEAK_LQI_CONFORMANCE);
	AdpSetRequest(ADP_IB_WEAK_LQI_VALUE_RF, 0, 1, CONF_WEAK_LQI_RF_CONFORMANCE);
	AdpSetRequest(ADP_IB_TRICKLE_DATA_ENABLED, 0, 1, CONF_TRICKLE_DATA_ENABLED_CONFORMANCE);
	AdpSetRequest(ADP_IB_TRICKLE_ADAPTIVE_KI, 0, 1, CONF_TRICKLE_ADAPTIVE_KI_CONFORMANCE);
	AdpSetRequest(ADP_IB_TRICKLE_LQI_THRESHOLD_LOW_RF, 0, 1, CONF_TRICKLE_LQI_THRESHOLD_LOW_RF_CONFORMANCE);
	AdpSetRequest(ADP_IB_TRICKLE_LQI_THRESHOLD_HIGH_RF, 0, 1, CONF_TRICKLE_LQI_THRESHOLD_HIGH_RF_CONFORMANCE);
	AdpSetRequest(ADP_IB_CLUSTER_TRICKLE_ENABLED, 0, 1, CONF_CLUSTER_TRICKLE_ENABLED_CONFORMANCE);
	AdpSetRequest(ADP_IB_DELAY_LOW_LQI_RF, 0, 2, CONF_DELAY_LOW_LQI_CONFORMANCE);
	AdpSetRequest(ADP_IB_DELAY_HIGH_LQI_RF, 0, 2, CONF_DELAY_HIGH_LQI_CONFORMANCE);
	AdpSetRequest(ADP_IB_RREQ_JITTER_LOW_LQI_RF, 0, 1, CONF_JITTER_LOW_LQI_RF_CONFORMANCE);
	AdpSetRequest(ADP_IB_RREQ_JITTER_HIGH_LQI_RF, 0, 1, CONF_JITTER_HIGH_LQI_RF_CONFORMANCE);
	AdpSetRequest(ADP_IB_CLUSTER_MIN_LQI_RF, 0, 1, CONF_CLUSTER_MIN_LQI_RF_CONFORMANCE);
	AdpSetRequest(ADP_IB_CLUSTER_TRICKLE_I_RF, 0, 2, CONF_CLUSTER_TRICKLE_I_CONFORMANCE);
	AdpSetRequest(ADP_IB_DELAY_LOW_LQI, 0, 2, CONF_DELAY_LOW_LQI_CONFORMANCE);
	AdpSetRequest(ADP_IB_DELAY_HIGH_LQI, 0, 2, CONF_DELAY_HIGH_LQI_CONFORMANCE);
	AdpSetRequest(ADP_IB_CLUSTER_TRICKLE_I, 0, 2, CONF_CLUSTER_TRICKLE_I_CONFORMANCE);
	AdpSetRequest(ADP_IB_PROBING_INTERVAL, 0, 1, CONF_PROBING_INTERVAL);
	AdpMacSetRequest(MAC_WRP_PIB_TMR_TTL, 0, 1, CONF_TMR_TTL);
  #if (ENABLE_ROUTING == 1)
	AdpSetRequest(ADP_IB_DESTINATION_ADDRESS_SET, 0, 2, CONF_DEST_ADDR_SET_0);
  #endif
	AdpMacSetRequest(MAC_WRP_PIB_MAX_CSMA_BACKOFFS, 0, 1, CONF_MAX_CSMA_BACKOFFS_CONFORMANCE);
	AdpMacSetRequest(MAC_WRP_PIB_MAX_CSMA_BACKOFFS_RF, 0, 1, CONF_MAX_CSMA_BACKOFFS_RF_CONFORMANCE);
	AdpMacSetRequest(MAC_WRP_PIB_MAX_FRAME_RETRIES_RF, 0, 1, CONF_MAX_FRAME_RETRIES_RF_CONFORMANCE);
	AdpMacSetRequest(MAC_WRP_PIB_POS_TABLE_ENTRY_TTL, 0, 1, CONF_POS_TABLE_TTL_CONFORMANCE);
	AdpMacSetRequest(MAC_WRP_PIB_POS_RECENT_ENTRY_THRESHOLD, 0, 1, CONF_POS_RECENT_THRESHOLD_CONFORMANCE);
	AdpMacSetRequest(MAC_WRP_PIB_ADAPTIVE_POWER_LOW_BOUND_RF, 0, 1, CONF_ADAPTIVE_POWER_LOW_BOUND_CONFORMANCE);
  #if (ENABLE_ROUTING == 1)
	AdpSetRequest(ADP_IB_KR, 0, 1, CONF_KR_CONFORMANCE);
	AdpSetRequest(ADP_IB_KM, 0, 1, CONF_KM_CONFORMANCE);
	AdpSetRequest(ADP_IB_KC, 0, 1, CONF_KC_CONFORMANCE);
	AdpSetRequest(ADP_IB_KQ, 0, 1, CONF_KQ_CONFORMANCE);
	AdpSetRequest(ADP_IB_KH, 0, 1, CONF_KH_CONFORMANCE);
	AdpSetRequest(ADP_IB_KRT, 0, 1, CONF_KRT_CONFORMANCE);
	AdpSetRequest(ADP_IB_KQ_RF, 0, 1, CONF_KQ_RF_CONFORMANCE);
	AdpSetRequest(ADP_IB_KH_RF, 0, 1, CONF_KH_RF_CONFORMANCE);
	AdpSetRequest(ADP_IB_KRT_RF, 0, 1, CONF_KRT_RF_CONFORMANCE);
	AdpSetRequest(ADP_IB_KDC_RF, 0, 1, CONF_KDC_RF_CONFORMANCE);
  #endif
  #if defined(__RF_MAC__)
	// In case we get here from a Kick event, restore RF configuration set by UDP responder
	change_conformance_rf_config(savedRFParams, savedRFParamLength);
  #endif
}

static void InitializeModemParameters(void)
{
	struct TLbpSetParamConfirm lbpSetConfirm;
	g_u8MibInitIndex = 0;

	while (g_u8MibInitIndex < APP_MIB_TABLE_SIZE) {
		if (g_u8MibInitIndex == 0) {
			LOG_APP_DEBUG(("Start modem initialization.\r\n"));
		}

		/* this method assumes that the g_u8MibInitIndex value has been already checked: (g_u8MibInitIndex < APP_MIB_TABLE_SIZE) */
		LOG_APP_DEBUG(("Setting command %hhu: %s / %hu\r\n", g_u8MibInitIndex, g_MibSettings[g_u8MibInitIndex].m_szName,
				g_MibSettings[g_u8MibInitIndex].m_u16Index));
		if (g_MibSettings[g_u8MibInitIndex].m_u8Type == MIB_ADP) {
			AdpSetRequest(g_MibSettings[g_u8MibInitIndex].m_u32Id, g_MibSettings[g_u8MibInitIndex].m_u16Index,
					g_MibSettings[g_u8MibInitIndex].m_u8ValueLength, g_MibSettings[g_u8MibInitIndex].m_pu8Value);
		} else {
			AdpMacSetRequest(g_MibSettings[g_u8MibInitIndex].m_u32Id, g_MibSettings[g_u8MibInitIndex].m_u16Index,
					g_MibSettings[g_u8MibInitIndex].m_u8ValueLength, g_MibSettings[g_u8MibInitIndex].m_pu8Value);
		}
	}

	if (bConformanceMode) {
		/* Override configuration with Conformance ones */
		SetConformanceModemParameters();
	}

	LBP_SetParamDev(LBP_IB_PSK, 0, 16, CONF_PSK_KEY, &lbpSetConfirm);
}

void adp_app_set_conformance_config(void)
{
	bConformanceMode = true;
	/* Set Conformance parameters, just in case this function is called after standard parameter initialization */
	SetConformanceModemParameters();
}

uint8_t change_conformance_rf_config(uint8_t *pParameters, uint16_t u16ParamLength)
{
	uint8_t u8Result;
#if defined(__RF_MAC__)
	uint8_t u8FrequencyBand;
	uint8_t u8OperatingMode;
	uint16_t u16FreqBandOpMode;
	uint8_t u8HoppingActivation;
	uint16_t u16ChannelNumber;
	struct TAdpMacSetConfirm setConfirm;

	if ((u16ParamLength > 2) && (u16ParamLength < 5)) {
		/* Store parameters to restore them if needed */
		savedRFParamLength = u16ParamLength;
		memcpy(savedRFParams, pParameters, u16ParamLength);
		/* Decode parameters */
		u8FrequencyBand = *pParameters++;
		u8OperatingMode = *pParameters++;
		u8HoppingActivation = *pParameters++;
		if (u8HoppingActivation == 0x00) {
			u16ChannelNumber = (uint16_t)(*pParameters++);
		} else {
			u16ChannelNumber = 0;
		}

		/* Set parameters */
		u16FreqBandOpMode = ((uint16_t)(u8FrequencyBand) << 8) | (uint16_t)(u8OperatingMode);
		AdpMacSetRequestSync(MAC_WRP_PIB_MANUF_PHY_PARAM_RF, MAC_WRP_RF_PHY_PARAM_PHY_BAND_OPERATING_MODE, 2, (const uint8_t *)&u16FreqBandOpMode, &setConfirm);
		if (setConfirm.m_u8Status == G3_SUCCESS) {
			if (u8HoppingActivation == 0x00) {
				AdpMacSetRequestSync(MAC_WRP_PIB_MANUF_PHY_PARAM_RF, MAC_WRP_RF_PHY_PARAM_PHY_CHANNEL_NUM, 2, (const uint8_t *)&u16ChannelNumber, &setConfirm);
				if (setConfirm.m_u8Status == G3_SUCCESS) {
					u8Result = 0x00;
				}
				else {
					/* Set confirm not success */
					u8Result = 0x01;
				}
			}
			else {
				/* Hopping not yet supported */
				u8Result = 0x02;
			}
		}
		else {
			/* Set confirm not success */
			u8Result = 0x01;
		}
	}
	else {
		/* Incorrect request length */
		u8Result = 0x02;
	}
#else
	UNUSED(pParameters);
	UNUSED(u16ParamLength);
	/* Return 0x02 which means unsupported feature */
	u8Result = 0x02;
#endif
	return u8Result;
}

uint8_t set_rf_continuous_tx(void)
{
	uint8_t u8Result;
#if defined(__RF_MAC__)
	struct TAdpMacSetConfirm setConfirm;
	uint8_t u8True = 1;

	AdpMacSetRequestSync(MAC_WRP_PIB_MANUF_PHY_PARAM_RF, MAC_WRP_RF_PHY_PARAM_CONTINUOUS_TX_MODE, 1, (const uint8_t *)&u8True, &setConfirm);
	u8Result = setConfirm.m_u8Status;
#else
	/* Return Invalid Parameter if not Hybrid */
	u8Result = G3_INVALID_PARAMETER;
#endif
	return u8Result;
}

uint8_t change_conformance_trickle_config(uint8_t *pParameters)
{
	uint8_t u8Result;
	uint8_t u8TrickleActivation;
	uint8_t u8ParamTrue = 1;
	uint8_t u8ParamFalse = 0;
	uint16_t u16ParamDealyLowLQI;
	uint16_t u16ParamDealyHighLQI;
	struct TAdpMacSetConfirm macSetConfirm;
	struct TAdpSetConfirm adpSetConfirm;

	/* Decode parameters */
	u8TrickleActivation = *pParameters;
	if (u8TrickleActivation == 0x01) {
		/* Disable PHY RF */
		AdpMacSetRequestSync(MAC_WRP_PIB_DISABLE_PHY_RF, 0, 1, (const uint8_t *)&u8ParamTrue, &macSetConfirm);
		/* Enable Trickle Data */
		AdpSetRequestSync(ADP_IB_TRICKLE_DATA_ENABLED, 0, 1, (const uint8_t *)&u8ParamTrue, &adpSetConfirm);
		if (adpSetConfirm.m_u8Status == G3_SUCCESS) {
			/* Enable Cluster Trickle */
			AdpSetRequestSync(ADP_IB_CLUSTER_TRICKLE_ENABLED, 0, 1, (const uint8_t *)&u8ParamTrue, &adpSetConfirm);
			if (adpSetConfirm.m_u8Status == G3_SUCCESS) {
				/* Set Delay Low LQI */
				u16ParamDealyLowLQI = 1500;
				AdpSetRequestSync(ADP_IB_DELAY_LOW_LQI, 0, 2, (const uint8_t *)&u16ParamDealyLowLQI, &adpSetConfirm);
				if (adpSetConfirm.m_u8Status == G3_SUCCESS) {
					/* Set Delay High LQI */
					u16ParamDealyHighLQI = 100;
					AdpSetRequestSync(ADP_IB_DELAY_HIGH_LQI, 0, 2, (const uint8_t *)&u16ParamDealyHighLQI, &adpSetConfirm);
					if (adpSetConfirm.m_u8Status == G3_SUCCESS) {
						/* All params correctly set */
						u8Result = 0x00;
					}
					else {
						/* Set confirm not success */
						u8Result = 0x01;
					}
				}
				else {
					/* Set confirm not success */
					u8Result = 0x01;
				}
			}
			else {
				/* Set confirm not success */
				u8Result = 0x01;
			}
		}
		else {
			/* Set confirm not success */
			u8Result = 0x01;
		}
	}
	else if (u8TrickleActivation == 0x00) {
		/* Enable PHY RF */
		AdpMacSetRequestSync(MAC_WRP_PIB_DISABLE_PHY_RF, 0, 1, (const uint8_t *)&u8ParamFalse, &macSetConfirm);
		/* Disable Trickle Data */
		AdpSetRequestSync(ADP_IB_TRICKLE_DATA_ENABLED, 0, 1, (const uint8_t *)&u8ParamFalse, &adpSetConfirm);
		if (adpSetConfirm.m_u8Status == G3_SUCCESS) {
			/* Disable Cluster Trickle */
			AdpSetRequestSync(ADP_IB_CLUSTER_TRICKLE_ENABLED, 0, 1, (const uint8_t *)&u8ParamFalse, &adpSetConfirm);
			if (adpSetConfirm.m_u8Status == G3_SUCCESS) {
				/* Set Delay Low LQI */
				u16ParamDealyLowLQI = 0;
				AdpSetRequestSync(ADP_IB_DELAY_LOW_LQI, 0, 2, (const uint8_t *)&u16ParamDealyLowLQI, &adpSetConfirm);
				if (adpSetConfirm.m_u8Status == G3_SUCCESS) {
					/* Set Delay High LQI */
					u16ParamDealyHighLQI = 0;
					AdpSetRequestSync(ADP_IB_DELAY_HIGH_LQI, 0, 2, (const uint8_t *)&u16ParamDealyHighLQI, &adpSetConfirm);
					if (adpSetConfirm.m_u8Status == G3_SUCCESS) {
						/* All params correctly set */
						u8Result = 0x00;
					}
					else {
						/* Set confirm not success */
						u8Result = 0x01;
					}
				}
				else {
					/* Set confirm not success */
					u8Result = 0x01;
				}
			}
			else {
				/* Set confirm not success */
				u8Result = 0x01;
			}
		}
		else {
			/* Set confirm not success */
			u8Result = 0x01;
		}
	}
	else {
		/* Unexpected value */
		u8Result = 0x01;
	}

	return u8Result;
}

void set_last_gasp_mode(void)
{
	struct TAdpSetConfirm adpSetConfirm;
	uint8_t u8True = 1;

	/* Enable Last Gasp */
	AdpSetRequestSync(ADP_IB_LAST_GASP, 0, 1, (const uint8_t *)&u8True, &adpSetConfirm);
}

static void DiscoveryNetwork(void)
{
	LOG_APP_DEBUG(("Search for networks...\r\n"));

	g_BestNetwork.m_u16PanId = 0xFFFF;
	g_BestNetwork.m_u16LbaAddress = 0xFFFF;
	g_BestNetwork.m_u16RcCoord = 0xFFFF;
	g_BestNetwork.m_u8LinkQuality = 0x00;

	AdpDiscoveryRequest(CONF_DISCOVERY_TIMEOUT);
}

static void StartDevice(void)
{
	/* Set configuration values for user-specified ADP parameters */
	InitializeModemParameters();

#ifdef ENABLE_PIB_RESTORE
	if (bLoadPersistentInfo) {
		/* Only load persistent info on reset, not every time this function is called */
		bLoadPersistentInfo = false;
		load_persistent_info();
	}
#endif
}

void adp_app_init(void)
{
	/* Set local vars */
	g_u8NetworkJoinStatus = NJS_INIT_DELAY;
	LogJoinStatus();
	u32BackOffTimer = 0;
	u32ChannelCheckTimer = 0;
	u8JoinTries = 0;
	g_u8MibInitIndex = 0;
	g_u16ShortAddress = 0;
	g_u16PanId = 0;
	u32BackOffDiscoveryCW = NETWORK_DISCOVERY_MIN_BACKOFF_TIME_MS;
	u32BackOffJoinCW = NETWORK_JOIN_MIN_BACKOFF_TIME_MS;
	/* Init G3 */
	InitializeStack();

	/* Set initial Start Up random process */
	u32DelayStartUp = GetRandomDelay(0, CONF_STARTUP_TIMEOUT_MAX);

	/* Init IP stack */
	ipv6_mng_initialize();
}

void adp_app_timers_update(void)
{
	if (u32DelayStartUp) {
		u32DelayStartUp--;
	}

	if (u32BackOffTimer) {
		u32BackOffTimer--;
	}

	if (u32ChannelCheckTimer) {
		u32ChannelCheckTimer--;
	}
}

void adp_app_process(void)
{
	if (u32DelayStartUp) {
		/* Random Start Up Process */
		return;
	} else {
		if (g_u8NetworkJoinStatus == NJS_INIT_DELAY) {
			/* Check valid Meter Id before starting G3 */
			if (g_bHasMeterId) {
				/* Set parameters defined by application */
				StartDevice();
				/* Continue to next state */
				g_u8NetworkJoinStatus = NJS_NOT_JOINED;
				LogJoinStatus();
			} else {
				u32DelayStartUp = GetRandomDelay(0, CONF_STARTUP_TIMEOUT_MAX);
			}
		}
	}

	if (u32BackOffTimer) {
		/* Waiting ... Backoff time */
		return;
	} else {
		switch (g_u8NetworkJoinStatus) {
		case NJS_NOT_JOINED:
			g_u8NetworkJoinStatus = NJS_CHECK_FOR_BEACONS;
			LogJoinStatus();

			break;

		case NJS_SCAN_BACKOFF:
			g_u8NetworkJoinStatus = NJS_CHECK_FOR_BEACONS;
			LogJoinStatus();
			ResetChannelStatusIndicators();
			u32ChannelCheckTimer = NETWORK_DISCOVERY_CHANNEL_CHECK_TIME_MS;
			u32BackOffDiscoveryCW = u32BackOffDiscoveryCW * 2;
			if (u32BackOffDiscoveryCW > NETWORK_DISCOVERY_MAX_BACKOFF_TIME_MS) {
				u32BackOffDiscoveryCW = NETWORK_DISCOVERY_MAX_BACKOFF_TIME_MS;
			}

			break;

		case NJS_JOINNING_BACKOFF:
			g_u8NetworkJoinStatus = NJS_CHECK_FOR_LBP_LNG;
			LogJoinStatus();
			ResetChannelStatusIndicators();
			u32ChannelCheckTimer = NETWORK_JOIN_CHANNEL_CHECK_TIME_MS;
			u32BackOffJoinCW = u32BackOffJoinCW * 2;
			if (u32BackOffJoinCW > NETWORK_JOIN_MAX_BACKOFF_TIME_MS) {
				u32BackOffJoinCW = NETWORK_JOIN_MAX_BACKOFF_TIME_MS;
			}

			break;

		case NJS_SCANNING:
			LOG_APP_DEBUG(("ERR AppAdpDiscoveryConfirm not received\r\n"));
			g_u8NetworkJoinStatus = NJS_NOT_JOINED;
			LogJoinStatus();
			break;

		default:
			break;
		}
	}

	if (u32ChannelCheckTimer) {
		/* Waiting ... Network In Progress */
		return;
	} else {
		switch (g_u8NetworkJoinStatus) {
		case NJS_CHECK_FOR_BEACONS:
			if (CheckBeaconFramesReceived()) {
				g_u8NetworkJoinStatus = NJS_SCAN_BACKOFF;
				LogJoinStatus();
				u32BackOffTimer = GetRandomDelay(NETWORK_DISCOVERY_MIN_BACKOFF_TIME_MS, u32BackOffDiscoveryCW);
			} else {
				g_u8NetworkJoinStatus = NJS_SCANNING;
				u32BackOffDiscoveryCW = u32BackOffDiscoveryCW >> 1;
				if (u32BackOffDiscoveryCW < NETWORK_DISCOVERY_MIN_BACKOFF_TIME_MS) {
					u32BackOffDiscoveryCW = NETWORK_DISCOVERY_MIN_BACKOFF_TIME_MS;
				}

				/* Set a timeout double the Discovery Time in case Discovery Confirm is not called */
				u32BackOffTimer = CONF_DISCOVERY_TIMEOUT * 2000;
				LogJoinStatus();
				DiscoveryNetwork();
			}

			break;

		case NJS_CHECK_FOR_LBP_LNG:
			if (CheckLBPFramesReceived() || CheckLNGFramesReceived() || CheckBeaconFramesReceived()) {
				g_u8NetworkJoinStatus = NJS_JOINNING_BACKOFF;
				LogJoinStatus();
				u32BackOffTimer =  GetRandomDelay(NETWORK_JOIN_MIN_BACKOFF_TIME_MS, u32BackOffJoinCW);
			} else {
				g_u8NetworkJoinStatus = NJS_JOINNING;
				u32BackOffJoinCW = u32BackOffJoinCW >> 1;
				if (u32BackOffJoinCW < NETWORK_JOIN_MIN_BACKOFF_TIME_MS) {
					u32BackOffJoinCW = NETWORK_JOIN_MIN_BACKOFF_TIME_MS;
				}

				LogJoinStatus();
				AdpNetworkJoinRequest(g_BestNetwork.m_u16PanId, g_BestNetwork.m_u16LbaAddress, g_BestNetwork.m_u8MediaType);
			}

			break;

		default:
			break;
		}
	}
}
