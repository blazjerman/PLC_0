/**
 * \file
 *
 * \brief PHY_CHAT : G3-PLC Phy PLC And Go Applicationn. Module to manage PL360 PHY Layer
 *
 * Copyright (c) 2020 Atmel Corporation. All rights reserved.
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

/* Atmel library includes. */
#include "asf.h"
#include "conf_project.h"
#include "app_phy_ctl.h"

#ifdef PHY_CONTROLLER_DEBUG_ENABLE
#define LOG_PHY_CTL_DEBUG(a)   printf a
#else
#define LOG_PHY_CTL_DEBUG(a)   (void)0
#endif

/* Phy controller callbacks */
static phy_ctl_callbacks_t sx_phy_ctl_cbs;

/* ATPL360 descriptor definition */
static atpl360_descriptor_t sx_atpl360_desc;

/* Exception counters */
static uint8_t suc_err_unexpected;
static uint8_t suc_err_critical;
static uint8_t suc_err_reset;
static uint8_t suc_err_none;
static bool sb_exception_pend;
static bool sb_enabling_pl360;

/* G3 Band identifier */
static uint8_t suc_phy_band;

/* Transmission management */
static tx_msg_t sx_tx_msg;

#ifdef CONF_TONEMASK_STATIC_NOTCHING
/* Tone Mask (Static Notching) array. Each carrier corresponding to the band can be notched (no energy is sent in those carriers) */
/* Each carrier is represented by one byte (0: carrier used; 1: carrier notched). By default it is all 0's in PL360 device */
/* The same Tone Mask must be set in both transmitter and receiver. Otherwise they don't understand each other */
static const uint8_t spuc_tone_mask[PROTOCOL_CARRIERS_MAX] = TONE_MASK_STATIC_NOTCHING;
#endif

/* Tone Map size in bytes {CENELEC-A, FCC, ARIB, CENELEC-B} */
static const uint8_t spuc_tonemap_size[4] = {TONE_MAP_SIZE_CENELEC, TONE_MAP_SIZE_FCC_ARIB, TONE_MAP_SIZE_FCC_ARIB, TONE_MAP_SIZE_CENELEC};
/* Number of subbands {CENELEC-A, FCC, ARIB, CENELEC-B} */
static const uint8_t spuc_numsubbands[4] = {NUM_SUBBANDS_CENELEC_A, NUM_SUBBANDS_FCC, NUM_SUBBANDS_ARIB, NUM_SUBBANDS_CENELEC_B};
#ifdef CONF_TONEMASK_STATIC_NOTCHING
/* Number of carriers {CENELEC-A, FCC, ARIB, CENELEC-B} */
static const uint8_t spuc_num_carriers[4] = {NUM_CARRIERS_CENELEC_A, NUM_CARRIERS_FCC, NUM_CARRIERS_ARIB, NUM_CARRIERS_CENELEC_B};
#endif

/**
 * \brief Set PL360 configuration. Called at initialization (once the binary is loaded) to configure required parameters in PL360 device
 */
static void _set_pl360_configuration(void)
{
	uint8_t uc_value;

	/********* The following lines show how to configure different parameters on PL360 device *********/
	/********* The user can customize it depending on the requirements ********************************/

	/* Configure Band in PL360 Host Controller */
	sx_atpl360_desc.set_config(ATPL360_HOST_BAND_ID, &suc_phy_band, 1);

	/* Configure Coupling and TX parameters */
	pl360_g3_coup_tx_config(&sx_atpl360_desc, suc_phy_band);

	/* Force Transmission to VLO mode by default in order to maximize signal level in anycase */
	/* Disable autodetect mode */
	uc_value = 0;
	sx_atpl360_desc.set_config(ATPL360_REG_CFG_AUTODETECT_IMPEDANCE, &uc_value, 1);
	/* Set VLO mode */
	uc_value = 2;
	sx_atpl360_desc.set_config(ATPL360_REG_CFG_IMPEDANCE, &uc_value, 1);

#ifdef CONF_TONEMASK_STATIC_NOTCHING
	/* Example to configure Tone Mask (Static Notching). Each carrier corresponding to the band can be notched (no energy is sent in those carriers) */
	/* Each carrier is represented by one byte (0: carrier used; 1: carrier notched). By default it is all 0's in PL360 device */
	/* The length is the number of carriers corresponding to the band in use (see "general_defs.h") */
	/* The same Tone Mask must be set in both transmitter and receiver. Otherwise they don't understand each other */
	sx_atpl360_desc.set_config(ATPL360_REG_TONE_MASK, (uint8_t *)spuc_tone_mask, spuc_num_carriers[suc_phy_band - 1]);
#endif

	/* Enable CRC calculation for transmission and reception in PL360 PHY layer */
	/* In Transmission, 16-bit CRC is computed and added to data payload by PHY layer in PL360 device */
	/* In Reception, CRC is checked by PHY layer in PL360 device and the result is reported in uc_crc_ok field in rx_msg_t structure */
	/* The CRC format is the same that uses the G3-PLC stack, which is described in the IEEE 802.15.4 standard. */
	uc_value = 1;
	sx_atpl360_desc.set_config(ATPL360_REG_CRC_TX_RX_CAPABILITY, &uc_value, 1);
}

