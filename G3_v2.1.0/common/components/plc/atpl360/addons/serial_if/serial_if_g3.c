/**
 * \file
 *
 * \brief ATPL360_Host Serial Interface for Physical layer
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

/* System includes */
#include "compiler.h"

#include "atpl360_comm.h"
/* Serial interface */
#include "serial_if.h"
#include "addon_api.h"
#include "general_defs.h"
#include "atpl360_IB.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif

/**INDENT-ON**/
/* / @endcond */

/* Carriers Buffer Len */
#define SERIAL_IF_CARR_BUFFER_LEN  16

static uint8_t spuc_serial_data_buf[808];  /* !<  Receive working buffer */

/* { CEN_A, FCC, ARIB, CEN_B } */
static uint8_t spuc_tonemap_size[4] = {TONE_MAP_SIZE_CENELEC, TONE_MAP_SIZE_FCC_ARIB, TONE_MAP_SIZE_FCC_ARIB, TONE_MAP_SIZE_CENELEC};
static uint8_t spuc_numsubbands[4] = {NUM_SUBBANDS_CENELEC_A, NUM_SUBBANDS_FCC, NUM_SUBBANDS_ARIB, NUM_SUBBANDS_CENELEC_B};
static uint8_t spuc_num_carriers[4] = {NUM_CARRIERS_CENELEC_A, NUM_CARRIERS_FCC, NUM_CARRIERS_ARIB, NUM_CARRIERS_CENELEC_B};

/**
 * \brief Converts Phy RX Data struct to byte buffering in order to report data through ADDONs
 *
 * \param puc_ind_data       Pointer to destiny buffer
 * \param pv_src             Pointer to struct to convert
 *
 * \return Length of data to buffering
 */
uint16_t atpl360_addon_stringify_ind(uint8_t *puc_ind_data, rx_msg_t *px_ind_data)
{
	uint8_t *puc_dst_buf;
	uint8_t uc_band, uc_size;

	puc_dst_buf = puc_ind_data;

	atpl360_ib_get_param(ATPL360_HOST_BAND_ID, &uc_band, 1);

	*puc_dst_buf++ = (uint8_t)SERIAL_IF_PHY_COMMAND_RECEIVE_MSG;
	*puc_dst_buf++ = (uint8_t)px_ind_data->uc_mod_type;
	*puc_dst_buf++ = (uint8_t)px_ind_data->uc_mod_scheme;

	uc_size = spuc_tonemap_size[uc_band - 1];
	for (uint8_t uc_i = 0; uc_i < uc_size; uc_i++) {
		*puc_dst_buf++ = px_ind_data->puc_tone_map[uc_size - uc_i - 1];
	}
	*puc_dst_buf++ = (uint8_t)(px_ind_data->ss_snr_fch >> 8);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->ss_snr_fch);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->ss_snr_pay >> 8);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->ss_snr_pay);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->us_rssi >> 8);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->us_rssi);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->ul_agc_factor >> 24);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->ul_agc_factor >> 16);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->ul_agc_factor >> 8);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->ul_agc_factor);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->uc_zct_diff);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->uc_delimiter_type);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->uc_lqi);
	uc_size = spuc_num_carriers[uc_band - 1];
	memcpy(puc_dst_buf, px_ind_data->puc_carrier_snr, uc_size);
	puc_dst_buf += uc_size;
	*puc_dst_buf++ = (uint8_t)(px_ind_data->uc_payload_snr_worst_carrier);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->us_payload_corrupted_carriers >> 8);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->us_payload_corrupted_carriers);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->us_payload_noised_symbols >> 8);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->us_payload_noised_symbols);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->uc_payload_snr_worst_symbol);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->uc_payload_snr_impulsive);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->uc_payload_snr_band);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->uc_payload_snr_background);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->uc_rs_corrected_errors);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->us_data_len >> 8);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->us_data_len);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->ul_rx_time >> 24);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->ul_rx_time >> 16);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->ul_rx_time >> 8);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->ul_rx_time);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->ul_frame_duration >> 24);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->ul_frame_duration >> 16);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->ul_frame_duration >> 8);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->ul_frame_duration);

	memcpy(puc_dst_buf, px_ind_data->puc_data_buf, px_ind_data->us_data_len);
	puc_dst_buf += px_ind_data->us_data_len;

	return (puc_dst_buf - puc_ind_data);
}

/**
 * \brief Converts Phy TX Cfm struct to byte buffering in order to report data through ADDONs
 *
 * \param puc_cfm_data       Pointer to destiny buffer
 * \param *px_cfm_data       Pointer to struct to convert
 *
 * \return Length of data to buffering
 */
