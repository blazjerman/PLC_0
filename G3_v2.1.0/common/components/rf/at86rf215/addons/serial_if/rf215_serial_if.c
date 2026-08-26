/**
 *
 * \file
 *
 * \brief RF215 serial (PHY Tester) addon.
 *
 * Copyright (c) 2021 Microchip Technology Inc. and its subsidiaries.
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

/* System includes */
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Serial interface commands identifiers */
#define SERIAL_IF_PHY_COMMAND_GET_CFG          0  /* Get configuration parameter query */
#define SERIAL_IF_PHY_COMMAND_GET_CFG_RSP      1  /* Get configuration parameter response */
#define SERIAL_IF_PHY_COMMAND_SET_CFG          2  /* Set configuration parameter query */
#define SERIAL_IF_PHY_COMMAND_SET_CFG_RSP      3  /* Set configuration parameter response */
#define SERIAL_IF_PHY_COMMAND_SEND_MSG         6  /* Send message data */
#define SERIAL_IF_PHY_COMMAND_SEND_MSG_RSP     7  /* Send message data response */
#define SERIAL_IF_PHY_COMMAND_RECEIVE_MSG      8  /* Receive message data */

/* Serial interface RF TRX identifiers (Sub-1GHz, 2.4GHz) */
#define SERIAL_IF_TRX_RF09_ID                  0
#define SERIAL_IF_TRX_RF24_ID                  1
#define SERIAL_IF_TRX_ERROR_ID                 0xFF

/* Serial message max length (RX indication), including data and parameters */
#define RF215_SERIAL_RX_MSG_HEADER_LEN         (sizeof(at86rf_tx_params_t) - 4)
#define RF215_SERIAL_RX_MSG_MAX_LEN            (AT86RF215_MAX_PSDU_LEN + RF215_SERIAL_RX_MSG_HEADER_LEN)

/* Configuration parameter max length */
#define RF215_SERIAL_GET_CFG_MAX_LEN           (sizeof(at86rf_phy_cfg_t))

#ifdef AT86RF_ADDONS_ENABLE

/** RF215 internal global variables declared as extern */
extern at86rf_callbacks_t gx_rf215_callbacks;

/* Serial buffer to write commands from addon to upper layer */
static uint8_t spuc_serial_buff[RF215_SERIAL_RX_MSG_MAX_LEN];

/* Buffer to read configuration parameters */
static uint8_t spuc_get_cfg_buff[RF215_SERIAL_GET_CFG_MAX_LEN];

/**
 * \internal
 * \brief Memcopy with byte order reversal.
 *
 * Copies puc_buf[] into puc_dst[], re-ordering the bytes to adapt to the serial
 * communication.
 *
 * \param puc_dst    Pointer to buffer where the data will be copied
 * \param puc_src    Pointer to buffer source data
 * \param us_len     Length of data to copy
 */
static void _memcpy_rev(uint8_t *puc_dst, uint8_t *puc_src, uint16_t us_len)
{
	uint8_t *ptr_uc_mem_dst, *ptr_uc_mem_src;
	uint16_t us_idx;

	if (us_len <= 4) {
		ptr_uc_mem_dst = puc_dst + us_len - 1;
		ptr_uc_mem_src = puc_src;
		for (us_idx = 0; us_idx < us_len; us_idx++) {
			*ptr_uc_mem_dst-- = (uint8_t)*ptr_uc_mem_src++;
		}
	} else {
		memcpy(puc_dst, puc_src, us_len);
	}
}

/**
 * \brief Convert TRX identifier from PHY format to serial format
 *
 * \param uc_trx_id_phy  TRX identifier in PHY format
 *
 * \return TRX idendifier (SERIAL_IF_TRX_RF09_ID, SERIAL_IF_TRX_RF24_ID)
 */
static uint8_t _rf215_trx_id_phy_to_serial(uint8_t uc_trx_id_phy)
{
#ifdef AT86RF_TRX_RF09_ID
	if (uc_trx_id_phy == AT86RF_TRX_RF09_ID) {
		return SERIAL_IF_TRX_RF09_ID;
	}
#endif

#ifdef AT86RF_TRX_RF24_ID
	if (uc_trx_id_phy == AT86RF_TRX_RF24_ID) {
		return SERIAL_IF_TRX_RF24_ID;
	}
#endif

	return uc_trx_id_phy;
}

