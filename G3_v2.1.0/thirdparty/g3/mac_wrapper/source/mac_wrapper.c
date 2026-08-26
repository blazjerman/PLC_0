/**
 *
 * \file
 *
 * \brief MAC Wrapper Implementation file
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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "conf_global.h"
#include "conf_tables.h"
#include "mac_wrapper.h"
#include "mac_wrapper_defs.h"
#if defined(__PLC_MAC__)
	#include "MacApi.h"
#endif
#if defined(__RF_MAC__)
	#include "MacRfApi.h"
#endif

#define LOG_LEVEL LOG_LVL_INFO
#include <Logger.h>

#if defined(__PLC_MAC__)

#define MAC_MAX_DEVICE_TABLE_ENTRIES (128)

static struct TMacWrpNotifications sUpperLayerNotifications;

static struct TMacTables sMacTables;
struct TDeviceTableEntry macDeviceTable[MAC_MAX_DEVICE_TABLE_ENTRIES];

#endif // defined(__PLC_MAC__)

#if defined(__RF_MAC__)

#ifndef CONF_MAC_POS_TABLE_ENTRIES_RF
#define CONF_MAC_POS_TABLE_ENTRIES_RF    100
#endif

#ifndef CONF_MAC_DSN_TABLE_ENTRIES_RF
#define CONF_MAC_DSN_TABLE_ENTRIES_RF    8
#endif

#define MAC_MAX_POS_TABLE_ENTRIES_RF       CONF_MAC_POS_TABLE_ENTRIES_RF
#define MAC_MAX_DSN_TABLE_ENTRIES_RF       CONF_MAC_DSN_TABLE_ENTRIES_RF

#define MAC_MAX_DEVICE_TABLE_ENTRIES_RF    (128)

static struct TMacWrpNotifications sUpperLayerNotificationsRF;

static struct TMacTablesRF sMacTablesRF;
struct TPOSEntryRF macPOSTableRF[MAC_MAX_POS_TABLE_ENTRIES_RF];
struct TDeviceTableEntry macDeviceTableRF[MAC_MAX_DEVICE_TABLE_ENTRIES_RF];
struct TDsnTableEntryRF macDsnTableRF[MAC_MAX_DSN_TABLE_ENTRIES_RF];

#endif // defined(__RF_MAC__)

#if defined(__PLC_MAC__) && defined(__RF_MAC__)

/* Buffer size to store data to be sent as Mac Data Request */
#define HYAL_BACKUP_BUF_SIZE   400

static const uint16_t crc16_tab[256] = {
  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
  0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
  0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
  0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
  0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
  0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
  0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
  0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
  0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
  0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
  0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
  0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
  0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
  0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
  0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
  0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
  0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
  0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
  0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
  0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
  0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
  0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
  0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
  0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
  0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
  0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
  0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
  0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
  0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
  0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
  0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
  0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0,
};

struct THyALDuplicatesEntry {
	uint16_t m_u16SrcAddr;
	uint16_t m_u16MsduLen;
	uint16_t m_u16Crc;
	uint8_t m_u8MediaType;
};

#define HYAL_DUPLICATES_TABLE_SIZE   3

static struct THyALDuplicatesEntry hyALDuplicatesTable[HYAL_DUPLICATES_TABLE_SIZE] = {{0}};

struct THyALDataReq {
  struct TMcpsDataRequest m_sDataReqParameters;
  enum EMacWrpMediaTypeRequest m_eDataReqMediaType;
  uint8_t m_au8BackupBuffer[HYAL_BACKUP_BUF_SIZE];
  enum EMacStatus m_eFirstConfirmStatus;
  uint8_t m_u8ProbingInterval;
  bool bWaitingSecondConfirm;
  bool bUsed;
};

#define HYAL_DATA_REQ_QUEUE_SIZE   2

struct THyALData {
  // Data Service Control
  struct THyALDataReq m_DataReqQueue[HYAL_DATA_REQ_QUEUE_SIZE];
  enum EMacStatus m_eFirstScanConfirmStatus;
  bool bWaitingSecondScanConfirm;
  enum EMacStatus m_eFirstResetConfirmStatus;
  bool bWaitingSecondResetConfirm;
  enum EMacStatus m_eFirstStartConfirmStatus;
  bool bWaitingSecondStartConfirm;
  bool bMediaProbing;
};

static const struct THyALData g_HyALDefaults = {
  {{{{MAC_ADDRESS_MODE_NO_ADDRESS}}}}, // m_DataReqQueue
  MAC_STATUS_SUCCESS, // m_eFirstScanConfirmStatus
  false, // bWaitingSecondScanConfirm
  MAC_STATUS_SUCCESS, // m_eFirstResetConfirmStatus
  false, // bWaitingSecondResetConfirm
  MAC_STATUS_SUCCESS, // m_eFirstStartConfirmStatus
  false, // bWaitingSecondStartConfirm
  false, // bMediaProbing
};

static struct THyALData g_HyAL;

static uint16_t _HyALCrc16(const uint8_t *pu8Data, uint32_t u32Length)
{
	uint16_t u16Crc = 0;

	// polynom(16): X16 + X12 + X5 + 1 = 0x1021
	while (u32Length--) {
		u16Crc = crc16_tab[(u16Crc >> 8) ^ (*pu8Data ++)] ^ ((u16Crc & 0xFF) << 8);
	}
	return u16Crc;
}

static bool _checkDuplicates(uint16_t u16SrcAddr, uint8_t *pMsdu, uint16_t u16MsduLen, uint8_t u8MediaType)
{
	bool bDuplicate = false;
	uint8_t u8Index = 0;
	uint16_t u16Crc;

	// Calculate CRC for incoming frame
	u16Crc = _HyALCrc16(pMsdu, u16MsduLen);

	// Look for entry in the Duplicates Table
	struct THyALDuplicatesEntry *pEntry = &hyALDuplicatesTable[0];
	while (u8Index < HYAL_DUPLICATES_TABLE_SIZE) {
		// Look for same fields and different MediaType
		if ((pEntry->m_u16SrcAddr == u16SrcAddr) && (pEntry->m_u16MsduLen == u16MsduLen) &&
				(pEntry->m_u16Crc == u16Crc) && (pEntry->m_u8MediaType != u8MediaType)) {
			bDuplicate = true;
			break;
		}
		u8Index ++;
		pEntry ++;
	}

	if (!bDuplicate) {
		// Entry not found, store it
		memmove(&hyALDuplicatesTable[1], &hyALDuplicatesTable[0],
			(HYAL_DUPLICATES_TABLE_SIZE - 1) * sizeof(struct THyALDuplicatesEntry));
		// Populate the new entry.
		hyALDuplicatesTable[0].m_u16SrcAddr = u16SrcAddr;
		hyALDuplicatesTable[0].m_u16MsduLen = u16MsduLen;
		hyALDuplicatesTable[0].m_u16Crc = u16Crc;
		hyALDuplicatesTable[0].m_u8MediaType = u8MediaType;
	}

	// Return duplicate or not
	return bDuplicate;
}

static struct THyALDataReq *_getFreeDataReqEntry(void)
{
	uint8_t u8Idx;
	struct THyALDataReq *pFound = NULL;

	for (u8Idx = 0; u8Idx < HYAL_DATA_REQ_QUEUE_SIZE; u8Idx++) {
		if (!g_HyAL.m_DataReqQueue[u8Idx].bUsed) {
			pFound = &g_HyAL.m_DataReqQueue[u8Idx];
			g_HyAL.m_DataReqQueue[u8Idx].bUsed = true;
			LOG_INFO(Log("_getFreeDataReqEntry() Found free HyALDataReqQueueEntry on index %u", u8Idx));
			break;
		}
	}

	return pFound;
}

static struct THyALDataReq *_getDataReqEntryByHandler(uint8_t u8Handle)
{
	uint8_t u8Idx;
	struct THyALDataReq *pFound = NULL;

	for (u8Idx = 0; u8Idx < HYAL_DATA_REQ_QUEUE_SIZE; u8Idx++) {
		if (g_HyAL.m_DataReqQueue[u8Idx].bUsed &&
				(g_HyAL.m_DataReqQueue[u8Idx].m_sDataReqParameters.m_u8MsduHandle == u8Handle)) {
			pFound = &g_HyAL.m_DataReqQueue[u8Idx];
			LOG_INFO(Log("_getDataReqEntryByHandler() Found matching HyALDataReqQueueEntry on index %u, Handle: 0x%02X", u8Idx, u8Handle));
			break;
		}
	}

	return pFound;
}

static bool _IsAttributeInPLCRange(enum EMacWrpPibAttribute eAttribute)
{
	/* Check attribute ID range to distinguish between PLC and RF MAC */
	if (eAttribute < 0x00000200) {
		/* Standard PLC MAC IB */
		return true;
	}
	else if (eAttribute < 0x00000400) {
		/* Standard RF MAC IB */
		return false;
	}
	else if (eAttribute < 0x08000200) {
		/* Manufacturer PLC MAC IB */
		return true;
	}
	else {
		/* Manufacturer RF MAC IB */
		return false;
	}
}

