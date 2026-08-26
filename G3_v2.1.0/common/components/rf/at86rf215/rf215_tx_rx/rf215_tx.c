/**
 *
 * \file
 *
 * \brief RF215 TX controller.
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
#include "rf215_tx_rx.h"
#include "rf215_tx_rx_defs.h"
#include "at86rf.h"
#include "rf215_phy_defs.h"
#include "rf215_fe.h"
#include "rf215_trx_ctl.h"
#include "rf215_bbc.h"
#include "rf215_pll.h"
#ifdef AT86RF_ADDONS_ENABLE
# include "rf215_addon.h"
#endif

/* System includes */
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif

/* TX control struct (one per TRX) */
static volatile rf215_tx_ctl_t spx_tx_ctl[AT86RF_NUM_TRX];

/**
 * \brief Check transmission request parameters.
 *
 * \param uc_trx_id TRX identifier
 * \param px_tx_params Pointer to transmission parameters
 */
static inline at86rf_tx_cfm_res_t _tx_check_params(uint8_t uc_trx_id, at86rf_tx_params_t *px_tx_params)
{
	at86rf_tx_cfm_res_t uc_tx_res = AT86RF_TX_SUCCESS;
	uint16_t us_psdu_len = px_tx_params->us_psdu_len;

	if ((us_psdu_len > AT86RF215_MAX_PSDU_LEN) || (us_psdu_len <= AT86RF_FCS_LEN)) {
		/* Error: invalid data length */
		uc_tx_res = AT86RF_TX_INVALID_LEN;
	} else if (px_tx_params->uc_cca_mode > AT86RF_CCA_OFF) {
		/* Error: invalid CCA mode */
		uc_tx_res = AT86RF_TX_INVALID_PARAM;
	} else if (px_tx_params->uc_time_mode > AT86RF_TX_CANCEL) {
		/* Error: invalid time mode */
		uc_tx_res = AT86RF_TX_INVALID_PARAM;
	} else {
		at86rf_phy_cfg_t *px_phy_cfg = &gpx_phy_cfg[uc_trx_id];
		at86rf_phy_mod_t uc_phy_mod = px_phy_cfg->uc_phy_mod;
		if (uc_phy_mod == AT86RF_PHY_MOD_FSK) {
			if (px_tx_params->x_mod_params.x_fsk.uc_fec_enabled > AT86RF_FSK_FEC_ON) {
				/* Error: invalid FSK FEC mode */
				uc_tx_res = AT86RF_TX_INVALID_PARAM;
			}
		} else if (uc_phy_mod == AT86RF_PHY_MOD_OFDM) {
			at86rf_ofdm_mcs_t uc_mcs = px_tx_params->x_mod_params.x_ofdm.uc_mcs;
			if (uc_mcs > AT86RF_OFDM_MCS_6) {
				/* Error: invalid OFDM MCS */
				uc_tx_res = AT86RF_TX_INVALID_PARAM;
			} else {
				at86rf_ofdm_opt_t uc_opt = px_phy_cfg->u_mod_cfg.x_ofdm.uc_opt;
				if (uc_opt == AT86RF_OFDM_OPT_3) {
					/* Minimum MCS for OFDM Option 3 is 1 */
					if (uc_mcs < AT86RF_OFDM_MCS_1) {
						uc_tx_res = AT86RF_TX_INVALID_PARAM;
					}
				} else if (uc_opt == AT86RF_OFDM_OPT_4) {
					/* Minimum MCS for OFDM Option 3 is 2 */
					if (uc_mcs < AT86RF_OFDM_MCS_2) {
						uc_tx_res = AT86RF_TX_INVALID_PARAM;
					}
				}
			}
		}
	}

	return uc_tx_res;
}

/**
 * \brief Compute delay from TX command to actual TX time. For slotted CSMA-CA,
 * more than one CCA needed (with turnaround time delay).
 *
 * \param uc_trx_id TRX identifier
 * \param b_cca_ed true if CCA with ED, false otherwise
 * \param us_ed_duration_us Duration of ED in us
 * \param uc_cw Contention window length (slotted CSMA-CA)
 *
 * \return Delay in us
 */
static inline uint16_t _tx_cmd_delay(uint8_t uc_trx_id, bool b_cca_ed, uint16_t us_ed_duration_us, uint8_t uc_cw)
{
	uint32_t ul_tx_cmd_delay_q5;

	if (uc_cw <= 1) {
		/* No contention window: Next command is TX or CCATX */
		ul_tx_cmd_delay_q5 = (uint32_t)guc_spi_byte_time_us_q5 * 3;
		ul_tx_cmd_delay_q5 += spx_tx_ctl[uc_trx_id].us_bb_delay_us_q5;
		ul_tx_cmd_delay_q5 += spx_tx_ctl[uc_trx_id].us_proc_delay_us_q5;
		ul_tx_cmd_delay_q5 += RF215_TX_TIME_OFFSET_US_Q5;
		if (b_cca_ed) {
			/* Delay with CCATX: ED (RX) -> TXPREP -> TX */
			ul_tx_cmd_delay_q5 += RF215_RX_TX_TIME_US_Q5;
			ul_tx_cmd_delay_q5 += RF215_RX_CCA_ED_TIME_US_Q5;
			ul_tx_cmd_delay_q5 += ((uint32_t)us_ed_duration_us << 5);
		} else {
			/* Delay without CCATX: TXPREP -> TX */
			ul_tx_cmd_delay_q5 += RF215_TXPREP_TX_TIME_US_Q5;
		}
	} else {
		/* Contention window: Next CCA is not the last one before TX */
		ul_tx_cmd_delay_q5 = ((uint32_t)gpx_phy_ctl[uc_trx_id].us_turnaround_time_us << 5) * (uc_cw - 1);
		if (b_cca_ed) {
			ul_tx_cmd_delay_q5 += (uint32_t)guc_spi_byte_time_us_q5 * 3;
			ul_tx_cmd_delay_q5 += RF215_RX_CCA_ED_TIME_US_Q5;
			ul_tx_cmd_delay_q5 += ((uint32_t)us_ed_duration_us << 5) * uc_cw;
		}
	}

	return RF215_TIME_US_Q5_TO_US(ul_tx_cmd_delay_q5);
}

/**
 * \brief FW and SPI delay due to TX parameter configuration (_tx_param_cfg).
 *
 * \return Delay in us
 */
static inline uint16_t _tx_param_cfg_delay(void)
{
	uint16_t us_delay_cfg;
	uint16_t us_delay_cfg_q5;

	/* 16 SPI bytes + 12 us */
	us_delay_cfg_q5 = (uint16_t)guc_spi_byte_time_us_q5 * 16;
	us_delay_cfg = RF215_TIME_US_Q5_TO_US(us_delay_cfg_q5);
	us_delay_cfg += RF215_TX_PROG_CFG_DELAY_US;
	return us_delay_cfg;
}

/**
 * \brief FW and SPI delay due to TX preparation (_tx_txprep) from TRXOFF state.
 *
 * \return Delay in us
 */
static inline uint16_t _tx_trxoff_txprep_delay(void)
{
	/* Margin for high priority IRQ: 75 us
	 * Timer IRQ delay: 8 us
	 * Time check FW delay: 6 us
	 * TRXOFF->TXPREP: 3 SPI bytes + 200 us */
	uint16_t us_delay_txprep_q5 = (uint16_t)guc_spi_byte_time_us_q5 * 3;
	uint16_t us_delay_txprep = RF215_TIME_US_Q5_TO_US(us_delay_txprep_q5);
	us_delay_txprep += RF215_TX_PROG_INT_MARGIN_US;
	us_delay_txprep += RF215_TIMER_INT_DELAY_US;
	us_delay_txprep += RF215_TX_PROG_TCHECK_DELAY_US;
	us_delay_txprep += RF215_TRXOFF_TXPREP_TIME_US;
	return us_delay_txprep;
}

/**
 * \brief FW and SPI delay due to TX preparation (_tx_txprep) from RX state.
 *
 * \param b_cca_ed true if CCA with ED, false otherwise
 * \param uc_cw Contention window length
 *
 * \return Delay in us
 */
static inline uint16_t _tx_rx_txprep_delay(bool b_cca_ed, uint8_t uc_cw)
{
	uint16_t us_delay_txprep_q5;
	uint16_t us_delay_txprep;
	uint8_t uc_spi_bytes;

	if ((!b_cca_ed) && (uc_cw > 1)) {
		/* Delay 0 if contention window (CW > 1) without ED */
		return 0;
	}

	if (b_cca_ed) {
		us_delay_txprep = RF215_TX_PROG_TXPREP_CCA_DELAY_US;
		if (uc_cw <= 1) {
			/* RX->TXPREP (with CCA ED): 32 SPI bytes + 17 us */
			uc_spi_bytes = 32;
		} else {
			/* RX->TXPREP (with CCA ED): 12 SPI bytes + 17 us */
			uc_spi_bytes = 12;
		}
	} else {
		/* RX->TXPREP (without CCA ED): 22 SPI bytes + 12 us */
		us_delay_txprep = RF215_TX_PROG_TXPREP_DELAY_US;
		uc_spi_bytes = 22;
	}

	/* Margin for high priority IRQ: 75 us
	 * Timer IRQ delay: 8 us
	 * Time check FW delay: 6 us */
	us_delay_txprep_q5 = (uint16_t)guc_spi_byte_time_us_q5 * uc_spi_bytes;
	us_delay_txprep += RF215_TX_PROG_INT_MARGIN_US;
	us_delay_txprep += RF215_TIMER_INT_DELAY_US;
	us_delay_txprep += RF215_TX_PROG_TCHECK_DELAY_US;
	us_delay_txprep += RF215_TIME_US_Q5_TO_US(us_delay_txprep_q5);

	return us_delay_txprep;
}

/**
 * \brief FW and SPI delay due to TRXRDY interrupt.
 *
 * \return Delay in us
 */
