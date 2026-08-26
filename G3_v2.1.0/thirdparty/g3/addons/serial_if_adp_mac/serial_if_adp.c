/**
 *
 * \file
 *
 * \brief ADP Serialization file
 *
 * Copyright (c) 2023 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */

/* System includes */
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "string.h"

/* Serial interface */
#include "hal/hal.h"
#include "serial_if_adp.h"
#include "serial_if_common.h"
#include "serial_if_mib_common.h"
#include "AdpApi.h"
#include "AdpApiTypes.h"
#include "mac_wrapper.h"
#include "ProcessLbpCoord.h"
#include "ProcessLbpDev.h"
#include "usi.h"
#include <storage/storage.h>
#include "conf_project.h"

#define UNUSED(v)          (void)(v)

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

/* Known message ids */
enum ESerialMessageId {
	/* Generic messages */
	SERIAL_MSG_STATUS = 0,

	/* ADP ACCESS */
	SERIAL_MSG_ADP_INITIALIZE = 10,
	SERIAL_MSG_ADP_DATA_REQUEST,
	SERIAL_MSG_ADP_DISCOVERY_REQUEST,
	SERIAL_MSG_ADP_NETWORK_START_REQUEST,
	SERIAL_MSG_ADP_NETWORK_JOIN_REQUEST,
	SERIAL_MSG_ADP_NETWORK_LEAVE_REQUEST,
	SERIAL_MSG_ADP_RESET_REQUEST,
	SERIAL_MSG_ADP_SET_REQUEST,
	SERIAL_MSG_ADP_GET_REQUEST,
	SERIAL_MSG_ADP_LBP_REQUEST,
	SERIAL_MSG_ADP_ROUTE_DISCOVERY_REQUEST,
	SERIAL_MSG_ADP_PATH_DISCOVERY_REQUEST,
	SERIAL_MSG_ADP_MAC_SET_REQUEST,
	SERIAL_MSG_ADP_MAC_GET_REQUEST,
	SERIAL_MSG_ADP_NO_IP_DATA_REQUEST,

	SERIAL_MSG_ADP_DATA_CONFIRM = 30,
	SERIAL_MSG_ADP_DATA_INDICATION,
	SERIAL_MSG_ADP_NETWORK_STATUS_INDICATION,
	SERIAL_MSG_ADP_DISCOVERY_CONFIRM,
	SERIAL_MSG_ADP_NETWORK_START_CONFIRM,
	SERIAL_MSG_ADP_NETWORK_JOIN_CONFIRM,
	SERIAL_MSG_ADP_NETWORK_LEAVE_CONFIRM,
	SERIAL_MSG_ADP_NETWORK_LEAVE_INDICATION,
	SERIAL_MSG_ADP_RESET_CONFIRM,
	SERIAL_MSG_ADP_SET_CONFIRM,
	SERIAL_MSG_ADP_GET_CONFIRM,
	SERIAL_MSG_ADP_LBP_CONFIRM,
	SERIAL_MSG_ADP_LBP_INDICATION,
	SERIAL_MSG_ADP_ROUTE_DISCOVERY_CONFIRM,
	SERIAL_MSG_ADP_PATH_DISCOVERY_CONFIRM,
	SERIAL_MSG_ADP_MAC_SET_CONFIRM,
	SERIAL_MSG_ADP_MAC_GET_CONFIRM,
	SERIAL_MSG_ADP_BUFFER_INDICATION,
	SERIAL_MSG_ADP_DISCOVERY_INDICATION,
	SERIAL_MSG_ADP_PREQ_INDICATION,
	SERIAL_MSG_ADP_UPD_NON_VOLATILE_DATA_INDICATION,
	SERIAL_MSG_ADP_ROUTE_NOT_FOUND_INDICATION,

	SERIAL_MSG_LBP_SET_REQUEST = 60,
	SERIAL_MSG_LBP_DEV_FORCE_REGISTER,
	SERIAL_MSG_LBP_COORD_KICK_DEVICE,
	SERIAL_MSG_LBP_COORD_REKEY,
	SERIAL_MSG_LBP_COORD_SET_REKEY_PHASE,
	SERIAL_MSG_LBP_COORD_ACTIVATE_NEW_KEY,
	SERIAL_MSG_LBP_COORD_SHORT_ADDRESS_ASSIGN,

	SERIAL_MSG_LBP_SET_CONFIRM = 70,
	SERIAL_MSG_LBP_COORD_JOIN_REQUEST_INDICATION,
	SERIAL_MSG_LBP_COORD_JOIN_COMPLETE_INDICATION,
	SERIAL_MSG_LBP_COORD_LEAVE_INDICATION,
};

/* Status codes related to HostInterface processing */
enum ESerialStatus {
	SERIAL_STATUS_SUCCESS = 0,
	SERIAL_STATUS_NOT_ALLOWED,
	SERIAL_STATUS_UNKNOWN_COMMAND,
	SERIAL_STATUS_INVALID_PARAMETER
};

/* ! \name Data structure to communicate with USI layer */
/* @{ */
static x_usi_serial_cmd_params_t x_adp_serial_msg;
/* @} */

static uint8_t uc_serial_rsp_buf[2048]; /* !<  Response working buffer */
static uint8_t auc_aux_endiannes_buf[256]; /* !<  Set Request Endianness tranformation buffer */
static uint8_t auc_ext_address_adp[8]; /* EUI64 */

static struct TAdpNotifications ss_notifications;

static bool sb_arib_band;

static void mem_copy_to_usi_endianness_uint32(uint8_t *puc_dst, uint8_t *puc_src)
{
	uint32_t ul_aux;

	memcpy((uint8_t *)&ul_aux, puc_src, 4);

	*puc_dst++ = (uint8_t)((ul_aux >> 24) & 0xFF);
	*puc_dst++ = (uint8_t)((ul_aux >> 16) & 0xFF);
	*puc_dst++ = (uint8_t)((ul_aux >> 8) & 0xFF);
	*puc_dst = (uint8_t)(ul_aux & 0xFF);
}

static void mem_copy_to_usi_endianness_uint16(uint8_t *puc_dst, uint8_t *puc_src)
{
	uint16_t us_aux;

	memcpy((uint8_t *)&us_aux, puc_src, 2);

	*puc_dst++ = (uint8_t)(us_aux >> 8);
	*puc_dst = (uint8_t)(us_aux);
}

static void mem_copy_from_usi_endianness_uint32(uint8_t *puc_dst, uint8_t *puc_src)
{
	uint32_t ul_aux = 0;

	ul_aux = (*puc_src++) << 24;
	ul_aux += (*puc_src++) << 16;
	ul_aux += (*puc_src++) << 8;
	ul_aux += *puc_src;

	memcpy(puc_dst, (uint8_t *)&ul_aux, 4);
}