static bool _checkRFMediaProbing(uint8_t u8ProbingInterval, struct TMacAddress dstAddress)
{
	struct TMacWrpPOSEntryRF posEntry;
	struct TMacPibValue pibValue;
	enum EMacWrpStatus status;
	uint8_t u8POSTableEntryTtl;
	uint8_t u8LqiValidTime;

	/* Probing interval has to be greater than 0 */
	if (u8ProbingInterval == 0) {
		return false;
	}

	/* Destination addressing has to be Short Addressing */
	if (dstAddress.m_eAddrMode != MAC_ADDRESS_MODE_SHORT) {
		return false;
	}

	/* Look for entry in RF POS Table */
	status = (enum EMacWrpStatus) MacMlmeGetRequestSyncRF(MAC_PIB_MANUF_POS_TABLE_ELEMENT_RF,
		dstAddress.m_nShortAddress, &pibValue);
	if (status == MAC_WRP_STATUS_SUCCESS) {
		(void)memcpy(&posEntry, pibValue.m_au8Value, sizeof(struct TMacWrpPOSEntryRF));
		status = (enum EMacWrpStatus) MacCommonGetRequestSync(MAC_COMMON_PIB_POS_TABLE_ENTRY_TTL,
			0, &pibValue);
		if (status == MAC_WRP_STATUS_SUCCESS) {
			u8POSTableEntryTtl = pibValue.m_au8Value[0];
			u8LqiValidTime = (posEntry.m_u16ReverseLqiValidTime + 59) / 60;
			if ((u8POSTableEntryTtl > u8LqiValidTime) &&
				((u8POSTableEntryTtl - u8LqiValidTime) >= u8ProbingInterval)) {
				/* Conditions met to perform the media probing */
				return true;
			}
		}
	}

	/* If this point is reached, no probing is done */
	return false;
}

static bool _checkPLCMediaProbing(uint8_t u8ProbingInterval, struct TMacAddress dstAddress)
{
	struct TMacWrpPOSEntry posEntry;
	struct TMacWrpNeighbourEntry nbEntry;
	struct TMacPibValue pibValue;
	enum EMacWrpStatus status;
	uint8_t u8TMRTtl;
	uint8_t u8TMRValidTime;

	/* Probing interval has to be greater than 0 */
	if (u8ProbingInterval == 0) {
		return false;
	}

	/* Destination addressing has to be Short Addressing */
	if (dstAddress.m_eAddrMode != MAC_ADDRESS_MODE_SHORT) {
		return false;
	}

	/* Look for entry in POS Table */
	status = (enum EMacWrpStatus) MacMlmeGetRequestSync(MAC_PIB_MANUF_POS_TABLE_ELEMENT,
		dstAddress.m_nShortAddress, &pibValue);
	if (status == MAC_WRP_STATUS_SUCCESS) {
		(void)memcpy(&posEntry, pibValue.m_au8Value, sizeof(struct TMacWrpPOSEntry));
		/* Look for entry in Neighbour Table to fill TMR Valid time */
		u8TMRValidTime = 0;
		status = (enum EMacWrpStatus) MacMlmeGetRequestSync(MAC_PIB_MANUF_NEIGHBOUR_TABLE_ELEMENT,
			dstAddress.m_nShortAddress, &pibValue);
		if (status == MAC_WRP_STATUS_SUCCESS) {
			(void)memcpy(&nbEntry, pibValue.m_au8Value, sizeof(struct TMacWrpNeighbourEntry));
			u8TMRValidTime = (nbEntry.m_u16TmrValidTime + 59) / 60;
		}
		status = (enum EMacWrpStatus) MacMlmeGetRequestSync(MAC_PIB_TMR_TTL, 0, &pibValue);
		if (status == MAC_WRP_STATUS_SUCCESS) {
			u8TMRTtl = pibValue.m_au8Value[0];
			if ((u8TMRTtl > u8TMRValidTime) &&
				((u8TMRTtl - u8TMRValidTime) >= u8ProbingInterval)) {
				/* Conditions met to perform the media probing */
				/* Reset TMR TTL for entry before probing, so TMR is exchanged */
				/* (pibValue is not used, same variable as prevoius can be used) */
				(void)MacMlmeSetRequestSync(MAC_PIB_MANUF_RESET_TMR_TTL, dstAddress.m_nShortAddress, &pibValue);
				return true;
			}
		}
	}

	/* If this point is reached, no probing is done */
	return false;
}

#endif // defined(__PLC_MAC__) && defined(__RF_MAC__)

static bool _IsSharedAttribute(enum EMacWrpPibAttribute eAttribute)
{
	/* Check if attribute in the list of shared between MAC layers */
	if ((eAttribute == MAC_WRP_PIB_MANUF_EXTENDED_ADDRESS) ||
		(eAttribute == MAC_WRP_PIB_PAN_ID) ||
		(eAttribute == MAC_WRP_PIB_PROMISCUOUS_MODE) ||
		(eAttribute == MAC_WRP_PIB_SHORT_ADDRESS) ||
		(eAttribute == MAC_WRP_PIB_POS_TABLE_ENTRY_TTL) ||
		(eAttribute == MAC_WRP_PIB_POS_RECENT_ENTRY_THRESHOLD) ||
		(eAttribute == MAC_WRP_PIB_RC_COORD) ||
		(eAttribute == MAC_WRP_PIB_KEY_TABLE)) {
		/* Shared IB */
		return true;
	}
	else {
		/* Non-shared IB */
		return false;
	}
}

/* ------------------------------------------------ */
/* ------------ Mac Wrapper callbacks ------------- */
/* ------------------------------------------------ */

#if defined(__PLC_MAC__)

