/**
 *
 * \file
 *
 * \brief G3 RF Phy Abstraction Layer for RF215
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

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "pal_rf.h"
#include "conf_prf_if.h"
#include "prf_if.h"
#include "timer_1us.h"
#include "at86rf.h"
#include "conf_project.h"
#include "conf_usi.h"
#include "ieee_15_4_sun_fsk.h"
#include "ieee_15_4_sun_ofdm.h"

#ifdef ENABLE_SNIFFER
#define CONF_PHY_SNIFFER_MODE
#endif

#ifdef CONF_PHY_SNIFFER_MODE
#include "usi.h"
#endif

#ifdef PAL_RF_DEBUG_ENABLE
# define LOG_PAL_DEBUG(...)         printf(__VA_ARGS__ )
#else
# define LOG_PAL_DEBUG(...)
#endif

/* Default Band, Operation Mode and Channel */
#define CONF_INIT_BAND_OPMODE   AT86RF_SUN_FSK_BAND_863_OPM1
#define CONF_INIT_CHANNEL_NUM   29

/* PAL callbacks */
static struct TPalRfNotifications sx_pal_rf_notifications = {NULL};
static uint32_t sul_exception_reset;
static uint32_t sul_exception_spi;
static uint32_t sul_exception_init_err;
static uint32_t sul_exception_tot;

/* PIB managed by pal_rf */
static at86rf_fsk_fec_t suc_tx_fsk_fec;
static at86rf_ofdm_mcs_t suc_tx_ofdm_mcs;

/* Flag to control whether RF transceiver is available */
static bool sb_rf_trx_available;

/**
 * \brief AT86RF TX confirm callback
 *
 * \param uc_trx_id TRX identifier
 * \param px_tx_cfm Pointer to TX confirm
 */
static void _at86rf_tx_cfm_cb(uint8_t uc_trx_id, at86rf_tx_cfm_t *px_tx_cfm)
{
	enum EPalRFStatus eStatus;
	at86rf_tx_cfm_res_t uc_tx_res;
	uint32_t ul_tx_time_ini, ul_tx_time_end;

	/* Time of TX frame end */
	ul_tx_time_ini = px_tx_cfm->ul_tx_time_ini;
	ul_tx_time_end = px_tx_cfm->ul_tx_time_ini + px_tx_cfm->ul_frame_duration;

	/* Translate TX reusult to enum EPalRFStatus */
	uc_tx_res = px_tx_cfm->uc_tx_res;
	switch (uc_tx_res) {
	case AT86RF_TX_SUCCESS:
		eStatus = PAL_RF_SUCCESS;
		break;

	case AT86RF_TX_BUSY_RX:
	case AT86RF_TX_BUSY_CHN:
	case AT86RF_TX_CANCEL_BY_RX:
		eStatus = PAL_RF_CHANNEL_ACCESS_FAILURE;
		break;

	case AT86RF_TX_BUSY_TX:
	case AT86RF_TX_FULL_BUFFERS:
	case AT86RF_TX_TRX_SLEPT:
		eStatus = PAL_RF_BUSY_TX;
		break;

	case AT86RF_TX_INVALID_LEN:
	case AT86RF_TX_INVALID_TRX_ID:
	case AT86RF_TX_INVALID_PARAM:
		eStatus = PAL_RF_INVALID_PARAM;
		break;

	case AT86RF_TX_ERROR_UNDERRUN:
	case AT86RF_TX_TIMEOUT:
		eStatus = PAL_RF_TIMEOUT;
		break;

	case AT86RF_TX_CANCELLED:
		eStatus = PAL_RF_TX_CANCELLED;
		break;

	case AT86RF_TX_ABORTED:
	default:
		eStatus = PAL_RF_ERROR;
		break;
	}

	LOG_PAL_DEBUG("PalRfDataConfirm (%hhu). TimeIni %u TimeEnd %u\r\n", uc_tx_res, ul_tx_time_ini, ul_tx_time_end);

	if (sx_pal_rf_notifications.m_pPalRFDataConfirm) {
		/* Notify data confirm */
		sx_pal_rf_notifications.m_pPalRFDataConfirm(eStatus, ul_tx_time_ini, ul_tx_time_end);
	}

	UNUSED(uc_trx_id);
}

