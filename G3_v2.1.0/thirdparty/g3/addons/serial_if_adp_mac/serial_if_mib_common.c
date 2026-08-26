/**
 *
 * \file
 *
 * \brief MIB Serialization file
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
#include "serial_if_mib_common.h"
#include "string.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

static uint8_t auc_aux_endiannes_buf[256]; /* !<  Set Request Endianness tranformation buffer */
static uint16_t debug_set_length = 0;

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
	*puc_dst = (uint8_t)(us_aux & 0xFF);
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

void process_MIB_get_request(uint8_t *puc_serial_data, enum EMacWrpPibAttribute *eAttribute, uint16_t *u16AttributeIndex)
{
	uint32_t ul_aux;

	ul_aux = ((uint32_t)*puc_serial_data++) << 24;
	ul_aux += ((uint32_t)*puc_serial_data++) << 16;
	ul_aux += ((uint32_t)*puc_serial_data++) << 8;
	ul_aux += (uint32_t)*puc_serial_data++;
	*eAttribute = (enum EMacWrpPibAttribute)ul_aux;

	*u16AttributeIndex = ((uint16_t)*puc_serial_data++) << 8;
	*u16AttributeIndex += (uint16_t)*puc_serial_data;
}

void process_MIB_set_request(uint8_t *puc_serial_data, enum EMacWrpPibAttribute *eAttribute, uint16_t *u16AttributeIndex, struct TMacWrpPibValue *pibValue)
{
	uint8_t u8AttributeLengthCnt = 0;
#if defined(__PLC_MAC__)
	struct TMacWrpNeighbourEntry s_aux_NE;
	struct TMacWrpPOSEntry s_aux_PE;
#endif
#if defined(__RF_MAC__)
	struct TMacWrpPOSEntryRF s_aux_PE_RF;
#endif
	uint32_t ul_aux;

	ul_aux = ((uint32_t)*puc_serial_data++) << 24;
	ul_aux += ((uint32_t)*puc_serial_data++) << 16;
	ul_aux += ((uint32_t)*puc_serial_data++) << 8;
	ul_aux += (uint32_t)*puc_serial_data++;
	*eAttribute = (enum EMacWrpPibAttribute)ul_aux;

	*u16AttributeIndex = ((uint16_t)*puc_serial_data++) << 8;
	*u16AttributeIndex += (uint16_t)*puc_serial_data++;

	pibValue->m_u8Length = *puc_serial_data++;

	switch (*eAttribute) {
	/* 8-bit IBs */
	case MAC_WRP_PIB_PROMISCUOUS_MODE:
	case MAC_WRP_PIB_POS_TABLE_ENTRY_TTL:
	case MAC_WRP_PIB_POS_RECENT_ENTRY_THRESHOLD:
	case MAC_WRP_PIB_MANUF_PLC_IFACE_AVAILABLE:
	case MAC_WRP_PIB_MANUF_RF_IFACE_AVAILABLE:
#if defined(__PLC_MAC__)
	case MAC_WRP_PIB_BSN:
	case MAC_WRP_PIB_DSN:
	case MAC_WRP_PIB_MAX_BE:
	case MAC_WRP_PIB_MAX_CSMA_BACKOFFS:
	case MAC_WRP_PIB_MAX_FRAME_RETRIES:
	case MAC_WRP_PIB_MIN_BE:
	case MAC_WRP_PIB_HIGH_PRIORITY_WINDOW_SIZE:
	case MAC_WRP_PIB_FREQ_NOTCHING:
	case MAC_WRP_PIB_CSMA_FAIRNESS_LIMIT:
	case MAC_WRP_PIB_TMR_TTL:
	case MAC_WRP_PIB_DUPLICATE_DETECTION_TTL:
	case MAC_WRP_PIB_BEACON_RANDOMIZATION_WINDOW_LENGTH:
	case MAC_WRP_PIB_A:
	case MAC_WRP_PIB_K:
	case MAC_WRP_PIB_MIN_CW_ATTEMPTS:
	case MAC_WRP_PIB_BROADCAST_MAX_CW_ENABLE:
	case MAC_WRP_PIB_PLC_DISABLE:
	case MAC_WRP_PIB_MANUF_FORCED_MOD_SCHEME:
	case MAC_WRP_PIB_MANUF_FORCED_MOD_TYPE:
	case MAC_WRP_PIB_MANUF_FORCED_MOD_SCHEME_ON_TMRESPONSE:
	case MAC_WRP_PIB_MANUF_FORCED_MOD_TYPE_ON_TMRESPONSE:
	case MAC_WRP_PIB_MANUF_LBP_FRAME_RECEIVED:
	case MAC_WRP_PIB_MANUF_LNG_FRAME_RECEIVED:
	case MAC_WRP_PIB_MANUF_BCN_FRAME_RECEIVED:
	case MAC_WRP_PIB_MANUF_ENABLE_MAC_SNIFFER:
	case MAC_WRP_PIB_MANUF_RETRIES_LEFT_TO_FORCE_ROBO:
	case MAC_WRP_PIB_MANUF_SLEEP_MODE:
	case MAC_WRP_PIB_MANUF_TRICKLE_MIN_LQI:
#endif
#if defined(__RF_MAC__)
	case MAC_WRP_PIB_DSN_RF:
	case MAC_WRP_PIB_MAX_BE_RF:
	case MAC_WRP_PIB_MAX_CSMA_BACKOFFS_RF:
	case MAC_WRP_PIB_MAX_FRAME_RETRIES_RF:
	case MAC_WRP_PIB_MIN_BE_RF:
	case MAC_WRP_PIB_DUPLICATE_DETECTION_TTL_RF:
	case MAC_WRP_PIB_EBSN_RF:
	case MAC_WRP_PIB_OPERATING_MODE_RF:
	case MAC_WRP_PIB_DUTY_CYCLE_USAGE_RF:
	case MAC_WRP_PIB_DUTY_CYCLE_THRESHOLD_RF:
	case MAC_WRP_PIB_FREQUENCY_BAND_RF:
	case MAC_WRP_PIB_TRANSMIT_ATTEN_RF:
	case MAC_WRP_PIB_ADAPTIVE_POWER_STEP_RF:
	case MAC_WRP_PIB_ADAPTIVE_POWER_HIGH_BOUND_RF:
	case MAC_WRP_PIB_ADAPTIVE_POWER_LOW_BOUND_RF:
	case MAC_WRP_PIB_DISABLE_PHY_RF:
	case MAC_WRP_PIB_MANUF_LBP_FRAME_RECEIVED_RF:
	case MAC_WRP_PIB_MANUF_LNG_FRAME_RECEIVED_RF:
	case MAC_WRP_PIB_MANUF_BCN_FRAME_RECEIVED_RF:
	case MAC_WRP_PIB_MANUF_ENABLE_MAC_SNIFFER_RF:
	case MAC_WRP_PIB_MANUF_TRICKLE_MIN_LQI_RF:
#endif
		auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_serial_data++;
		break;

	/* 16-bit IBs */
	case MAC_WRP_PIB_PAN_ID:
	case MAC_WRP_PIB_SHORT_ADDRESS:
	case MAC_WRP_PIB_RC_COORD:
#if defined(__PLC_MAC__)
	case MAC_WRP_PIB_POS_RECENT_ENTRIES:
	case MAC_WRP_PIB_MANUF_LAST_FRAME_DURATION_PLC:
#endif
#if defined(__RF_MAC__)
	case MAC_WRP_PIB_CHANNEL_NUMBER_RF:
	case MAC_WRP_PIB_DUTY_CYCLE_PERIOD_RF:
	case MAC_WRP_PIB_DUTY_CYCLE_LIMIT_RF:
	case MAC_WRP_PIB_MANUF_POS_TABLE_COUNT_RF:
	case MAC_WRP_PIB_POS_RECENT_ENTRIES_RF:
	case MAC_WRP_PIB_MANUF_LAST_FRAME_DURATION_RF:
#endif
		mem_copy_from_usi_endianness_uint16((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_serial_data);
		u8AttributeLengthCnt += 2;
		break;

	/* 32-bit IBs */
#if defined(__PLC_MAC__)
	case MAC_WRP_PIB_FRAME_COUNTER:
	case MAC_WRP_PIB_TX_DATA_PACKET_COUNT:
	case MAC_WRP_PIB_RX_DATA_PACKET_COUNT:
	case MAC_WRP_PIB_TX_CMD_PACKET_COUNT:
	case MAC_WRP_PIB_RX_CMD_PACKET_COUNT:
	case MAC_WRP_PIB_CSMA_FAIL_COUNT:
	case MAC_WRP_PIB_CSMA_NO_ACK_COUNT:
	case MAC_WRP_PIB_RX_DATA_BROADCAST_COUNT:
	case MAC_WRP_PIB_TX_DATA_BROADCAST_COUNT:
	case MAC_WRP_PIB_BAD_CRC_COUNT:
	case MAC_WRP_PIB_MANUF_RX_OTHER_DESTINATION_COUNT:
	case MAC_WRP_PIB_MANUF_RX_INVALID_FRAME_LENGTH_COUNT:
	case MAC_WRP_PIB_MANUF_RX_MAC_REPETITION_COUNT:
	case MAC_WRP_PIB_MANUF_RX_WRONG_ADDR_MODE_COUNT:
	case MAC_WRP_PIB_MANUF_RX_UNSUPPORTED_SECURITY_COUNT:
	case MAC_WRP_PIB_MANUF_RX_WRONG_KEY_ID_COUNT:
	case MAC_WRP_PIB_MANUF_RX_INVALID_KEY_COUNT:
	case MAC_WRP_PIB_MANUF_RX_WRONG_FC_COUNT:
	case MAC_WRP_PIB_MANUF_RX_DECRYPTION_ERROR_COUNT:
	case MAC_WRP_PIB_MANUF_RX_SEGMENT_DECODE_ERROR_COUNT:
#endif
#if defined(__RF_MAC__)
	case MAC_WRP_PIB_FRAME_COUNTER_RF:
	case MAC_WRP_PIB_RETRY_COUNT_RF:
	case MAC_WRP_PIB_MULTIPLE_RETRY_COUNT_RF:
	case MAC_WRP_PIB_TX_FAIL_COUNT_RF:
	case MAC_WRP_PIB_TX_SUCCESS_COUNT_RF:
	case MAC_WRP_PIB_FCS_ERROR_COUNT_RF:
	case MAC_WRP_PIB_SECURITY_FAILURE_COUNT_RF:
	case MAC_WRP_PIB_DUPLICATE_FRAME_COUNT_RF:
	case MAC_WRP_PIB_RX_SUCCESS_COUNT_RF:
	case MAC_WRP_PIB_MANUF_ACK_TX_DELAY_RF:
	case MAC_WRP_PIB_MANUF_ACK_RX_WAIT_TIME_RF:
	case MAC_WRP_PIB_MANUF_ACK_CONFIRM_WAIT_TIME_RF:
	case MAC_WRP_PIB_MANUF_DATA_CONFIRM_WAIT_TIME_RF:
	case MAC_WRP_PIB_MANUF_RX_OTHER_DESTINATION_COUNT_RF:
	case MAC_WRP_PIB_MANUF_RX_INVALID_FRAME_LENGTH_COUNT_RF:
	case MAC_WRP_PIB_MANUF_RX_WRONG_ADDR_MODE_COUNT_RF:
	case MAC_WRP_PIB_MANUF_RX_UNSUPPORTED_SECURITY_COUNT_RF:
	case MAC_WRP_PIB_MANUF_RX_WRONG_KEY_ID_COUNT_RF:
	case MAC_WRP_PIB_MANUF_RX_INVALID_KEY_COUNT_RF:
	case MAC_WRP_PIB_MANUF_RX_WRONG_FC_COUNT_RF:
	case MAC_WRP_PIB_MANUF_RX_DECRYPTION_ERROR_COUNT_RF:
	case MAC_WRP_PIB_MANUF_TX_DATA_PACKET_COUNT_RF:
	case MAC_WRP_PIB_MANUF_RX_DATA_PACKET_COUNT_RF:
	case MAC_WRP_PIB_MANUF_TX_CMD_PACKET_COUNT_RF:
	case MAC_WRP_PIB_MANUF_RX_CMD_PACKET_COUNT_RF:
	case MAC_WRP_PIB_MANUF_CSMA_FAIL_COUNT_RF:
	case MAC_WRP_PIB_MANUF_RX_DATA_BROADCAST_COUNT_RF:
	case MAC_WRP_PIB_MANUF_TX_DATA_BROADCAST_COUNT_RF:
	case MAC_WRP_PIB_MANUF_BAD_CRC_COUNT_RF:
#endif
		mem_copy_from_usi_endianness_uint32((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_serial_data);
		u8AttributeLengthCnt += 4;
		break;

	/* Tables and lists */
	case MAC_WRP_PIB_MANUF_EXTENDED_ADDRESS:
		/* m_au8Address */
		memcpy((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_serial_data, 8);
		u8AttributeLengthCnt += 8;
		break;

	case MAC_WRP_PIB_KEY_TABLE:
		memcpy((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_serial_data, MAC_WRP_SECURITY_KEY_LENGTH);
		u8AttributeLengthCnt += MAC_WRP_SECURITY_KEY_LENGTH;
		break;

#if defined(__PLC_MAC__)
	case MAC_WRP_PIB_NEIGHBOUR_TABLE:
		s_aux_NE.m_nShortAddress = (uint16_t)((*puc_serial_data++) << 8);
		s_aux_NE.m_nShortAddress += (uint16_t)(*puc_serial_data++);
		memcpy((uint8_t *)&s_aux_NE.m_ToneMap.m_au8Tm[0], puc_serial_data, (MAC_WRP_MAX_TONE_GROUPS + 7) / 8);
		puc_serial_data += (MAC_WRP_MAX_TONE_GROUPS + 7) / 8;
		s_aux_NE.m_nModulationType = (uint8_t)(*puc_serial_data++);
		s_aux_NE.m_nTxGain = (uint8_t)(*puc_serial_data++);
		s_aux_NE.m_nTxRes = (uint8_t)(*puc_serial_data++);
		memcpy((uint8_t *)&s_aux_NE.m_TxCoef.m_au8TxCoef[0], puc_serial_data, 6);
		puc_serial_data += 6;
		s_aux_NE.m_nModulationScheme = (uint8_t)(*puc_serial_data++);
		s_aux_NE.m_nPhaseDifferential = (uint8_t)(*puc_serial_data++);
		s_aux_NE.m_u8Lqi  = (uint8_t)(*puc_serial_data++);
		s_aux_NE.m_u16TmrValidTime = (uint16_t)((*puc_serial_data++) << 8);
		s_aux_NE.m_u16TmrValidTime += (uint16_t)(*puc_serial_data++);
		memcpy((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], &s_aux_NE, sizeof(struct TMacWrpNeighbourEntry));
		u8AttributeLengthCnt += sizeof(struct TMacWrpNeighbourEntry);
		/* Struct save 2 bytes with bit-fields */
		pibValue->m_u8Length  -= 2;
		break;

	case MAC_WRP_PIB_POS_TABLE:
		s_aux_PE.m_nShortAddress = (uint16_t)((*puc_serial_data++) << 8);
		s_aux_PE.m_nShortAddress += (uint16_t)(*puc_serial_data++);
		s_aux_PE.m_u8Lqi  = (uint8_t)(*puc_serial_data++);
		s_aux_PE.m_u16POSValidTime = (uint16_t)((*puc_serial_data++) << 8);
		s_aux_PE.m_u16POSValidTime += (uint16_t)(*puc_serial_data++);
		memcpy((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], &s_aux_PE, sizeof(struct TMacWrpPOSEntry));
		u8AttributeLengthCnt += sizeof(struct TMacWrpPOSEntry);
		break;

	case MAC_WRP_PIB_TONE_MASK:
		memcpy((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_serial_data, (MAC_WRP_MAX_TONES + 7) / 8);
		u8AttributeLengthCnt += (MAC_WRP_MAX_TONES + 7) / 8;
		break;

	case MAC_WRP_PIB_MANUF_FORCED_TONEMAP:
		auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_serial_data++;
		auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_serial_data++;
		auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_serial_data++;
		break;

	case MAC_WRP_PIB_MANUF_FORCED_TONEMAP_ON_TMRESPONSE:
		auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_serial_data++;
		auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_serial_data++;
		auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_serial_data++;
		break;

	case MAC_WRP_PIB_MANUF_DEBUG_SET:
		memcpy((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_serial_data, 7);
		mem_copy_from_usi_endianness_uint16((uint8_t *)&debug_set_length, &puc_serial_data[5]);
		if (debug_set_length > 255) {
			debug_set_length = 0;
		}
		u8AttributeLengthCnt += 7;
		break;
#endif

#if defined(__RF_MAC__)
	case MAC_WRP_PIB_POS_TABLE_RF: /* 9 Byte entries. */
		s_aux_PE_RF.m_nShortAddress = (uint16_t)((*puc_serial_data++) << 8);
		s_aux_PE_RF.m_nShortAddress += (uint16_t)(*puc_serial_data++);
		s_aux_PE_RF.m_u8ForwardLqi  = (uint8_t)(*puc_serial_data++);
		s_aux_PE_RF.m_u8ReverseLqi  = (uint8_t)(*puc_serial_data++);
		s_aux_PE_RF.m_u8DutyCycle  = (uint8_t)(*puc_serial_data++);
		s_aux_PE_RF.m_u8ForwardTxPowerOffset  = (uint8_t)(*puc_serial_data++);
		s_aux_PE_RF.m_u8ReverseTxPowerOffset  = (uint8_t)(*puc_serial_data++);
		s_aux_PE_RF.m_u16POSValidTime = (uint16_t)((*puc_serial_data++) << 8);
		s_aux_PE_RF.m_u16POSValidTime += (uint16_t)(*puc_serial_data++);
		s_aux_PE_RF.m_u16ReverseLqiValidTime = (uint16_t)((*puc_serial_data++) << 8);
		s_aux_PE_RF.m_u16ReverseLqiValidTime += (uint16_t)(*puc_serial_data++);
		memcpy((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], &s_aux_PE_RF, sizeof(struct TMacWrpPOSEntryRF));
		u8AttributeLengthCnt += sizeof(struct TMacWrpPOSEntryRF);
		break;
#endif

#if defined(__PLC_MAC__)
	case MAC_WRP_PIB_SECURITY_ENABLED:
	case MAC_WRP_PIB_TIMESTAMP_SUPPORTED:
	case MAC_WRP_PIB_CENELEC_LEGACY_MODE:
	case MAC_WRP_PIB_FCC_LEGACY_MODE:
	case MAC_WRP_PIB_MANUF_DEVICE_TABLE:
	case MAC_WRP_PIB_MANUF_NEIGHBOUR_TABLE_ELEMENT:
	case MAC_WRP_PIB_MANUF_BAND_INFORMATION:
	case MAC_WRP_PIB_MANUF_COORD_SHORT_ADDRESS:
	case MAC_WRP_PIB_MANUF_MAX_MAC_PAYLOAD_SIZE:
	case MAC_WRP_PIB_MANUF_LAST_RX_MOD_SCHEME:
	case MAC_WRP_PIB_MANUF_LAST_RX_MOD_TYPE:
	case MAC_WRP_PIB_MANUF_NEIGHBOUR_TABLE_COUNT:
	case MAC_WRP_PIB_MANUF_POS_TABLE_COUNT:
	case MAC_WRP_PIB_MANUF_MAC_INTERNAL_VERSION:
	case MAC_WRP_PIB_MANUF_MAC_RT_INTERNAL_VERSION:
	case MAC_WRP_PIB_MANUF_DEBUG_READ:
	case MAC_WRP_PIB_MANUF_POS_TABLE_ELEMENT:
		/* MAC_WRP_STATUS_READ_ONLY */
		break;

	case MAC_WRP_PIB_MANUF_SECURITY_RESET:
		/* If length is 0 then DeviceTable is going to be reset else response will be MAC_WRP_STATUS_INVALID_PARAMETER */
		break;
	case MAC_WRP_PIB_MANUF_RESET_MAC_STATS:
		/* If length is 0 then MAC Statistics will be reset */
		break;

	case MAC_WRP_PIB_MANUF_PHY_PARAM:
		switch (*u16AttributeIndex) {
		case MAC_WRP_PHY_PARAM_VERSION:
		case MAC_WRP_PHY_PARAM_TX_TOTAL:
		case MAC_WRP_PHY_PARAM_TX_TOTAL_BYTES:
		case MAC_WRP_PHY_PARAM_TX_TOTAL_ERRORS:
		case MAC_WRP_PHY_PARAM_BAD_BUSY_TX:
		case MAC_WRP_PHY_PARAM_TX_BAD_BUSY_CHANNEL:
		case MAC_WRP_PHY_PARAM_TX_BAD_LEN:
		case MAC_WRP_PHY_PARAM_TX_BAD_FORMAT:
		case MAC_WRP_PHY_PARAM_TX_TIMEOUT:
		case MAC_WRP_PHY_PARAM_RX_TOTAL:
		case MAC_WRP_PHY_PARAM_RX_TOTAL_BYTES:
		case MAC_WRP_PHY_PARAM_RX_RS_ERRORS:
		case MAC_WRP_PHY_PARAM_RX_EXCEPTIONS:
		case MAC_WRP_PHY_PARAM_RX_BAD_LEN:
		case MAC_WRP_PHY_PARAM_RX_BAD_CRC_FCH:
		case MAC_WRP_PHY_PARAM_RX_FALSE_POSITIVE:
		case MAC_WRP_PHY_PARAM_RX_BAD_FORMAT:
		case MAC_WRP_PHY_PARAM_TIME_BETWEEN_NOISE_CAPTURES:
			mem_copy_from_usi_endianness_uint32((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_serial_data);
			u8AttributeLengthCnt += 4;
			break;

		case MAC_WRP_PHY_PARAM_LAST_MSG_RSSI:
		case MAC_WRP_PHY_PARAM_LAST_MSG_DURATION:
		case MAC_WRP_PHY_PARAM_ACK_TX_CFM:
			mem_copy_from_usi_endianness_uint16((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_serial_data);
			u8AttributeLengthCnt += 2;
			break;

		case MAC_WRP_PHY_PARAM_ENABLE_AUTO_NOISE_CAPTURE:
		case MAC_WRP_PHY_PARAM_DELAY_NOISE_CAPTURE_AFTER_RX:
		case MAC_WRP_PHY_PARAM_CFG_AUTODETECT_BRANCH:
		case MAC_WRP_PHY_PARAM_CFG_IMPEDANCE:
		case MAC_WRP_PHY_PARAM_RRC_NOTCH_ACTIVE:
		case MAC_WRP_PHY_PARAM_RRC_NOTCH_INDEX:
		case MAC_WRP_PHY_PARAM_PLC_DISABLE:
		case MAC_WRP_PHY_PARAM_NOISE_PEAK_POWER:
		case MAC_WRP_PHY_PARAM_LAST_MSG_LQI:
		case MAC_WRP_PHY_PARAM_PREAMBLE_NUM_SYNCP:
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_serial_data++;
			break;

		default:
			break;
		}
		break;
#endif

#if defined(__RF_MAC__)
	case MAC_WRP_PIB_TIMESTAMP_SUPPORTED_RF:
	case MAC_WRP_PIB_DEVICE_TABLE_RF:
	case MAC_WRP_PIB_COUNTER_OCTETS_RF:
	case MAC_WRP_PIB_USE_ENHANCED_BEACON_RF:
	case MAC_WRP_PIB_EB_HEADER_IE_LIST_RF:
	case MAC_WRP_PIB_EB_PAYLOAD_IE_LIST_RF:
	case MAC_WRP_PIB_EB_FILTERING_ENABLED_RF:
	case MAC_WRP_PIB_EB_AUTO_SA_RF:
	case MAC_WRP_PIB_SEC_SECURITY_LEVEL_LIST_RF:
	case MAC_WRP_PIB_MANUF_MAC_INTERNAL_VERSION_RF:
	case MAC_WRP_PIB_MANUF_POS_TABLE_ELEMENT_RF:
		/* MAC_WRP_STATUS_READ_ONLY */
		break;

	case MAC_WRP_PIB_MANUF_SECURITY_RESET_RF:
		/* If length is 0 then DeviceTableRF is going to be reset else response will be MAC_WRP_STATUS_INVALID_PARAMETER */
		break;
	case MAC_WRP_PIB_MANUF_RESET_MAC_STATS_RF:
		/* If length is 0 then MAC Statistics will be reset */
		break;

	case MAC_WRP_PIB_MANUF_PHY_PARAM_RF:
		switch (*u16AttributeIndex) {
		case MAC_WRP_RF_PHY_PARAM_PHY_CHANNEL_FREQ_HZ:
		case MAC_WRP_RF_PHY_PARAM_PHY_TX_TOTAL:
		case MAC_WRP_RF_PHY_PARAM_PHY_TX_TOTAL_BYTES:
		case MAC_WRP_RF_PHY_PARAM_PHY_TX_ERR_TOTAL:
		case MAC_WRP_RF_PHY_PARAM_PHY_TX_ERR_BUSY_TX:
		case MAC_WRP_RF_PHY_PARAM_PHY_TX_ERR_BUSY_RX:
		case MAC_WRP_RF_PHY_PARAM_PHY_TX_ERR_BUSY_CHN:
		case MAC_WRP_RF_PHY_PARAM_PHY_TX_ERR_BAD_LEN:
		case MAC_WRP_RF_PHY_PARAM_PHY_TX_ERR_BAD_FORMAT:
		case MAC_WRP_RF_PHY_PARAM_PHY_TX_ERR_TIMEOUT:
		case MAC_WRP_RF_PHY_PARAM_PHY_TX_ERR_ABORTED:
		case MAC_WRP_RF_PHY_PARAM_PHY_TX_CFM_NOT_HANDLED:
		case MAC_WRP_RF_PHY_PARAM_PHY_RX_TOTAL:
		case MAC_WRP_RF_PHY_PARAM_PHY_RX_TOTAL_BYTES:
		case MAC_WRP_RF_PHY_PARAM_PHY_RX_ERR_TOTAL:
		case MAC_WRP_RF_PHY_PARAM_PHY_RX_ERR_FALSE_POSITIVE:
		case MAC_WRP_RF_PHY_PARAM_PHY_RX_ERR_BAD_LEN:
		case MAC_WRP_RF_PHY_PARAM_PHY_RX_ERR_BAD_FORMAT:
		case MAC_WRP_RF_PHY_PARAM_PHY_RX_ERR_BAD_FCS_PAY:
		case MAC_WRP_RF_PHY_PARAM_PHY_RX_ERR_ABORTED:
		case MAC_WRP_RF_PHY_PARAM_PHY_RX_OVERRIDE:
		case MAC_WRP_RF_PHY_PARAM_PHY_RX_IND_NOT_HANDLED:
			mem_copy_from_usi_endianness_uint32((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_serial_data);
			u8AttributeLengthCnt += 4;
			break;

		case MAC_WRP_RF_PHY_PARAM_DEVICE_ID:
		case MAC_WRP_RF_PHY_PARAM_PHY_BAND_OPERATING_MODE:
		case MAC_WRP_RF_PHY_PARAM_PHY_CHANNEL_NUM:
		case MAC_WRP_RF_PHY_PARAM_PHY_CCA_ED_DURATION_US:
		case MAC_WRP_RF_PHY_PARAM_PHY_CCA_ED_DURATION_SYMBOLS:
		case MAC_WRP_RF_PHY_PARAM_PHY_TURNAROUND_TIME:
		case MAC_WRP_RF_PHY_PARAM_PHY_TX_PAY_SYMBOLS:
		case MAC_WRP_RF_PHY_PARAM_PHY_RX_PAY_SYMBOLS:
		case MAC_WRP_RF_PHY_PARAM_MAC_UNIT_BACKOFF_PERIOD:
			mem_copy_from_usi_endianness_uint16((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_serial_data);
			u8AttributeLengthCnt += 2;
			break;

		case MAC_WRP_RF_PHY_PARAM_DEVICE_RESET:
		case MAC_WRP_RF_PHY_PARAM_TRX_RESET:
		case MAC_WRP_RF_PHY_PARAM_TRX_SLEEP:
		case MAC_WRP_RF_PHY_PARAM_PHY_CCA_ED_THRESHOLD_DBM:
		case MAC_WRP_RF_PHY_PARAM_PHY_CCA_ED_THRESHOLD_SENSITIVITY:
		case MAC_WRP_RF_PHY_PARAM_PHY_STATS_RESET:
		case MAC_WRP_RF_PHY_PARAM_TX_FSK_FEC:
		case MAC_WRP_RF_PHY_PARAM_TX_OFDM_MCS:
		case MAC_WRP_RF_PHY_PARAM_CONTINUOUS_TX_MODE:
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_serial_data++;
			break;

		case MAC_WRP_RF_PHY_PARAM_FW_VERSION:
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_serial_data++; /* Major */
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_serial_data++; /* Minor */
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_serial_data++; /* Revision */
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_serial_data++; /* Year */
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_serial_data++; /* Month */
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_serial_data++; /* Day */
			break;

		case MAC_WRP_RF_PHY_PARAM_PHY_CCA_ED_CONFIG:
			mem_copy_from_usi_endianness_uint16((uint8_t *)&auc_aux_endiannes_buf[u8AttributeLengthCnt], puc_serial_data); /* us_duration_us */
			u8AttributeLengthCnt += 2;
			puc_serial_data += 2;
			auc_aux_endiannes_buf[u8AttributeLengthCnt++] = *puc_serial_data++; /* sc_threshold_dBm */
			break;

		default:
			break;
		}
		break;
#endif

	default:
		break;
	}

	memcpy(&pibValue->m_au8Value[0], &auc_aux_endiannes_buf[0], pibValue->m_u8Length);
}

uint8_t process_MIB_get_confirm(uint8_t *puc_serial_data, enum EMacWrpStatus eGetStatus, enum EMacWrpPibAttribute eAttribute,
                                uint16_t u16Index, struct TMacWrpPibValue *pibValue)
{
	uint8_t us_serial_response_len;
#if defined(__PLC_MAC__)
	struct TMacWrpNeighbourEntry *p_aux;
	struct TMacWrpPOSEntry *p_aux2;
#endif
#if defined(__RF_MAC__)
	struct TMacWrpPOSEntryRF *p_aux3;
#endif

	us_serial_response_len = 0;

	puc_serial_data[us_serial_response_len++] = (uint8_t)eGetStatus;
	puc_serial_data[us_serial_response_len++] = (uint8_t)(((uint32_t)(eAttribute >> 24)) & 0xFF);
	puc_serial_data[us_serial_response_len++] = (uint8_t)(((uint32_t)(eAttribute >> 16)) & 0xFF);
	puc_serial_data[us_serial_response_len++] = (uint8_t)(((uint32_t)(eAttribute >> 8)) & 0xFF);
	puc_serial_data[us_serial_response_len++] = (uint8_t)(((uint32_t)(eAttribute)) & 0xFF);
	puc_serial_data[us_serial_response_len++] = (uint8_t)(u16Index >> 8);
	puc_serial_data[us_serial_response_len++] = (uint8_t)(u16Index);

	puc_serial_data[us_serial_response_len++] = pibValue->m_u8Length;

	if (eGetStatus == MAC_WRP_STATUS_SUCCESS) {
		switch (eAttribute) {
		/* 8-bit IBs */
		case MAC_WRP_PIB_PROMISCUOUS_MODE:
		case MAC_WRP_PIB_POS_TABLE_ENTRY_TTL:
		case MAC_WRP_PIB_POS_RECENT_ENTRY_THRESHOLD:
		case MAC_WRP_PIB_MANUF_PLC_IFACE_AVAILABLE:
		case MAC_WRP_PIB_MANUF_RF_IFACE_AVAILABLE:
#if defined(__PLC_MAC__)
		case MAC_WRP_PIB_BSN:
		case MAC_WRP_PIB_DSN:
		case MAC_WRP_PIB_MAX_BE:
		case MAC_WRP_PIB_MAX_CSMA_BACKOFFS:
		case MAC_WRP_PIB_MAX_FRAME_RETRIES:
		case MAC_WRP_PIB_MIN_BE:
		case MAC_WRP_PIB_SECURITY_ENABLED:
		case MAC_WRP_PIB_TIMESTAMP_SUPPORTED:
		case MAC_WRP_PIB_HIGH_PRIORITY_WINDOW_SIZE:
		case MAC_WRP_PIB_FREQ_NOTCHING:
		case MAC_WRP_PIB_CSMA_FAIRNESS_LIMIT:
		case MAC_WRP_PIB_TMR_TTL:
		case MAC_WRP_PIB_DUPLICATE_DETECTION_TTL:
		case MAC_WRP_PIB_BEACON_RANDOMIZATION_WINDOW_LENGTH:
		case MAC_WRP_PIB_A:
		case MAC_WRP_PIB_K:
		case MAC_WRP_PIB_MIN_CW_ATTEMPTS:
		case MAC_WRP_PIB_CENELEC_LEGACY_MODE:
		case MAC_WRP_PIB_FCC_LEGACY_MODE:
		case MAC_WRP_PIB_BROADCAST_MAX_CW_ENABLE:
		case MAC_WRP_PIB_PLC_DISABLE:
		case MAC_WRP_PIB_MANUF_FORCED_MOD_SCHEME:
		case MAC_WRP_PIB_MANUF_FORCED_MOD_TYPE:
		case MAC_WRP_PIB_MANUF_FORCED_MOD_SCHEME_ON_TMRESPONSE:
		case MAC_WRP_PIB_MANUF_FORCED_MOD_TYPE_ON_TMRESPONSE:
		case MAC_WRP_PIB_MANUF_LAST_RX_MOD_SCHEME:
		case MAC_WRP_PIB_MANUF_LAST_RX_MOD_TYPE:
		case MAC_WRP_PIB_MANUF_LBP_FRAME_RECEIVED:
		case MAC_WRP_PIB_MANUF_LNG_FRAME_RECEIVED:
		case MAC_WRP_PIB_MANUF_BCN_FRAME_RECEIVED:
		case MAC_WRP_PIB_MANUF_ENABLE_MAC_SNIFFER:
		case MAC_WRP_PIB_MANUF_RETRIES_LEFT_TO_FORCE_ROBO:
		case MAC_WRP_PIB_MANUF_SLEEP_MODE:
		case MAC_WRP_PIB_MANUF_TRICKLE_MIN_LQI:
#endif
#if defined(__RF_MAC__)
		case MAC_WRP_PIB_DSN_RF:
		case MAC_WRP_PIB_MAX_BE_RF:
		case MAC_WRP_PIB_MAX_CSMA_BACKOFFS_RF:
		case MAC_WRP_PIB_MAX_FRAME_RETRIES_RF:
		case MAC_WRP_PIB_MIN_BE_RF:
		case MAC_WRP_PIB_TIMESTAMP_SUPPORTED_RF:
		case MAC_WRP_PIB_DUPLICATE_DETECTION_TTL_RF:
		case MAC_WRP_PIB_COUNTER_OCTETS_RF:
		case MAC_WRP_PIB_USE_ENHANCED_BEACON_RF:
		case MAC_WRP_PIB_EB_HEADER_IE_LIST_RF:
		case MAC_WRP_PIB_EB_FILTERING_ENABLED_RF:
		case MAC_WRP_PIB_EBSN_RF:
		case MAC_WRP_PIB_EB_AUTO_SA_RF:
		case MAC_WRP_PIB_OPERATING_MODE_RF:
		case MAC_WRP_PIB_DUTY_CYCLE_USAGE_RF:
		case MAC_WRP_PIB_DUTY_CYCLE_THRESHOLD_RF:
		case MAC_WRP_PIB_FREQUENCY_BAND_RF:
		case MAC_WRP_PIB_TRANSMIT_ATTEN_RF:
		case MAC_WRP_PIB_ADAPTIVE_POWER_STEP_RF:
		case MAC_WRP_PIB_ADAPTIVE_POWER_HIGH_BOUND_RF:
		case MAC_WRP_PIB_ADAPTIVE_POWER_LOW_BOUND_RF:
		case MAC_WRP_PIB_DISABLE_PHY_RF:
		case MAC_WRP_PIB_MANUF_SECURITY_RESET_RF:
		case MAC_WRP_PIB_MANUF_LBP_FRAME_RECEIVED_RF:
		case MAC_WRP_PIB_MANUF_LNG_FRAME_RECEIVED_RF:
		case MAC_WRP_PIB_MANUF_BCN_FRAME_RECEIVED_RF:
		case MAC_WRP_PIB_MANUF_ENABLE_MAC_SNIFFER_RF:
		case MAC_WRP_PIB_MANUF_TRICKLE_MIN_LQI_RF:
#endif
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[0];
			break;

		/* 16-bit IBs */
		case MAC_WRP_PIB_PAN_ID:
		case MAC_WRP_PIB_SHORT_ADDRESS:
		case MAC_WRP_PIB_RC_COORD:
#if defined(__PLC_MAC__)
		case MAC_WRP_PIB_POS_RECENT_ENTRIES:
		case MAC_WRP_PIB_MANUF_COORD_SHORT_ADDRESS:
		case MAC_WRP_PIB_MANUF_MAX_MAC_PAYLOAD_SIZE:
		case MAC_WRP_PIB_MANUF_NEIGHBOUR_TABLE_COUNT:
		case MAC_WRP_PIB_MANUF_POS_TABLE_COUNT:
		case MAC_WRP_PIB_MANUF_LAST_FRAME_DURATION_PLC:
#endif
#if defined(__RF_MAC__)
		case MAC_WRP_PIB_CHANNEL_NUMBER_RF:
		case MAC_WRP_PIB_DUTY_CYCLE_PERIOD_RF:
		case MAC_WRP_PIB_DUTY_CYCLE_LIMIT_RF:
		case MAC_WRP_PIB_MANUF_POS_TABLE_COUNT_RF:
		case MAC_WRP_PIB_POS_RECENT_ENTRIES_RF:
		case MAC_WRP_PIB_MANUF_LAST_FRAME_DURATION_RF:
#endif
			mem_copy_to_usi_endianness_uint16((uint8_t *)&puc_serial_data[us_serial_response_len],
					(uint8_t *)&pibValue->m_au8Value[0]);
			us_serial_response_len += 2;
			break;

		/* 32-bit IBs */
#if defined(__PLC_MAC__)
		case MAC_WRP_PIB_FRAME_COUNTER:
		case MAC_WRP_PIB_TX_DATA_PACKET_COUNT:
		case MAC_WRP_PIB_RX_DATA_PACKET_COUNT:
		case MAC_WRP_PIB_TX_CMD_PACKET_COUNT:
		case MAC_WRP_PIB_RX_CMD_PACKET_COUNT:
		case MAC_WRP_PIB_CSMA_FAIL_COUNT:
		case MAC_WRP_PIB_CSMA_NO_ACK_COUNT:
		case MAC_WRP_PIB_RX_DATA_BROADCAST_COUNT:
		case MAC_WRP_PIB_TX_DATA_BROADCAST_COUNT:
		case MAC_WRP_PIB_BAD_CRC_COUNT:
		case MAC_WRP_PIB_MANUF_RX_OTHER_DESTINATION_COUNT:
		case MAC_WRP_PIB_MANUF_RX_INVALID_FRAME_LENGTH_COUNT:
		case MAC_WRP_PIB_MANUF_RX_MAC_REPETITION_COUNT:
		case MAC_WRP_PIB_MANUF_RX_WRONG_ADDR_MODE_COUNT:
		case MAC_WRP_PIB_MANUF_RX_UNSUPPORTED_SECURITY_COUNT:
		case MAC_WRP_PIB_MANUF_RX_WRONG_KEY_ID_COUNT:
		case MAC_WRP_PIB_MANUF_RX_INVALID_KEY_COUNT:
		case MAC_WRP_PIB_MANUF_RX_WRONG_FC_COUNT:
		case MAC_WRP_PIB_MANUF_RX_DECRYPTION_ERROR_COUNT:
		case MAC_WRP_PIB_MANUF_RX_SEGMENT_DECODE_ERROR_COUNT:
#endif
#if defined(__RF_MAC__)
		case MAC_WRP_PIB_FRAME_COUNTER_RF:
		case MAC_WRP_PIB_RETRY_COUNT_RF:
		case MAC_WRP_PIB_MULTIPLE_RETRY_COUNT_RF:
		case MAC_WRP_PIB_TX_FAIL_COUNT_RF:
		case MAC_WRP_PIB_TX_SUCCESS_COUNT_RF:
		case MAC_WRP_PIB_FCS_ERROR_COUNT_RF:
		case MAC_WRP_PIB_SECURITY_FAILURE_COUNT_RF:
		case MAC_WRP_PIB_DUPLICATE_FRAME_COUNT_RF:
		case MAC_WRP_PIB_RX_SUCCESS_COUNT_RF:
		case MAC_WRP_PIB_MANUF_ACK_TX_DELAY_RF:
		case MAC_WRP_PIB_MANUF_ACK_RX_WAIT_TIME_RF:
		case MAC_WRP_PIB_MANUF_ACK_CONFIRM_WAIT_TIME_RF:
		case MAC_WRP_PIB_MANUF_DATA_CONFIRM_WAIT_TIME_RF:
		case MAC_WRP_PIB_MANUF_RX_OTHER_DESTINATION_COUNT_RF:
		case MAC_WRP_PIB_MANUF_RX_INVALID_FRAME_LENGTH_COUNT_RF:
		case MAC_WRP_PIB_MANUF_RX_WRONG_ADDR_MODE_COUNT_RF:
		case MAC_WRP_PIB_MANUF_RX_UNSUPPORTED_SECURITY_COUNT_RF:
		case MAC_WRP_PIB_MANUF_RX_WRONG_KEY_ID_COUNT_RF:
		case MAC_WRP_PIB_MANUF_RX_INVALID_KEY_COUNT_RF:
		case MAC_WRP_PIB_MANUF_RX_WRONG_FC_COUNT_RF:
		case MAC_WRP_PIB_MANUF_RX_DECRYPTION_ERROR_COUNT_RF:
		case MAC_WRP_PIB_MANUF_TX_DATA_PACKET_COUNT_RF:
		case MAC_WRP_PIB_MANUF_RX_DATA_PACKET_COUNT_RF:
		case MAC_WRP_PIB_MANUF_TX_CMD_PACKET_COUNT_RF:
		case MAC_WRP_PIB_MANUF_RX_CMD_PACKET_COUNT_RF:
		case MAC_WRP_PIB_MANUF_CSMA_FAIL_COUNT_RF:
		case MAC_WRP_PIB_MANUF_RX_DATA_BROADCAST_COUNT_RF:
		case MAC_WRP_PIB_MANUF_TX_DATA_BROADCAST_COUNT_RF:
		case MAC_WRP_PIB_MANUF_BAD_CRC_COUNT_RF:
#endif
			mem_copy_to_usi_endianness_uint32((uint8_t *)&puc_serial_data[us_serial_response_len],
					(uint8_t *)&pibValue->m_au8Value[0]);
			us_serial_response_len += 4;
			break;



		/* Tables and lists */
		case MAC_WRP_PIB_MANUF_EXTENDED_ADDRESS:
			/* m_au8Address */
			memcpy((uint8_t *)&puc_serial_data[us_serial_response_len], (uint8_t *)pibValue->m_au8Value, 8);
			us_serial_response_len += 8;
			break;

#if defined(__PLC_MAC__)
		case MAC_WRP_PIB_NEIGHBOUR_TABLE:
			p_aux = (struct TMacWrpNeighbourEntry *)&pibValue->m_au8Value[0];
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux->m_nShortAddress >> 8);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux->m_nShortAddress & 0xFF);
			memcpy((uint8_t *)&puc_serial_data[us_serial_response_len], (uint8_t *)&p_aux->m_ToneMap.m_au8Tm[0], (MAC_WRP_MAX_TONE_GROUPS + 7) / 8);
			us_serial_response_len += (MAC_WRP_MAX_TONE_GROUPS + 7) / 8;
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux->m_nModulationType);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux->m_nTxGain);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux->m_nTxRes);
			memcpy((uint8_t *)&puc_serial_data[us_serial_response_len], (uint8_t *)&p_aux->m_TxCoef.m_au8TxCoef[0], 6);
			us_serial_response_len += 6;
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux->m_nModulationScheme);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux->m_nPhaseDifferential);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux->m_u8Lqi);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux->m_u16TmrValidTime >> 8);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux->m_u16TmrValidTime & 0xFF);
			/* Length has to be incremented by 2 due to bitfields in the entry are serialized in separate fields */
			puc_serial_data[7] = pibValue->m_u8Length + 2;
			break;

		case MAC_WRP_PIB_POS_TABLE:
			p_aux2 = (struct TMacWrpPOSEntry *)&pibValue->m_au8Value[0];
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux2->m_nShortAddress >> 8);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux2->m_nShortAddress & 0xFF);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux2->m_u8Lqi);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux2->m_u16POSValidTime >> 8);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux2->m_u16POSValidTime & 0xFF);
			break;

		case MAC_WRP_PIB_TONE_MASK:
			memcpy((uint8_t *)&puc_serial_data[us_serial_response_len],
					(uint8_t *)&pibValue->m_au8Value[0], (MAC_WRP_MAX_TONES + 7) / 8);
			us_serial_response_len += (MAC_WRP_MAX_TONES + 7) / 8;
			break;

		case MAC_WRP_PIB_MANUF_DEVICE_TABLE:
			/* m_nPanId */
			mem_copy_to_usi_endianness_uint16((uint8_t *)&puc_serial_data[us_serial_response_len],
					(uint8_t *)&pibValue->m_au8Value[0]);
			us_serial_response_len += 2;
			/* m_nShortAddress */
			mem_copy_to_usi_endianness_uint16((uint8_t *)&puc_serial_data[us_serial_response_len],
					(uint8_t *)&pibValue->m_au8Value[2]);
			us_serial_response_len += 2;
			/* m_au32FrameCounter */
			mem_copy_to_usi_endianness_uint32((uint8_t *)&puc_serial_data[us_serial_response_len],
					(uint8_t *)&pibValue->m_au8Value[4]);
			us_serial_response_len += 4;
			break;

		case MAC_WRP_PIB_MANUF_NEIGHBOUR_TABLE_ELEMENT:
			p_aux = (struct TMacWrpNeighbourEntry *)&pibValue->m_au8Value[0];
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux->m_nShortAddress >> 8);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux->m_nShortAddress & 0xFF);
			memcpy((uint8_t *)&puc_serial_data[us_serial_response_len], (uint8_t *)&p_aux->m_ToneMap.m_au8Tm[0], (MAC_WRP_MAX_TONE_GROUPS + 7) / 8);
			us_serial_response_len += (MAC_WRP_MAX_TONE_GROUPS + 7) / 8;
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux->m_nModulationType);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux->m_nTxGain);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux->m_nTxRes);
			memcpy((uint8_t *)&puc_serial_data[us_serial_response_len], (uint8_t *)&p_aux->m_TxCoef.m_au8TxCoef[0], 6);
			us_serial_response_len += 6;
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux->m_nModulationScheme);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux->m_nPhaseDifferential);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux->m_u8Lqi);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux->m_u16TmrValidTime >> 8);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux->m_u16TmrValidTime & 0xFF);
			/* Length has to be incremented by 2 due to bitfields in the entry are serialized in separate fields */
			puc_serial_data[7] = pibValue->m_u8Length + 2;
			break;

		case MAC_WRP_PIB_MANUF_BAND_INFORMATION:
			/* m_u16FlMax */
			mem_copy_to_usi_endianness_uint16((uint8_t *)&puc_serial_data[us_serial_response_len],
					(uint8_t *)&pibValue->m_au8Value[0]);
			us_serial_response_len += 2;
			/* m_u8Band */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[2];
			/* m_u8Tones */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[3];
			/* m_u8Carriers */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[4];
			/* m_u8TonesInCarrier */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[5];
			/* m_u8FlBand */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[6];

			/* m_u8MaxRsBlocks */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[7];
			/* m_u8TxCoefBits */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[8];
			/* m_u8PilotsFreqSpa */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[9];
			break;

		case MAC_WRP_PIB_MANUF_FORCED_TONEMAP:
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[0];
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[1];
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[2];
			break;

		case MAC_WRP_PIB_MANUF_FORCED_TONEMAP_ON_TMRESPONSE:
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[0];
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[1];
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[2];
			break;

		case MAC_WRP_PIB_MANUF_MAC_INTERNAL_VERSION:
			/* Version */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[0]; /* m_u8Major */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[1]; /* m_u8Minor */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[2]; /* m_u8Revision */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[3]; /* m_u8Year */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[4]; /* m_u8Month */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[5]; /* m_u8Day */
			break;

		case MAC_WRP_PIB_MANUF_MAC_RT_INTERNAL_VERSION:
			/* Version */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[0]; /* m_u8Major */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[1]; /* m_u8Minor */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[2]; /* m_u8Revision */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[3]; /* m_u8Year */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[4]; /* m_u8Month */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[5]; /* m_u8Day */
			break;

		case MAC_WRP_PIB_MANUF_DEBUG_SET:
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[0];
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[1];
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[2];
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[3];
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[4];
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[5];
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[6];
			break;

		case MAC_WRP_PIB_MANUF_DEBUG_READ:
			memcpy((uint8_t *)&puc_serial_data[us_serial_response_len], (uint8_t *)pibValue->m_au8Value, debug_set_length);
			us_serial_response_len += debug_set_length;
			break;

		case MAC_WRP_PIB_MANUF_POS_TABLE_ELEMENT:
			p_aux2 = (struct TMacWrpPOSEntry *)&pibValue->m_au8Value[0];
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux2->m_nShortAddress >> 8);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux2->m_nShortAddress & 0xFF);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux2->m_u8Lqi);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux2->m_u16POSValidTime >> 8);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux2->m_u16POSValidTime & 0xFF);
			break;
