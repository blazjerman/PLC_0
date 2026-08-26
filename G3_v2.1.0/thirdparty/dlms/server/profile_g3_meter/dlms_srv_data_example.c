/**
 * \file
 *
 * \brief DLMS_SRV_LIB : DLMS server lib: G3 Profile
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

#include "dlms_srv_lib.h"
#include "dlms_srv_data_example.h"

static meter_params_t *spx_meter_params;
extern uint8_t g_auc_ext_address[8];     /* EUI64 */

/** ADP_IB_ROUTING_TABLE format
 * Bytes 0 - 1: indicate the destination address
 * Bytes 2 - 3: indicate the next hop address
 * Bytes 4 - 5: indicate the route cost
 * Byte 6, bits 0 - 3: indicate the weak links count
 * Byte 6, bits 4 - 7: indicate the hop count
 * Bytes 7 - 8: indicate the time this entry is valid in minutes
 */
struct RoutingTableEntry {
	uint16_t m_u16DstAddr;
	uint16_t m_u16NextHopAddr;
	uint16_t m_u16RouteCost;
	uint8_t m_u8WeakLinkCount : 4;
	uint8_t m_u8HopCount : 4;
	uint16_t m_u16ValidTime;
};


/** Dummy load profile to fill load profile register request */
static load_profile_reg_t dummy_load_prof_reg = {
	{0x07, 0xE0, 0x08, 0x06, 0x06, 0x08, 0x0F, 0x00, 0x00, 0x80, 0x00, 0x00},
	0x00,
	0xA1A1A1A1, 0xB2B2B2B2, 0xC3C3C3C3, 0xD4D4D4D4, 0xE5E5E5E5, 0xF6F6F6F6
};

#if  !defined(__G3_GATEWAY__)
static const uint8_t days_per_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/**
 * \brief Read date in the reception buffer and get how many fields have to be send in the S02 profile
 *
 * \param puc_data      Pointer to received data
 *
 * \retval date
 */
static uint16_t _get_number_of_S02(assoc_info_t *px_assoc_info /*, uint8_t *puc_data*/)
{
	uint32_t iniday, endday;
	uint8_t *puc_ini, *puc_end;

	if ((px_assoc_info->access_selector.selector != SEL_IC07) ||
			(px_assoc_info->access_selector.ic07_range.from.type != DT_OCTET_STRING) ||
			(px_assoc_info->access_selector.ic07_range.to.type != DT_OCTET_STRING) ||
			(px_assoc_info->access_selector.ic07_range.from.length != SIZE_DATE_TIME) ||
			(px_assoc_info->access_selector.ic07_range.to.length != SIZE_DATE_TIME)) {
		return 0;
	}

	puc_ini = px_assoc_info->access_selector.ic07_range.from.value;
	puc_end = px_assoc_info->access_selector.ic07_range.to.value;

	/* calculation have know accuracy errors ... assume all months
	 * are 30 days and leap year is not corrected*/
	endday = (puc_end[0] * 256 + puc_end[1]) * 365 * 24 + puc_end[2] * 30 * 24 + puc_end[3] * 24 + puc_end[5];
	iniday = (puc_ini[0] * 256 + puc_ini[1]) * 365 * 24 + puc_ini[2] * 30 * 24 + puc_ini[3] * 24 + puc_ini[5];
	/* update msg puc_obis */
	dummy_load_prof_reg.puc_date_time [6] = 0;
	/* init hour register  */
	dummy_load_prof_reg.puc_date_time [5] = puc_ini[5];
	/* week day */
	dummy_load_prof_reg.puc_date_time [4] = puc_ini[4];
	/* init register day */
	dummy_load_prof_reg.puc_date_time [3] = puc_ini[3];
	/* init register month convert to hex from bcd */
	dummy_load_prof_reg.puc_date_time [2] = puc_ini[2];
	/* init register year */
	dummy_load_prof_reg.puc_date_time [1] = puc_ini[1];
	dummy_load_prof_reg.puc_date_time [0] = puc_ini[0];

	return (uint16_t)(endday - iniday);
}

/**
 * \brief timestamp field increment 1 hour. A single calendar implementation.
 */
static void _update_date(void)
{
	/* increment one hour	*/
	dummy_load_prof_reg.puc_date_time[5]++;
	if (dummy_load_prof_reg.puc_date_time[5] > 23) {
		/* start new day at 00H */
		dummy_load_prof_reg.puc_date_time[5] = 0;
		/* next day of week	*/
		dummy_load_prof_reg.puc_date_time[4]++;
		if (dummy_load_prof_reg.puc_date_time[4] > 7) {
			/* sunday to monday */
			dummy_load_prof_reg.puc_date_time[4] = 1;
		}

		/* new day */
		dummy_load_prof_reg.puc_date_time[3]++;
		/* change month	*/
		if (dummy_load_prof_reg.puc_date_time[3] > days_per_month[dummy_load_prof_reg.puc_date_time[2] - 1]) {
			dummy_load_prof_reg.puc_date_time[3] = 1;
			dummy_load_prof_reg.puc_date_time[2]++;
			/* change year */
			if (dummy_load_prof_reg.puc_date_time[2] > 12) {
				dummy_load_prof_reg.puc_date_time[2] = 1;
				dummy_load_prof_reg.puc_date_time[1]++;
			}
		}
	}
}

data_access_result_t obis_1_0_1_8_x_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	obis_element_conf_t x_requested_obis;
	uint8_t puc_resp_data[MAX_APDU_SIZE_SEND];
	uint8_t uc_resp_data_idx = 0;

	x_requested_obis = dlms_srv_get_obis_from_idx(px_assoc_info->current_req.obis_idx);

	switch (uc_attr) {
	case IC03_LOGICAL_NAME:
		memcpy(puc_resp_data, x_requested_obis.obis_code, SIZE_LN);
		break;

	case IC03_VALUE:
		switch (x_requested_obis.obis_code[4]) {
		case 0:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 255:
			puc_resp_data[uc_resp_data_idx++] = DT_DOUBLE_LONG_UNSIGNED;
			puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_act_import >> 24) & 0xFF;
			puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_act_import >> 16) & 0xFF;
			puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_act_import >> 8) & 0xFF;
			puc_resp_data[uc_resp_data_idx++] = dummy_load_prof_reg.us_act_import & 0xFF;
			break;

		default:
			return DAR_OTHER_REASON;
		}

		break;

	case IC03_SCALER_UNIT:
		puc_resp_data[uc_resp_data_idx++] = 0;  /* Scaler: 0 */
		puc_resp_data[uc_resp_data_idx++] = 30; /* Units: Wh */
		break;

	default:
		return DAR_OTHER_REASON;
	}

	dlms_srv_encode_ic03(px_assoc_info, (ic03_attr_t)uc_attr, (void *)puc_resp_data, uc_resp_data_idx);
	return DAR_SUCCESS;
}

