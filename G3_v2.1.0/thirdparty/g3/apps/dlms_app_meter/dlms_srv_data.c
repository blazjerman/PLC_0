/**
 * \file
 *
 * \brief DLMS_SRV_LIB : DLMS server lib: G3 Profile
 *
 * Copyright (c) 2019 Atmel Corporation. All rights reserved.
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

#include "conf_project.h"
#include "dlms_srv_lib.h"
#include "dlms_srv_data.h"
#include "dlms_srv_data_example.h"

#ifdef BOARD_SUPPORTS_METERING
#include "app_metering.h"

/** Dummy load profile to fill load profile register request */
static load_profile_reg_t dummy_load_prof_reg = {
	{0x07, 0xE0, 0x08, 0x06, 0x06, 0x08, 0x0F, 0x00, 0x00, 0x80, 0x00, 0x00},
	0x00,
	0xA1A1A1A1, 0xB2B2B2B2, 0xC3C3C3C3, 0xD4D4D4D4, 0xE5E5E5E5, 0xF6F6F6F6
};
#endif /* BOARD_SUPPORTS_METERING */

data_access_result_t obis_1_0_1_8_x_255_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	return obis_1_0_1_8_x_255_example_cb(px_assoc_info, uc_attr);
}

data_access_result_t obis_0_0_96_1_x_255_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	return obis_0_0_96_1_x_255_example_cb(px_assoc_info, uc_attr);
}

data_access_result_t obis_0_0_28_7_0_255_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	return obis_0_0_28_7_0_255_example_cb(px_assoc_info, uc_attr);
}

data_access_result_t obis_0_0_21_0_5_255_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
#ifdef BOARD_SUPPORTS_METERING
	uint8_t puc_logical_name[SIZE_LN] = {0x00, 0x00, 0x15, 0x00, 0x05, 0xFF};
	uint8_t puc_capture_objects[] = {0x01, 0x0D, /* array --- 13 elements*/

		                         /* structure 4 elements -->	IC (long-unsigned 18) / OBIS (octet-string 9) / ATTR (integer 17) / INDEX (long-unsigned
		                          * 18)*/
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
		metering_app_get_info(&sx_meter_info);

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
		puc_resp_data[uc_resp_data_idx++] = (sx_meter_info.Inst_V_rms_L1 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = sx_meter_info.Inst_V_rms_L1 & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (sx_meter_info.Inst_I_rms_L1 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = sx_meter_info.Inst_I_rms_L1 & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (sx_meter_info.Inst_V_rms_L2 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = sx_meter_info.Inst_V_rms_L2 & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (sx_meter_info.Inst_I_rms_L2 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = sx_meter_info.Inst_I_rms_L2 & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (sx_meter_info.Inst_V_rms_L3 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = sx_meter_info.Inst_V_rms_L3 & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (sx_meter_info.Inst_I_rms_L3 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = sx_meter_info.Inst_I_rms_L3 & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (sx_meter_info.Inst_I_rms_L123 >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = sx_meter_info.Inst_I_rms_L123 & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_DOUBLE_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (sx_meter_info.Inst_P_import >> 24) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (sx_meter_info.Inst_P_import >> 16) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (sx_meter_info.Inst_P_import >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = sx_meter_info.Inst_P_import & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_DOUBLE_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (sx_meter_info.Inst_P_export >> 24) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (sx_meter_info.Inst_P_export >> 16) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (sx_meter_info.Inst_P_export >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = sx_meter_info.Inst_P_export & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_DOUBLE_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (sx_meter_info.Inst_Q_import >> 24) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (sx_meter_info.Inst_Q_import >> 16) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (sx_meter_info.Inst_Q_import >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = sx_meter_info.Inst_Q_import & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_DOUBLE_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (sx_meter_info.Inst_Q_export >> 24) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (sx_meter_info.Inst_Q_export >> 16) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = (sx_meter_info.Inst_Q_export >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = sx_meter_info.Inst_Q_export & 0xFF;

		puc_resp_data[uc_resp_data_idx++] = DT_LONG_UNSIGNED;
		puc_resp_data[uc_resp_data_idx++] = (sx_meter_info.Inst_Power_factor >> 8) & 0xFF;
		puc_resp_data[uc_resp_data_idx++] = sx_meter_info.Inst_Power_factor & 0xFF;
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

#else
	return obis_0_0_21_0_5_255_example_cb(px_assoc_info, uc_attr);
#endif
}

data_access_result_t obis_0_0_21_0_6_255_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	return obis_0_0_21_0_6_255_example_cb(px_assoc_info, uc_attr);
}

data_access_result_t obis_1_0_99_1_0_255_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	return obis_1_0_99_1_0_255_example_cb(px_assoc_info, uc_attr);
}

data_access_result_t obis_0_0_29_0_0_255_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	return obis_0_0_29_0_0_255_example_cb(px_assoc_info, uc_attr);
}

data_access_result_t obis_0_0_29_1_0_255_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	return obis_0_0_29_1_0_255_example_cb(px_assoc_info, uc_attr);
}

data_access_result_t obis_0_0_29_2_0_255_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	return obis_0_0_29_2_0_255_example_cb(px_assoc_info, uc_attr);
}

data_access_result_t obis_0_0_29_2_2_128_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	return obis_0_0_29_2_2_128_example_cb(px_assoc_info, uc_attr);
}

data_access_result_t obis_1_0_0_2_0_255_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	return obis_1_0_0_2_0_255_example_cb(px_assoc_info, uc_attr);
}

data_access_result_t obis_0_0_1_0_0_255_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	return obis_0_0_1_0_0_255_example_cb(px_assoc_info, uc_attr);
}

data_access_result_t obis_0_0_40_0_0_255_cb(assoc_info_t *px_assoc_info, uint8_t uc_attr)
{
	return obis_0_0_40_0_0_255_example_cb(px_assoc_info, uc_attr);
}

void dlms_srv_data_init(meter_params_t *px_meter_params)
{
	dlms_srv_data_init_example(px_meter_params);
}