#endif
#if defined(__RF_MAC__)
		case MAC_WRP_PIB_DEVICE_TABLE_RF:
			/* m_nPanId */
			mem_copy_to_usi_endianness_uint16((uint8_t *)&puc_serial_data[us_serial_response_len],
					(uint8_t *)&pibValue->m_au8Value[0]);
			us_serial_response_len += 2;
			/* m_nShortAddress */
			mem_copy_to_usi_endianness_uint16((uint8_t *)&puc_serial_data[us_serial_response_len],
					(uint8_t *)&pibValue->m_au8Value[2]);
			us_serial_response_len += 2;
			/* m_au32FrameCounter */
			mem_copy_to_usi_endianness_uint32((uint8_t *)&puc_serial_data[us_serial_response_len],
					(uint8_t *)&pibValue->m_au8Value[4]);
			us_serial_response_len += 4;
			break;

		case MAC_WRP_PIB_SEC_SECURITY_LEVEL_LIST_RF:
			/* 4 Byte entries. */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[0]; /* m_u8FrameType */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[1]; /* m_u8CommandId */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[2]; /* m_u8SecurityMinimum */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[3]; /* m_bOverrideSecurityMinimum */
			break;

		case MAC_WRP_PIB_POS_TABLE_RF: /* 9 Byte entries. */
			p_aux3 = (struct TMacWrpPOSEntryRF *)&pibValue->m_au8Value[0];
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux3->m_nShortAddress >> 8);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux3->m_nShortAddress & 0xFF);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux3->m_u8ForwardLqi);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux3->m_u8ReverseLqi);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux3->m_u8DutyCycle);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux3->m_u8ForwardTxPowerOffset);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux3->m_u8ReverseTxPowerOffset);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux3->m_u16POSValidTime >> 8);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux3->m_u16POSValidTime & 0xFF);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux3->m_u16ReverseLqiValidTime >> 8);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux3->m_u16ReverseLqiValidTime & 0xFF);
			break;

		case MAC_WRP_PIB_MANUF_MAC_INTERNAL_VERSION_RF:
			/* Version */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[0]; /* m_u8Major */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[1]; /* m_u8Minor */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[2]; /* m_u8Revision */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[3]; /* m_u8Year */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[4]; /* m_u8Month */
			puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[5]; /* m_u8Day */
			break;

		case MAC_WRP_PIB_MANUF_POS_TABLE_ELEMENT_RF:
			p_aux3 = (struct TMacWrpPOSEntryRF *)&pibValue->m_au8Value[0];
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux3->m_nShortAddress >> 8);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux3->m_nShortAddress & 0xFF);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux3->m_u8ForwardLqi);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux3->m_u8ReverseLqi);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux3->m_u8DutyCycle);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux3->m_u8ForwardTxPowerOffset);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux3->m_u8ReverseTxPowerOffset);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux3->m_u16POSValidTime >> 8);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux3->m_u16POSValidTime & 0xFF);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux3->m_u16ReverseLqiValidTime >> 8);
			puc_serial_data[us_serial_response_len++] = (uint8_t)(p_aux3->m_u16ReverseLqiValidTime & 0xFF);
			break;