static uint8_t strlen_with_max(char *p, uint8_t max)
{
	uint8_t ret = 0;

	while (*p++) {
		ret++;
		if (ret >= max) {
			ret = max;
			break;
		}
	}

	return ret;
}

data_access_result_t obis_0_0_96_1_x_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	uint8_t puc_resp_data[MAX_APDU_SIZE_SEND];
	uint8_t uc_resp_data_idx = 0;
	/* Default device ID 1 = "0123456789" */
	uint8_t puc_def_dev_id_1[10] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
	/* Device ID 2 = UNESA " A" + Model: "02" + Year "16" */
	uint8_t puc_def_dev_id_2[6] = {0x20, 0x41, 0x30, 0x32, 0x31, 0x36};
	/* Device ID 3 = "contador  " + "DLMS" + "0106" */
	uint8_t puc_def_dev_id_3[18] = {0x63, 0x6f, 0x6e, 0x74, 0x61, 0x64, 0x6f, 0x72, 0x20, 0x20, 0x44, 0x4c, 0x4d, 0x53, 0x30, 0x31, 0x30, 0x36};

	obis_element_conf_t x_requested_obis;

	x_requested_obis = dlms_srv_get_obis_from_idx(px_assoc_info->current_req.obis_idx);

	switch (uc_attr) {
	case IC01_LOGICAL_NAME:
		memcpy(puc_resp_data, x_requested_obis.obis_code, SIZE_LN);
		break;

	case IC01_VALUE:
		puc_resp_data[uc_resp_data_idx++] = DT_OCTET_STRING;

		switch (x_requested_obis.obis_code[4]) {
		case 0: /* Device ID 1 */
			/* Encode octet string length */
			uc_resp_data_idx += dlms_srv_encode_a_xdr_length(10, &puc_resp_data[uc_resp_data_idx]);
			/* Copy octet string */
			if (spx_meter_params != NULL) { /* Check if meter serial is defined */
				if (strlen_with_max((char *)spx_meter_params->meter_serial, 14) == 13) { /* Check if meter serial is correct */
					memcpy(&puc_resp_data[uc_resp_data_idx], &spx_meter_params->meter_serial[3], 10);
					uc_resp_data_idx += 10;
					break;
				}
			}

			/* Bad definition of meter serial or no definition, send dummy serial */
			memcpy(&puc_resp_data[uc_resp_data_idx], puc_def_dev_id_1, 10);
			uc_resp_data_idx += 10;
			break;

		case 1: /* Device ID 2 */
			/* Encode octet string length */
			uc_resp_data_idx += dlms_srv_encode_a_xdr_length(6, &puc_resp_data[uc_resp_data_idx]);
			/* Copy octet string */
			memcpy(&puc_resp_data[uc_resp_data_idx], puc_def_dev_id_2, 6);
			uc_resp_data_idx += 6;
			break;

		case 2: /* Device ID 3 */
			/* Encode octet string length */
			uc_resp_data_idx += dlms_srv_encode_a_xdr_length(18, &puc_resp_data[uc_resp_data_idx]);
			/* Copy octet string */
			memcpy(&puc_resp_data[uc_resp_data_idx], puc_def_dev_id_3, 18);
			uc_resp_data_idx += 18;
			break;

		default:
			return DAR_OTHER_REASON;
		}
		break;

	default:
		return DAR_OTHER_REASON;
	}

	dlms_srv_encode_ic01(px_assoc_info, (ic01_attr_t)uc_attr, (void *)puc_resp_data, uc_resp_data_idx);
	return DAR_SUCCESS;
}

data_access_result_t obis_0_0_28_7_0_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	uint8_t puc_resp_data[MAX_APDU_SIZE_SEND];
	uint8_t uc_resp_data_idx = 0;
	/* Default device ID 1 = "0123456789" */

	obis_element_conf_t x_requested_obis;

	x_requested_obis = dlms_srv_get_obis_from_idx(px_assoc_info->current_req.obis_idx);

	switch (uc_attr) {
	case IC86_LOGICAL_NAME:
		memcpy(puc_resp_data, x_requested_obis.obis_code, SIZE_LN);
		break;

	case IC86_FW_VERSION:
		puc_resp_data[uc_resp_data_idx++] = DT_OCTET_STRING;
		/* Encode octet string length */
		uc_resp_data_idx += dlms_srv_encode_a_xdr_length(LEN_ID_PIB_FW_VERSION, &puc_resp_data[uc_resp_data_idx]);
		/* Copy octet string */
		memcpy(&puc_resp_data[uc_resp_data_idx], spx_meter_params->pib_fw_version, LEN_ID_PIB_FW_VERSION);
		uc_resp_data_idx += LEN_ID_PIB_FW_VERSION;
		break;

	case IC86_VENDOR_ID:
		puc_resp_data[uc_resp_data_idx++] = DT_LONG_UNSIGNED;
		/* Encode value */
		puc_resp_data[uc_resp_data_idx++] = spx_meter_params->pib_vendor_id[0];
		puc_resp_data[uc_resp_data_idx++] = spx_meter_params->pib_vendor_id[1];
		break;

	case IC86_PRODUCT_ID:
		puc_resp_data[uc_resp_data_idx++] = DT_LONG_UNSIGNED;
		/* Encode value */
		puc_resp_data[uc_resp_data_idx++] = spx_meter_params->pib_product_id[0];
		puc_resp_data[uc_resp_data_idx++] = spx_meter_params->pib_product_id[1];
		break;

	default:
		return DAR_OTHER_REASON;
	}

	dlms_srv_encode_ic86(px_assoc_info, (ic86_attr_t)uc_attr, (void *)puc_resp_data, uc_resp_data_idx);
	return DAR_SUCCESS;
}

