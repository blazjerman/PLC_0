/**
 *
 * \file
 *
 * \brief RF215 Frequency Synthesizer (PLL).
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
#include "rf215_pll.h"
#include "rf215_pll_defs.h"
#include "rf215_trx_ctl.h"
#include "rf215_tx_rx.h"
#include "rf215_fe.h"
#include "rf215_bbc.h"

#ifdef __cplusplus
extern "C" {
#endif

/** RF215 PLL constants (one per TRX) */
static const pll_const_t spx_pll_const[AT86RF_NUM_TRX] = RF215_PLL_CONST;

/* Current PLL frequency configuration parameters (one per TRX) */
static pll_params_t spx_pll_params[AT86RF_NUM_TRX];

/**
 * \brief Channel frequency computation from channel center frequency (F0),
 * channel spacing (CS) and channel number (CN). Values are in Hz.
 *
 * \param px_chn_cfg Pointer to channel configuration
 * \param us_chn_num Channel number
 *
 * \return Channel frequency in Hz
 */
static inline uint32_t _pll_chn_freq(at86rf_chn_cfg_t *px_chn_cfg, uint16_t us_chn_num)
{
	uint64_t ull_chn_freq;
	uint32_t ul_chn_freq;

	/* Compute channel frequency: F = F0 + (CS * CN) */
	ull_chn_freq = (uint64_t)px_chn_cfg->ul_chn_spa_hz * us_chn_num;
	ull_chn_freq += px_chn_cfg->ul_f0_hz;

	/* Saturate value to 32 bits */
	ul_chn_freq = min(ull_chn_freq, UINT32_MAX);
	return ul_chn_freq;
}

/**
 * \brief Check that frequency is within supported range(s) for the
 * corresponding TRX.
 *
 * \param uc_trx_id TRX identifier
 * \param ul_freq Frequency in Hz
 *
 * \return Frequency range index [0,1] or 0xFF if frequency is not within any
 * supported range
 */
static inline uint8_t _pll_freq_range(uint8_t uc_trx_id, uint32_t ul_freq)
{
	/* Pointer to constants struct for corresponding TRX */
	const pll_const_t *px_const = &spx_pll_const[uc_trx_id];

	for (uint8_t uc_rng = 0; uc_rng < px_const->uc_num_freq_rng; uc_rng++) {
		const pll_freq_rng_t *px_rng = &px_const->px_freq_rng[uc_rng];
		uint32_t ul_freq_min = px_rng->ul_freq_min;
		uint32_t ul_freq_max = px_rng->ul_freq_max;

		if ((ul_freq >= ul_freq_min) && (ul_freq <= ul_freq_max)) {
			/* Frequency is within this supported range */
			return uc_rng;
		}
	}

	/* Frequency is not within any supported range */
	return 0xFF;
}

/**
 * \brief Select RF215 channel mode (CNM.CM) for specific channel configuration
 * and TRX
 *
 * \param uc_trx_id TRX identifier
 * \param px_chn_cfg Pointer to channel configuration
 * \param us_chn_num Channel number
 * \param uc_rng Frequency range index [0,1]
 *
 * \return Channel mode (CNM.CM subregister value)
 */
static inline uint8_t _pll_chn_mode(uint8_t uc_trx_id, at86rf_chn_cfg_t *px_chn_cfg, uint16_t us_chn_num, uint8_t uc_rng)
{
	uint8_t uc_chn_mode;
	uint32_t ul_chn_spa = px_chn_cfg->ul_chn_spa_hz;

	if (((px_chn_cfg->ul_f0_hz % PLL_IEEE_FREQ_STEP_Hz) == 0) &&
			((ul_chn_spa % PLL_IEEE_FREQ_STEP_Hz) == 0) &&
			(ul_chn_spa <= PLL_IEEE_CHN_SPA_MAX_Hz) &&
			(us_chn_num <= PLL_IEEE_CHN_NUM_MAX)) {
		/* IEEE Mode: Frequencies (F0 and CS) multiples of 25kHz,
		 * channel number (CN) fits in 9 bits and channel spacing (CS)
		 * fits in 8 bits (in 25kHz steps) */
		uc_chn_mode = RF215_RFn_CNM_CM_IEEE;
	} else {
		/* Fine Mode. Get channel mode depending on frequency range */
		const pll_const_t *px_const = &spx_pll_const[uc_trx_id];
		if (uc_rng < px_const->uc_num_freq_rng) {
			uc_chn_mode = px_const->puc_fine_chn_mode[uc_rng];
		} else {
			uc_chn_mode = 0;
		}
	}

	return uc_chn_mode;
}