#endif

		case MAC_WRP_PIB_KEY_TABLE:
			/* response will be MAC_WRP_STATUS_UNAVAILABLE_KEY */
			break;

#if defined(__PLC_MAC__)
		case MAC_WRP_PIB_MANUF_SECURITY_RESET:
		case MAC_WRP_PIB_MANUF_RESET_MAC_STATS:
			/* Response will be mac_status_denied */
			break;

		case MAC_WRP_PIB_MANUF_PHY_PARAM:
			switch (u16Index) {
			case MAC_WRP_PHY_PARAM_VERSION:
			case MAC_WRP_PHY_PARAM_TX_TOTAL:
			case MAC_WRP_PHY_PARAM_TX_TOTAL_BYTES:
			case MAC_WRP_PHY_PARAM_TX_TOTAL_ERRORS:
			case MAC_WRP_PHY_PARAM_BAD_BUSY_TX:
			case MAC_WRP_PHY_PARAM_TX_BAD_BUSY_CHANNEL:
			case MAC_WRP_PHY_PARAM_TX_BAD_LEN:
			case MAC_WRP_PHY_PARAM_TX_BAD_FORMAT:
			case MAC_WRP_PHY_PARAM_TX_TIMEOUT:
			case MAC_WRP_PHY_PARAM_RX_TOTAL:
			case MAC_WRP_PHY_PARAM_RX_TOTAL_BYTES:
			case MAC_WRP_PHY_PARAM_RX_RS_ERRORS:
			case MAC_WRP_PHY_PARAM_RX_EXCEPTIONS:
			case MAC_WRP_PHY_PARAM_RX_BAD_LEN:
			case MAC_WRP_PHY_PARAM_RX_BAD_CRC_FCH:
			case MAC_WRP_PHY_PARAM_RX_FALSE_POSITIVE:
			case MAC_WRP_PHY_PARAM_RX_BAD_FORMAT:
			case MAC_WRP_PHY_PARAM_TIME_BETWEEN_NOISE_CAPTURES:
				mem_copy_to_usi_endianness_uint32((uint8_t *)&puc_serial_data[us_serial_response_len],
						(uint8_t *)&pibValue->m_au8Value[0]);
				us_serial_response_len += 4;
				break;

			case MAC_WRP_PHY_PARAM_LAST_MSG_RSSI:
			case MAC_WRP_PHY_PARAM_LAST_MSG_DURATION:
			case MAC_WRP_PHY_PARAM_ACK_TX_CFM:
				mem_copy_to_usi_endianness_uint16((uint8_t *)&puc_serial_data[us_serial_response_len],
						(uint8_t *)&pibValue->m_au8Value[0]);
				us_serial_response_len += 2;
				break;

			case MAC_WRP_PHY_PARAM_ENABLE_AUTO_NOISE_CAPTURE:
			case MAC_WRP_PHY_PARAM_DELAY_NOISE_CAPTURE_AFTER_RX:
			case MAC_WRP_PHY_PARAM_CFG_AUTODETECT_BRANCH:
			case MAC_WRP_PHY_PARAM_CFG_IMPEDANCE:
			case MAC_WRP_PHY_PARAM_RRC_NOTCH_ACTIVE:
			case MAC_WRP_PHY_PARAM_RRC_NOTCH_INDEX:
			case MAC_WRP_PHY_PARAM_PLC_DISABLE:
			case MAC_WRP_PHY_PARAM_NOISE_PEAK_POWER:
			case MAC_WRP_PHY_PARAM_LAST_MSG_LQI:
			case MAC_WRP_PHY_PARAM_PREAMBLE_NUM_SYNCP:
				puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[0];
				break;

			default:
				break;
			}
			break;
