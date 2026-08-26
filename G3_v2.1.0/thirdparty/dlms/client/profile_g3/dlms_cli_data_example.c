/**
 * \file
 *
 * \brief DLMS_CLI_LIB : DLMS client lib: G3 Profile
 *
 * Copyright (c) 2016 Atmel Corporation. All rights reserved.
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
#include <string.h>
#include <stdio.h>

#include "dlms_cli_lib.h"
#include "dlms_cli_data_example.h"

/* #define DLMS_RESULT_PRINT */

/**
 * \brief Get meter index (user defined, result must be a number between 0 and dlms_cli_num_max_nodes - 1)
 * \param us_short_addr     Node short address
 *
 * \retval Meter index
 */
uint16_t dlms_cli_get_dlms_idx(uint16_t us_short_addr)
{
#ifndef WIN32
	return (us_short_addr - 1) % dlms_cli_num_max_nodes();

#else
	return (us_short_addr) % dlms_cli_num_max_nodes();
#endif
}

/**
 * \brief Print simple data
 * \param x_data_type      Data type
 * \param us_length        Data length
 * \param puc_rx_data      Pointer to data
 *
 * \retval Data access result
 */
static data_access_result_t _dlms_cli_data_print_data(data_type_t x_data_type, uint16_t us_length, uint8_t *puc_rx_data)
{
#ifndef DLMS_RESULT_PRINT
	(void)x_data_type;
	(void)us_length;
	(void)puc_rx_data;
#else
	uint16_t us_idx;
	uint16_t us_data_idx = 0;
	uint16_t us_aux;
        uint32_t ul_aux;
	uint16_t us_struct_elements;

	printf("[DLMS_DAT] ");
	if (puc_rx_data[us_data_idx++] != x_data_type) {
		printf("Expected type was %hhu but received %hhu\n", x_data_type, puc_rx_data[us_data_idx - 1]);
		return DAR_OTHER_REASON;
	}

	switch (x_data_type) {
	case DT_NULL_DATA:
		printf("DT_NULL\n");
		return DAR_SUCCESS;

	case DT_BOOLEAN:
		printf("DT_BOOLEAN %hhu\n", puc_rx_data[us_data_idx++]);
		return DAR_SUCCESS;

	case DT_BIT_STRING:
		return DAR_SUCCESS;
                
	case DT_DOUBLE_LONG:
                ul_aux = ((uint16_t)puc_rx_data[us_data_idx++]) << 24;
                ul_aux |= ((uint16_t)puc_rx_data[us_data_idx++]) << 16;
                ul_aux |= ((uint16_t)puc_rx_data[us_data_idx++]) << 8;
		ul_aux |= puc_rx_data[us_data_idx++];
		printf("DT_DOUBLE_LONG_UNSIGNED %d\n", (int32_t)ul_aux);
		return DAR_SUCCESS;

	case DT_DOUBLE_LONG_UNSIGNED:
                ul_aux = ((uint16_t)puc_rx_data[us_data_idx++]) << 24;
                ul_aux |= ((uint16_t)puc_rx_data[us_data_idx++]) << 16;
                ul_aux |= ((uint16_t)puc_rx_data[us_data_idx++]) << 8;
		ul_aux |= puc_rx_data[us_data_idx++];
		printf("DT_DOUBLE_LONG_UNSIGNED %u\n", ul_aux);
		return DAR_SUCCESS;

	case DT_OCTET_STRING:
	case DT_DATE_TIME:
	case DT_DATE:
	case DT_TIME:
		us_data_idx += dlms_cli_decode_a_xdr_length(&us_aux, &puc_rx_data[us_data_idx]);
		if (us_aux != us_length - us_data_idx) {
			printf("Expected length was %hhu but received %hhu\n", us_length - us_data_idx, us_aux);
			return DAR_OTHER_REASON;
		}

		printf("DT_OCTET_STRING ");
		for (us_idx = 0; us_idx < (us_aux - 1); us_idx++) {
			printf("%02X ", puc_rx_data[us_data_idx++]);
		}

		/* last byte is printed apart */
		printf("%02X\n", puc_rx_data[us_data_idx++]);
		return DAR_SUCCESS;

	case DT_VISIBLE_STRING:
		return DAR_SUCCESS;

	case DT_UTF8_STRING:
		return DAR_SUCCESS;

	case DT_BCD:
		return DAR_SUCCESS;

	case DT_INTEGER:
		printf("DT_INTEGER %hhd\n", (int16_t)puc_rx_data[us_data_idx++]);
		return DAR_SUCCESS;

	case DT_LONG:
		us_aux = ((uint16_t)puc_rx_data[us_data_idx++]) << 8;
		us_aux |= puc_rx_data[us_data_idx++];
		printf("DT_LONG %hd\n", (int16_t)us_aux);
		return DAR_SUCCESS;

	case DT_UNSIGNED:
		printf("DT_UNSIGNED %hhu\n", puc_rx_data[us_data_idx++]);
		return DAR_SUCCESS;

	case DT_LONG_UNSIGNED:
		us_aux = ((uint16_t)puc_rx_data[us_data_idx++]) << 8;
		us_aux |= puc_rx_data[us_data_idx++];
		printf("DT_LONG_UNSIGNED %hu\n", us_aux);
		return DAR_SUCCESS;

	case DT_LONG_64:
		return DAR_SUCCESS;

	case DT_LONG_64_UNSIGNED:
		return DAR_SUCCESS;

	case DT_ENUM:
		printf("DT_ENUM %hhu\n", puc_rx_data[us_data_idx++]);
		return DAR_SUCCESS;

	case DT_FLOAT_32:
		return DAR_SUCCESS;

	case DT_FLOAT_64:
		return DAR_SUCCESS;

	case DT_ARRAY:
		return DAR_SUCCESS;

	case DT_STRUCTURE:
		us_data_idx += dlms_cli_decode_a_xdr_length(&us_struct_elements, &puc_rx_data[us_data_idx]);
		printf("DT_STRUCTURE (%hu elems.): ", us_struct_elements);

		us_aux = us_length - us_data_idx;
		for (us_idx = 0; us_idx < us_aux - 1; us_idx++) {
			printf("%02X ", puc_rx_data[us_data_idx++]);
		}

		/* last byte is printed apart */
		printf("%02X\n", puc_rx_data[us_data_idx++]);
		return DAR_SUCCESS;

	case DT_COMPACT_ARRAY:
		return DAR_SUCCESS;
	}
#endif

	return DAR_SUCCESS;
}

