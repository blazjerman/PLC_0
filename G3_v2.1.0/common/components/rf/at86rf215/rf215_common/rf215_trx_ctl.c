/**
 *
 * \file
 *
 * \brief RF215 TRX control.
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
#include "rf215_trx_ctl.h"
#include "rf215_tx_rx.h"
#include "rf215_bbc.h"
#include "rf215_pll.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Prepare configuration by switching to TRXOFF state. If there is a
 * transmission or reception in progress, it is aborted. Update internal
 * variables. It is assumed that IRQ is disabled before calling this function.
 *
 * \param uc_trx_id TRX identifier
 *
 * \reval true TRX is in TRXOFF state and it is prepared for configuration
 * \reval false TRX is in RESET or SLEEP state and it is not changed
 */
bool rf215_trx_switch_trxoff(uint8_t uc_trx_id)
{
	bool b_trxoff;

	/* Check current TRX state */
	switch (gpx_phy_ctl[uc_trx_id].uc_trx_state) {
	case RF215_RFn_STATE_RF_RESET:
		/* Configuration can't be done in RESET state */
		b_trxoff = false;
		break;

	case RF215_RFn_STATE_RF_TRXOFF:
		/* Already in TRXOFF state */
		b_trxoff = true;
		break;

	default:
		/* TXPREP, RX or TX state: Needed to switch to TRXOFF state */
		rf215_trx_cmd_trxoff(uc_trx_id);
		b_trxoff = true;
		/* Check current PHY state and abort TX/RX if needed */
		rf215_tx_rx_check_aborts(uc_trx_id, false);
		break;
	}

	return b_trxoff;
}

/**
 * \brief Prepare configuration by switching to TXPREP state. If there is a
 * transmission or reception in progress, it is aborted. Update internal
 * variables. It is assumed that IRQ is disabled before calling this function.
 *
 * \param uc_trx_id TRX identifier
 *
 * \reval true TRX is in TXPREP state and it is prepared for configuration
 * \reval false TRX is in RESET or SLEEP state and it is not changed
 */
bool rf215_trx_switch_txprep(uint8_t uc_trx_id)
{
	bool b_txprep;

	/* Check current TRX state */
	switch (gpx_phy_ctl[uc_trx_id].uc_trx_state) {
	case RF215_RFn_STATE_RF_RESET:
		/* Configuration can't be done in RESET state */
		b_txprep = false;
		break;

	case RF215_RFn_STATE_RF_TXPREP:
		/* Already in TXPREP state */
		b_txprep = true;
		break;

	default:
		/* TRXOFF, RX or TX state: Needed to switch to TXPREP state */
		rf215_trx_cmd_txprep(uc_trx_id);
		b_txprep = true;
		/* Check current PHY state and abort TX/RX if needed */
		rf215_tx_rx_check_aborts(uc_trx_id, false);
		break;
	}

	return b_txprep;
}

/**
 * \brief Start listening after reset, RX/TX end or configuration. Update
 * internal variables. It is assumed that IRQ is disabled before calling this
 * function.
 *
 * \param uc_trx_id TRX identifier
 * \param b_restore_channel Flag to indicate if channel has to be restored
 */
