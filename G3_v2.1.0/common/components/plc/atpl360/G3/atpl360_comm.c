/**
 * \file
 *
 * \brief atpl360_host G3 Physical layer
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

/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

/* System includes */
#include <string.h>

#include "compiler.h"
#include "atpl360_comm.h"
#include "conf_atpl360.h"
#include "atpl360.h"
#include "atpl360_IB.h"

#ifdef __cplusplus
extern "C" {
#endif

static const uint16_t pcus_event_mem_ids[NUM_EV_TYPES] =
{ATPL360_RX_DATA_ID, ATPL360_RX_PARAM_ID, ATPL360_TX_CFM_ID, ATPL360_REG_INFO_ID};

/**
 * \brief Converts Phy Data struct to byte buffering in order to send data through SPI to ATPL360
 *
 * \param pv_dst       Pointer to destiny buffer
 * \param pv_src       Pointer to struct to convert
 * \param us_src_size  Size of the struct to convert
 *
 */
uint16_t atpl360_comm_stringify(uint8_t *pv_dst, void *pv_src, uint16_t us_src_size)
{
	uint8_t *puc_dest_buf;
	uint16_t us_param_len;

	puc_dest_buf = pv_dst;
	us_param_len = 0;

	if (us_src_size == sizeof(tx_msg_t)) {
		tx_msg_t *tx_msg_data;
		tx_msg_data = (tx_msg_t *)pv_src;

		/* Protection: Check data length */
		if ((tx_msg_data->us_data_len == 0) || tx_msg_data->us_data_len > ATPL360_MAX_DATA_LENGTH) {
			return 0;
		}

		*puc_dest_buf++ = (uint8_t)tx_msg_data->ul_tx_time;
		*puc_dest_buf++ = (uint8_t)(tx_msg_data->ul_tx_time >> 8);
		*puc_dest_buf++ = (uint8_t)(tx_msg_data->ul_tx_time >> 16);
		*puc_dest_buf++ = (uint8_t)(tx_msg_data->ul_tx_time >> 24);
		*puc_dest_buf++ = (uint8_t)tx_msg_data->us_data_len;
		*puc_dest_buf++ = (uint8_t)(tx_msg_data->us_data_len >> 8);
		memcpy(puc_dest_buf, tx_msg_data->puc_preemphasis, sizeof(tx_msg_data->puc_preemphasis));
		puc_dest_buf += sizeof(tx_msg_data->puc_preemphasis);
		memcpy(puc_dest_buf, tx_msg_data->puc_tone_map, sizeof(tx_msg_data->puc_tone_map));
		puc_dest_buf += sizeof(tx_msg_data->puc_tone_map);
		*puc_dest_buf++ = tx_msg_data->uc_tx_mode;
		*puc_dest_buf++ = tx_msg_data->uc_tx_power;
		*puc_dest_buf++ = tx_msg_data->uc_mod_type;
		*puc_dest_buf++ = tx_msg_data->uc_mod_scheme;
		*puc_dest_buf++ = tx_msg_data->uc_pdc;
		*puc_dest_buf++ = tx_msg_data->uc_2_rs_blocks;
		*puc_dest_buf++ = tx_msg_data->uc_delimiter_type;

		us_param_len = (uint16_t)(puc_dest_buf - pv_dst);

		memcpy(puc_dest_buf, tx_msg_data->puc_data_buf, tx_msg_data->us_data_len);
	}

	return us_param_len;
}

/**
 * \brief Converts Phy Data buffer receive through SPI from ATPL360 to phy structs
 *
 * \param pv_dst       Pointer to struct to parse
 * \param puc_src      Pointer to buffer to get data
 * \param us_dst_size  Size of the struct to parse
 *
 */
atpl360_comm_status_t atpl360_comm_parse(void *pv_dst, uint8_t *puc_src, uint16_t us_dst_size)
{
	uint8_t *puc_src_buf;
	atpl360_comm_status_t uc_result;

	uc_result = ATPL360_COMM_SUCCESS;
	puc_src_buf = puc_src;

	if (us_dst_size == sizeof(rx_msg_t)) {
		rx_msg_t *rx_msg_data;

		rx_msg_data = (rx_msg_t *)pv_dst;

		/* Parse gral info */
		rx_msg_data->ul_rx_time = (uint32_t)*puc_src_buf++;
		rx_msg_data->ul_rx_time += (uint32_t)*puc_src_buf++ << 8;
		rx_msg_data->ul_rx_time += (uint32_t)*puc_src_buf++ << 16;
		rx_msg_data->ul_rx_time += (uint32_t)*puc_src_buf++ << 24;

		rx_msg_data->ul_frame_duration = (uint32_t)*puc_src_buf++;
		rx_msg_data->ul_frame_duration += (uint32_t)*puc_src_buf++ << 8;
		rx_msg_data->ul_frame_duration += (uint32_t)*puc_src_buf++ << 16;
		rx_msg_data->ul_frame_duration += (uint32_t)*puc_src_buf++ << 24;

		rx_msg_data->us_rssi = (uint16_t)*puc_src_buf++;
		rx_msg_data->us_rssi += (uint16_t)*puc_src_buf++ << 8;
		rx_msg_data->us_data_len = (uint16_t)*puc_src_buf++;
		rx_msg_data->us_data_len += (uint16_t)*puc_src_buf++ << 8;
		if ((rx_msg_data->us_data_len == 0) || (rx_msg_data->us_data_len > ATPL360_MAX_DATA_LENGTH)) {
			return ATPL360_COMM_ERROR;
		}

		rx_msg_data->uc_zct_diff = *puc_src_buf++;
		rx_msg_data->uc_rs_corrected_errors = *puc_src_buf++;
		rx_msg_data->uc_mod_type = (enum mod_types)*puc_src_buf++;
		rx_msg_data->uc_mod_scheme = (enum mod_schemes)*puc_src_buf++;

		rx_msg_data->ul_agc_factor = (uint32_t)*puc_src_buf++;
		rx_msg_data->ul_agc_factor += (uint32_t)*puc_src_buf++ << 8;
		rx_msg_data->ul_agc_factor += (uint32_t)*puc_src_buf++ << 16;
		rx_msg_data->ul_agc_factor += (uint32_t)*puc_src_buf++ << 24;
		rx_msg_data->us_agc_fine = (uint16_t)*puc_src_buf++;
		rx_msg_data->us_agc_fine += (uint16_t)*puc_src_buf++ << 8;
		rx_msg_data->ss_agc_offset_meas = (uint16_t)*puc_src_buf++;
		rx_msg_data->ss_agc_offset_meas += (uint16_t)*puc_src_buf++ << 8;
		rx_msg_data->uc_agc_active = *puc_src_buf++;
		rx_msg_data->uc_agc_pga_value = *puc_src_buf++;
		rx_msg_data->ss_snr_fch = (uint16_t)*puc_src_buf++;
		rx_msg_data->ss_snr_fch += (uint16_t)*puc_src_buf++ << 8;
		rx_msg_data->ss_snr_pay = (uint16_t)*puc_src_buf++;
		rx_msg_data->ss_snr_pay += (uint16_t)*puc_src_buf++ << 8;
		rx_msg_data->us_payload_corrupted_carriers = (uint16_t)*puc_src_buf++;
		rx_msg_data->us_payload_corrupted_carriers += (uint16_t)*puc_src_buf++ << 8;
		rx_msg_data->us_payload_noised_symbols = (uint16_t)*puc_src_buf++;
		rx_msg_data->us_payload_noised_symbols += (uint16_t)*puc_src_buf++ << 8;
		rx_msg_data->uc_payload_snr_worst_carrier = *puc_src_buf++;
		rx_msg_data->uc_payload_snr_worst_symbol = *puc_src_buf++;
		rx_msg_data->uc_payload_snr_impulsive = *puc_src_buf++;
		rx_msg_data->uc_payload_snr_band = *puc_src_buf++;
		rx_msg_data->uc_payload_snr_background = *puc_src_buf++;
		rx_msg_data->uc_lqi = *puc_src_buf++;

		rx_msg_data->uc_delimiter_type = (enum delimiter_types)*puc_src_buf++;
		rx_msg_data->uc_crc_ok = *puc_src_buf++;
		memcpy(rx_msg_data->puc_tone_map, puc_src_buf, sizeof(rx_msg_data->puc_tone_map));
		puc_src_buf += sizeof(rx_msg_data->puc_tone_map);
		memcpy(rx_msg_data->puc_carrier_snr, puc_src_buf, sizeof(rx_msg_data->puc_carrier_snr));
		puc_src_buf += sizeof(rx_msg_data->puc_carrier_snr);

		/* Store only pointer to data. Use the same bufffer */
		rx_msg_data->puc_data_buf = puc_src_buf;
	} else if (us_dst_size == sizeof(tx_cfm_t)) {
		tx_cfm_t *cfm_msg_data;

		cfm_msg_data = (tx_cfm_t *)pv_dst;
		cfm_msg_data->ul_rms_calc = (uint32_t)*puc_src_buf++;
		cfm_msg_data->ul_rms_calc += (uint32_t)*puc_src_buf++ << 8;
		cfm_msg_data->ul_rms_calc += (uint32_t)*puc_src_buf++ << 16;
		cfm_msg_data->ul_rms_calc += (uint32_t)*puc_src_buf++ << 24;

		cfm_msg_data->ul_tx_time = (uint32_t)*puc_src_buf++;
		cfm_msg_data->ul_tx_time += (uint32_t)*puc_src_buf++ << 8;
		cfm_msg_data->ul_tx_time += (uint32_t)*puc_src_buf++ << 16;
		cfm_msg_data->ul_tx_time += (uint32_t)*puc_src_buf++ << 24;

		cfm_msg_data->uc_tx_result = (enum tx_result_values)*puc_src_buf++;
	} else {
		uc_result = ATPL360_COMM_ERROR;
	}

	return uc_result;
}

uint16_t atpl360_comm_get_event_id(enum atpl360_event_type ev_type, uint16_t us_int_flags)
{
	(void)us_int_flags;
	if (ev_type < NUM_EV_TYPES) {
		return pcus_event_mem_ids[ev_type];
	} else {
		return NULL;
	}
}

uint16_t atpl360_comm_get_tx_params_id(tx_msg_t *px_msg)
{
	(void)px_msg;
	return ATPL360_TX_PARAM_ID;
}

uint16_t atpl360_comm_get_tx_data_id(tx_msg_t *px_msg)
{
	(void)px_msg;
	return ATPL360_TX_DATA_ID;
}

void atpl360_comm_set_event_info(atpl360_events_t *px_events_info, uint16_t us_int_flags)
{
	if (us_int_flags & ATPL360_TX_CFM_FLAG_MASK) {
		px_events_info->b_cfm_event_enable[0] = true;
	} else {
		px_events_info->b_cfm_event_enable[0] = false;
	}

	if (us_int_flags & ATPL360_RX_DATA_IND_FLAG_MASK) {
		px_events_info->b_data_ind_event_enable = true;
	} else {
		px_events_info->b_data_ind_event_enable = false;
	}

	if (us_int_flags & ATPL360_RX_QPAR_IND_FLAG_MASK) {
		px_events_info->b_qpar_ind_event_enable = true;
	} else {
		px_events_info->b_qpar_ind_event_enable = false;
	}

	if (us_int_flags & ATPL360_REG_RSP_MASK) {
		px_events_info->b_reg_data_enable = true;
	} else {
		px_events_info->b_reg_data_enable = false;
	}

	px_events_info->ul_timer_ref = 0;
	px_events_info->ul_event_info = 0;
}

uint32_t atpl360_comm_get_cfg_param_access_type(uint16_t us_param_id)
{
	uint32_t ul_address;

	ul_address = 0;

	if (us_param_id & ATPL360_REG_ADC_MASK) {
		ul_address = (uint32_t)ATPL360_REG_ADC_BASE;
	} else if (us_param_id & ATPL360_REG_DAC_MASK) {
		ul_address = (uint32_t)ATPL360_REG_DAC_BASE;
	} else if (us_param_id & ATPL360_FUSES_MASK) {
		ul_address = (uint32_t)ATPL360_FUSES_BASE;
	} else if ((us_param_id & ATPL360_REG_MASK) && (us_param_id < ATPL360_REG_END_ID)) {
		ul_address = (uint32_t)ATPL360_REG_BASE;
	}

	return ul_address;
}

uint32_t atpl360_comm_get_cfg_param_delay_us(uint16_t us_param_id)
{
	uint32_t ul_delay = 50;

	if ((us_param_id & ATPL360_REG_MASK) && (us_param_id < ATPL360_REG_END_ID)) {
		switch (us_param_id) {
		case ATPL360_REG_TONE_MASK:
			ul_delay = 600;
			break;

		case ATPL360_REG_PREDIST_COEF_TABLE_HI:
		case ATPL360_REG_PREDIST_COEF_TABLE_LO:
			ul_delay = 250;
			break;

		case ATPL360_REG_PREDIST_COEF_TABLE_VLO:
			ul_delay = 350;
			break;

		default:
			ul_delay = 50;
			break;
		}
	}

	return ul_delay;
}

#ifdef __cplusplus
}
#endif