static inline uint16_t _tx_trxrdy_delay(void)
{
	/* Margin for high priority IRQ: 75 us
	 * TRXRDY IRQ: 6 SPI bytes + 10 us */
	uint16_t us_delay_trxrdy_q5 = (uint16_t)guc_spi_byte_time_us_q5 * 6;
	uint16_t us_delay_trxrdy = RF215_TIME_US_Q5_TO_US(us_delay_trxrdy_q5);
	us_delay_trxrdy += RF215_TX_PROG_INT_MARGIN_US;
	us_delay_trxrdy += RF215_RF_INT_DELAY_US;
	us_delay_trxrdy += RF215_TX_PROG_TCHECK_DELAY_US;
	return us_delay_trxrdy;
}

/**
 * \brief Total delay (FW and SPI) from next command (TX_CONFIG, TXPREP,
 * CCA ED or TX, depending on current and configuration) to TX start time.
 *
 * \param[in] uc_trx_id TRX identifier
 * \param[in] px_tx_params Pointer to TX parameters
 * \param[out] pus_tx_cmd_delay Pointer to TX command delay (including CW)
 *
 * \return Delay in us
 */
static uint16_t _tx_total_delay(uint8_t uc_trx_id, at86rf_tx_params_t *px_tx_params, uint16_t *pus_tx_cmd_delay)
{
	uint16_t us_tx_total_delay;
	uint16_t us_cca_ed_duration;
	uint16_t us_tx_cmd_delay;
	bool b_cca_ed;
	bool b_tx_id_ongoing;
	uint8_t uc_cw;
	at86rf_cca_t uc_cca_mode;
	rf215_tx_ctl_t *px_tx_ctl = (rf215_tx_ctl_t *)&spx_tx_ctl[uc_trx_id];

	/* Check if TX is in progress */
	b_tx_id_ongoing = (bool)(px_tx_ctl->b_tx_on && (px_tx_ctl->uc_tx_id == px_tx_params->uc_tx_id));

	/* CCA with Energy Detection */
	uc_cca_mode = px_tx_params->uc_cca_mode;
	b_cca_ed = (bool)((uc_cca_mode == AT86RF_CCA_MODE_1) || (uc_cca_mode == AT86RF_CCA_MODE_3));
	if (b_tx_id_ongoing) {
		us_cca_ed_duration = px_tx_ctl->x_cca_ed_cfg.us_duration_us;
		uc_cw = px_tx_ctl->uc_cw;
	} else {
		us_cca_ed_duration = gpx_phy_cfg[uc_trx_id].x_cca_ed_cfg.us_duration_us;
		uc_cw = px_tx_params->uc_cw;
	}

	/* Delay from next command to TX start time */
	us_tx_cmd_delay = _tx_cmd_delay(uc_trx_id, b_cca_ed, us_cca_ed_duration, uc_cw);
	*pus_tx_cmd_delay = us_tx_cmd_delay;
	us_tx_total_delay = us_tx_cmd_delay;

	/* Required time before TX command (worst case) */
	if (!b_tx_id_ongoing) {
		/* TX parameter configuration delay */
		us_tx_total_delay += px_tx_ctl->us_tx_param_cfg_delay_us;
	}

	if ((!b_tx_id_ongoing) || (gpx_phy_ctl[uc_trx_id].uc_trx_state == RF215_RFn_STATE_RF_TRXOFF)) {
		/* TRXOFF->TXPREP delay */
		us_tx_total_delay += px_tx_ctl->us_trxoff_txprep_delay_us;
	} else {
		/* RX->TXPREP delay */
		us_tx_total_delay += _tx_rx_txprep_delay(b_cca_ed, uc_cw);
	}

	/* TRXRDY IRQ delay */
	us_tx_total_delay += px_tx_ctl->us_trxrdy_delay_us;

	return us_tx_total_delay;
}

/**
 * \brief Update TX PHY statistics, depending on TX result
 *
 * \param uc_trx_id TRX identifier
 * \param uc_tx_res TX result
 */
static void _tx_upd_phy_stats(uint8_t uc_trx_id, at86rf_tx_cfm_res_t uc_tx_res)
{
	rf215_phy_stats_t *px_phy_stats = &gpx_phy_stats[uc_trx_id];

	if (uc_tx_res != AT86RF_TX_SUCCESS) {
		px_phy_stats->ul_tx_err_total++;
	}

	switch (uc_tx_res) {
	case AT86RF_TX_SUCCESS:
		px_phy_stats->ul_tx_total++;
		px_phy_stats->ul_tx_total_bytes += spx_tx_ctl[uc_trx_id].us_psdu_len;
		break;

	case AT86RF_TX_ABORTED:
	case AT86RF_TX_CANCEL_BY_RX:
	case AT86RF_TX_CANCELLED:
		px_phy_stats->ul_tx_err_aborted++;
		break;

	case AT86RF_TX_BUSY_TX:
	case AT86RF_TX_FULL_BUFFERS:
	case AT86RF_TX_TRX_SLEPT:
		px_phy_stats->ul_tx_err_busy_tx++;
		break;

	case AT86RF_TX_BUSY_RX:
		px_phy_stats->ul_tx_err_busy_rx++;
		break;

	case AT86RF_TX_BUSY_CHN:
		px_phy_stats->ul_tx_err_busy_chn++;
		break;

	case AT86RF_TX_INVALID_LEN:
		px_phy_stats->ul_tx_err_bad_len++;
		break;

	case AT86RF_TX_INVALID_TRX_ID:
	case AT86RF_TX_INVALID_PARAM:
		px_phy_stats->ul_tx_err_bad_format++;
		break;

	case AT86RF_TX_ERROR_UNDERRUN:
	case AT86RF_TX_TIMEOUT:
	default:
		px_phy_stats->ul_tx_err_timeout++;
		break;
	}
}

/**
 * \brief Add TX confirm to the queue.
 *
 * \param uc_trx_id TRX identifier
 * \param px_tx_cfm Pointer to TX confirm parameters
 */
static void _tx_confirm(uint8_t uc_trx_id, at86rf_tx_cfm_t *px_tx_cfm)
{
	rf215_tx_ctl_t *px_tx_ctl;
	uint8_t uc_tx_cfm_wr;
	uint8_t uc_tx_cfm_rd;
	uint8_t uc_tx_id;
	uint8_t uc_tx_cfm_pending;

	px_tx_ctl = (rf215_tx_ctl_t *)&spx_tx_ctl[uc_trx_id];
	uc_tx_id = px_tx_cfm->uc_tx_id;
	uc_tx_cfm_rd = px_tx_ctl->uc_tx_cfm_rd;
	uc_tx_cfm_pending = px_tx_ctl->uc_tx_cfm_pending;

	/* Check pending TX confirm with same TX identifier */
	for (uint8_t uc_i = 0; uc_i < uc_tx_cfm_pending; uc_i++) {
		at86rf_tx_cfm_t *px_tx_cfm_pending = &px_tx_ctl->px_tx_cfm[uc_tx_cfm_rd];
		if (uc_tx_id == px_tx_cfm_pending->uc_tx_id) {
			/* Update TX confirm already in the queue */
			*px_tx_cfm_pending = *px_tx_cfm;
			return;
		}

		if (uc_tx_cfm_rd == (RF215_NUM_TX_CONFIRMS - 1)) {
			uc_tx_cfm_rd = 0;
		} else {
			uc_tx_cfm_rd++;
		}
	}

	if (uc_tx_cfm_pending == RF215_NUM_TX_CONFIRMS) {
		/* TX confirm buffer full. The oldest TX confirm won't be
		 * handled */
		gpx_phy_stats[uc_trx_id].ul_tx_cfm_not_handled++;
		if (px_tx_ctl->uc_tx_cfm_rd == (RF215_NUM_TX_CONFIRMS - 1)) {
			px_tx_ctl->uc_tx_cfm_rd = 0;
		} else {
			px_tx_ctl->uc_tx_cfm_rd++;
		}
	} else {
		/* Set pending TX confirm */
		px_tx_ctl->uc_tx_cfm_pending++;
	}

	/* Store TX confirm in the queue */
	uc_tx_cfm_wr = px_tx_ctl->uc_tx_cfm_wr;
	px_tx_ctl->px_tx_cfm[uc_tx_cfm_wr] = *px_tx_cfm;

	/* Update index for next TX confirm */
	if (uc_tx_cfm_wr == (RF215_NUM_TX_CONFIRMS - 1)) {
		px_tx_ctl->uc_tx_cfm_wr = 0;
	} else {
		px_tx_ctl->uc_tx_cfm_wr = uc_tx_cfm_wr + 1;
	}

	/* Update PHY statistics */
	_tx_upd_phy_stats(uc_trx_id, px_tx_cfm->uc_tx_res);
}

/**
 * \brief Abort ongoing/programmed TX. Send TX confirm and update PHY statistics
 *
 * \param uc_trx_id TRX identifier
 * \param uc_tx_id TX identifier
 * \param uc_tx_res TX result for TX confirm
 * \param b_reset True if TX is aborted due to TRX reset
 */