/**
 * \brief Calculates maximun length in bytes allowed depending on configuration
 * If CRC capability is enabled (ATPL360_REG_CRC_TX_RX_CAPABILITY), 2 bytes corresponding to CRC are discounted
 * The maximum data length depends on:
 *                 - G3 band (CENELEC-A, CENELEC-B, FCC or ARIB)
 *                 - Modulation Type (BPSK Robust, BPSK, QPSK or 8PSK). See "enum mod_types" in "atpl360_comm.h"
 *                 - Modulation Scheme (Differential of coherent). See "enum mod_schemes" in "atpl360_comm.h"
 *                 - Number of Reed-Solomon blocks (uc_2_rs_blocks). Only applies to FCC band. 1 or 2 blocks. With 2 blocks the maximum length is the double
 *                 - Tone Map (puc_tone_map). Dynamic notching: The subbands used to send the data can be chosen with Tone Map
 *                 - Tone Mask (ATPL360_REG_TONE_MASK). Static notching: Carriers can be notched and no energy is sent in those carriers
 *
 * \return Max PSDU length in bytes
 */
static uint16_t _get_max_psdu_len(void)
{
	max_psdu_len_params_t x_max_psdu_len_params;
	uint16_t us_max_psdu_len;

	/* First, ATPL360_REG_MAX_PSDU_LEN_PARAMS has to be set with Tx message parameters to compute maximum PSDU length */
	/* See "max_psdu_len_params_t" struct in "atpl360_comm.h" */

	/* Modulation Type */
	x_max_psdu_len_params.uc_mod_type = sx_tx_msg.uc_mod_type;

	/* Modulation scheme */
	x_max_psdu_len_params.uc_mod_scheme = sx_tx_msg.uc_mod_scheme;

	/* 1 or 2 Reed-Solomon blocks (only for FCC) */
	if (suc_phy_band == ATPL360_WB_FCC) {
		x_max_psdu_len_params.uc_2_rs_blocks = sx_tx_msg.uc_2_rs_blocks;
	}

	/* Tone Map */
	memset(x_max_psdu_len_params.puc_tone_map, 0, TONE_MAP_SIZE_MAX);
	memcpy(x_max_psdu_len_params.puc_tone_map, sx_tx_msg.puc_tone_map, spuc_tonemap_size[suc_phy_band - 1]);

	/* Set parameters for MAX_PSDU_LEN computation in PL360 device */
	sx_atpl360_desc.set_config(ATPL360_REG_MAX_PSDU_LEN_PARAMS, &x_max_psdu_len_params, sizeof(max_psdu_len_params_t));

	/* Get MAX_PSDU_LEN from PL360 device */
	sx_atpl360_desc.get_config(ATPL360_REG_MAX_PSDU_LEN, &us_max_psdu_len, 2, true);

	return us_max_psdu_len;
}

/**
 * \brief Setup parameters to use in Tx message
 */