#endif

#if defined(__RF_MAC__)
		case MAC_WRP_PIB_EB_PAYLOAD_IE_LIST_RF:
			/* This IB is an empty array */
			break;

		case MAC_WRP_PIB_MANUF_RESET_MAC_STATS_RF:
			/* Response will be mac_status_denied */
			break;

		case MAC_WRP_PIB_MANUF_PHY_PARAM_RF:
			switch (u16Index) {
			case MAC_WRP_RF_PHY_PARAM_PHY_CHANNEL_FREQ_HZ:
			case MAC_WRP_RF_PHY_PARAM_PHY_TX_TOTAL:
			case MAC_WRP_RF_PHY_PARAM_PHY_TX_TOTAL_BYTES:
			case MAC_WRP_RF_PHY_PARAM_PHY_TX_ERR_TOTAL:
			case MAC_WRP_RF_PHY_PARAM_PHY_TX_ERR_BUSY_TX:
			case MAC_WRP_RF_PHY_PARAM_PHY_TX_ERR_BUSY_RX:
			case MAC_WRP_RF_PHY_PARAM_PHY_TX_ERR_BUSY_CHN:
			case MAC_WRP_RF_PHY_PARAM_PHY_TX_ERR_BAD_LEN:
			case MAC_WRP_RF_PHY_PARAM_PHY_TX_ERR_BAD_FORMAT:
			case MAC_WRP_RF_PHY_PARAM_PHY_TX_ERR_TIMEOUT:
			case MAC_WRP_RF_PHY_PARAM_PHY_TX_ERR_ABORTED:
			case MAC_WRP_RF_PHY_PARAM_PHY_TX_CFM_NOT_HANDLED:
			case MAC_WRP_RF_PHY_PARAM_PHY_RX_TOTAL:
			case MAC_WRP_RF_PHY_PARAM_PHY_RX_TOTAL_BYTES:
			case MAC_WRP_RF_PHY_PARAM_PHY_RX_ERR_TOTAL:
			case MAC_WRP_RF_PHY_PARAM_PHY_RX_ERR_FALSE_POSITIVE:
			case MAC_WRP_RF_PHY_PARAM_PHY_RX_ERR_BAD_LEN:
			case MAC_WRP_RF_PHY_PARAM_PHY_RX_ERR_BAD_FORMAT:
			case MAC_WRP_RF_PHY_PARAM_PHY_RX_ERR_BAD_FCS_PAY:
			case MAC_WRP_RF_PHY_PARAM_PHY_RX_ERR_ABORTED:
			case MAC_WRP_RF_PHY_PARAM_PHY_RX_OVERRIDE:
			case MAC_WRP_RF_PHY_PARAM_PHY_RX_IND_NOT_HANDLED:
				mem_copy_to_usi_endianness_uint32((uint8_t *)&puc_serial_data[us_serial_response_len],
						(uint8_t *)&pibValue->m_au8Value[0]);
				us_serial_response_len += 4;
				break;

			case MAC_WRP_RF_PHY_PARAM_DEVICE_ID:
			case MAC_WRP_RF_PHY_PARAM_PHY_BAND_OPERATING_MODE:
			case MAC_WRP_RF_PHY_PARAM_PHY_CHANNEL_NUM:
			case MAC_WRP_RF_PHY_PARAM_PHY_CCA_ED_DURATION_US:
			case MAC_WRP_RF_PHY_PARAM_PHY_CCA_ED_DURATION_SYMBOLS:
			case MAC_WRP_RF_PHY_PARAM_PHY_TURNAROUND_TIME:
			case MAC_WRP_RF_PHY_PARAM_PHY_TX_PAY_SYMBOLS:
			case MAC_WRP_RF_PHY_PARAM_PHY_RX_PAY_SYMBOLS:
			case MAC_WRP_RF_PHY_PARAM_MAC_UNIT_BACKOFF_PERIOD:
				mem_copy_to_usi_endianness_uint16((uint8_t *)&puc_serial_data[us_serial_response_len],
						(uint8_t *)&pibValue->m_au8Value[0]);
				us_serial_response_len += 2;
				break;

			case MAC_WRP_RF_PHY_PARAM_DEVICE_RESET:
			case MAC_WRP_RF_PHY_PARAM_TRX_RESET:
			case MAC_WRP_RF_PHY_PARAM_TRX_SLEEP:
			case MAC_WRP_RF_PHY_PARAM_PHY_CCA_ED_THRESHOLD_DBM:
			case MAC_WRP_RF_PHY_PARAM_PHY_CCA_ED_THRESHOLD_SENSITIVITY:
			case MAC_WRP_RF_PHY_PARAM_PHY_STATS_RESET:
			case MAC_WRP_RF_PHY_PARAM_TX_FSK_FEC:
			case MAC_WRP_RF_PHY_PARAM_TX_OFDM_MCS:
			case MAC_WRP_RF_PHY_PARAM_CONTINUOUS_TX_MODE:
				puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[0];
				break;

			case MAC_WRP_RF_PHY_PARAM_FW_VERSION:
				puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[0]; /* Major */
				puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[1]; /* Minor */
				puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[2]; /* Revision */
				puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[3]; /* Year */
				puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[4]; /* Month */
				puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[5]; /* Day */
				break;

			case MAC_WRP_RF_PHY_PARAM_PHY_CCA_ED_CONFIG:
				mem_copy_to_usi_endianness_uint16((uint8_t *)&puc_serial_data[us_serial_response_len],
						(uint8_t *)&pibValue->m_au8Value[0]); /* us_duration_us */
				us_serial_response_len += 2;
				puc_serial_data[us_serial_response_len++] = pibValue->m_au8Value[2]; /* sc_threshold_dBm */
				break;

			default:
				break;
			}
			break;
#endif

		default:
			break;
		}
	}

	return us_serial_response_len;
}

uint8_t process_MIB_set_confirm(uint8_t *puc_serial_data, enum EMacWrpStatus eGetStatus, enum EMacWrpPibAttribute eAttribute, uint16_t u16Index)
{
	uint8_t us_serial_response_len;

	us_serial_response_len = 0;

	puc_serial_data[us_serial_response_len++] = (uint8_t)eGetStatus;
	puc_serial_data[us_serial_response_len++] = (uint8_t)(((uint32_t)(eAttribute >> 24)) & 0xFF);
	puc_serial_data[us_serial_response_len++] = (uint8_t)(((uint32_t)(eAttribute >> 16)) & 0xFF);
	puc_serial_data[us_serial_response_len++] = (uint8_t)(((uint32_t)(eAttribute >> 8)) & 0xFF);
	puc_serial_data[us_serial_response_len++] = (uint8_t)(((uint32_t)(eAttribute)) & 0xFF);
	puc_serial_data[us_serial_response_len++] = (uint8_t)(u16Index >> 8);
	puc_serial_data[us_serial_response_len++] = (uint8_t)(u16Index & 0xFF);

	return us_serial_response_len;
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