static void _Callback_MacMcpsDataConfirm(struct TMcpsDataConfirm *pParameters)
{
	LOG_INFO(Log("_Callback_MacMcpsDataConfirm() Handle: 0x%02X Status: %u", pParameters->m_u8MsduHandle, (uint8_t)pParameters->m_eStatus));

	struct TMacWrpDataConfirm dataConfirm;

#if defined(__PLC_MAC__) && defined(__RF_MAC__)
	struct THyALDataReq *pMatchingDataReq;
	struct TMacPibValue pibValue;
	enum EMacWrpStatus status;

	/* Get Data Request entry matching confirm */
	pMatchingDataReq = _getDataReqEntryByHandler(pParameters->m_u8MsduHandle);

	/* Avoid unmached handling */
	if (pMatchingDataReq == NULL) {
		LOG_ERR(Log("_Callback_MacMcpsDataConfirm() Confirm does not match any previous request!!"));
		return;
	}

	/* Copy parameters from Mac. Media Type will be filled later */
	memcpy(&dataConfirm, pParameters, sizeof(struct TMcpsDataConfirm));

	switch (pMatchingDataReq->m_eDataReqMediaType) {
		case MAC_WRP_MEDIA_TYPE_REQ_PLC_BACKUP_RF:
			if (pParameters->m_eStatus == MAC_STATUS_SUCCESS) {
				/* Check Media Probing */
				if (_checkRFMediaProbing(pMatchingDataReq->m_u8ProbingInterval,
					pMatchingDataReq->m_sDataReqParameters.m_DstAddr)) {
					/* Perform Media probing */
					g_HyAL.bMediaProbing = true;
					LOG_INFO(Log("RF Media Probing"));
					/* Set Msdu pointer to backup buffer, as current pointer is no longer valid */
					pMatchingDataReq->m_sDataReqParameters.m_pMsdu = pMatchingDataReq->m_au8BackupBuffer;
					MacMcpsDataRequestRF(&pMatchingDataReq->m_sDataReqParameters);
				}
				else {
					g_HyAL.bMediaProbing = false;
					/* Send confirm to upper layer */
					dataConfirm.m_eMediaType = MAC_WRP_MEDIA_TYPE_CONF_PLC;
					/* Release Data Req entry and send confirm */
					pMatchingDataReq->bUsed = false;
					if (sUpperLayerNotifications.m_MacWrpDataConfirm != NULL) {
						sUpperLayerNotifications.m_MacWrpDataConfirm(&dataConfirm);
					}
				}
			}
			else {
				/* Check Dest Address mode and/or RF POS table before attempting data request */
				if (pMatchingDataReq->m_sDataReqParameters.m_DstAddr.m_eAddrMode == MAC_ADDRESS_MODE_EXTENDED) {
					status = MAC_WRP_STATUS_SUCCESS;
					LOG_INFO(Log("Extended Address Dest allows backup medium"));
				}
				else {
					LOG_INFO(Log("Look for RF POS Table entry for %0004X", pMatchingDataReq->m_sDataReqParameters.m_DstAddr.m_nShortAddress));
					status = (enum EMacWrpStatus) MacMlmeGetRequestSyncRF(MAC_PIB_MANUF_POS_TABLE_ELEMENT_RF,
							pMatchingDataReq->m_sDataReqParameters.m_DstAddr.m_nShortAddress, &pibValue);
				}
				/* Check status to try backup medium */
				if (status == MAC_WRP_STATUS_SUCCESS) {
					/* Try on backup medium */
					LOG_INFO(Log("Try RF as Backup Medium"));
					/* Set Msdu pointer to backup buffer, as current pointer is no longer valid */
					pMatchingDataReq->m_sDataReqParameters.m_pMsdu = pMatchingDataReq->m_au8BackupBuffer;
					MacMcpsDataRequestRF(&pMatchingDataReq->m_sDataReqParameters);
				}
				else {
					LOG_INFO(Log("No POS entry found, discard backup medium"));
					/* Send confirm to upper layer */
					dataConfirm.m_eMediaType = MAC_WRP_MEDIA_TYPE_CONF_PLC;
					/* Release Data Req entry and send confirm */
					pMatchingDataReq->bUsed = false;
					if (sUpperLayerNotifications.m_MacWrpDataConfirm != NULL) {
						sUpperLayerNotifications.m_MacWrpDataConfirm(&dataConfirm);
					}
				}
			}
			break;
		case MAC_WRP_MEDIA_TYPE_REQ_RF_BACKUP_PLC:
			if (g_HyAL.bMediaProbing) {
				/* PLC was probed after RF success. Send confirm to upper layer */
				g_HyAL.bMediaProbing = false;
				dataConfirm.m_eStatus = MAC_WRP_STATUS_SUCCESS;
				if (pParameters->m_eStatus == MAC_STATUS_SUCCESS) {
					dataConfirm.m_eMediaType = MAC_WRP_MEDIA_TYPE_CONF_RF;
				}
				else {
					dataConfirm.m_eMediaType = MAC_WRP_MEDIA_TYPE_CONF_RF_AS_BACKUP;
				}
				/* Release Data Req entry and send confirm */
				pMatchingDataReq->bUsed = false;
				if (sUpperLayerNotifications.m_MacWrpDataConfirm != NULL) {
					sUpperLayerNotifications.m_MacWrpDataConfirm(&dataConfirm);
				}
			}
			else {
				/* PLC was used as backup medium. Send confirm to upper layer */
				dataConfirm.m_eMediaType = MAC_WRP_MEDIA_TYPE_CONF_PLC_AS_BACKUP;
				/* Release Data Req entry and send confirm */
				pMatchingDataReq->bUsed = false;
				if (sUpperLayerNotifications.m_MacWrpDataConfirm != NULL) {
					sUpperLayerNotifications.m_MacWrpDataConfirm(&dataConfirm);
				}
			}
			break;
		case MAC_WRP_MEDIA_TYPE_REQ_BOTH:
			if (pMatchingDataReq->bWaitingSecondConfirm) {
				/* Second Confirm arrived. Send confirm to upper layer depending on results */
				dataConfirm.m_eMediaType = MAC_WRP_MEDIA_TYPE_CONF_BOTH;
				if ((pMatchingDataReq->m_eFirstConfirmStatus == MAC_STATUS_SUCCESS) ||
						(pParameters->m_eStatus == MAC_STATUS_SUCCESS)) {
					/* At least one SUCCESS, send confirm with SUCCESS */
					dataConfirm.m_eStatus = MAC_WRP_STATUS_SUCCESS;
					/* Release Data Req entry and send confirm */
					pMatchingDataReq->bUsed = false;
					if (sUpperLayerNotifications.m_MacWrpDataConfirm != NULL) {
						sUpperLayerNotifications.m_MacWrpDataConfirm(&dataConfirm);
					}
				}
				else {
					/* None SUCCESS. Return result from second confirm */
					dataConfirm.m_eStatus = (enum EMacWrpStatus)pParameters->m_eStatus;
					/* Release Data Req entry and send confirm */
					pMatchingDataReq->bUsed = false;
					if (sUpperLayerNotifications.m_MacWrpDataConfirm != NULL) {
						sUpperLayerNotifications.m_MacWrpDataConfirm(&dataConfirm);
					}
				}
			}
			else {
				/* This is the First Confirm, store status and wait for Second */
				pMatchingDataReq->m_eFirstConfirmStatus = pParameters->m_eStatus;
				pMatchingDataReq->bWaitingSecondConfirm = true;
			}
			break;
		case MAC_WRP_MEDIA_TYPE_REQ_PLC_NO_BACKUP:
			/* Send confirm to upper layer */
			dataConfirm.m_eMediaType = MAC_WRP_MEDIA_TYPE_CONF_PLC;
			/* Release Data Req entry and send confirm */
			pMatchingDataReq->bUsed = false;
			if (sUpperLayerNotifications.m_MacWrpDataConfirm != NULL) {
				sUpperLayerNotifications.m_MacWrpDataConfirm(&dataConfirm);
			}
			break;
		case MAC_WRP_MEDIA_TYPE_REQ_RF_NO_BACKUP:
			/* PLC confirm not expected on RF_NO_BACKUP request. Ignore it */
			LOG_ERR(Log("_Callback_MacMcpsDataConfirm() called from a MEDIA_TYPE_REQ_RF_NO_BACKUP request!!"));
			break;
		default: /* PLC only */
			dataConfirm.m_eMediaType = MAC_WRP_MEDIA_TYPE_CONF_PLC;
			/* Release Data Req entry and send confirm */
			pMatchingDataReq->bUsed = false;
			if (sUpperLayerNotifications.m_MacWrpDataConfirm != NULL) {
				sUpperLayerNotifications.m_MacWrpDataConfirm(&dataConfirm);
			}
			break;
	}
#else
	/* Copy parameters from Mac and fill Media Type */
	memcpy(&dataConfirm, pParameters, sizeof(struct TMcpsDataConfirm));
	dataConfirm.m_eMediaType = MAC_WRP_MEDIA_TYPE_CONF_PLC;
	if (sUpperLayerNotifications.m_MacWrpDataConfirm != NULL) {
		sUpperLayerNotifications.m_MacWrpDataConfirm(&dataConfirm);
	}
#endif
}

#endif

#if defined(__RF_MAC__)

