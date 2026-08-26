/**
 *
 * \file
 *
 * \brief RF215 Transmitter and Receiver Frontend.
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
#include "rf215_fe.h"
#include "rf215_pll.h"
#include "rf215_bbc.h"
#include "rf215_trx_ctl.h"
#include "rf215_tx_rx.h"
#include "rf215_phy_defs.h"

/* System includes */
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif

static const uint32_t spul_cutoff_freq[12] = TXRXAFE_CUTOFF_Hz;

static const uint16_t spus_agc_tu_1[4][10] = RXFE_AGC_UPD_TIME_1;

static const uint16_t spus_agc_tu_0[4][10] = RXFE_AGC_UPD_TIME_0;

/** Transmitter / Receiver frontend parameters (one per TRX) */
static fe_params_t spx_fe_params[AT86RF_NUM_TRX];

/**
 * \brief Compute TXCUTC.LPFCUT subregister value so that
 * fLPFCUT >= ul_cutoff_freq or RXBWC.BW so that fBW >= 2*ul_cutoff_freq
 *
 * \param ul_cutoff_freq Desired cut-off frequency in Hz
 *
 * \return TXCUTC.LPFCUT/RXBWC.BW subregister value
 */
__always_inline static uint8_t _txrxafe_cutoff(uint32_t ul_cutoff_freq)
{
	uint8_t uc_i;

	/* Find value so that fLPFCUT >= ul_cutoff_freq */
	for (uc_i = 0; uc_i < 12; uc_i++) {
		if (ul_cutoff_freq <= spul_cutoff_freq[uc_i]) {
			/* Cut-off frequency found */
			return uc_i;
		}
	}

	/* Any cut-off frequency meets the requiremt */
	return 11;
}

/**
 * \brief Compute TX/RXDFE.RCUT subregister value so that fCUT >= ul_cutoff_freq
 *
 * \param ul_cutoff_freq Desired cut-off frequency in Hz
 * \param uc_sr TX/RXDFE.SR subregister value
 *
 * \return TX/RXDFE.RCUT subregister value
 */
__always_inline static uint8_t _txrxdfe_rcut(uint32_t ul_cutoff_freq, uint8_t uc_sr)
{
	uint32_t pul_rcut_hz[4];
	uint32_t ul_fs_hz;

	/* Get Transmitter digital frontend sampling frequency in Hz */
	ul_fs_hz = TXRXDFE_SR_MAX_Hz / uc_sr;

	/** fCUT = x*fs/2; fs is sampling rate; x is between 0.25 and 1 */
	/** Compute cut-off frequencies for fs used */
	/* Value 0: fCUT=0.25*fs/2 */
	pul_rcut_hz[0] = ul_fs_hz >> 3;
	/* Value 1: fCUT=0.375*fs/2 */
	pul_rcut_hz[1] = (ul_fs_hz * 3) >> 4;
	/* Value 2: fCUT=0.5*fs/2 */
	pul_rcut_hz[2] = ul_fs_hz >> 2;
	/* Value 3: fCUT=0.75*fs/2 */
	pul_rcut_hz[3] = (ul_fs_hz * 3) >> 3;
	/* Value 4: fCUT=1.00*fs/2 (bypassed) */

	/* Find value so that fCUT >= ul_cutoff_freq */
	for (uint8_t uc_i = 0; uc_i < 4; uc_i++) {
		if (ul_cutoff_freq <= pul_rcut_hz[uc_i]) {
			/* Cut-off frequency found */
			return uc_i;
		}
	}

	/* Any RCUT cut-off frequency meet the requiremt: bypass */
	return 4;
}

/**
 * \brief Adjust Energy Detection duration depending on AGC Update time
 * (minum ED duration) and RFn_EDD resolution (nearest higher value).
 *
 * \param uc_trx_id TRX identifier
 */
