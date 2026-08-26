/**
 *
 * \file
 *
 * \brief RF215 RX controller.
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
#include "rf215_reg.h"
#include "rf215_phy_defs.h"
#include "rf215_fe.h"
#include "rf215_bbc.h"
#include "rf215_trx_ctl.h"
#ifdef AT86RF_ADDONS_ENABLE
# include "rf215_addon.h"
#endif

/* System includes */
#include "string.h"

/* RX control struct (one per TRX) */
static volatile rf215_rx_ctl_t spx_rx_ctl[AT86RF_NUM_TRX];

/* RX indication data buffer for upper layer */
static uint8_t spuc_rx_ind_data_buf[AT86RF215_MAX_PSDU_LEN];

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief RF215 AGC Hold Event. Process the detection of preamble (AGC hold)
 * after BBCn_IRQS.AGCH interrupt. This function is called from IRQ.
 *
 * \param uc_trx_id TRX identifier
 */
static inline void _rf215_rx_agch(uint8_t uc_trx_id)
{
	rf_phy_state_t uc_phy_state = gpx_phy_ctl[uc_trx_id].uc_phy_state;
	if ((uc_phy_state == RF_PHY_STATE_RX_HEADER) || (uc_phy_state == RF_PHY_STATE_RX_PAYLOAD)) {
		/* New preamble detected while reception in progress (overrided
		 * by higher signal level detected) */
		gpx_phy_stats[uc_trx_id].ul_rx_override++;
	}

	if ((uc_phy_state >= RF_PHY_STATE_RX_LISTEN) && (uc_phy_state <= RF_PHY_STATE_RX_PAYLOAD)) {
		/* Preamble detected: Update PHY state to RX_HEADER (SHR + PHR) */
		gpx_phy_ctl[uc_trx_id].uc_phy_state = RF_PHY_STATE_RX_HEADER;

		/* Clear RX abort flag */
		spx_rx_ctl[uc_trx_id].b_rx_abort = false;
	}
}

/**
 * \brief RF215 Receiver Frame Start. Process the detection of valid PHY header
 * during frame reception after BBCn_IRQS.RXFS interrupt. This function is
 * called from IRQ.
 *
 * \param uc_trx_id TRX identifier
 * \param b_irq_rxfe BBCn_IRQS.RXFE is pending (true)
 */