static void _setup_tx_parameters(void)
{
	uint16_t us_max_data_len;
	uint8_t uc_num_subbands;
	uint8_t uc_i;
	uint8_t uc_tonemap_byte_index;
	uint8_t uc_tonemap_bit_index;

	/* Configure Tone Map (Dynamic Notching) to use all carriers */
	/* Each bit corresponds to a subband */
	/* If the bit is '1' the subband is used to carry data. If the bit is '0' the subband does not carry data, but energy is sent in those carriers */
	/* The number of subbands is different in each G3 band (CENELEC-A, CENELEC-B, FCC, ARIB). See "general_defs.h" */
	/* The number of carriers per subband is 6 for CENELEC-A, 4 for CENELEC-B and 3 for FCC and ARIB */
	/* Full Tone Map: 0x3F0000 (CENELEC-A, 6 subbands); 0x0F0000 (CENELEC-B, 4 subbands); 0xFFFFFF (FCC, 24 subbands); 0xFFFF03 (ARIB, 18 subbands) */
	/* The next loop shows how to go across all subbands from lower to higher frequency */
	memset(sx_tx_msg.puc_tone_map, 0, TONE_MAP_SIZE_MAX);
	uc_num_subbands = spuc_numsubbands[suc_phy_band - 1];
	for (uc_i = 0; uc_i < uc_num_subbands; uc_i++) {
		/* Byte index in Tone Map array */
		uc_tonemap_byte_index = uc_i >> 3;

		/* Bit index in Tone Map byte */
		uc_tonemap_bit_index = uc_i - (uc_tonemap_byte_index << 3);

		/* Set subband active */
		sx_tx_msg.puc_tone_map[uc_tonemap_byte_index] |= (1 << uc_tonemap_bit_index);
	}

	/* Number of Reed-Solomon blocks (parameter only used in FCC). "uc_2_rs_blocks = 0" means 1 block and "uc_2_rs_blocks = 1" means 2 blocks */
	/* 2 Reed-Solomon blocks allows to send the double number of bytes in the same frame than 1 Reed-Solomon blocks (see "_get_max_psdu_len()") */
	/* However, if the message can be sent in one single block, it is better to use 1 block because 2 blocks would add overhead */
	if (suc_phy_band == ATPL360_WB_FCC) {
		/* Set 1 Reed-Solomon block. In this example it cannot be configured dynamically. To test 2 Reed-Solomon blocks change 0 by 1 */
		sx_tx_msg.uc_2_rs_blocks = 0;
	}

	/* Delimiter Type. Data Frame (requiring ACK or not) or acknowledgement (positive or negative). See "enum delimiter_types" in "atpl360_comm.h" file */
	/* In this example ACK management is not needed */
	sx_tx_msg.uc_delimiter_type = DT_SOF_NO_RESP;

	/* Modulation Scheme. Differential or Coherent. See "enum mod_schemes" in "atpl360_comm.h" file */
	/* Coherent Scheme supports worst SNR (about 3 dB) than Differential Scheme */
	/* Differential Scheme provides a bit higher data rate because Coherent Scheme uses some carriers for pilots */
	/* Coherent Scheme requires an accurate crystal oscillator. G3-PLC specifies that the frequency error must be less than 25 PPM */
	sx_tx_msg.uc_mod_scheme = MOD_SCHEME_DIFFERENTIAL;

	/* Modulation Type. See "enum mod_types" in "atpl360_comm.h" file */
	/* Ordered from higher to lower data rate and from higher to lower required SNR (Signal to Noise Ratio): 8PSK, QPSK, BPSK, Robust BPSK */
	sx_tx_msg.uc_mod_type = MOD_TYPE_BPSK;

	/* Transmission Mode. See TX Mode Bit Mask in "atpl360_comm.h" file */
	/* TX_MODE_RELATIVE: Time (ul_tx_time) in relative mode. The message is sent with a delay from the time of Tx Request */
	/* TX_MODE_ABSOLUTE: Time (ul_tx_time) in absolute mode. The message is sent at the specified time, referred to PL360 internal timer (1 us) */
	/* TX_MODE_FORCED: If there is a reception in progress at the same time of transmission, the message is transmitted and the reception is aborted */
	sx_tx_msg.uc_tx_mode = TX_MODE_FORCED | TX_MODE_RELATIVE;

	/* Transmission Time in us. Relative or Absolute time (depending on Transmission Mode) */
	/* TX_MODE_RELATIVE and ul_tx_time = 0: Instantaneous transmission */
	sx_tx_msg.ul_tx_time = 0;

	/* Set transmission power. It represents 3dBs of signal level attenuation per Unit. 0 value means maximum signal level. */
	sx_tx_msg.uc_tx_power = 0;

	/* Get maximum data length allowed with configured Tx Parameters */
	us_max_data_len = _get_max_psdu_len();

	/* Update configured Modulation and maximum data length in Chat App */
	if (sx_phy_ctl_cbs.phy_ctl_update_tx_configuration) {
		sx_phy_ctl_cbs.phy_ctl_update_tx_configuration(sx_tx_msg.uc_mod_scheme, sx_tx_msg.uc_mod_type, us_max_data_len);
	}
}

/**
 * \brief Handler to manage confirmation of the last PLC transmission.
 *
 * \param px_msg_cfm Pointer to struct containing Tx Confirm paramters
 *
 */
static void _handler_data_cfm(tx_cfm_t *px_msg_cfm)
{
	if (sx_phy_ctl_cbs.phy_ctl_data_confirm) {
		sx_phy_ctl_cbs.phy_ctl_data_confirm(px_msg_cfm->uc_tx_result);
	}
}

/**
 * \brief Handler to manage new received PLC message.
 *
 * \param px_msg Pointer to struct containing message parameters and data
 *
 */