/**
 * \brief Compute channel frequency, range and mode of desired channel
 * configuration
 *
 * \param uc_trx_id TRX identifier
 * \param px_chn_cfg Pointer to channel configuration
 * \param us_chn_num Channel number
 * \param px_params Pointer to PLL parameters
 */
static void _pll_get_params(uint8_t uc_trx_id, at86rf_chn_cfg_t *px_chn_cfg, uint16_t us_chn_num, pll_params_t *px_params)
{
	px_params->ul_chn_freq = _pll_chn_freq(px_chn_cfg, us_chn_num);
	px_params->uc_freq_rng = _pll_freq_range(uc_trx_id, px_params->ul_chn_freq);
	px_params->uc_chn_mode = _pll_chn_mode(uc_trx_id, px_chn_cfg, us_chn_num, px_params->uc_freq_rng);
}

/**
 * \brief Function to check if channel configuration is correct.
 *
 * \param uc_trx_id TRX identifier
 * \px_chn_cfg Pointer to channel configuration
 * \param us_chn_num Channel number
 * \param px_params Pointer to PLL parameters
 *
 * \retval true Correct configuration
 * \retval false Wrong configuration or not supported
 */
static bool _pll_check_chn_cfg(uint8_t uc_trx_id, at86rf_chn_cfg_t *px_chn_cfg, uint16_t us_chn_num, pll_params_t *px_params)
{
	/* Check that channel number is within configured range */
	if (((us_chn_num < px_chn_cfg->us_chn_num_min) || (us_chn_num > px_chn_cfg->us_chn_num_max)) &&
			((us_chn_num < px_chn_cfg->us_chn_num_min2) || (us_chn_num > px_chn_cfg->us_chn_num_max2))) {
		return false;
	}

	/* Check that frequency is within allowed RF215 ranges */
	if (px_params->uc_freq_rng >= spx_pll_const[uc_trx_id].uc_num_freq_rng) {
		return false;
	}

	return true;
}

/**
 * \brief Compute PLL register values (RFn_CS, RFn_CCF0L, RFn_CCF0H, RFn_CNL,
 * RFn_CNM) depending on channel configuration and write them to RF215 (only if
 * values change from reset or previous configuration, except RFn_CNM which must
 * always be written).
 * IEEE-compliant Scheme and Fine Resolution Channel Scheme are supported.
 *
 * \param uc_trx_id TRX identifier
 */