static void _rf215_rx_start(uint8_t uc_trx_id, bool b_irq_rxfe)
{
	uint16_t us_psdu_len;
	bool b_header_valid;
	bool b_len_error;
	rf_phy_state_t uc_phy_state = gpx_phy_ctl[uc_trx_id].uc_phy_state;
	bool b_rx_abort = spx_rx_ctl[uc_trx_id].b_rx_abort;
	bool b_rx_state = (bool)((uc_phy_state >= RF_PHY_STATE_RX_LISTEN) && (uc_phy_state <= RF_PHY_STATE_RX_PAYLOAD));

	if ((b_rx_abort || (!b_rx_state)) && (!b_irq_rxfe)) {
		/* This interrupt comes from a previously aborted reception.
		 * Nothing to do. If RXFE is pending (RXFS and RXFE captured
		 * in the same IRQ), the reception was aborted, but there was
		 * time to finish it, so we should process it. */
		return;
	}

	/* Get frame length and check if it is valid */
	us_psdu_len = rf215_bbc_get_rx_len(uc_trx_id);
	if ((us_psdu_len > AT86RF_FCS_LEN) && (us_psdu_len <= AT86RF215_MAX_PSDU_LEN)) {
		/* Check if PHR is valid, update modulation specific parameters
		 * (FSK/OFDM), compute frame duration and configure Frame Buffer
		 * Level Interrupt (BBCn_FBLI) */
		rf215_rx_ctl_t *px_rx_ctl = (rf215_rx_ctl_t *)&spx_rx_ctl[uc_trx_id];
		at86rf_rx_ind_t *px_rx_ind = &px_rx_ctl->px_rx_ind[px_rx_ctl->uc_rx_ind_wr].x_ind;
		uint16_t us_pay_symbols = 0;
		b_header_valid = rf215_bbc_check_rx_params(uc_trx_id, us_psdu_len, px_rx_ind, &us_pay_symbols);
		gpx_phy_ctl[uc_trx_id].us_rx_pay_symbols = us_pay_symbols;
		b_len_error = false;

		if (b_header_valid) {
			/* Reset RX buffer offset */
			px_rx_ctl->us_rx_buf_offset = 0;

			if (px_rx_ctl->uc_rx_ind_pending == AT86RF215_NUM_RX_BUFFERS) {
				/* RX indication buffer full. The oldest RX
				 * indication won't be handled */
				gpx_phy_stats[uc_trx_id].ul_rx_ind_not_handled++;
				px_rx_ctl->uc_rx_ind_pending--;
				if (px_rx_ctl->uc_rx_ind_rd == (AT86RF215_NUM_RX_BUFFERS - 1)) {
					px_rx_ctl->uc_rx_ind_rd = 0;
				} else {
					px_rx_ctl->uc_rx_ind_rd++;
				}
			}

			/* Valid SHR + PHR received. Start receivig payload */
			if ((!b_rx_abort) && b_rx_state) {
				/* Only update PHY state if not aborted reception */
				gpx_phy_ctl[uc_trx_id].uc_phy_state = RF_PHY_STATE_RX_PAYLOAD;

				/* Turn on RX LED */
				gx_rf215_hal_wrp.rf_led(RF215_LED_RX, true);
			}
		}
	} else {
		/* Invalid length */
		b_header_valid = false;
		b_len_error = true;
	}

	if (!b_header_valid) {
		/* Invalid PHR received */
		if ((!b_rx_abort) && b_rx_state) {
			/* Abort ongoing RX and start listening again */
			rf215_trx_cmd_txprep(uc_trx_id);
			rf215_trx_rx_listen(uc_trx_id, false);
		}

		/* Update PHY statistics */
		gpx_phy_stats[uc_trx_id].ul_rx_err_total++;
		if (b_len_error) {
			gpx_phy_stats[uc_trx_id].ul_rx_err_bad_len++;
		} else {
			gpx_phy_stats[uc_trx_id].ul_rx_err_bad_format++;
		}
	}
}

/**
 * \brief RF215 RX Frame Buffer Level Event. Process the reception of partial RX
 * frame payload after BBCn_IRQS.FBLI interrupt. This function is called from IRQ.
 *
 * \param uc_trx_id TRX identifier
 */
static inline void _rf215_rx_fbli(uint8_t uc_trx_id)
{
	rf215_rx_ind_t *px_rx_ind;
	uint16_t us_buf_lvl;
	rf215_rx_ctl_t *px_rx_ctl;

	if (gpx_phy_ctl[uc_trx_id].uc_phy_state != RF_PHY_STATE_RX_PAYLOAD) {
		/* If state is not RX_PAYLOAD, FBLI and RXFE were captured in
		 * in the same IRQ and the full buffer was read in RXFE */
		return;
	}

	/* Get Frame Buffer Level to know how many bytes can be read */
	us_buf_lvl = rf215_bbc_get_buf_lvl(uc_trx_id);

	px_rx_ctl = (rf215_rx_ctl_t *)&spx_rx_ctl[uc_trx_id];
	px_rx_ind = &px_rx_ctl->px_rx_ind[px_rx_ctl->uc_rx_ind_wr];
	if ((us_buf_lvl > 0) && (us_buf_lvl <= px_rx_ind->x_ind.us_psdu_len)) {
		/* Read PSDU bytes already stored in RX Frame Buffer */
		uint8_t *puc_rx_data = px_rx_ind->puc_buf;
		rf215_bbc_read_rx_buf(uc_trx_id, puc_rx_data, us_buf_lvl, 0);
		px_rx_ctl->us_rx_buf_offset = us_buf_lvl;
	}
}