/**
 * \brief AT86RF RX indication callback
 *
 * \param uc_trx_id TRX identifier
 * \param px_rx_ind Pointer to RX indication
 */
static void _at86rf_rx_ind_cb(uint8_t uc_trx_id, at86rf_rx_ind_t *px_rx_ind)
{
	struct TPalRfRxParams x_rf_rx_params;
	uint32_t ul_rx_time_ini;
	uint32_t ul_rx_time_end;
	uint16_t us_psdu_len;
	int8_t sc_rssi_dBm;
	bool b_fcs_ok;

	if (!sb_rf_trx_available) {
		LOG_PAL_DEBUG("No transceiver available. Indication not sent to upper layer\r\n");
		/* Return without calling upper layer */
		return;
	}

	/* FCS computed in RF PHY layer */
	b_fcs_ok = px_rx_ind->b_fcs_ok;
	x_rf_rx_params.m_bFCSOK = b_fcs_ok;

	/* Time of RX frame start */
	ul_rx_time_ini = px_rx_ind->ul_rx_time_ini;
	x_rf_rx_params.m_u32TimeIni = ul_rx_time_ini;

	/* Time of RX frame end */
	ul_rx_time_end = ul_rx_time_ini + px_rx_ind->ul_frame_duration;
	x_rf_rx_params.m_u32TimeEnd = ul_rx_time_end;

	/* RSSI in dBm */
	sc_rssi_dBm = px_rx_ind->sc_rssi_dBm;
	x_rf_rx_params.m_s8RSSI = sc_rssi_dBm;

	us_psdu_len = px_rx_ind->us_psdu_len;
	LOG_PAL_DEBUG("PalRfDataIndication. %hu bytes. Time %u. FCSOK %hhu. RSSI %hhd\r\n", us_psdu_len, ul_rx_time_end, b_fcs_ok, sc_rssi_dBm);

	if (sx_pal_rf_notifications.m_pPalRFDataIndication) {
		/* Notify data indication */
		sx_pal_rf_notifications.m_pPalRFDataIndication(px_rx_ind->puc_data, us_psdu_len, &x_rf_rx_params);
	}

	UNUSED(uc_trx_id);
}

/**
 * \brief AT86RF exception callback
 *
 * \param uc_exception_mask Exception type mask
 */
static void _at86rf_exception_cb(uint8_t uc_exception_mask)
{
	sul_exception_tot++;
	if (uc_exception_mask & AT86RF_EXCEPTION_RESET) {
		sul_exception_reset++;
	}

	if (uc_exception_mask & AT86RF_EXCEPTION_SPI_ERR) {
		sul_exception_spi++;
	}

	if (uc_exception_mask & AT86RF_EXCEPTION_INIT_ERR) {
		sul_exception_init_err++;
		sb_rf_trx_available = false;
	}

	LOG_PAL_DEBUG("AT86F Exception: 0x%02x\r\n", uc_exception_mask);
}

#ifdef CONF_PHY_SNIFFER_MODE

/**
 * \brief Handler to receive data from RF215.
 */
static void _handler_serial_at86rf_event(uint8_t *puc_serial_data, uint16_t us_len)
{
	x_usi_serial_cmd_params_t x_usi_msg;

	x_usi_msg.uc_protocol_type = PROTOCOL_SNIF_G3;
	x_usi_msg.ptr_buf = puc_serial_data;
	x_usi_msg.us_len = us_len;
	usi_send_cmd(&x_usi_msg);
}

#endif

/**
 * \brief Conver get/set result from RF215 PHY layer to pal_rf result.
 *
 * \param uc_result RF215 result
 *
 * \return pal_rf result
 */
