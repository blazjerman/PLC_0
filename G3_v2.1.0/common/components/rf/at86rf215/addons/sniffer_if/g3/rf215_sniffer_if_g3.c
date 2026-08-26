/**
 *
 * \file
 *
 * \brief RF215 G3 sniffer addon.
 *
 * Copyright (c) 2020 Microchip Technology Inc. and its subsidiaries.
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

/* RF215 includes */
#include "at86rf.h"
#include "rf215_addon.h"
#include "rf215_sniffer_if.h"
#include "rf215_phy_defs.h"

/* System includes */
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif

/* G3 sniffer identifiers and version */
#define SNIFFER_ATPL250                                 0x02
#define SNIFFER_PL360_G3                                0x12
#define SNIFFER_PL360_G3_EXTENDED                       0x32
#define SNIFFER_PLC_G3_SIMULATOR                        0xC0
#define SNIFFER_RF215_G3                                0x14
#define SNIFFER_RF215_G3_EXTENDED                       0x34
#define SNIFFER_RF215_G3_SIMULATOR                      0xD4

#define SNIFFER_VERSION                                 0x02
#define SNIFFER_IF_PHY_COMMAND_G3_VERSION               0x00

/* G3 sniffer commands identifiers */
#define SNIFFER_IF_PHY_G3_SET_TONE_MASK_PLC             1     /* Set G3-PLC PHY Tone Mask */
#define SNIFFER_IF_PHY_G3_SET_RF_BAND_OPM_CHANNEL       2     /* Set RF band, operating mode and channel */

/* G3 sniffer modulation types (PLC modulation types + RF PHY types) */
#define SNIFFER_IF_MOD_TYPE_PLC_BPSK                    0x00
#define SNIFFER_IF_MOD_TYPE_PLC_QPSK                    0x01
#define SNIFFER_IF_MOD_TYPE_PLC_8PSK                    0x02
#define SNIFFER_IF_MOD_TYPE_PLC_16QAM                   0x03
#define SNIFFER_IF_MOD_TYPE_PLC_BPSK_ROBO               0x04
#define SNIFFER_IF_MOD_TYPE_RF_FSK50                    0x05
#define SNIFFER_IF_MOD_TYPE_RF_FSK100                   0x06
#define SNIFFER_IF_MOD_TYPE_RF_FSK150                   0x07
#define SNIFFER_IF_MOD_TYPE_RF_FSK200                   0x08
#define SNIFFER_IF_MOD_TYPE_RF_FSK300                   0x09
#define SNIFFER_IF_MOD_TYPE_RF_FSK400                   0x0A
#define SNIFFER_IF_MOD_TYPE_RF_4FSK50                   0x0B
#define SNIFFER_IF_MOD_TYPE_RF_4FSK100                  0x0C
#define SNIFFER_IF_MOD_TYPE_RF_4FSK150                  0x0D
#define SNIFFER_IF_MOD_TYPE_RF_4FSK200                  0x0E
#define SNIFFER_IF_MOD_TYPE_RF_4FSK300                  0x0F
#define SNIFFER_IF_MOD_TYPE_RF_4FSK400                  0x10
#define SNIFFER_IF_MOD_TYPE_RF_OFDM1                    0x11
#define SNIFFER_IF_MOD_TYPE_RF_OFDM2                    0x12
#define SNIFFER_IF_MOD_TYPE_RF_OFDM3                    0x13
#define SNIFFER_IF_MOD_TYPE_RF_OFDM4                    0x14