static void _pll_set_chn_cfg(uint8_t uc_trx_id)
{
	uint32_t ul_f0;
	uint8_t *puc_pll_regs_prev;
	uint8_t puc_pll_regs_new[5];
	uint8_t uc_rfn_cnm;
	const pll_const_t *px_const = &spx_pll_const[uc_trx_id];
	pll_params_t *px_params = &spx_pll_params[uc_trx_id];

	/** CNM.CM: Channel Setting Mode */
	uc_rfn_cnm = px_params->uc_chn_mode;

	/* Pointer to PLL register values from reset or previous config */
	puc_pll_regs_prev = px_params->puc_pll_regs;

	if (uc_rfn_cnm == RF215_RFn_CNM_CM_IEEE) {
		/** IEEE-compliant Scheme (CNM.CM=0). Write 5 registers */
		uint16_t us_f0_25kHz;
		uint16_t us_chn_num;
		uint8_t uc_chn_spa_25kHz;
		at86rf_chn_cfg_t *px_chn_cfg = &gpx_phy_cfg[uc_trx_id].x_chn_cfg;

		/** RFn_CS – Channel Spacing. Convert to 25kHz steps */
		uc_chn_spa_25kHz = (uint8_t)(px_chn_cfg->ul_chn_spa_hz / PLL_IEEE_FREQ_STEP_Hz);
		puc_pll_regs_new[0] = uc_chn_spa_25kHz;

		/** Channel Center Frequency F0 */
		/* For RF24 there is a 1.5GHz offset */
		ul_f0 = px_chn_cfg->ul_f0_hz - px_const->ul_ieee_freq_offset;
		/* Convert to 25kHz steps */
		us_f0_25kHz = (uint16_t)(ul_f0 / PLL_IEEE_FREQ_STEP_Hz);
		/* RFn_CCF0L – Channel Center Frequency F0 Low Byte */
		puc_pll_regs_new[1] = (uint8_t)(us_f0_25kHz & 0xFF);
		/* RFn_CCF0H – Channel Center Frequency F0 High Byte */
		puc_pll_regs_new[2] = (uint8_t)(us_f0_25kHz >> 8);

		/** Channel number (<=511) */
		us_chn_num = gpx_phy_ctl[uc_trx_id].us_chn_num;
		/* RFn_CNL – Channel Number Low Byte */
		puc_pll_regs_new[3] = (uint8_t)(us_chn_num & 0xFF);
		/* RFn_CNM – CNM.CNH: Channel Number CN[8] */
		uc_rfn_cnm |= RF215_RFn_CNM_CNH(us_chn_num >> 8);

		/** RFn_CNM – Channel Mode and Channel Number High Bit */
		puc_pll_regs_new[4] = uc_rfn_cnm;
	} else {
		/** Fine Resolution Channel Scheme. Write 4 registers */
		uint32_t ul_freq_offset;
		uint32_t ul_freq_res;
		uint32_t ul_Nchannel;
		uint8_t uc_freq_rng;

		/* RFn_CS not used in Fine Resolution Channel Scheme */
		puc_pll_regs_new[0] = puc_pll_regs_prev[0];

		/** Channel Center Frequency F0 */
		/* Frequency offset depending on range */
		uc_freq_rng = px_params->uc_freq_rng;
		ul_freq_offset = px_const->pul_fine_freq_offset[uc_freq_rng];
		/* Remove offset from desired frequency */
		ul_f0 = px_params->ul_chn_freq - ul_freq_offset;
		/* Frequency resolution depending on range */
		ul_freq_res = px_const->pul_fine_freq_res[uc_freq_rng];
		/* Compute 24-bit Nchannel as (F0-offset)*2^16/resolution */
		ul_Nchannel = (uint32_t)div_round((uint64_t)ul_f0 << 16, ul_freq_res);
		/* RFn_CCF0L – Channel Center Frequency F0 Low Byte (middle byte) */
		puc_pll_regs_new[1] = (uint8_t)((ul_Nchannel >> 8) & 0xFF);
		/* RFn_CCF0H – Channel Center Frequency F0 High Byte (high byte) */
		puc_pll_regs_new[2] = (uint8_t)(ul_Nchannel >> 16);
		/* RFn_CNL – Channel Number Low Byte (low byte) */
		puc_pll_regs_new[3] = (uint8_t)(ul_Nchannel & 0xFF);

		/** RFn_CNM – Channel Mode and Channel Number High Bit */
		puc_pll_regs_new[4] = uc_rfn_cnm;

		/* Recompute channel frequency in Hz as ((Nchannel*resolution)/2^16)+offset */
		ul_f0 = (uint32_t)div_round((uint64_t)ul_Nchannel * ul_freq_res, 1 << 16);
		ul_f0 += ul_freq_offset;
		px_params->ul_chn_freq = ul_f0;
	}

	/* RFn_CNM must always be written */
	puc_pll_regs_prev[4] = puc_pll_regs_new[4] + 1;

	/* Write up to 5 registers: RFn_CS, RFn_CCF0L, RFn_CCF0H, RFn_CNL, RFn_CNM */
	/* New values automatically updated in static array */
	rf215_spi_write_upd(RF215_ADDR_RFn_CS(uc_trx_id), puc_pll_regs_new, puc_pll_regs_prev, 5);
}

/**
 * \brief Set channel frequency configuration. It's assumed that IRQ is
 * disabled before calling this function. Not needed to update fontend and
 * baseband core configurations here (done after calling this function). After
 * updating the channel configuration, the TRX will be TXPREP or TRXOFF (if not
 * in sleep state). It is assumed that configuration is correct.
 *
 * \param uc_trx_id TRX identifier
 * \param px_chn_cfg_new Pointer to new channel configuration
 * \param us_chn_num_new New channel number
 */
