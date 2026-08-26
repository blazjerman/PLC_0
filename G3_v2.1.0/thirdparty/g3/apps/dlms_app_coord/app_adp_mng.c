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

#include <storage/storage.h>
#include "compiler.h"
#include "AdpApi.h"
#include "ProcessLbpCoord.h"
#include "LbpDefs.h"
#include "drivers/g3/network_adapter_g3.h"
#include "ipv6_mng.h"
#include "app_ping_coord.h"
#include "app_adp_mng.h"
#include "app_dlms_coord.h"
#include "g3_app_config.h"
#include "conf_project.h"
#include "conf_global.h"
#include "hal/hal.h"
#include "bs_api.h"

/* MAC include */
#include "mac_wrapper.h"

#ifdef APP_DEBUG_CONSOLE
#       define LOG_APP_DEBUG(a)   printf a
#else
#       define LOG_APP_DEBUG(a)   (void)0
#endif

/* constants related to network join */
enum NetworkJoinStatus {
	NETWORK_NOT_JOINED = 0,
	NETWORK_JOIN_PENDING = 1,
	NETWORK_JOINED = 2
};

/* constants related to network start */
enum NetworkStartStatus {
	NETWORK_NOT_STARTED = 0,
	NETWORK_START_PENDING = 1,
	NETWORK_STARTED = 2
};

/* global variables */
uint8_t g_u8NetworkJoinStatus = NETWORK_NOT_JOINED;
uint8_t g_u8NetworkStartStatus = NETWORK_NOT_STARTED;

/* Context information, to be updated when PanId is known */
#define CONTEXT_INFORMATION_0_SIZE   14
static uint8_t au8ConfContextInformationTable0[CONTEXT_INFORMATION_0_SIZE];

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
	uint8_t m_u8Type;
	const char *m_szName;
	uint32_t m_u32Id;
	uint16_t m_u16Index;
	uint8_t m_u8ValueLength;
	const uint8_t *m_pu8Value;
};

extern uint8_t g_auc_ext_address[8]; /* EUI64 */ uint8_t g_u8MibInitIndex = 0;

#define APP_MIB_TABLE_SIZE (sizeof(g_MibSettings) / sizeof(struct MibData))