static void _Callback_MacMcpsDataConfirmRF(struct TMcpsDataConfirm *pParameters)
{
	LOG_INFO(Log("_Callback_MacMcpsDataConfirmRF() Handle: 0x%02X Status: %u", pParameters->m_u8MsduHandle, (uint8_t)pParameters->m_eStatus));

	struct TMacWrpDataConfirm dataConfirm;

#if defined(__PLC_MAC__) && defined(__RF_MAC__)
	struct THyALDataReq *pMatchingDataReq;
	struct TMacPibValue pibValue;
	enum EMacWrpStatus status;

	/* Get Data Request entry matching confirm */
	pMatchingDataReq = _getDataReqEntryByHandler(pParameters->m_u8MsduHandle);

	/* Avoid unmached handling */
	if (pMatchingDataReq == NULL) {
		LOG_ERR(Log("_Callback_MacMcpsDataConfirmRF() Confirm does not match any previous request!!"));
		return;
	}

	/* Copy parameters from Mac. Media Type will be filled later */
	memcpy(&dataConfirm, pParameters, sizeof(struct TMcpsDataConfirm));

	switch (pMatchingDataReq->m_eDataReqMediaType) {
		case MAC_WRP_MEDIA_TYPE_REQ_PLC_BACKUP_RF:
			if (g_HyAL.bMediaProbing) {
				/* RF was probed after PLC success. Send confirm to upper layer */
				g_HyAL.bMediaProbing = false;
				dataConfirm.m_eStatus = MAC_WRP_STATUS_SUCCESS;
				if (pParameters->m_eStatus == MAC_STATUS_SUCCESS) {
					dataConfirm.m_eMediaType = MAC_WRP_MEDIA_TYPE_CONF_PLC;
				}
				else {
					dataConfirm.m_eMediaType = MAC_WRP_MEDIA_TYPE_CONF_PLC_AS_BACKUP;
				}
				/* Release Data Req entry and send confirm */
				pMatchingDataReq->bUsed = false;
				if (sUpperLayerNotifications.m_MacWrpDataConfirm != NULL) {
					sUpperLayerNotifications.m_MacWrpDataConfirm(&dataConfirm);
				}
			}
			else {
				/* RF was used as backup medium. Send confirm to upper layer */
				dataConfirm.m_eMediaType = MAC_WRP_MEDIA_TYPE_CONF_RF_AS_BACKUP;
				/* Release Data Req entry and send confirm */
				pMatchingDataReq->bUsed = false;
				if (sUpperLayerNotifications.m_MacWrpDataConfirm != NULL) {
					sUpperLayerNotifications.m_MacWrpDataConfirm(&dataConfirm);
				}
			}
			break;
		case MAC_WRP_MEDIA_TYPE_REQ_RF_BACKUP_PLC:
			if (pParameters->m_eStatus == MAC_STATUS_SUCCESS) {
				/* Check Media Probing */
				if (_checkPLCMediaProbing(pMatchingDataReq->m_u8ProbingInterval,
					pMatchingDataReq->m_sDataReqParameters.m_DstAddr)) {
					/* Perform Media probing */
					g_HyAL.bMediaProbing = true;
					LOG_INFO(Log("PLC Media Probing"));
					/* Set Msdu pointer to backup buffer, as current pointer is no longer valid */
					pMatchingDataReq->m_sDataReqParameters.m_pMsdu = pMatchingDataReq->m_au8BackupBuffer;
					MacMcpsDataRequest(&pMatchingDataReq->m_sDataReqParameters);
				}
				else {
					g_HyAL.bMediaProbing = false;
					/* Send confirm to upper layer */
					dataConfirm.m_eMediaType = MAC_WRP_MEDIA_TYPE_CONF_RF;
					/* Release Data Req entry and send confirm */
					pMatchingDataReq->bUsed = false;
					if (sUpperLayerNotifications.m_MacWrpDataConfirm != NULL) {
						sUpperLayerNotifications.m_MacWrpDataConfirm(&dataConfirm);
					}
				}
			}
			else {
				/* Check Dest Address mode and/or RF POS table before attempting data request */
				if (pMatchingDataReq->m_sDataReqParameters.m_DstAddr.m_eAddrMode == MAC_ADDRESS_MODE_EXTENDED) {
					status = MAC_WRP_STATUS_SUCCESS;
					LOG_INFO(Log("Extended Address Dest allows backup medium"));
				}
				else {
					LOG_INFO(Log("Look for PLC POS Table entry for %0004X", pMatchingDataReq->m_sDataReqParameters.m_DstAddr.m_nShortAddress));
					status = (enum EMacWrpStatus) MacMlmeGetRequestSync(MAC_PIB_MANUF_POS_TABLE_ELEMENT,
							pMatchingDataReq->m_sDataReqParameters.m_DstAddr.m_nShortAddress, &pibValue);
				}
				/* Check status to try backup medium */
				if (status == MAC_WRP_STATUS_SUCCESS) {
					/* Try on backup medium */
					LOG_INFO(Log("Try PLC as Backup Meduim"));
					/* Set Msdu pointer to backup buffer, as current pointer is no longer valid */
					pMatchingDataReq->m_sDataReqParameters.m_pMsdu = pMatchingDataReq->m_au8BackupBuffer;
					MacMcpsDataRequest(&pMatchingDataReq->m_sDataReqParameters);
				}
				else {
					LOG_INFO(Log("No POS entry found, discard backup medium"));
					/* Send confirm to upper layer */
					dataConfirm.m_eMediaType = MAC_WRP_MEDIA_TYPE_CONF_RF;
					/* Release Data Req entry and send confirm */
					pMatchingDataReq->bUsed = false;
					if (sUpperLayerNotifications.m_MacWrpDataConfirm != NULL) {
						sUpperLayerNotifications.m_MacWrpDataConfirm(&dataConfirm);
					}
				}
			}
			break;
		case MAC_WRP_MEDIA_TYPE_REQ_BOTH:
			if (pMatchingDataReq->bWaitingSecondConfirm) {
				/* Second Confirm arrived. Send confirm to upper layer depending on results */
				dataConfirm.m_eMediaType = MAC_WRP_MEDIA_TYPE_CONF_BOTH;
				if ((pMatchingDataReq->m_eFirstConfirmStatus == MAC_STATUS_SUCCESS) ||
						(pParameters->m_eStatus == MAC_STATUS_SUCCESS)) {
					/* At least one SUCCESS, send confirm with SUCCESS */
					dataConfirm.m_eStatus = MAC_WRP_STATUS_SUCCESS;
					/* Release Data Req entry and send confirm */
					pMatchingDataReq->bUsed = false;
					if (sUpperLayerNotifications.m_MacWrpDataConfirm != NULL) {
						sUpperLayerNotifications.m_MacWrpDataConfirm(&dataConfirm);
					}
				}
				else {
					/* None SUCCESS. Return result from second confirm */
					dataConfirm.m_eStatus = (enum EMacWrpStatus)pParameters->m_eStatus;
					/* Release Data Req entry and send confirm */
					pMatchingDataReq->bUsed = false;
					if (sUpperLayerNotifications.m_MacWrpDataConfirm != NULL) {
						sUpperLayerNotifications.m_MacWrpDataConfirm(&dataConfirm);
					}
				}
			}
			else {
				/* This is the First Confirm, store status and wait for Second */
				pMatchingDataReq->m_eFirstConfirmStatus = pParameters->m_eStatus;
				pMatchingDataReq->bWaitingSecondConfirm = true;
			}
			break;
		case MAC_WRP_MEDIA_TYPE_REQ_PLC_NO_BACKUP:
			/* RF confirm not expected after a PLC_NO_BACKUP request. Ignore it */
			LOG_ERR(Log("_Callback_MacMcpsDataConfirm() called from a MEDIA_TYPE_REQ_PLC_NO_BACKUP request!!"));
			break;
		case MAC_WRP_MEDIA_TYPE_REQ_RF_NO_BACKUP:
			/* Send confirm to upper layer */
			dataConfirm.m_eMediaType = MAC_WRP_MEDIA_TYPE_CONF_RF;
			/* Release Data Req entry and send confirm */
			pMatchingDataReq->bUsed = false;
			if (sUpperLayerNotifications.m_MacWrpDataConfirm != NULL) {
				sUpperLayerNotifications.m_MacWrpDataConfirm(&dataConfirm);
			}
			break;
		default: /* RF only */
			dataConfirm.m_eMediaType = MAC_WRP_MEDIA_TYPE_CONF_RF;
			/* Release Data Req entry and send confirm */
			pMatchingDataReq->bUsed = false;
			if (sUpperLayerNotifications.m_MacWrpDataConfirm != NULL) {
				sUpperLayerNotifications.m_MacWrpDataConfirm(&dataConfirm);
			}
			break;
	}
#else
	/* Copy parameters from Mac and fill Media Type */
	memcpy(&dataConfirm, pParameters, sizeof(struct TMcpsDataConfirm));
	dataConfirm.m_eMediaType = MAC_WRP_MEDIA_TYPE_CONF_RF;
	if (sUpperLayerNotifications.m_MacWrpDataConfirm != NULL) {
		sUpperLayerNotifications.m_MacWrpDataConfirm(&dataConfirm);
	}
#endif
}

#endif

#if defined(__PLC_MAC__)

static void _Callback_MacMcpsDataIndication(struct TMcpsDataIndication *pParameters)
{
	LOG_INFO(Log("_Callback_MacMcpsDataIndication"));
	struct TMacWrpDataIndication dataIndication;

#if defined(__PLC_MAC__) && defined(__RF_MAC__)
	/* Check if the same frame has been received on the other medium (duplicate detection), except for broadcast */
	if (MAC_SHORT_ADDRESS_BROADCAST != pParameters->m_DstAddr.m_nShortAddress) {
		if (_checkDuplicates(pParameters->m_SrcAddr.m_nShortAddress, pParameters->m_pMsdu,
			pParameters->m_u16MsduLength, MAC_WRP_MEDIA_TYPE_IND_PLC)) {
			/* Same frame was received on RF medium. Drop indication */
			LOG_INFO(Log("Same frame was received on RF medium. Drop indication"));
			return;
		}
	}
#endif

	/* Copy parameters from Mac. Media Type will be filled later */
	memcpy(&dataIndication, pParameters, sizeof(struct TMcpsDataIndication));

	if (sUpperLayerNotifications.m_MacWrpDataIndication != NULL) {
		dataIndication.m_eMediaType = MAC_WRP_MEDIA_TYPE_IND_PLC;
		sUpperLayerNotifications.m_MacWrpDataIndication(&dataIndication);
	}
}

#endif

#if defined(__RF_MAC__)

static void _Callback_MacMcpsDataIndicationRF(struct TMcpsDataIndication *pParameters)
{
	LOG_INFO(Log("_Callback_MacMcpsDataIndicationRF"));
	struct TMacWrpDataIndication dataIndication;

#if defined(__PLC_MAC__) && defined(__RF_MAC__)
	/* Check if the same frame has been received on the other medium (duplicate detection), except for broadcast */
	if (MAC_SHORT_ADDRESS_BROADCAST != pParameters->m_DstAddr.m_nShortAddress) {
		if (_checkDuplicates(pParameters->m_SrcAddr.m_nShortAddress, pParameters->m_pMsdu,
			pParameters->m_u16MsduLength, MAC_WRP_MEDIA_TYPE_IND_RF)) {
			/* Same frame was received on PLC medium. Drop indication */
			LOG_INFO(Log("Same frame was received on PLC medium. Drop indication"));
			return;
		}
	}
#endif

	/* Copy parameters from Mac. Media Type will be filled later */
	memcpy(&dataIndication, pParameters, sizeof(struct TMcpsDataIndication));

	if (sUpperLayerNotifications.m_MacWrpDataIndication != NULL) {
		dataIndication.m_eMediaType = MAC_WRP_MEDIA_TYPE_IND_RF;
		sUpperLayerNotifications.m_MacWrpDataIndication(&dataIndication);
	}
}