static enum EPalRfGetSetResult _convert_get_set_result(at86rf_res_t uc_result)
{
	switch (uc_result) {
	case AT86RF_SUCCESS:
		return PAL_RF_GETSET_RESULT_OK;

	case AT86RF_READ_ONLY:
		return PAL_RF_GETSET_RESULT_READ_ONLY;

	case AT86RF_WRITE_ONLY:
		return PAL_RF_GETSET_RESULT_WRITE_ONLY;

	case AT86RF_INVALID_PARAM:
	case AT86RF_INVALID_CFG:
	case AT86RF_INVALID_ATTR:
		return PAL_RF_GETSET_RESULT_INVALID_PARAM;

	default:
		return PAL_RF_GETSET_RESULT_ERROR;
	}
}

/**
 * \brief RF PHY Initialization.
 *
 * \param pNotifications Pointer to upper layer notifications
 */
bool PalRfInitialize(struct TPalRfNotifications *pNotifications)
{
	at86rf_hal_wrapper_t x_at86rf_hal_wrp;
	at86rf_callbacks_t x_at86rf_callbacks;
	at86rf_phy_ini_params_t x_phy_ini_params_09;
	at86rf_res_t uc_init_res;

	/* Initialize exception counters */
	sul_exception_reset = 0;
	sul_exception_spi = 0;
	sul_exception_init_err = 0;
	sul_exception_tot = 0;

	/* Initialize Timer of 1us service */
	timer_1us_init();

	/* Fill AT86RF HAL wrapper */
	x_at86rf_hal_wrp.rf_init = prf_if_init;
	x_at86rf_hal_wrp.rf_reset = prf_if_reset;
	x_at86rf_hal_wrp.rf_enable_int = prf_if_enable_interrupt;
	x_at86rf_hal_wrp.rf_set_handler = prf_if_set_handler;
	x_at86rf_hal_wrp.rf_send_spi_cmd = prf_if_send_spi_cmd;
	x_at86rf_hal_wrp.rf_is_spi_busy = prf_if_is_spi_busy;
	x_at86rf_hal_wrp.rf_led = prf_if_led;
	x_at86rf_hal_wrp.timer_get = timer_1us_get;
	x_at86rf_hal_wrp.timer_enable_int = timer_1us_enable_interrupt;
	x_at86rf_hal_wrp.timer_set_int = timer_1us_set_int;
	x_at86rf_hal_wrp.timer_cancel_int = timer_1us_cancel_int;

	/* Fill AT86RF callbacks */
	x_at86rf_callbacks.rf_exception_cb = _at86rf_exception_cb;
	x_at86rf_callbacks.rf_tx_cfm_cb = _at86rf_tx_cfm_cb;
	x_at86rf_callbacks.rf_rx_ind_cb = _at86rf_rx_ind_cb;
	x_at86rf_callbacks.rf_chn_switch_cb = NULL;
#ifdef AT86RF_ADDONS_ENABLE
#ifdef CONF_PHY_SNIFFER_MODE
	x_at86rf_callbacks.rf_addon_event_cb = _handler_serial_at86rf_event;
#else
	x_at86rf_callbacks.rf_addon_event_cb = NULL;
#endif
#endif

	/* Initialize AT86RF component */
	at86rf_init(&x_at86rf_hal_wrp, &x_at86rf_callbacks);

	/* Initial PHY configuration (RF09, Sub-1GHz) */
	x_phy_ini_params_09.us_band_opm = CONF_INIT_BAND_OPMODE;
	/* Set freq to 866 MHz (channel #29). Freq = Freq_0 + ChSpa*ChNum = 863.1 + 0.1*29 = 866 */
	x_phy_ini_params_09.us_chn_num_ini = CONF_INIT_CHANNEL_NUM;
	/* x_phy_ini_params_09.us_band_opm = AT86RF_SUN_OFDM_BAND_863_OPT4; */

	/* Enable AT86RF component */
	uc_init_res = at86rf_enable(&x_phy_ini_params_09, NULL);

	if (uc_init_res == AT86RF_SUCCESS) {
		LOG_PAL_DEBUG("PalRfInitialize OK\r\n");
		sb_rf_trx_available = true;
	} else {
		LOG_PAL_DEBUG("PalRfInitialize ERROR (%hhu)\r\n", uc_init_res);
		sb_rf_trx_available = false;
	}

	/* Save upper layer notification callbacks */
	sx_pal_rf_notifications = *pNotifications;

	/* Initialize PIB managed by pal_rf */
	suc_tx_fsk_fec = AT86RF_FSK_FEC_OFF;
	suc_tx_ofdm_mcs = AT86RF_OFDM_MCS_0;

	return sb_rf_trx_available;
}