static void _tx_abort(uint8_t uc_trx_id, uint8_t uc_tx_id, at86rf_tx_cfm_res_t uc_tx_res, bool b_reset)
{
	uint32_t ul_current_time_phy;
	uint32_t ul_tx_time_phy;
	uint32_t ul_tx_duration;
	int32_t sl_tx_duration;
	at86rf_tx_cfm_t x_tx_cfm;
	rf215_tx_ctl_t *px_tx_ctl = (rf215_tx_ctl_t *)&spx_tx_ctl[uc_trx_id];

	/* Get current time */
	ul_current_time_phy = gx_rf215_hal_wrp.timer_get();

	if ((px_tx_ctl->uc_tx_id == uc_tx_id) && px_tx_ctl->b_tx_on) {
		/* Clear TX flag */
		px_tx_ctl->b_tx_on = false;

		if (gpx_phy_ctl[uc_trx_id].uc_phy_state == RF_PHY_STATE_TX) {
			bool b_ongoing_tx;

			if (b_reset) {
				/* If TX aborted after TRX reset, the TX event
				 * time can't be read from TRX anymore */
				ul_tx_time_phy = px_tx_ctl->ul_tx_time_cmd + px_tx_ctl->us_tx_cmd_delay_us;
			} else {
				/* Read counter (in capture mode). The TX start
				 * event occurs tx_bb_delay after TX command */
				uint32_t ul_tx_time_trx = rf215_bbc_get_cnt(uc_trx_id);
				ul_tx_time_trx += px_tx_ctl->us_proc_delay_us_q5;
				ul_tx_time_trx -= px_tx_ctl->uc_pe_delay_us_q5;

				/* Convert time from RF215 TRX to PHY time */
				ul_tx_time_phy = rf215_trx_to_phy_time(uc_trx_id, ul_tx_time_trx);

				/* Set ongoing TX aborted flag */
				px_tx_ctl->b_ongoing_tx_aborted = true;
			}

			/* Check TX in progress in the another TRX */
			b_ongoing_tx = false;
			for (uint8_t uc_id = 0; uc_id < AT86RF_NUM_TRX; uc_id++) {
				if (uc_id != uc_trx_id) {
					if ((gpx_phy_ctl[uc_id].uc_phy_state == RF_PHY_STATE_TX) || (gpx_phy_ctl[uc_id].uc_phy_state == RF_PHY_STATE_TX_CONTINUOUS)) {
						b_ongoing_tx = true;
					}
				}
			}

			/* Turn off TX LED if there is not TX in progress */
			if (!b_ongoing_tx) {
				gx_rf215_hal_wrp.rf_led(RF215_LED_TX, false);
			}
		} else {
			/* If TX has not started yet, set TX duration to 0 */
			ul_tx_time_phy = ul_current_time_phy;
		}
	} else {
		/* If TX has not started yet, set TX duration to 0 */
		ul_tx_time_phy = ul_current_time_phy;
	}

	/* TX duration (from TX start to TX aborted) */
	ul_tx_duration = ul_current_time_phy - ul_tx_time_phy;
	sl_tx_duration = (int32_t)ul_tx_duration;

	/* Check valid duration */
	if (sl_tx_duration < 0) {
		ul_tx_duration = 0;
	} else if (ul_tx_duration > px_tx_ctl->ul_frame_duration) {
		ul_tx_duration = px_tx_ctl->ul_frame_duration;
	}

	/* Set pending TX confirm with error result and actual duration */
	x_tx_cfm.ul_tx_time_ini = ul_tx_time_phy;
	x_tx_cfm.ul_frame_duration = ul_tx_duration;
	x_tx_cfm.uc_tx_res = uc_tx_res;
	x_tx_cfm.uc_tx_id = uc_tx_id;
	_tx_confirm(uc_trx_id, &x_tx_cfm);

	/* Look for programmed TX buffer with same TX ID */
	for (uint8_t uc_i = 0; uc_i < AT86RF215_NUM_TX_PROG_BUFFERS; uc_i++) {
		rf215_tx_prog_t *px_tx_prog = &px_tx_ctl->px_tx_prog[uc_i];
		if ((px_tx_prog->x_params.uc_tx_id ==  uc_tx_id) && (!px_tx_prog->b_free)) {
			/* Set programmed TX buffer as free */
			px_tx_prog->b_free = true;
			/* Cancel programmed timer interrupt */
			gx_rf215_hal_wrp.timer_cancel_int(px_tx_prog->ul_time_int_id);
		}
	}
}

/**
 * \brief Start transmission. Send TX command and write TX data buffer. It is
 * assumed that IRQ is disabled before calling this function.
 *
 * \param uc_trx_id TRX identifier
 * \param puc_data Pointer to data buffer
 */
__always_inline static void _tx_start(uint8_t uc_trx_id, uint8_t *puc_data)
{
	rf215_tx_ctl_t *px_tx_ctl = (rf215_tx_ctl_t *)&spx_tx_ctl[uc_trx_id];
	if (px_tx_ctl->b_cca_ed) {
		/* Start CCATX automatic procedure with Energy Detection */
		rf215_fe_ed_single(uc_trx_id);
		gpx_phy_ctl[uc_trx_id].uc_phy_state = RF_PHY_STATE_TX_CCA_ED;
	} else {
		/* Start transmission with TX command */
		rf215_trx_cmd_tx(uc_trx_id);
	}

	/* Store the time when TX procedure is started */
	px_tx_ctl->ul_tx_time_cmd = gx_rf215_hal_wrp.timer_get();

	if (px_tx_ctl->uc_cw <= 1) {
		/* Write TX data buffer, including FCS to avoid TX underrun
		 * error. FCS content doesn't matter because it will be
		 * overwritten by RF215 */
		rf215_bbc_write_tx_buf(uc_trx_id, puc_data, px_tx_ctl->us_psdu_len);
	}
}

/**
 * \brief Start transmission process. Perform carrier sense CCA and configure TX
 *  parameters if needed.
 *
 * \param uc_trx_id TRX identifier
 * \param px_tx_params Pointer to transmission parameters
 */
static at86rf_tx_cfm_res_t _tx_param_cfg(uint8_t uc_trx_id, at86rf_tx_params_t *px_tx_params)
{
	at86rf_cca_t uc_cca_mode;
	at86rf_tx_cfm_res_t uc_tx_res;
	rf215_tx_ctl_t volatile *px_tx_ctl = &spx_tx_ctl[uc_trx_id];
	rf215_phy_ctl_t volatile *px_phy_ctl = &gpx_phy_ctl[uc_trx_id];

	if (px_tx_ctl->b_tx_on && (px_tx_params->uc_tx_id != px_tx_ctl->uc_tx_id)) {
		/* TX in progress: Busy TX error */
		uc_tx_res = AT86RF_TX_BUSY_TX;
	} else {
		uc_tx_res = AT86RF_TX_SUCCESS;
	}

	uc_cca_mode = px_tx_params->uc_cca_mode;
	switch (px_phy_ctl->uc_phy_state) {
	case RF_PHY_STATE_RX_HEADER:
		/* RX in progress: Busy RX error (if CCA uses carrier sense) */
		if ((uc_cca_mode == AT86RF_CCA_MODE_2) || (uc_cca_mode == AT86RF_CCA_MODE_3)) {
			uc_tx_res = AT86RF_TX_BUSY_RX;
		}

		break;

	case RF_PHY_STATE_RX_PAYLOAD:
		/* RX payload in progress: Busy RX error (except CCA off) */
		if (uc_cca_mode != AT86RF_CCA_OFF) {
			uc_tx_res = AT86RF_TX_BUSY_RX;
		}

		break;

	case RF_PHY_STATE_TX_CONTINUOUS:
	case RF_PHY_STATE_IDLE:
		/* Continuous TX in progress: Busy TX error */
		uc_tx_res = AT86RF_TX_BUSY_TX;
		break;

	case RF_PHY_STATE_SLEPT:
		uc_tx_res = AT86RF_TX_TRX_SLEPT;
		break;

	case RF_PHY_STATE_RESET:
		uc_tx_res =  AT86RF_TX_TIMEOUT;
		break;

	default:
		break;
	}

	if (uc_tx_res == AT86RF_TX_SUCCESS) {
		/* CCA with Energy Detection */
		bool b_cca_ed = (bool)((uc_cca_mode == AT86RF_CCA_MODE_1) || (uc_cca_mode == AT86RF_CCA_MODE_3));
		at86rf_phy_cfg_t *px_phy_cfg = &gpx_phy_cfg[uc_trx_id];
		px_tx_ctl->x_cca_ed_cfg = px_phy_cfg->x_cca_ed_cfg;

		if (!px_tx_ctl->b_tx_on) {
			uint32_t ul_frame_duration;
			uint16_t us_pay_symbols;

			/* Update TX power and BBC (PHY) configuration */
			ul_frame_duration = rf215_bbc_upd_tx_params(uc_trx_id, px_tx_params, &us_pay_symbols);

			/* Update TX channel if needed */
			if (rf215_pll_check_chn_cfg(uc_trx_id, &px_phy_cfg->x_chn_cfg, px_tx_params->us_chn_num)) {
				if (px_tx_params->us_chn_num != px_phy_ctl->us_chn_num) {
					rf215_pll_set_chn_cfg(uc_trx_id, &px_phy_cfg->x_chn_cfg, px_tx_params->us_chn_num);
				}
			}

			/* Update TX control variables */
			px_phy_ctl->us_tx_pay_symbols = us_pay_symbols;
			px_tx_ctl->ul_frame_duration = ul_frame_duration;
			px_tx_ctl->uc_cw = px_tx_params->uc_cw;
			px_tx_ctl->b_tx_on = true;
			px_tx_ctl->b_cca_ed = b_cca_ed;
			px_tx_ctl->us_psdu_len = px_tx_params->us_psdu_len;
			px_tx_ctl->uc_tx_id = px_tx_params->uc_tx_id;
		}

		if ((!b_cca_ed) && (px_tx_ctl->uc_cw != 0)) {
			/* Update contention window for CCA without ED */
			px_tx_ctl->uc_cw--;
		}
	}

	return uc_tx_res;
}

/**
 * \brief Prepare transmission. Switch TRX to TXPREP state,
 * update synchronization for TX confirm time and write buffer length.
 *
 * \param uc_trx_id TRX identifier
 */