#endif

#if defined(__PLC_MAC__)

static void _Callback_MacMlmeResetConfirm(struct TMlmeResetConfirm *pParameters)
{
	LOG_DBG(Log("_Callback_MacMlmeResetConfirm: Status: %u", pParameters->m_eStatus));

#if defined(__PLC_MAC__) && defined(__RF_MAC__)
	struct TMacWrpResetConfirm resetConfirm;

	if (g_HyAL.bWaitingSecondResetConfirm) {
		/* Second Confirm arrived. Send confirm to upper layer depending on results */
		if ((g_HyAL.m_eFirstResetConfirmStatus == MAC_STATUS_SUCCESS) &&
				(pParameters->m_eStatus == MAC_STATUS_SUCCESS)) {
			/* Both SUCCESS, send confirm with SUCCESS */
			resetConfirm.m_eStatus = MAC_WRP_STATUS_SUCCESS;
			if (sUpperLayerNotifications.m_MacWrpResetConfirm != NULL) {
				sUpperLayerNotifications.m_MacWrpResetConfirm(&resetConfirm);
			}
		}
		else {
			/* Check which reset failed and report its status */
			if (g_HyAL.m_eFirstResetConfirmStatus != MAC_STATUS_SUCCESS) {
				resetConfirm.m_eStatus = (enum EMacWrpStatus)g_HyAL.m_eFirstResetConfirmStatus;
				if (sUpperLayerNotifications.m_MacWrpResetConfirm != NULL) {
					sUpperLayerNotifications.m_MacWrpResetConfirm(&resetConfirm);
				}
			}
			else {
				resetConfirm.m_eStatus = (enum EMacWrpStatus)pParameters->m_eStatus;
				if (sUpperLayerNotifications.m_MacWrpResetConfirm != NULL) {
					sUpperLayerNotifications.m_MacWrpResetConfirm(&resetConfirm);
				}
			}
		}
	}
	else {
		/* This is the First Confirm, store status and wait for Second */
		g_HyAL.m_eFirstResetConfirmStatus = pParameters->m_eStatus;
		g_HyAL.bWaitingSecondResetConfirm = true;
	}
#else
	if (sUpperLayerNotifications.m_MacWrpResetConfirm != NULL) {
		sUpperLayerNotifications.m_MacWrpResetConfirm((struct TMacWrpResetConfirm *)pParameters);
	}
#endif
}

#endif

#if defined(__RF_MAC__)

static void _Callback_MacMlmeResetConfirmRF(struct TMlmeResetConfirm *pParameters)
{
	LOG_DBG(Log("_Callback_MacMlmeResetConfirmRF: Status: %u", pParameters->m_eStatus));

#if defined(__PLC_MAC__) && defined(__RF_MAC__)
	struct TMacWrpResetConfirm resetConfirm;

	if (g_HyAL.bWaitingSecondResetConfirm) {
		/* Second Confirm arrived. Send confirm to upper layer depending on results */
		if ((g_HyAL.m_eFirstResetConfirmStatus == MAC_STATUS_SUCCESS) &&
				(pParameters->m_eStatus == MAC_STATUS_SUCCESS)) {
			/* Both SUCCESS, send confirm with SUCCESS */
			resetConfirm.m_eStatus = MAC_WRP_STATUS_SUCCESS;
			if (sUpperLayerNotifications.m_MacWrpResetConfirm != NULL) {
				sUpperLayerNotifications.m_MacWrpResetConfirm(&resetConfirm);
			}
		}
		else {
			/* Check which reset failed and report its status */
			if (g_HyAL.m_eFirstResetConfirmStatus != MAC_STATUS_SUCCESS) {
				resetConfirm.m_eStatus = (enum EMacWrpStatus)g_HyAL.m_eFirstResetConfirmStatus;
				if (sUpperLayerNotifications.m_MacWrpResetConfirm != NULL) {
					sUpperLayerNotifications.m_MacWrpResetConfirm(&resetConfirm);
				}
			}
			else {
				resetConfirm.m_eStatus = (enum EMacWrpStatus)pParameters->m_eStatus;
				if (sUpperLayerNotifications.m_MacWrpResetConfirm != NULL) {
					sUpperLayerNotifications.m_MacWrpResetConfirm(&resetConfirm);
				}
			}
		}
	}
	else {
		/* This is the First Confirm, store status and wait for Second */
		g_HyAL.m_eFirstResetConfirmStatus = pParameters->m_eStatus;
		g_HyAL.bWaitingSecondResetConfirm = true;
	}
#else
	if (sUpperLayerNotifications.m_MacWrpResetConfirm != NULL) {
		sUpperLayerNotifications.m_MacWrpResetConfirm((struct TMacWrpResetConfirm *)pParameters);
	}
#endif
}

#endif

#if defined(__PLC_MAC__)

static void _Callback_MacMlmeBeaconNotify(struct TMlmeBeaconNotifyIndication *pParameters)
{
	LOG_INFO(Log("_Callback_MacMlmeBeaconNotify: Pan ID: %04X", pParameters->m_PanDescriptor.m_nPanId));

	struct TMacWrpBeaconNotifyIndication notifyIndication;

	/* Copy parameters from Mac. Media Type will be filled later */
	memcpy(&notifyIndication, pParameters, sizeof(struct TMlmeBeaconNotifyIndication));

	if (sUpperLayerNotifications.m_MacWrpBeaconNotifyIndication != NULL) {
		notifyIndication.m_PanDescriptor.m_eMediaType = MAC_WRP_MEDIA_TYPE_IND_PLC;
		sUpperLayerNotifications.m_MacWrpBeaconNotifyIndication(&notifyIndication);
	}
}

#endif

#if defined(__RF_MAC__)

static void _Callback_MacMlmeBeaconNotifyRF(struct TMlmeBeaconNotifyIndication *pParameters)
{
	LOG_INFO(Log("_Callback_MacMlmeBeaconNotifyRF: Pan ID: %04X", pParameters->m_PanDescriptor.m_nPanId));

	struct TMacWrpBeaconNotifyIndication notifyIndication;

	/* Copy parameters from Mac. Media Type will be filled later */
	memcpy(&notifyIndication, pParameters, sizeof(struct TMlmeBeaconNotifyIndication));

	if (sUpperLayerNotifications.m_MacWrpBeaconNotifyIndication != NULL) {
		notifyIndication.m_PanDescriptor.m_eMediaType = MAC_WRP_MEDIA_TYPE_IND_RF;
		sUpperLayerNotifications.m_MacWrpBeaconNotifyIndication(&notifyIndication);
	}
}

#endif

#if defined(__PLC_MAC__)

static void _Callback_MacMlmeScanConfirm(struct TMlmeScanConfirm *pParameters)
{
	LOG_INFO(Log("_Callback_MacMlmeScanConfirm: Status: %u", pParameters->m_eStatus));

#if defined(__PLC_MAC__) && defined(__RF_MAC__)
	struct TMacWrpScanConfirm scanConfirm;

	if (g_HyAL.bWaitingSecondScanConfirm) {
		/* Second Confirm arrived. Send confirm to upper layer depending on results */
		if ((g_HyAL.m_eFirstScanConfirmStatus == MAC_STATUS_SUCCESS) ||
				(pParameters->m_eStatus == MAC_STATUS_SUCCESS)) {
			/* One or Both SUCCESS, send confirm with SUCCESS */
			scanConfirm.m_eStatus = MAC_WRP_STATUS_SUCCESS;
			if (sUpperLayerNotifications.m_MacWrpScanConfirm != NULL) {
				sUpperLayerNotifications.m_MacWrpScanConfirm(&scanConfirm);
			}
		}
		else {
			/* None of confirms SUCCESS, send confirm with latests status */
			scanConfirm.m_eStatus = (enum EMacWrpStatus)pParameters->m_eStatus;
			if (sUpperLayerNotifications.m_MacWrpScanConfirm != NULL) {
				sUpperLayerNotifications.m_MacWrpScanConfirm(&scanConfirm);
			}
		}
	}
	else {
		/* This is the First Confirm, store status and wait for Second */
		g_HyAL.m_eFirstScanConfirmStatus = pParameters->m_eStatus;
		g_HyAL.bWaitingSecondScanConfirm = true;
	}
#else
	if (sUpperLayerNotifications.m_MacWrpScanConfirm != NULL) {
		sUpperLayerNotifications.m_MacWrpScanConfirm((struct TMacWrpScanConfirm *)pParameters);
	}
#endif
}

#endif

#if defined(__RF_MAC__)

