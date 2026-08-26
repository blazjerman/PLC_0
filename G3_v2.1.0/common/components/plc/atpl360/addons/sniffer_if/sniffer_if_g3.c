/**
 * \file
 *
 * \brief ATPL360_Host Sniffer Interface for Physical layer
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
#include "sniffer_if.h"
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

/** USI protocol */
#define SNIFFER_USI_PROTOCOL                            0x13

/** ! \name SNIFFER version */
/* @{ */
#define SNIFFER_ATPL250                                 0x02
#define SNIFFER_PL360_G3                                0x12
#define SNIFFER_VERSION                                 0x02
/* @} */

/* ! \name Serial interface commands identifiers */
/* @{ */
#define SERIAL_IF_PHY_MESSAGE_G3_SNIFFER                0x23  /* !< Sniffer Frame Command */
/* ! \name Sniffer interface commands identifiers */
/* @{ */
#define SNIFFER_IF_PHY_COMMAND_G3_VERSION               0x00  /* !< TO DO */
/* @} */

#if ATPL360_WB == ATPL360_WB_FCC
#define NUM_CARRIERS NUM_CARRIERS_FCC
#elif ATPL360_WB == ATPL360_WB_CENELEC_A
#define NUM_CARRIERS NUM_CARRIERS_CENELEC_A
#elif ATPL360_WB == ATPL360_WB_CENELEC_B
#define NUM_CARRIERS NUM_CARRIERS_CENELEC_B
#elif ATPL360_WB == ATPL360_WB_ARIB
#define NUM_CARRIERS NUM_CARRIERS_ARIB
#else
#error
#endif

static atpl360_addon_descriptor_t *spx_desc;
static tx_msg_t sx_last_tx_msg;
static uint8_t spuc_sniffer_tx_data_buffer[ATPL360_MAX_DATA_LENGTH];

/**
 * \brief Converts Phy RX Data struct to byte buffering in order to report data through ADDONs
 *
 * \param puc_ind_data       Pointer to destiny buffer
 * \param px_ind_data        Pointer to struct to convert
 *
 * \return Length of data to buffering
 */
uint16_t atpl360_addon_stringify_ind(uint8_t *puc_ind_data, rx_msg_t *px_ind_data)
{
	uint8_t *puc_dst_buf;
	uint32_t ul_time_ini, ul_time_end;
	uint16_t us_num_symbols;
	uint8_t uc_band;

	puc_dst_buf = puc_ind_data;

	*puc_dst_buf++ = (uint8_t)SNIFFER_IF_PHY_COMMAND_G3_VERSION;
	*puc_dst_buf++ = (uint8_t)SNIFFER_VERSION;
	*puc_dst_buf++ = (uint8_t)SNIFFER_PL360_G3;

	/* Fill data depending on Data/ACK frame */
	if ((px_ind_data->uc_delimiter_type == DT_SOF_NO_RESP) || (px_ind_data->uc_delimiter_type == DT_SOF_RESP)) {
		/* Data frame */
		/* ModType (high) + ModScheme (low) */
		*puc_dst_buf++ = (uint8_t)((px_ind_data->uc_mod_scheme & 0x0F) + (((px_ind_data->uc_mod_type) << 4) & 0xF0));
		/* Tone Map */
		atpl360_ib_get_param(ATPL360_HOST_BAND_ID, &uc_band, 1);
		if ((uc_band == ATPL360_WB_CENELEC_A) || (uc_band == ATPL360_WB_CENELEC_B)) {
			*puc_dst_buf++ = 0;
			*puc_dst_buf++ = 0;
			*puc_dst_buf++ = px_ind_data->puc_tone_map[0];
		} else {
			*puc_dst_buf++ = px_ind_data->puc_tone_map[2];
			*puc_dst_buf++ = px_ind_data->puc_tone_map[1];
			*puc_dst_buf++ = px_ind_data->puc_tone_map[0];
		}

		/* Number of symbols (2 bytes) */
		spx_desc->get_config(ATPL360_REG_PAY_SYMBOLS_RX, &us_num_symbols, 2, true);

		*puc_dst_buf++ = (uint8_t)(us_num_symbols >> 8);
		*puc_dst_buf++ = (uint8_t)us_num_symbols;
		/* SNR */
		*puc_dst_buf++ = px_ind_data->uc_lqi;
	} else {
		/* ACK */
		/* ModType (high) + ModScheme (low) */
		*puc_dst_buf++ = 0;
		/* Tone Map */
		*puc_dst_buf++ = 0;
		*puc_dst_buf++ = 0;
		*puc_dst_buf++ = 0;
		/* Number of symbols (2 bytes) */
		*puc_dst_buf++ = 0;
		*puc_dst_buf++ = 0;
		/* SNR */
		*puc_dst_buf++ = 0xFF;
	}

	/* Delimiter Type */
	*puc_dst_buf++ = px_ind_data->uc_delimiter_type;
	/* TimeIni */
	ul_time_ini = px_ind_data->ul_rx_time - px_ind_data->ul_frame_duration;

	*puc_dst_buf++ = (ul_time_ini >> 24);
	*puc_dst_buf++ = (ul_time_ini >> 16) & 0xFF;
	*puc_dst_buf++ = (ul_time_ini >> 8) & 0xFF;
	*puc_dst_buf++ = ul_time_ini & 0xFF;
	/* TimeEnd */
	ul_time_end = px_ind_data->ul_rx_time;
	*puc_dst_buf++ = (ul_time_end >> 24);
	*puc_dst_buf++ = (ul_time_end >> 16) & 0xFF;
	*puc_dst_buf++ = (ul_time_end >> 8) & 0xFF;
	*puc_dst_buf++ = ul_time_end & 0xFF;
	/* RSSI */
	*puc_dst_buf++ = (uint8_t)(px_ind_data->us_rssi >> 8);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->us_rssi);
	/* AGC_Factor */
	*puc_dst_buf++ = (uint8_t)(px_ind_data->ul_agc_factor >> 8);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->ul_agc_factor);
	/* Length in bytes */
	*puc_dst_buf++ = (uint8_t)(px_ind_data->us_data_len >> 8);
	*puc_dst_buf++ = (uint8_t)(px_ind_data->us_data_len);
	/* PDU */
	memcpy(puc_dst_buf, px_ind_data->puc_data_buf, px_ind_data->us_data_len);
	puc_dst_buf += px_ind_data->us_data_len;

	/* printf("%u;%u;%u;%u\r\n", px_ind_data->ul_rx_time, px_ind_data->ul_frame_duration, ul_time_ini, ul_time_end); */

	return (puc_dst_buf - puc_ind_data);
}