data_access_result_t obis_0_0_21_0_5_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	uint8_t puc_logical_name[SIZE_LN] = {0x00, 0x00, 0x15, 0x00, 0x05, 0xFF};
	uint8_t puc_capture_objects[] = {0x01, 0x0D, /* array --- 13 elements*/
		                         /* structure 4 elements -->	IC (long-unsigned 18) / OBIS (octet-string 9) / ATTR (integer 17) / INDEX (long-unsigned 18)*/
					 0x02, 0x04, 0x12, 0x00, 0x08, 0x09, 0x06, 0x00, 0x00, 0x01, 0x00, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x01,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x20, 0x07, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x02,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x1F, 0x07, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x03,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x34, 0x07, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x04,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x33, 0x07, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x05,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x48, 0x07, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x06,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x47, 0x07, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x07,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x01, 0x07, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x08,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x01, 0x07, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x09,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x02, 0x07, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x0A,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x03, 0x07, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x0B,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x04, 0x07, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x0C,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x0D, 0x07, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x0D};
	uint8_t puc_resp_data[MAX_APDU_SIZE_SEND];
	uint8_t uc_resp_data_idx = 0;

	switch (uc_attr) {
	case IC07_LOGICAL_NAME:
		memcpy(puc_resp_data, puc_logical_name, SIZE_LN);
		break;

	case IC07_BUFFER:
		/* Start: add array TAG and number of elements */
		puc_resp_data[uc_resp_data_idx++] = DT_ARRAY;
		puc_resp_data[uc_resp_data_idx++] = 0X01; /* one element */

		/* Copy registers to data packet */
		puc_resp_data[uc_resp_data_idx++] = DT_STRUCTURE;
		puc_resp_data[uc_resp_data_idx++] = 13; /* struct num elements */

		puc_resp_data[uc_resp_data_idx++] = DT_OCTET_STRING;
		puc_resp_data[uc_resp_data_idx++] = SIZE_DATE_TIME; /* num elements */
		memcpy(&puc_resp_data[uc_resp_data_idx], dummy_load_prof_reg.puc_date_time, SIZE_DATE_TIME);
		uc_resp_data_idx += SIZE_DATE_TIME;

		puc_resp_data[uc_resp_data_idx++] = DT_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (0xAA00 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 0xAA00 & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (0xBB00 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 0xBB00 & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (0xCC00 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 0xCC00 & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (0xDD00 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 0xDD00 & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (0xEE00 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 0xEE00 & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (0xFF00 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 0xFF00 & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (0x1100 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 0x1100 & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_DOUBLE_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (0XAA00AA00 >> 24) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (0XAA00AA00 >> 16) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (0XAA00AA00 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 0XAA00AA00 & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_DOUBLE_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (0XBB00BB00 >> 24) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (0XBB00BB00 >> 16) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (0XBB00BB00 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 0XBB00BB00 & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_DOUBLE_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (0XCC00CC00 >> 24) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (0XCC00CC00 >> 16) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (0XCC00CC00 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 0XCC00CC00 & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_DOUBLE_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (0XDD00DD00 >> 24) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (0XDD00DD00 >> 16) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (0XDD00DD00 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 0XDD00DD00 & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (0x2200 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 0x2200 & 0xFF;
		break;

	case IC07_CAPTURE_OBJECTS:
		memcpy(puc_resp_data, puc_capture_objects, sizeof(puc_capture_objects));
		uc_resp_data_idx = sizeof(puc_capture_objects);
		break;

	case IC07_CAPTURE_PERIOD:
		puc_resp_data[uc_resp_data_idx++] = 0;
		puc_resp_data[uc_resp_data_idx++] = 0;
		puc_resp_data[uc_resp_data_idx++] = 0;
		puc_resp_data[uc_resp_data_idx++] = 0;
		break;

	case IC07_SORT_METHOD:
		puc_resp_data[uc_resp_data_idx++] = 1;
		break;

	case IC07_SORT_OBJECT:
		puc_resp_data[uc_resp_data_idx++] = DT_NULL_DATA;
		break;

	case IC07_ENTRIES_IN_USE:
		puc_resp_data[uc_resp_data_idx++] = (1 >> 24) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (1 >> 16) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (1 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 1 & 0xFF;
		break;

	case IC07_PROFILE_ENTRIES:
		puc_resp_data[uc_resp_data_idx++] = (1 >> 24) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (1 >> 16) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (1 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 1 & 0xFF;
		break;

	default:
		return DAR_SCOPE_VIOLATED;
	}

	dlms_srv_encode_ic07(px_assoc_info, (ic07_attr_t)uc_attr, (void *)puc_resp_data, uc_resp_data_idx);
	return DAR_SUCCESS;
}

data_access_result_t obis_0_0_21_0_6_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	uint8_t puc_logical_name[SIZE_LN] = {0x00, 0x00, 0x15, 0x00, 0x06, 0xFF};
	uint8_t puc_capture_objects[] = {0x01, 0x0D, /* array --- 6 elements*/
		                         /* structure 4 elements -->	IC (long-unsigned 18) / OBIS (octet-string 9) / ATTR (integer 17) / INDEX (long-unsigned 18)*/
					 0x02, 0x04, 0x12, 0x00, 0x08, 0x09, 0x06, 0x00, 0x00, 0x01, 0x00, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x01,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x01, 0x08, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x02,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x02, 0x08, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x03,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x05, 0x08, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x04,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x06, 0x08, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x05,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x07, 0x08, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x06,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x08, 0x08, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x07};
	uint8_t puc_resp_data[MAX_APDU_SIZE_SEND];
	uint8_t uc_resp_data_idx = 0;

	switch (uc_attr) {
	case IC07_LOGICAL_NAME:
		memcpy(puc_resp_data, puc_logical_name, SIZE_LN);
		break;

	case IC07_BUFFER:
		/* Start: add array TAG and number of elements */
		puc_resp_data[uc_resp_data_idx++] = DT_ARRAY;
		puc_resp_data[uc_resp_data_idx++] = 0X01; /* one element */

		/* Copy registers to data packet */
		puc_resp_data[uc_resp_data_idx++] = DT_STRUCTURE;
		puc_resp_data[uc_resp_data_idx++] = 7; /* struct num elements */

		puc_resp_data[uc_resp_data_idx++] = DT_OCTET_STRING;
		puc_resp_data[uc_resp_data_idx++] = SIZE_DATE_TIME; /* num elements */
		memcpy(&puc_resp_data[uc_resp_data_idx], dummy_load_prof_reg.puc_date_time, SIZE_DATE_TIME);
		uc_resp_data_idx += SIZE_DATE_TIME;

		puc_resp_data[uc_resp_data_idx++] = DT_DOUBLE_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_act_import >> 24) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_act_import >> 16) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_act_import >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = dummy_load_prof_reg.us_act_import & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_DOUBLE_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_act_export >> 24) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_act_export >> 16) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_act_export >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = dummy_load_prof_reg.us_act_export & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_DOUBLE_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_i >> 24) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_i >> 16) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_i >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = dummy_load_prof_reg.us_react_q_i & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_DOUBLE_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_ii >> 24) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_ii >> 16) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_ii >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = dummy_load_prof_reg.us_react_q_ii & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_DOUBLE_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_iii >> 24) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_iii >> 16) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_iii >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = dummy_load_prof_reg.us_react_q_iii & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_DOUBLE_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_iv >> 24) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_iv >> 16) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_iv >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = dummy_load_prof_reg.us_react_q_iv & 0xFF;

		break;

	case IC07_CAPTURE_OBJECTS:
		memcpy(puc_resp_data, puc_capture_objects, sizeof(puc_capture_objects));
		uc_resp_data_idx = sizeof(puc_capture_objects);
		break;

	case IC07_CAPTURE_PERIOD:
		puc_resp_data[uc_resp_data_idx++] = 0;
		puc_resp_data[uc_resp_data_idx++] = 0;
		puc_resp_data[uc_resp_data_idx++] = 0;
		puc_resp_data[uc_resp_data_idx++] = 0;
		break;

	case IC07_SORT_METHOD:
		puc_resp_data[uc_resp_data_idx++] = 1;
		break;

	case IC07_SORT_OBJECT:
		puc_resp_data[uc_resp_data_idx++] = DT_NULL_DATA;
		break;

	case IC07_ENTRIES_IN_USE:
		puc_resp_data[uc_resp_data_idx++] = (1 >> 24) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (1 >> 16) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (1 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 1 & 0xFF;
		break;

	case IC07_PROFILE_ENTRIES:
		puc_resp_data[uc_resp_data_idx++] = (1 >> 24) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (1 >> 16) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (1 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 1 & 0xFF;
		break;

	default:
		return DAR_SCOPE_VIOLATED;
	}

	dlms_srv_encode_ic07(px_assoc_info, (ic07_attr_t)uc_attr, (void *)puc_resp_data, uc_resp_data_idx);
	return DAR_SUCCESS;
}

data_access_result_t obis_1_0_99_1_0_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	static uint16_t us_pending_regs;
	uint8_t puc_logical_name[SIZE_LN] = {0x01, 0x00, 0x63, 0x01, 0x00, 0xFF};
	uint8_t puc_capture_objects[] = {0x01, 0x08, /* array --- 8 elements*/
		                         /* structure 4 elements -->	IC (long-unsigned 18) / OBIS (octet-string 9) / ATTR (integer 17) / INDEX (long-unsigned 18)*/
					 0x02, 0x04, 0x12, 0x00, 0x08, 0x09, 0x06, 0x00, 0x00, 0x01, 0x00, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x01,
					 0x02, 0x04, 0x12, 0x00, 0x01, 0x09, 0x06, 0x00, 0x00, 0x60, 0x0A, 0x07, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x02,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x01, 0x00, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x03,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x02, 0x00, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x04,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x05, 0x00, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x05,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x06, 0x00, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x06,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x07, 0x00, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x07,
					 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, 0x01, 0x00, 0x08, 0x00, 0x00, 0xFF, 0x11, 0x02, 0x12, 0x00, 0x08};
	uint8_t puc_resp_data[MAX_APDU_SIZE_SEND];
	uint8_t uc_idx, uc_resp_data_idx = 0;
	uint8_t uc_reg_per_block = px_assoc_info->max_apdu_size / SIZE_LOAD_PROFILE_REG;

	switch (uc_attr) {
	case IC07_LOGICAL_NAME:
		memcpy(puc_resp_data, puc_logical_name, SIZE_LN);
		break;

	case IC07_BUFFER:
		if (px_assoc_info->long_get.state == DLMS_ONE_BLOCK) {
			/* Get total number of registers */
			us_pending_regs = _get_number_of_S02(px_assoc_info);

			/* Check if all registers fit in one block */
			if (us_pending_regs > uc_reg_per_block) {
				/* Initialize get with block process block numbering */
				px_assoc_info->long_get.state = DLMS_FIRST_BLOCK;
				px_assoc_info->long_get.block = 1;
			}

			/* Start: add array TAG and number of elements */
			puc_resp_data[uc_resp_data_idx++] = DT_ARRAY;
			uc_resp_data_idx += dlms_srv_encode_a_xdr_length(us_pending_regs, &puc_resp_data[uc_resp_data_idx]);
		}

		/* Check if there are no regs to send */
		if (!us_pending_regs) {
			break;
		}

		/* Copy registers to data packet */
		for (uc_idx = 0; uc_idx < uc_reg_per_block; uc_idx++) {
			us_pending_regs--;

			puc_resp_data[uc_resp_data_idx++] = DT_STRUCTURE;
			puc_resp_data[uc_resp_data_idx++] = 8; /* struct num elements */

			puc_resp_data[uc_resp_data_idx++] = DT_OCTET_STRING;
			puc_resp_data[uc_resp_data_idx++] = SIZE_DATE_TIME; /* num elements */
			memcpy(&puc_resp_data[uc_resp_data_idx], dummy_load_prof_reg.puc_date_time, SIZE_DATE_TIME);
			uc_resp_data_idx += SIZE_DATE_TIME;

			_update_date(); /* fake increase one hour date-time */

			puc_resp_data[uc_resp_data_idx++] = DT_UNSIGNED;
			puc_resp_data[uc_resp_data_idx++] = dummy_load_prof_reg.uc_amr_status;

			puc_resp_data[uc_resp_data_idx++] = DT_DOUBLE_LONG_UNSIGNED;
			puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_act_import >> 24) & 0xFF;
			puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_act_import >> 16) & 0xFF;
			puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_act_import >> 8) & 0xFF;
			puc_resp_data[uc_resp_data_idx++] = dummy_load_prof_reg.us_act_import & 0xFF;

			puc_resp_data[uc_resp_data_idx++] = DT_DOUBLE_LONG_UNSIGNED;
			puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_act_export >> 24) & 0xFF;
			puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_act_export >> 16) & 0xFF;
			puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_act_export >> 8) & 0xFF;
			puc_resp_data[uc_resp_data_idx++] = dummy_load_prof_reg.us_act_export & 0xFF;

			puc_resp_data[uc_resp_data_idx++] = DT_DOUBLE_LONG_UNSIGNED;
			puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_i >> 24) & 0xFF;
			puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_i >> 16) & 0xFF;
			puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_i >> 8) & 0xFF;
			puc_resp_data[uc_resp_data_idx++] = dummy_load_prof_reg.us_react_q_i & 0xFF;

			puc_resp_data[uc_resp_data_idx++] = DT_DOUBLE_LONG_UNSIGNED;
			puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_ii >> 24) & 0xFF;
			puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_ii >> 16) & 0xFF;
			puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_ii >> 8) & 0xFF;
			puc_resp_data[uc_resp_data_idx++] = dummy_load_prof_reg.us_react_q_ii & 0xFF;

			puc_resp_data[uc_resp_data_idx++] = DT_DOUBLE_LONG_UNSIGNED;
			puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_iii >> 24) & 0xFF;
			puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_iii >> 16) & 0xFF;
			puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_iii >> 8) & 0xFF;
			puc_resp_data[uc_resp_data_idx++] = dummy_load_prof_reg.us_react_q_iii & 0xFF;

			puc_resp_data[uc_resp_data_idx++] = DT_DOUBLE_LONG_UNSIGNED;
			puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_iv >> 24) & 0xFF;
			puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_iv >> 16) & 0xFF;
			puc_resp_data[uc_resp_data_idx++] = (dummy_load_prof_reg.us_react_q_iv >> 8) & 0xFF;
			puc_resp_data[uc_resp_data_idx++] = dummy_load_prof_reg.us_react_q_iv & 0xFF;

			/* Check if it is last block */
			if (!us_pending_regs) {
				if (px_assoc_info->long_get.state == DLMS_NEXT_BLOCK) {
					px_assoc_info->long_get.state = DLMS_LAST_BLOCK;
				}

				break;
			}
		}

		break;

	case IC07_CAPTURE_OBJECTS:
		memcpy(puc_resp_data, puc_capture_objects, sizeof(puc_capture_objects));
		uc_resp_data_idx = sizeof(puc_capture_objects);
		break;

	case IC07_CAPTURE_PERIOD:
		puc_resp_data[uc_resp_data_idx++] = (3600 >> 24) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (3600 >> 16) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (3600 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 3600 & 0xFF;
		break;

	case IC07_SORT_METHOD:
		puc_resp_data[uc_resp_data_idx++] = 1;
		break;

	case IC07_SORT_OBJECT:
		puc_resp_data[uc_resp_data_idx++] = DT_NULL_DATA;
		break;

	case IC07_ENTRIES_IN_USE:
		puc_resp_data[uc_resp_data_idx++] = (666 >> 24) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (666 >> 16) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (666 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 666 & 0xFF;
		break;

	case IC07_PROFILE_ENTRIES:
		puc_resp_data[uc_resp_data_idx++] = (2160 >> 24) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (2160 >> 16) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (2160 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 2160 & 0xFF;
		break;

	default:
		return DAR_SCOPE_VIOLATED;
	}

	dlms_srv_encode_ic07(px_assoc_info, (ic07_attr_t)uc_attr, (void *)puc_resp_data, uc_resp_data_idx);
	return DAR_SUCCESS;
}

#else /* !defined(__G3_GATEWAY__) */

static x_dev_addr spx_addr_list_snapshot[DLMS_MAX_DEV_NUM];

data_access_result_t obis_0_0_29_2_1_128_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	struct TAdpGetConfirm x_get_confirm;
	uint8_t puc_logical_name[SIZE_LN] = {0x00, 0x00, 0x1D, 0x02, 0x01, 0x80};
	uint8_t puc_resp_data[MAX_APDU_SIZE_SEND];
	uint8_t uc_resp_data_idx = 0;

	switch (uc_attr) {
	case IC01_LOGICAL_NAME:
		memcpy(puc_resp_data, puc_logical_name, SIZE_LN);
		break;

	case IC01_VALUE:
		/* Get short address */
		puc_resp_data[uc_resp_data_idx++] = DT_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = 0;
		puc_resp_data[uc_resp_data_idx++] = 0;
		break;

	default:
		return DAR_SCOPE_VIOLATED;
	}

	dlms_srv_encode_ic01(px_assoc_info, (ic01_attr_t)uc_attr, (void *)puc_resp_data, uc_resp_data_idx);
	return DAR_SUCCESS;
}