/* G3 sniffer modulation schemes (PLC modulation schemes + RF modulations) */
#define SNIFFER_IF_MOD_SCHEME_PLC_DIF                   0x00
#define SNIFFER_IF_MOD_SCHEME_PLC_COH                   0x01
#define SNIFFER_IF_MOD_SCHEME_RF_FSK_FEC_OFF            0x02
#define SNIFFER_IF_MOD_SCHEME_RF_FSK_FEC_ON             0x03
#define SNIFFER_IF_MOD_SCHEME_RF_OFDM_MCS0              0x04
#define SNIFFER_IF_MOD_SCHEME_RF_OFDM_MCS1              0x05
#define SNIFFER_IF_MOD_SCHEME_RF_OFDM_MCS2              0x06
#define SNIFFER_IF_MOD_SCHEME_RF_OFDM_MCS3              0x07
#define SNIFFER_IF_MOD_SCHEME_RF_OFDM_MCS4              0x08
#define SNIFFER_IF_MOD_SCHEME_RF_OFDM_MCS5              0x09
#define SNIFFER_IF_MOD_SCHEME_RF_OFDM_MCS6              0x0A

#ifdef AT86RF_ADDONS_ENABLE

static uint8_t _rf215_sniffer_mod_type(uint8_t uc_trx_id)
{
	at86rf_phy_cfg_t *px_phy_cfg;
	uint8_t uc_type;

	/* Modulation type depending on RF PHY configuration */
	px_phy_cfg = &gpx_phy_cfg[uc_trx_id];
	if (px_phy_cfg->uc_phy_mod == AT86RF_PHY_MOD_FSK) {
		at86rf_fsk_cfg_t *px_fsk_cfg = &px_phy_cfg->u_mod_cfg.x_fsk;
		uc_type = (uint8_t)px_fsk_cfg->uc_symrate;
		if (px_fsk_cfg->uc_modord == AT86RF_FSK_MODORD_2FSK) {
			uc_type += SNIFFER_IF_MOD_TYPE_RF_FSK50;
		} else { /* AT86RF_FSK_MODORD_4FSK */
			uc_type += SNIFFER_IF_MOD_TYPE_RF_4FSK50;
		}
	} else { /* AT86RF_PHY_MOD_OFDM */
		uc_type = (uint8_t)px_phy_cfg->u_mod_cfg.x_ofdm.uc_opt;
		uc_type += SNIFFER_IF_MOD_TYPE_RF_OFDM1;
	}

	return uc_type;
}

/**
 * \brief Converts PHY RX indication struct to byte buffer for G3 sniffer
 *
 * \param uc_trx_id TRX identifier
 * \param puc_dst_buf Pointer to destination buffer
 * \param px_rx_ind Pointer to RX indication struct
 *
 * \return Length of sniffer data buffer
 */