/**
 * \brief Process the end of completed PSDU reception. Read pending bytes from
 *  RX Frame Buffer, read Energy Detection Value (RRSI) and set pending RX
 * indication.
 *
 * \param uc_trx_id TRX identifier
 */
static void _rf215_rx_psdu_end(uint8_t uc_trx_id)
{
	uint8_t *puc_rx_data;
	rf215_rx_ind_t *px_rx_ind;
	uint16_t us_buf_offset;
	uint16_t us_psdu_len;
	uint8_t uc_rx_ind_wr;
	rf215_rx_ctl_t *px_rx_ctl = (rf215_rx_ctl_t *)&spx_rx_ctl[uc_trx_id];

	/* Get Energy Detection Value (RRSI) */
	uc_rx_ind_wr = px_rx_ctl->uc_rx_ind_wr;
	px_rx_ind = &px_rx_ctl->px_rx_ind[uc_rx_ind_wr];
	px_rx_ind->x_ind.sc_rssi_dBm = rf215_fe_get_edv(uc_trx_id);

	/* Read the remaining bytes of the received PSDU, including FCS (part of
	 * the payload can be read in FBLI interrupt) */
	us_buf_offset = px_rx_ctl->us_rx_buf_offset;
	puc_rx_data = &px_rx_ind->puc_buf[us_buf_offset];
	us_psdu_len = px_rx_ind->x_ind.us_psdu_len;
	if (us_psdu_len > us_buf_offset) {
		uint16_t us_pending_len = us_psdu_len - us_buf_offset;
		rf215_bbc_read_rx_buf(uc_trx_id, puc_rx_data, us_pending_len, us_buf_offset);
	}

	/* Set pending RX indication */
	px_rx_ctl->uc_rx_ind_pending++;

	/* Update index for next RX indication */
	if (uc_rx_ind_wr == (AT86RF215_NUM_RX_BUFFERS - 1)) {
		px_rx_ctl->uc_rx_ind_wr = 0;
	} else {
		px_rx_ctl->uc_rx_ind_wr = uc_rx_ind_wr + 1;
	}

	/* Check if there are programmed TX to cancel */
	rf215_tx_abort_by_rx(uc_trx_id);
}

/**
 * \brief RF215 Receiver Frame End Event. Process the end of successful frame
 * reception after BBCn_IRQS.RXFE interrupt. This function is called from IRQ.
 *
 * \param uc_trx_id TRX identifier
 */
static void _rf215_rx_frame_end(uint8_t uc_trx_id)
{
	at86rf_rx_ind_t *px_rx_ind;
	rf215_rx_ctl_t *px_rx_ctl;
	rf215_phy_stats_t *px_phy_stats = &gpx_phy_stats[uc_trx_id];
	rf_phy_state_t uc_phy_state = gpx_phy_ctl[uc_trx_id].uc_phy_state;

	/* Check RX abort flag */
	if ((!spx_rx_ctl[uc_trx_id].b_rx_abort) &&
			(uc_phy_state >= RF_PHY_STATE_RX_LISTEN) && (uc_phy_state <= RF_PHY_STATE_RX_PAYLOAD)) {
		/* Normal case: TRX switches automatically to TXPREP. Start
		 * listening by sending RX command */
		gpx_phy_ctl[uc_trx_id].uc_trx_state = RF215_RFn_STATE_RF_TXPREP;
		rf215_trx_rx_listen(uc_trx_id, false);
	} else {
		/* RX aborted just before, but it has actually finished. TRX
		 * state is not changed, but RX indication is filled. Undo PHY
		 * stats update */
		px_phy_stats->ul_rx_err_total--;
		px_phy_stats->ul_rx_err_aborted--;
	}

	/* If automatic FCS computation (AT86RF215_ENABLE_AUTO_FCS), RXFE
	 * interrupt means that FCS is valid */
	px_rx_ctl = (rf215_rx_ctl_t *)&spx_rx_ctl[uc_trx_id];
	px_rx_ind = &px_rx_ctl->px_rx_ind[px_rx_ctl->uc_rx_ind_wr].x_ind;
	px_rx_ind->b_fcs_ok = true;
	px_phy_stats->ul_rx_total++;
	px_phy_stats->ul_rx_total_bytes += px_rx_ind->us_psdu_len;

	/* Process the end of completed PSDU reception. Read pending bytes from
	 * RX Frame Buffer, read Energy Detection Value (RRSI) and set pending
	 * RX indication */
	_rf215_rx_psdu_end(uc_trx_id);
}