static void _handler_data_ind(rx_msg_t *px_msg)
{
	uint8_t *puc_data_buf;
	uint16_t us_data_len;
	uint16_t us_rssi;
	uint8_t uc_lqi;
	enum mod_schemes uc_mod_scheme;
	enum mod_types uc_mod_type;

	/* CRC capability (ATPL360_REG_CRC_TX_RX_CAPABILITY) is enabled */
	/* CRC correctness is checked by PL360 PHY Layer */
	/* The result is reported in uc_crc_ok (1: OK; 0: BAD; 0xFE: Timeout Error; 0xFF: CRC capability disabled) */
	if (px_msg->uc_crc_ok == 1) {
		/* CRC Ok. Get some parameters to show in console */
		/* There are more parameters not used in this example application (see "rx_msg_t" struct in "atpl360_comm.h") */

		/* Get pointer to buffer containing received data */
		puc_data_buf = px_msg->puc_data_buf;

		/* Get data length in bytes. 2 bytes corresponding to CRC are already discounted (ATPL360_REG_CRC_TX_RX_CAPABILITY is enabled) */
		us_data_len = px_msg->us_data_len;

		/* Get Modulation Scheme used in received message */
		uc_mod_scheme = px_msg->uc_mod_scheme;

		/* Get Modulation Type used in received message */
		uc_mod_type = px_msg->uc_mod_type;

		/* Get RSSI (Received Signal Strength Indicator) in dBuV */
		us_rssi = px_msg->us_rssi;

		/* Get LQI (Link Quality Indicator). It is in quarters of dB and 10-dB offset: SNR(dB) = (LQI - 40) / 4 */
		uc_lqi = px_msg->uc_lqi;

		if (sx_phy_ctl_cbs.phy_ctl_rx_msg) {
			sx_phy_ctl_cbs.phy_ctl_rx_msg(puc_data_buf, us_data_len, uc_mod_scheme, uc_mod_type, us_rssi, uc_lqi);
		}
	} else {
		/* CRC Error */
		if (sx_phy_ctl_cbs.phy_ctl_rx_msg_discarded) {
			sx_phy_ctl_cbs.phy_ctl_rx_msg_discarded();
		}
	}
}

/**
 * \brief Handler to manage PL360 Exceptions. This callback is also called after loading binary at initization
 */
static void _handler_exception_event(atpl360_exception_t exception)
{
	LOG_PHY_CTL_DEBUG(("\r\n"));

	switch (exception) {
	case ATPL360_EXCEPTION_UNEXPECTED_SPI_STATUS:
		/* SPI has detected an unexpected status, reset is recommended */
		suc_err_unexpected++;
		LOG_PHY_CTL_DEBUG(("ATPL360_EXCEPTION_UNEXPECTED_SPI_STATUS\r\n"));
		break;

	case ATPL360_EXCEPTION_SPI_CRITICAL_ERROR:
		/* SPI critical error */
		suc_err_critical++;
		LOG_PHY_CTL_DEBUG(("ATPL360_EXCEPTION_SPI_CRITICAL_ERROR\r\n"));
		break;

	case ATPL360_EXCEPTION_RESET:
		/* Device Reset */
		if (sb_enabling_pl360) {
			/* This callback is also called after loading binary at initization */
			/* This message is shown to indicate that the followitn exception is normal because PL360 binary has just been loaded */
			sb_enabling_pl360 = false;
			LOG_PHY_CTL_DEBUG(("PL360 initialization event: "));
		}

		suc_err_reset++;
		LOG_PHY_CTL_DEBUG(("ATPL360_EXCEPTION_RESET\r\n"));
		break;

	default:
		LOG_PHY_CTL_DEBUG(("ATPL360_EXCEPTION_UNKNOWN\r\n"));
		suc_err_none++;
	}

	/* Set flag to manage exception in phy_ctl_process() */
	sb_exception_pend = true;
}

/**
 * \brief Get PL360 binary addressing.
 *
 * \param pul_address   Pointer to store the initial address of PL360 binary data
 *
 * \return Size of PL360 binary file
 */