data_access_result_t obis_0_0_29_2_3_128_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	static uint16_t us_pending_regs;
	uint8_t puc_logical_name[SIZE_LN] = {0x00, 0x00, 0x1D, 0x02, 0x03, 0x80};
	uint8_t puc_capture_objects[] = {0x01, 0x02, /* array --- 2 elements*/
		                         /* structure 4 elements -->	IC (long-unsigned 18) / OBIS (octet-string 9) / ATTR (integer 17) / INDEX (long-unsigned 18)*/
					 0x02, 0x04, 0x12, 0x00, 0x01, 0x09, 0x06, 0x00, 0x00, 0x1D, 0x02, 0x01, 0x80, 0x11, 0x02, 0x12, 0x00, 0x01,
					 0x02, 0x04, 0x12, 0x00, 0x01, 0x09, 0x06, 0x00, 0x00, 0x1D, 0x02, 0x02, 0x80, 0x11, 0x02, 0x12, 0x00, 0x02};
	uint8_t puc_resp_data[MAX_APDU_SIZE_SEND];
	uint8_t uc_idx, uc_resp_data_idx = 0;
	uint8_t uc_reg_per_block = px_assoc_info->max_apdu_size / SIZE_EXTENDED_ADDR_TABLE_ENTRY;

	switch (uc_attr) {
	case IC07_LOGICAL_NAME:
		memcpy(puc_resp_data, puc_logical_name, SIZE_LN);
		break;

	case IC07_BUFFER:
		if (px_assoc_info->long_get.state == DLMS_ONE_BLOCK) {
			uint16_t us_index;
			us_pending_regs = 0;

			/* Get total number of registers */
			for (us_index = 0; us_index < spx_meter_params->us_max_num_devices; us_index++) {
				if (spx_meter_params->px_current_addr_list[us_index].us_short_addr != 0) {
					spx_addr_list_snapshot[us_pending_regs].us_short_addr = spx_meter_params->px_current_addr_list[us_index].us_short_addr;
					memcpy(spx_addr_list_snapshot[us_pending_regs].puc_ext_addr, spx_meter_params->px_current_addr_list[us_index].puc_ext_addr, 8);
					us_pending_regs++;
				}
			}

			/* Check if all registers fit in one block */
			if (us_pending_regs > uc_reg_per_block) {
				/* Initialize get with block process block numbering */
				px_assoc_info->long_get.state = DLMS_FIRST_BLOCK;
				px_assoc_info->long_get.block = 1;
			}

			/* Start: add array TAG and number of elements */
			puc_resp_data[uc_resp_data_idx++] = DT_ARRAY;
			uc_resp_data_idx += dlms_srv_encode_a_xdr_length(us_pending_regs, &puc_resp_data[uc_resp_data_idx]);
		}

		/* Check if there are no regs to send */
		if (!us_pending_regs) {
			break;
		}

		/* Copy registers to data packet */
		for (uc_idx = 0; uc_idx < uc_reg_per_block; uc_idx++) {
			us_pending_regs--;

			puc_resp_data[uc_resp_data_idx++] = DT_STRUCTURE;
			puc_resp_data[uc_resp_data_idx++] = 2; /* struct num elements */

			/* Short address */
			puc_resp_data[uc_resp_data_idx++] = DT_LONG_UNSIGNED;
			puc_resp_data[uc_resp_data_idx++] = (spx_addr_list_snapshot[us_pending_regs].us_short_addr >> 8) & 0xFF;
			puc_resp_data[uc_resp_data_idx++] = spx_addr_list_snapshot[us_pending_regs].us_short_addr & 0xFF;

			/* Extended address */
			puc_resp_data[uc_resp_data_idx++] = DT_OCTET_STRING;
			puc_resp_data[uc_resp_data_idx++] = 8; /* octet string num elements */
			memcpy(&puc_resp_data[uc_resp_data_idx], spx_addr_list_snapshot[us_pending_regs].puc_ext_addr, 8);
			uc_resp_data_idx += 8;

			/* Check if it is last block */
			if (!us_pending_regs) {
				if (px_assoc_info->long_get.state == DLMS_NEXT_BLOCK) {
					px_assoc_info->long_get.state = DLMS_LAST_BLOCK;
				}

				break;
			}
		}

		break;

	case IC07_CAPTURE_OBJECTS:
		memcpy(puc_resp_data, puc_capture_objects, sizeof(puc_capture_objects));
		uc_resp_data_idx = sizeof(puc_capture_objects);
		break;

	case IC07_CAPTURE_PERIOD: /* Asynchronous trigger */
		puc_resp_data[uc_resp_data_idx++] = 0;
		puc_resp_data[uc_resp_data_idx++] = 0;
		puc_resp_data[uc_resp_data_idx++] = 0;
		puc_resp_data[uc_resp_data_idx++] = 0;
		break;

	case IC07_SORT_METHOD: /* unsorted */
		puc_resp_data[uc_resp_data_idx++] = 1;
		break;

	case IC07_SORT_OBJECT: /* unsorted */
		puc_resp_data[uc_resp_data_idx++] = DT_NULL_DATA;
		break;

	case IC07_ENTRIES_IN_USE:
		/* get number of devices */
		puc_resp_data[uc_resp_data_idx++] = (*(spx_meter_params->pus_current_num_devices) >> 24) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (*(spx_meter_params->pus_current_num_devices) >> 16) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (*(spx_meter_params->pus_current_num_devices) >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = *(spx_meter_params->pus_current_num_devices) & 0xFF;
		break;

	case IC07_PROFILE_ENTRIES:
		puc_resp_data[uc_resp_data_idx++] = (2000 >> 24) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (2000 >> 16) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (2000 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 2000 & 0xFF;
		break;

	default:
		return DAR_SCOPE_VIOLATED;
	}

	dlms_srv_encode_ic07(px_assoc_info, (ic07_attr_t)uc_attr, (void *)puc_resp_data, uc_resp_data_idx);
	return DAR_SUCCESS;
}

#endif  /* !defined(__G3_GATEWAY__) */

data_access_result_t obis_0_0_29_0_0_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	uint8_t puc_logical_name[SIZE_LN] = {0x00, 0x00, 0x1D, 0x00, 0x00, 0xFF};
	uint8_t puc_resp_data[MAX_APDU_SIZE_SEND];
	uint8_t uc_resp_data_idx = 0;


	switch (uc_attr) {
	case IC90_LOGICAL_NAME:
		memcpy(puc_resp_data, puc_logical_name, SIZE_LN);
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
          puc_resp_data[uc_resp_data_idx++] = 0x0;
          puc_resp_data[uc_resp_data_idx++] = 0x0;
          puc_resp_data[uc_resp_data_idx++] = 0x0;
          puc_resp_data[uc_resp_data_idx++] = 0x0;
          break;

	default:
		return DAR_SCOPE_VIOLATED;
	}

	dlms_srv_encode_ic90(px_assoc_info, (ic91_attr_t)uc_attr, (void *)puc_resp_data, uc_resp_data_idx);
	return DAR_SUCCESS;
}