/**
 * \brief RF215 AGC Release Event. Process the end of reception in progress
 * (correct, invalid or cancelled) (AGC release) after BBCn_IRQS.AGCR interrupt.
 * This function is called from IRQ.
 *
 * \param uc_trx_id TRX identifier
 */
static void _rf215_rx_agcr(uint8_t uc_trx_id)
{
	rf215_phy_stats_t *px_phy_stats = &gpx_phy_stats[uc_trx_id];
	rf215_phy_ctl_t *px_phy_ctl = (rf215_phy_ctl_t *)&gpx_phy_ctl[uc_trx_id];

	/* Check current PHY state */
	if (gpx_phy_ctl[uc_trx_id].uc_phy_state == RF_PHY_STATE_RX_HEADER) {
		/* AGC released before RXFS interrupt */
		px_phy_stats->ul_rx_err_false_positive++;
		px_phy_stats->ul_rx_err_total++;

		/* Set PHY state to RX_LISTEN */
		gpx_phy_ctl[uc_trx_id].uc_phy_state = RF_PHY_STATE_RX_LISTEN;

		/* Check pending PHY configuration */
		if (px_phy_ctl->b_phy_cfg_pending) {
			rf215_bbc_set_phy_cfg(uc_trx_id, &px_phy_ctl->x_phy_cfg_pending, px_phy_ctl->us_chn_num_pending);
		}
	} else if (gpx_phy_ctl[uc_trx_id].uc_phy_state == RF_PHY_STATE_RX_PAYLOAD) {
#ifdef AT86RF215_ENABLE_AUTO_FCS
		at86rf_rx_ind_t *px_rx_ind;
		rf215_rx_ctl_t *px_rx_ctl;
		uint16_t us_buf_lvl;

		/* Get Frame Buffer Level */
		us_buf_lvl = rf215_bbc_get_buf_lvl(uc_trx_id);

		px_rx_ctl = (rf215_rx_ctl_t *)&spx_rx_ctl[uc_trx_id];
		px_rx_ind = &px_rx_ctl->px_rx_ind[px_rx_ctl->uc_rx_ind_wr].x_ind;
		if (us_buf_lvl == px_rx_ind->us_psdu_len) {
			/* PSDU received completely, but no RXFE interrupt. It
			 * means that the FCS is not valid */
			px_rx_ind->b_fcs_ok = false;
			px_phy_stats->ul_rx_err_bad_fcs_pay++;
			px_phy_stats->ul_rx_err_total++;

			/* Process the end of completed PSDU reception. Read
			 * pending bytes from RX Frame Buffer, read Energy
			 * Detection Value (RRSI) and set pending RX indication
			 */
			_rf215_rx_psdu_end(uc_trx_id);
		} else {
			/* AGC released before completing PSDU reception
			* (overrided by higher signal level detected) */
			px_phy_stats->ul_rx_override++;
		}

#else

		/* AGC released before completing PSDU reception (overrided by
		 * higher signal level detected) */
		px_phy_stats->ul_rx_override++;
#endif

		/* Set PHY state to RX_LISTEN */
		gpx_phy_ctl[uc_trx_id].uc_phy_state = RF_PHY_STATE_RX_LISTEN;

		/* Check pending PHY configuration */
		if (px_phy_ctl->b_phy_cfg_pending) {
			rf215_bbc_set_phy_cfg(uc_trx_id, &px_phy_ctl->x_phy_cfg_pending, px_phy_ctl->us_chn_num_pending);
		}
	}
}