static void _tx_txprep(uint8_t uc_trx_id)
{
	uint8_t uc_cw;
	bool b_cca_ed;
	rf215_tx_ctl_t *px_tx_ctl;

	/* Switch TRX to TXPREP state to prepare for transmission */
	rf215_trx_switch_txprep(uc_trx_id);

	/* Configure CCATX/TX2RX automatic procedure */
	px_tx_ctl = (rf215_tx_ctl_t *)&spx_tx_ctl[uc_trx_id];
	b_cca_ed = px_tx_ctl->b_cca_ed;
	uc_cw = px_tx_ctl->uc_cw;
	rf215_bbc_tx_auto_cfg(uc_trx_id, b_cca_ed, &px_tx_ctl->x_cca_ed_cfg, uc_cw);

	if (b_cca_ed) {
		/* Switch TRX to RX state for Energy Detection */
		rf215_trx_cmd_rx(uc_trx_id);
	}

	if (uc_cw <= 1) {
		/* Write buffer length (PSDU length, including FCS) */
		rf215_bbc_set_tx_len(uc_trx_id, spx_tx_ctl[uc_trx_id].us_psdu_len);

		/* Update sync (TRX and PHY) for TX confirm time */
		rf215_trx_upd_sync(uc_trx_id);
	}

	/* Update PHY state */
	gpx_phy_ctl[uc_trx_id].uc_phy_state = RF_PHY_STATE_TX_TXPREP;
}

/**
 * \brief Start programmed transmission. Send the TX command at the appropriate
 * time.
 *
 * \param uc_trx_id TRX identifier
 * \param px_tx_prog Pointer to programmed TX buffer
 */
static void _tx_prog_start(uint8_t uc_trx_id, rf215_tx_prog_t *px_tx_prog)
{
	uint8_t *puc_data = px_tx_prog->puc_buf;
	uint32_t ul_tx_time_cmd = spx_tx_ctl[uc_trx_id].ul_tx_time_cmd;
	uint32_t ul_basepri_prev = __get_BASEPRI();
	uint16_t us_timeout = 5000;
	bool b_tx_started = false;

	do {
		uint32_t ul_current_time;
		int32_t sl_time_diff;

		/* Get current time */
		ul_current_time = gx_rf215_hal_wrp.timer_get();

		/* Check the remaining time to send TX command */
		sl_time_diff = (int32_t)ul_tx_time_cmd - ul_current_time;

		if ((sl_time_diff <= 0) && (sl_time_diff >= RF215_TX_PROG_MIN_DELAY_US)) {
			_tx_start(uc_trx_id, puc_data);
			b_tx_started = true;
		} else if ((sl_time_diff < RF215_TX_PROG_MIN_DELAY_US) || (sl_time_diff > RF215_TX_PROG_MAX_DELAY_US)) {
			/* Timeout error */
			break;
		} else if (sl_time_diff <= 30) {
			/* Enter critical region. Disable all interrupts except
			 * highest priority (<1: 0) to avoid TX start delay */
			__set_BASEPRI(1 << (8 - __NVIC_PRIO_BITS));
		}

		/* Timeout protection to avoid infinite loop */
		us_timeout--;
	} while ((!b_tx_started) && (us_timeout > 0));

	/* Leave critical region */
	__set_BASEPRI(ul_basepri_prev);

	if (!b_tx_started) {
		/* TX command wasn't sent (timeout error) */
		_tx_abort(uc_trx_id, px_tx_prog->x_params.uc_tx_id, AT86RF_TX_TIMEOUT, false);
		if (spx_tx_ctl[uc_trx_id].b_cca_ed) {
			/* Restore configuration after aborted ED by timeout */
			rf215_trx_cmd_txprep(uc_trx_id);
			rf215_bbc_ccatx_abort(uc_trx_id);
		}

		/* Start listening */
		rf215_trx_rx_listen(uc_trx_id, true);
	} else if (spx_tx_ctl[uc_trx_id].uc_cw <= 1) {
		/* Set programmed TX buffer as free */
		px_tx_prog->b_free = true;
	}
}

/**
 * \brief Timer interrupt handler for programmed TX
 *
 * \param ul_id Interrupt identifier
 */
static void _tx_prog_handler(uint32_t ul_id)
{
	rf215_phy_ctl_t *px_phy_ctl;
	rf215_tx_ctl_t *px_tx_ctl = NULL;
	rf215_tx_prog_t *px_tx_prog = NULL;
	at86rf_tx_params_t *px_params = NULL;
	uint8_t uc_trx_id;
	uint8_t uc_tx_buf_idx = 0;
	bool b_tx_id_ongoing;

	/* Critical region to avoid conflicts from RF IRQ */
	gx_rf215_hal_wrp.rf_enable_int(false);

	/* Search programmed TX buffer with same timer interrupt ID */
	for (uc_trx_id = 0; uc_trx_id < AT86RF_NUM_TRX; uc_trx_id++) {
		px_tx_ctl = (rf215_tx_ctl_t *)&spx_tx_ctl[uc_trx_id];
		for (uc_tx_buf_idx = 0; uc_tx_buf_idx < AT86RF215_NUM_TX_PROG_BUFFERS; uc_tx_buf_idx++) {
			px_tx_prog = &px_tx_ctl->px_tx_prog[uc_tx_buf_idx];
			if ((px_tx_prog->ul_time_int_id == ul_id) && (!px_tx_prog->b_free)) {
				px_params = &px_tx_prog->x_params;
				break;
			}
		}

		if (uc_tx_buf_idx < AT86RF215_NUM_TX_PROG_BUFFERS) {
			break;
		}
	}

	if ((uc_tx_buf_idx >= AT86RF215_NUM_TX_PROG_BUFFERS) ||
		(px_tx_ctl == NULL) ||
		(px_tx_prog == NULL) ||
		(px_params == NULL)) {
		/* Timer interrupt ID didn't match any programmed TX buffer */
		gx_rf215_hal_wrp.rf_enable_int(true);
		return;
	}

	/* Carrier sense CCA and TX parameters configuration */
	b_tx_id_ongoing = (bool)(px_tx_ctl->b_tx_on && (px_tx_ctl->uc_tx_id == px_params->uc_tx_id));
	px_phy_ctl = (rf215_phy_ctl_t *)&gpx_phy_ctl[uc_trx_id];
	if ((!b_tx_id_ongoing) || (px_phy_ctl->uc_phy_state < RF_PHY_STATE_TX_TXPREP)) {
		at86rf_tx_cfm_res_t uc_tx_res = _tx_param_cfg(uc_trx_id, px_params);
		if (uc_tx_res != AT86RF_TX_SUCCESS) {
			/* TX error: TX confirm and start listening if needed */
			_tx_abort(uc_trx_id, px_params->uc_tx_id, uc_tx_res, false);
			if (b_tx_id_ongoing) {
				rf215_trx_rx_listen(uc_trx_id, true);
			}
		}
	}

	/* TX preparation: TXPREP (without CCATX) or RX (with CCATX) */
	b_tx_id_ongoing = (bool)(px_tx_ctl->b_tx_on && (px_tx_ctl->uc_tx_id == px_params->uc_tx_id));
	if (b_tx_id_ongoing && (px_phy_ctl->uc_phy_state < RF_PHY_STATE_TX_TXPREP)) {
		uint32_t ul_tx_time;
		uint32_t ul_tx_time_int;
		uint32_t ul_tx_time_cmd;
		uint32_t ul_int_id;
		uint16_t us_tx_cmd_delay;
		uint16_t us_tx_total_delay;
		bool b_timer_prog;

		/* Total delay from next command to TX time */
		us_tx_total_delay = _tx_total_delay(uc_trx_id, px_params, &us_tx_cmd_delay);

		/* Compensate TX start delay */
		ul_tx_time = px_params->ul_tx_time;
		ul_tx_time_int = ul_tx_time - us_tx_total_delay;

		/* Try to program new timer IRQ to delay TX preparation */
		b_timer_prog = gx_rf215_hal_wrp.timer_set_int(ul_tx_time_int, false, _tx_prog_handler, &ul_int_id);

		if (b_timer_prog) {
			/* Timer interrupt programmed. Nothing more to do */
			px_tx_prog->ul_time_int_id = ul_int_id;
			px_tx_ctl->us_tx_cmd_delay_us = us_tx_cmd_delay;
			ul_tx_time_cmd = ul_tx_time - us_tx_cmd_delay;
			px_tx_ctl->ul_tx_time_cmd = ul_tx_time_cmd;
		} else {
			/* Not enough time to program new timer IRQ */
			if ((px_tx_ctl->uc_cw > 1) && (!px_tx_ctl->b_cca_ed)) {
				/* Force last CCA (contention window) */
				px_tx_ctl->uc_cw = 0;
				/* Update TX command delay */
				us_tx_cmd_delay = _tx_cmd_delay(uc_trx_id, false, px_tx_ctl->x_cca_ed_cfg.us_duration_us, 0);
			}

			/* TX preparation */
			_tx_txprep(uc_trx_id);

			/* Store TX command time and delay */
			ul_tx_time_cmd = ul_tx_time - us_tx_cmd_delay;
			px_tx_ctl->ul_tx_time_cmd = ul_tx_time_cmd;
			px_tx_ctl->us_tx_cmd_delay_us = us_tx_cmd_delay;
		}
	}

	/* TX or CCA command */
	if (b_tx_id_ongoing && (px_phy_ctl->uc_phy_state == RF_PHY_STATE_TX_TXPREP)) {
		/* Check if we can wait for TRXRDY IRQ */
		uint32_t ul_current_time = gx_rf215_hal_wrp.timer_get();
		int32_t sl_time_margin = (int32_t)px_tx_ctl->ul_tx_time_cmd - ul_current_time;
		if (px_phy_ctl->b_trxrdy || (sl_time_margin < px_tx_ctl->us_trxrdy_delay_us)) {
			/* No time to wait for TRXRDY: Start TX, waiting for the
			 * appropriate time */
			_tx_prog_start(uc_trx_id, px_tx_prog);
		}
	}

	/* Leave critical region */
	gx_rf215_hal_wrp.rf_enable_int(true);
}