void rf215_pll_set_chn_cfg(uint8_t uc_trx_id, at86rf_chn_cfg_t *px_chn_cfg_new, uint16_t us_chn_num_new)
{
	pll_params_t x_params_new;
	at86rf_chn_cfg_t *px_chn_cfg;
	uint16_t us_chn_num;
	pll_params_t *px_params;
	bool b_upd_cfg;

	/* Check if channel configuration changes */
	px_chn_cfg = &gpx_phy_cfg[uc_trx_id].x_chn_cfg;
	us_chn_num = gpx_phy_ctl[uc_trx_id].us_chn_num;
	if ((px_chn_cfg->ul_f0_hz == px_chn_cfg_new->ul_f0_hz) &&
			(px_chn_cfg->ul_chn_spa_hz == px_chn_cfg_new->ul_chn_spa_hz) &&
			(us_chn_num == us_chn_num_new)) {
		return;
	}

	/* Compute channel frequency, range and mode of desired channel
	 * configuration */
	_pll_get_params(uc_trx_id, px_chn_cfg_new, us_chn_num_new, &x_params_new);

	/* The channel frequency must be configured in TRXOFF state, or TXPREP
	 * when changing within the same frequency range. If state is reset or
	 * sleep, configuration is saved and it will be updated after TRX
	 * Wake-up interrupt */
	px_params = &spx_pll_params[uc_trx_id];
	if (x_params_new.uc_freq_rng == px_params->uc_freq_rng) {
		/* Same frequency range: can be configured in TXPREP state */
		b_upd_cfg = rf215_trx_switch_txprep(uc_trx_id);
		if (b_upd_cfg) {
			/* Make sure that TXPREP has been reached */
			rf215_trx_wait_pll_lock(uc_trx_id);
		}
	} else {
		/* Different frequency range: must be configured in TRXOFF */
		b_upd_cfg = rf215_trx_switch_trxoff(uc_trx_id);
	}

	/* Save new PLL parameters and channel configuration */
	px_params->ul_chn_freq = x_params_new.ul_chn_freq;
	px_params->uc_chn_mode = x_params_new.uc_chn_mode;
	px_params->uc_freq_rng = x_params_new.uc_freq_rng;
	*px_chn_cfg = *px_chn_cfg_new;
	gpx_phy_ctl[uc_trx_id].us_chn_num = us_chn_num_new;

	if (b_upd_cfg) {
		/* Send new channel configuration to RF215 */
		_pll_set_chn_cfg(uc_trx_id);
	}
}

/**
 * \brief RF215 Frequency Synthesizer (PLL) TRX Reset Event. Initialize specific
 * registers of the corresponing transceiver PLL after TRX Reset (Wake-up
 * interrupt). This function is called from IRQ. Assumed that TRX is in TRXOFF
 * state, so register configuration is done without changing TRX state
 *
 * \param uc_trx_id TRX identifier
 */
void rf215_pll_trx_reset_event(uint8_t uc_trx_id)
{
	uint8_t *puc_regs;

	/* Initial values of PLL registers after reset */
	puc_regs = spx_pll_params[uc_trx_id].puc_pll_regs;
	*puc_regs++ = RF215_RFn_CS_Rst;
	*puc_regs++ = RF215_RFn_CF0L_Rst;
	*puc_regs++ = RF215_RFn_CF0H_Rst;
	*puc_regs++ = RF215_RFn_CNL_Rst;
	*puc_regs = RF215_RFn_CNM_Rst;

	/* Send channel configuration to RF215 */
	_pll_set_chn_cfg(uc_trx_id);
}

/**
 * \brief Function to check if channel configuration is correct.
 *
 * \param uc_trx_id TRX identifier
 * \px_chn_cfg Pointer to channel configuration
 * \param us_chn_num Channel number
 *
 * \retval true Correct configuration
 * \retval false Wrong configuration or not supported
 */
bool rf215_pll_check_chn_cfg(uint8_t uc_trx_id, at86rf_chn_cfg_t *px_chn_cfg, uint16_t us_chn_num)
{
	pll_params_t x_params;

	/* Compute channel frequency, range and mode of desired channel
	 * configuration */
	_pll_get_params(uc_trx_id, px_chn_cfg, us_chn_num, &x_params);

	/* Check channel configuration */
	return _pll_check_chn_cfg(uc_trx_id, px_chn_cfg, us_chn_num, &x_params);
}