static void _Callback_MacMlmeScanConfirmRF(struct TMlmeScanConfirm *pParameters)
{
	LOG_INFO(Log("_Callback_MacMlmeScanConfirmRF: Status: %u", pParameters->m_eStatus));

#if defined(__PLC_MAC__) && defined(__RF_MAC__)
	struct TMacWrpScanConfirm scanConfirm;

	if (g_HyAL.bWaitingSecondScanConfirm) {
		/* Second Confirm arrived. Send confirm to upper layer depending on results */
		if ((g_HyAL.m_eFirstScanConfirmStatus == MAC_STATUS_SUCCESS) ||
				(pParameters->m_eStatus == MAC_STATUS_SUCCESS)) {
			/* One or Both SUCCESS, send confirm with SUCCESS */
			scanConfirm.m_eStatus = MAC_WRP_STATUS_SUCCESS;
			if (sUpperLayerNotifications.m_MacWrpScanConfirm != NULL) {
				sUpperLayerNotifications.m_MacWrpScanConfirm(&scanConfirm);
			}
		}
		else {
			/* None of confirms SUCCESS, send confirm with latests status */
			scanConfirm.m_eStatus = (enum EMacWrpStatus)pParameters->m_eStatus;
			if (sUpperLayerNotifications.m_MacWrpScanConfirm != NULL) {
				sUpperLayerNotifications.m_MacWrpScanConfirm(&scanConfirm);
			}
		}
	}
	else {
		/* This is the First Confirm, store status and wait for Second */
		g_HyAL.m_eFirstScanConfirmStatus = pParameters->m_eStatus;
		g_HyAL.bWaitingSecondScanConfirm = true;
	}
#else
	if (sUpperLayerNotifications.m_MacWrpScanConfirm != NULL) {
		sUpperLayerNotifications.m_MacWrpScanConfirm((struct TMacWrpScanConfirm *)pParameters);
	}
#endif
}

#endif

#if defined(__PLC_MAC__)

static void _Callback_MacMlmeStartConfirm(struct TMlmeStartConfirm *pParameters)
{
	LOG_DBG(Log("_Callback_MacMlmeStartConfirm: Status: %u", pParameters->m_eStatus));

#if defined(__PLC_MAC__) && defined(__RF_MAC__)
	struct TMacWrpStartConfirm startConfirm;

	if (g_HyAL.bWaitingSecondStartConfirm) {
		/* Second Confirm arrived. Send confirm to upper layer depending on results */
		if ((g_HyAL.m_eFirstStartConfirmStatus == MAC_STATUS_SUCCESS) &&
				(pParameters->m_eStatus == MAC_STATUS_SUCCESS)) {
			/* Both SUCCESS, send confirm with SUCCESS */
			startConfirm.m_eStatus = MAC_WRP_STATUS_SUCCESS;
			if (sUpperLayerNotifications.m_MacWrpStartConfirm != NULL) {
				sUpperLayerNotifications.m_MacWrpStartConfirm(&startConfirm);
			}
		}
		else {
			/* Check which start failed and report its status */
			if (g_HyAL.m_eFirstStartConfirmStatus != MAC_STATUS_SUCCESS) {
				startConfirm.m_eStatus = (enum EMacWrpStatus)g_HyAL.m_eFirstStartConfirmStatus;
				if (sUpperLayerNotifications.m_MacWrpStartConfirm != NULL) {
					sUpperLayerNotifications.m_MacWrpStartConfirm(&startConfirm);
				}
			}
			else {
				startConfirm.m_eStatus = (enum EMacWrpStatus)pParameters->m_eStatus;
				if (sUpperLayerNotifications.m_MacWrpStartConfirm != NULL) {
					sUpperLayerNotifications.m_MacWrpStartConfirm(&startConfirm);
				}
			}
		}
	}
	else {
		/* This is the First Confirm, store status and wait for Second */
		g_HyAL.m_eFirstStartConfirmStatus = pParameters->m_eStatus;
		g_HyAL.bWaitingSecondStartConfirm = true;
	}
#else
	if (sUpperLayerNotifications.m_MacWrpStartConfirm != NULL) {
		sUpperLayerNotifications.m_MacWrpStartConfirm((struct TMacWrpStartConfirm *)pParameters);
	}
#endif
}

#endif

#if defined(__RF_MAC__)

static void _Callback_MacMlmeStartConfirmRF(struct TMlmeStartConfirm *pParameters)
{
	LOG_DBG(Log("_Callback_MacMlmeStartConfirmRF: Status: %u", pParameters->m_eStatus));

#if defined(__PLC_MAC__) && defined(__RF_MAC__)
	struct TMacWrpStartConfirm startConfirm;

	if (g_HyAL.bWaitingSecondStartConfirm) {
		/* Second Confirm arrived. Send confirm to upper layer depending on results */
		if ((g_HyAL.m_eFirstStartConfirmStatus == MAC_STATUS_SUCCESS) &&
				(pParameters->m_eStatus == MAC_STATUS_SUCCESS)) {
			/* Both SUCCESS, send confirm with SUCCESS */
			startConfirm.m_eStatus = MAC_WRP_STATUS_SUCCESS;
			if (sUpperLayerNotifications.m_MacWrpStartConfirm != NULL) {
				sUpperLayerNotifications.m_MacWrpStartConfirm(&startConfirm);
			}
		}
		else {
			/* Check which start failed and report its status */
			if (g_HyAL.m_eFirstStartConfirmStatus != MAC_STATUS_SUCCESS) {
				startConfirm.m_eStatus = (enum EMacWrpStatus)g_HyAL.m_eFirstStartConfirmStatus;
				if (sUpperLayerNotifications.m_MacWrpStartConfirm != NULL) {
					sUpperLayerNotifications.m_MacWrpStartConfirm(&startConfirm);
				}
			}
			else {
				startConfirm.m_eStatus = (enum EMacWrpStatus)pParameters->m_eStatus;
				if (sUpperLayerNotifications.m_MacWrpStartConfirm != NULL) {
					sUpperLayerNotifications.m_MacWrpStartConfirm(&startConfirm);
				}
			}
		}
	}
	else {
		/* This is the First Confirm, store status and wait for Second */
		g_HyAL.m_eFirstStartConfirmStatus = pParameters->m_eStatus;
		g_HyAL.bWaitingSecondStartConfirm = true;
	}
#else
	if (sUpperLayerNotifications.m_MacWrpStartConfirm != NULL) {
		sUpperLayerNotifications.m_MacWrpStartConfirm((struct TMacWrpStartConfirm *)pParameters);
	}
#endif
}

#endif

#if defined(__PLC_MAC__)

static void _Callback_MacMlmeCommStatusIndication(struct TMlmeCommStatusIndication *pParameters)
{
	LOG_DBG(Log("_Callback_MacMlmeCommStatusIndication: Status: %u", pParameters->m_eStatus));

	struct TMacWrpCommStatusIndication commStatusIndication;

	/* Copy parameters from Mac. Media Type will be filled later */
	memcpy(&commStatusIndication, pParameters, sizeof(struct TMlmeCommStatusIndication));

	if (sUpperLayerNotifications.m_MacWrpCommStatusIndication != NULL) {
		commStatusIndication.m_eMediaType = MAC_WRP_MEDIA_TYPE_IND_PLC;
		sUpperLayerNotifications.m_MacWrpCommStatusIndication(&commStatusIndication);
	}
}

#endif

#if defined(__RF_MAC__)

static void _Callback_MacMlmeCommStatusIndicationRF(struct TMlmeCommStatusIndication *pParameters)
{
	LOG_DBG(Log("_Callback_MacMlmeCommStatusIndicationRF: Status: %u", pParameters->m_eStatus));

	struct TMacWrpCommStatusIndication commStatusIndication;

	/* Copy parameters from Mac. Media Type will be filled later */
	memcpy(&commStatusIndication, pParameters, sizeof(struct TMlmeCommStatusIndication));

	if (sUpperLayerNotifications.m_MacWrpCommStatusIndication != NULL) {
		commStatusIndication.m_eMediaType = MAC_WRP_MEDIA_TYPE_IND_RF;
		sUpperLayerNotifications.m_MacWrpCommStatusIndication(&commStatusIndication);
	}
}

#endif

#if defined(__PLC_MAC__)

static void _Callback_MacMcpsMacSnifferIndication(struct TMcpsMacSnifferIndication *pParameters)
{
	LOG_DBG(LogBuffer(pParameters->m_pMsdu, pParameters->m_u16MsduLength, "_Callback_MacMcpsMacSnifferIndication:  MSDU:"));

	if (sUpperLayerNotifications.m_MacWrpSnifferIndication != NULL) {
		sUpperLayerNotifications.m_MacWrpSnifferIndication((struct TMacWrpSnifferIndication *)pParameters);
	}
}

#endif

#if defined(__RF_MAC__)

static void _Callback_MacMcpsMacSnifferIndicationRF(struct TMcpsMacSnifferIndication *pParameters)
{
	LOG_DBG(LogBuffer(pParameters->m_pMsdu, pParameters->m_u16MsduLength, "_Callback_MacMcpsMacSnifferIndicationRF:  MSDU:"));

	if (sUpperLayerNotificationsRF.m_MacWrpSnifferIndication != NULL) {
		sUpperLayerNotificationsRF.m_MacWrpSnifferIndication((struct TMacWrpSnifferIndication *)pParameters);
	}
}