/**
 * \brief Check RF PHY pending events.
 */
void PalRfEventHandler(void)
{
	if (sb_rf_trx_available) {
		at86rf_event_handler();
	}
}

/**
 * \brief RF TX request
 *
 * \param pPsdu Pointer to data to be transmitted
 * \param u16PsduLength PSDU length in bytes (including FCS)
 * \param pParameters Pointer to RF TX parameters
 */
void PalRfTxRequest(uint8_t *pPsdu, uint16_t u16PsduLength, struct TPalRfTxParams *pParameters)
{
	at86rf_tx_params_t x_tx_params;
	at86rf_phy_cfg_t x_phy_cfg;
	uint32_t ul_tx_time;

	if (!sb_rf_trx_available) {
		LOG_PAL_DEBUG("No transceiver available. Tx not requested\r\n");
		/* Generate confirm with Channel Access Failure */
		if (sx_pal_rf_notifications.m_pPalRFDataConfirm) {
			/* Notify data confirm with Invalid Parameter result */
			sx_pal_rf_notifications.m_pPalRFDataConfirm(PAL_RF_TRX_OFF, pParameters->m_u32Time, pParameters->m_u32Time);
		}

		/* Return without tx attempt */
		return;
	}

	/* TX parameters from upper layer */
	x_tx_params.puc_data = pPsdu;
	x_tx_params.us_psdu_len = u16PsduLength;
	ul_tx_time = pParameters->m_u32Time;
	x_tx_params.ul_tx_time = ul_tx_time;
	x_tx_params.uc_txpwr_att = pParameters->m_u8TxPowerAtt;
	x_tx_params.uc_cw = 1;
	if (pParameters->m_bCSMA) {
		/* CSMA used: Energy above threshold and carrier sense CCA Mode.
		 * Programmed TX canceled once RX frame detected */
		x_tx_params.uc_cca_mode = AT86RF_CCA_MODE_3;
		x_tx_params.b_cancel_by_rx = true;
	} else {
		/* CSMA not used */
		x_tx_params.uc_cca_mode = AT86RF_CCA_OFF;
		x_tx_params.b_cancel_by_rx = false;
	}

	/* Abusoulute time mode */
	x_tx_params.uc_time_mode = AT86RF_TX_TIME_ABS;

	/* TX identifier not used (single TX buffer) */
	x_tx_params.uc_tx_id = 0;

	/* Same channel for TX and RX */
	x_tx_params.us_chn_num = 0xFFFF;

	/* Get PHY configuration */
	x_phy_cfg = SUN_FSK_BAND_863_870_OPM1; /* Set a default value just in case */
	at86rf_pib_get(AT86RF_TRX_RF09_ID, AT86RF_PIB_PHY_CONFIG, &x_phy_cfg);

	/* RF Modulation parameters (FSK: FEC; OFDM: MCS) used for transmission */
	if (x_phy_cfg.uc_phy_mod == AT86RF_PHY_MOD_FSK) {
		x_tx_params.x_mod_params.x_fsk.uc_fec_enabled = suc_tx_fsk_fec;
	} else if (x_phy_cfg.uc_phy_mod == AT86RF_PHY_MOD_OFDM) {
		/* OFDM: MCS most robust by default (depending on option) */
		switch (x_phy_cfg.u_mod_cfg.x_ofdm.uc_opt) {
		case AT86RF_OFDM_OPT_4:
			if (suc_tx_ofdm_mcs < AT86RF_OFDM_MCS_2) {
				x_tx_params.x_mod_params.x_ofdm.uc_mcs = AT86RF_OFDM_MCS_2;
			} else {
				x_tx_params.x_mod_params.x_ofdm.uc_mcs = suc_tx_ofdm_mcs;
			}

			break;

		case AT86RF_OFDM_OPT_3:
			if (suc_tx_ofdm_mcs < AT86RF_OFDM_MCS_1) {
				x_tx_params.x_mod_params.x_ofdm.uc_mcs = AT86RF_OFDM_MCS_1;
			} else {
				x_tx_params.x_mod_params.x_ofdm.uc_mcs = suc_tx_ofdm_mcs;
			}

			break;

		case AT86RF_OFDM_OPT_2:
		case AT86RF_OFDM_OPT_1:
		default:
			x_tx_params.x_mod_params.x_ofdm.uc_mcs = suc_tx_ofdm_mcs;
			break;
		}
	}

	LOG_PAL_DEBUG("PalRfTxRequest. %hu bytes. Time %u\r\n", u16PsduLength, ul_tx_time);

	/* Send TX request to RF PHY */
	at86rf_tx_req(AT86RF_TRX_RF09_ID, &x_tx_params);
}

