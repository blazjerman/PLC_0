/**
 *
 * \file
 *
 * \brief MAC Serialization file
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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Serial interface */
#include "hal/hal.h"
#include "mac_wrapper.h"
#include "AdpApi.h"
#include "Random.h"
#include "serial_if_mac.h"
#include "serial_if_common.h"
#include "serial_if_mib_common.h"
#include "usi.h"
#include <crypto/aes_wrapper.h>

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
	#endif
/**INDENT-ON**/
/* / @endcond */

/* Default empty USI interface*/

/*
 * extern int8_t usi_send_cmd(void *msg);
 * int8_t Dummy_serial_send_cmd(void *msg);
 *
 * #ifdef __GNUC__
 * int8_t usi_send_cmd ( void *msg ) __attribute__ ((weak, alias("Dummy_serial_send_cmd")));
 * #endif
 *
 * #ifdef __ICCARM__
 * #pragma weak usi_send_cmd=Dummy_serial_send_cmd
 * #endif
 */

/* ! \name Data structure to communicate with USI layer */
/* @{ */
static x_usi_serial_cmd_params_t x_mac_serial_msg;
/* @} */

static uint8_t uc_serial_rsp_buf[2048]; /* !<  Response working buffer */
static uint8_t auc_ext_address_mac[8]; /* EUI64 */

/* Known message ids */
enum ESerialMessageId {
	/* Generic messages */
	SERIAL_MSG_STATUS = 0,

	/* MAC ACCESS */
	SERIAL_MSG_MAC_REQUEST_MESSAGES_BEGIN = 50,
	SERIAL_MSG_MAC_INITIALIZE = SERIAL_MSG_MAC_REQUEST_MESSAGES_BEGIN,
	SERIAL_MSG_MAC_DATA_REQUEST,
	SERIAL_MSG_MAC_GET_REQUEST,
	SERIAL_MSG_MAC_SET_REQUEST,
	SERIAL_MSG_MAC_RESET_REQUEST,
	SERIAL_MSG_MAC_SCAN_REQUEST,
	SERIAL_MSG_MAC_START_REQUEST,
	SERIAL_MSG_MAC_REQUEST_MESSAGES_END = SERIAL_MSG_MAC_START_REQUEST,

	SERIAL_MSG_MAC_DATA_CONFIRM = 60,
	SERIAL_MSG_MAC_DATA_INDICATION,
	SERIAL_MSG_MAC_GET_CONFIRM,
	SERIAL_MSG_MAC_SET_CONFIRM,
	SERIAL_MSG_MAC_RESET_CONFIRM,
	SERIAL_MSG_MAC_SCAN_CONFIRM,
	SERIAL_MSG_MAC_BEACON_NOTIFY,
	SERIAL_MSG_MAC_START_CONFIRM,
	SERIAL_MSG_MAC_COMM_STATUS_INDICATION,
	SERIAL_MSG_MAC_SNIFFER_INDICATION
};

/* Status codes related to HostInterface processing */
enum ESerialStatus {
	SERIAL_STATUS_SUCCESS = 0,
	SERIAL_STATUS_NOT_ALLOWED,
	SERIAL_STATUS_UNKNOWN_COMMAND,
	SERIAL_STATUS_INVALID_PARAMETER
};
/* typedef enum ESerialStatus (*MsgFunct)(const struct TSerialMessage *pMessage); */