/**
 * \brief Converts Phy TX Cfm struct to byte buffering in order to report data through ADDONs
 *
 * \param puc_cfm_data       Pointer to destiny buffer
 * \param px_cfm_data        Pointer to struct to convert
 *
 * \return Length of data to buffering
 */
uint16_t atpl360_addon_stringify_cfm(uint8_t *puc_cfm_data, tx_cfm_t *px_cfm_data)
{
	uint8_t *puc_dst_buf;
	uint32_t ul_time_ini, ul_time_end;
	uint16_t us_num_symbols;
	uint8_t uc_band;

	if (px_cfm_data->uc_tx_result != TX_RESULT_SUCCESS) {
		return 0;
	}

	puc_dst_buf = puc_cfm_data;

	*puc_dst_buf++ = SNIFFER_IF_PHY_COMMAND_G3_VERSION;
	*puc_dst_buf++ = SNIFFER_VERSION;
	*puc_dst_buf++ = SNIFFER_PL360_G3;
	/* ModType (high) + ModScheme (low) */
	*puc_dst_buf++ = (uint8_t)((sx_last_tx_msg.uc_mod_scheme & 0x0F) + (((sx_last_tx_msg.uc_mod_type) << 4) & 0xF0));
	/* Tone Map */
	atpl360_ib_get_param(ATPL360_HOST_BAND_ID, &uc_band, 1);
	if ((uc_band == ATPL360_WB_CENELEC_A) || (uc_band == ATPL360_WB_CENELEC_B)) {
		*puc_dst_buf++ = 0;
		*puc_dst_buf++ = 0;
		*puc_dst_buf++ = sx_last_tx_msg.puc_tone_map[0];
	} else {
		*puc_dst_buf++ = sx_last_tx_msg.puc_tone_map[2];
		*puc_dst_buf++ = sx_last_tx_msg.puc_tone_map[1];
		*puc_dst_buf++ = sx_last_tx_msg.puc_tone_map[0];
	}

	/* Number of symbols (2 bytes) */
	spx_desc->get_config(ATPL360_REG_PAY_SYMBOLS_TX, &us_num_symbols, 2, true);
	*puc_dst_buf++ = (uint8_t)(us_num_symbols >> 8);
	*puc_dst_buf++ = (uint8_t)us_num_symbols;

	/* SNR */
	*puc_dst_buf++ = 0xFF;
	/* Delimiter Type */
	*puc_dst_buf++ = sx_last_tx_msg.uc_delimiter_type;
	/* TimeIni */
	ul_time_ini = sx_last_tx_msg.ul_tx_time;
	*puc_dst_buf++ = (uint8_t)(ul_time_ini >> 24);
	*puc_dst_buf++ = (uint8_t)(ul_time_ini >> 16);
	*puc_dst_buf++ = (uint8_t)(ul_time_ini >> 8);
	*puc_dst_buf++ = (uint8_t)ul_time_ini;
	/* TimeEnd */
	ul_time_end = px_cfm_data->ul_tx_time;
	*puc_dst_buf++ = (uint8_t)(ul_time_end >> 24);
	*puc_dst_buf++ = (uint8_t)(ul_time_end >> 16);
	*puc_dst_buf++ = (uint8_t)(ul_time_end >> 8);
	*puc_dst_buf++ = (uint8_t)ul_time_end;
	/* RSSI */
	*puc_dst_buf++ = (uint8_t)(0xFF >> 8);
	*puc_dst_buf++ = (uint8_t)(0xFF);
	/* AGC_Factor */
	*puc_dst_buf++ = (uint8_t)(0xFF >> 8);
	*puc_dst_buf++ = (uint8_t)(0xFF);
	/* Length in bytes */
	*puc_dst_buf++ = (uint8_t)(sx_last_tx_msg.us_data_len >> 8);
	*puc_dst_buf++ = (uint8_t)(sx_last_tx_msg.us_data_len);
	/* PDU */
	memcpy(puc_dst_buf, spuc_sniffer_tx_data_buffer, sx_last_tx_msg.us_data_len);
	puc_dst_buf += sx_last_tx_msg.us_data_len;

	return (puc_dst_buf - puc_cfm_data);
}