uint16_t rf215_sniffer_if_stringify_ind(uint8_t uc_trx_id, uint8_t *puc_dst_buf, at86rf_rx_ind_t *px_rx_ind)
{
	uint32_t ul_time_ini;
	uint32_t ul_time_end;
	uint16_t us_len;
	uint16_t us_pay_symbols;
	int16_t ss_rssi;

	/* G3 version (not used in RF) */
	puc_dst_buf[0] = (uint8_t)SNIFFER_IF_PHY_COMMAND_G3_VERSION;

	/* Sniffer version and sniffer type */
	puc_dst_buf[1] = (uint8_t)SNIFFER_VERSION;
	puc_dst_buf[2] = (uint8_t)SNIFFER_RF215_G3;

	/* Modulation scheme depending on RF PHY configuration */
	puc_dst_buf[3] = (uint8_t)px_rx_ind->x_mod_params.x_ofdm.uc_mcs;
	if (gpx_phy_cfg[uc_trx_id].uc_phy_mod == AT86RF_PHY_MOD_FSK) {
		/* FSK modulation (FEC on/off) */
		puc_dst_buf[3] += SNIFFER_IF_MOD_SCHEME_RF_FSK_FEC_OFF;
	} else {
		/* OFDM modulation (MCS) */
		puc_dst_buf[3] += SNIFFER_IF_MOD_SCHEME_RF_OFDM_MCS0;
	}

	/* Modulation type depending on RF PHY configuration */
	puc_dst_buf[4] = _rf215_sniffer_mod_type(uc_trx_id);

	/* Correct FCS flag */
	puc_dst_buf[5] = (uint8_t)px_rx_ind->b_fcs_ok;

	/* Number of payload symbols */
	us_pay_symbols = gpx_phy_ctl[uc_trx_id].us_rx_pay_symbols;
	puc_dst_buf[7] = (uint8_t)(us_pay_symbols >> 8);
	puc_dst_buf[8] = (uint8_t)(us_pay_symbols);

	/* Initial and end time of RX frame */
	ul_time_ini = px_rx_ind->ul_rx_time_ini;
	ul_time_end = ul_time_ini + px_rx_ind->ul_frame_duration;
	puc_dst_buf[11] = (uint8_t)(ul_time_ini >> 24);
	puc_dst_buf[12] = (uint8_t)(ul_time_ini >> 16);
	puc_dst_buf[13] = (uint8_t)(ul_time_ini >> 8);
	puc_dst_buf[14] = (uint8_t)(ul_time_ini);
	puc_dst_buf[15] = (uint8_t)(ul_time_end >> 24);
	puc_dst_buf[16] = (uint8_t)(ul_time_end >> 16);
	puc_dst_buf[17] = (uint8_t)(ul_time_end >> 8);
	puc_dst_buf[18] = (uint8_t)(ul_time_end);

	/* RSSI */
	ss_rssi = (int16_t)px_rx_ind->sc_rssi_dBm;
	puc_dst_buf[19] = (uint8_t)(ss_rssi >> 8);
	puc_dst_buf[20] = (uint8_t)(ss_rssi);

	/* Data PSDU length (including G3-RF FCS) */
	us_len = px_rx_ind->us_psdu_len;
	puc_dst_buf[23] = (uint8_t)(us_len >> 8);
	puc_dst_buf[24] = (uint8_t)(us_len);

	/* Copy PHY data message */
	memcpy(puc_dst_buf + RF215_SNIFFER_MSG_HEADER_LEN, px_rx_ind->puc_data, us_len);

	return us_len + RF215_SNIFFER_MSG_HEADER_LEN;
}

/**
 * \brief Converts PHY TX confirm struct to byte buffer for G3 sniffer
 *
 * \param uc_trx_id TRX identifier
 * \param puc_dst_buf Pointer to destination buffer
 * \param px_tx_cfm Pointer to TX confirm struct
 */
void rf215_sniffer_if_stringify_cfm(uint8_t uc_trx_id, uint8_t *puc_dst_buf, at86rf_tx_cfm_t *px_tx_cfm)
{
	uint32_t ul_time_ini;
	uint32_t ul_time_end;
	uint16_t us_pay_symbols;

	/* G3 version (not used in RF) */
	puc_dst_buf[0] = (uint8_t)SNIFFER_IF_PHY_COMMAND_G3_VERSION;

	/* Sniffer version and sniffer type */
	puc_dst_buf[1] = (uint8_t)SNIFFER_VERSION;
	puc_dst_buf[2] = (uint8_t)SNIFFER_RF215_G3;

	/* Modulation scheme depending on RF PHY configuration */
	if (gpx_phy_cfg[uc_trx_id].uc_phy_mod == AT86RF_PHY_MOD_FSK) {
		/* FSK modulation (FEC on/off) */
		puc_dst_buf[3] += SNIFFER_IF_MOD_SCHEME_RF_FSK_FEC_OFF;
	} else {
		/* OFDM modulation (MCS) */
		puc_dst_buf[3] += SNIFFER_IF_MOD_SCHEME_RF_OFDM_MCS0;
	}

	/* Modulation type depending on RF PHY configuration */
	puc_dst_buf[4] = _rf215_sniffer_mod_type(uc_trx_id);

	/* Correct FCS flag */
	puc_dst_buf[5] = (uint8_t)true;

	/* Number of payload symbols */
	us_pay_symbols = gpx_phy_ctl[uc_trx_id].us_tx_pay_symbols;
	puc_dst_buf[7] = (uint8_t)(us_pay_symbols >> 8);
	puc_dst_buf[8] = (uint8_t)(us_pay_symbols);

	/* Initial and end time of RX frame */
	ul_time_ini = px_tx_cfm->ul_tx_time_ini;
	ul_time_end = ul_time_ini + px_tx_cfm->ul_frame_duration;
	puc_dst_buf[11] = (uint8_t)(ul_time_ini >> 24);
	puc_dst_buf[12] = (uint8_t)(ul_time_ini >> 16);
	puc_dst_buf[13] = (uint8_t)(ul_time_ini >> 8);
	puc_dst_buf[14] = (uint8_t)(ul_time_ini);
	puc_dst_buf[15] = (uint8_t)(ul_time_end >> 24);
	puc_dst_buf[16] = (uint8_t)(ul_time_end >> 16);
	puc_dst_buf[17] = (uint8_t)(ul_time_end >> 8);
	puc_dst_buf[18] = (uint8_t)(ul_time_end);
}