/**
 * \brief RF Current TX Cancel
 */
void PalRfTxCancel(void)
{
	at86rf_tx_params_t x_tx_params;

	if (!sb_rf_trx_available) {
		LOG_PAL_DEBUG("No transceiver available. Cancel not requested\r\n");
		/* Return without cancel */
		return;
	}

	/* Abusoulute time mode */
	x_tx_params.uc_time_mode = AT86RF_TX_CANCEL;
	x_tx_params.ul_tx_time = 0;

	/* TX identifier not used (single TX buffer) */
	x_tx_params.uc_tx_id = 0;

	LOG_PAL_DEBUG("PalRfTxCancel\r\n");

	/* Send TX request to RF PHY */
	at86rf_tx_req(AT86RF_TRX_RF09_ID, &x_tx_params);
}

/**
 * \brief RF PHY reset.
 */
void PalRfResetRequest(void)
{
	uint8_t uc_dummy = 1;
	at86rf_phy_ini_params_t x_phy_ini_params_09;
	at86rf_res_t uc_init_res;

	if (!sb_rf_trx_available) {
		/* Transceiver was not available upon initialization. Try to initialize now with default parameters */
		x_phy_ini_params_09.us_band_opm = CONF_INIT_BAND_OPMODE;
		x_phy_ini_params_09.us_chn_num_ini = CONF_INIT_CHANNEL_NUM;
		/* Enable AT86RF component */
		uc_init_res = at86rf_enable(&x_phy_ini_params_09, NULL);
		/* Set flag depending on result */
		if (uc_init_res == AT86RF_SUCCESS) {
			LOG_PAL_DEBUG("RfInitialize OK on reset\r\n");
			sb_rf_trx_available = true;
		} else {
			LOG_PAL_DEBUG("RfInitialize ERROR (%hhu) on reset\r\n", uc_init_res);
			sb_rf_trx_available = false;
		}
	} else {
		/* Perform regular reset */
		at86rf_pib_set(AT86RF_TRX_RF09_ID, AT86RF_PIB_DEVICE_RESET, &uc_dummy);
		at86rf_pib_set(AT86RF_TRX_RF09_ID, AT86RF_PIB_PHY_STATS_RESET, &uc_dummy);
	}
}

/**
 * \brief Get current RF PHY time in us.
 *
 * \return Current time in us, referred to system time (32-bit counter of 1us)
 */
uint32_t PalRfGetPhyTime(void)
{
	return timer_1us_get();
}