data_access_result_t obis_0_0_29_1_0_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	static uint16_t us_pending_regs, us_index;
	uint8_t puc_logical_name[SIZE_LN] = {0x00, 0x00, 0x1D, 0x01, 0x00, 0xFF};
	uint8_t puc_resp_data[MAX_APDU_SIZE_SEND];
	uint8_t uc_resp_data_idx = 0;

	switch (uc_attr) {
	case IC91_LOGICAL_NAME:
		memcpy(puc_resp_data, puc_logical_name, SIZE_LN);
		break;

	case IC91_MAC_SHORT_ADDRESS:
		puc_resp_data[uc_resp_data_idx++] = 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 0xFF;
		break;

	case IC91_MAC_RC_COORD:
		puc_resp_data[uc_resp_data_idx++] = 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 0xFF;
		break;

	case IC91_MAC_PAN_ID:
		puc_resp_data[uc_resp_data_idx++] = 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 0xFF;
		break;

	case IC91_MAC_TONE_MASK:
		/* ToDo: Also valid for CENELEC? */
		puc_resp_data[uc_resp_data_idx++] = 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 0xFF;
		puc_resp_data[uc_resp_data_idx++] = 0xFF;
		break;

	case IC91_MAC_TMR_TTL:
#ifdef CONF_TMR_TTL
		memcpy(puc_resp_data, CONF_TMR_TTL, sizeof(CONF_TMR_TTL));
		uc_resp_data_idx += sizeof(CONF_TMR_TTL);
#else
		puc_resp_data[uc_resp_data_idx++] = 10;
#endif
		break;

	case IC91_MAC_MAX_FRAME_RETRIES:
		/* ToDo: Extract from G3 configuration */
		puc_resp_data[uc_resp_data_idx++] = 5;
		break;

	case IC91_MAC_NEIGHBOUR_TABLE_ENTRY_TTL:
		/* ToDo: Extract from G3 configuration */
		puc_resp_data[uc_resp_data_idx++] = 0x05;
		break;

	case IC91_MAC_HIGH_PRIORITY_WINDOW_SIZE:
		puc_resp_data[uc_resp_data_idx++] = 7;
		break;

	case IC91_MAC_CSMA_FAIRNESS_LIMIT:
		puc_resp_data[uc_resp_data_idx++] = 15;
		break;

	case IC91_MAC_BEACON_RANDOMIZATION_WINDOW_LENGTH:
		puc_resp_data[uc_resp_data_idx++] = 12;
		break;

	case IC91_MAC_A:
		puc_resp_data[uc_resp_data_idx++] = 8;
		break;

	case IC91_MAC_K:
		puc_resp_data[uc_resp_data_idx++] = 5;
		break;

	case IC91_MAC_MIN_CW_ATTEMPTS:
		puc_resp_data[uc_resp_data_idx++] = 10;
		break;

	case IC91_MAC_CENELEC_LEGACY_MODE:
		puc_resp_data[uc_resp_data_idx++] = 0;
		break;

	case IC91_MAC_FCC_LEGACY_MODE:
		puc_resp_data[uc_resp_data_idx++] = 0;
		break;

	case IC91_MAC_MAX_BE:
		puc_resp_data[uc_resp_data_idx++] = 8;
		break;

	case IC91_MAC_MAX_CSMA_BACKOFFS:
		puc_resp_data[uc_resp_data_idx++] = 50;
		break;

	case IC91_MAC_MIN_BE:
		puc_resp_data[uc_resp_data_idx++] = 3;
		break;



	case IC91_MAC_NEIGHBOUR_TABLE:
		if (px_assoc_info->long_get.state == DLMS_ONE_BLOCK) {
			UNUSED(us_index);

			/* Check if it is last block */
			if (!us_pending_regs) {
				if (px_assoc_info->long_get.state == DLMS_NEXT_BLOCK) {
					px_assoc_info->long_get.state = DLMS_LAST_BLOCK;
				}

				break;
			}
		}
		break;

	default:
		return DAR_SCOPE_VIOLATED;
	}

	dlms_srv_encode_ic91(px_assoc_info, (ic91_attr_t)uc_attr, (void *)puc_resp_data, uc_resp_data_idx);
	return DAR_SUCCESS;
}