static inline void _fe_adjust_edd(uint8_t uc_trx_id)
{
	uint16_t us_agc_upd_time_us;
	uint16_t us_edd_us;

	/* Minimum EDD is AGC update time */
	us_agc_upd_time_us = RF215_TIME_US_Q5_TO_US(spx_fe_params[uc_trx_id].us_agc_upd_time_us_q5);
	us_edd_us = gpx_phy_cfg[uc_trx_id].x_cca_ed_cfg.us_duration_us;
	if (us_edd_us < us_agc_upd_time_us) {
		us_edd_us = us_agc_upd_time_us;
	}

	/* Adjust duration dependig on RFn_EDD.DTB resolution (ceiling) */
	if (us_edd_us <= (63 << 1)) {
		/* EDD.DTB: 2us */
		us_edd_us = ((us_edd_us + 1) >> 1) << 1;
	} else if (us_edd_us <= (63 << 3)) {
		/* EDD.DTB: 8us */
		us_edd_us = ((us_edd_us + 7) >> 3) << 3;
	} else if (us_edd_us <= (63 << 5)) {
		/* EDD.DTB: 32us */
		us_edd_us = ((us_edd_us + 31) >> 5) << 5;
	} else {
		/* EDD.DTB: 128us */
		us_edd_us = min(((us_edd_us + 127) >> 7) << 7, 63 << 7);
	}

	/* Update ED duration */
	gpx_phy_cfg[uc_trx_id].x_cca_ed_cfg.us_duration_us = us_edd_us;
}

/**
 * \brief Compute transmitter frontend register values (RFn_TXCUTC, RFn_TXDFE)
 * depending on PHY configuration.
 *
 * \param uc_trx_id TRX identifier
 * \param puc_txfe_regs_new Pointer to store reg values (2 bytes, local array)
 */
static void _txfe_regs(uint8_t uc_trx_id, uint8_t *puc_txfe_regs_new)
{
	uint32_t ul_lpfcut_freq;
	uint32_t ul_rcut_freq;
	uint8_t uc_lpfcut_val;
	uint8_t uc_rcut_val;
	uint8_t uc_sr_val;
	uint8_t uc_txdfe;
	uint8_t uc_txcutc;

	if (gpx_phy_cfg[uc_trx_id].uc_phy_mod == AT86RF_PHY_MOD_FSK) {
		const uint8_t puc_txdfe_sr[RF215_NUM_FSK_SYMRATES] = TXDFE_SR_FSK;
		const uint8_t puc_paramp[RF215_NUM_FSK_SYMRATES] = TXPA_PARAMP_FSK;
		uint32_t ul_fdev;
		at86rf_fsk_cfg_t *px_fsk_cfg;
		at86rf_fsk_symrate_t uc_symrate;

		/* Transmitter frontend configuration for FSK */
		px_fsk_cfg = &gpx_phy_cfg[uc_trx_id].u_mod_cfg.x_fsk;
		uc_symrate = px_fsk_cfg->uc_symrate;

		/* TXCUTC.PARAMP: Power Amplifier ramp time
		 * TXDFE.SR: Transmitter digital frontend sampling rate
		 * Depending on FSK symbol rate */
		uc_txcutc = puc_paramp[uc_symrate];
		uc_txdfe = puc_txdfe_sr[uc_symrate];
		uc_sr_val = uc_txdfe >> RF215_RFn_TXDFE_SR_Pos;

		/* TXDFE.DM: Direct modulation. It must be enabled for FSK (also
		 * in FSKDM.EN). It improves the modulation quality */
		uc_txdfe |= RF215_RFn_TXDFE_DM_EN;

		/* Reduce spurious transmissions as much as possible without
		 * attenuating transmitted carrier (fdev).
		 * fLPFCUT >= 3*fdev; fCUT >= 5*fdev */
		ul_fdev = rf215_bbc_fsk_fdev(px_fsk_cfg);
		ul_lpfcut_freq = ul_fdev * 3;
		ul_rcut_freq = ul_fdev * 5;
	} else { /* AT86RF_PHY_MOD_OFDM */
		const uint8_t puc_txdfe_sr[RF215_NUM_OFDM_OPTIONS] = TXDFE_SR_OFDM;
		uint32_t ul_bandwidth;
		at86rf_ofdm_opt_t uc_opt;

		/* Transmitter frontend configuration for OFDM */
		uc_opt = gpx_phy_cfg[uc_trx_id].u_mod_cfg.x_ofdm.uc_opt;

		/* TXCUTC.PARAMP: Power Amplifier ramp time
		 * TXDFE.SR: Transmitter digital frontend sampling rate
		 * Depending on FSK symbol rate */
		uc_txcutc = TXPA_PARAMP_OFDM;
		uc_txdfe = puc_txdfe_sr[uc_opt];
		uc_sr_val = uc_txdfe >> RF215_RFn_TXDFE_SR_Pos;

		/* Reduce spurious transmissions as much as possible without
		 * attenuating transmitted signal (BW / 2).
		 * fLPFCUT >= 0.875*BW; fCUT >= 0.875*BW */
		ul_bandwidth = gpul_ofdm_bw_hz[uc_opt];
		ul_lpfcut_freq = (ul_bandwidth * 7) >> 3;
		ul_rcut_freq = ul_lpfcut_freq;
	}

	/* TXCUTC.LPFCUT: TX analog frontend low pass filter cut-off freq */
	uc_lpfcut_val = _txrxafe_cutoff(ul_lpfcut_freq);
	uc_txcutc |= RF215_RFn_TXCUTC_LPFCUT(uc_lpfcut_val);

	/* TXDFE.RCUT: TX digital frontend pre-filter normalized cut-off freq */
	uc_rcut_val = _txrxdfe_rcut(ul_rcut_freq, uc_sr_val);
	uc_txdfe |= RF215_RFn_TXDFE_RCUT(uc_rcut_val);

	/* RFn_TXCUTC */
	puc_txfe_regs_new[0] = uc_txcutc;
	/* RFn_TXDFE */
	puc_txfe_regs_new[1] = uc_txdfe;
}