/**
 * \brief RF215 Transceiver Ready Event. Update internal states after
 * RFn_IRQS.TRXRDY interrupt. Programmed TX can be started here. This function
 * is called from IRQ.
 *
 * \param uc_trx_id TRX identifier
 */
void rf215_tx_trxrdy_event(uint8_t uc_trx_id)
{
	uint8_t uc_buf_idx;
	rf215_tx_ctl_t *px_tx_ctl = (rf215_tx_ctl_t *)&spx_tx_ctl[uc_trx_id];
	rf215_tx_prog_t *px_tx_prog = &px_tx_ctl->px_tx_prog[0];

	/* Check if there is programmed TX pending to start */
	uc_buf_idx = 0xFF;
	if (gpx_phy_ctl[uc_trx_id].uc_phy_state == RF_PHY_STATE_TX_TXPREP) {
		for (uc_buf_idx = 0; uc_buf_idx < AT86RF215_NUM_TX_PROG_BUFFERS; uc_buf_idx++) {
			px_tx_prog = &px_tx_ctl->px_tx_prog[uc_buf_idx];
			if ((px_tx_prog->x_params.uc_tx_id == px_tx_ctl->uc_tx_id) && (!px_tx_prog->b_free)) {
				break;
			}
		}
	}

	if (uc_buf_idx < AT86RF215_NUM_TX_PROG_BUFFERS) {
		uint32_t ul_int_id;
		uint32_t ul_tx_time_int;
		bool b_timer_prog;

		/* Try to program new timer IRQ */
		ul_tx_time_int = px_tx_ctl->ul_tx_time_cmd;
		ul_tx_time_int -= RF215_TIMER_INT_DELAY_US;
		ul_tx_time_int -= RF215_TX_PROG_INT_MARGIN_US;
		b_timer_prog = gx_rf215_hal_wrp.timer_set_int(ul_tx_time_int, false, _tx_prog_handler, &ul_int_id);
		if (b_timer_prog) {
			/* Timer interrupt programmed */
			px_tx_prog->ul_time_int_id = ul_int_id;
		} else {
			/* Start TX, waiting for the appropriate time */
			_tx_prog_start(uc_trx_id, px_tx_prog);
		}
	}
}

/**
 * \brief RF215 Energy Detection Completion Event. Update internal states after
 * RFn_IRQS.EDC interrupt. Check the result of CCA Energy Detection. This
 * function is called from IRQ.
 *
 * \param uc_trx_id TRX identifier
 */
void rf215_tx_edc_event(uint8_t uc_trx_id)
{
	at86rf_cca_ed_cfg_t *px_cca_ed_cfg;
	bool b_busy_chn;
	rf215_tx_ctl_t *px_tx_ctl = (rf215_tx_ctl_t *)&spx_tx_ctl[uc_trx_id];
	rf215_phy_ctl_t *px_phy_ctl = (rf215_phy_ctl_t *)&gpx_phy_ctl[uc_trx_id];

	if (px_phy_ctl->uc_phy_state != RF_PHY_STATE_TX_CCA_ED) {
		/* Nothing to do */
		return;
	}

	/* Check result of Energy Detection and update configuration */
	px_cca_ed_cfg = &px_tx_ctl->x_cca_ed_cfg;
	b_busy_chn = rf215_bbc_ccatx_edc_event(uc_trx_id, px_cca_ed_cfg, px_tx_ctl->uc_cw);

	if (b_busy_chn) {
		/* Busy channel: TX confirm and start listening */
		_tx_abort(uc_trx_id, px_tx_ctl->uc_tx_id, AT86RF_TX_BUSY_CHN, false);
		rf215_trx_rx_listen(uc_trx_id, true);
	} else {
		/* Clear channel */
		if (px_tx_ctl->uc_cw != 0) {
			/* Update contention window length */
			px_tx_ctl->uc_cw--;
		}

		if (px_tx_ctl->uc_cw == 0) {
			/* Contention window finished */
			/* Turn on TX LED */
			gx_rf215_hal_wrp.rf_led(RF215_LED_TX, true);
			/* Update TRX and PHY state */
			px_phy_ctl->uc_trx_state = RF215_RFn_STATE_RF_TX;
			px_phy_ctl->uc_phy_state = RF_PHY_STATE_TX;
		} else {
			/* Contention window not finished */
			uint8_t uc_buf_idx;
			bool b_timer_prog;
			rf215_tx_prog_t *px_tx_prog = &px_tx_ctl->px_tx_prog[0];

			/* Start listening before next CCA */
			rf215_trx_rx_listen(uc_trx_id, false);

			/* Look for ongoing programmed buffer */
			uc_buf_idx = 0xFF;
			for (uc_buf_idx = 0; uc_buf_idx < AT86RF215_NUM_TX_PROG_BUFFERS; uc_buf_idx++) {
				px_tx_prog = &px_tx_ctl->px_tx_prog[uc_buf_idx];
				if ((px_tx_prog->x_params.uc_tx_id == px_tx_ctl->uc_tx_id) && (!px_tx_prog->b_free)) {
					break;
				}
			}

			if (uc_buf_idx < AT86RF215_NUM_TX_PROG_BUFFERS) {
				uint32_t ul_int_id;
				uint32_t ul_tx_time;
				uint32_t ul_tx_time_int;
				uint32_t ul_current_time;
				int32_t sl_time_margin;
				uint16_t us_tx_total_delay;
				uint16_t us_tx_cmd_delay;
				uint32_t ul_tx_time_cmd;

				/* Total delay from next command to TX time */
				us_tx_total_delay = _tx_total_delay(uc_trx_id, &px_tx_prog->x_params, &us_tx_cmd_delay);

				/* Compensate TX start delay */
				ul_tx_time = px_tx_prog->x_params.ul_tx_time;
				ul_tx_time_int = ul_tx_time - us_tx_total_delay;

				/* Try to program new timer IRQ */
				b_timer_prog = gx_rf215_hal_wrp.timer_set_int(ul_tx_time_int, false, _tx_prog_handler, &ul_int_id);

				if (b_timer_prog) {
					/* Timer interrupt programmed */
					px_tx_prog->ul_time_int_id = ul_int_id;
					px_tx_ctl->us_tx_cmd_delay_us = us_tx_cmd_delay;
					ul_tx_time_cmd = ul_tx_time - us_tx_cmd_delay;
					px_tx_ctl->ul_tx_time_cmd = ul_tx_time_cmd;
				} else {
					/* Not enough time: Prepare transmission */
					_tx_txprep(uc_trx_id);

					/* Store TX command time and delay */
					ul_tx_time_cmd = ul_tx_time - us_tx_cmd_delay;
					px_tx_ctl->ul_tx_time_cmd = ul_tx_time_cmd;
					px_tx_ctl->us_tx_cmd_delay_us = us_tx_cmd_delay;

					/* Check if there is time to wait for TRXRDY */
					ul_current_time = gx_rf215_hal_wrp.timer_get();
					sl_time_margin = (int32_t)ul_tx_time_cmd - ul_current_time;
					if (sl_time_margin < px_tx_ctl->us_trxrdy_delay_us) {
						/* No time to wait for TRXRDY: Start TX,
						 * waiting for the appropriate time */
						_tx_prog_start(uc_trx_id, px_tx_prog);
					}
				}
			} else {
				/* Unexpected error */
				_tx_abort(uc_trx_id, px_tx_ctl->uc_tx_id, AT86RF_TX_TIMEOUT, false);
			}
		}
	}
}

/**
 * \brief RF215 Transmitter Frame End Event. Process end of transmission after
 * BBCn_IRQS.TXFE interrupt. This function is called from IRQ.
 *
 * \param uc_trx_id TRX identifier
 */