uint16_t atpl360_addon_stringify_cfm(uint8_t *puc_cfm_data, tx_cfm_t *px_cfm_data)
{
	uint8_t *puc_dst_buf;

	puc_dst_buf = puc_cfm_data;

	*puc_dst_buf++ = (uint8_t)SERIAL_IF_PHY_COMMAND_SEND_MSG_RSP;
	*puc_dst_buf++ = (uint8_t)px_cfm_data->uc_tx_result;
	*puc_dst_buf++ = (uint8_t)(px_cfm_data->ul_rms_calc >> 24);
	*puc_dst_buf++ = (uint8_t)(px_cfm_data->ul_rms_calc >> 16);
	*puc_dst_buf++ = (uint8_t)(px_cfm_data->ul_rms_calc >> 8);
	*puc_dst_buf++ = (uint8_t)(px_cfm_data->ul_rms_calc);
	*puc_dst_buf++ = (uint8_t)(px_cfm_data->ul_tx_time >> 24);
	*puc_dst_buf++ = (uint8_t)(px_cfm_data->ul_tx_time >> 16);
	*puc_dst_buf++ = (uint8_t)(px_cfm_data->ul_tx_time >> 8);
	*puc_dst_buf++ = (uint8_t)(px_cfm_data->ul_tx_time & 0x000000FF);

	return (puc_dst_buf - puc_cfm_data);
}

/**
 * \brief Converts Phy TX info to byte buffering in order to report data through ADDONs   (NOT USE IN SERIAL ADDON)
 *
 * \param puc_tx_data       Pointer to destiny buffer
 * \param px_tx_data        Pointer to struct to convert
 *
 * \return Length of data to buffering
 */
uint16_t atpl360_addon_stringify_tx(uint8_t *puc_tx_data, tx_msg_t *px_tx_data)
{
	(void)puc_tx_data;
	(void)px_tx_data;

	return 0;
}

/**
 * \brief Extract TX info from data buffer
 *
 * \param px_tx_msg         Pointer to struct to extract data
 * \param puc_tx_data       Pointer to tx data buffer
 *
 */
void serial_if_parse_tx_msg(tx_msg_t *px_tx_msg, uint8_t *puc_tx_data)
{
	uint8_t uc_i;
	uint8_t uc_band, uc_size;
	uint8_t *puc_data;

	puc_data = puc_tx_data;

	atpl360_ib_get_param(ATPL360_HOST_BAND_ID, &uc_band, 1);

	/* Update write parameters of TX struct */
	px_tx_msg->uc_tx_mode = *puc_data++;
	px_tx_msg->uc_tx_power = *puc_data++;
	px_tx_msg->uc_mod_type = (enum mod_types)(*puc_data++);
	px_tx_msg->uc_mod_scheme = (enum mod_schemes)(*puc_data++);
	px_tx_msg->uc_pdc = *puc_data++;

	uc_size = spuc_tonemap_size[uc_band - 1];
	for (uc_i = 0; uc_i < uc_size; uc_i++) {
		px_tx_msg->puc_tone_map[uc_size - uc_i - 1] = *puc_data++;
	}

	if ((uc_band == ATPL360_WB_FCC) || (uc_band == ATPL360_WB_ARIB)) {
		px_tx_msg->uc_2_rs_blocks = *puc_data++;
	} else {
		px_tx_msg->uc_2_rs_blocks = 0;
	}

	uc_size = spuc_numsubbands[uc_band - 1];
	memcpy(px_tx_msg->puc_preemphasis, puc_data, uc_size);
	puc_data += uc_size;

	px_tx_msg->uc_delimiter_type = (enum delimiter_types)(*puc_data++);
	px_tx_msg->ul_tx_time = ((uint32_t)*puc_data++) << 24;
	px_tx_msg->ul_tx_time += ((uint32_t)*puc_data++) << 16;
	px_tx_msg->ul_tx_time += ((uint32_t)*puc_data++) << 8;
	px_tx_msg->ul_tx_time += (uint32_t)*puc_data++;
	px_tx_msg->us_data_len = ((uint16_t)*puc_data++) << 8;
	px_tx_msg->us_data_len += (uint16_t)*puc_data++;
	px_tx_msg->puc_data_buf = spuc_serial_data_buf;

	/* copy data */
	memcpy(spuc_serial_data_buf, puc_data, px_tx_msg->us_data_len);
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