static uint32_t _get_pl360_bin_addressing(uint32_t *pul_address)
{
	uint32_t ul_bin_addr;
	uint8_t *puc_bin_start;
	uint8_t *puc_bin_end;

#if ((ATPL360_WB != ATPL360_WB_CENELEC_A) && (ATPL360_WB != ATPL360_WB_CENELEC_B) && (ATPL360_WB != ATPL360_WB_FCC) && (ATPL360_WB != ATPL360_WB_ARIB))
#  error ERROR in PHY band definition
#endif

#if defined(CONF_MULTIBAND_FCC_CENA)
#  if ((ATPL360_WB == ATPL360_WB_FCC) || (ATPL360_WB == ATPL360_WB_CENELEC_A))
	/* Multiband: FCC and CENELEC-A binaries are linked */
	if (suc_phy_band == ATPL360_WB_FCC) {
		/* Select FCC binary */
#    if defined (__CC_ARM)
		extern uint8_t atpl_bin_fcc_start[];
		extern uint8_t atpl_bin_fcc_end[];
		ul_bin_addr = (int)(atpl_bin_fcc_start - 1);
		puc_bin_start = atpl_bin_fcc_start - 1;
		puc_bin_end = atpl_bin_fcc_end;
#    elif defined (__GNUC__)
		extern uint8_t atpl_bin_fcc_start;
		extern uint8_t atpl_bin_fcc_end;
		ul_bin_addr = (int)&atpl_bin_fcc_start;
		puc_bin_start = (uint8_t *)&atpl_bin_fcc_start;
		puc_bin_end = (uint8_t *)&atpl_bin_fcc_end;
#    elif defined (__ICCARM__)
#      pragma section = "P_atpl_bin_fcc"
		extern uint8_t atpl_bin_fcc;
		ul_bin_addr = (int)&atpl_bin_fcc;
		puc_bin_start = __section_begin("P_atpl_bin_fcc");
		puc_bin_end = __section_end("P_atpl_bin_fcc");
#    else
#      error This compiler is not supported for now.
#    endif
	} else { /* ATPL360_WB_CENELEC_A */
		 /* Select CENELEC-A binary */
#    if defined (__CC_ARM)
		extern uint8_t atpl_bin_cena_start[];
		extern uint8_t atpl_bin_cena_end[];
		ul_bin_addr = (int)(atpl_bin_cena_start - 1);
		puc_bin_start = atpl_bin_cena_start - 1;
		puc_bin_end = atpl_bin_cena_end;
#    elif defined (__GNUC__)
		extern uint8_t atpl_bin_cena_start;
		extern uint8_t atpl_bin_cena_end;
		ul_bin_addr = (int)&atpl_bin_cena_start;
		puc_bin_start = (int)&atpl_bin_cena_start;
		puc_bin_end = (int)&atpl_bin_cena_end;
#    elif defined (__ICCARM__)
#      pragma section = "P_atpl_bin_cena"
		extern uint8_t atpl_bin_cena;
		ul_bin_addr = (int)&atpl_bin_cena;
		puc_bin_start = __section_begin("P_atpl_bin_cena");
		puc_bin_end = __section_end("P_atpl_bin_cena");
#    else
#      error This compiler is not supported for now.
#    endif
	}

#  else
#    error Work-band not supported
#  endif
#elif defined(CONF_MULTIBAND_FCC_CENB)
#  if ((ATPL360_WB == ATPL360_WB_FCC) || (ATPL360_WB == ATPL360_WB_CENELEC_B))
	/* Multiband: FCC and CENELEC-B binaries are linked */
	if (suc_phy_band == ATPL360_WB_FCC) {
		/* Select FCC binary */
#    if defined (__CC_ARM)
		extern uint8_t atpl_bin_fcc_start[];
		extern uint8_t atpl_bin_fcc_end[];
		ul_bin_addr = (int)(atpl_bin_fcc_start - 1);
		puc_bin_start = atpl_bin_fcc_start - 1;
		puc_bin_end = atpl_bin_fcc_end;
#    elif defined (__GNUC__)
		extern uint8_t atpl_bin_fcc_start;
		extern uint8_t atpl_bin_fcc_end;
		ul_bin_addr = (int)&atpl_bin_fcc_start;
		puc_bin_start = (uint8_t *)&atpl_bin_fcc_start;
		puc_bin_end = (uint8_t *)&atpl_bin_fcc_end;
#    elif defined (__ICCARM__)
#      pragma section = "P_atpl_bin_fcc"
		extern uint8_t atpl_bin_fcc;
		ul_bin_addr = (int)&atpl_bin_fcc;
		puc_bin_start = __section_begin("P_atpl_bin_fcc");
		puc_bin_end = __section_end("P_atpl_bin_fcc");
#    else
#      error This compiler is not supported for now.
#    endif
	} else { /* ATPL360_WB_CENELEC_B */
		 /* Select CENELEC-B binary */
#    if defined (__CC_ARM)
		extern uint8_t atpl_bin_cenb_start[];
		extern uint8_t atpl_bin_cenb_end[];
		ul_bin_addr = (int)(atpl_bin_cenb_start - 1);
		puc_bin_start = atpl_bin_cenb_start - 1;
		puc_bin_end = atpl_bin_cenb_end;
#    elif defined (__GNUC__)
		extern uint8_t atpl_bin_cenb_start;
		extern uint8_t atpl_bin_cenb_end;
		ul_bin_addr = (int)&atpl_bin_cenb_start;
		puc_bin_start = (int)&atpl_bin_cenb_start;
		puc_bin_end = (int)&atpl_bin_cenb_end;
#    elif defined (__ICCARM__)
#      pragma section = "P_atpl_bin_cenb"
		extern uint8_t atpl_bin_cenb;
		ul_bin_addr = (int)&atpl_bin_cenb;
		puc_bin_start = __section_begin("P_atpl_bin_cenb");
		puc_bin_end = __section_end("P_atpl_bin_cenb");
#    else
#      error This compiler is not supported for now.
#    endif
	}

#  else
#    error Work-band not supported
#  endif
#else /* #if defined(CONF_MULTIBAND_FCC_CENA) */
	/* Only one binary linked */
#  if defined (__CC_ARM)
	extern uint8_t atpl_bin_start[];
	extern uint8_t atpl_bin_end[];
	ul_bin_addr = (int)(atpl_bin_start - 1);
	puc_bin_start = atpl_bin_start - 1;
	puc_bin_end = atpl_bin_end;
#  elif defined (__GNUC__)
	extern uint8_t atpl_bin_start;
	extern uint8_t atpl_bin_end;
	ul_bin_addr = (int)&atpl_bin_start;
	puc_bin_start = (int)&atpl_bin_start;
	puc_bin_end = (int)&atpl_bin_end;
#  elif defined (__ICCARM__)
#    pragma section = "P_atpl_bin"
	extern uint8_t atpl_bin;
	ul_bin_addr = (int)&atpl_bin;
	puc_bin_start = __section_begin("P_atpl_bin");
	puc_bin_end = __section_end("P_atpl_bin");
#  else
#    error This compiler is not supported for now.
#  endif
#endif /* #if defined(CONF_MULTIBAND_FCC_CENA) */
	*pul_address = ul_bin_addr;
	/* cppcheck-suppress deadpointer */
	return ((uint32_t)puc_bin_end - (uint32_t)puc_bin_start);
}