/**
 * \brief Converts PHY TX request struct to byte buffer for G3 sniffer
 *
 * \param puc_dst_buf Pointer to destination buffer
 * \param px_tx_params Pointer to TX parameters
 *
 * \return Length of sniffer data buffer
 */
uint16_t rf215_sniffer_if_stringify_tx(uint8_t *puc_dst_buf, at86rf_tx_params_t *px_tx_params)
{
	uint16_t us_psdu_len;
	uint16_t us_data_len;
	int16_t ss_rssi;

	/* Store modulation frame parameter */
	puc_dst_buf[3] = (uint8_t)px_tx_params->x_mod_params.x_ofdm.uc_mcs;

	/* RSSI */
	ss_rssi = 14 - (int16_t)px_tx_params->uc_txpwr_att;
	puc_dst_buf[19] = (uint8_t)(ss_rssi >> 8);
	puc_dst_buf[20] = (uint8_t)(ss_rssi);

	/* Data PSDU length (including G3-RF FCS) */
	us_psdu_len = px_tx_params->us_psdu_len;
	puc_dst_buf[23] = (uint8_t)(us_psdu_len >> 8);
	puc_dst_buf[24] = (uint8_t)(us_psdu_len);

	/* Copy PHY data message (without FCS) */
	us_data_len = us_psdu_len - AT86RF_FCS_LEN;
	memcpy(puc_dst_buf + RF215_SNIFFER_MSG_HEADER_LEN, px_tx_params->puc_data, us_data_len);

	/* FCS set to 0 if automatically computed */
	memset(puc_dst_buf + RF215_SNIFFER_MSG_HEADER_LEN + us_data_len, 0, AT86RF_FCS_LEN);

	return us_psdu_len + RF215_SNIFFER_MSG_HEADER_LEN;
}

/**
 * \brief Function to execute addons commnad
 *
 * \param puc_rx_msg  Pointer to command message
 * \param us_len  Length of the command message
 *
 */
void at86rf_addon_command(uint8_t *puc_rx_msg, uint16_t us_len)
{
	uint16_t us_band_opmode, us_channel;
	uint8_t uc_sniffer_if_cmd;

	/* Protection for invalid length */
	if (!us_len) {
		return;
	}

	/* Process received message. Get command */
	uc_sniffer_if_cmd  = puc_rx_msg[0];

	switch (uc_sniffer_if_cmd) {
	case SNIFFER_IF_PHY_G3_SET_RF_BAND_OPM_CHANNEL:
		/* Set RF band, operating mode and channel */
		us_band_opmode  = (puc_rx_msg[1] << 8) | puc_rx_msg[2];
		us_channel  = (puc_rx_msg[3] << 8) | puc_rx_msg[4];
		at86rf_pib_set(AT86RF_TRX_RF09_ID, AT86RF_PIB_PHY_BAND_OPERATING_MODE, &us_band_opmode);
		at86rf_pib_set(AT86RF_TRX_RF09_ID, AT86RF_PIB_PHY_CHANNEL_NUM, &us_channel);
		break;

	case SNIFFER_IF_PHY_G3_SET_TONE_MASK_PLC:
	default:
		/* Not supported */
		break;
	}
}

#endif

#ifdef __cplusplus
}
#endif