void rf215_trx_rx_listen(uint8_t uc_trx_id, bool b_restore_channel)
{
	bool b_txprep_cmd;
	bool b_wait_pll;
	bool b_rx_cmd;
	rf215_phy_ctl_t *px_phy_ctl = (rf215_phy_ctl_t *)&gpx_phy_ctl[uc_trx_id];

	switch (px_phy_ctl->uc_trx_state) {
	case RF215_RFn_STATE_RF_TRXOFF:
	case RF215_RFn_STATE_RF_TX:
	case RF215_RFn_STATE_RF_TXPREP:
		/* TRXOFF/TXPREP/TX state: RX command is needed */
		b_rx_cmd = true;
		break;

	default:
		/* RX/RESET state: do nothing */
		b_rx_cmd = false;
		break;
	}

	if (b_rx_cmd) {
		gpx_phy_ctl[uc_trx_id].uc_phy_state = RF_PHY_STATE_RX_LISTEN;
	}

	if (px_phy_ctl->b_phy_cfg_pending) {
		/* Pending PHY configuration */
		rf215_bbc_set_phy_cfg(uc_trx_id, &px_phy_ctl->x_phy_cfg_pending, px_phy_ctl->us_chn_num_pending);
	} else if (b_restore_channel && (px_phy_ctl->us_chn_num_listen != px_phy_ctl->us_chn_num)) {
		/* Restore frequency channel for RX if needed */
		rf215_pll_set_chn_cfg(uc_trx_id, &gpx_phy_cfg[uc_trx_id].x_chn_cfg, px_phy_ctl->us_chn_num_listen);
		px_phy_ctl->b_trxrdy = false;
	}

	switch (px_phy_ctl->uc_trx_state) {
	case RF215_RFn_STATE_RF_TRXOFF:
	case RF215_RFn_STATE_RF_TX:
		/* TRXOFF/TX state: send TXPREP and RX commands */
		b_txprep_cmd = true;
		b_wait_pll = false;
		b_rx_cmd = true;
		break;

	case RF215_RFn_STATE_RF_TXPREP:
		/* TXPREP state: wait for PLL lock and send RX command */
		b_txprep_cmd = false;
		b_wait_pll = true;
		b_rx_cmd = true;
		break;

	default:
		/* RX/RESET state: do nothing */
		b_txprep_cmd = false;
		b_wait_pll = false;
		b_rx_cmd = false;
		break;
	}

	/* TXPREP command */
	if (b_txprep_cmd) {
		rf215_trx_cmd_txprep(uc_trx_id);
	}

	/* Wait until PLL has locked before sending RX command. The PLL lock
	 * status is indicated by RFn_PLL.LS subregister or by TRXRDY interrupt.
	 * We don't want to enable IRQ here (switch to RX state could be
	 * delayed), so polling of PLL.LS is done */
	if (b_wait_pll) {
		rf215_trx_wait_pll_lock(uc_trx_id);
	}

	/* RX command. Two commands can be written consecutively, command queue
	 * depth of one element */
	if (b_rx_cmd) {
		rf215_trx_cmd_rx(uc_trx_id);
		gpx_phy_ctl[uc_trx_id].uc_phy_state = RF_PHY_STATE_RX_LISTEN;
	}
}

/**
 * \brief Wait until PLL is locked. The PLL lock status is indicated by
 * RFn_PLL.LS subregister or by TRXRDY interrupt.
 *
 * \param uc_trx_id TRX identifier
 */
void rf215_trx_wait_pll_lock(uint8_t uc_trx_id)
{
	if (!gpx_phy_ctl[uc_trx_id].b_trxrdy) {
		/* TRXRDY not indicated yet. Do polling of RFn_PLL.LS */
		uint8_t uc_pll_ls;
		uint32_t ul_timeout = 1000;
		do {
			uc_pll_ls = rf215_spi_reg_read(RF215_ADDR_RFn_PLL(uc_trx_id));
			ul_timeout--;
		} while (((uc_pll_ls & RF215_RFn_PLL_LS) == 0) && (ul_timeout > 0));
	}
}

/**
 * \brief Update synchronization between TRX and PHY time. Used to convert the
 * the RF time read from TRX to PHY time (for TX confirm and RX indication)
 *
 * \param uc_trx_id TRX identifier
 */
void rf215_trx_upd_sync(uint8_t uc_trx_id)
{
	uint32_t ul_basepri_prev;
	uint32_t ul_sync_time_trx;
	uint8_t puc_bbc_cnt[4] = {0};

	/* Configure Counter in Free-Running Mode to read current TRX time */
	rf215_bbc_cnt_free_run(uc_trx_id);

	/* Enter critical region. Disable all interrupts except highest priority
	 * (<1: 0) to avoid delays in computations with current time */
	ul_basepri_prev = __get_BASEPRI();
	__set_BASEPRI(1 << (8 - __NVIC_PRIO_BITS));

	/* Read current TRX time and PHY time */
	rf215_spi_read_no_block(RF215_ADDR_BBCn_CNT0(uc_trx_id), puc_bbc_cnt, 4);
	gpx_phy_ctl[uc_trx_id].ul_sync_time_phy = gx_rf215_hal_wrp.timer_get();

	/* Leave critical region */
	__set_BASEPRI(ul_basepri_prev);

	/* Reconfigure Timestap Counter in Capture Mode (RX and TX) */
	rf215_bbc_cnt_capture(uc_trx_id);

	/* BBCn_CNT0..3 (4 bytes). BBCn_CNT0 is least significant byte */
	ul_sync_time_trx = (uint32_t)puc_bbc_cnt[0];
	ul_sync_time_trx += (uint32_t)puc_bbc_cnt[1] << 8;
	ul_sync_time_trx += (uint32_t)puc_bbc_cnt[2] << 16;
	ul_sync_time_trx += (uint32_t)puc_bbc_cnt[3] << 24;

	/* Adjust TRX time to compensate delay between TRX / PHY time reads */
	ul_sync_time_trx += RF215_SYNC_TIME_OFFSET_US_Q5;
	ul_sync_time_trx -= (uint16_t)guc_spi_byte_time_us_q5 << 1;
	gpx_phy_ctl[uc_trx_id].ul_sync_time_trx = ul_sync_time_trx;
}

#ifdef __cplusplus
}
#endif