/**
 * \brief OBIS 1-0:99.1.0.255 callback function
 * \param us_short_addr      Node short address
 * \param px_assoc_info      Association info pointer
 * \param uc_attr            Attribute
 * \param puc_rx_data        Pointer to data
 * \param us_data_len        Data length
 *
 * \retval Data access result
 */
data_access_result_t obis_1_0_99_1_0_255_example_cb(uint16_t us_short_addr, assoc_info_t *px_assoc_info, uint8_t uc_attr, uint8_t *puc_rx_data, uint16_t us_data_len)
{
	(void)us_short_addr;

	uint8_t puc_resp_data[2 * MAX_APDU_SIZE_SEND]; /* Must fit up to one complete APDU plus remaining bytes of previous block */
	uint16_t us_num_regs, us_idx;

	switch (uc_attr) {
	case IC07_LOGICAL_NAME:
		_dlms_cli_data_print_data(DT_OCTET_STRING, us_data_len, puc_rx_data);
		break;

	case IC07_BUFFER:
		us_num_regs = dlms_cli_get_buffer_regs(px_assoc_info, SIZE_LOAD_PROFILE_REG, puc_rx_data, us_data_len, puc_resp_data);

		/* Process registers */
		for (us_idx = 0; us_idx < us_num_regs; us_idx++) {
			_dlms_cli_data_print_data(DT_STRUCTURE, SIZE_LOAD_PROFILE_REG, &puc_resp_data[us_idx * SIZE_LOAD_PROFILE_REG]);
		}

		break;

	case IC07_CAPTURE_OBJECTS:
		_dlms_cli_data_print_data(DT_ARRAY, us_data_len, puc_rx_data);
		break;

	case IC07_SORT_METHOD:
		_dlms_cli_data_print_data(DT_ENUM, SIZE_ENUM, puc_rx_data);
		break;

	case IC07_SORT_OBJECT:
		_dlms_cli_data_print_data(DT_NULL_DATA, 0, puc_rx_data);
		break;

	case IC07_CAPTURE_PERIOD:
	case IC07_ENTRIES_IN_USE:
	case IC07_PROFILE_ENTRIES:
		_dlms_cli_data_print_data(DT_DOUBLE_LONG_UNSIGNED, SIZE_DOUBLE_LONG_U, puc_rx_data);
		break;

	default:
		return DAR_SCOPE_VIOLATED;
	}

	return DAR_SUCCESS;
}

/**
 * \brief OBIS 0-0:1.0.0.255 callback function
 * \param us_short_addr      Node short address
 * \param px_assoc_info      Association info pointer
 * \param uc_attr            Attribute
 * \param puc_rx_data        Pointer to data
 * \param us_data_len        Data length
 *
 * \retval Data access result
 */