struct MibData g_MibSettings[] = {
	{ MIB_MAC, "MAC_WRP_PIB_MANUF_EXTENDED_ADDRESS", MAC_WRP_PIB_MANUF_EXTENDED_ADDRESS, 0, 8, g_auc_ext_address },
	{ MIB_MAC, "MAC_WRP_PIB_SHORT_ADDRESS", MAC_WRP_PIB_SHORT_ADDRESS, 0, 2, CONF_SHORT_ADDRESS },
#if defined(__RF_MAC__)
	{ MIB_MAC, "MAC_WRP_PIB_DUTY_CYCLE_LIMIT_RF", MAC_WRP_PIB_DUTY_CYCLE_LIMIT_RF, 0, 2, CONF_DUTY_CYCLE_LIMIT_RF },
#endif
#ifdef APP_CONFORMANCE_TEST
  #ifdef CONF_BAND_ARIB
    #if (ENABLE_ROUTING == 1)
	{ MIB_ADP, "ADP_IB_DISABLE_DEFAULT_ROUTING", ADP_IB_DISABLE_DEFAULT_ROUTING, 0, 1, CONF_DISABLE_LOADNG },
    #endif
  #endif
	{ MIB_ADP, "ADP_IB_BLACKLIST_TABLE_ENTRY_TTL", ADP_IB_BLACKLIST_TABLE_ENTRY_TTL, 0, 2, CONF_BLACKLIST_TABLE_ENTRY_TTL },
	{ MIB_ADP, "ADP_IB_GROUP_TABLE", ADP_IB_GROUP_TABLE, 0, 2, CONF_GROUP_TABLE_0 },
	{ MIB_ADP, "ADP_IB_GROUP_TABLE", ADP_IB_GROUP_TABLE, 1, 2, CONF_GROUP_TABLE_1 },
	{ MIB_MAC, "MAC_WRP_PIB_TMR_TTL", MAC_WRP_PIB_TMR_TTL, 0, 1, CONF_TMR_TTL },
	{ MIB_ADP, "ADP_IB_BROADCAST_LOG_TABLE_ENTRY_TTL", ADP_IB_BROADCAST_LOG_TABLE_ENTRY_TTL, 0, 2, CONF_BROADCAST_LOG_TABLE_ENTRY_TTL },
	{ MIB_ADP, "ADP_IB_WEAK_LQI_VALUE", ADP_IB_WEAK_LQI_VALUE, 0, 1, CONF_WEAK_LQI_CONFORMANCE },
	{ MIB_ADP, "ADP_IB_WEAK_LQI_VALUE_RF", ADP_IB_WEAK_LQI_VALUE_RF, 0, 1, CONF_WEAK_LQI_RF_CONFORMANCE },
	{ MIB_ADP, "ADP_IB_TRICKLE_DATA_ENABLED", ADP_IB_TRICKLE_DATA_ENABLED, 0, 1, CONF_TRICKLE_DATA_ENABLED_CONFORMANCE },
	{ MIB_ADP, "ADP_IB_TRICKLE_ADAPTIVE_KI", ADP_IB_TRICKLE_ADAPTIVE_KI, 0, 1, CONF_TRICKLE_ADAPTIVE_KI_CONFORMANCE },
	{ MIB_ADP, "ADP_IB_TRICKLE_LQI_THRESHOLD_LOW_RF", ADP_IB_TRICKLE_LQI_THRESHOLD_LOW_RF, 0, 1, CONF_TRICKLE_LQI_THRESHOLD_LOW_RF_CONFORMANCE },
	{ MIB_ADP, "ADP_IB_TRICKLE_LQI_THRESHOLD_HIGH_RF", ADP_IB_TRICKLE_LQI_THRESHOLD_HIGH_RF, 0, 1, CONF_TRICKLE_LQI_THRESHOLD_HIGH_RF_CONFORMANCE },
	{ MIB_ADP, "ADP_IB_CLUSTER_TRICKLE_ENABLED", ADP_IB_CLUSTER_TRICKLE_ENABLED, 0, 1, CONF_CLUSTER_TRICKLE_ENABLED_CONFORMANCE },
	{ MIB_ADP, "ADP_IB_DELAY_LOW_LQI_RF", ADP_IB_DELAY_LOW_LQI_RF, 0, 2, CONF_DELAY_LOW_LQI_CONFORMANCE },
	{ MIB_ADP, "ADP_IB_DELAY_HIGH_LQI_RF", ADP_IB_DELAY_HIGH_LQI_RF, 0, 2, CONF_DELAY_HIGH_LQI_CONFORMANCE },
	{ MIB_ADP, "ADP_IB_RREQ_JITTER_LOW_LQI_RF", ADP_IB_RREQ_JITTER_LOW_LQI_RF, 0, 1, CONF_JITTER_LOW_LQI_RF_CONFORMANCE },
	{ MIB_ADP, "ADP_IB_RREQ_JITTER_HIGH_LQI_RF", ADP_IB_RREQ_JITTER_HIGH_LQI_RF, 0, 1, CONF_JITTER_HIGH_LQI_RF_CONFORMANCE },
	{ MIB_ADP, "ADP_IB_CLUSTER_MIN_LQI_RF", ADP_IB_CLUSTER_MIN_LQI_RF, 0, 1, CONF_CLUSTER_MIN_LQI_RF_CONFORMANCE },
	{ MIB_ADP, "ADP_IB_CLUSTER_TRICKLE_I_RF", ADP_IB_CLUSTER_TRICKLE_I_RF, 0, 2, CONF_CLUSTER_TRICKLE_I_CONFORMANCE },
	{ MIB_ADP, "ADP_IB_DELAY_LOW_LQI", ADP_IB_DELAY_LOW_LQI, 0, 2, CONF_DELAY_LOW_LQI_CONFORMANCE },
	{ MIB_ADP, "ADP_IB_DELAY_HIGH_LQI", ADP_IB_DELAY_HIGH_LQI, 0, 2, CONF_DELAY_HIGH_LQI_CONFORMANCE },
	{ MIB_ADP, "ADP_IB_CLUSTER_TRICKLE_I", ADP_IB_CLUSTER_TRICKLE_I, 0, 2, CONF_CLUSTER_TRICKLE_I_CONFORMANCE },
	{ MIB_ADP, "ADP_IB_PROBING_INTERVAL", ADP_IB_PROBING_INTERVAL, 0, 1, CONF_PROBING_INTERVAL },
	{ MIB_ADP, "ADP_IB_KR", ADP_IB_KR, 0, 2, CONF_KR },
    #if (ENABLE_ROUTING == 1)
	{ MIB_ADP, "ADP_IB_DESTINATION_ADDRESS_SET", ADP_IB_DESTINATION_ADDRESS_SET, 0, 2, CONF_DEST_ADDR_SET_0 },
    #endif
    #if defined(__RF_MAC__)
	{ MIB_MAC, "MAC_WRP_PIB_MAX_CSMA_BACKOFFS_RF", MAC_WRP_PIB_MAX_CSMA_BACKOFFS_RF, 0, 1, CONF_MAX_CSMA_BACKOFFS_RF },
	{ MIB_MAC, "MAC_WRP_PIB_MAX_FRAME_RETRIES_RF", MAC_WRP_PIB_MAX_FRAME_RETRIES_RF, 0, 1, CONF_MAX_FRAME_RETRIES_RF },
	{ MIB_MAC, "MAC_WRP_PIB_ADAPTIVE_POWER_LOW_BOUND_RF", MAC_WRP_PIB_ADAPTIVE_POWER_LOW_BOUND_RF, 0, 1, CONF_ADAPTIVE_POWER_LOW_BOUND_CONFORMANCE },
    #endif
	{ MIB_MAC, "MAC_WRP_PIB_POS_TABLE_ENTRY_TTL", MAC_WRP_PIB_POS_TABLE_ENTRY_TTL, 0, 1, CONF_POS_TABLE_TTL },
	{ MIB_MAC, "MAC_WRP_PIB_MAX_CSMA_BACKOFFS", MAC_WRP_PIB_MAX_CSMA_BACKOFFS, 0, 1, CONF_MAX_CSMA_BACKOFFS_CONFORMANCE },
	{ MIB_MAC, "MAC_WRP_PIB_POS_RECENT_ENTRY_THRESHOLD", MAC_WRP_PIB_POS_RECENT_ENTRY_THRESHOLD, 0, 1, CONF_POS_RECENT_THRESHOLD_CONFORMANCE },
    #if (ENABLE_ROUTING == 1)
	{ MIB_ADP, "ADP_IB_KR", ADP_IB_KR, 0, 1, CONF_KR },
	{ MIB_ADP, "ADP_IB_KM", ADP_IB_KM, 0, 1, CONF_KM },
	{ MIB_ADP, "ADP_IB_KC", ADP_IB_KC, 0, 1, CONF_KC },
	{ MIB_ADP, "ADP_IB_KQ", ADP_IB_KQ, 0, 1, CONF_KQ },
	{ MIB_ADP, "ADP_IB_KH", ADP_IB_KH, 0, 1, CONF_KH },
	{ MIB_ADP, "ADP_IB_KRT", ADP_IB_KRT, 0, 1, CONF_KRT },
	{ MIB_ADP, "ADP_IB_KQ_RF", ADP_IB_KQ_RF, 0, 1, CONF_KQ_RF },
	{ MIB_ADP, "ADP_IB_KH_RF", ADP_IB_KH_RF, 0, 1, CONF_KH_RF },
	{ MIB_ADP, "ADP_IB_KRT_RF", ADP_IB_KRT_RF, 0, 1, CONF_KRT_RF },
	{ MIB_ADP, "ADP_IB_KDC_RF", ADP_IB_KDC_RF, 0, 1, CONF_KDC_RF },
    #endif
#endif
	{ MIB_ADP, "ADP_IB_CONTEXT_INFORMATION_TABLE", ADP_IB_CONTEXT_INFORMATION_TABLE, 0, 14, CONF_CONTEXT_INFORMATION_TABLE_0 },
	{ MIB_ADP, "ADP_IB_CONTEXT_INFORMATION_TABLE", ADP_IB_CONTEXT_INFORMATION_TABLE, 1, 10, CONF_CONTEXT_INFORMATION_TABLE_1 },
#if (ENABLE_ROUTING == 1)
	{ MIB_ADP, "ADP_IB_ROUTING_TABLE_ENTRY_TTL", ADP_IB_ROUTING_TABLE_ENTRY_TTL, 0, 2, CONF_ROUTING_TABLE_ENTRY_TTL },
#endif
	{ MIB_ADP, "ADP_IB_MAX_JOIN_WAIT_TIME", ADP_IB_MAX_JOIN_WAIT_TIME, 0, 2, CONF_MAX_JOIN_WAIT_TIME },
	{ MIB_ADP, "ADP_IB_MAX_HOPS", ADP_IB_MAX_HOPS, 0, 1, CONF_MAX_HOPS }
};