data_access_result_t obis_0_0_29_2_0_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	static uint16_t us_pending_regs, us_index;
	uint8_t puc_logical_name[SIZE_LN] = {0x00, 0x00, 0x1D, 0x02, 0x00, 0xFF};
	uint8_t puc_resp_data[MAX_APDU_SIZE_SEND];
	uint8_t uc_resp_data_idx = 0;

	switch (uc_attr) {
	case IC92_LOGICAL_NAME:
		memcpy(puc_resp_data, puc_logical_name, SIZE_LN);
		break;

	case IC92_ADP_MAX_HOPS:
		puc_resp_data[uc_resp_data_idx++] = 8;
		break;

	case IC92_ADP_WEAK_LQI_VALUE:
		puc_resp_data[uc_resp_data_idx++] = 52;
		break;

	case IC92_ADP_SECURITY_LEVEL:
		puc_resp_data[uc_resp_data_idx++] = 0x05; /* MAC_FRAME_SECURED_ENC_MIC_32 */
		break;

	case IC92_ADP_BROADCAST_LOG_TABLE_ENTRY_TTL:
		puc_resp_data[uc_resp_data_idx++] = 2; /* minutes */
		puc_resp_data[uc_resp_data_idx++] = 0;
		break;

	case IC92_ADP_MAX_JOIN_WAIT_TIME:
		puc_resp_data[uc_resp_data_idx++] = 20; /* seconds */
		puc_resp_data[uc_resp_data_idx++] = 0;
		break;

	case IC92_ADP_PATH_DISCOVERY_TIME:
		puc_resp_data[uc_resp_data_idx++] = 40; /* seconds */
		break;

	case IC92_ADP_ACTIVE_KEY_INDEX:
		puc_resp_data[uc_resp_data_idx++] = 0;
		break;

	case IC92_ADP_METRIC_TYPE:
		puc_resp_data[uc_resp_data_idx++] = 0x0F;
		break;
	case IC92_ADP_COORD_SHORT_ADDRESS:
		puc_resp_data[uc_resp_data_idx++] = 0;
		break;

	case IC92_ADP_DISABLE_DEFAULT_ROUTING:
		puc_resp_data[uc_resp_data_idx++] = 0;
		break;

	case IC92_ADP_DEVICE_TYPE:
		puc_resp_data[uc_resp_data_idx++] = 2; /* ADP_TYPE_NOT_DEFINED */
		break;

	case IC92_ADP_ROUTING_TABLE:
		if (px_assoc_info->long_get.state == DLMS_ONE_BLOCK) {
			UNUSED(us_index);

			/* Check if it is last block */
			if (!us_pending_regs) {
				if (px_assoc_info->long_get.state == DLMS_NEXT_BLOCK) {
					px_assoc_info->long_get.state = DLMS_LAST_BLOCK;
				}

				break;
			}
		}

		break;

	case IC92_ADP_PREFIX_TABLE:
	case IC92_ADP_ROUTING_CONFIGURATION:
	case IC92_ADP_CONTEXT_INFORMATION_TABLE:
	case IC92_ADP_BLACKLIST_TABLE:
	case IC92_ADP_BROADCAST_LOG_TABLE:
	case IC92_ADP_GROUP_TABLE:
		return DAR_OBJ_UNAVAIL;

	default:
		return DAR_SCOPE_VIOLATED;
	}

	dlms_srv_encode_ic92(px_assoc_info, (ic92_attr_t)uc_attr, (void *)puc_resp_data, uc_resp_data_idx);
	return DAR_SUCCESS;
}