/**
 * \brief Convert TRX identifier from serial format to PHY format
 *
 * \param uc_trx_id_serial  TRX identifier in serial format
 *
 * \return TRX idendifier (AT86RF_TRX_RF09_ID, AT86RF_TRX_RF24_ID)
 */
static uint8_t _rf215_trx_id_serial_to_phy(uint8_t uc_trx_id_serial)
{
	uint8_t uc_trx_id_phy = uc_trx_id_serial;

	if (uc_trx_id_serial == SERIAL_IF_TRX_RF09_ID) {
#ifdef AT86RF_TRX_RF09_ID
		uc_trx_id_phy = AT86RF_TRX_RF09_ID;
#endif
	} else if (uc_trx_id_serial == SERIAL_IF_TRX_RF24_ID) {
#ifdef AT86RF_TRX_RF24_ID
		uc_trx_id_phy = AT86RF_TRX_RF24_ID;
#endif
	}

	return uc_trx_id_phy;
}

/**
 * \brief Converts byte buffer from Phy Tester Tool (SEND_MSG command) to PHY TX
 * request struct
 *
 * \param[in] puc_serial_data Pointer to serial data buffer
 * \param[in] px_tx_params Pointer to TX request struct
 *
 * \return TRX identifier
 */
static uint8_t _rf215_addon_parse_tx_req(uint8_t *puc_serial_data, at86rf_tx_params_t *px_tx_params)
{
	uint8_t *puc_src_buf;
	uint32_t ul_aux;
	uint16_t us_len;
	uint8_t uc_trx_id;

	/* Pointer to source buffer */
	puc_src_buf = puc_serial_data;

	/* Parse TRX identifier (Sub-1GHz, 2.4GHz) */
	uc_trx_id = _rf215_trx_id_serial_to_phy(*puc_src_buf++);

	/* Parse TX parameters */
	ul_aux = ((uint32_t)*puc_src_buf++) << 24;
	ul_aux += ((uint32_t)*puc_src_buf++) << 16;
	ul_aux += ((uint32_t)*puc_src_buf++) << 8;
	ul_aux += (uint32_t)*puc_src_buf++;
	px_tx_params->ul_tx_time = ul_aux;
	us_len = ((uint16_t)*puc_src_buf++) << 8;
	us_len += (uint16_t)*puc_src_buf++;
	px_tx_params->us_psdu_len = us_len;
	px_tx_params->x_mod_params = *((at86rf_mod_frame_params_t *)puc_src_buf++);
	px_tx_params->uc_cca_mode = (at86rf_cca_t)*puc_src_buf++;
	px_tx_params->uc_time_mode = (at86rf_tx_time_mode_t)*puc_src_buf++;
	px_tx_params->b_cancel_by_rx = (bool)*puc_src_buf++;
	px_tx_params->uc_txpwr_att = *puc_src_buf++;
	px_tx_params->uc_tx_id = *puc_src_buf++;
	px_tx_params->uc_cw = *puc_src_buf++;

	/* Same channel for TX and RX */
	px_tx_params->us_chn_num = 0xFFFF;

	/* Pointer to TX data */
	px_tx_params->puc_data = puc_src_buf;

	return uc_trx_id;
}

/**
 * \brief Converts byte buffer from Phy Tester Tool (GET_CFG/SET_CFG command) to
 * PHY TRX and PIB identifiers
 *
 * \param[in] puc_serial_data Pointer to serial data buffer
 * \param[out] pus_pib_id Pointer to PIB identifier
 * \param[out] puc_pib_len Pointer to PIB length
 * \param[out] puc_trx_id Pointer to TRX identifier
 *
 * \return Pointer to PIB value to set (only for SET_CFG)
 */