/**
 * \brief RF215 Receiver Events. Process AGCH, AGCR, RXFS, RXFE interrupts in
 * the right order to avoid issues in case more than one is captured in the same
 * interrupt. This function is called from IRQ.
 *
 * \param uc_trx_id TRX identifier
 * \param uc_rx_flags BBCn_IRQS (only Receiver flags: BBC_IRQS_RX_FLAGS)
 */
void rf215_rx_event(uint8_t uc_trx_id, uint8_t uc_rx_flags)
{
	/* If PHY state is RX_HEADER, check RXFS interrupt */
	if ((gpx_phy_ctl[uc_trx_id].uc_phy_state == RF_PHY_STATE_RX_HEADER) && (uc_rx_flags & RF215_BBCn_IRQ_RXFS)) {
		_rf215_rx_start(uc_trx_id, false);
		uc_rx_flags &= ~RF215_BBCn_IRQ_RXFS;
	}

	/* If PHY state is RX_PAYLOAD, check RXFE/AGCR interrupt */
	if (gpx_phy_ctl[uc_trx_id].uc_phy_state == RF_PHY_STATE_RX_PAYLOAD) {
		if (uc_rx_flags & RF215_BBCn_IRQ_RXFE) {
			_rf215_rx_frame_end(uc_trx_id);
			uc_rx_flags &= ~RF215_BBCn_IRQ_RXFE;
		}

		if (uc_rx_flags & RF215_BBCn_IRQ_AGCR) {
			_rf215_rx_agcr(uc_trx_id);
			uc_rx_flags &= ~RF215_BBCn_IRQ_AGCR;
		}
	}

	/* Check remaining flags */
	if (uc_rx_flags != 0) {
		/* Check AGCH interrupt */
		if (uc_rx_flags & RF215_BBCn_IRQ_AGCH) {
			_rf215_rx_agch(uc_trx_id);
		}

		/* Check RXFS interrupt */
		if (uc_rx_flags & RF215_BBCn_IRQ_RXFS) {
			bool b_irq_rxfe = (bool)(uc_rx_flags & RF215_BBCn_IRQ_RXFE);
			_rf215_rx_start(uc_trx_id, b_irq_rxfe);
		}

		/* Check RXFE interrupt */
		if (uc_rx_flags & RF215_BBCn_IRQ_RXFE) {
			_rf215_rx_frame_end(uc_trx_id);
		}

		/* Check AGCR interrupt */
		if (uc_rx_flags & RF215_BBCn_IRQ_AGCR) {
			_rf215_rx_agcr(uc_trx_id);
		}

		/* Check FBLI interrupt */
		if (uc_rx_flags & RF215_BBCn_IRQ_FBLI) {
			_rf215_rx_fbli(uc_trx_id);
		}
	}

	/* Clear RX abort flag */
	spx_rx_ctl[uc_trx_id].b_rx_abort = false;
}

/**
 * \brief Abort ongoing RX. Update PHY statistics
 *
 * \param uc_trx_id TRX identifier
 */
void rf215_rx_abort(uint8_t uc_trx_id)
{
	bool b_ongoing_rx;

	/* Update PHY statistics */
	gpx_phy_stats[uc_trx_id].ul_rx_err_total++;
	gpx_phy_stats[uc_trx_id].ul_rx_err_aborted++;

	/* Set RX abort flag */
	spx_rx_ctl[uc_trx_id].b_rx_abort = true;

	/* Check if there is RX in progress in the another TRX */
	b_ongoing_rx = false;
	for (uint8_t uc_id = 0; uc_id < AT86RF_NUM_TRX; uc_id++) {
		if (uc_id != uc_trx_id) {
			rf_phy_state_t uc_phy_state = gpx_phy_ctl[uc_id].uc_phy_state;
			if ((uc_phy_state == RF_PHY_STATE_RX_HEADER) || (uc_phy_state == RF_PHY_STATE_RX_PAYLOAD)) {
				b_ongoing_rx = true;
			}
		}
	}

	/* Turn off RX LED (if there is not RX in progress) */
	if (!b_ongoing_rx) {
		gx_rf215_hal_wrp.rf_led(RF215_LED_RX, false);
	}
}