/**
 * \brief Write transmitter frontend registers (RFn_TXCUTC, RFn_TXDFE) to RF215
 * (only if values change from reset or previous configuration). Register values
 * computed with _txfe_regs(). Update TX frontend delay.
 *
 * \param uc_trx_id TRX identifier
 * \param puc_txfe_regs_new Pointer to register values (2 bytes, local array)
 */
static void _txfe_set_cfg(uint8_t uc_trx_id, uint8_t *puc_txfe_regs_new)
{
	uint16_t us_proc_delay_us_q5;
	uint8_t uc_rcut_val;
	uint8_t uc_sr_val;
	uint8_t uc_txdfe;

	/* Write up to 2 registers: RFn_TXCUTC, RFn_TXDFE. New values
	 * automatically updated in static array */
	rf215_spi_write_upd(RF215_ADDR_RFn_TXCUTC(uc_trx_id), puc_txfe_regs_new, spx_fe_params[uc_trx_id].puc_txfe_regs, 2);

	/* tx_start_delay: constant */
	us_proc_delay_us_q5 = TXFE_START_DELAY;

	/* tx_proc_delay: depending on TXDFE.SR and TXDFE.RCUT */
	uc_txdfe = puc_txfe_regs_new[1];
	uc_rcut_val = (uc_txdfe & RF215_RFn_TXDFE_RCUT_Msk) >> RF215_RFn_TXDFE_RCUT_Pos;
	uc_sr_val = (uc_txdfe & RF215_RFn_TXDFE_SR_Msk) >> RF215_RFn_TXDFE_SR_Pos;
	if (uc_rcut_val == 4) {
		const uint16_t pus_proc_delay[10] = TXDFE_PROC_DELAY;
		us_proc_delay_us_q5 += pus_proc_delay[uc_sr_val - 1];
	} else {
		const uint16_t pus_proc_delay[10] = TXDFE_PROC_DELAY_RCUT;
		us_proc_delay_us_q5 += pus_proc_delay[uc_sr_val - 1];
	}

	/* Transmitter Frontend delay: tx_start_delay + tx_proc_delay */
	rf215_tx_set_proc_delay(uc_trx_id, us_proc_delay_us_q5);
}