void rf215_tx_frame_end_event(uint8_t uc_trx_id)
{
	rf215_tx_ctl_t *px_tx_ctl = (rf215_tx_ctl_t *)&spx_tx_ctl[uc_trx_id];
	rf215_phy_stats_t *px_phy_stats = &gpx_phy_stats[uc_trx_id];
	rf215_phy_ctl_t *px_phy_ctl = (rf215_phy_ctl_t *)&gpx_phy_ctl[uc_trx_id];
	rf_phy_state_t uc_phy_state = px_phy_ctl->uc_phy_state;
	bool b_tx_confirm = false;

	/* Check current PHY state */
	if (uc_phy_state == RF_PHY_STATE_TX) {
		/* Normal case: PHY state was TX */
		b_tx_confirm = true;
		px_tx_ctl->b_tx_on = false;
		if (!px_tx_ctl->b_cca_ed) {
			/* TRX switches automatically to RX (TX2RX enabled) */
			px_phy_ctl->uc_trx_state = RF215_RFn_STATE_RF_RX;
			px_phy_ctl->uc_phy_state = RF_PHY_STATE_RX_LISTEN;

			if (px_phy_ctl->b_phy_cfg_pending) {
				/* Pending PHY configuration */
				rf215_bbc_set_phy_cfg(uc_trx_id, &px_phy_ctl->x_phy_cfg_pending, px_phy_ctl->us_chn_num_pending);
			} else if (px_phy_ctl->us_chn_num_listen != px_phy_ctl->us_chn_num) {
				/* Restore frequency channel for RX if needed */
				rf215_pll_set_chn_cfg(uc_trx_id, &gpx_phy_cfg[uc_trx_id].x_chn_cfg, px_phy_ctl->us_chn_num_listen);
				px_phy_ctl->b_trxrdy = false;
				rf215_trx_rx_listen(uc_trx_id, true);
			}
		} else {
			/* TRX switches automatically to TXPREP */
			px_phy_ctl->uc_trx_state = RF215_RFn_STATE_RF_TXPREP;
			rf215_trx_rx_listen(uc_trx_id, true);
		}
	} else if (uc_phy_state == RF_PHY_STATE_TX_CCA_ED) {
		/* This should not happen (EDC interrupt should be first) */
		/* Busy channel: TX confirm and start listening */
		_tx_abort(uc_trx_id, spx_tx_ctl[uc_trx_id].uc_tx_id, AT86RF_TX_BUSY_CHN, false);
		rf215_trx_cmd_txprep(uc_trx_id);
		rf215_bbc_ccatx_abort(uc_trx_id);
		rf215_trx_rx_listen(uc_trx_id, true);
	} else if (px_tx_ctl->b_ongoing_tx_aborted && (px_tx_ctl->uc_tx_cfm_pending != 0)) {
		/* TX aborted just before, but it has actually finished. TRX
		 * state is not changed, but TX confirm is filled (overwriting
		 * the aborted result). Undo PHY stats update */
		b_tx_confirm = true;
		px_phy_stats->ul_tx_err_total--;
		px_phy_stats->ul_tx_err_aborted--;
		px_tx_ctl->uc_tx_cfm_pending--;
		if (px_tx_ctl->uc_tx_cfm_wr == 0) {
			px_tx_ctl->uc_tx_cfm_wr = RF215_NUM_TX_CONFIRMS - 1;
		} else {
			px_tx_ctl->uc_tx_cfm_wr--;
		}
	}

	/* Clear ongoing TX aborted flag */
	px_tx_ctl->b_ongoing_tx_aborted = false;

	if (b_tx_confirm) {
		at86rf_tx_cfm_t x_tx_cfm;
		uint32_t ul_tx_time_trx;
		uint8_t uc_tx_underrun;

		/* Read counter (in capture mode). The TX start event occurs tx_bb_delay
		 * after TX command */
		ul_tx_time_trx = rf215_bbc_get_cnt(uc_trx_id);
		ul_tx_time_trx += px_tx_ctl->us_proc_delay_us_q5;
		ul_tx_time_trx -= px_tx_ctl->uc_pe_delay_us_q5;

		/* Convert time read from RF215 TRX to PHY time */
		x_tx_cfm.ul_tx_time_ini = rf215_trx_to_phy_time(uc_trx_id, ul_tx_time_trx);
		x_tx_cfm.ul_frame_duration = px_tx_ctl->ul_frame_duration;
		x_tx_cfm.uc_tx_id = px_tx_ctl->uc_tx_id;

		/* Read TX Underrun status */
		uc_tx_underrun = rf215_spi_reg_read(RF215_ADDR_BBCn_PS(uc_trx_id));
		if (uc_tx_underrun & RF215_BBCn_PS_TXUR) {
			/* TX underrun error */
			x_tx_cfm.uc_tx_res = AT86RF_TX_ERROR_UNDERRUN;
		} else {
			/* Successful transmission */
			x_tx_cfm.uc_tx_res = AT86RF_TX_SUCCESS;
		}

		/* Set pending TX confirm */
		_tx_confirm(uc_trx_id, &x_tx_cfm);
	}
}

/**
 * \brief Transmission request.
 *
 * \param uc_trx_id TRX identifier
 * \param px_tx_params Pointer to transmission parameters
 */
void at86rf_tx_req(uint8_t uc_trx_id, at86rf_tx_params_t *px_tx_params)
{
	at86rf_tx_cfm_res_t uc_tx_res;

	if (guc_rf215_comp_state != RF215_COMPONENT_ENABLED) {
		/* Error: Component not enabled */
		return;
	}

	if (uc_trx_id >= AT86RF_NUM_TRX) {
		/* Error: invalid TRX */
		uc_tx_res = AT86RF_TX_INVALID_TRX_ID;
	} else {
		rf215_tx_ctl_t *px_tx_ctl;
		rf215_tx_prog_t *px_tx_id_prog;
		at86rf_tx_time_mode_t uc_time_mode;
		uint32_t ul_tx_id_timer_id = 0;
		uint32_t ul_tx_time;
		uint8_t uc_tx_id;
		uint8_t uc_tx_id_buf_idx;
		uint8_t uc_free_buf_idx;
		bool b_tx_id_ongoing;

		/* Critical region to avoid state changes from IRQ */
		gx_rf215_hal_wrp.rf_enable_int(false);
		gx_rf215_hal_wrp.timer_enable_int(false);

		/* Check if there is TX in progress with same TX identifier */
		uc_tx_id = px_tx_params->uc_tx_id;
		px_tx_ctl = (rf215_tx_ctl_t *)&spx_tx_ctl[uc_trx_id];
		px_tx_id_prog = &px_tx_ctl->px_tx_prog[0];
		b_tx_id_ongoing = (bool)(px_tx_ctl->b_tx_on && (px_tx_ctl->uc_tx_id == uc_tx_id));

		/* Look for programmed TX buffer free and with same TX ID */
		uc_tx_id_buf_idx = 0xFF;
		uc_free_buf_idx = 0xFF;
		for (uint8_t uc_i = 0; uc_i < AT86RF215_NUM_TX_PROG_BUFFERS; uc_i++) {
			if (px_tx_ctl->px_tx_prog[uc_i].b_free) {
				/* Free programmed TX buffer found */
				uc_free_buf_idx = uc_i;
			} else if (px_tx_ctl->px_tx_prog[uc_i].x_params.uc_tx_id ==  uc_tx_id) {
				/* Programmed TX buffer with same TX ID found */
				uc_tx_id_buf_idx = uc_i;
				px_tx_id_prog = &px_tx_ctl->px_tx_prog[uc_i];
				ul_tx_id_timer_id = px_tx_id_prog->ul_time_int_id;
			}
		}

		uc_time_mode = px_tx_params->uc_time_mode;
		if (uc_time_mode == AT86RF_TX_CANCEL) {
			if (b_tx_id_ongoing) {
				/* Abort TX auto procedure with control,
				 * avoiding RFn_CMD confict */
				rf215_tx_auto_stop(uc_trx_id, AT86RF_TX_CANCELLED);

				if (px_tx_ctl->b_tx_on) {
					/* TX not aborted yet */
					_tx_abort(uc_trx_id, uc_tx_id, AT86RF_TX_CANCELLED, false);
				}

				/* Cancel TX in progress and start listening */
				rf215_trx_rx_listen(uc_trx_id, true);
			} else if (uc_tx_id_buf_idx != 0xFF) {
				/* Notify TX confirm with aborted result */
				_tx_abort(uc_trx_id, uc_tx_id, AT86RF_TX_CANCELLED, false);
			}

			/* Nothing more to do */
			uc_tx_res = AT86RF_TX_SUCCESS;
		} else {
			/* Check TX parameters */
			uc_tx_res = _tx_check_params(uc_trx_id, px_tx_params);
			if ((uc_tx_res == AT86RF_TX_SUCCESS) && b_tx_id_ongoing) {
				/* TX ID already in progress: do nothing */
				uc_time_mode = AT86RF_TX_CANCEL;
			}
		}

		if ((uc_tx_res == AT86RF_TX_SUCCESS) && (uc_time_mode == AT86RF_TX_TIME_INST) && (px_tx_params->uc_cw > 1)) {
			/* Instantaneous TX with slotted CSMA-CA (CW > 1)
			 * handled as programmed TX with minimum delay */
			uc_time_mode = AT86RF_TX_TIME_REL;
			ul_tx_time = 0;
		} else {
			ul_tx_time = px_tx_params->ul_tx_time;
		}

		if ((uc_tx_res == AT86RF_TX_SUCCESS) && ((uc_time_mode == AT86RF_TX_TIME_REL) || (uc_time_mode == AT86RF_TX_TIME_ABS))) {
			/* Check if there are TX programmed buffers available */
			if ((uc_tx_id_buf_idx == 0xFF) && (uc_free_buf_idx == 0xFF)) {
				uc_tx_res = AT86RF_TX_FULL_BUFFERS;
			}
		}

		if ((uc_tx_res == AT86RF_TX_SUCCESS) && ((uc_time_mode == AT86RF_TX_TIME_REL) || (uc_time_mode == AT86RF_TX_TIME_ABS))) {
			uint32_t ul_basepri_prev;
			uint32_t ul_current_time;
			uint32_t ul_tx_time_int;
			uint32_t ul_int_id;
			uint16_t us_tx_cmd_delay;
			int32_t sl_time_margin;
			uint16_t us_tx_total_delay;
			uint8_t uc_tx_prog_buf_idx;
			bool b_prog_timer_int;

			/* Delay from TX command to actual TX time */
			us_tx_total_delay = _tx_total_delay(uc_trx_id, px_tx_params, &us_tx_cmd_delay);

			/* Minimum delay for relative time: TX total delay */
			if (uc_time_mode == AT86RF_TX_TIME_REL) {
				ul_tx_time = max(ul_tx_time, us_tx_total_delay);
			}

			/* Compensate TX start delay */
			ul_tx_time_int = ul_tx_time - us_tx_total_delay;

			/* Enter critical region. Disable all interrupts except
			 * highest priority (<1: 0) to avoid delays in
			 * computations with current time */
			ul_basepri_prev = __get_BASEPRI();
			__set_BASEPRI(1 << (8 - __NVIC_PRIO_BITS));

			/* Get current time */
			ul_current_time = gx_rf215_hal_wrp.timer_get();

			if (uc_time_mode == AT86RF_TX_TIME_REL) {
				/* Relative time mode */
				ul_tx_time_int += ul_current_time;
				ul_tx_time += ul_current_time;
			}

			/* Margin time until programmed interrupt */
			sl_time_margin = (int32_t)ul_tx_time_int - ul_current_time;

			uc_tx_prog_buf_idx = 0xFF;
			b_prog_timer_int = false;
			if (uc_tx_id_buf_idx != 0xFF) {
				/* New TX time for already programmed TX ID
				 * If no time to reprogram: only update time
				 * If not valid time: do nothing
				 * Otherwise: Reprogram interrupt */
				if ((sl_time_margin >= RF215_TX_PROG_MIN_DELAY_US) && (sl_time_margin < RF215_TIMER_PROG_INT_MARGIN_US)) {
					uc_tx_prog_buf_idx = uc_tx_id_buf_idx;
					ul_int_id = ul_tx_id_timer_id;
				} else if ((sl_time_margin >= RF215_TIMER_PROG_INT_MARGIN_US) && (sl_time_margin <= RF215_TX_PROG_MAX_DELAY_US)) {
					uc_tx_prog_buf_idx = uc_tx_id_buf_idx;
					b_prog_timer_int = true;
					/* TX ID reprogrammed: cancel previous IRQ */
					gx_rf215_hal_wrp.timer_cancel_int(ul_tx_id_timer_id);
				}
			} else if ((sl_time_margin < RF215_TX_PROG_MIN_DELAY_US) || (sl_time_margin > RF215_TX_PROG_MAX_DELAY_US)) {
				/* New TX ID with invalid time: notify error */
				uc_tx_res = AT86RF_TX_TIMEOUT;
			} else if (sl_time_margin < RF215_TIMER_PROG_INT_MARGIN_US) {
				/* New TX ID with few margin: adjust IRQ time */
				ul_tx_time_int += (RF215_TIMER_PROG_INT_MARGIN_US - sl_time_margin);
				uc_tx_prog_buf_idx = uc_free_buf_idx;
				b_prog_timer_int = true;
			} else {
				/* New TX ID with valid time and enough margin */
				uc_tx_prog_buf_idx = uc_free_buf_idx;
				b_prog_timer_int = true;
			}

			if (b_prog_timer_int) {
				/* Program timer interrupt for programmed TX */
				bool b_timer_prog;
				b_timer_prog = gx_rf215_hal_wrp.timer_set_int(ul_tx_time_int, false, _tx_prog_handler, &ul_int_id);
				if (!b_timer_prog) {
					uc_tx_res = AT86RF_TX_TIMEOUT;
				}
			}

			/* Leave critical region */
			__set_BASEPRI(ul_basepri_prev);

			if ((uc_tx_prog_buf_idx != 0xFF) && (uc_tx_res == AT86RF_TX_SUCCESS)) {
				rf215_tx_prog_t *px_tx_prog;
				uint16_t us_data_len;

				/* Save TX parameters and buffer */
				px_tx_prog = &px_tx_ctl->px_tx_prog[uc_tx_prog_buf_idx];
				px_tx_prog->b_free = false;
				px_tx_prog->ul_time_int_id = ul_int_id;
				px_tx_prog->x_params = *px_tx_params;
				px_tx_prog->x_params.ul_tx_time = ul_tx_time;
				us_data_len = px_tx_params->us_psdu_len - AT86RF_FCS_LEN;
				memcpy(px_tx_prog->puc_buf, px_tx_params->puc_data, us_data_len);
			}
		}

		if ((uc_tx_res == AT86RF_TX_SUCCESS) && (uc_time_mode == AT86RF_TX_TIME_INST)) {
			/* Carrier sense CCA and TX parameters configuration */
			uc_tx_res = _tx_param_cfg(uc_trx_id, px_tx_params);
		}

		if ((uc_tx_res == AT86RF_TX_SUCCESS) && (uc_time_mode == AT86RF_TX_TIME_INST)) {
			uint16_t us_tx_cmd_delay;
			at86rf_cca_t uc_cca_mode;
			uint16_t us_cca_ed_duration;
			bool b_cca_ed;

			if (uc_tx_id_buf_idx != 0xFF) {
				/* TX ID reprogrammed: cancel previous IRQ */
				gx_rf215_hal_wrp.timer_cancel_int(ul_tx_id_timer_id);
				px_tx_id_prog->b_free = true;
			}

			/* Prepare and start transmission as soon as possible */
			_tx_txprep(uc_trx_id);
			_tx_start(uc_trx_id, px_tx_params->puc_data);

			/* Delay from TX command to actual TX time */
			uc_cca_mode = px_tx_params->uc_cca_mode;
			b_cca_ed = (bool)((uc_cca_mode == AT86RF_CCA_MODE_1) || (uc_cca_mode == AT86RF_CCA_MODE_3));
			us_cca_ed_duration = gpx_phy_cfg[uc_trx_id].x_cca_ed_cfg.us_duration_us;
			us_tx_cmd_delay = _tx_cmd_delay(uc_trx_id, b_cca_ed, us_cca_ed_duration, px_tx_params->uc_cw);
			px_tx_ctl->us_tx_cmd_delay_us = us_tx_cmd_delay;
		}

		/* Leave critical region */
		gx_rf215_hal_wrp.rf_enable_int(true);
		gx_rf215_hal_wrp.timer_enable_int(true);
	}

	if (uc_tx_res != AT86RF_TX_SUCCESS) {
		/* Update PHY statistics */
		_tx_upd_phy_stats(uc_trx_id, uc_tx_res);

		/* There is an error: report in TX confirm */
		if (gx_rf215_callbacks.rf_tx_cfm_cb) {
			at86rf_tx_cfm_t x_tx_cfm;
			x_tx_cfm.uc_tx_res = uc_tx_res;
			x_tx_cfm.ul_tx_time_ini = gx_rf215_hal_wrp.timer_get();
			x_tx_cfm.ul_frame_duration = 0;
			x_tx_cfm.uc_tx_id = px_tx_params->uc_tx_id;
			gx_rf215_callbacks.rf_tx_cfm_cb(uc_trx_id, &x_tx_cfm);
		}
	} else {
#ifdef AT86RF_ADDONS_ENABLE
		if (px_tx_params->uc_time_mode != AT86RF_TX_CANCEL) {
			/* Fill addon buffer from TX request */
			rf215_addon_stringify_tx(uc_trx_id, px_tx_params);
		}
#endif
	}
}