/**
 * \brief Function to check pending RX indication(s)
 *
 * \param uc_trx_id TRX identifier
 */
void rf215_rx_event_handler(uint8_t uc_trx_id)
{
	volatile rf215_rx_ctl_t *px_rx_ctl = &spx_rx_ctl[uc_trx_id];

	/* Check pending RX indication with SPI free (the data buffer
	 * read of the can be in progress by DMA) */
	while ((px_rx_ctl->uc_rx_ind_pending != 0) && (!gx_rf215_hal_wrp.rf_is_spi_busy())) {
		at86rf_rx_ind_t x_rx_ind;
		bool b_report_ind = false;

		/* Critical region to avoid modification of pending RX
		 * indications from interrupt */
		gx_rf215_hal_wrp.rf_enable_int(false);

		/* Check again pending RX indication (could be modified
		 * from IRQ, before disabling it) */
		if (px_rx_ctl->uc_rx_ind_pending != 0) {
			rf215_rx_ind_t *px_rx_ind;
			uint8_t uc_idx;

			/* Copy RX indication to static struct */
			uc_idx = px_rx_ctl->uc_rx_ind_rd;
			px_rx_ind = (rf215_rx_ind_t *)&px_rx_ctl->px_rx_ind[uc_idx];
			x_rx_ind = px_rx_ind->x_ind;

			/* Copy received PSDU to satic buffer */
			memcpy(spuc_rx_ind_data_buf, px_rx_ind->puc_buf, x_rx_ind.us_psdu_len);

			/* Update index for next RX indication */
			if (uc_idx == (AT86RF215_NUM_RX_BUFFERS - 1)) {
				px_rx_ctl->uc_rx_ind_rd = 0;
			} else {
				px_rx_ctl->uc_rx_ind_rd = uc_idx + 1;
			}

			/* Update number of pending RX indication */
			px_rx_ctl->uc_rx_ind_pending--;
			b_report_ind = true;
		}

		/* Leave critical region */
		gx_rf215_hal_wrp.rf_enable_int(true);

		if (gx_rf215_callbacks.rf_rx_ind_cb && b_report_ind) {
			/* Indicate RX indication to upper layer */
			gx_rf215_callbacks.rf_rx_ind_cb(uc_trx_id, &x_rx_ind);
		}

#ifdef AT86RF_ADDONS_ENABLE
		if (gx_rf215_callbacks.rf_addon_event_cb && b_report_ind) {
			uint8_t *puc_addon_buf;
			uint16_t us_addon_len;

			/* Get addon buffer from RX indication */
			puc_addon_buf = rf215_addon_stringify_ind(uc_trx_id, &x_rx_ind, &us_addon_len);

			/* Indicate addon event */
			gx_rf215_callbacks.rf_addon_event_cb(puc_addon_buf, us_addon_len);
		}
#endif
	}
}

/**
 * \brief Function to initialize RX controller module.
 *
 * \param uc_trx_id TRX identifier
 */
void rf215_rx_init(uint8_t uc_trx_id)
{
	rf215_rx_ctl_t *px_rx_ctl = (rf215_rx_ctl_t *)&spx_rx_ctl[uc_trx_id];
	px_rx_ctl->uc_rx_ind_pending = 0;
	px_rx_ctl->uc_rx_ind_wr = 0;
	px_rx_ctl->uc_rx_ind_rd = 0;
	px_rx_ctl->b_rx_abort = false;
	for (uint8_t uc_i = 0; uc_i < AT86RF215_NUM_RX_BUFFERS; uc_i++) {
		px_rx_ctl->px_rx_ind[uc_i].x_ind.puc_data = spuc_rx_ind_data_buf;
	}
}

#ifdef __cplusplus
}
#endif