static uint8_t *_rf215_addon_parse_cfg_cmd(uint8_t *puc_serial_data, at86rf_pib_attr_t *pus_pib_id, uint8_t *puc_pib_len, uint8_t *puc_trx_id)
{
	uint8_t *puc_src_buf;
	uint16_t us_aux;

	/* Pointer to source buffer */
	puc_src_buf = puc_serial_data;

	/* Parse TRX identifier (Sub-1GHz, 2.4GHz) */
	*puc_trx_id = _rf215_trx_id_serial_to_phy(*puc_src_buf++);

	/* Parse PIB identifier and length */
	us_aux = ((uint16_t)*puc_src_buf++) << 8;
	us_aux += (uint16_t)*puc_src_buf++;
	*pus_pib_id = (at86rf_pib_attr_t)us_aux;
	*puc_pib_len = *puc_src_buf++;

	/* Return pointer to PIB value to set (only for SET_CFG) */
	return puc_src_buf;
}

/**
 * \brief Build GET_CFG_RSP command for Phy Tester Tool
 *
 * \param[in] uc_trx_id TRX identifier
 * \param[in] us_pib_id PIB identifier
 * \param[in] uc_pib_len PIB length
 * \param[in] uc_pib_res PIB set result
 *
 * \return Length of serial data buffer
 */
static uint16_t _rf215_addon_stringify_get_cfg_rsp(uint8_t uc_trx_id, at86rf_pib_attr_t us_pib_id, uint8_t uc_pib_len, at86rf_res_t uc_pib_res)
{
	uint8_t *puc_dst_buf;

	/* Pointer to destination buffer */
	puc_dst_buf = spuc_serial_buff;

	/* Command type */
	*puc_dst_buf++ = (uint8_t)SERIAL_IF_PHY_COMMAND_GET_CFG_RSP;

	/* TRX identifier (Sub-1GHz, 2.4GHz) */
	*puc_dst_buf++ = _rf215_trx_id_phy_to_serial(uc_trx_id);

	/* PIB identifier, length and result */
	*puc_dst_buf++ = (uint8_t)(us_pib_id >> 8);
	*puc_dst_buf++ = (uint8_t)(us_pib_id);
	*puc_dst_buf++ = uc_pib_len;
	*puc_dst_buf++ = (uint8_t)(uc_pib_res);

	/* PIB data */
	_memcpy_rev(puc_dst_buf, spuc_get_cfg_buff, uc_pib_len);

	/* Return pointer to buffer and length */
	return uc_pib_len + (uint16_t)(puc_dst_buf - spuc_serial_buff);
}

/**
 * \brief Build SET_CFG_RSP command for Phy Tester Tool
 *
 * \param[in] uc_trx_id TRX identifier
 * \param[in] us_pib_id PIB identifier
 * \param[in] uc_pib_len PIB length
 * \param[in] uc_pib_res PIB set result
 *
 * \return Length of serial data buffer
 */