/**
 * \brief Check if there is a RF215 TX automatic procedure in process and abort
 * it under control, avoiding writing RFn_CMD at the same time that RF215
 * automatically changes the TRX state (CCATX / TX2RX).  It is assumed that IRQ
 * is disabled before calling this function.
 *
 * \param uc_trx_id TRX identifier
 * \param uc_tx_res TX result for TX confirm
 */
void rf215_tx_auto_stop(uint8_t uc_trx_id, at86rf_tx_cfm_res_t uc_tx_res)
{
	uint32_t ul_basepri_prev = __get_BASEPRI();
	uint32_t ul_current_time;
	int32_t sl_margin_time;
	bool b_tx_aborted;
	bool b_abort_tx = false;
	bool b_abort_cca = false;
	bool b_abort_wait = false;
	rf215_tx_ctl_t *px_tx_ctl = (rf215_tx_ctl_t *)&spx_tx_ctl[uc_trx_id];
	uint32_t ul_time_end = px_tx_ctl->ul_tx_time_cmd;
	rf_phy_state_t uc_phy_state = gpx_phy_ctl[uc_trx_id].uc_phy_state;
	bool b_cca_ed = px_tx_ctl->b_cca_ed;

	if ((uc_phy_state == RF_PHY_STATE_TX_TXPREP) && b_cca_ed) {
		/* Preparation for ED (Baseband Core disabled). Not needed to
		 * wait for any auto procedure end */
		b_abort_cca = true;
	} else if (uc_phy_state == RF_PHY_STATE_TX_CCA_ED) {
		/* ED in progress (Baseband Core disabled) */
		b_abort_cca = true;
		if (px_tx_ctl->uc_cw <= 1) {
			/* CCATX in progress */
			b_abort_wait = true;
			/* Time of Energy Detection end */
			ul_time_end += px_tx_ctl->x_cca_ed_cfg.us_duration_us;
		}
	} else if ((uc_phy_state == RF_PHY_STATE_TX) && (!b_cca_ed)) {
		/* TX2RX in progress */
		b_abort_tx = true;
		b_abort_wait = true;

		/* Time of TX frame end */
		ul_time_end += px_tx_ctl->us_tx_cmd_delay_us;
		ul_time_end += px_tx_ctl->ul_frame_duration;
	}

	if (b_abort_tx || b_abort_cca) {
		if (b_abort_wait) {
			/* Enter critical region. Disable all interrupts except
			 * highest prio (<1: 0) to avoid delays when checking
			 * current time */
			__set_BASEPRI(1 << (8 - __NVIC_PRIO_BITS));

			/* Get current time and remaining time for auto
			 * procedure end */
			ul_current_time = gx_rf215_hal_wrp.timer_get();
			sl_margin_time = (int32_t)ul_time_end - ul_current_time;

			if (sl_margin_time > 200) {
				/* It is safe to abort TX / CCATX by RFn_CMD
				 * (TXPREP). Automatic procedures
				 * (CCATX / TX2RX) disabled to avoid conflict
				 * issues */
				rf215_bbc_tx_auto_stop(uc_trx_id);
				rf215_trx_cmd_txprep(uc_trx_id);
				b_tx_aborted = true;
			} else {
				/* Not enough margin */
				b_tx_aborted = false;
			}

			/* Leave critical region */
			__set_BASEPRI(ul_basepri_prev);
		} else {
			/* It is safe to abort CCA preparation (RX state) by
			 * RFn_CMD (TXPREP). Automatic procedures (CCATX)
			 * disabled to avoid conflict issues */
			rf215_bbc_tx_auto_stop(uc_trx_id);
			rf215_trx_cmd_txprep(uc_trx_id);
			b_tx_aborted = true;
		}

		if (b_tx_aborted) {
			if (b_abort_cca) {
				/* Restore configuration after aborted CCATX */
				rf215_bbc_ccatx_abort(uc_trx_id);
			}

			/* Transmission aborted */
			_tx_abort(uc_trx_id, px_tx_ctl->uc_tx_id, uc_tx_res, false);

			/* Update PHY state to avoid double TX confirm */
			gpx_phy_ctl[uc_trx_id].uc_phy_state = RF_PHY_STATE_TX_ABORT;
		} else {
			uint8_t uc_timeout;

			/* Enable RF IRQ */
			gx_rf215_hal_wrp.rf_enable_int(true);

			/* Make sure that interrupt is enabled in BASEPRI */
			if (ul_basepri_prev != 0) {
				__set_BASEPRI(0);
			}

			/* Wait for PHY state change from IRQ.
			 * TX2RX will finish normally (TX successful)
			 * CCATX will finish normally (start TX or busy channel)
			 */
			uc_timeout = 100;
			while ((uc_timeout > 0) && (gpx_phy_ctl[uc_trx_id].uc_phy_state == uc_phy_state)) {
				__WFE();
				uc_timeout--;
			}

			/* Restore BASEPRI */
			__set_BASEPRI(ul_basepri_prev);

			/* Enable RF IRQ */
			gx_rf215_hal_wrp.rf_enable_int(false);
		}
	}

	uc_phy_state = gpx_phy_ctl[uc_trx_id].uc_phy_state;
	if ((uc_phy_state == RF_PHY_STATE_TX) && b_cca_ed) {
		/* CCATX finished and TX started. We need to disable CCATX auto
		 * procedure to avoid RFn_CMD conflicts */
		ul_time_end = px_tx_ctl->ul_tx_time_cmd;

		/* Time of TX frame end */
		ul_time_end += px_tx_ctl->us_tx_cmd_delay_us;
		ul_time_end += px_tx_ctl->ul_frame_duration;

		/* Enter critical region. Disable all interrupts except highest
		 * prio (<1: 0) to avoid delays when checking current time */
		__set_BASEPRI(1 << (8 - __NVIC_PRIO_BITS));

		/* Get current time and remaining time for auto procedure end */
		ul_current_time = gx_rf215_hal_wrp.timer_get();
		sl_margin_time = (int32_t)ul_time_end - ul_current_time;

		if (sl_margin_time > 200) {
			/* It is safe to abort TX by RFn_CMD (TXPREP). Automatic
			 * procedures (CCATX) disabled to avoid conflict
			 * issues */
			rf215_bbc_tx_auto_stop(uc_trx_id);
			rf215_trx_cmd_txprep(uc_trx_id);
			b_tx_aborted = true;
		} else {
			/* Not enough margin */
			b_tx_aborted = false;
		}

		/* Leave critical region */
		__set_BASEPRI(ul_basepri_prev);

		if (b_tx_aborted) {
			/* Transmission aborted */
			_tx_abort(uc_trx_id, px_tx_ctl->uc_tx_id, uc_tx_res, false);

			/* Update PHY state to avoid double TX confirm */
			gpx_phy_ctl[uc_trx_id].uc_phy_state = RF_PHY_STATE_TX_ABORT;
		} else {
			uint8_t uc_timeout;

			/* Enable RF IRQ */
			gx_rf215_hal_wrp.rf_enable_int(true);

			/* Make sure that interrupt is enabled in BASEPRI */
			if (ul_basepri_prev != 0) {
				__set_BASEPRI(0);
			}

			/* Wait for PHY state change from IRQ.
			 * TX will finish normally (TX successful)
			 */
			uc_timeout = 100;
			while ((uc_timeout > 0) && (gpx_phy_ctl[uc_trx_id].uc_phy_state == uc_phy_state)) {
				__WFE();
				uc_timeout--;
			}

			/* Restore BASEPRI */
			__set_BASEPRI(ul_basepri_prev);

			/* Enable RF IRQ */
			gx_rf215_hal_wrp.rf_enable_int(false);
		}
	}
}