/**
 * \brief Compute receiver frontend register values (RFn_RXBWC, RFn_RXDFE,
 * RFn_AGCC, RFn_AGCS, RFn_EDC, RFn_EDD) depending on PHY configuration.
 *
 * \param uc_trx_id TRX identifier
 * \param puc_rxfe_regs_new Pointer to store reg values (7 bytes, local array)
 */
static void _rxfe_regs(uint8_t uc_trx_id, uint8_t *puc_rxfe_regs_new)
{
	uint32_t ul_fdelta;
	uint32_t ul_rxbwc_bw;
	uint32_t ul_rcut_freq;
	uint8_t uc_rxbwc;
	uint8_t uc_bw_val;
	uint8_t uc_rxdfe;
	uint8_t uc_rcut_val;
	uint8_t uc_sr_val;
	uint8_t uc_agcc;
	uint8_t uc_agcs;

	/* Receiver frontend configuration. Get maximum frequency offset due to
	 * tolerance, depending on PHY and channel configuration. Multiply by 2
	 * because offset is given for single-sided clock */
	ul_fdelta = rf215_pll_get_fdelta(uc_trx_id);
	ul_fdelta <<= 1;

	if (gpx_phy_cfg[uc_trx_id].uc_phy_mod == AT86RF_PHY_MOD_FSK) {
		const uint8_t puc_rxdfe_sr[RF215_NUM_FSK_SYMRATES] = RXDFE_SR_FSK;
		uint32_t ul_fdev;
		at86rf_fsk_cfg_t *px_fsk_cfg;

		/* Receiver frontend configuration for FSK */
		px_fsk_cfg = &gpx_phy_cfg[uc_trx_id].u_mod_cfg.x_fsk;

		/* Reduce as much noise / interference as possible, but making
		 * sure that in the worst case (frequency tolerance) the
		 * received signal is attenuated <=3dB and in the best case
		 * (perfect frequency alignment) the received signal is not
		 * attenuated.
		 * fBW >= 2*(max(2.5*fdev, fdev + fdelta))
		 * fCUT >= max(2.5*fdev, fdev + fdelta) */
		ul_fdev = rf215_bbc_fsk_fdev(px_fsk_cfg);
		ul_rxbwc_bw = max((ul_fdev * 5) >> 1, ul_fdev + ul_fdelta);
		ul_rcut_freq = ul_rxbwc_bw;

		/* RXDFE.SR: RX DFE sampling rate, depending on FSK symbol rate */
		uc_rxdfe = puc_rxdfe_sr[px_fsk_cfg->uc_symrate];
		uc_sr_val = uc_rxdfe >> RF215_RFn_RXDFE_SR_Pos;

		/* RFn_AGCC, RFn_AGCS */
		uc_agcc = RXFE_AGCC_FSK;
		uc_agcs = RXFE_AGCS_FSK;
	} else { /* AT86RF_PHY_MOD_OFDM */
		const uint8_t puc_rxdfe_sr[RF215_NUM_OFDM_OPTIONS] = RXDFE_SR_OFDM;
		uint32_t ul_bandwidth;
		uint32_t ul_bw_fdelta;
		at86rf_ofdm_opt_t uc_opt;

		/* Receiver frontend configuration for OFDM */
		uc_opt = gpx_phy_cfg[uc_trx_id].u_mod_cfg.x_ofdm.uc_opt;

		/* Reduce as much noise / interference as possible, but making
		 * sure that in the worst case (frequency tolerance) the
		 * received signal is attenuated <=3dB and in the best case
		 * (perfect frequency alignment) the received signal is not
		 * attenuated (BW / 2).
		 * fBW >= 2*(max(0.5625*BW, (BW/2) + fdelta))
		 * fCUT >= max(0.8125*BW, (BW/2) + fdelta) */
		ul_bandwidth = gpul_ofdm_bw_hz[uc_opt];
		ul_bw_fdelta = (ul_bandwidth >> 1) + ul_fdelta;
		ul_rxbwc_bw = max((ul_bandwidth * 9) >> 4, ul_bw_fdelta);
		ul_rcut_freq = max((ul_bandwidth * 13) >> 4, ul_bw_fdelta);

		/* RXDFE.SR: RX DFE sampling rate, depending on FSK symbol rate */
		uc_rxdfe = puc_rxdfe_sr[uc_opt];
		uc_sr_val = uc_rxdfe >> RF215_RFn_RXDFE_SR_Pos;

		/* RFn_AGCC, RFn_AGCS */
		uc_agcc = RXFE_AGCC_OFDM;
		if (rf215_bbc_get_ofdm_scp(uc_trx_id)) {
			uc_agcs = RXFE_AGCS_OFDM_SPC;
		} else {
			uc_agcs = RXFE_AGCS_OFDM;
		}
	}

	/* RXBWC.BW: Receiver analog frontend band pass filter bandwidth */
	uc_bw_val = _txrxafe_cutoff(ul_rxbwc_bw);
	uc_rxbwc = RF215_RFn_RXBWC_BW(uc_bw_val);

	/* RXBWC.IFS: Multiply fIF by 1.25 if fBW==fIF */
	if ((uc_rxbwc == RF215_RFn_RXBWC_BW250_IF250kHz) ||
			(uc_rxbwc == RF215_RFn_RXBWC_BW500_IF500kHz) ||
			(uc_rxbwc == RF215_RFn_RXBWC_BW1000_IF1000kHz) ||
			(uc_rxbwc == RF215_RFn_RXBWC_BW2000_IF2000kHz)) {
		uc_rxbwc |= RF215_RFn_RXBWC_IFS;
	}

	/* RFn_RXBWC */
	puc_rxfe_regs_new[0] = uc_rxbwc;

	/* RXDFE.RCUT: Transmitter digital frontend pre-filter normalized
	 * cut-off frequency */
	uc_rcut_val = _txrxdfe_rcut(ul_rcut_freq, uc_sr_val);
	uc_rxdfe |= RF215_RFn_RXDFE_RCUT(uc_rcut_val);

	/* RFn_RXDFE */
	puc_rxfe_regs_new[1] = uc_rxdfe;
	/* RFn_AGCC */
	puc_rxfe_regs_new[2] = uc_agcc;
	/* RFn_AGCS */
	puc_rxfe_regs_new[3] = uc_agcs;
	/* RFn_RSSI (read-only) */
	puc_rxfe_regs_new[4] = RF215_RFn_RSSI_Rst;
	/* RFn_EDC: Automatic Energy Detection Mode (triggered by reception) */
	puc_rxfe_regs_new[5] = RF215_RFn_EDC_EDM_AUTO;
	/*  RFn_EDD: Energy detection duration for automatic mode */
	puc_rxfe_regs_new[6] = RXFE_EDD_AUTO;
}