data_access_result_t obis_0_0_1_0_0_255_example_cb(uint16_t us_short_addr, assoc_info_t *px_assoc_info, uint8_t uc_attr, uint8_t *puc_rx_data, uint16_t us_data_len)
{
	(void)us_short_addr;
	(void)px_assoc_info;

	switch (uc_attr) {
	case IC08_LOGICAL_NAME:
		_dlms_cli_data_print_data(DT_OCTET_STRING, us_data_len, puc_rx_data);
		break;

	case IC08_TIME:
	case IC08_DAYLIGHT_BEGIN:
	case IC08_DAYLIGHT_END:
		_dlms_cli_data_print_data(DT_OCTET_STRING, us_data_len, puc_rx_data);
		break;

	case IC08_TIME_ZONE:
		_dlms_cli_data_print_data(DT_LONG, SIZE_LONG, puc_rx_data);
		break;

	case IC08_STATUS:
		_dlms_cli_data_print_data(DT_UNSIGNED, SIZE_UNSIGNED, puc_rx_data);
		break;

	case IC08_DAYLIGHT_DEV:
		_dlms_cli_data_print_data(DT_INTEGER, SIZE_INTEGER, puc_rx_data);
		break;

	case IC08_DAYLIGHT_ENABLED:
		_dlms_cli_data_print_data(DT_BOOLEAN, SIZE_BOOLEAN, puc_rx_data);
		break;

	case IC08_CLK_BASE:
		_dlms_cli_data_print_data(DT_ENUM, SIZE_ENUM, puc_rx_data);
		break;

	default:
		return DAR_SCOPE_VIOLATED;
	}

	return DAR_SUCCESS;
}

/**
 * \brief OBIS 0-0:29.0.0.255 callback function
 * \param us_short_addr      Node short address
 * \param px_assoc_info      Association info pointer
 * \param uc_attr            Attribute
 * \param puc_rx_data        Pointer to data
 * \param us_data_len        Data length
 *
 * \retval Data access result
 */
data_access_result_t obis_0_0_29_0_0_255_example_cb(uint16_t us_short_addr, assoc_info_t *px_assoc_info, uint8_t uc_attr, uint8_t *puc_rx_data, uint16_t us_data_len)
{
	(void)us_short_addr;

        UNUSED(px_assoc_info);
          
	switch (uc_attr) {
        case IC91_LOGICAL_NAME:
		_dlms_cli_data_print_data(DT_OCTET_STRING, us_data_len, puc_rx_data);
		break;
                
	case IC90_MAC_TX_DATA_PACKET_COUNT:
	case IC90_MAC_RX_DATA_PACKET_COUNT:
	case IC90_MAC_TX_CMD_PACKET_COUNT:
	case IC90_MAC_RX_CMD_PACKET_COUNT:
	case IC90_MAC_CSMA_FAIL_COUNT:
	case IC90_MAC_CSMA_NO_ACK_COUNT:
	case IC90_MAC_BAD_CRC_COUNT:
	case IC90_MAC_TX_DATA_BROADCAST_COUNT:
	case IC90_MAC_RX_DATA_BROADCAST_COUNT:  
		_dlms_cli_data_print_data(DT_DOUBLE_LONG_UNSIGNED, us_data_len, puc_rx_data);
		break;

	default:
		return DAR_SCOPE_VIOLATED;
	}

	return DAR_SUCCESS;
}


/**
 * \brief OBIS 0-0:29.1.0.255 callback function
 * \param us_short_addr      Node short address
 * \param px_assoc_info      Association info pointer
 * \param uc_attr            Attribute
 * \param puc_rx_data        Pointer to data
 * \param us_data_len        Data length
 *
 * \retval Data access result
 */