/**
 * \brief Check if there is transmission or reception in progress and abort it.
 *
 * \param uc_trx_id TRX identifier
 * \param b_reset True if checking aborts due to TRX reset
 */
void rf215_tx_rx_check_aborts(uint8_t uc_trx_id, bool b_reset)
{
	switch (gpx_phy_ctl[uc_trx_id].uc_phy_state) {
	case RF_PHY_STATE_TX_TXPREP:
	case RF_PHY_STATE_TX_CCA_ED:
	case RF_PHY_STATE_TX:
		/* Transmission aborted */
		_tx_abort(uc_trx_id, spx_tx_ctl[uc_trx_id].uc_tx_id, AT86RF_TX_ABORTED, b_reset);
		break;

	case RF_PHY_STATE_RX_HEADER:
	case RF_PHY_STATE_RX_PAYLOAD:
		/* Reception aborted */
		rf215_rx_abort(uc_trx_id);
		break;

	default:
		break;
	}
}

/**
 * \brief Check if there are programmed TX to cancel because of valid header
 * received.
 *
 * \param uc_trx_id TRX identifier
 */
void rf215_tx_abort_by_rx(uint8_t uc_trx_id)
{
	for (uint8_t uc_i = 0; uc_i < AT86RF215_NUM_TX_PROG_BUFFERS; uc_i++) {
		rf215_tx_prog_t *px_tx_prog = (rf215_tx_prog_t *)&spx_tx_ctl->px_tx_prog[uc_i];
		if ((!px_tx_prog->b_free) && px_tx_prog->x_params.b_cancel_by_rx) {
			_tx_abort(uc_trx_id, px_tx_prog->x_params.uc_tx_id, AT86RF_TX_CANCEL_BY_RX, false);
		}
	}
}

/**
 * \brief Set TX Baseband Processing delays.
 *
 * \param uc_trx_id TRX identifier
 * \param us_bb_delay_us_q5 Baseband processing delay in us [uQ6.5]
 * \param uc_pe_delay_us_q5 Pre-emphasis processing delay to compensate in TX
 * confirm time in us [uQ3.5]
 */
void rf215_tx_set_bb_delay(uint8_t uc_trx_id, uint16_t us_bb_delay_us_q5, uint8_t uc_pe_delay_us_q5)
{
	spx_tx_ctl[uc_trx_id].us_bb_delay_us_q5 = us_bb_delay_us_q5;
	spx_tx_ctl[uc_trx_id].uc_pe_delay_us_q5 = uc_pe_delay_us_q5;
}

/**
 * \brief Set Transmitter Frontend delay.
 *
 * \param uc_trx_id TRX identifier
 * \param us_proc_delay_us_q5 Transmitter Frontend delay in us [uQ6.5]
 */
void rf215_tx_set_proc_delay(uint8_t uc_trx_id, uint16_t us_proc_delay_us_q5)
{
	spx_tx_ctl[uc_trx_id].us_proc_delay_us_q5 = us_proc_delay_us_q5;
}

/**
 * \brief Function to check pending TX confirm(s)
 *
 * \param uc_trx_id TRX identifier
 */
void rf215_tx_event_handler(uint8_t uc_trx_id)
{
	volatile rf215_tx_ctl_t *px_tx_ctl = &spx_tx_ctl[uc_trx_id];

	/* Check pending TX confirm */
	while (px_tx_ctl->uc_tx_cfm_pending != 0) {
		at86rf_tx_cfm_t x_tx_cfm;
		uint8_t uc_idx;

		/* Critical region to avoid modification of pending TX
		 * confirms from interrupt */
		gx_rf215_hal_wrp.rf_enable_int(false);
		gx_rf215_hal_wrp.timer_enable_int(false);

		/* Copy TX confirm to local struct */
		uc_idx = px_tx_ctl->uc_tx_cfm_rd;
		x_tx_cfm = px_tx_ctl->px_tx_cfm[uc_idx];

		/* Update index for next RX indication */
		if (uc_idx == (RF215_NUM_TX_CONFIRMS - 1)) {
			px_tx_ctl->uc_tx_cfm_rd = 0;
		} else {
			px_tx_ctl->uc_tx_cfm_rd = uc_idx + 1;
		}

		/* Update number of pending TX confirm */
		px_tx_ctl->uc_tx_cfm_pending--;

		/* Leave critical region */
		gx_rf215_hal_wrp.timer_enable_int(true);
		gx_rf215_hal_wrp.rf_enable_int(true);

#ifdef AT86RF_ADDONS_ENABLE
		uint8_t *puc_addon_buf;
		uint16_t us_addon_len;

		/* Get addon buffer from TX confirm */
		puc_addon_buf = rf215_addon_stringify_cfm(uc_trx_id, &x_tx_cfm, &us_addon_len);

		if (gx_rf215_callbacks.rf_addon_event_cb && (us_addon_len != 0)) {
			/* Indicate addon event */
			gx_rf215_callbacks.rf_addon_event_cb(puc_addon_buf, us_addon_len);
		}
#endif
		if (gx_rf215_callbacks.rf_tx_cfm_cb && (x_tx_cfm.uc_tx_res != AT86RF_TX_CANCELLED)) {
			/* Indicate RX indication to upper layer */
			gx_rf215_callbacks.rf_tx_cfm_cb(uc_trx_id, &x_tx_cfm);
		}
	}
}

/**
 * \brief Function to initialize TX controller module.
 *
 * \param uc_trx_id TRX identifier
 */
void rf215_tx_init(uint8_t uc_trx_id)
{
	rf215_tx_ctl_t *px_tx_ctl = (rf215_tx_ctl_t *)&spx_tx_ctl[uc_trx_id];
	px_tx_ctl->us_trxrdy_delay_us = _tx_trxrdy_delay();
	px_tx_ctl->us_trxoff_txprep_delay_us = _tx_trxoff_txprep_delay();
	px_tx_ctl->us_tx_param_cfg_delay_us = _tx_param_cfg_delay();
	px_tx_ctl->uc_tx_cfm_pending = 0;
	px_tx_ctl->uc_tx_cfm_wr = 0;
	px_tx_ctl->uc_tx_cfm_rd = 0;
	px_tx_ctl->b_tx_on = false;
	px_tx_ctl->b_ongoing_tx_aborted = false;
	for (uint8_t uc_i = 0; uc_i < AT86RF215_NUM_TX_PROG_BUFFERS; uc_i++) {
		px_tx_ctl->px_tx_prog[uc_i].b_free = true;
	}
}

#ifdef __cplusplus
}
#endif