#endif


/* ------------------------------------------------ */
/* ---------------- Mac Wrapper API --------------- */
/* ------------------------------------------------ */

void MacWrapperInitialize(struct TMacWrpNotifications *pNotifications, uint8_t u8PlcBand)
{
#if defined(__PLC_MAC__)
	struct TMacNotifications macNotificationsPLC;
#endif
#if defined(__RF_MAC__)
	struct TMacNotificationsRF macWrapperNotificationsRF;
#endif

	memcpy(&sUpperLayerNotifications, pNotifications, sizeof(struct TMacWrpNotifications));

#if defined(__PLC_MAC__) && defined(__RF_MAC__)
	// Set default HyAL variables
	g_HyAL = g_HyALDefaults;
	memset(hyALDuplicatesTable, 0, sizeof(hyALDuplicatesTable));
#endif

#if defined(__PLC_MAC__)
	LOG_INFO(Log("MacWrapperInitialize: Initializing PLC MAC..."));

	macNotificationsPLC.m_McpsDataConfirm = _Callback_MacMcpsDataConfirm;
	macNotificationsPLC.m_McpsDataIndication = _Callback_MacMcpsDataIndication;
	macNotificationsPLC.m_MlmeResetConfirm = _Callback_MacMlmeResetConfirm;
	macNotificationsPLC.m_MlmeBeaconNotifyIndication = _Callback_MacMlmeBeaconNotify;
	macNotificationsPLC.m_MlmeScanConfirm = _Callback_MacMlmeScanConfirm;
	macNotificationsPLC.m_MlmeStartConfirm = _Callback_MacMlmeStartConfirm;
	macNotificationsPLC.m_MlmeCommStatusIndication = _Callback_MacMlmeCommStatusIndication;
	macNotificationsPLC.m_McpsMacSnifferIndication = _Callback_MacMcpsMacSnifferIndication;

	memset(macDeviceTable, 0, sizeof(macDeviceTable));

	sMacTables.m_MacDeviceTableSize = MAC_MAX_DEVICE_TABLE_ENTRIES;
	sMacTables.m_DeviceTable = macDeviceTable;

	MacInitialize(&macNotificationsPLC, u8PlcBand, &sMacTables);
#endif

#if defined(__RF_MAC__)
	LOG_INFO(Log("MacWrapperInitialize: Initializing RF MAC..."));

	macWrapperNotificationsRF.m_McpsDataConfirm = _Callback_MacMcpsDataConfirmRF;
	macWrapperNotificationsRF.m_McpsDataIndication = _Callback_MacMcpsDataIndicationRF;
	macWrapperNotificationsRF.m_MlmeResetConfirm = _Callback_MacMlmeResetConfirmRF;
	macWrapperNotificationsRF.m_MlmeBeaconNotifyIndication = _Callback_MacMlmeBeaconNotifyRF;
	macWrapperNotificationsRF.m_MlmeScanConfirm = _Callback_MacMlmeScanConfirmRF;
	macWrapperNotificationsRF.m_MlmeStartConfirm = _Callback_MacMlmeStartConfirmRF;
	macWrapperNotificationsRF.m_MlmeCommStatusIndication = _Callback_MacMlmeCommStatusIndicationRF;
	macWrapperNotificationsRF.m_McpsMacSnifferIndication = _Callback_MacMcpsMacSnifferIndicationRF;

	memset(macPOSTableRF, 0, sizeof(macPOSTableRF));
	memset(macDeviceTableRF, 0, sizeof(macDeviceTableRF));
	memset(macDsnTableRF, 0, sizeof(macDsnTableRF));

	sMacTablesRF.m_MacDeviceTableSizeRF = MAC_MAX_DEVICE_TABLE_ENTRIES_RF;
	sMacTablesRF.m_MacDsnTableSizeRF = MAC_MAX_DSN_TABLE_ENTRIES_RF;
	sMacTablesRF.m_MacPosTableSizeRF = MAC_MAX_POS_TABLE_ENTRIES_RF;
	sMacTablesRF.m_PosTableRF = macPOSTableRF;
	sMacTablesRF.m_DeviceTableRF = macDeviceTableRF;
	sMacTablesRF.m_DsnTableRF = macDsnTableRF;

	MacInitializeRF(&macWrapperNotificationsRF, &sMacTablesRF);
#endif

	MacCommonInitialize();
}

void MacWrapperEventHandler(void)
{
#if defined(__PLC_MAC__)
	MacEventHandler();
#endif
#if defined(__RF_MAC__)
	MacEventHandlerRF();
#endif
}

void MacWrapperMcpsDataRequest(struct TMacWrpDataRequest *pParameters)
{
#if defined(__PLC_MAC__) && defined(__RF_MAC__)
	struct THyALDataReq *pDataReq;

	LOG_INFO(LogBuffer(pParameters->m_pMsdu, pParameters->m_u16MsduLength, "MacWrapperMcpsDataRequest (Handle: 0x%02X Media Type: %02X): ", pParameters->m_u8MsduHandle, pParameters->m_eMediaType));

	/* Look for free Data Request Entry */
	pDataReq = _getFreeDataReqEntry();

	if (pDataReq == NULL) {
		/* Too many data requests */
		struct TMacWrpDataConfirm dataConfirm;
		/* Send confirm to upper layer */
		if (sUpperLayerNotifications.m_MacWrpDataConfirm != NULL) {
			dataConfirm.m_u8MsduHandle = pParameters->m_u8MsduHandle;
			dataConfirm.m_eStatus = MAC_WRP_STATUS_QUEUE_FULL;
			dataConfirm.m_nTimestamp = 0;
			dataConfirm.m_eMediaType = (enum EMacWrpMediaTypeConfirm)pParameters->m_eMediaType;
			sUpperLayerNotifications.m_MacWrpDataConfirm(&dataConfirm);
		}
	}
	else {
		/* Accept request */
		/* Copy parameters from MacWrp struct to Mac struct */
		memcpy(&pDataReq->m_sDataReqParameters, pParameters, sizeof(pDataReq->m_sDataReqParameters));
		/* Copy MediaType */
		pDataReq->m_eDataReqMediaType = pParameters->m_eMediaType;
		/* Copy Probing Interval */
		pDataReq->m_u8ProbingInterval = pParameters->m_u8ProbingInterval;
		/* Copy data to backup buffer, just in case backup media has to be used, current pointer will not be valid later */
		if (pParameters->m_u16MsduLength <= HYAL_BACKUP_BUF_SIZE) {
			memcpy(pDataReq->m_au8BackupBuffer, pParameters->m_pMsdu, pParameters->m_u16MsduLength);
		}

		switch (pDataReq->m_eDataReqMediaType) {
			case MAC_WRP_MEDIA_TYPE_REQ_PLC_BACKUP_RF:
				MacMcpsDataRequest(&pDataReq->m_sDataReqParameters);
				break;
			case MAC_WRP_MEDIA_TYPE_REQ_RF_BACKUP_PLC:
				MacMcpsDataRequestRF(&pDataReq->m_sDataReqParameters);
				break;
			case MAC_WRP_MEDIA_TYPE_REQ_BOTH:
				/* Set control variable */
				pDataReq->bWaitingSecondConfirm = false;
				/* Request on both Media */
				MacMcpsDataRequest(&pDataReq->m_sDataReqParameters);
				MacMcpsDataRequestRF(&pDataReq->m_sDataReqParameters);
				break;
			case MAC_WRP_MEDIA_TYPE_REQ_PLC_NO_BACKUP:
				MacMcpsDataRequest(&pDataReq->m_sDataReqParameters);
				break;
			case MAC_WRP_MEDIA_TYPE_REQ_RF_NO_BACKUP:
				MacMcpsDataRequestRF(&pDataReq->m_sDataReqParameters);
				break;
			default: /* PLC only */
				pDataReq->m_eDataReqMediaType = MAC_WRP_MEDIA_TYPE_REQ_PLC_NO_BACKUP;
				MacMcpsDataRequest(&pDataReq->m_sDataReqParameters);
				break;
		}
	}
#elif defined(__PLC_MAC__)
	struct TMcpsDataRequest dataRequest;
	LOG_INFO(LogBuffer(pParameters->m_pMsdu, pParameters->m_u16MsduLength, "MacWrapperMcpsDataRequest (Handle %02X): ", pParameters->m_u8MsduHandle));
	// Copy data to Mac struct (media type is not copied as it is the last field of pParameters)
	memcpy(&dataRequest, pParameters, sizeof(dataRequest));
	MacMcpsDataRequest(&dataRequest);
#elif defined(__RF_MAC__)
	struct TMcpsDataRequest dataRequest;
	LOG_INFO(LogBuffer(pParameters->m_pMsdu, pParameters->m_u16MsduLength, "MacWrapperMcpsDataRequest (Handle %02X): ", pParameters->m_u8MsduHandle));
	// Copy data to Mac struct (media type is not copied as it is the last field of pParameters)
	memcpy(&dataRequest, pParameters, sizeof(dataRequest));
	MacMcpsDataRequestRF(&dataRequest);
#endif
}