/**
 * \brief Enables PL360 device. Load PHY binary.
 *
 */
static void _pl360_enable(void)
{
	uint32_t ul_bin_addr;
	uint32_t ul_bin_size;
	uint8_t uc_ret;
	char puc_version_str[11];
	uint8_t puc_version_num[4];

	/* Get PL360 bininary address and size */
	ul_bin_size = _get_pl360_bin_addressing(&ul_bin_addr);

	/* Enable PL360: Load binary */
	LOG_PHY_CTL_DEBUG(("\r\nEnabling PL360 device: Loading PHY binary\r\n"));
	sb_enabling_pl360 = true;
	uc_ret = atpl360_enable(ul_bin_addr, ul_bin_size);
	if (uc_ret == ATPL360_ERROR) {
		LOG_PHY_CTL_DEBUG(("\r\nCRITICAL ERROR: PL360 binary load failed (%d)\r\n", uc_ret));
		while (1) {
		}
	}

	LOG_PHY_CTL_DEBUG(("\r\nPL360 binary loaded correctly\r\n"));

	/* Get PHY version (string) */
	sx_atpl360_desc.get_config(ATPL360_REG_VERSION_STR, puc_version_str, 11, true);
	LOG_PHY_CTL_DEBUG(("PHY version: %.*s (", 11, puc_version_str));

	/* Get PHY version (hex) */
	sx_atpl360_desc.get_config(ATPL360_REG_VERSION_NUM, puc_version_num, 4, true);

	/* puc_version_num[2] correspons to G3 band [0x01: CEN-A, 0x02: FCC, 0x03: ARIB, 0x04: CEN-B] */
	switch (puc_version_num[2]) {
	case ATPL360_WB_CENELEC_A:
		LOG_PHY_CTL_DEBUG(("CENELEC-A band: 35 - 91 kHz)\r\n"));
		break;

	case ATPL360_WB_FCC:
		LOG_PHY_CTL_DEBUG(("FCC band: 154 - 488 kHz)\r\n"));
		break;

	case ATPL360_WB_ARIB:
		LOG_PHY_CTL_DEBUG(("ARIB band: 154 - 404 kHz)\r\n"));
		break;

	case ATPL360_WB_CENELEC_B:
		LOG_PHY_CTL_DEBUG(("CENELEC-B band: 98 - 122 kHz)\r\n"));
		break;

	default:
		LOG_PHY_CTL_DEBUG(("Unknown band)\r\n"));
		break;
	}

	if (puc_version_num[2] != suc_phy_band) {
		LOG_PHY_CTL_DEBUG(("ERROR: PHY band does not match with band configured in application\r\n"));
	}
}

/**
 * \brief Initialization of PL360 PHY Layer.
 *
 */