static uint16_t _rf215_addon_stringify_set_cfg_rsp(uint8_t uc_trx_id, at86rf_pib_attr_t us_pib_id, uint8_t uc_pib_len, at86rf_res_t uc_pib_res)
{
	uint8_t *puc_dst_buf;

	/* Pointer to destination buffer */
	puc_dst_buf = spuc_serial_buff;

	/* Command type */
	*puc_dst_buf++ = (uint8_t)SERIAL_IF_PHY_COMMAND_SET_CFG_RSP;

	/* TRX identifier (Sub-1GHz, 2.4GHz) */
	*puc_dst_buf++ = _rf215_trx_id_phy_to_serial(uc_trx_id);

	/* PIB identifier, length and result */
	*puc_dst_buf++ = (uint8_t)(us_pib_id >> 8);
	*puc_dst_buf++ = (uint8_t)(us_pib_id);
	*puc_dst_buf++ = uc_pib_len;
	*puc_dst_buf++ = (uint8_t)(uc_pib_res);

	/* Return buffer length */
	return (uint16_t)(puc_dst_buf - spuc_serial_buff);
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
	uint8_t *puc_serial_if_data;
	uint8_t uc_serial_if_cmd;

	/* Protection for invalid length */
	if (!us_len) {
		return;
	}

	/* Process received serial message */
	uc_serial_if_cmd  = puc_rx_msg[0];
	puc_serial_if_data = &puc_rx_msg[1];

	switch (uc_serial_if_cmd) {
	case SERIAL_IF_PHY_COMMAND_GET_CFG:
	{
		/* GET command */
		at86rf_pib_attr_t us_id;
		at86rf_res_t uc_pib_res;
		uint16_t us_addon_len;
		uint8_t uc_id_len_serial;
		uint8_t uc_trx_id;
		uint8_t uc_id_len_phy;

		/* Parse GET parameters */
		_rf215_addon_parse_cfg_cmd(puc_serial_if_data, &us_id, &uc_id_len_serial, &uc_trx_id);
		uc_id_len_phy = at86rf_pib_get_len(us_id);
		if (uc_id_len_serial >= uc_id_len_phy) {
			/* Get parameter from PHY */
			uc_pib_res = at86rf_pib_get(uc_trx_id, us_id, spuc_get_cfg_buff);
		} else {
			/* Invalid length */
			uc_pib_res = AT86RF_INVALID_PARAM;
		}

		if (gx_rf215_callbacks.rf_addon_event_cb) {
			/* Serialize GET_CFG_RSP */
			us_addon_len = _rf215_addon_stringify_get_cfg_rsp(uc_trx_id, us_id, uc_id_len_phy, uc_pib_res);

			/* Indicate addon event */
			gx_rf215_callbacks.rf_addon_event_cb(spuc_serial_buff, us_addon_len);
		}

		break;
	}

	case SERIAL_IF_PHY_COMMAND_SET_CFG:
	{
		/* SET command */
		void *p_set_val;
		at86rf_pib_attr_t us_id;
		at86rf_res_t uc_pib_res;
		uint16_t us_addon_len;
		uint8_t uc_id_len_serial;
		uint8_t uc_trx_id;
		uint8_t uc_id_len_phy;

		/* Parse SET parameters */
		p_set_val = _rf215_addon_parse_cfg_cmd(puc_serial_if_data, &us_id, &uc_id_len_serial, &uc_trx_id);
		uc_id_len_phy = at86rf_pib_get_len(us_id);
		if (uc_id_len_serial >= uc_id_len_phy) {
			/* Set PHY parameter */
			uc_pib_res = at86rf_pib_set(uc_trx_id, us_id, p_set_val);
		} else {
			/* Invalid length */
			uc_pib_res = AT86RF_INVALID_PARAM;
		}

		if (gx_rf215_callbacks.rf_addon_event_cb) {
			/* Serialize SET_CFG_RSP */
			us_addon_len = _rf215_addon_stringify_set_cfg_rsp(uc_trx_id, us_id, uc_id_len_phy, uc_pib_res);

			/* Indicate addon event */
			gx_rf215_callbacks.rf_addon_event_cb(spuc_serial_buff, us_addon_len);
		}

		break;
	}

	case SERIAL_IF_PHY_COMMAND_SEND_MSG:
	{
		/* TX command */
		at86rf_tx_params_t x_tx_params;
		uint8_t uc_trx_id;

		/* Parse TX parameters */
		uc_trx_id = _rf215_addon_parse_tx_req(puc_serial_if_data, &x_tx_params);

		/* Request TX */
		at86rf_tx_req(uc_trx_id, &x_tx_params);
		break;
	}

	default:
		break;
	}
}

/**
 * \brief Converts PHY RX indication struct to byte buffer for Phy Tester Tool
 *
 * \param[in] uc_trx_id TRX identifier
 * \param[in] px_rx_ind Pointer to RX indication struct
 * \pus_len[out] pus_len Length of serial data buffer
 *
 * \return Pointer to serial data buffer
 */