enum EMacWrpStatus MacWrapperMlmeGetRequestSync(enum EMacWrpPibAttribute eAttribute, uint16_t u16Index, struct TMacWrpPibValue *pValue)
{
	LOG_DBG(Log("MacWrapperMlmeGetRequestSync: Attribute: %08X; Index: %u", eAttribute, u16Index));
#if defined(__PLC_MAC__) && defined(__RF_MAC__)
	/* Check attribute ID range to redirect to Common, PLC or RF MAC */
	if (_IsSharedAttribute(eAttribute)) {
		/* Get from MAC Common */
		return (enum EMacWrpStatus)(MacCommonGetRequestSync((enum EMacCommonPibAttribute)eAttribute, u16Index, (struct TMacPibValue *)pValue));
	}
	else if (_IsAttributeInPLCRange(eAttribute)) {
		/* Get from PLC MAC */
		return (enum EMacWrpStatus)(MacMlmeGetRequestSync((enum EMacPibAttribute)eAttribute, u16Index, (struct TMacPibValue *)pValue));
	}
	else {
		/* Get from RF MAC */
		return (enum EMacWrpStatus)(MacMlmeGetRequestSyncRF((enum EMacPibAttributeRF)eAttribute, u16Index, (struct TMacPibValue *)pValue));
	}
#elif defined(__PLC_MAC__)
	/* Check attribute ID range to redirect to Common or PLC MAC */
	if (_IsSharedAttribute(eAttribute)) {
		/* Get from MAC Common */
		return (enum EMacWrpStatus)(MacCommonGetRequestSync((enum EMacCommonPibAttribute)eAttribute, u16Index, (struct TMacPibValue *)pValue));
	}
	else {
		/* RF Available IB has to be handled here */
		if (eAttribute == MAC_WRP_PIB_MANUF_RF_IFACE_AVAILABLE) {
			pValue->m_u8Length = 1;
			pValue->m_au8Value[0] = 0;
			return MAC_WRP_STATUS_SUCCESS;
		}
		else {
			/* Get from PLC MAC */
			return (enum EMacWrpStatus)(MacMlmeGetRequestSync((enum EMacPibAttribute)eAttribute, u16Index, (struct TMacPibValue *)pValue));
		}
	}
#elif defined(__RF_MAC__)
	/* Check attribute ID range to redirect to Common or RF MAC */
	if (_IsSharedAttribute(eAttribute)) {
		/* Get from MAC Common */
		return (enum EMacWrpStatus)(MacCommonGetRequestSync((enum EMacCommonPibAttribute)eAttribute, u16Index, (struct TMacPibValue *)pValue));
	}
	else {
		/* PLC Available IB has to be handled here */
		if (eAttribute == MAC_WRP_PIB_MANUF_PLC_IFACE_AVAILABLE) {
			pValue->m_u8Length = 1;
			pValue->m_au8Value[0] = 0;
			return MAC_WRP_STATUS_SUCCESS;
		}
		else {
			/* Get from RF MAC */
			return (enum EMacWrpStatus)(MacMlmeGetRequestSyncRF((enum EMacPibAttributeRF)eAttribute, u16Index, (struct TMacPibValue *)pValue));
		}
	}
#endif
}

enum EMacWrpStatus MacWrapperMlmeSetRequestSync(enum EMacWrpPibAttribute eAttribute, uint16_t u16Index, const struct TMacWrpPibValue *pValue)
{
	LOG_DBG(LogBuffer(pValue->m_au8Value, pValue->m_u8Length, "MacWrapperMlmeSetRequestSync: Attribute: %08X; Index: %u; Value: ", eAttribute, u16Index));
#if defined(__PLC_MAC__) && defined(__RF_MAC__)
	/* Check attribute ID range to redirect to Common, PLC or RF MAC */
	if (_IsSharedAttribute(eAttribute)) {
		/* Set to MAC Common */
		return (enum EMacWrpStatus)(MacCommonSetRequestSync((enum EMacCommonPibAttribute)eAttribute, u16Index, (const struct TMacPibValue *)pValue));
	}
	else if (_IsAttributeInPLCRange(eAttribute)) {
		/* Set to PLC MAC */
		return (enum EMacWrpStatus)(MacMlmeSetRequestSync((enum EMacPibAttribute)eAttribute, u16Index, (const struct TMacPibValue *)pValue));
	}
	else {
		/* Set to RF MAC */
		return (enum EMacWrpStatus)(MacMlmeSetRequestSyncRF((enum EMacPibAttributeRF)eAttribute, u16Index, (const struct TMacPibValue *)pValue));
	}
#elif defined(__PLC_MAC__)
	/* Check attribute ID range to redirect to Common or PLC MAC */
	if (_IsSharedAttribute(eAttribute)) {
		/* Set to MAC Common */
		return (enum EMacWrpStatus)(MacCommonSetRequestSync((enum EMacCommonPibAttribute)eAttribute, u16Index, (const struct TMacPibValue *)pValue));
	}
	else {
		/* Set to PLC MAC */
		return (enum EMacWrpStatus)(MacMlmeSetRequestSync((enum EMacPibAttribute)eAttribute, u16Index, (const struct TMacPibValue *)pValue));
	}
#elif defined(__RF_MAC__)
	/* Check attribute ID range to redirect to Common or RF MAC */
	if (_IsSharedAttribute(eAttribute)) {
		/* Set to MAC Common */
		return (enum EMacWrpStatus)(MacCommonSetRequestSync((enum EMacCommonPibAttribute)eAttribute, u16Index, (const struct TMacPibValue *)pValue));
	}
	else {
		/* Set to RF MAC */
		return (enum EMacWrpStatus)(MacMlmeSetRequestSyncRF((enum EMacPibAttributeRF)eAttribute, u16Index, (const struct TMacPibValue *)pValue));
	}
#endif
}

void MacWrapperMlmeResetRequest(struct TMacWrpResetRequest *pParameters)
{
	LOG_DBG(Log("MacWrapperMlmeResetRequest: Set default PIB: %u", pParameters->m_bSetDefaultPib));
#if defined(__PLC_MAC__) && defined(__RF_MAC__)
	// Set control variable
	g_HyAL.bWaitingSecondResetConfirm = false;
#endif
#if defined(__PLC_MAC__)
	// Reset PLC MAC
	MacMlmeResetRequest((struct TMlmeResetRequest *)pParameters);
#endif
#if defined(__RF_MAC__)
	// Reset RF MAC
	MacMlmeResetRequestRF((struct TMlmeResetRequest *)pParameters);
#endif
	// Reset Common MAC if IB has to be reset
	if (pParameters->m_bSetDefaultPib) {
		MacCommonReset();
	}
}

void MacWrapperMlmeScanRequest(struct TMacWrpScanRequest *pParameters)
{
	LOG_INFO(Log("MacWrapperMlmeScanRequest: Duration: %u", pParameters->m_u16ScanDuration));
#if defined(__PLC_MAC__) && defined(__RF_MAC__)
	// Set control variable
	g_HyAL.bWaitingSecondScanConfirm = false;
#endif
#if defined(__PLC_MAC__)
	// Set PLC MAC on Scan state
	MacMlmeScanRequest((struct TMlmeScanRequest *)pParameters);
#endif
#if defined(__RF_MAC__)
	// Set RF MAC on Scan state
	MacMlmeScanRequestRF((struct TMlmeScanRequest *)pParameters);
#endif
}

void MacWrapperMlmeStartRequest(struct TMacWrpStartRequest *pParameters)
{
	LOG_DBG(Log("MacWrapperMlmeStartRequest: Pan ID: %u", pParameters->m_nPanId));
#if defined(__PLC_MAC__) && defined(__RF_MAC__)
	// Set control variable
	g_HyAL.bWaitingSecondStartConfirm = false;
#endif
#if defined(__PLC_MAC__)
	// Start Network on PLC MAC
	MacMlmeStartRequest((struct TMlmeStartRequest *)pParameters);
#endif
#if defined(__RF_MAC__)
	// Start Network on PLC MAC
	MacMlmeStartRequestRF((struct TMlmeStartRequest *)pParameters);
#endif
}

enum EMacWrpAvailableMacLayers MacWrapperGetAvailableMacLayers(void)
{
#if defined(__PLC_MAC__) && defined(__RF_MAC__)
	return MAC_WRP_AVAILABLE_MAC_BOTH;
#elif defined(__PLC_MAC__)
	return MAC_WRP_AVAILABLE_MAC_PLC;
#elif defined(__RF_MAC__)
	return MAC_WRP_AVAILABLE_MAC_RF;
#endif
}