void phy_ctl_pl360_init(uint8_t uc_band)
{
	atpl360_dev_callbacks_t x_atpl360_cbs;
	atpl360_hal_wrapper_t x_atpl360_hal_wrp;

	/* Initialize G3 band static variable */
	suc_phy_band = uc_band;

	/* Initialize PL360 controller */
	x_atpl360_hal_wrp.plc_init = pplc_if_init;
	x_atpl360_hal_wrp.plc_reset = pplc_if_reset;
	x_atpl360_hal_wrp.plc_set_stby_mode = pplc_if_set_stby_mode;
	x_atpl360_hal_wrp.plc_set_handler = pplc_if_set_handler;
	x_atpl360_hal_wrp.plc_send_boot_cmd = pplc_if_send_boot_cmd;
	x_atpl360_hal_wrp.plc_write_read_cmd = pplc_if_send_wrrd_cmd;
	x_atpl360_hal_wrp.plc_enable_int = pplc_if_enable_interrupt;
	x_atpl360_hal_wrp.plc_delay = pplc_if_delay;
	x_atpl360_hal_wrp.plc_get_thw = pplc_if_get_thermal_warning;
	atpl360_init(&sx_atpl360_desc, &x_atpl360_hal_wrp);

	/* Callback functions configuration. Set NULL as Not used */
	x_atpl360_cbs.data_confirm = _handler_data_cfm;
	x_atpl360_cbs.data_indication = _handler_data_ind;
	x_atpl360_cbs.exception_event = _handler_exception_event;
	x_atpl360_cbs.addons_event = NULL;
	x_atpl360_cbs.sleep_mode_cb = NULL;
	x_atpl360_cbs.debug_mode_cb = NULL;
	sx_atpl360_desc.set_callbacks(&x_atpl360_cbs);

	/* Enable PL360 device: Load binary */
	_pl360_enable();
}

/**
 * \brief Set the callbacks for PHY controller.
 *
 * \param px_phy_callbacks Pointer to structure containing the callback functions.
 *
 */
void phy_ctl_set_callbacks(phy_ctl_callbacks_t *px_phy_callbacks)
{
	sx_phy_ctl_cbs.phy_ctl_data_confirm = px_phy_callbacks->phy_ctl_data_confirm;
	sx_phy_ctl_cbs.phy_ctl_rx_msg_discarded = px_phy_callbacks->phy_ctl_rx_msg_discarded;
	sx_phy_ctl_cbs.phy_ctl_rx_msg = px_phy_callbacks->phy_ctl_rx_msg;
	sx_phy_ctl_cbs.phy_ctl_update_tx_configuration = px_phy_callbacks->phy_ctl_update_tx_configuration;
}

/**
 * \brief Send PLC message.
 *
 * \param puc_data_buff Pointer to buffer containing the data to send.
 * \param us_data_len Data length in bytes.
 *
 * \return Result of sending the message to the PL360
 */
uint8_t phy_ctl_send_msg(uint8_t *puc_data_buff, uint16_t us_data_len)
{
	uint8_t uc_result;

	/* Set pointer to data buffer in Tx Parameters structure */
	sx_tx_msg.puc_data_buf = puc_data_buff;

	/* Set data length in Tx Parameters structure */
	/* It should be equal or less than Maximum Data Length (see _get_max_psdu_len) */
	/*Otherwise TX_RESULT_INV_LENGTH will be reported in Tx Confirm (_handler_data_cfm) */
	sx_tx_msg.us_data_len = us_data_len;

	/* Send PLC message. send_data returns TX_RESULT_PROCESS if transmission was correctly programmed */
	/* The result will be reported in Tx Confirm (_handler_data_cfm) when message is completely sent */
	uc_result = sx_atpl360_desc.send_data(&sx_tx_msg);

	return uc_result;
}

/**
 * \brief Set Modulation Scheme.
 * It is stored at application level, not in PL360 PHY Layer. It is sent to PL360 PHY Layer in Tx Request (send_data).
 *
 * \param uc_mod_scheme Modulation Scheme to be configured.
 *
 * \return Maximum data length that can be transmitted with new Modulation.
 */
uint16_t phy_ctl_set_mod_scheme(enum mod_schemes uc_mod_scheme)
{
	uint16_t us_max_data_len;

	/* Store Modulation Scheme in Tx Parameters structure */
	sx_tx_msg.uc_mod_scheme = uc_mod_scheme;

	/* Get maximum data length that can be transmitted with new Modulation */
	us_max_data_len = _get_max_psdu_len();
	return us_max_data_len;
}

/**
 * \brief Set Modulation Type.
 * It is stored at application level, not in PL360 PHY Layer. It is sent to PL360 PHY Layer in Tx Request (send_data).
 *
 * \param uc_mod_type Modulation Type to be configured.
 *
 * \return Maximum data length that can be transmitted with new Modulation.
 */
uint16_t phy_ctl_set_mod_type(enum mod_types uc_mod_type)
{
	uint16_t us_max_data_len;

	/* Store Modulation Type in Tx Parameters structure */
	sx_tx_msg.uc_mod_type = uc_mod_type;

	/* Get maximum data length that can be transmitted with new Modulation */
	us_max_data_len = _get_max_psdu_len();
	return us_max_data_len;
}