static void mem_copy_from_usi_endianness_uint16(uint8_t *puc_dst, uint8_t *puc_src)
{
	uint16_t us_aux = 0;

	us_aux += (*puc_src++) << 8;
	us_aux += *puc_src;

	memcpy(puc_dst, (uint8_t *)&us_aux, 2);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void MsgStatus(enum ESerialStatus status, uint8_t uc_serial_if_cmd)
{
	uint8_t us_serial_response_len;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_STATUS;
	uc_serial_rsp_buf[us_serial_response_len++] = status;
	uc_serial_rsp_buf[us_serial_response_len++] = uc_serial_if_cmd;
	/* set usi parameters */
	x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
	x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_adp_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_adp_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void AdpNotification_BufferIndication(TAdpBufferIndication *pBufferIndication)
{
	uint8_t us_serial_response_len;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_ADP_BUFFER_INDICATION;
	uc_serial_rsp_buf[us_serial_response_len++] = pBufferIndication->m_u8BufferIndicationBitmap;
	/* set usi parameters */
	x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
	x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_adp_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_adp_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void AppAdpNotification_PREQIndication(void)
{
	uint8_t us_serial_response_len;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_ADP_PREQ_INDICATION;
	/* set usi parameters */
	x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
	x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_adp_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_adp_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void AppAdpNotification_UpdNonVolatileDataIndication(struct TAdpNonVolatileData *pNonVolatileData)
{
#ifdef ENABLE_PIB_RESTORE
	store_persistent_data_GPBR(pNonVolatileData);
#else
	uint8_t us_serial_response_len;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_ADP_UPD_NON_VOLATILE_DATA_INDICATION;
	/* set usi parameters */
	x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
	x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_adp_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_adp_serial_msg);
#endif
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void AppAdpNotification_RouteNotFoundIndication(struct TAdpRouteNotFoundIndication *pRouteNotFoundIndication)
{
	uint8_t us_serial_response_len;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_ADP_ROUTE_NOT_FOUND_INDICATION;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pRouteNotFoundIndication->m_u16SrcAddr >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pRouteNotFoundIndication->m_u16SrcAddr & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pRouteNotFoundIndication->m_u16DestAddr >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pRouteNotFoundIndication->m_u16DestAddr & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pRouteNotFoundIndication->m_u16NextHopAddr >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pRouteNotFoundIndication->m_u16NextHopAddr & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pRouteNotFoundIndication->m_u16PreviousHopAddr >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pRouteNotFoundIndication->m_u16PreviousHopAddr & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pRouteNotFoundIndication->m_u16RouteCost >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pRouteNotFoundIndication->m_u16RouteCost & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pRouteNotFoundIndication->m_u8HopCount);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pRouteNotFoundIndication->m_u8WeakLinkCount);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pRouteNotFoundIndication->m_bRouteJustBroken);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pRouteNotFoundIndication->m_bCompressedHeader);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pRouteNotFoundIndication->m_u16NsduLength >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pRouteNotFoundIndication->m_u16NsduLength & 0xFF);
	memcpy(&uc_serial_rsp_buf[us_serial_response_len], &pRouteNotFoundIndication->m_pNsdu, pRouteNotFoundIndication->m_u16NsduLength);
	us_serial_response_len += pRouteNotFoundIndication->m_u16NsduLength;

	/* set usi parameters */
	x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
	x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_adp_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_adp_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void AdpNotification_DataConfirm(struct TAdpDataConfirm *pDataConfirm)
{
	uint8_t us_serial_response_len;

	/* Manage Result */
	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_ADP_DATA_CONFIRM;
	uc_serial_rsp_buf[us_serial_response_len++] = pDataConfirm->m_u8Status;
	uc_serial_rsp_buf[us_serial_response_len++] = pDataConfirm->m_u8NsduHandle;
	/* set usi parameters */
	x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
	x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_adp_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_adp_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void AdpNotification_AdpdDataIndication(struct TAdpDataIndication *pDataIndication)
{
	uint16_t us_serial_response_len;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_ADP_DATA_INDICATION;
	uc_serial_rsp_buf[us_serial_response_len++] = pDataIndication->m_u8LinkQualityIndicator;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pDataIndication->m_u16NsduLength >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pDataIndication->m_u16NsduLength & 0xFF);
	memcpy(&uc_serial_rsp_buf[us_serial_response_len], pDataIndication->m_pNsdu, pDataIndication->m_u16NsduLength);
	us_serial_response_len += pDataIndication->m_u16NsduLength;

	/* set usi parameters */
	x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
	x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_adp_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_adp_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void AdpNotification_NetworkStatusIndication(struct TAdpNetworkStatusIndication *pNetworkStatusIndication)
{
	uint8_t us_serial_response_len;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_ADP_NETWORK_STATUS_INDICATION;

	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pNetworkStatusIndication->m_u16PanId >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pNetworkStatusIndication->m_u16PanId & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = pNetworkStatusIndication->m_SrcDeviceAddress.m_u8AddrSize;
	if (pNetworkStatusIndication->m_SrcDeviceAddress.m_u8AddrSize == ADP_ADDRESS_16BITS) {
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pNetworkStatusIndication->m_SrcDeviceAddress.m_u16ShortAddr >> 8);
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pNetworkStatusIndication->m_SrcDeviceAddress.m_u16ShortAddr & 0xFF);
	} else { /* ADP_ADDRESS_64BITS */
		memcpy(&uc_serial_rsp_buf[us_serial_response_len], (uint8_t *)&pNetworkStatusIndication->m_SrcDeviceAddress.m_ExtendedAddress,
				pNetworkStatusIndication->m_SrcDeviceAddress.m_u8AddrSize);
		us_serial_response_len += pNetworkStatusIndication->m_SrcDeviceAddress.m_u8AddrSize;
	}

	uc_serial_rsp_buf[us_serial_response_len++] = pNetworkStatusIndication->m_DstDeviceAddress.m_u8AddrSize;
	if (pNetworkStatusIndication->m_DstDeviceAddress.m_u8AddrSize == ADP_ADDRESS_16BITS) {
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pNetworkStatusIndication->m_DstDeviceAddress.m_u16ShortAddr >> 8);
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pNetworkStatusIndication->m_DstDeviceAddress.m_u16ShortAddr & 0xFF);
	} else { /* ADP_ADDRESS_64BITS */
		memcpy(&uc_serial_rsp_buf[us_serial_response_len], (uint8_t *)&pNetworkStatusIndication->m_DstDeviceAddress.m_ExtendedAddress,
				pNetworkStatusIndication->m_DstDeviceAddress.m_u8AddrSize);
		us_serial_response_len += pNetworkStatusIndication->m_DstDeviceAddress.m_u8AddrSize;
	}

	uc_serial_rsp_buf[us_serial_response_len++] = pNetworkStatusIndication->m_u8Status;
	uc_serial_rsp_buf[us_serial_response_len++] = pNetworkStatusIndication->m_u8SecurityLevel;
	uc_serial_rsp_buf[us_serial_response_len++] = pNetworkStatusIndication->m_u8KeyIndex;
	uc_serial_rsp_buf[us_serial_response_len++] = pNetworkStatusIndication->m_u8MediaType;

	/* set usi parameters */
	x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
	x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_adp_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_adp_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void AdpNotification_DiscoveryConfirm(uint8_t u8Status)
{
	uint8_t us_serial_response_len;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_ADP_DISCOVERY_CONFIRM;
	uc_serial_rsp_buf[us_serial_response_len++] = u8Status;
	/* set usi parameters */
	x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
	x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_adp_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_adp_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void AdpNotification_DiscoveryIndication(struct TAdpPanDescriptor *pPanDescriptor)
{
	uint8_t us_serial_response_len;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_ADP_DISCOVERY_INDICATION;

	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pPanDescriptor->m_u16PanId >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pPanDescriptor->m_u16PanId & 0xFF);

	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pPanDescriptor->m_u8LinkQuality);

	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pPanDescriptor->m_u16LbaAddress >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pPanDescriptor->m_u16LbaAddress & 0xFF);

	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pPanDescriptor->m_u16RcCoord >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pPanDescriptor->m_u16RcCoord & 0xFF);

	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pPanDescriptor->m_u8MediaType);

	/* set usi parameters */
	x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
	x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_adp_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_adp_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void AdpNotification_NetworkStartConfirm(struct TAdpNetworkStartConfirm *pNetworkStartConfirm)
{
	uint8_t us_serial_response_len;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_ADP_NETWORK_START_CONFIRM;
	uc_serial_rsp_buf[us_serial_response_len++] = pNetworkStartConfirm->m_u8Status;

	/* set usi parameters */
	x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
	x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_adp_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_adp_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void AdpNotification_NetworkJoinConfirm(struct TAdpNetworkJoinConfirm *pNetworkJoinConfirm)
{
	uint8_t us_serial_response_len;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_ADP_NETWORK_JOIN_CONFIRM;
	uc_serial_rsp_buf[us_serial_response_len++] = pNetworkJoinConfirm->m_u8Status;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pNetworkJoinConfirm->m_u16NetworkAddress >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pNetworkJoinConfirm->m_u16NetworkAddress & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pNetworkJoinConfirm->m_u16PanId >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pNetworkJoinConfirm->m_u16PanId & 0xFF);

	/* set usi parameters */
	x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
	x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_adp_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_adp_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void AdpNotification_NetworkLeaveIndication(void)
{
	uint8_t us_serial_response_len;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_ADP_NETWORK_LEAVE_INDICATION;

	/* set usi parameters */
	x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
	x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_adp_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_adp_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void AdpNotification_NetworkLeaveConfirm(struct TAdpNetworkLeaveConfirm *pLeaveConfirm)
{
	uint8_t us_serial_response_len;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_ADP_NETWORK_LEAVE_CONFIRM;
	uc_serial_rsp_buf[us_serial_response_len++] = pLeaveConfirm->m_u8Status;

	/* set usi parameters */
	x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
	x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_adp_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_adp_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void AdpNotification_ResetConfirm(struct TAdpResetConfirm *pResetConfirm)
{
	uint8_t us_serial_response_len;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_ADP_RESET_CONFIRM;
	uc_serial_rsp_buf[us_serial_response_len++] = pResetConfirm->m_u8Status;

	/* set usi parameters */
	x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
	x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_adp_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_adp_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void AdpNotification_SetConfirm(struct TAdpSetConfirm *pSetConfirm)
{
	uint8_t us_serial_response_len;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_ADP_SET_CONFIRM;
	uc_serial_rsp_buf[us_serial_response_len++] = pSetConfirm->m_u8Status;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)((pSetConfirm->m_u32AttributeId >> 24) & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)((pSetConfirm->m_u32AttributeId >> 16) & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)((pSetConfirm->m_u32AttributeId >> 8) & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pSetConfirm->m_u32AttributeId & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pSetConfirm->m_u16AttributeIndex >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pSetConfirm->m_u16AttributeIndex & 0xFF);
	/* set usi parameters */
	x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
	x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_adp_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_adp_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void AdpNotification_GetConfirm(struct TAdpGetConfirm *pGetConfirm)
{
	uint8_t us_serial_response_len;

	uint8_t u8PrefixLength_bytes;
	uint8_t u8ContextLength;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_ADP_GET_CONFIRM;
	uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_u8Status;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)((pGetConfirm->m_u32AttributeId >> 24) & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)((pGetConfirm->m_u32AttributeId >> 16) & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)((pGetConfirm->m_u32AttributeId >> 8) & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pGetConfirm->m_u32AttributeId & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pGetConfirm->m_u16AttributeIndex >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pGetConfirm->m_u16AttributeIndex & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_u8AttributeLength;

	if (pGetConfirm->m_u8Status == G3_SUCCESS) {
		switch (pGetConfirm->m_u32AttributeId) {
		/* 8-bit IBs */
		case ADP_IB_SECURITY_LEVEL:
		case ADP_IB_METRIC_TYPE:
		case ADP_IB_LOW_LQI_VALUE:
		case ADP_IB_HIGH_LQI_VALUE:
		case ADP_IB_RREP_WAIT:
		case ADP_IB_RLC_ENABLED:
		case ADP_IB_ADD_REV_LINK_COST:
		case ADP_IB_UNICAST_RREQ_GEN_ENABLE:
		case ADP_IB_MAX_HOPS:
		case ADP_IB_DEVICE_TYPE:
		case ADP_IB_NET_TRAVERSAL_TIME:
		case ADP_IB_KR:
		case ADP_IB_KM:
		case ADP_IB_KC:
		case ADP_IB_KQ:
		case ADP_IB_KH:
		case ADP_IB_RREQ_RETRIES:
		case ADP_IB_RREQ_WAIT:
		case ADP_IB_WEAK_LQI_VALUE:
		case ADP_IB_KRT:
		case ADP_IB_PATH_DISCOVERY_TIME:
		case ADP_IB_ACTIVE_KEY_INDEX:
		case ADP_IB_DEFAULT_COORD_ROUTE_ENABLED:
		case ADP_IB_DISABLE_DEFAULT_ROUTING:
		case ADP_IB_RREQ_JITTER_LOW_LQI:
		case ADP_IB_RREQ_JITTER_HIGH_LQI:
		case ADP_IB_RREQ_JITTER_LOW_LQI_RF:
		case ADP_IB_RREQ_JITTER_HIGH_LQI_RF:
		case ADP_IB_TRICKLE_DATA_ENABLED:
		case ADP_IB_TRICKLE_LQI_THRESHOLD_LOW:
		case ADP_IB_TRICKLE_LQI_THRESHOLD_LOW_RF:
		case ADP_IB_TRICKLE_LQI_THRESHOLD_HIGH:
		case ADP_IB_TRICKLE_LQI_THRESHOLD_HIGH_RF:
		case ADP_IB_TRICKLE_STEP:
		case ADP_IB_TRICKLE_MAX_KI:
		case ADP_IB_TRICKLE_ADAPTIVE_I_MIN:
		case ADP_IB_TRICKLE_ADAPTIVE_KI:
		case ADP_IB_CLUSTER_TRICKLE_ENABLED:
		case ADP_IB_CLUSTER_MIN_LQI:
		case ADP_IB_CLUSTER_MIN_LQI_RF:
		case ADP_IB_CLUSTER_TRICKLE_K:
		case ADP_IB_CLUSTER_TRICKLE_K_RF:
		case ADP_IB_CLUSTER_RREQ_ROUTE_COST_DEVIATION:
		case ADP_IB_LAST_GASP:
		case ADP_IB_PROBING_INTERVAL:
		case ADP_IB_MANUF_IPV6_HEADER_COMPRESSION:
		case ADP_IB_MANUF_BROADCAST_SEQUENCE_NUMBER:
		case ADP_IB_MANUF_FORCED_NO_ACK_REQUEST:
		case ADP_IB_MANUF_LQI_TO_COORD:
		case ADP_IB_MANUF_BROADCAST_ROUTE_ALL:
		case ADP_IB_MANUF_MAX_REPAIR_RESEND_ATTEMPTS:
		case ADP_IB_MANUF_DISABLE_AUTO_RREQ:
		case ADP_IB_MANUF_GET_BAND_CONTEXT_TONES:
		case ADP_IB_MANUF_UPDATE_NON_VOLATILE_DATA:
		case ADP_IB_MANUF_DYNAMIC_FRAGMENT_DELAY_ENABLED:
		case ADP_IB_MANUF_HYBRID_PROFILE:
		case ADP_IB_MANUF_LAST_PHASEDIFF:
		case ADP_IB_LOW_LQI_VALUE_RF:
		case ADP_IB_HIGH_LQI_VALUE_RF:
		case ADP_IB_KQ_RF:
		case ADP_IB_KH_RF:
		case ADP_IB_KRT_RF:
		case ADP_IB_KDC_RF:
		case ADP_IB_WEAK_LQI_VALUE_RF:
		case ADP_IB_USE_BACKUP_MEDIA:
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[0];
			break;

		/* 16-bit IBs */
		case ADP_IB_BROADCAST_LOG_TABLE_ENTRY_TTL:
		case ADP_IB_COORD_SHORT_ADDRESS:
		case ADP_IB_ROUTING_TABLE_ENTRY_TTL:
		case ADP_IB_BLACKLIST_TABLE_ENTRY_TTL:
		case ADP_IB_MAX_JOIN_WAIT_TIME:
		case ADP_IB_DELAY_LOW_LQI:
		case ADP_IB_DELAY_HIGH_LQI:
		case ADP_IB_DELAY_LOW_LQI_RF:
		case ADP_IB_DELAY_HIGH_LQI_RF:
		case ADP_IB_DESTINATION_ADDRESS_SET:
		case ADP_IB_TRICKLE_I_MIN:
		case ADP_IB_CLUSTER_TRICKLE_I:
		case ADP_IB_CLUSTER_TRICKLE_I_RF:
		case ADP_IB_MANUF_REASSEMBY_TIMER:
		case ADP_IB_MANUF_DATAGRAM_TAG:
		case ADP_IB_MANUF_DISCOVER_SEQUENCE_NUMBER: /* ADP_IB_MANUF_DISCOVER_ROUTE_GLOBAL_SEQ_NUM */
		case ADP_IB_MANUF_CIRCULAR_ROUTES_DETECTED:
		case ADP_IB_MANUF_LAST_CIRCULAR_ROUTE_ADDRESS:
		case ADP_IB_MANUF_IPV6_ULA_DEST_SHORT_ADDRESS:
		case ADP_IB_MANUF_ALL_NEIGHBORS_BLACKLISTED_COUNT:
		case ADP_IB_MANUF_QUEUED_ENTRIES_REMOVED_TIMEOUT_COUNT:
		case ADP_IB_MANUF_QUEUED_ENTRIES_REMOVED_ROUTE_ERROR_COUNT:
		case ADP_IB_MANUF_PENDING_DATA_IND_SHORT_ADDRESS:
		case ADP_IB_MANUF_FRAGMENT_DELAY:
		case ADP_IB_MANUF_DYNAMIC_FRAGMENT_DELAY_FACTOR:
		case ADP_IB_MANUF_BLACKLIST_TABLE_COUNT:
		case ADP_IB_MANUF_BROADCAST_LOG_TABLE_COUNT:
		case ADP_IB_MANUF_CONTEXT_INFORMATION_TABLE_COUNT:
		case ADP_IB_MANUF_GROUP_TABLE_COUNT:
		case ADP_IB_MANUF_PAN_ID:
		case ADP_IB_MANUF_SHORT_ADDRESS:
			mem_copy_to_usi_endianness_uint16((uint8_t *)&uc_serial_rsp_buf[us_serial_response_len],
					(uint8_t *)&pGetConfirm->m_au8AttributeValue[0]);
			us_serial_response_len += 2;
			break;

		/* 32-bit IBs */
		case ADP_IB_MANUF_ROUTING_TABLE_COUNT:
			mem_copy_to_usi_endianness_uint32((uint8_t *)&uc_serial_rsp_buf[us_serial_response_len],
					(uint8_t *)&pGetConfirm->m_au8AttributeValue[0]);
			us_serial_response_len += 4;
			break;

		/* Tables and lists */
		case ADP_IB_PREFIX_TABLE:
			u8PrefixLength_bytes = pGetConfirm->m_u8AttributeLength - 11;
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[0]; /* m_u8PrefixLength */
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[1]; /* m_bOnLinkFlag */
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[2]; /* m_bAutonomousAddressConfigurationFlag */
			mem_copy_to_usi_endianness_uint32((uint8_t *)&uc_serial_rsp_buf[us_serial_response_len],
					(uint8_t *)&pGetConfirm->m_au8AttributeValue[3]);  /* u32ValidTime */
			us_serial_response_len += 4;
			mem_copy_to_usi_endianness_uint32((uint8_t *)&uc_serial_rsp_buf[us_serial_response_len],
					(uint8_t *)&pGetConfirm->m_au8AttributeValue[7]);  /* u32PreferredTime */
			us_serial_response_len += 4;
			memcpy((uint8_t *)&uc_serial_rsp_buf[us_serial_response_len], (uint8_t *)&pGetConfirm->m_au8AttributeValue[11], u8PrefixLength_bytes);  /* m_au8Prefix */
			us_serial_response_len += u8PrefixLength_bytes;
			break;

		case ADP_IB_CONTEXT_INFORMATION_TABLE:
			u8ContextLength = pGetConfirm->m_u8AttributeLength - 4;
			mem_copy_to_usi_endianness_uint16((uint8_t *)&uc_serial_rsp_buf[us_serial_response_len],
					(uint8_t *)&pGetConfirm->m_au8AttributeValue[0]);  /* u16ValidTime */
			us_serial_response_len += 2;
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[2]; /* m_bValidForCompression */
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[3]; /* m_u8BitsContextLength */
			memcpy((uint8_t *)&uc_serial_rsp_buf[us_serial_response_len], (uint8_t *)&pGetConfirm->m_au8AttributeValue[4], u8ContextLength);  /* m_au8Context */
			us_serial_response_len += u8ContextLength;
			break;

		case ADP_IB_BROADCAST_LOG_TABLE:
			mem_copy_to_usi_endianness_uint16((uint8_t *)&uc_serial_rsp_buf[us_serial_response_len],
					(uint8_t *)&pGetConfirm->m_au8AttributeValue[0]);  /* m_u16SrcAddr */
			us_serial_response_len += 2;
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[2]; /* m_u8SequenceNumber */
			mem_copy_to_usi_endianness_uint16((uint8_t *)&uc_serial_rsp_buf[us_serial_response_len],
					(uint8_t *)&pGetConfirm->m_au8AttributeValue[3]);  /* u16ValidTime */
			us_serial_response_len += 2;
			break;

		case ADP_IB_ROUTING_TABLE:
			mem_copy_to_usi_endianness_uint16((uint8_t *)&uc_serial_rsp_buf[us_serial_response_len],
					(uint8_t *)&pGetConfirm->m_au8AttributeValue[0]);  /* m_u16DstAddr */
			us_serial_response_len += 2;
			mem_copy_to_usi_endianness_uint16((uint8_t *)&uc_serial_rsp_buf[us_serial_response_len],
					(uint8_t *)&pGetConfirm->m_au8AttributeValue[2]);  /* m_u16NextHopAddr */
			us_serial_response_len += 2;
			mem_copy_to_usi_endianness_uint16((uint8_t *)&uc_serial_rsp_buf[us_serial_response_len],
					(uint8_t *)&pGetConfirm->m_au8AttributeValue[4]);  /* m_u16RouteCost */
			us_serial_response_len += 2;
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[6]; /* m_u8HopCount || m_u8WeakLinkCount */
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[7]; /* m_u8MediaType */
			mem_copy_to_usi_endianness_uint16((uint8_t *)&uc_serial_rsp_buf[us_serial_response_len],
					(uint8_t *)&pGetConfirm->m_au8AttributeValue[8]);  /* u16ValidTime */
			us_serial_response_len += 2;
			break;

		case ADP_IB_GROUP_TABLE:
			mem_copy_to_usi_endianness_uint16((uint8_t *)&uc_serial_rsp_buf[us_serial_response_len],
					(uint8_t *)&pGetConfirm->m_au8AttributeValue[0]);  /* m_u16GroupAddress */
			us_serial_response_len += 2;
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[2]; /* m_bValid */
			break;

		case ADP_IB_SOFT_VERSION:
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[0]; /* m_u8Major */
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[1]; /* m_u8Minor */
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[2]; /* m_u8Revision */
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[3]; /* m_u8Year */
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[4]; /* m_u8Month */
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[5]; /* m_u8Day */
			break;

		case ADP_IB_BLACKLIST_TABLE:
			mem_copy_to_usi_endianness_uint16((uint8_t *)&uc_serial_rsp_buf[us_serial_response_len],
					(uint8_t *)&pGetConfirm->m_au8AttributeValue[0]);  /* m_u16Addr */
			us_serial_response_len += 2;
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[2]; /* m_u8MediaType */
			mem_copy_to_usi_endianness_uint16((uint8_t *)&uc_serial_rsp_buf[us_serial_response_len],
					(uint8_t *)&pGetConfirm->m_au8AttributeValue[3]);  /* u16ValidTime */
			us_serial_response_len += 2;
			break;

		case ADP_IB_MANUF_ADP_INTERNAL_VERSION:
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[0]; /* m_u8Major */
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[1]; /* m_u8Minor */
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[2]; /* m_u8Revision */
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[3]; /* m_u8Year */
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[4]; /* m_u8Month */
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[5]; /* m_u8Day */
			break;

		case ADP_IB_MANUF_ROUTING_TABLE_ELEMENT:
			mem_copy_to_usi_endianness_uint16((uint8_t *)&uc_serial_rsp_buf[us_serial_response_len],
					(uint8_t *)&pGetConfirm->m_au8AttributeValue[0]);  /* m_u16DstAddr */
			us_serial_response_len += 2;
			mem_copy_to_usi_endianness_uint16((uint8_t *)&uc_serial_rsp_buf[us_serial_response_len],
					(uint8_t *)&pGetConfirm->m_au8AttributeValue[2]);  /* m_u16NextHopAddr */
			us_serial_response_len += 2;
			mem_copy_to_usi_endianness_uint16((uint8_t *)&uc_serial_rsp_buf[us_serial_response_len],
					(uint8_t *)&pGetConfirm->m_au8AttributeValue[4]);  /* m_u16RouteCost */
			us_serial_response_len += 2;
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[6]; /* m_u8HopCount || m_u8WeakLinkCount */
			uc_serial_rsp_buf[us_serial_response_len++] = pGetConfirm->m_au8AttributeValue[7]; /* m_u8MediaType */
			mem_copy_to_usi_endianness_uint16((uint8_t *)&uc_serial_rsp_buf[us_serial_response_len],
					(uint8_t *)&pGetConfirm->m_au8AttributeValue[8]);  /* u16ValidTime */
			us_serial_response_len += 2;
			break;

		case ADP_IB_SNIFFER_MODE:
			/* TODO */
			break;

		default:
			break;
		}
	}

	/* set usi parameters */
	x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
	x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_adp_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_adp_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void AdpNotification_RouteDiscoveryConfirm(struct TAdpRouteDiscoveryConfirm *pRouteDiscoveryConfirm)
{
	uint8_t us_serial_response_len;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_ADP_ROUTE_DISCOVERY_CONFIRM;
	uc_serial_rsp_buf[us_serial_response_len++] = pRouteDiscoveryConfirm->m_u8Status;

	/* set usi parameters */
	x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
	x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_adp_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_adp_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void AdpNotification_PathDiscoveryConfirm(struct TAdpPathDiscoveryConfirm *pPathDiscoveryConfirm)
{
	uint8_t us_serial_response_len;
	uint8_t u8Index;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_ADP_PATH_DISCOVERY_CONFIRM;
	uc_serial_rsp_buf[us_serial_response_len++] = pPathDiscoveryConfirm->m_u8Status;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pPathDiscoveryConfirm->m_u16DstAddr >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pPathDiscoveryConfirm->m_u16DstAddr);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pPathDiscoveryConfirm->m_u16OrigAddr >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pPathDiscoveryConfirm->m_u16OrigAddr);
	uc_serial_rsp_buf[us_serial_response_len++] = pPathDiscoveryConfirm->m_u8MetricType;
	uc_serial_rsp_buf[us_serial_response_len++] = pPathDiscoveryConfirm->m_u8ForwardHopsCount;
	uc_serial_rsp_buf[us_serial_response_len++] = pPathDiscoveryConfirm->m_u8ReverseHopsCount;

	for (u8Index = 0; u8Index < pPathDiscoveryConfirm->m_u8ForwardHopsCount; u8Index++) {
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pPathDiscoveryConfirm->m_aForwardPath[u8Index].m_u16HopAddress >> 8);
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pPathDiscoveryConfirm->m_aForwardPath[u8Index].m_u16HopAddress);
		uc_serial_rsp_buf[us_serial_response_len++] = pPathDiscoveryConfirm->m_aForwardPath[u8Index].m_u8Mns;
		uc_serial_rsp_buf[us_serial_response_len++] = pPathDiscoveryConfirm->m_aForwardPath[u8Index].m_u8LinkCost;
		uc_serial_rsp_buf[us_serial_response_len++] = pPathDiscoveryConfirm->m_aForwardPath[u8Index].m_u8PhaseDiff;
		uc_serial_rsp_buf[us_serial_response_len++] = pPathDiscoveryConfirm->m_aForwardPath[u8Index].m_u8Mrx;
		uc_serial_rsp_buf[us_serial_response_len++] = pPathDiscoveryConfirm->m_aForwardPath[u8Index].m_u8Mtx;
	}

	for (u8Index = 0; u8Index < pPathDiscoveryConfirm->m_u8ReverseHopsCount; u8Index++) {
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pPathDiscoveryConfirm->m_aReversePath[u8Index].m_u16HopAddress >> 8);
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pPathDiscoveryConfirm->m_aReversePath[u8Index].m_u16HopAddress);
		uc_serial_rsp_buf[us_serial_response_len++] = pPathDiscoveryConfirm->m_aReversePath[u8Index].m_u8Mns;
		uc_serial_rsp_buf[us_serial_response_len++] = pPathDiscoveryConfirm->m_aReversePath[u8Index].m_u8LinkCost;
		uc_serial_rsp_buf[us_serial_response_len++] = pPathDiscoveryConfirm->m_aReversePath[u8Index].m_u8PhaseDiff;
		uc_serial_rsp_buf[us_serial_response_len++] = pPathDiscoveryConfirm->m_aReversePath[u8Index].m_u8Mrx;
		uc_serial_rsp_buf[us_serial_response_len++] = pPathDiscoveryConfirm->m_aReversePath[u8Index].m_u8Mtx;
	}

	/* set usi parameters */
	x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
	x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_adp_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_adp_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void LbpCoordNotification_JoinRequestIndication(uint8_t *pLbdAddress)
{
	uint8_t us_serial_response_len = 0;

	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_LBP_COORD_JOIN_REQUEST_INDICATION;
	memcpy(&uc_serial_rsp_buf[us_serial_response_len], pLbdAddress, 8);
	us_serial_response_len += 8;

	/* set usi parameters */
	x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
	x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_adp_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_adp_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void LbpCoordNotification_JoinCompleteIndication(uint8_t *pLbdAddress, uint16_t u16AssignedAddress)
{
	uint8_t us_serial_response_len = 0;

	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_LBP_COORD_JOIN_COMPLETE_INDICATION;
	memcpy(&uc_serial_rsp_buf[us_serial_response_len], pLbdAddress, 8);
	us_serial_response_len += 8;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(u16AssignedAddress >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(u16AssignedAddress);

	/* set usi parameters */
	x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
	x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_adp_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_adp_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void LbpCoordNotification_LeaveIndication(uint16_t u16NetworkAddress)
{
	uint8_t us_serial_response_len = 0;

	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_LBP_COORD_LEAVE_INDICATION;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(u16NetworkAddress >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(u16NetworkAddress);

	/* set usi parameters */
	x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
	x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_adp_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_adp_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerAdpInitialize(const uint8_t *puc_msg_content)
{
	struct TAdpMacSetConfirm macSetConfirm;
	enum ESerialStatus status = SERIAL_STATUS_SUCCESS;
	struct TLbpNotificationsCoord lbpCoordNotifications;
	struct TLbpNotificationsDev lbpDevNotifications;
	bool b_coord;

	uint8_t u8Band = puc_msg_content[0];
	sb_arib_band = u8Band == ADP_BAND_ARIB;
	b_coord = (puc_msg_content[1] != 0);

	AdpInitialize(serial_if_adp_get_notifications(), (enum TAdpBand)u8Band);

	platform_init_eui64(auc_ext_address_adp);
	AdpMacSetRequestSync(MAC_WRP_PIB_MANUF_EXTENDED_ADDRESS, 0, sizeof(auc_ext_address_adp), auc_ext_address_adp, &macSetConfirm);

	if (b_coord) {
		LBP_InitCoord(sb_arib_band);
		lbpCoordNotifications.fnctJoinRequestIndication = LbpCoordNotification_JoinRequestIndication;
		lbpCoordNotifications.fnctJoinCompleteIndication = LbpCoordNotification_JoinCompleteIndication;
		lbpCoordNotifications.fnctLeaveIndication = LbpCoordNotification_LeaveIndication;
		LBP_SetNotificationsCoord(&lbpCoordNotifications);
		adp_mac_serial_if_set_state(SERIAL_MODE_ADP_COORD);
	}
	else {
		LBP_InitDev();
		lbpDevNotifications.fnctJoinConfirm = AdpNotification_NetworkJoinConfirm;
		lbpDevNotifications.fnctLeaveIndication = AdpNotification_NetworkLeaveIndication;
		lbpDevNotifications.fnctLeaveConfirm = AdpNotification_NetworkLeaveConfirm;
		LBP_SetNotificationsDev(&lbpDevNotifications);
		adp_mac_serial_if_set_state(SERIAL_MODE_ADP_DEV);
	}

#ifdef ENABLE_PIB_RESTORE
	load_persistent_info();
#endif

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerAdpDataRequest(const uint8_t *puc_msg_content)
{
	uint8_t *puc_buffer = (uint8_t *)puc_msg_content;
	enum ESerialStatus status = SERIAL_STATUS_NOT_ALLOWED;
	uint8_t u8NsduHandle;
	bool bDiscoverRoute;
	uint8_t u8QualityOfService;
	uint16_t u16NsduLength;
	const uint8_t *pNsdu;

	if (adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_DEV || adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_COORD) {
		u8NsduHandle = (uint8_t)*puc_buffer++;
		bDiscoverRoute = (bool) * puc_buffer++;
		u8QualityOfService = (uint8_t)*puc_buffer++;
		u16NsduLength = ((uint16_t)*puc_buffer++) << 8;
		u16NsduLength += (uint16_t)*puc_buffer++;
		pNsdu = puc_buffer;

		AdpDataRequest(u16NsduLength, pNsdu, u8NsduHandle, bDiscoverRoute,
				u8QualityOfService);

		status = SERIAL_STATUS_SUCCESS;
	}

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerAdpNoIPDataRequest(const uint8_t *puc_msg_content)
{
	uint8_t *puc_buffer = (uint8_t *)puc_msg_content;
	enum ESerialStatus status = SERIAL_STATUS_NOT_ALLOWED;
	uint8_t u8NsduHandle;
	bool bDiscoverRoute;
	uint8_t u8QualityOfService;
	uint16_t u16DstAddr;
	uint16_t u16NsduLength;
	const uint8_t *pNsdu;

	if (adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_DEV || adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_COORD) {
		u16DstAddr = ((uint16_t)*puc_buffer++) << 8;
		u16DstAddr += (uint16_t)*puc_buffer++;
		u8NsduHandle = (uint8_t)*puc_buffer++;
		bDiscoverRoute = (bool) * puc_buffer++;
		u8QualityOfService = (uint8_t)*puc_buffer++;
		u16NsduLength = ((uint16_t)*puc_buffer++) << 8;
		u16NsduLength += (uint16_t)*puc_buffer++;
		pNsdu = puc_buffer;

		AdpNoIPDataRequest(u16NsduLength, pNsdu, u16DstAddr, u8NsduHandle, bDiscoverRoute,
				u8QualityOfService);

		status = SERIAL_STATUS_SUCCESS;
	}

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerAdpDiscoveryRequest(const uint8_t *puc_msg_content)
{
	enum ESerialStatus status = SERIAL_STATUS_NOT_ALLOWED;
	uint8_t u8Duration = puc_msg_content[0];
	if (adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_DEV || adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_COORD) {
		AdpDiscoveryRequest(u8Duration);
		status = SERIAL_STATUS_SUCCESS;
	}

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerAdpNetworkStartRequest(const uint8_t *puc_msg_content)
{
	enum ESerialStatus status = SERIAL_STATUS_NOT_ALLOWED;
	uint16_t u16PanId;
	uint8_t *puc_buffer = (uint8_t *)puc_msg_content;

	if (adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_DEV || adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_COORD) {
		u16PanId = ((uint16_t)*puc_buffer++) << 8;
		u16PanId += (uint16_t)*puc_buffer;

		AdpNetworkStartRequest(u16PanId);
		status = SERIAL_STATUS_SUCCESS;
	}

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerAdpNetworkJoinRequest(const uint8_t *puc_msg_content)
{
	enum ESerialStatus status = SERIAL_STATUS_NOT_ALLOWED;
	uint16_t u16PanId;
	uint16_t u16LbaAddress;
	uint8_t u8MediaType;

	uint8_t *puc_buffer = (uint8_t *)puc_msg_content;
	if (adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_DEV || adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_COORD) {
		u16PanId = ((uint16_t)*puc_buffer++) << 8;
		u16PanId += (uint16_t)*puc_buffer++;
		u16LbaAddress = ((uint16_t)*puc_buffer++) << 8;
		u16LbaAddress += (uint16_t)*puc_buffer++;
		u8MediaType = *puc_buffer++;
		AdpNetworkJoinRequest(u16PanId, u16LbaAddress, u8MediaType);
		status = SERIAL_STATUS_SUCCESS;
	}

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerAdpNetworkLeaveRequest(const uint8_t *puc_msg_content)
{
	enum ESerialStatus status = SERIAL_STATUS_NOT_ALLOWED;
	UNUSED(puc_msg_content);
	if (adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_DEV || adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_COORD) {
		/* NetworkLeave_Request takes no parameters */
		AdpNetworkLeaveRequest();
		status = SERIAL_STATUS_SUCCESS;
	}

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerAdpResetRequest(const uint8_t *puc_msg_content)
{
	enum ESerialStatus status = SERIAL_STATUS_NOT_ALLOWED;
	UNUSED(puc_msg_content);
	if (adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_DEV || adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_COORD) {
		/* Reset_Request takes no parameters */
		AdpResetRequest();
		status = SERIAL_STATUS_SUCCESS;
	}

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerAdpSetRequest(const uint8_t *puc_msg_content)
{
	enum ESerialStatus status = SERIAL_STATUS_NOT_ALLOWED;
	uint32_t u32AttributeId;
	uint16_t u16AttributeIndex;
	uint8_t *puc_buffer = (uint8_t *)puc_msg_content;
	uint8_t u8AttributeLengthCnt = 0;
	uint8_t u8AttributeLength = 0;
	uint8_t u8PrefixLength_bytes;

	if (adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_DEV || adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_COORD) {
		u32AttributeId = ((uint32_t)*puc_buffer++) << 24;
		u32AttributeId += ((uint32_t)*puc_buffer++) << 16;
		u32AttributeId += ((uint32_t)*puc_buffer++) << 8;
		u32AttributeId += (uint32_t)*puc_buffer++;

		u16AttributeIndex = ((uint16_t)*puc_buffer++) << 8;
		u16AttributeIndex += (uint16_t)*puc_buffer++;

		u8AttributeLength = *puc_buffer++;

		switch (u32AttributeId) {
		/* 8-bit IBs */
		case ADP_IB_SECURITY_LEVEL:
		case ADP_IB_METRIC_TYPE:
		case ADP_IB_LOW_LQI_VALUE:
		case ADP_IB_HIGH_LQI_VALUE:
		case ADP_IB_RREP_WAIT:
		case ADP_IB_RLC_ENABLED:
		case ADP_IB_ADD_REV_LINK_COST:
		case ADP_IB_UNICAST_RREQ_GEN_ENABLE:
		case ADP_IB_MAX_HOPS:
		case ADP_IB_DEVICE_TYPE:
		case ADP_IB_NET_TRAVERSAL_TIME:
		case ADP_IB_KR:
		case ADP_IB_KM:
		case ADP_IB_KC:
		case ADP_IB_KQ:
		case ADP_IB_KH:
		case ADP_IB_RREQ_RETRIES:
		case ADP_IB_RREQ_WAIT:
		case ADP_IB_WEAK_LQI_VALUE:
		case ADP_IB_KRT:
		case ADP_IB_PATH_DISCOVERY_TIME:
		case ADP_IB_ACTIVE_KEY_INDEX:
		case ADP_IB_DEFAULT_COORD_ROUTE_ENABLED:
		case ADP_IB_DISABLE_DEFAULT_ROUTING:
		case ADP_IB_RREQ_JITTER_LOW_LQI:
		case ADP_IB_RREQ_JITTER_HIGH_LQI:
		case ADP_IB_RREQ_JITTER_LOW_LQI_RF:
		case ADP_IB_RREQ_JITTER_HIGH_LQI_RF:
		case ADP_IB_TRICKLE_DATA_ENABLED:
		case ADP_IB_TRICKLE_LQI_THRESHOLD_LOW:
		case ADP_IB_TRICKLE_LQI_THRESHOLD_LOW_RF:
		case ADP_IB_TRICKLE_LQI_THRESHOLD_HIGH:
		case ADP_IB_TRICKLE_LQI_THRESHOLD_HIGH_RF:
		case ADP_IB_TRICKLE_STEP:
		case ADP_IB_TRICKLE_MAX_KI:
		case ADP_IB_TRICKLE_ADAPTIVE_I_MIN:
		case ADP_IB_TRICKLE_ADAPTIVE_KI:
		case ADP_IB_CLUSTER_TRICKLE_ENABLED:
		case ADP_IB_CLUSTER_MIN_LQI:
		case ADP_IB_CLUSTER_MIN_LQI_RF:
		case ADP_IB_CLUSTER_TRICKLE_K:
		case ADP_IB_CLUSTER_TRICKLE_K_RF:
		case ADP_IB_CLUSTER_RREQ_ROUTE_COST_DEVIATION:
		case ADP_IB_LAST_GASP:
		case ADP_IB_PROBING_INTERVAL:
		case ADP_IB_MANUF_IPV6_HEADER_COMPRESSION:
		case ADP_IB_MANUF_BROADCAST_SEQUENCE_NUMBER:
		case ADP_IB_MANUF_FORCED_NO_ACK_REQUEST:
		case ADP_IB_MANUF_LQI_TO_COORD:
		case ADP_IB_MANUF_BROADCAST_ROUTE_ALL:
		case ADP_IB_MANUF_MAX_REPAIR_RESEND_ATTEMPTS:
		case ADP_IB_MANUF_DISABLE_AUTO_RREQ:
		case ADP_IB_MANUF_GET_BAND_CONTEXT_TONES:
		case ADP_IB_MANUF_UPDATE_NON_VOLATILE_DATA:
		case ADP_IB_MANUF_DYNAMIC_FRAGMENT_DELAY_ENABLED:
		case ADP_IB_MANUF_HYBRID_PROFILE:
		case ADP_IB_MANUF_LAST_PHASEDIFF:
		case ADP_IB_LOW_LQI_VALUE_RF:
		case ADP_IB_HIGH_LQI_VALUE_RF:
		case ADP_IB_KQ_RF:
		case ADP_IB_KH_RF:
		case ADP_IB_KRT_RF:
		case ADP_IB_KDC_RF:
		case ADP_IB_USE_BACKUP_MEDIA:
		case ADP_IB_WEAK_LQI_VALUE_RF:
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++;
			break;

		/* 16-bit IBs */
		case ADP_IB_BROADCAST_LOG_TABLE_ENTRY_TTL:
		case ADP_IB_COORD_SHORT_ADDRESS:
		case ADP_IB_ROUTING_TABLE_ENTRY_TTL:
		case ADP_IB_BLACKLIST_TABLE_ENTRY_TTL:
		case ADP_IB_MAX_JOIN_WAIT_TIME:
		case ADP_IB_DELAY_LOW_LQI:
		case ADP_IB_DELAY_HIGH_LQI:
		case ADP_IB_DELAY_LOW_LQI_RF:
		case ADP_IB_DELAY_HIGH_LQI_RF:
		case ADP_IB_DESTINATION_ADDRESS_SET:
		case ADP_IB_TRICKLE_I_MIN:
		case ADP_IB_CLUSTER_TRICKLE_I:
		case ADP_IB_CLUSTER_TRICKLE_I_RF:
		case ADP_IB_MANUF_REASSEMBY_TIMER:
		case ADP_IB_MANUF_DATAGRAM_TAG:
		case ADP_IB_MANUF_DISCOVER_SEQUENCE_NUMBER: /* ADP_IB_MANUF_DISCOVER_ROUTE_GLOBAL_SEQ_NUM */
		case ADP_IB_MANUF_CIRCULAR_ROUTES_DETECTED:
		case ADP_IB_MANUF_LAST_CIRCULAR_ROUTE_ADDRESS:
		case ADP_IB_MANUF_IPV6_ULA_DEST_SHORT_ADDRESS:
		case ADP_IB_MANUF_ALL_NEIGHBORS_BLACKLISTED_COUNT:
		case ADP_IB_MANUF_QUEUED_ENTRIES_REMOVED_TIMEOUT_COUNT:
		case ADP_IB_MANUF_QUEUED_ENTRIES_REMOVED_ROUTE_ERROR_COUNT:
		case ADP_IB_MANUF_PENDING_DATA_IND_SHORT_ADDRESS:
		case ADP_IB_MANUF_FRAGMENT_DELAY:
		case ADP_IB_MANUF_DYNAMIC_FRAGMENT_DELAY_FACTOR:
		case ADP_IB_MANUF_BLACKLIST_TABLE_COUNT:
		case ADP_IB_MANUF_BROADCAST_LOG_TABLE_COUNT:
		case ADP_IB_MANUF_CONTEXT_INFORMATION_TABLE_COUNT:
		case ADP_IB_MANUF_GROUP_TABLE_COUNT:
		case ADP_IB_MANUF_PAN_ID:
		case ADP_IB_MANUF_SHORT_ADDRESS:
			mem_copy_from_usi_endianness_uint16((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_buffer);
			u8AttributeLengthCnt += 2;
			puc_buffer += 2;
			break;

		/* 32-bit IBs */
		case ADP_IB_MANUF_ROUTING_TABLE_COUNT:
			mem_copy_from_usi_endianness_uint32((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_buffer);
			u8AttributeLengthCnt += 4;
			puc_buffer += 4;
			break;

		/* Tables and lists */
		case ADP_IB_PREFIX_TABLE:
			if (u8AttributeLength) { /* len = 0 => Delete Entry */
				u8PrefixLength_bytes = u8AttributeLength - 11;
				auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++; /* m_u8PrefixLength */
				auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++; /* m_bOnLinkFlag */
				auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++; /* m_bAutonomousAddressConfigurationFlag */
				mem_copy_from_usi_endianness_uint32((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_buffer);  /* u32ValidTime */
				u8AttributeLengthCnt += 4;
				puc_buffer += 4;
				mem_copy_from_usi_endianness_uint32((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_buffer);  /* u32PreferredTime */
				u8AttributeLengthCnt += 4;
				puc_buffer += 4;
				memcpy((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_buffer, u8PrefixLength_bytes); /* m_au8Prefix */
				u8AttributeLengthCnt += u8PrefixLength_bytes;
				puc_buffer += u8PrefixLength_bytes;
			}
			break;

		case ADP_IB_CONTEXT_INFORMATION_TABLE:
			if (u8AttributeLength) { /* len = 0 => Delete Entry */
				uint8_t u8ContextLength  = u8AttributeLength - 4;
				mem_copy_from_usi_endianness_uint16((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_buffer);  /* u16ValidTime */
				u8AttributeLengthCnt += 2;
				puc_buffer += 2;
				auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++; /* m_bValidForCompression */
				auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++; /* m_u8BitsContextLength */
				memcpy((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_buffer, u8ContextLength); /* m_au8Context */
				u8AttributeLengthCnt += u8ContextLength;
				puc_buffer += u8ContextLength;
			}
			break;

		case ADP_IB_BROADCAST_LOG_TABLE:
			mem_copy_from_usi_endianness_uint16((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_buffer);  /* m_u16SrcAddr */
			u8AttributeLengthCnt += 2;
			puc_buffer += 2;
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++; /* m_u8SequenceNumber */
			mem_copy_from_usi_endianness_uint16((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_buffer);  /* u16ValidTime */
			u8AttributeLengthCnt += 2;
			puc_buffer += 2;
			break;

		case ADP_IB_ROUTING_TABLE:
			if (u8AttributeLength) { /* len = 0 => Delete Entry */
				mem_copy_from_usi_endianness_uint16((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_buffer);  /* m_u16DstAddr */
				u8AttributeLengthCnt += 2;
				puc_buffer += 2;
				mem_copy_from_usi_endianness_uint16((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_buffer);  /* m_u16NextHopAddr */
				u8AttributeLengthCnt += 2;
				puc_buffer += 2;
				mem_copy_from_usi_endianness_uint16((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_buffer);  /* m_u16RouteCost */
				u8AttributeLengthCnt += 2;
				puc_buffer += 2;
				auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++; /* m_u8HopCount || m_u8WeakLinkCount */
				auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++; /* m_u8MediaType */
				mem_copy_from_usi_endianness_uint16((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_buffer);  /* m_i32ValidTime */
				u8AttributeLengthCnt += 2;
				puc_buffer += 2;
			}
			break;

		case ADP_IB_GROUP_TABLE:
			mem_copy_from_usi_endianness_uint16((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_buffer);  /* m_u16GroupAddress */
			u8AttributeLengthCnt += 2;
			puc_buffer += 2;
			/* auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++ ;//m_bValid */
			break;

		case ADP_IB_SOFT_VERSION:
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++; /* m_u8Major */
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++; /* m_u8Minor */
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++; /* m_u8Revision */
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++; /* m_u8Year */
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++; /* m_u8Month */
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++; /* m_u8Day */
			break;

		case ADP_IB_BLACKLIST_TABLE:
			mem_copy_from_usi_endianness_uint16((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_buffer);  /* m_u16Addr */
			u8AttributeLengthCnt += 2;
			puc_buffer += 2;
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++; /* m_u8MediaType */
			mem_copy_from_usi_endianness_uint16((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_buffer);  /* u16ValidTime */
			u8AttributeLengthCnt += 2;
			puc_buffer += 2;
			break;

		case ADP_IB_MANUF_ADP_INTERNAL_VERSION:
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++; /* m_u8Major */
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++; /* m_u8Minor */
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++; /* m_u8Revision */
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++; /* m_u8Year */
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++; /* m_u8Month */
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++; /* m_u8Day */
			break;

		case ADP_IB_MANUF_ROUTING_TABLE_ELEMENT:
			if (u8AttributeLength) { /* len = 0 => Delete Entry */
				mem_copy_from_usi_endianness_uint16((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_buffer);  /* m_u16DstAddr */
				u8AttributeLengthCnt += 2;
				puc_buffer += 2;
				mem_copy_from_usi_endianness_uint16((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_buffer);  /* m_u16NextHopAddr */
				u8AttributeLengthCnt += 2;
				puc_buffer += 2;
				mem_copy_from_usi_endianness_uint16((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_buffer);  /* m_u16RouteCost */
				u8AttributeLengthCnt += 2;
				puc_buffer += 2;
				auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++; /* m_u8HopCount || m_u8WeakLinkCount */
				auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_buffer++; /* m_u8MediaType */
				mem_copy_from_usi_endianness_uint16((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_buffer);  /* m_i32ValidTime */
				u8AttributeLengthCnt += 2;
				puc_buffer += 2;
			}
			break;

		case ADP_IB_SNIFFER_MODE:
			/* TODO */
			break;

		default:
			break;
		}
		if (u8AttributeLength == u8AttributeLengthCnt) {
			AdpSetRequest(u32AttributeId, u16AttributeIndex, u8AttributeLengthCnt, &auc_aux_endiannes_buf[0]);
			status = SERIAL_STATUS_SUCCESS;
		} else {
			status = SERIAL_STATUS_INVALID_PARAMETER;
		}
	}

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerAdpGetRequest(const uint8_t *puc_msg_content)
{
	enum ESerialStatus status = SERIAL_STATUS_NOT_ALLOWED;
	uint32_t u32AttributeId;
	uint16_t u16AttributeIndex;
	uint8_t *puc_buffer = (uint8_t *)puc_msg_content;

	if (adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_DEV || adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_COORD) {
		u32AttributeId = ((uint32_t)*puc_buffer++) << 24;
		u32AttributeId += ((uint32_t)*puc_buffer++) << 16;
		u32AttributeId += ((uint32_t)*puc_buffer++) << 8;
		u32AttributeId += (uint32_t)*puc_buffer++;

		u16AttributeIndex = ((uint16_t)*puc_buffer++) << 8;
		u16AttributeIndex += (uint16_t)*puc_buffer;
		AdpGetRequest(u32AttributeId, u16AttributeIndex);
		status = SERIAL_STATUS_SUCCESS;
	}

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerAdpRouteDiscoveryRequest(const uint8_t *puc_msg_content)
{
	enum ESerialStatus status = SERIAL_STATUS_NOT_ALLOWED;
	uint16_t u16DstAddr;
	uint8_t u8MaxHops;
	uint8_t *puc_buffer = (uint8_t *)puc_msg_content;

	if (adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_DEV || adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_COORD) {
		u16DstAddr = ((uint16_t)*puc_buffer++) << 8;
		u16DstAddr += (uint16_t)*puc_buffer++;
		u8MaxHops = (uint8_t)*puc_buffer;

		AdpRouteDiscoveryRequest(u16DstAddr, u8MaxHops);
		status = SERIAL_STATUS_SUCCESS;
	}

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerAdpMacSetRequest(const uint8_t *puc_msg_content)
{
	enum ESerialStatus status = SERIAL_STATUS_INVALID_PARAMETER;
	struct TAdpMacSetConfirm setConfirm;
	uint32_t u32AttributeId;
	uint16_t u16AttributeIndex;
	struct TMacWrpPibValue pibValue;
	uint8_t uc_serial_response_len;

	if (adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_DEV || adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_COORD) {
		uint8_t *puc_buffer = (uint8_t *)puc_msg_content;
		process_MIB_set_request(puc_buffer, (enum EMacWrpPibAttribute*)&u32AttributeId, &u16AttributeIndex, &pibValue);
		AdpMacSetRequestSync(u32AttributeId, u16AttributeIndex, pibValue.m_u8Length, &pibValue.m_au8Value[0], &setConfirm);
		status = SERIAL_STATUS_SUCCESS;

		uc_serial_response_len = 0;
		uc_serial_rsp_buf[uc_serial_response_len++] = SERIAL_MSG_ADP_MAC_SET_CONFIRM;
		uc_serial_rsp_buf[uc_serial_response_len++] = setConfirm.m_u8Status;
		uc_serial_rsp_buf[uc_serial_response_len++] = (uint8_t)((setConfirm.m_u32AttributeId >> 24) & 0xFF);
		uc_serial_rsp_buf[uc_serial_response_len++] = (uint8_t)((setConfirm.m_u32AttributeId >> 16) & 0xFF);
		uc_serial_rsp_buf[uc_serial_response_len++] = (uint8_t)((setConfirm.m_u32AttributeId >> 8) & 0xFF);
		uc_serial_rsp_buf[uc_serial_response_len++] = (uint8_t)(setConfirm.m_u32AttributeId & 0xFF);
		uc_serial_rsp_buf[uc_serial_response_len++] = (uint8_t)(setConfirm.m_u16AttributeIndex >> 8);
		uc_serial_rsp_buf[uc_serial_response_len++] = (uint8_t)(setConfirm.m_u16AttributeIndex & 0xFF);
		/* set usi parameters */
		x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
		x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
		x_adp_serial_msg.us_len = uc_serial_response_len;
		usi_send_cmd(&x_adp_serial_msg);
	}

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerAdpMacGetRequest(const uint8_t *puc_msg_content)
{
	enum ESerialStatus status = SERIAL_STATUS_NOT_ALLOWED;
	struct TAdpMacGetConfirm getConfirm;
	uint32_t u32AttributeId;
	uint16_t u16AttributeIndex;
	uint8_t uc_serial_response_len;
	uint8_t *puc_buffer = (uint8_t *)puc_msg_content;

	enum EMacWrpPibAttribute eAttribute;
	struct TMacWrpPibValue pibValue;
	enum EMacWrpStatus eGetStatus;

	if (adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_DEV || adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_COORD) {
		u32AttributeId = ((uint32_t)*puc_buffer++) << 24;
		u32AttributeId += ((uint32_t)*puc_buffer++) << 16;
		u32AttributeId += ((uint32_t)*puc_buffer++) << 8;
		u32AttributeId += (uint32_t)*puc_buffer++;

		u16AttributeIndex = ((uint16_t)*puc_buffer++) << 8;
		u16AttributeIndex += (uint16_t)*puc_buffer;
		AdpMacGetRequestSync(u32AttributeId, u16AttributeIndex, &getConfirm);
		status = SERIAL_STATUS_SUCCESS;

		uc_serial_response_len = 0;
		uc_serial_rsp_buf[uc_serial_response_len++] = SERIAL_MSG_ADP_MAC_GET_CONFIRM;
		eGetStatus = (enum EMacWrpStatus)(getConfirm.m_u8Status);
		eAttribute = (enum EMacWrpPibAttribute)(getConfirm.m_u32AttributeId);
		u16AttributeIndex = getConfirm.m_u16AttributeIndex;
		pibValue.m_u8Length = getConfirm.m_u8AttributeLength;
		memcpy(&pibValue.m_au8Value, &getConfirm.m_au8AttributeValue, sizeof(pibValue.m_au8Value));

		uc_serial_response_len += process_MIB_get_confirm(&uc_serial_rsp_buf[uc_serial_response_len], eGetStatus, eAttribute, u16AttributeIndex, &pibValue);

		/* set usi parameters */
		x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
		x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
		x_adp_serial_msg.us_len = uc_serial_response_len;
		usi_send_cmd(&x_adp_serial_msg);
	}

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerAdpPathDiscoveryRequest(const uint8_t *puc_msg_content)
{
	enum ESerialStatus status = SERIAL_STATUS_NOT_ALLOWED;
	uint16_t u16DstAddr;
	uint8_t u8MetricType;
	uint8_t *puc_buffer = (uint8_t *)puc_msg_content;

	if (adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_DEV || adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_COORD) {
		u16DstAddr = ((uint16_t)*puc_buffer++) << 8;
		u16DstAddr += (uint16_t)*puc_buffer++;
		u8MetricType = (uint8_t)*puc_buffer;

		AdpPathDiscoveryRequest(u16DstAddr, u8MetricType);
		status = SERIAL_STATUS_SUCCESS;
	}

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerLbpSetRequest(const uint8_t *puc_msg_content)
{
	enum ESerialStatus status = SERIAL_STATUS_NOT_ALLOWED;
	struct TLbpSetParamConfirm setConfirm;
	uint32_t u32AttributeId;
	uint16_t u16AttributeIndex;
	uint8_t u8AttributeLength;
	uint8_t *puc_buffer = (uint8_t *)puc_msg_content;
	uint8_t u8AttributeLengthCnt = 0;
	uint8_t uc_serial_response_len;

	if (adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_DEV || adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_COORD) {
		u32AttributeId = ((uint32_t)*puc_buffer++) << 24;
		u32AttributeId += ((uint32_t)*puc_buffer++) << 16;
		u32AttributeId += ((uint32_t)*puc_buffer++) << 8;
		u32AttributeId += (uint32_t)*puc_buffer++;

		u16AttributeIndex = ((uint16_t)*puc_buffer++) << 8;
		u16AttributeIndex += (uint16_t)*puc_buffer++;

		u8AttributeLength = *puc_buffer++;

		switch (u32AttributeId) {
		/* 16-bit IBs */
		case LBP_IB_MSG_TIMEOUT:
			mem_copy_from_usi_endianness_uint16(auc_aux_endiannes_buf, puc_buffer);
			u8AttributeLengthCnt = 2;
			break;

		/* Tables and lists */
		case LBP_IB_IDS:
			if (sb_arib_band) {
				u8AttributeLengthCnt = NETWORK_ACCESS_IDENTIFIER_SIZE_S_ARIB;
			} else {
				u8AttributeLengthCnt = NETWORK_ACCESS_IDENTIFIER_SIZE_S_CENELEC_FCC;
			}

			memcpy(auc_aux_endiannes_buf, puc_buffer, u8AttributeLengthCnt);
			break;

		case LBP_IB_IDP:
			if (sb_arib_band) {
				u8AttributeLengthCnt = NETWORK_ACCESS_IDENTIFIER_SIZE_P_ARIB;
			} else {
				u8AttributeLengthCnt = NETWORK_ACCESS_IDENTIFIER_SIZE_P_CENELEC_FCC;
			}

			memcpy(auc_aux_endiannes_buf, puc_buffer, u8AttributeLengthCnt);
			break;

		case LBP_IB_PSK:
		case LBP_IB_GMK:
		case LBP_IB_REKEY_GMK:
		case LBP_IB_RANDP:
			memcpy(auc_aux_endiannes_buf, puc_buffer, 16);
			u8AttributeLengthCnt = 16;
			break;

		default:
			break;
		}

		if (u8AttributeLength == u8AttributeLengthCnt) {
			status = SERIAL_STATUS_SUCCESS;
			if (adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_COORD) {
				LBP_SetParamCoord(u32AttributeId, u16AttributeIndex, u8AttributeLengthCnt, auc_aux_endiannes_buf, &setConfirm);
			} else {
				LBP_SetParamDev(u32AttributeId, u16AttributeIndex, u8AttributeLengthCnt, auc_aux_endiannes_buf, &setConfirm);
			}

			uc_serial_response_len = 0;
			uc_serial_rsp_buf[uc_serial_response_len++] = SERIAL_MSG_LBP_SET_CONFIRM;
			uc_serial_rsp_buf[uc_serial_response_len++] = (uint8_t)setConfirm.eStatus;
			uc_serial_rsp_buf[uc_serial_response_len++] = (uint8_t)(u32AttributeId >> 24);
			uc_serial_rsp_buf[uc_serial_response_len++] = (uint8_t)(u32AttributeId >> 16);
			uc_serial_rsp_buf[uc_serial_response_len++] = (uint8_t)(u32AttributeId >> 8);
			uc_serial_rsp_buf[uc_serial_response_len++] = (uint8_t)u32AttributeId;
			uc_serial_rsp_buf[uc_serial_response_len++] = (uint8_t)(u16AttributeIndex >> 8);
			uc_serial_rsp_buf[uc_serial_response_len++] = (uint8_t)u16AttributeIndex;
			/* set usi parameters */
			x_adp_serial_msg.uc_protocol_type = PROTOCOL_ADP_G3;
			x_adp_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
			x_adp_serial_msg.us_len = uc_serial_response_len;
			usi_send_cmd(&x_adp_serial_msg);
		} else {
			status = SERIAL_STATUS_INVALID_PARAMETER;
		}
	}

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerLbpDevForceRegister(const uint8_t *puc_msg_content)
{
	enum ESerialStatus status = SERIAL_STATUS_NOT_ALLOWED;
	struct TAdpExtendedAddress *pEUI64Address;
	struct TGroupMasterKey *pGMK;
	uint16_t u16ShortAddress, u16PanId;
	uint8_t *puc_buffer = (uint8_t *)puc_msg_content;

	if (adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_DEV) {
		pEUI64Address = (struct TAdpExtendedAddress *)puc_buffer;
		puc_buffer += 8;
		u16ShortAddress = ((uint16_t)*puc_buffer++) << 8;
		u16ShortAddress += (uint16_t)*puc_buffer++;
		u16PanId = ((uint16_t)*puc_buffer++) << 8;
		u16PanId += (uint16_t)*puc_buffer++;
		pGMK = (struct TGroupMasterKey *)puc_buffer;

		LBP_ForceRegister(pEUI64Address, u16ShortAddress, u16PanId, pGMK);
		status = SERIAL_STATUS_SUCCESS;
	}

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerLbpCoordKickDevice(const uint8_t *puc_msg_content)
{
	enum ESerialStatus status = SERIAL_STATUS_NOT_ALLOWED;
	struct TAdpExtendedAddress *pEUI64Address;
	uint16_t u16ShortAddress;
	uint8_t *puc_buffer = (uint8_t *)puc_msg_content;

	if (adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_COORD) {
		u16ShortAddress = ((uint16_t)*puc_buffer++) << 8;
		u16ShortAddress += (uint16_t)*puc_buffer++;
		pEUI64Address = (struct TAdpExtendedAddress *)puc_buffer;

		LBP_KickDevice(u16ShortAddress, pEUI64Address);
		status = SERIAL_STATUS_SUCCESS;
	}

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerLbpCoordRekey(const uint8_t *puc_msg_content)
{
	enum ESerialStatus status = SERIAL_STATUS_NOT_ALLOWED;
	struct TAdpExtendedAddress *pEUI64Address;
	uint16_t u16ShortAddress;
	bool bDistribute;
	uint8_t *puc_buffer = (uint8_t *)puc_msg_content;

	if (adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_COORD) {
		u16ShortAddress = ((uint16_t)*puc_buffer++) << 8;
		u16ShortAddress += (uint16_t)*puc_buffer++;
		pEUI64Address = (struct TAdpExtendedAddress *)puc_buffer;
		puc_buffer += 8;
		bDistribute = (bool)*puc_buffer;

		LBP_Rekey(u16ShortAddress, pEUI64Address, bDistribute);
		status = SERIAL_STATUS_SUCCESS;
	}

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerLbpCoordSetRekeyPhase(const uint8_t *puc_msg_content)
{
	enum ESerialStatus status = SERIAL_STATUS_NOT_ALLOWED;
	bool bRekeyStart;
	uint8_t *puc_buffer = (uint8_t *)puc_msg_content;

	if (adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_COORD) {
		bRekeyStart = (bool)*puc_buffer;

		LBP_SetRekeyPhase(bRekeyStart);
		status = SERIAL_STATUS_SUCCESS;
	}

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerLbpCoordActivateNewKey(const uint8_t *puc_msg_content)
{
	UNUSED(puc_msg_content);
	enum ESerialStatus status = SERIAL_STATUS_NOT_ALLOWED;

	if (adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_COORD) {
		LBP_activate_new_key();
		status = SERIAL_STATUS_SUCCESS;
	}

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerLbpCoordShortAddressAssign(const uint8_t *puc_msg_content)
{
	enum ESerialStatus status = SERIAL_STATUS_NOT_ALLOWED;
	uint8_t *pEUI64Address;
	uint16_t u16AssignedAddress;
	uint8_t *puc_buffer = (uint8_t *)puc_msg_content;

	if (adp_mac_serial_if_get_state() == SERIAL_MODE_ADP_COORD) {
		pEUI64Address = puc_buffer;
		puc_buffer += 8;
		u16AssignedAddress = ((uint16_t)*puc_buffer++) << 8;
		u16AssignedAddress += (uint16_t)*puc_buffer++;

		LBP_ShortAddressAssign(pEUI64Address, u16AssignedAddress);
		status = SERIAL_STATUS_SUCCESS;
	}

	return status;
}

uint8_t serial_if_g3adp_api_parser(uint8_t *puc_rx_msg, uint16_t us_len)
{
	uint8_t uc_serial_if_cmd;
	uint8_t *puc_rx;
	enum ESerialStatus status = SERIAL_STATUS_UNKNOWN_COMMAND;

	/* Protection for invalid us_length */
	if (!us_len) {
		return false;
	}

	/* Process received message */
	uc_serial_if_cmd = puc_rx_msg[0] & 0x7F;
	puc_rx = &puc_rx_msg[1];

	switch (uc_serial_if_cmd) {
	case SERIAL_MSG_ADP_INITIALIZE:
		status = _triggerAdpInitialize(puc_rx);
		break;

	case SERIAL_MSG_ADP_DATA_REQUEST:
		status = _triggerAdpDataRequest(puc_rx);
		break;

	case SERIAL_MSG_ADP_DISCOVERY_REQUEST:
		status = _triggerAdpDiscoveryRequest(puc_rx);
		break;

	case SERIAL_MSG_ADP_NETWORK_START_REQUEST:
		status = _triggerAdpNetworkStartRequest(puc_rx);
		break;

	case SERIAL_MSG_ADP_NETWORK_JOIN_REQUEST:
		status = _triggerAdpNetworkJoinRequest(puc_rx);
		break;

	case SERIAL_MSG_ADP_NETWORK_LEAVE_REQUEST:
		status = _triggerAdpNetworkLeaveRequest(puc_rx);
		break;

	case SERIAL_MSG_ADP_RESET_REQUEST:
		status = _triggerAdpResetRequest(puc_rx);
		break;

	case SERIAL_MSG_ADP_SET_REQUEST:
		_triggerAdpSetRequest(puc_rx);
		break;

	case SERIAL_MSG_ADP_GET_REQUEST:
		_triggerAdpGetRequest(puc_rx);
		break;

	case SERIAL_MSG_ADP_ROUTE_DISCOVERY_REQUEST:
		status = _triggerAdpRouteDiscoveryRequest(puc_rx);
		break;

	case SERIAL_MSG_ADP_PATH_DISCOVERY_REQUEST:
		status = _triggerAdpPathDiscoveryRequest(puc_rx);
		break;

	case SERIAL_MSG_ADP_MAC_SET_REQUEST:
		status = _triggerAdpMacSetRequest(puc_rx);
		break;

	case SERIAL_MSG_ADP_MAC_GET_REQUEST:
		status = _triggerAdpMacGetRequest(puc_rx);
		break;

	case SERIAL_MSG_ADP_NO_IP_DATA_REQUEST:
		status = _triggerAdpNoIPDataRequest(puc_rx);
		break;

	case SERIAL_MSG_LBP_SET_REQUEST:
		status = _triggerLbpSetRequest(puc_rx);
		break;

	case SERIAL_MSG_LBP_DEV_FORCE_REGISTER:
		status = _triggerLbpDevForceRegister(puc_rx);
		break;

	case SERIAL_MSG_LBP_COORD_KICK_DEVICE:
		status = _triggerLbpCoordKickDevice(puc_rx);
		break;

	case SERIAL_MSG_LBP_COORD_REKEY:
		status = _triggerLbpCoordRekey(puc_rx);
		break;

	case SERIAL_MSG_LBP_COORD_SET_REKEY_PHASE:
		status = _triggerLbpCoordSetRekeyPhase(puc_rx);
		break;

	case SERIAL_MSG_LBP_COORD_ACTIVATE_NEW_KEY:
		status = _triggerLbpCoordActivateNewKey(puc_rx);
		break;

	case SERIAL_MSG_LBP_COORD_SHORT_ADDRESS_ASSIGN:
		status = _triggerLbpCoordShortAddressAssign(puc_rx);
		break;

	default:
		break;
	}

	/* initialize doesn't have Confirm so send Status */
	/* other messages all have send / confirm send status only if there is a processing error */
	if ((status != SERIAL_STATUS_UNKNOWN_COMMAND) && ((status != SERIAL_STATUS_SUCCESS) || (uc_serial_if_cmd == SERIAL_MSG_ADP_INITIALIZE))) {
		MsgStatus(status, uc_serial_if_cmd);
		status = SERIAL_STATUS_SUCCESS;
	}

	/* if (status == SERIAL_STATUS_SUCCESS) return true; */
	return true;
}

struct TAdpNotifications *serial_if_adp_get_notifications(void)
{
	ss_notifications.fnctAdpDataConfirm = AdpNotification_DataConfirm;
	ss_notifications.fnctAdpDataIndication = AdpNotification_AdpdDataIndication;
	ss_notifications.fnctAdpDiscoveryConfirm = AdpNotification_DiscoveryConfirm;
	ss_notifications.fnctAdpDiscoveryIndication = AdpNotification_DiscoveryIndication;
	ss_notifications.fnctAdpNetworkStartConfirm = AdpNotification_NetworkStartConfirm;
	ss_notifications.fnctAdpResetConfirm = AdpNotification_ResetConfirm;
	ss_notifications.fnctAdpSetConfirm = AdpNotification_SetConfirm;
	ss_notifications.fnctAdpGetConfirm = AdpNotification_GetConfirm;
	ss_notifications.fnctAdpRouteDiscoveryConfirm = AdpNotification_RouteDiscoveryConfirm;
	ss_notifications.fnctAdpPathDiscoveryConfirm = AdpNotification_PathDiscoveryConfirm;
	ss_notifications.fnctAdpNetworkStatusIndication = AdpNotification_NetworkStatusIndication;
	ss_notifications.fnctAdpBufferIndication = AdpNotification_BufferIndication;
	ss_notifications.fnctAdpPREQIndication = AppAdpNotification_PREQIndication;
	ss_notifications.fnctAdpUpdNonVolatileDataIndication = AppAdpNotification_UpdNonVolatileDataIndication;
	ss_notifications.fnctAdpRouteNotFoundIndication = AppAdpNotification_RouteNotFoundIndication;
	return &ss_notifications;
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