/**
 * \brief Write receiver frontend registers (RFn_RXBWC, RFn_RXDFE, RFn_AGCC,
 * RFn_AGCS, RFn_EDC, RFn_EDD) to RF215 (only if values change from reset or
 * previous configuration). Register values computed with _rxfe_regs(). Update
 * RX frontend delay and adjust CCA duration (minimum is AGC update time).
 *
 * \param uc_trx_id TRX identifier
 * \param puc_rxfe_regs_new Pointer to register values (7 bytes, local array)
 */
static void _rxfe_set_cfg(uint8_t uc_trx_id, uint8_t *puc_rxfe_regs_new)
{
	uint16_t us_rx_proc_delay_us_q5;
	uint16_t us_agc_upd_time_us_q5;
	uint8_t uc_rxdfe;
	uint8_t uc_sr_val;
	uint8_t uc_agcc;
	uint8_t uc_avgs_val;
	uint8_t uc_rcut_val;

	/* Write up to 7 registers: RFn_RXBWC, RFn_RXDFE, RFn_AGCC, RFn_AGCS,
	 * RFn_RSSI (dummy, read-only), RFn_EDC, RFn_EDD. New values
	 * automatically updated in static array */
	rf215_spi_write_upd(RF215_ADDR_RFn_RXBWC(uc_trx_id), puc_rxfe_regs_new, spx_fe_params[uc_trx_id].puc_rxfe_regs, 7);

	/* AGC update time, depending on RXDFE.SR, AGCC.AGCI and AGCC.AVGS */
	uc_rxdfe = puc_rxfe_regs_new[1];
	uc_sr_val = (uc_rxdfe & RF215_RFn_RXDFE_SR_Msk) >> RF215_RFn_RXDFE_SR_Pos;
	uc_rcut_val = (uc_rxdfe & RF215_RFn_RXDFE_RCUT_Msk) >> RF215_RFn_RXDFE_RCUT_Pos;
	uc_agcc = puc_rxfe_regs_new[2];
	uc_avgs_val = (uc_agcc & RF215_RFn_AGCC_AVGS_Msk) >> RF215_RFn_AGCC_AVGS_Pos;
	if (uc_agcc & RF215_RFn_AGCC_AGCI_x1) {
		us_agc_upd_time_us_q5 = spus_agc_tu_1[uc_avgs_val][uc_sr_val - 1];
	} else {
		us_agc_upd_time_us_q5 = spus_agc_tu_0[uc_avgs_val][uc_sr_val - 1];
	}

	/* RX processing delay, depending on RXDFE.SR and RXDFE.RCUT */
	us_rx_proc_delay_us_q5 = RXDFE_PROC_DELAY;
	if (uc_rcut_val != 4) {
		const uint16_t pus_proc_delay[10] = RXDFE_PROC_DELAY_RCUT;
		us_rx_proc_delay_us_q5 += pus_proc_delay[uc_sr_val - 1];
	}

	/* Store delays */
	spx_fe_params[uc_trx_id].us_agc_upd_time_us_q5 = us_agc_upd_time_us_q5;
	rf215_bbc_set_rx_proc_delay(uc_trx_id, us_rx_proc_delay_us_q5);
}