/**
 * \brief Get RF PHY parameter.
 *
 * \param us_id Parameter identifier (see 'enum EPalRfPhyParam' and 'at86rf_pib_attr_t')
 * \param p_val Pointer to store parameter value
 *
 * \return Get result
 */
enum EPalRfGetSetResult PalRfGetParam(uint16_t us_id, void *p_val)
{
	at86rf_res_t uc_result;

	if (!sb_rf_trx_available) {
		LOG_PAL_DEBUG("No transceiver available. Get not invoked\r\n");
		/* Return invalid parameter */
		return PAL_RF_GETSET_RESULT_INVALID_PARAM;
	}

	switch (us_id) {
	case PAL_RF_PHY_PARAM_TX_FSK_FEC:
		*((at86rf_fsk_fec_t *)p_val) = suc_tx_fsk_fec;
		uc_result = AT86RF_SUCCESS;
		break;

	case PAL_RF_PHY_PARAM_TX_OFDM_MCS:
		*((at86rf_ofdm_mcs_t *)p_val) = suc_tx_ofdm_mcs;
		uc_result = AT86RF_SUCCESS;
		break;

	default:
		uc_result = at86rf_pib_get(AT86RF_TRX_RF09_ID, (at86rf_pib_attr_t)us_id, p_val);
		break;
	}

	return _convert_get_set_result(uc_result);
}

/**
 * \brief Set RF PHY parameter.
 *
 * \param us_id Parameter identifier (see 'enum EPalRfPhyParam')
 * \param p_val Pointer to parameter value to set
 *
 * \return Set result
 */
enum EPalRfGetSetResult PalRfSetParam(uint16_t us_id, void *p_val)
{
	at86rf_res_t uc_result;

	if (!sb_rf_trx_available) {
		LOG_PAL_DEBUG("No transceiver available. Set not invoked\r\n");
		/* Return invalid parameter */
		return PAL_RF_GETSET_RESULT_INVALID_PARAM;
	}

	switch (us_id) {
	case PAL_RF_PHY_PARAM_TX_FSK_FEC:
	{
		at86rf_fsk_fec_t uc_tx_fsk_fec_new = *((at86rf_fsk_fec_t *)p_val);
		if (uc_tx_fsk_fec_new <= AT86RF_FSK_FEC_ON) {
			suc_tx_fsk_fec = uc_tx_fsk_fec_new;
			uc_result = AT86RF_SUCCESS;
		} else {
			uc_result = AT86RF_INVALID_PARAM;
		}

		break;
	}

	case PAL_RF_PHY_PARAM_TX_OFDM_MCS:
	{
		at86rf_ofdm_mcs_t uc_tx_ofdm_mcs_new = *((at86rf_ofdm_mcs_t *)p_val);
		if (uc_tx_ofdm_mcs_new <= AT86RF_OFDM_MCS_6) {
			suc_tx_ofdm_mcs = uc_tx_ofdm_mcs_new;
			uc_result = AT86RF_SUCCESS;
		} else {
			uc_result = AT86RF_INVALID_PARAM;
		}

		break;
	}

	default:
		uc_result = at86rf_pib_set(AT86RF_TRX_RF09_ID, (at86rf_pib_attr_t)us_id, p_val);
		break;
	}

	return _convert_get_set_result(uc_result);
}

/**
 * \brief Get RF PHY parameter length.
 *
 * \param us_id Parameter identifier (see 'enum EPalRfPhyParam')
 *
 * \return Parameter length in bytes
 */
uint8_t PalRfGetParamLen(uint16_t us_id)
{
	uint8_t uc_len;
	switch (us_id) {
	case PAL_RF_PHY_PARAM_TX_FSK_FEC:
	case PAL_RF_PHY_PARAM_TX_OFDM_MCS:
		uc_len = 1;
		break;

	default:
		uc_len = at86rf_pib_get_len((at86rf_pib_attr_t)us_id);
		break;
	}

	return uc_len;
}