data_access_result_t obis_0_0_29_2_2_128_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	uint8_t puc_logical_name[SIZE_LN] = {0x00, 0x00, 0x1D, 0x02, 0x02, 0x80};
	uint8_t puc_resp_data[MAX_APDU_SIZE_SEND];
	uint8_t uc_resp_data_idx = 0;

	switch (uc_attr) {
	case IC01_LOGICAL_NAME:
		memcpy(puc_resp_data, puc_logical_name, SIZE_LN);
		break;

	case IC01_VALUE:
		puc_resp_data[uc_resp_data_idx++] = DT_OCTET_STRING;
		puc_resp_data[uc_resp_data_idx++] = 8; /* num elements */
		memcpy(&puc_resp_data[uc_resp_data_idx], g_auc_ext_address, 8);
		uc_resp_data_idx += 8;
		break;

	default:
		return DAR_SCOPE_VIOLATED;
	}

	dlms_srv_encode_ic01(px_assoc_info, (ic01_attr_t)uc_attr, (void *)puc_resp_data, uc_resp_data_idx);
	return DAR_SUCCESS;
}

data_access_result_t obis_1_0_0_2_0_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	uint8_t puc_logical_name[SIZE_LN] = {0x01, 0x00, 0x00, 0x02, 0x00, 0xFF};
	uint8_t puc_fw_version[] = {0x56, 0x30, 0x31, 0x30, 0x34};
	uint8_t puc_resp_data[MAX_APDU_SIZE_SEND];
	uint8_t uc_resp_data_idx = 0;

	switch (uc_attr) {
	case IC01_LOGICAL_NAME:
		memcpy(puc_resp_data, puc_logical_name, SIZE_LN);
		break;

	case IC01_VALUE:
		puc_resp_data[uc_resp_data_idx++] = DT_OCTET_STRING;
		puc_resp_data[uc_resp_data_idx++] = 5; /* num elements */
		memcpy(&puc_resp_data[uc_resp_data_idx], puc_fw_version, 5);
		uc_resp_data_idx += 5;
		break;

	default:
		return DAR_SCOPE_VIOLATED;
	}

	dlms_srv_encode_ic01(px_assoc_info, (ic01_attr_t)uc_attr, (void *)puc_resp_data, uc_resp_data_idx);
	return DAR_SUCCESS;
}