uint8_t *rf215_addon_stringify_ind(uint8_t uc_trx_id, at86rf_rx_ind_t *px_rx_ind, uint16_t *pus_len)
{
	uint8_t *puc_dst_buf;
	uint32_t ul_aux;
	uint16_t us_len;

	/* Pointer to destination buffer */
	puc_dst_buf = spuc_serial_buff;

	/* Command type */
	*puc_dst_buf++ = (uint8_t)SERIAL_IF_PHY_COMMAND_RECEIVE_MSG;

	/* TRX identifier (Sub-1GHz, 2.4GHz) */
	*puc_dst_buf++ = _rf215_trx_id_phy_to_serial(uc_trx_id);

	/* RX indication parameters */
	ul_aux = px_rx_ind->ul_rx_time_ini;
	*puc_dst_buf++ = (uint8_t)(ul_aux >> 24);
	*puc_dst_buf++ = (uint8_t)(ul_aux >> 16);
	*puc_dst_buf++ = (uint8_t)(ul_aux >> 8);
	*puc_dst_buf++ = (uint8_t)(ul_aux);
	ul_aux = px_rx_ind->ul_frame_duration;
	*puc_dst_buf++ = (uint8_t)(ul_aux >> 24);
	*puc_dst_buf++ = (uint8_t)(ul_aux >> 16);
	*puc_dst_buf++ = (uint8_t)(ul_aux >> 8);
	*puc_dst_buf++ = (uint8_t)(ul_aux);
	us_len = px_rx_ind->us_psdu_len;
	*puc_dst_buf++ = (uint8_t)(us_len >> 8);
	*puc_dst_buf++ = (uint8_t)(us_len);
	*((at86rf_mod_frame_params_t *)puc_dst_buf++) = px_rx_ind->x_mod_params;
	*puc_dst_buf++ = (uint8_t)(px_rx_ind->sc_rssi_dBm);
	*puc_dst_buf++ = (uint8_t)(px_rx_ind->b_fcs_ok);

	/* RX data */
	memcpy(puc_dst_buf, px_rx_ind->puc_data, us_len);

	/* Return pointer to buffer and length */
	*pus_len = us_len + (uint16_t)(puc_dst_buf - spuc_serial_buff);
	return spuc_serial_buff;
}

/**
 * \brief Converts PHY TX confirm struct to byte buffer for Phy Tester Tool
 *
 * \param[in] uc_trx_id TRX identifier
 * \param[in] px_tx_cfm Pointer to TX confirm struct
 * \pus_len[out] pus_len Length of serial data buffer (0 if not successful TX)
 *
 * \return Pointer to serial data buffer (NULL if not successful TX)
 */
uint8_t *rf215_addon_stringify_cfm(uint8_t uc_trx_id, at86rf_tx_cfm_t *px_tx_cfm, uint16_t *pus_len)
{
	uint8_t *puc_dst_buf;
	uint32_t ul_aux;

	/* Pointer to destination buffer */
	puc_dst_buf = spuc_serial_buff;

	/* Command type */
	*puc_dst_buf++ = (uint8_t)SERIAL_IF_PHY_COMMAND_SEND_MSG_RSP;

	/* TRX identifier (Sub-1GHz, 2.4GHz) */
	*puc_dst_buf++ = _rf215_trx_id_phy_to_serial(uc_trx_id);

	/* TX confirm parameters */
	ul_aux = px_tx_cfm->ul_tx_time_ini;
	*puc_dst_buf++ = (uint8_t)(ul_aux >> 24);
	*puc_dst_buf++ = (uint8_t)(ul_aux >> 16);
	*puc_dst_buf++ = (uint8_t)(ul_aux >> 8);
	*puc_dst_buf++ = (uint8_t)(ul_aux);
	ul_aux = px_tx_cfm->ul_frame_duration;
	*puc_dst_buf++ = (uint8_t)(ul_aux >> 24);
	*puc_dst_buf++ = (uint8_t)(ul_aux >> 16);
	*puc_dst_buf++ = (uint8_t)(ul_aux >> 8);
	*puc_dst_buf++ = (uint8_t)(ul_aux);
	*puc_dst_buf++ = px_tx_cfm->uc_tx_id;
	*puc_dst_buf++ = (uint8_t)(px_tx_cfm->uc_tx_res);

	/* Return pointer to buffer and length */
	*pus_len = (uint16_t)(puc_dst_buf - spuc_serial_buff);
	return spuc_serial_buff;
}

/**
 * \brief Converts PHY TX request struct to byte buffer
 *
 * \param[in] uc_trx_id TRX identifier
 * \param[in] px_tx_params Pointer to TX parameters struct
 */
void rf215_addon_stringify_tx(uint8_t uc_trx_id, at86rf_tx_params_t *px_tx_params)
{
	/* Not used in serial addon */
	UNUSED(uc_trx_id);
	UNUSED(px_tx_params);
}

/**
 * \brief Serial addon initialization
 */
void rf215_addon_init(void)
{
}

#endif

#ifdef __cplusplus
}
#endif