data_access_result_t obis_0_0_29_1_0_255_example_cb(uint16_t us_short_addr, assoc_info_t *px_assoc_info, uint8_t uc_attr, uint8_t *puc_rx_data, uint16_t us_data_len)
{
	(void)us_short_addr;

	uint8_t puc_resp_data[2 * MAX_APDU_SIZE_SEND]; /* Must fit up to one complete APDU plus remaining bytes of previous block */
	uint16_t us_num_regs, us_idx;

	switch (uc_attr) {
	case IC91_LOGICAL_NAME:
	case IC91_MAC_TONE_MASK:
	case IC91_MAC_TMR_TTL:
	case IC91_MAC_MAX_FRAME_RETRIES:
	case IC91_MAC_NEIGHBOUR_TABLE_ENTRY_TTL:
	case IC91_MAC_HIGH_PRIORITY_WINDOW_SIZE:
	case IC91_MAC_CSMA_FAIRNESS_LIMIT:
	case IC91_MAC_BEACON_RANDOMIZATION_WINDOW_LENGTH:
	case IC91_MAC_A:
	case IC91_MAC_K:
	case IC91_MAC_MIN_CW_ATTEMPTS:
	case IC91_MAC_CENELEC_LEGACY_MODE:
	case IC91_MAC_FCC_LEGACY_MODE:
	case IC91_MAC_MAX_BE:
	case IC91_MAC_MAX_CSMA_BACKOFFS:
	case IC91_MAC_MIN_BE:
		_dlms_cli_data_print_data(DT_OCTET_STRING, us_data_len, puc_rx_data);
		break;

	case IC91_MAC_NEIGHBOUR_TABLE:
		us_num_regs = dlms_cli_get_buffer_regs(px_assoc_info, SIZE_MAC_NEIGHBOUR_TABLE_ENTRY, puc_rx_data, us_data_len, puc_resp_data);

		/* Process registers */
		for (us_idx = 0; us_idx < us_num_regs; us_idx++) {
			_dlms_cli_data_print_data(DT_STRUCTURE, SIZE_MAC_NEIGHBOUR_TABLE_ENTRY, &puc_resp_data[us_idx * SIZE_MAC_NEIGHBOUR_TABLE_ENTRY]);
		}

		break;

	case IC91_MAC_SHORT_ADDRESS:
	case IC91_MAC_RC_COORD:
	case IC91_MAC_PAN_ID:
		return DAR_OBJ_UNAVAIL;

	default:
		return DAR_SCOPE_VIOLATED;
	}

	return DAR_SUCCESS;
}

/**
 * \brief OBIS 0-0:29.2.0.255 callback function
 * \param us_short_addr      Node short address
 * \param px_assoc_info      Association info pointer
 * \param uc_attr            Attribute
 * \param puc_rx_data        Pointer to data
 * \param us_data_len        Data length
 *
 * \retval Data access result
 */
data_access_result_t obis_0_0_29_2_0_255_example_cb(uint16_t us_short_addr, assoc_info_t *px_assoc_info, uint8_t uc_attr, uint8_t *puc_rx_data, uint16_t us_data_len)
{
	(void)us_short_addr;

	uint8_t puc_resp_data[2 * MAX_APDU_SIZE_SEND]; /* Must fit up to one complete APDU plus remaining bytes of previous block */
	uint16_t us_num_regs, us_idx;

	switch (uc_attr) {
	case IC92_LOGICAL_NAME:
	case IC92_ADP_MAX_HOPS:
	case IC92_ADP_WEAK_LQI_VALUE:
	case IC92_ADP_SECURITY_LEVEL:
	case IC92_ADP_BROADCAST_LOG_TABLE_ENTRY_TTL:
	case IC92_ADP_MAX_JOIN_WAIT_TIME:
	case IC92_ADP_PATH_DISCOVERY_TIME:
	case IC92_ADP_ACTIVE_KEY_INDEX:
	case IC92_ADP_METRIC_TYPE:
		_dlms_cli_data_print_data(DT_OCTET_STRING, us_data_len, puc_rx_data);
		break;

	case IC92_ADP_ROUTING_TABLE:
		us_num_regs = dlms_cli_get_buffer_regs(px_assoc_info, SIZE_ADP_ROUTING_PROFILE_ENTRY, puc_rx_data, us_data_len, puc_resp_data);

		/* Process registers */
		for (us_idx = 0; us_idx < us_num_regs; us_idx++) {
			_dlms_cli_data_print_data(DT_STRUCTURE, SIZE_ADP_ROUTING_PROFILE_ENTRY, &puc_resp_data[us_idx * SIZE_ADP_ROUTING_PROFILE_ENTRY]);
		}

		break;

	case IC92_ADP_PREFIX_TABLE:
	case IC92_ADP_ROUTING_CONFIGURATION:
	case IC92_ADP_CONTEXT_INFORMATION_TABLE:
	case IC92_ADP_BLACKLIST_TABLE:
	case IC92_ADP_BROADCAST_LOG_TABLE:
	case IC92_ADP_GROUP_TABLE:
	case IC92_ADP_COORD_SHORT_ADDRESS:
	case IC92_ADP_DISABLE_DEFAULT_ROUTING:
	case IC92_ADP_DEVICE_TYPE:
		return DAR_OBJ_UNAVAIL;

	default:
		return DAR_SCOPE_VIOLATED;
	}

	return DAR_SUCCESS;
}