data_access_result_t obis_0_0_1_0_0_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	uint8_t puc_logical_name[SIZE_LN] = {0x00, 0x00, 0x01, 0x00, 0x00, 0xFF};
	uint8_t puc_resp_data[MAX_APDU_SIZE_SEND];

	switch (uc_attr) {
	case IC08_LOGICAL_NAME:
		memcpy(puc_resp_data, puc_logical_name, SIZE_LN);
		break;

	case IC08_TIME:
		memcpy(&puc_resp_data[0], dummy_load_prof_reg.puc_date_time, SIZE_DATE_TIME);
		break;

	case IC08_TIME_ZONE:
		puc_resp_data[0] = (32400 & 0xFF00) >> 8; /* UTC+9 */
		puc_resp_data[1] = 32400 & 0x00FF; /* UTC+9 */
		break;

	case IC08_STATUS:
		puc_resp_data[0] = 0x81; /* clock_status */
		break;

	case IC08_DAYLIGHT_BEGIN:
		memcpy(&puc_resp_data[0], dummy_load_prof_reg.puc_date_time, SIZE_DATE_TIME);
		break;

	case IC08_DAYLIGHT_END:
		memcpy(&puc_resp_data[0], dummy_load_prof_reg.puc_date_time, SIZE_DATE_TIME);
		break;

	case IC08_DAYLIGHT_DEV:
		puc_resp_data[0] = 60; /* One hour */
		break;

	case IC08_DAYLIGHT_ENABLED:
		puc_resp_data[0] = 1; /* true */
		break;

	case IC08_CLK_BASE:
		puc_resp_data[0] = 1; /* INTERNAL_XTAL */
		break;

	default:
		return DAR_SCOPE_VIOLATED;
	}

	dlms_srv_encode_ic08(px_assoc_info, (ic08_attr_t)uc_attr, (void *)puc_resp_data);
	return DAR_SUCCESS;
}

data_access_result_t obis_0_0_40_0_0_255_example_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	static uint16_t us_obis_idx = 0;
	static uint16_t us_configured_objects_num = 0;
	uint8_t puc_temp_buffer[2 * MAX_APDU_SIZE_SEND];
	uint8_t puc_logical_name[SIZE_LN] = {0x00, 0x00, 0x28, 0x00, 0x00, 0xFF};
	uint8_t puc_resp_data[MAX_APDU_SIZE_SEND];
	uint16_t us_reg_len;
	uint8_t uc_resp_data_idx = 0;

	switch (uc_attr) {
	case IC15_LOGICAL_NAME:
		memcpy(puc_resp_data, puc_logical_name, SIZE_LN);
		break;

	case IC15_OBJECT_LIST:
		if (px_assoc_info->long_get.state == DLMS_ONE_BLOCK) {
			/* Get total number of registers */
			us_configured_objects_num = dlms_srv_get_num_objects();
			/* Reset obis index */
			us_obis_idx = 0;
			/* Start: add array TAG and number of elements */
			puc_resp_data[uc_resp_data_idx++] = DT_ARRAY;
			uc_resp_data_idx += dlms_srv_encode_a_xdr_length(us_configured_objects_num, &puc_resp_data[uc_resp_data_idx]);
		}

		/* Copy registers to data packet */
		while (us_obis_idx < us_configured_objects_num) {
			us_reg_len = dlms_srv_encode_ic15_object(px_assoc_info, us_obis_idx, puc_temp_buffer);

			/* Check if register fits in current block */
			if ((uc_resp_data_idx + us_reg_len) < px_assoc_info->max_apdu_size) {
				memcpy(&puc_resp_data[uc_resp_data_idx], puc_temp_buffer, us_reg_len);
				uc_resp_data_idx += us_reg_len;
				us_obis_idx++;
			} else {
				if (px_assoc_info->long_get.state == DLMS_ONE_BLOCK) {
					/* Initialize get with block process block numbering */
					px_assoc_info->long_get.state = DLMS_FIRST_BLOCK;
					px_assoc_info->long_get.block = 1;
				}

				break;
			}
		}

		/* Check if all registers have been processed */
		if (us_obis_idx == us_configured_objects_num) {
			if (px_assoc_info->long_get.state == DLMS_NEXT_BLOCK) {
				px_assoc_info->long_get.state = DLMS_LAST_BLOCK; /* Set last block flag */
			}
		}

		break;

	case IC15_ASSOC_STATUS:
		puc_resp_data[uc_resp_data_idx++] = DT_ENUM;
		puc_resp_data[uc_resp_data_idx++] = 2; /* associated */
		break;

	case IC15_ASSOCIATED_PARTNERS_ID:
	case IC15_APP_CTXT_NAME:
	case IC15_XDLMS_CTXT_INFO:
	case IC15_AUTH_MECH_NAME:
	case IC15_SECRET:
	case IC15_SECURITY_SETUP_REFERENCE:
	case IC15_USER_LIST:
	case IC15_CURRENT_USER:
		return DAR_OBJ_UNAVAIL;

	default:
		return DAR_SCOPE_VIOLATED;
	}

	dlms_srv_encode_ic15(px_assoc_info, (ic15_attr_t)uc_attr, (void *)puc_resp_data, uc_resp_data_idx);
	return DAR_SUCCESS;
}

/**
 *  \brief Init DLMS server data
 *
 * \retval Meter params struct pointer
 */
void dlms_srv_data_init_example(meter_params_t *px_meter_params)
{
	spx_meter_params = px_meter_params;
}