/**
 * \brief RF215 Transmitter and Receiver Frontend TRX Reset Event.
 * Initialize specific registers of the corresponing transceiver TX&RX frontend
 * after TRX Reset (Wake-up interrupt). This function is called from IRQ.
 * Assumed that TRX is in TRXOFF state, so register configuration is done
 * without changing TRX state.
 *
 * \param uc_trx_id TRX identifier
 */
void rf215_fe_trx_reset_event(uint8_t uc_trx_id)
{
	uint8_t puc_rxfe_regs_new[7];
	uint8_t puc_txfe_regs_new[2];
	uint8_t *puc_regs;

	/* Compute transmitter frontend register values
	 * (RFn_TXCUTC, RFn_TXDFE) */
	_txfe_regs(uc_trx_id, puc_txfe_regs_new);

	/* Initial values of transmitter frontend registers after reset */
	puc_regs = spx_fe_params[uc_trx_id].puc_txfe_regs;
	*puc_regs++ = RF215_RFn_TXCUTC_Rst;
	*puc_regs = RF215_RFn_TXDFE_Rst;

	/* Write TX frontend registers (only if values change from reset) */
	_txfe_set_cfg(uc_trx_id, puc_txfe_regs_new);

	/* RFn_PAC is reset with maximum transmitter output power */
	spx_fe_params[uc_trx_id].uc_txpwr_att = 0;

	/* Compute receiver frontend register values (RFn_RXBWC, RFn_RXDFE,
	 * RFn_AGCC, RFn_AGCS, RFn_EDC, RFn_EDD) */
	_rxfe_regs(uc_trx_id, puc_rxfe_regs_new);

	/* Initial values of receiver frontend registers after reset */
	puc_regs = spx_fe_params[uc_trx_id].puc_rxfe_regs;
	*puc_regs++ = RF215_RFn_RXBWC_Rst;
	*puc_regs++ = RF215_RFn_RXDFE_Rst;
	*puc_regs++ = RF215_RFn_AGCC_Rst;
	*puc_regs++ = RF215_RFn_AGCS_Rst;
	*puc_regs++ = RF215_RFn_RSSI_Rst;
	*puc_regs++ = RF215_RFn_EDC_Rst;
	*puc_regs = RF215_RFn_EDD_Rst;

	/* Write RX frontend registers (only if values change from reset) */
	_rxfe_set_cfg(uc_trx_id, puc_rxfe_regs_new);

	/* Adjust CCA duration (minimum is AGC update time) */
	_fe_adjust_edd(uc_trx_id);
}