/* Flag to check whether MAC Wrapper is already initialized (if 2 MACs are initialized) */
static bool b_already_initialized = false;

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
	x_mac_serial_msg.uc_protocol_type = PROTOCOL_MAC_G3;
	x_mac_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_mac_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_mac_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void MacNotification_DataConfirm(struct TMacWrpDataConfirm *pParameters)
{
	uint8_t us_serial_response_len;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_MAC_DATA_CONFIRM;
	uc_serial_rsp_buf[us_serial_response_len++] = pParameters->m_u8MsduHandle;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_eStatus;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)((pParameters->m_nTimestamp >> 24) & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)((pParameters->m_nTimestamp >> 16) & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)((pParameters->m_nTimestamp >> 8) & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_nTimestamp & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_eMediaType;
	/* set usi parameters */
	x_mac_serial_msg.uc_protocol_type = PROTOCOL_MAC_G3;
	x_mac_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_mac_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_mac_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void MacNotification_DataIndication(struct TMacWrpDataIndication *pParameters)
{
	uint16_t us_serial_response_len;

	uint8_t uc_src_addr_len = 0;
	uint8_t uc_dst_addr_len = 0;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_MAC_DATA_INDICATION;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_nSrcPanId >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_nSrcPanId & 0xFF;

	if (pParameters->m_SrcAddr.m_eAddrMode == MAC_WRP_ADDRESS_MODE_SHORT) {
		uc_src_addr_len = 2;
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)uc_src_addr_len;
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_SrcAddr.m_nShortAddress >> 8);
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_SrcAddr.m_nShortAddress;
	} else if (pParameters->m_SrcAddr.m_eAddrMode == MAC_WRP_ADDRESS_MODE_EXTENDED) {
		uc_src_addr_len = 8;
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)uc_src_addr_len;
		memcpy(&uc_serial_rsp_buf[us_serial_response_len], &pParameters->m_SrcAddr.m_ExtendedAddress.m_au8Address[0], uc_src_addr_len);
		us_serial_response_len += uc_src_addr_len;
	} else {
		return; /* This line must never be reached */
	}

	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_nDstPanId >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_nDstPanId & 0xFF;

	if (pParameters->m_DstAddr.m_eAddrMode == MAC_WRP_ADDRESS_MODE_SHORT) {
		uc_dst_addr_len = 2;
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)uc_dst_addr_len;
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_DstAddr.m_nShortAddress >> 8);
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_DstAddr.m_nShortAddress & 0xFF;
	} else if (pParameters->m_DstAddr.m_eAddrMode == MAC_WRP_ADDRESS_MODE_EXTENDED) {
		uc_dst_addr_len = 8;
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)uc_dst_addr_len;
		memcpy(&uc_serial_rsp_buf[us_serial_response_len], &pParameters->m_DstAddr.m_ExtendedAddress.m_au8Address[0], uc_dst_addr_len);
		us_serial_response_len += uc_dst_addr_len;
	} else {
		return; /* This line must never be reached */
	}

	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_u8MpduLinkQuality;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_u8Dsn;

	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)((pParameters->m_nTimestamp >> 24) & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)((pParameters->m_nTimestamp >> 16) & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)((pParameters->m_nTimestamp >> 8) & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_nTimestamp & 0xFF);

	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_eSecurityLevel;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_u8KeyIndex;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_eQualityOfService;

	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_u8RecvModulation;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_u8RecvModulationScheme;
	memcpy(&uc_serial_rsp_buf[us_serial_response_len], &pParameters->m_RecvToneMap.m_au8Tm[0], 3 );
	us_serial_response_len += 3;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_u8ComputedModulation;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_u8ComputedModulationScheme;
	memcpy(&uc_serial_rsp_buf[us_serial_response_len], &pParameters->m_ComputedToneMap.m_au8Tm[0], 3 );
	us_serial_response_len += 3;

	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_eMediaType;

	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_u16MsduLength >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_u16MsduLength & 0xFF);

	memcpy(&uc_serial_rsp_buf[us_serial_response_len], pParameters->m_pMsdu, pParameters->m_u16MsduLength);
	us_serial_response_len += pParameters->m_u16MsduLength;

	/* set usi parameters */
	x_mac_serial_msg.uc_protocol_type = PROTOCOL_MAC_G3;
	x_mac_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_mac_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_mac_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void MacNotification_MacSnifferIndication(struct TMacWrpSnifferIndication *pParameters)
{
	/* TEST: Send sniffer indication as data indication */
	uint16_t us_serial_response_len;

	uint8_t uc_src_addr_len;
	uint8_t uc_dst_addr_len;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_MAC_SNIFFER_INDICATION;

	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_u8FrameType;

	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_nSrcPanId >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_nSrcPanId & 0xFF;

	if (pParameters->m_SrcAddr.m_eAddrMode == MAC_WRP_ADDRESS_MODE_SHORT) {
		uc_src_addr_len = 2;
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)uc_src_addr_len;
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_SrcAddr.m_nShortAddress >> 8);
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_SrcAddr.m_nShortAddress & 0xFF);
	} else if (pParameters->m_SrcAddr.m_eAddrMode == MAC_WRP_ADDRESS_MODE_EXTENDED) {
		uc_src_addr_len = 8;
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)uc_src_addr_len;
		memcpy(&uc_serial_rsp_buf[us_serial_response_len], &pParameters->m_SrcAddr.m_ExtendedAddress.m_au8Address[0], uc_src_addr_len);
		us_serial_response_len += uc_src_addr_len;
	} else {
		uc_src_addr_len = 0;
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)uc_src_addr_len;
	}

	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_nDstPanId >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_nDstPanId & 0xFF);

	if (pParameters->m_DstAddr.m_eAddrMode == MAC_WRP_ADDRESS_MODE_SHORT) {
		uc_dst_addr_len = 2;
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)uc_dst_addr_len;
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_DstAddr.m_nShortAddress >> 8);
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_DstAddr.m_nShortAddress & 0xFF);
	} else if (pParameters->m_DstAddr.m_eAddrMode == MAC_WRP_ADDRESS_MODE_EXTENDED) {
		uc_dst_addr_len = 8;
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)uc_dst_addr_len;
		memcpy(&uc_serial_rsp_buf[us_serial_response_len], &pParameters->m_DstAddr.m_ExtendedAddress.m_au8Address[0], uc_dst_addr_len);
		us_serial_response_len += uc_dst_addr_len;
	} else {
		uc_dst_addr_len = 0;
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)uc_dst_addr_len;
	}

	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_u8MpduLinkQuality;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_u8Dsn;

	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)((pParameters->m_nTimestamp >> 24) & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)((pParameters->m_nTimestamp >> 16) & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)((pParameters->m_nTimestamp >> 8) & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_nTimestamp & 0xFF);

	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_eSecurityLevel;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_u8KeyIndex;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_eQualityOfService;

	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_u8RecvModulation;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_u8RecvModulationScheme;
	memcpy(&uc_serial_rsp_buf[us_serial_response_len], &pParameters->m_RecvToneMap.m_au8Tm[0], 3 );
	us_serial_response_len += 3;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_u8ComputedModulation;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)pParameters->m_u8ComputedModulationScheme;
	memcpy(&uc_serial_rsp_buf[us_serial_response_len], &pParameters->m_ComputedToneMap.m_au8Tm[0], 3 );
	us_serial_response_len += 3;

	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_u16MsduLength >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_u16MsduLength & 0xFF);

	memcpy(&uc_serial_rsp_buf[us_serial_response_len], pParameters->m_pMsdu, pParameters->m_u16MsduLength);
	us_serial_response_len += pParameters->m_u16MsduLength;

	/* set usi parameters */
	x_mac_serial_msg.uc_protocol_type = PROTOCOL_MAC_G3;
	x_mac_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_mac_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_mac_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void MacNotification_ResetConfirm(struct TMacWrpResetConfirm *pParameters)
{
	uint8_t us_serial_response_len;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_MAC_RESET_CONFIRM;
	uc_serial_rsp_buf[us_serial_response_len++] = pParameters->m_eStatus;
	/* set usi parameters */
	x_mac_serial_msg.uc_protocol_type = PROTOCOL_MAC_G3;
	x_mac_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_mac_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_mac_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void MacNotification_BeaconNotify(struct TMacWrpBeaconNotifyIndication *pParameters)
{
	uint8_t us_serial_response_len;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_MAC_BEACON_NOTIFY;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_PanDescriptor.m_nPanId >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_PanDescriptor.m_nPanId & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_PanDescriptor.m_u8LinkQuality);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_PanDescriptor.m_nLbaAddress >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_PanDescriptor.m_nLbaAddress & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_PanDescriptor.m_u16RcCoord >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_PanDescriptor.m_u16RcCoord & 0xFF);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_PanDescriptor.m_eMediaType);
	/* set usi parameters */
	x_mac_serial_msg.uc_protocol_type = PROTOCOL_MAC_G3;
	x_mac_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_mac_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_mac_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void MacNotification_ScanConfirm(struct TMacWrpScanConfirm *pParameters)
{
	uint8_t us_serial_response_len;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_MAC_SCAN_CONFIRM;
	uc_serial_rsp_buf[us_serial_response_len++] = pParameters->m_eStatus;
	/* set usi parameters */
	x_mac_serial_msg.uc_protocol_type = PROTOCOL_MAC_G3;
	x_mac_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_mac_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_mac_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void MacNotification_StartConfirm(struct TMacWrpStartConfirm *pParameters)
{
	uint8_t us_serial_response_len;

	us_serial_response_len = 0;
	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_MAC_START_CONFIRM;
	uc_serial_rsp_buf[us_serial_response_len++] = pParameters->m_eStatus;
	/* set usi parameters */
	x_mac_serial_msg.uc_protocol_type = PROTOCOL_MAC_G3;
	x_mac_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_mac_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_mac_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static void MacNotification_CommStatusIndication(struct TMacWrpCommStatusIndication *pParameters)
{
	uint8_t us_serial_response_len;

	us_serial_response_len = 0;
	uint8_t uc_src_addr_len = 0;
	uint8_t uc_dst_addr_len = 0;

	uc_serial_rsp_buf[us_serial_response_len++] = SERIAL_MSG_MAC_COMM_STATUS_INDICATION;
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_nPanId >> 8);
	uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_nPanId & 0xFF);
	if (pParameters->m_SrcAddr.m_eAddrMode == MAC_WRP_ADDRESS_MODE_SHORT) {
		uc_src_addr_len = 2;
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)uc_src_addr_len;
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_SrcAddr.m_nShortAddress >> 8);
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_SrcAddr.m_nShortAddress & 0xFF);
	} else if (pParameters->m_SrcAddr.m_eAddrMode == MAC_WRP_ADDRESS_MODE_EXTENDED) {
		uc_src_addr_len = 8;
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)uc_src_addr_len;
		memcpy(&uc_serial_rsp_buf[us_serial_response_len], &pParameters->m_SrcAddr.m_ExtendedAddress.m_au8Address[0], uc_src_addr_len);
		us_serial_response_len += uc_src_addr_len;
	} else {
		return; /* This line must never be reached */
	}

	if (pParameters->m_DstAddr.m_eAddrMode == MAC_WRP_ADDRESS_MODE_SHORT) {
		uc_dst_addr_len = 2;
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)uc_dst_addr_len;
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_DstAddr.m_nShortAddress >> 8);
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)(pParameters->m_DstAddr.m_nShortAddress & 0xFF);
	} else if (pParameters->m_DstAddr.m_eAddrMode == MAC_WRP_ADDRESS_MODE_EXTENDED) {
		uc_dst_addr_len = 8;
		uc_serial_rsp_buf[us_serial_response_len++] = (uint8_t)uc_dst_addr_len;
		memcpy(&uc_serial_rsp_buf[us_serial_response_len], &pParameters->m_DstAddr.m_ExtendedAddress.m_au8Address[0], uc_dst_addr_len);
		us_serial_response_len += uc_dst_addr_len;
	} else {
		return; /* This line must never be reached */
	}

	uc_serial_rsp_buf[us_serial_response_len++] = pParameters->m_eStatus;
	uc_serial_rsp_buf[us_serial_response_len++] = pParameters->m_eSecurityLevel;
	uc_serial_rsp_buf[us_serial_response_len++] = pParameters->m_u8KeyIndex;

	uc_serial_rsp_buf[us_serial_response_len++] = pParameters->m_eMediaType;

	/* set usi parameters */
	x_mac_serial_msg.uc_protocol_type = PROTOCOL_MAC_G3;
	x_mac_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_mac_serial_msg.us_len = us_serial_response_len;
	usi_send_cmd(&x_mac_serial_msg);
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerMacInitialize(const uint8_t *puc_msg_content)
{
	struct TMacWrpPibValue pibValue;
	enum ESerialStatus status = SERIAL_STATUS_INVALID_PARAMETER;

	uint8_t u8Band = *puc_msg_content;

	/* Check whether already initialized */
	if (b_already_initialized) {
		return SERIAL_STATUS_SUCCESS;
	}

	/* Initialize Random and Crypto, as ADP has not initialized it (MAC Access) */
	Random_Initialize();
	crypto_init();

	struct TMacWrpNotifications notifications;

	notifications.m_MacWrpDataConfirm = MacNotification_DataConfirm;
	notifications.m_MacWrpDataIndication = MacNotification_DataIndication;
	notifications.m_MacWrpResetConfirm = MacNotification_ResetConfirm;
	notifications.m_MacWrpBeaconNotifyIndication = MacNotification_BeaconNotify;
	notifications.m_MacWrpScanConfirm = MacNotification_ScanConfirm;
	notifications.m_MacWrpStartConfirm = MacNotification_StartConfirm;
	notifications.m_MacWrpCommStatusIndication = MacNotification_CommStatusIndication;
	notifications.m_MacWrpSnifferIndication = MacNotification_MacSnifferIndication;

	MacWrapperInitialize(&notifications, u8Band);

	platform_init_eui64(auc_ext_address_mac);
	memcpy(pibValue.m_au8Value, auc_ext_address_mac, sizeof(auc_ext_address_mac));
	pibValue.m_u8Length = sizeof(auc_ext_address_mac);
	MacWrapperMlmeSetRequestSync(MAC_WRP_PIB_MANUF_EXTENDED_ADDRESS, 0, &pibValue);

	adp_mac_serial_if_set_state(SERIAL_MODE_MAC);
	status = SERIAL_STATUS_SUCCESS;

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerMacDataRequest(const uint8_t *puc_msg_content)
{
	uint8_t *puc_buffer = (uint8_t *)puc_msg_content;
	enum ESerialStatus status = SERIAL_STATUS_NOT_ALLOWED;

	if (adp_mac_serial_if_get_state() == SERIAL_MODE_MAC) {
		struct TMacWrpDataRequest parameters;
		uint8_t u8SrcAddrLen;
		uint8_t u8DstAddrLen;

		parameters.m_u8MsduHandle = (uint8_t)*puc_buffer++;
		parameters.m_eSecurityLevel =  (enum EMacWrpSecurityLevel)*puc_buffer++;
		parameters.m_u8KeyIndex = (uint8_t)*puc_buffer++;
		parameters.m_eQualityOfService = (enum EMacWrpQualityOfService)*puc_buffer++;
		parameters.m_u8TxOptions = (uint8_t)*puc_buffer++;
		parameters.m_nDstPanId = ((TMacWrpPanId) * puc_buffer++) << 8;
		parameters.m_nDstPanId +=  (TMacWrpPanId) * puc_buffer++;
		u8SrcAddrLen = (uint8_t)*puc_buffer++;
		if (u8SrcAddrLen == 2) {
			parameters.m_eSrcAddrMode = MAC_WRP_ADDRESS_MODE_SHORT;
		} else if (u8SrcAddrLen == 8) {
			parameters.m_eSrcAddrMode = MAC_WRP_ADDRESS_MODE_EXTENDED;
		} else {
			return SERIAL_STATUS_INVALID_PARAMETER;
		}

		u8DstAddrLen = (uint8_t)*puc_buffer++;
		if (u8DstAddrLen == 2) {
			parameters.m_DstAddr.m_eAddrMode = MAC_WRP_ADDRESS_MODE_SHORT;
			parameters.m_DstAddr.m_nShortAddress = ((TMacWrpShortAddress) * puc_buffer++) << 8;
			parameters.m_DstAddr.m_nShortAddress +=  (TMacWrpShortAddress) * puc_buffer++;
		} else if (u8DstAddrLen == 8) {
			parameters.m_DstAddr.m_eAddrMode = MAC_WRP_ADDRESS_MODE_EXTENDED;
			memcpy(&parameters.m_DstAddr.m_ExtendedAddress, puc_buffer, u8DstAddrLen);
			puc_buffer += u8DstAddrLen;
		} else {
			return SERIAL_STATUS_INVALID_PARAMETER;
		}

		parameters.m_eMediaType = (enum EMacWrpMediaTypeRequest)*puc_buffer++;

		parameters.m_u16MsduLength = ((uint16_t)*puc_buffer++) << 8;
		parameters.m_u16MsduLength +=  (uint16_t)*puc_buffer++;
		parameters.m_pMsdu = puc_buffer;

		MacWrapperMcpsDataRequest(&parameters);

		status = SERIAL_STATUS_SUCCESS;
	}

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerMacGetRequest(const uint8_t *puc_msg_content)
{
	enum ESerialStatus status = SERIAL_STATUS_SUCCESS;
	enum EMacWrpPibAttribute eAttribute;
	uint16_t u16Index;
	struct TMacWrpPibValue pibValue;
	enum EMacWrpStatus eGetStatus;
	uint8_t uc_serial_response_len;

	if (adp_mac_serial_if_get_state() == SERIAL_MODE_MAC) {
		uint8_t *puc_buffer = (uint8_t *)puc_msg_content;
		process_MIB_get_request(puc_buffer, &eAttribute, &u16Index);
		eGetStatus = MacWrapperMlmeGetRequestSync(eAttribute, u16Index, &pibValue);

		uc_serial_response_len = 0;
		uc_serial_rsp_buf[uc_serial_response_len++] = SERIAL_MSG_MAC_GET_CONFIRM;

		uc_serial_response_len += process_MIB_get_confirm(&uc_serial_rsp_buf[uc_serial_response_len], eGetStatus, eAttribute, u16Index, &pibValue);

		/* set usi parameters */
		x_mac_serial_msg.uc_protocol_type = PROTOCOL_MAC_G3;
		x_mac_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
		x_mac_serial_msg.us_len = uc_serial_response_len;
		usi_send_cmd(&x_mac_serial_msg);
	}

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerMacSetRequest(const uint8_t *puc_msg_content)
{
	enum ESerialStatus status = SERIAL_STATUS_INVALID_PARAMETER;
	enum EMacWrpPibAttribute eAttribute;
	uint16_t u16Index;
	struct TMacWrpPibValue pibValue;
	enum EMacWrpStatus eGetStatus = MAC_WRP_STATUS_INVALID_PARAMETER;
	uint8_t uc_serial_response_len;

	if (adp_mac_serial_if_get_state() == SERIAL_MODE_MAC) {
		uint8_t *puc_buffer = (uint8_t *)puc_msg_content;
		process_MIB_set_request(puc_buffer, &eAttribute, &u16Index, &pibValue);
		eGetStatus = MacWrapperMlmeSetRequestSync(eAttribute, u16Index, &pibValue);
		status = SERIAL_STATUS_SUCCESS;
	}

	uc_serial_response_len = 0;
	uc_serial_rsp_buf[uc_serial_response_len++] = SERIAL_MSG_MAC_SET_CONFIRM;

	uc_serial_response_len += process_MIB_set_confirm(&uc_serial_rsp_buf[uc_serial_response_len], eGetStatus, eAttribute, u16Index);

	/* set usi parameters */
	x_mac_serial_msg.uc_protocol_type = PROTOCOL_MAC_G3;
	x_mac_serial_msg.ptr_buf = &uc_serial_rsp_buf[0];
	x_mac_serial_msg.us_len = uc_serial_response_len;
	usi_send_cmd(&x_mac_serial_msg);

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerMacResetRequest(const uint8_t *puc_msg_content)
{
	enum ESerialStatus status = SERIAL_STATUS_NOT_ALLOWED;

	if (adp_mac_serial_if_get_state() == SERIAL_MODE_MAC) {
		struct TMacWrpResetRequest parameters;
		parameters.m_bSetDefaultPib = (bool) * puc_msg_content;
		MacWrapperMlmeResetRequest(&parameters);

		status = SERIAL_STATUS_SUCCESS;
	}

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerMacScanRequest(const uint8_t *puc_msg_content)
{
	uint8_t *puc_buffer = (uint8_t *)puc_msg_content;
	enum ESerialStatus status = SERIAL_STATUS_NOT_ALLOWED;

	if (adp_mac_serial_if_get_state() == SERIAL_MODE_MAC) {
		struct TMacWrpScanRequest parameters;
		parameters.m_u16ScanDuration = ((uint16_t)*puc_buffer++) << 8;
		parameters.m_u16ScanDuration +=  (uint16_t)*puc_buffer;

		MacWrapperMlmeScanRequest(&parameters);
		status = SERIAL_STATUS_SUCCESS;
	}

	return status;
}

/**********************************************************************************************************************/

/**
 **********************************************************************************************************************/
static enum ESerialStatus _triggerMacStartRequest(const uint8_t *puc_msg_content)
{
	uint8_t *puc_buffer = (uint8_t *)puc_msg_content;
	enum ESerialStatus status = SERIAL_STATUS_NOT_ALLOWED;

	if (adp_mac_serial_if_get_state() == SERIAL_MODE_MAC) {
		struct TMacWrpStartRequest parameters;
		parameters.m_nPanId = ((uint16_t)*puc_buffer++) << 8;
		parameters.m_nPanId +=  (uint16_t)*puc_buffer;

		MacWrapperMlmeStartRequest(&parameters);

		status = SERIAL_STATUS_SUCCESS;
	}

	return status;
}

uint8_t serial_if_g3mac_api_parser(uint8_t *puc_rx_msg, uint16_t us_len)
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
	case SERIAL_MSG_MAC_INITIALIZE:
		status = _triggerMacInitialize(puc_rx);
		break;

	case SERIAL_MSG_MAC_DATA_REQUEST:
		status = _triggerMacDataRequest(puc_rx);
		break;

	case SERIAL_MSG_MAC_GET_REQUEST:
		status = _triggerMacGetRequest(puc_rx);
		break;

	case SERIAL_MSG_MAC_SET_REQUEST:
		status = _triggerMacSetRequest(puc_rx);
		break;

	case SERIAL_MSG_MAC_RESET_REQUEST:
		status = _triggerMacResetRequest(puc_rx);
		break;

	case SERIAL_MSG_MAC_SCAN_REQUEST:
		status = _triggerMacScanRequest(puc_rx);
		break;

	case SERIAL_MSG_MAC_START_REQUEST:
		status = _triggerMacStartRequest(puc_rx);
		break;

	default:
		break;
	}

	/* initialize doesn't have Confirm so send Status */
	/* other messages all have send / confirm send status only if there is a processing error */
	if ((status != SERIAL_STATUS_UNKNOWN_COMMAND) && ((status != SERIAL_STATUS_SUCCESS) ||
		(uc_serial_if_cmd == SERIAL_MSG_MAC_INITIALIZE))) {
		MsgStatus(status, uc_serial_if_cmd);
		status = SERIAL_STATUS_SUCCESS;
	}

	return true;
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