/**
 * \brief Converts Phy REG info to byte buffering in order to report data through ADDONs  (NOT USE IN SNIFFER ADDON)
 *
 * \param puc_dst_data       Pointer to destiny buffer
 * \param puc_src_data       Pointer to buffer to extract reg value
 * \param us_reg_size        Register size
 *
 * \return Length of data to buffering
 */
uint16_t atpl360_addon_stringify_reg(uint8_t *puc_dst_data, uint8_t *puc_src_data, uint16_t us_reg_size)
{
	(void)puc_dst_data;
	(void)puc_src_data;
	(void)us_reg_size;

	return 0;
}

/**
 * \brief Converts Phy TX info to byte buffering in order to report data through ADDONs
 *
 * \param puc_tx_data       Pointer to destiny buffer
 * \param px_tx_data        Pointer to struct to convert (not use in sniffer addon)
 *
 * \return Length of data to buffering
 */
uint16_t atpl360_addon_stringify_tx(uint8_t *puc_tx_data, tx_msg_t *px_tx_data)
{
	/* Use internal buffer to report TX messages to sniffer app when cfm arrives */
	(void)puc_tx_data;

	memcpy((uint8_t *)&sx_last_tx_msg, (uint8_t *)px_tx_data, sizeof(tx_msg_t));
	memcpy(spuc_sniffer_tx_data_buffer, sx_last_tx_msg.puc_data_buf, sx_last_tx_msg.us_data_len);

	return 0;
}

/**
 * \brief Function to execute addons commnad
 *
 * \param px_msg  Pointer to command message
 * \param us_len  Length of the command message
 *
 */
void atpl360_addon_cmd(uint8_t *px_msg, uint16_t us_len)
{
	uint8_t uc_sniffer_if_cmd;
	uint8_t *puc_data;
	uint8_t puc_tonemask[72];
	int i, j;

	/* Protection for invalid length */
	if (!us_len) {
		return;
	}

	/* Process received message. Get command */
	uc_sniffer_if_cmd  = px_msg[0];

	switch (uc_sniffer_if_cmd) {
	case SNIFFER_IF_PHY_G3_SET_TONE_MASK_PLC:
		/* Convert bitfield to byte field, reversing order */
		puc_data = px_msg + 1;
		for (i = 8; i >= 0; i--) {
			uint8_t uc_bitfield = puc_data[i];
			for (j = 0; j < 8; j++) {
				puc_tonemask[(i * 8) + j] = (uc_bitfield & 0x01) ? 0 : 1;
				uc_bitfield >>= 1;
			}
		}

		/* Send data to PL360 */
		spx_desc->set_config(ATPL360_REG_TONE_MASK, (void *)&puc_tonemask, NUM_CARRIERS);
		break;

	case SNIFFER_IF_PHY_G3_SET_RF_BAND_OPM_CHANNEL:
	default:
		/* Not supported */
		break;
	}
}

void sniffer_if_init(atpl360_addon_descriptor_t *sx_desc)
{
	spx_desc = sx_desc;
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