static void SetConfirm(uint8_t u8Status, uint32_t u32AttributeId, uint16_t u16AttributeIndex)
{
	if (u8Status == G3_SUCCESS) {
		/* initialization phase? */
		if (g_u8MibInitIndex < APP_MIB_TABLE_SIZE) {
			if ((g_MibSettings[g_u8MibInitIndex].m_u32Id != u32AttributeId) || (g_MibSettings[g_u8MibInitIndex].m_u16Index != u16AttributeIndex)) {
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
	}
	else {
		LOG_APP_DEBUG(("ERR[AppAdpSetConfirm] status: %hhu\r\n", u8Status));
	}

	g_u8MibInitIndex++;
	if (g_u8MibInitIndex == APP_MIB_TABLE_SIZE) {
		LOG_APP_DEBUG(("Modem fully initialized.\r\n"));
	}
}

static void AppAdpDataConfirm(struct TAdpDataConfirm *pDataConfirm)
{
	UNUSED(pDataConfirm);
}

static void AppAdpDataIndication(struct TAdpDataIndication *pDataIndication)
{
	if (pDataIndication->m_u16NsduLength) {
		ipv6_receive_packet((struct TAdpDataIndication *)pDataIndication);
		LOG_APP_DEBUG(("ERR[AppAdpDataIndication] Len: %hu LQI: %hhu\r\n", pDataIndication->m_u16NsduLength, pDataIndication->m_u8LinkQualityIndicator));
	}
}

static void AppAdpDiscoveryConfirm(uint8_t uc_status)
{
	if (uc_status == G3_SUCCESS) {
		LOG_APP_DEBUG(("AppAdpDiscoveryConfirm SUCCESS"));

		/* The coordinator is the network creator. Network join is not used. */
		g_u8NetworkJoinStatus = NETWORK_JOINED;
	} else {
		LOG_APP_DEBUG(("ERR[AppAdpDiscoveryConfirm] Status %hhu", uc_status));
		g_u8NetworkJoinStatus = NETWORK_NOT_JOINED;
	}
}

static void AppAdpDiscoveryIndication(struct TAdpPanDescriptor *pPanDescriptor)
{
	UNUSED(pPanDescriptor);
}

static void AppAdpNetworkStartConfirm(struct TAdpNetworkStartConfirm *pNetworkStartConfirm)
{
	UNUSED(pNetworkStartConfirm);
}

static void AppAdpResetConfirm(struct TAdpResetConfirm *pResetConfirm)
{
	UNUSED(pResetConfirm);
}

static void AppAdpSetConfirm(struct TAdpSetConfirm *pSetConfirm)
{
	SetConfirm(pSetConfirm->m_u8Status, pSetConfirm->m_u32AttributeId, pSetConfirm->m_u16AttributeIndex);
}

static void AppAdpMacSetConfirm(struct TAdpMacSetConfirm *pSetConfirm)
{
	SetConfirm(pSetConfirm->m_u8Status, pSetConfirm->m_u32AttributeId, pSetConfirm->m_u16AttributeIndex);
}

static void AppAdpGetConfirm(struct TAdpGetConfirm *pGetConfirm)
{
	UNUSED(pGetConfirm);
}

static void AppAdpMacGetConfirm(struct TAdpMacGetConfirm *pGetConfirm)
{
	UNUSED(pGetConfirm);
}

static void AppAdpRouteDiscoveryConfirm(struct TAdpRouteDiscoveryConfirm *pRouteDiscoveryConfirm)
{
	UNUSED(pRouteDiscoveryConfirm);
}

static void AppAdpPathDiscoveryConfirm(struct TAdpPathDiscoveryConfirm *pPathDiscoveryConfirm)
{
	UNUSED(pPathDiscoveryConfirm);

	dlms_app_path_node_cfm(pPathDiscoveryConfirm);
}

static void AppAdpNetworkStatusIndication(struct TAdpNetworkStatusIndication *pNetworkStatusIndication)
{
	UNUSED(pNetworkStatusIndication);
}

static void AppAdpBufferIndication(TAdpBufferIndication *pBufferIndication)
{
	ping_app_set_buffer_ready((pBufferIndication->m_u8LargeBuffersAvailable == 1));
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

	/* Set ADP managment */
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
	notifications.fnctAdpBufferIndication = AppAdpBufferIndication;
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
}

static void InitializeModemParameters(void)
{
	g_u8MibInitIndex = 0;

	while (g_u8MibInitIndex < APP_MIB_TABLE_SIZE) {
		if (g_u8MibInitIndex == 0) {
			LOG_APP_DEBUG(("Start modem initialization.\r\n"));
		}

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
  #if defined(__RF_MAC__)
	// In case we get here from a Kick event, restore RF configuration set by UDP responder
	change_conformance_rf_config(savedRFParams, savedRFParamLength);
  #endif
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

#ifdef APP_CONFORMANCE_TEST
/* In conformance, send beacon request before starting network*/
static void StartDiscovery(void)
{
	g_u8NetworkJoinStatus = NETWORK_JOIN_PENDING;

	AdpDiscoveryRequest(15);
}

#endif

static void StartCoordinator(void)
{
	struct TLbpSetParamConfirm lbpSetConfirm;
	bool bArib;

	/* Set configuration values for user-specified ADP parameters */
	InitializeModemParameters();

#ifdef ENABLE_PIB_RESTORE
	load_persistent_info();
#endif

	/* Init Coordinator module */
#if defined (CONF_BAND_ARIB)
	bArib = true;
#else
	bArib = false;
#endif

	coord_init(bArib);
	coord_join_ind_set_cb(dlms_app_join_node);
	coord_leave_ind_set_cb(dlms_app_leave_node);

	/* Set PSK */
	LBP_SetParamCoord(LBP_IB_PSK, 0, 16, CONF_PSK_KEY, &lbpSetConfirm);

	/* Set GMK */
	LBP_SetParamCoord(LBP_IB_GMK, 0, 16, CONF_GMK_KEY, &lbpSetConfirm);

	/* Update IPv6 link */
	ipv6_mng_upd_link(G3_COORDINATOR_PAN_ID, 0);

	/* Set Context with network PanId before Network Start */
	memcpy(au8ConfContextInformationTable0, CONF_CONTEXT_INFORMATION_TABLE_0, CONTEXT_INFORMATION_0_SIZE);
	au8ConfContextInformationTable0[CONTEXT_INFORMATION_0_SIZE - 2] = (uint8_t)((G3_COORDINATOR_PAN_ID >> 8) & 0xFF);
	au8ConfContextInformationTable0[CONTEXT_INFORMATION_0_SIZE - 1] = (uint8_t)(G3_COORDINATOR_PAN_ID & 0xFF);
	AdpSetRequest(ADP_IB_CONTEXT_INFORMATION_TABLE, 0, CONTEXT_INFORMATION_0_SIZE, au8ConfContextInformationTable0);

	/* Start G3 Network */
	AdpNetworkStartRequest(G3_COORDINATOR_PAN_ID);
}

/**
 * \brief Create main Cycles Application task and create timer to update internal counters.
 *
 */
void app_init(void)
{
	/* Init modules */
	InitializeStack();
}

/**
 * \brief Update internal counters.
 *
 */
void app_timers_update(void)
{
}

/**
 * \brief Periodic task to process Cycles App. Initialize and start Cycles Application and launch timer
 * to update internal counters.
 *
 */
void app_process(void)
{
	if (g_u8NetworkJoinStatus == NETWORK_NOT_JOINED) {
#ifdef APP_CONFORMANCE_TEST
		/* In conformance, send beacon request before starting network*/
		StartDiscovery();
#else
		g_u8NetworkJoinStatus = NETWORK_JOINED;
#endif
	} else if (g_u8NetworkJoinStatus == NETWORK_JOINED) {
		/* network start will start only after the modem is initialized */
		if (g_u8NetworkStartStatus == NETWORK_NOT_STARTED) {
			/* Start node as coordinator */
			StartCoordinator();
			g_u8NetworkStartStatus = NETWORK_STARTED;
		}
	}

	/* Call bootstrap process*/
	coord_process();
}

/**
 * \brief Update the list of registered nodes from Bootstrap module.
 *
 */
uint16_t app_update_registered_nodes(void *pxNodeList)
{
	x_node_list_t *px_list_ptr;
	struct t_coord_get_param_confirm p_get_confirm;
	uint16_t us_idx, us_num_devices, us_device_cnt;

	px_list_ptr = pxNodeList;

	/* Get the number of devices from Bootstrap module */
	us_num_devices = coord_get_lbds_counter();
	us_device_cnt = 0;

	/* If no devices found, return */
	if (us_num_devices == 0) {
		return 0;
	}

	/* Update Device Addresses from Bootstrap module */
	for (us_idx = 0; us_idx < MAX_LBDS; us_idx++) {
		coord_get_param(COORD_IB_DEVICE_LIST, us_idx, &p_get_confirm);
		if (p_get_confirm.uc_status == COORD_STATUS_OK) {
			px_list_ptr->us_short_address = ((uint16_t)p_get_confirm.uc_attribute_value[1]) << 8;
			px_list_ptr->us_short_address += ((uint16_t)p_get_confirm.uc_attribute_value[0]);
			memcpy(px_list_ptr->puc_extended_address, &p_get_confirm.uc_attribute_value[2], sizeof(px_list_ptr->puc_extended_address));
			px_list_ptr++;
			us_device_cnt++;
			if (us_device_cnt == us_num_devices) {
				break;
			}
		}
	}

	return us_num_devices;
}