/**
 * \brief Function to initialize Frequency Synthesizer (PLL) module.
 *
 * \param uc_trx_id TRX identifier
 * \px_chn_cfg Pointer to channel configuration
 * \param us_chn_num Channel number
 *
 * \retval true Correct configuration
 * \retval false Wrong configuration or not supported
 */
bool rf215_pll_init(uint8_t uc_trx_id, at86rf_chn_cfg_t *px_chn_cfg, uint16_t us_chn_num)
{
	/* Compute channel frequency, range and mode of initial channel
	 * configuration */
	_pll_get_params(uc_trx_id, px_chn_cfg, us_chn_num, &spx_pll_params[uc_trx_id]);

	/* Check channel configuration */
	return _pll_check_chn_cfg(uc_trx_id, px_chn_cfg, us_chn_num, &spx_pll_params[uc_trx_id]);
}

/**
 * \brief Get current channel frequency in Hz
 *
 * \param uc_trx_id TRX identifier
 *
 * \return Channel frequency in Hz
 */
uint32_t rf215_pll_get_chn_freq(uint8_t uc_trx_id)
{
	return spx_pll_params[uc_trx_id].ul_chn_freq;
}

/**
 * \brief Compute maximum frequency offset due to tolerance, depending on PHY
 * and channel configuration (single-sided clock)
 *
 * \param uc_trx_id TRX identifier
 *
 * \return Frequency offset in Hz
 */
uint32_t rf215_pll_get_fdelta(uint8_t uc_trx_id)
{
	uint64_t ull_freq_aux;
	uint32_t ul_fdelta;
	uint32_t ul_freq_tol_q45;
	at86rf_phy_cfg_t *px_phy_cfg = &gpx_phy_cfg[uc_trx_id];
	uint32_t ul_chn_freq = spx_pll_params[uc_trx_id].ul_chn_freq;

	if (px_phy_cfg->uc_phy_mod == AT86RF_PHY_MOD_FSK) {
		/* FSK PHY: T<=min(50*10^-6, T0*R*h*F0/R0/h0/F) */
		uint64_t ull_tol_aux_q45;
		uint16_t us_symrate_khz;
		uint8_t uc_r_r0;
		at86rf_fsk_cfg_t *px_fsk_cfg = &px_phy_cfg->u_mod_cfg.x_fsk;

		/* Get T0, different for RF09 and RF24 */
		ull_tol_aux_q45 = spx_pll_const[uc_trx_id].ul_fsk_tol_t0;

		/* Multiply by R/R0 (R: FSK symbol rate; R0 = 50kHz) */
		us_symrate_khz = gpus_fsk_symrate_khz[px_fsk_cfg->uc_symrate];
		uc_r_r0 = (uint8_t)(us_symrate_khz / PLL_DELTA_FSK_R0_kHz);
		ull_tol_aux_q45 *= uc_r_r0;

		/* Multiply by F0 (F0 = 915MHz) */
		ull_tol_aux_q45 *= PLL_DELTA_FSK_F0_Hz;

		/* Multiply by h/h0 (h: FSK modulation index; h0 = 1) */
		if (px_fsk_cfg->uc_modidx == AT86RF_FSK_MODIDX_0_5) {
			ull_tol_aux_q45 >>= 1;
		}

		/* Divide by F (F: carrier frequency) */
		ull_tol_aux_q45 /= ul_chn_freq;

		/* Maximum is 50 PPM */
		ull_tol_aux_q45 = min(ull_tol_aux_q45, PLL_DELTA_FSK_TMAX_Q45);
		ul_freq_tol_q45 = (uint32_t)ull_tol_aux_q45;
	} else {
		/* OFDM PHY: T<=20*10^-6 */
		ul_freq_tol_q45 = PLL_DELTA_OFDM_TMAX_Q45;
	}

	/* Compute Fdelta = T * F; T is in uQ0.45 */
	ull_freq_aux = (uint64_t)ul_chn_freq * ul_freq_tol_q45;
	ul_fdelta = (uint32_t)(ull_freq_aux >> 45);

	return ul_fdelta;
}

#ifdef __cplusplus
}
#endif