#if defined(CONF_MULTIBAND_FCC_CENA)

/**
 * \brief Change G3 band. In this example it is only possible to change between CENELEC-A and FCC.
 * CONF_MULTIBAND_FCC_CENA must be defined to enable Multiband capability.
 * PLCOUP011 should be used for Multiband purposes (Branch 0 for CENELEC-A and Branch 1 for FCC)
 *
 * \param uc_band Band to be configured (see band definitions in "general_defs.h").
 *
 * \retval false. Invalid configuration
 * \retval true. Band reconfigured successfully
 */
bool phy_ctl_set_band(uint8_t uc_band)
{
	if (uc_band == suc_phy_band) {
		/* Band has not changed: Skipping Band Reconfiguration */
		return true;
	}

	if ((uc_band == ATPL360_WB_CENELEC_A) || (uc_band == ATPL360_WB_FCC)) {
		/* Reset PL360 device to load new binary */
		atpl360_disable();

		/* Set static Band variable */
		suc_phy_band = uc_band;

		/* Enable PL360 device: Load binary to change Band */
		_pl360_enable();

		return true;
	} else {
		return false;
	}
}

#elif defined(CONF_MULTIBAND_FCC_CENB)

/**
 * \brief Change G3 band. In this example it is only possible to change between CENELEC-B and FCC.
 * CONF_MULTIBAND_FCC_CENB must be defined to enable Multiband capability.
 *
 * \param uc_band Band to be configured (see band definitions in "general_defs.h").
 *
 * \retval false. Invalid configuration
 * \retval true. Band reconfigured successfully
 */
bool phy_ctl_set_band(uint8_t uc_band)
{
	if (uc_band == suc_phy_band) {
		/* Band has not changed: Skipping Band Reconfiguration */
		return true;
	}

	if ((uc_band == ATPL360_WB_CENELEC_B) || (uc_band == ATPL360_WB_FCC)) {
		/* Reset PL360 device to load new binary */
		atpl360_disable();

		/* Set static Band variable */
		suc_phy_band = uc_band;

		/* Enable PL360 device: Load binary to change Band */
		_pl360_enable();

		return true;
	} else {
		return false;
	}
}

#endif

/**
 * \brief Get current Number of Reed-Solomon blocks configured. Only applies to FCC band.
 * It is stored at application level, not in PL360 PHY Layer. It is sent to PL360 PHY Layer in Tx Request (send_data).
 *
 * \return Current Number of Reed-Solomon blocks configured.
 */
uint8_t phy_ctl_get_2_rs_blocks(void)
{
	if (suc_phy_band == ATPL360_WB_FCC) {
		/* Return Number of Reed-Solomon blocks stored in Tx Parameters structure */
		return sx_tx_msg.uc_2_rs_blocks;
	} else {
		/* This parameter is only used in FCC band. In other bands, only 1 Reed-Solomon block can be configured */
		return 0;
	}
}

/**
 * \brief Get current Modulation Scheme configured.
 * It is stored at application level, not in PL360 PHY Layer. It is sent to PL360 PHY Layer in Tx Request (send_data).
 *
 * \return Current Modulation Scheme configured.
 */
enum mod_schemes phy_ctl_get_mod_scheme(void)
{
	/* Return Modulation Scheme stored in Tx Parameters structure */
	return sx_tx_msg.uc_mod_scheme;
}

/**
 * \brief Get current Modulation Type configured.
 * It is stored at application level, not in PL360 PHY Layer. It is sent to PL360 PHY Layer in Tx Request (send_data).
 *
 * \return Current Modulation Type configured.
 */
enum mod_types phy_ctl_get_mod_type(void)
{
	/* Return Modulation Type stored in Tx Parameters structure */
	return sx_tx_msg.uc_mod_type;
}

/**
 * \brief Get current G3 band configured.
 *
 * \return Current G3 band configured.
 */
uint8_t phy_ctl_get_band(void)
{
	/* Return G3 band configured */
	return suc_phy_band;
}

/**
 * \brief Phy Controller module process.
 */
void phy_ctl_process(void)
{
	/* Manage PL360 exceptions. At initialization ATPL360_EXCEPTION_RESET is reported */
	if (sb_exception_pend) {
		/* Clear exception flag */
		sb_exception_pend = false;

		/* Set PL360 specific configuration from application */
		/* Called at initialization and if an exception ocurrs */
		/* If an exception occurs, PL360 is reset and some parameters may have to be reconfigured */
		_set_pl360_configuration();

		/* Setup G3-PLC parameters to use in transmission */
		_setup_tx_parameters();
	}

	/* Check ATPL360 pending events. It must be called from application periodically to handle PHY Layer events */
	atpl360_handle_events();
}