/**
 * \brief Update RF215 Transmitter and Receiver Frontend configuration after
 * PHY configuration change. It's assumed that IRQ is disabled before calling
 * this function. It's not assumed that TRX is in TRXOFF state (it could be
 * TXPREP if only channel frequency changes), so the TRX state will be changed
 * to TRXOFF if it is needed to update registers.
 *
 * \param uc_trx_id TRX identifier
 */
void rf215_fe_upd_phy_cfg(uint8_t uc_trx_id)
{
	uint8_t puc_rxfe_regs_new[7];
	uint8_t puc_txfe_regs_new[2];

	/* Compute transmitter frontend register values
	 * (RFn_TXCUTC, RFn_TXDFE) */
	_txfe_regs(uc_trx_id, puc_txfe_regs_new);

	/* Compute receiver frontend register values (RFn_RXBWC, RFn_RXDFE,
	 * RFn_AGCC, RFn_AGCS, RFn_EDC, RFn_EDD) */
	_rxfe_regs(uc_trx_id, puc_rxfe_regs_new);

	/* Switch to TRXOFF and write TX/RX frontend registers
	 * (only if register values change from previous configuration) */
	if ((memcmp(puc_txfe_regs_new, spx_fe_params[uc_trx_id].puc_txfe_regs, 2) != 0) ||
			(memcmp(puc_rxfe_regs_new, spx_fe_params[uc_trx_id].puc_rxfe_regs, 7) != 0)) {
		bool b_upd_cfg = rf215_trx_switch_trxoff(uc_trx_id);
		if (b_upd_cfg) {
			_txfe_set_cfg(uc_trx_id, puc_txfe_regs_new);
			_rxfe_set_cfg(uc_trx_id, puc_rxfe_regs_new);
		}
	}

	/* Adjust CCA duration (minimum is AGC update time) */
	_fe_adjust_edd(uc_trx_id);
}

/**
 * \brief Update Transmitter Amplifier Power (RFn_PAC). If register(s) need to
 * be updated, the TRX state will be changed to TRXOFF and TX/RX in progress
 * will be aborted. It is assumed that IRQ is disabled before calling this
 * function.
 *
 * \param uc_trx_id TRX identifier
 * \param uc_txpwr_att TX power attenuation in dB
 * \param px_mod_params Pointer to frame parameters
 */
void rf215_fe_set_txpwr(uint8_t uc_trx_id, uint8_t uc_txpwr_att, at86rf_mod_frame_params_t *px_mod_params)
{
	if (gpx_phy_cfg[uc_trx_id].uc_phy_mod == AT86RF_PHY_MOD_OFDM) {
		/* For OFDM, there is a minimum TX power attenuation to comply
		 * with EVM requeriments */
		uint8_t puc_txpwr_att_min[RF215_NUM_OFDM_MCS] = TXPA_TX_ATT_MIN_OFDM;
		uint8_t uc_att_min = puc_txpwr_att_min[px_mod_params->x_ofdm.uc_mcs];
		uint16_t us_txpwr_att_new = uc_txpwr_att + uc_att_min;
		uc_txpwr_att = (uint8_t)min(us_txpwr_att_new, 31);
	}

	/* Maximum attenuation is 31 dB */
	uc_txpwr_att = min(uc_txpwr_att, 31);

	if (uc_txpwr_att != spx_fe_params[uc_trx_id].uc_txpwr_att) {
		/* TX power attenuation changes: register needs to be written */
		/* TRX must be in TRXOFF state to configure RFn_PAC register */
		rf215_trx_switch_trxoff(uc_trx_id);
		spx_fe_params[uc_trx_id].uc_txpwr_att = uc_txpwr_att;

		/* Update RFn_PAC register */
		rf215_spi_reg_write(RF215_ADDR_RFn_PAC(uc_trx_id), TXPA_PAC(uc_txpwr_att));
	}
}

#ifdef __cplusplus
}
#endif
