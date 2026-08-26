/**
 *
 * \file
 *
 * \brief RF215 Baseband Core.
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
#include "rf215_bbc.h"
#include "rf215_bbc_defs.h"
#include "rf215_phy_defs.h"
#include "rf215_trx_ctl.h"
#include "rf215_tx_rx.h"
#include "rf215_fe.h"
#include "rf215_pll.h"

/* System includes */
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif

/** FSK symbol rate in kHz */
const uint16_t gpus_fsk_symrate_khz[RF215_NUM_FSK_SYMRATES] = FSK_SYMRATE_kHz;
/** FSK symbol rate in Hz */
const uint32_t gpul_fsk_symrate_hz[RF215_NUM_FSK_SYMRATES] = FSK_SYMRATE_Hz;
/** OFDM bandwidth in Hz */
const uint32_t gpul_ofdm_bw_hz[RF215_NUM_OFDM_OPTIONS] = OFDM_BANDWIDTH_Hz;
/** OFDM PHR symbols */
static const uint8_t spuc_ofdm_phr_symb[2][RF215_NUM_OFDM_OPTIONS] = OFDM_PHR_SYMBOLS;
/** OFDM number of data subcarriers */
static const uint8_t spuc_ofdm_data_carriers[RF215_NUM_OFDM_OPTIONS] = OFDM_DATA_CARRIERS;
/** OFDM frequency spreading repetition factor shift */
static const uint8_t spuc_ofdm_rep_fact_shift[RF215_NUM_OFDM_MCS] = OFDM_REP_FACT_SHIFT;
/** OFDM bits per subcarrier shift */
static const uint8_t spuc_ofdm_bits_carrier_shift[RF215_NUM_OFDM_MCS] = OFDM_BITS_CARRIER_SHIFT;
/** FSK sensitivity in dBm */
static const int8_t spsc_fsk_sensitivity[RF215_NUM_FSK_SYMRATES] = FSK_SENSITIVITY_dBm;
/** OFDM sensitivity in dBm */
static const int8_t spsc_ofdm_sensitivity[RF215_NUM_OFDM_OPTIONS] = OFDM_SENSITIVITY_dBm;

/** Baseband controller parameters (one per TRX) */
static bbc_params_t spx_bbc_params[AT86RF_NUM_TRX];

/**
 * \brief Check FSK configuration parameters
 *
 * \param px_fsk_cfg Pointer to FSK configuration
 *
 * \retval true Correct configuration
 * \retval false Wrong configuration or not supported
 */
__always_inline static bool _bbc_check_fsk_cfg(at86rf_fsk_cfg_t *px_fsk_cfg)
{
	at86rf_fsk_modidx_t uc_modidx_min;
	at86rf_fsk_modord_t uc_modord;

	if (px_fsk_cfg->uc_symrate > AT86RF_FSK_SYMRATE_400kHz) {
		/* Invalid symbol rate */
		return false;
	}

	uc_modord = px_fsk_cfg->uc_modord;
	if (uc_modord == AT86RF_FSK_MODORD_2FSK) {
		/* 2-FSK: 1.0 and 0.5 modulation indexes supported */
		uc_modidx_min = AT86RF_FSK_MODIDX_0_5;
	} else if (uc_modord == AT86RF_FSK_MODORD_4FSK) {
		/* 4-FSK: 1.0 modulation index supported */
		uc_modidx_min = AT86RF_FSK_MODIDX_1_0;
	} else {
		/* Invalid modulation order */
		return false;
	}

	if (px_fsk_cfg->uc_modidx <= uc_modidx_min) {
		/* Valid configuration */
		return true;
	} else {
		/* Invalid modulation order */
		return false;
	}
}

/**
 * \brief Check OFDM configuration parameters
 *
 * \param px_ofdm_cfg Pointer to OFDM configuration
 *
 * \retval true Correct configuration
 * \retval false Wrong configuration or not supported
 */
__always_inline static bool _bbc_check_ofdm_cfg(at86rf_ofdm_cfg_t *px_ofdm_cfg)
{
	if ((px_ofdm_cfg->uc_opt > AT86RF_OFDM_OPT_4) || (px_ofdm_cfg->uc_itlv > AT86RF_OFDM_INTERLEAVING_1)) {
		/* Invalid option / interleaving mode */
		return false;
	} else {
		/* Valid configuration */
		return true;
	}
}

/**
 * \brief Check PHY configuration parameters
 *
 * \param px_phy_cfg Pointer to PHY configuration
 *
 * \retval true Correct configuration
 * \retval false Wrong configuration or not supported
 */
__always_inline static bool _bbc_check_phy_cfg(at86rf_phy_cfg_t *px_phy_cfg)
{
	at86rf_phy_mod_t uc_phy_mod = px_phy_cfg->uc_phy_mod;
	at86rf_mod_cfg_t *px_mod_cfg = &px_phy_cfg->u_mod_cfg;

	if (uc_phy_mod == AT86RF_PHY_MOD_FSK) {
		return _bbc_check_fsk_cfg(&px_mod_cfg->x_fsk);
	} else if (uc_phy_mod == AT86RF_PHY_MOD_OFDM) {
		return _bbc_check_ofdm_cfg(&px_mod_cfg->x_ofdm);
	} else {
		/* PHY modulation not supported */
		return false;
	}
}

/**
 * \brief Compute number of FSK symbols per octet
 *
 * \param px_fsk_cfg Pointer to FSK configuration
 * \param px_fsk_params Pointer to FSK frame parameters
 *
 * \return Number of FSK symbols per octet
 */
__always_inline static uint8_t _bbc_symbols_octet_fsk(at86rf_fsk_cfg_t *px_fsk_cfg,
		at86rf_fsk_frame_params_t *px_fsk_params)
{
	uint8_t uc_symbols_octet = 8;

	if (px_fsk_params->uc_fec_enabled == AT86RF_FSK_FEC_ON) {
		/* If FEC enabled, the number of symbols per octet is doubled */
		uc_symbols_octet <<= 1;
	}

	if (px_fsk_cfg->uc_modord == AT86RF_FSK_MODORD_4FSK) {
		/* In 4-FSK, one symbol carries 2 bits */
		uc_symbols_octet >>= 1;
	}

	return uc_symbols_octet;
}

/**
 * \brief Compute frame duration for FSK
 *
 * \param[in] px_fsk_cfg Pointer to FSK configuration
 * \param[in] px_fsk_params Pointer to FSK frame parameters
 * \param[in] us_psdu_len PSDU length in bytes (octets), including FCS
 * \param[out] pus_pay_symbols Pointer to number of payload FSK symbols
 *
 * \return Frame duration for FSK in us
 */
__always_inline static uint32_t _bbc_frame_duration_fsk(at86rf_fsk_cfg_t *px_fsk_cfg,
		at86rf_fsk_frame_params_t *px_fsk_params, uint16_t us_psdu_len,
		uint16_t *pus_pay_symbols)
{
	uint32_t ul_symbols_aux;
	uint32_t ul_frame_duration;
	uint16_t us_symb_rate_khz;
	uint16_t us_total_symbols;
	uint16_t us_pay_symbols;
	uint8_t uc_shr_symbols;
	uint8_t uc_phr_symbols;
	uint8_t uc_symbols_octet;
	uint8_t uc_tail_pad_octets;

	/* SHR (Preamble + SFD): Preamble fixed to 8 octets, SFD 2 octets. 8
	 * symbols per octect (not affected by modulation order and FEC) */
	uc_shr_symbols = 10 << 3;

	/* Compute number of FSK symbols per octet (PHR + Payload) */
	uc_symbols_octet = _bbc_symbols_octet_fsk(px_fsk_cfg, px_fsk_params);

	/* PHR: 2 octets. Symbols depends on modulation order and FEC */
	uc_phr_symbols = uc_symbols_octet << 1;

	/* Payload: PSDU + tail + padding */
	if (px_fsk_params->uc_fec_enabled == AT86RF_FSK_FEC_ON) {
		/* FEC enabled: 3 tail bits added. 5 or 13 padding bits added */
		if (us_psdu_len & 1) {
			/* PSDU length odd: 3 tail bits + 5 padding bits */
			uc_tail_pad_octets = 1;
		} else {
			/* PSDU length even: 3 tail bits + 13 padding bits */
			uc_tail_pad_octets = 2;
		}
	} else {
		/* FEC disabled: No tail / padding added */
		uc_tail_pad_octets = 0;
	}

	/* Payload: PSDU + tail + padding */
	us_pay_symbols = (us_psdu_len + uc_tail_pad_octets) * uc_symbols_octet;

	/* Symbol rate in kHz */
	us_symb_rate_khz = gpus_fsk_symrate_khz[px_fsk_cfg->uc_symrate];

	/* Compute frame duration in us */
	us_total_symbols = uc_shr_symbols + uc_phr_symbols + us_pay_symbols;
	ul_symbols_aux = (uint32_t)us_total_symbols * 1000;
	ul_frame_duration = (uint32_t)(ul_symbols_aux / us_symb_rate_khz);

	*pus_pay_symbols = us_pay_symbols;
	return ul_frame_duration;
}

/**
 * \brief Compute frame duration for OFDM
 *
 * \param[in] px_ofdm_cfg Pointer to OFDM configuration
 * \param[in] px_ofdm_params Pointer to OFDM frame parameters
 * \param[in] us_psdu_len PSDU length in bytes (octets), including FCS
 * \param[out] pus_pay_symbols Pointer to number of payload OFDM symbols
 *
 * \return Frame duration for FSK in us
 */
__always_inline static uint32_t _bbc_frame_duration_ofdm(at86rf_ofdm_cfg_t *px_ofdm_cfg,
		at86rf_ofdm_frame_params_t *px_ofdm_params,
		uint16_t us_psdu_len, uint16_t *pus_pay_symbols)
{
	uint32_t ul_pay_bits_total;
	uint32_t ul_frame_duration;
	uint16_t us_total_symbols;
	uint16_t us_pay_symbols;
	uint16_t us_bits_per_symb;
	uint8_t uc_shr_symbols;
	uint8_t uc_phr_symbols;
	uint8_t uc_rep_fact_shift;
	uint8_t uc_data_carriers;
	uint8_t uc_bits_carrier_shift;
	at86rf_ofdm_mcs_t uc_mcs;
	at86rf_ofdm_itlv_mode_t uc_itlv;
	at86rf_ofdm_opt_t uc_opt;

	/* SHR (STF + LTF): 6 OFDM symbols (not affected by any parameter) */
	uc_shr_symbols = 6;

	/* PHR: Number of symbols depending on option and phyOfdmInterleaving */
	uc_itlv = px_ofdm_cfg->uc_itlv;
	uc_opt = px_ofdm_cfg->uc_opt;
	uc_phr_symbols = spuc_ofdm_phr_symb[uc_itlv][uc_opt];

	/* PHY Payload: PSDU bytes to bits */
	ul_pay_bits_total = us_psdu_len << 3;
	/* PHY Payload: Add PPDU Tail (6 bits for convolutional encoder) */
	ul_pay_bits_total += 6;

	/* PHY Payload: Convolutional encoder (rate depends on MCS) */
	uc_mcs = px_ofdm_params->uc_mcs;
	if ((uc_mcs == AT86RF_OFDM_MCS_4) || (uc_mcs == AT86RF_OFDM_MCS_6)) {
		/* Rate 3/4: Multiply by 4/3 */
		ul_pay_bits_total = div_ceil(ul_pay_bits_total << 2, 3);
	} else {
		/* Rate 1/2: Multiply by 2 */
		ul_pay_bits_total <<= 1;
	}

	/* Frequency spreading repetition factor depending on MCS */
	uc_rep_fact_shift = spuc_ofdm_rep_fact_shift[uc_mcs];
	ul_pay_bits_total <<= uc_rep_fact_shift;

	/* Compute number of OFDM payload symbols */
	uc_data_carriers = spuc_ofdm_data_carriers[uc_opt];
	uc_bits_carrier_shift = spuc_ofdm_bits_carrier_shift[uc_mcs];
	us_bits_per_symb = (uint16_t)uc_data_carriers << uc_bits_carrier_shift;
	us_pay_symbols = (uint16_t)div_ceil(ul_pay_bits_total, us_bits_per_symb);

	/* If phyOfdmInterleaving=1, symbols multiple of repetition factor */
	if (uc_itlv == AT86RF_OFDM_INTERLEAVING_1) {
		us_pay_symbols = div_ceil(us_pay_symbols, 1 << uc_rep_fact_shift) << uc_rep_fact_shift;
	}

	/* Compute frame duration in us */
	us_total_symbols = uc_shr_symbols + uc_phr_symbols + us_pay_symbols;
	ul_frame_duration = (uint32_t)us_total_symbols * OFDM_SYMB_DURATION_US;

	*pus_pay_symbols = us_pay_symbols;
	return ul_frame_duration;
}

/**
 * \brief Compute frame duration
 *
 * \param[in] px_phy_cfg Pointer to PHY configuration
 * \param[in] px_mod_params Pointer to frame parameters
 * \param[in] us_psdu_len PSDU length in bytes (octets), including FCS
 * \param[out] pus_pay_symbols Pointer to number of payload symbols
 *
 * \return Frame duration in us
 */
static uint32_t _bbc_frame_duration(at86rf_phy_cfg_t *px_phy_cfg,
		at86rf_mod_frame_params_t *px_mod_params, uint16_t us_psdu_len,
		uint16_t *pus_pay_symbols)
{
	uint32_t ul_frame_us;
	at86rf_mod_cfg_t *px_mod_cfg = &px_phy_cfg->u_mod_cfg;

	if (px_phy_cfg->uc_phy_mod == AT86RF_PHY_MOD_FSK) {
		ul_frame_us = _bbc_frame_duration_fsk(&px_mod_cfg->x_fsk,
				&px_mod_params->x_fsk, us_psdu_len,
				pus_pay_symbols);
	} else { /* AT86RF_PHY_MOD_OFDM */
		ul_frame_us = _bbc_frame_duration_ofdm(&px_mod_cfg->x_ofdm,
				&px_mod_params->x_ofdm, us_psdu_len,
				pus_pay_symbols);
	}

	return ul_frame_us;
}

/**
 * \brief Compute delay from RX frame start to RXFS IRQ for FSK
 *
 * \param px_fsk_cfg Pointer to FSK configuration
 * \param px_fsk_params Pointer to FSK frame parameters
 *
 * \return RXFS delay in us [uQ14.5] for FSK
 */
__always_inline static uint32_t _bbc_rxfs_delay_fsk_us_q5(at86rf_fsk_cfg_t *px_fsk_cfg,
		at86rf_fsk_frame_params_t *px_fsk_params)
{
	uint32_t ul_symbols_aux;
	uint32_t ul_rxfs_us_q5;
	uint16_t us_symb_rate_khz;
	uint16_t us_total_symbols;
	uint8_t uc_shr_symbols;
	uint8_t uc_phr_symbols;
	uint8_t uc_symbols_octet;
	uint8_t uc_fec_delay_q5;

	/* SHR (Preamble + SFD): Preamble fixed to 8 octets, SFD 2 octets. 8
	 * symbols per octect (not affected by modulation order and FEC) */
	uc_shr_symbols = 10 << 3;

	/* Compute number of FSK symbols per octet (PHR + Payload) */
	uc_symbols_octet = _bbc_symbols_octet_fsk(px_fsk_cfg, px_fsk_params);

	/* PHR: 2 octets. Symbols depends on modulation order and FEC */
	uc_phr_symbols = uc_symbols_octet << 1;

	/* Symbol rate in kHz */
	us_symb_rate_khz = gpus_fsk_symrate_khz[px_fsk_cfg->uc_symrate];

	/* Header (SHR + PHR) symbols */
	us_total_symbols = uc_shr_symbols + uc_phr_symbols;

	/* Additional delay if FEC is enabled */
	if (px_fsk_params->uc_fec_enabled == AT86RF_FSK_FEC_ON) {
		const uint8_t puc_fec_delay[RF215_NUM_FSK_SYMRATES] = BBC_RX_BB_DELAY_FSK_FEC;
		uc_fec_delay_q5 = puc_fec_delay[px_fsk_cfg->uc_symrate];
		us_total_symbols += 34;
	} else {
		uc_fec_delay_q5 = 0;
	}

	/* Compute RXFS delay in us [uQ14.5] */
	ul_symbols_aux = (uint32_t)us_total_symbols * (1000 << 5);
	ul_rxfs_us_q5 = (uint32_t)(ul_symbols_aux / us_symb_rate_khz);
	ul_rxfs_us_q5 += uc_fec_delay_q5;

	return ul_rxfs_us_q5;
}

/**
 * \brief Compute delay from RX frame start to RXFS IRQ for OFDM
 *
 * \param px_ofdm_cfg Pointer to OFDM configuration
 *
 * \return RXFS delay in us [uQ14.5] for OFDM
 */
__always_inline static uint32_t _bbc_rxfs_delay_ofdm_us_q5(at86rf_ofdm_cfg_t *px_ofdm_cfg)
{
	uint32_t ul_rxfs_us_q5;
	uint8_t uc_total_symbols;
	uint8_t uc_shr_symbols;
	uint8_t uc_phr_symbols;
	at86rf_ofdm_itlv_mode_t uc_itlv;
	at86rf_ofdm_opt_t uc_opt;

	/* SHR (STF + LTF): 6 OFDM symbols (not affected by any parameter) */
	uc_shr_symbols = 6;

	/* PHR: Number of symbols depending on option and phyOfdmInterleaving */
	uc_itlv = px_ofdm_cfg->uc_itlv;
	uc_opt = px_ofdm_cfg->uc_opt;
	uc_phr_symbols = spuc_ofdm_phr_symb[uc_itlv][uc_opt];

	/* Compute RXFS delay in us [uQ14.5] */
	uc_total_symbols = uc_shr_symbols + uc_phr_symbols;
	ul_rxfs_us_q5 = (uint32_t)uc_total_symbols * (OFDM_SYMB_DURATION_US << 5);

	return ul_rxfs_us_q5;
}

/**
 * \brief Compute delay from RX frame start to RXFS IRQ
 *
 * \param px_phy_cfg Pointer to PHY configuration
 * \param px_mod_params Pointer to frame parameters
 *
 * \return RXFS delay in us [uQ14.5]
 */
__always_inline static uint32_t _bbc_rxfs_delay_us_q5(at86rf_phy_cfg_t *px_phy_cfg,
		at86rf_mod_frame_params_t *px_mod_params)
{
	uint32_t ul_rxfs_us_q5;
	at86rf_mod_cfg_t *px_mod_cfg = &px_phy_cfg->u_mod_cfg;

	if (px_phy_cfg->uc_phy_mod == AT86RF_PHY_MOD_FSK) {
		ul_rxfs_us_q5 = _bbc_rxfs_delay_fsk_us_q5(&px_mod_cfg->x_fsk,
				&px_mod_params->x_fsk);
	} else { /* AT86RF_PHY_MOD_OFDM */
		ul_rxfs_us_q5 = _bbc_rxfs_delay_ofdm_us_q5(&px_mod_cfg->x_ofdm);
	}

	return ul_rxfs_us_q5;
}

/**
 * \brief Set Frame Buffer Level Interrupt in BBC frame buffer registers
 *
 * \param uc_trx_id TRX identifier
 * \param us_lvl Frame Buffer Level Interrupt
 */
__always_inline static void _bbc_set_buf_lvl_int(uint8_t uc_trx_id, uint16_t us_lvl)
{
	uint8_t puc_fbli_regs[2];

	/* BBCn_FBLIL � TX Frame Length Low Byte */
	puc_fbli_regs[0] = (uint8_t)(us_lvl & 0xFF);
	/* BBCn_FBLIH � TX Frame Length High Byte */
	puc_fbli_regs[1] = (uint8_t)RF215_BBCn_FBLIH_FBLIH(us_lvl >> 8);

	/* Write 2 registers: BBCn_FBLIL, BBCn_FBLIH */
	rf215_spi_write(RF215_ADDR_BBCn_FBLIL(uc_trx_id), puc_fbli_regs, 2);
}

/**
 * \brief Compute parameters for FBLI computation (FSK)
 *
 * \param[in] px_fsk_cfg Pointer to FSK configuration
 * \param[in] px_fsk_params Pointer to FSK frame parameters
 * \param[out] pus_buff_block_bits Frame Buffer updated block size in bits
 * \param[out] puc_fec_k FEC constraint length K (0 if FEC disabled)
 *
 * \return Duration of one octet for FSK in us [uQ9.5]
 */
__always_inline static uint16_t _bbc_fbli_params_fsk(at86rf_fsk_cfg_t *px_fsk_cfg,
		at86rf_fsk_frame_params_t *px_fsk_params,
		uint16_t *pus_buff_block_bits, uint8_t *puc_fec_k)
{
	uint32_t ul_symbols_aux;
	uint16_t us_symb_rate_khz;
	uint16_t us_octet_us_q5;
	uint8_t uc_symbols_octet;

	if (px_fsk_params->uc_fec_enabled == AT86RF_FSK_FEC_OFF) {
		/* FEC (and interleaver) disabled */
		*pus_buff_block_bits = 8;
		*puc_fec_k = 0;
	} else {
		/* FEC (and interleaver) enabled (K = 4). Interleaver works
		 * with blocks of 16 code-symbols, so Frame Buffer is updated
		 * every 2 octets */
		*pus_buff_block_bits = 16;
		*puc_fec_k = 4;
	}

	/* Symbol rate in kHz */
	us_symb_rate_khz = gpus_fsk_symrate_khz[px_fsk_cfg->uc_symrate];

	/* Compute number of FSK symbols per octet */
	uc_symbols_octet = _bbc_symbols_octet_fsk(px_fsk_cfg, px_fsk_params);

	/* Compute octet duration in us [uQ9.5] */
	ul_symbols_aux = (uint32_t)uc_symbols_octet * (1000 << 5);
	us_octet_us_q5 = (uint16_t)div_round(ul_symbols_aux, us_symb_rate_khz);

	return us_octet_us_q5;
}

/**
 * \brief Compute parameters for FBLI computation (OFDM)
 *
 * \param[in] px_ofdm_cfg Pointer to OFDM configuration
 * \param[in] px_ofdm_params Pointer to OFDM frame parameters
 * \param[out] pus_buff_block_bits Frame Buffer updated block size in bits
 * \param[out] puc_fec_k FEC constraint length K
 *
 * \return Duration of one octet for OFDM in us [uQ9.5]
 */
__always_inline static uint16_t _bbc_fbli_params_ofdm(at86rf_ofdm_cfg_t *px_ofdm_cfg,
		at86rf_ofdm_frame_params_t *px_ofdm_params,
		uint16_t *pus_buff_block_bits, uint8_t *puc_fec_k)
{
	uint32_t ul_num_aux;
	uint16_t us_bits_symb;
	uint16_t us_bits_block;
	uint16_t us_den_aux;
	uint16_t us_octet_us_q5;
	uint8_t uc_rep_fact_shift;
	uint8_t uc_data_carriers;
	uint8_t uc_bits_carrier_shift;
	at86rf_ofdm_mcs_t uc_mcs;
	at86rf_ofdm_opt_t uc_opt;

	/* Parameters depending on bandwith option and MCS */
	uc_opt = px_ofdm_cfg->uc_opt;
	uc_mcs = px_ofdm_params->uc_mcs;
	uc_data_carriers = spuc_ofdm_data_carriers[uc_opt];
	uc_bits_carrier_shift = spuc_ofdm_bits_carrier_shift[uc_mcs];
	uc_rep_fact_shift = spuc_ofdm_rep_fact_shift[uc_mcs];

	/* Total bits (coded, with repetition) of 1 OFDM symbol */
	us_bits_symb = (uint16_t)uc_data_carriers << uc_bits_carrier_shift;

	/* OctectDuration = (8*Rep*SymbDuration)/(CC_rate*DataCarr*BitsCarr) */
	ul_num_aux = OFDM_SYMB_DURATION_US << (uc_rep_fact_shift + 8);
	us_den_aux = us_bits_symb;

	/* Number of bits (uncoded, with repetition) of 1 OFDM symbol */
	if ((uc_mcs == AT86RF_OFDM_MCS_4) || (uc_mcs == AT86RF_OFDM_MCS_6)) {
		/* FEC rate 3/4: Multiply by 3/4 */
		us_bits_block = (us_bits_symb * 3) >> 2;
		ul_num_aux <<= 2;
		us_den_aux *= 3;
	} else {
		/* FEC rate 1/2: Divide by 2 */
		us_bits_block = us_bits_symb >> 1;
		ul_num_aux <<= 1;
	}

	/* FEC (and interleaver) always enabled in OFDM (K = 7). Interleaver
	 * works with blocks of 1 OFDM symbol (phyOfdmInterleaving=0) or FreqRep
	 * OFDM symbols (phyOfdmInterleaving=1) */
	*puc_fec_k = 7;
	if (px_ofdm_cfg->uc_itlv == AT86RF_OFDM_INTERLEAVING_0) {
		*pus_buff_block_bits = us_bits_block >> uc_rep_fact_shift;
	} else {
		*pus_buff_block_bits = us_bits_block;
	}

	/* Compute octet duration in us [uQ9.5] */
	us_octet_us_q5 = (uint16_t)div_round(ul_num_aux, us_den_aux);

	return us_octet_us_q5;
}

/**
 * \brief Compute parameters for FBLI computation (RF octect duration, Frame
 * Buffer update block size and Convolutional encoder (FEC) constraint length K)
 *
 * \param[in] px_phy_cfg Pointer to PHY configuration
 * \param[in] px_mod_params Pointer to frame parameters
 * \param[out] pus_buff_block_bits Frame Buffer updated block size in bits
 * \param[out] puc_fec_k Convolutional encoder (FEC) constraint length K
 *
 * \return Duration of one octet for OFDM in us [uQ9.5]
 */
__always_inline static uint16_t _bbc_fbli_params(at86rf_phy_cfg_t *px_phy_cfg,
		at86rf_mod_frame_params_t *px_mod_params,
		uint16_t *pus_buff_block_bits, uint8_t *puc_fec_k)
{
	uint16_t us_octet_us_q5;
	at86rf_mod_cfg_t *px_mod_cfg = &px_phy_cfg->u_mod_cfg;

	if (px_phy_cfg->uc_phy_mod == AT86RF_PHY_MOD_FSK) {
		us_octet_us_q5 = _bbc_fbli_params_fsk(&px_mod_cfg->x_fsk, &px_mod_params->x_fsk,
				pus_buff_block_bits, puc_fec_k);
	} else { /* AT86RF_PHY_MOD_OFDM */
		us_octet_us_q5 = _bbc_fbli_params_ofdm(&px_mod_cfg->x_ofdm, &px_mod_params->x_ofdm,
				pus_buff_block_bits, puc_fec_k);
	}

	return us_octet_us_q5;
}

/**
 * \brief Compute number of bytes to read in Frame Buffer Level Interrupt,
 * optimized to read less bytes as possible in RXFE interrupt, depending on the
 * RF frame parameters and SPI interface
 *
 * \param px_phy_cfg Pointer to PHY configuration
 * \param px_mod_params Pointer to frame parameters
 * \param us_psdu_len PSDU length in bytes (octets), including FCS
 *
 * \return Frame Buffer Level Interrupt for FSK
 */
__always_inline static uint16_t _bbc_fbli(at86rf_phy_cfg_t *px_phy_cfg,
		at86rf_mod_frame_params_t *px_mod_params, uint16_t us_psdu_len)
{
	uint32_t ul_pay_duration_us_q5;
	uint16_t us_margin_us_q5;
	uint16_t us_rf_octet_us_q5;
	uint16_t us_buf_lvl_int;
	uint16_t us_buff_block_bits;
	uint16_t us_pay_total_octects;
	uint16_t us_pay_total_bits;
	uint8_t uc_spi_byte_us_q5;
	uint8_t uc_fec_k;
	uint8_t uc_fec_flush_bits;
	uint8_t uc_fec_delay_bits;

	/* Compute RF duration of 1 octect, Frame Buffer update block size and
	 * FEC constraint length K */
	us_rf_octet_us_q5 = _bbc_fbli_params(px_phy_cfg, px_mod_params,
			&us_buff_block_bits, &uc_fec_k);

	if (uc_fec_k >= 2) {
		/* FEC enabled. FlushingBits=K-1; Delay=2^(K-2) */
		uc_fec_flush_bits = uc_fec_k - 1;
		uc_fec_delay_bits = 1 << (uc_fec_k - 2);
	} else {
		/* FEC disabled */
		uc_fec_flush_bits = 0;
		uc_fec_delay_bits = 0;
	}

	/* Compute total payload octects, including FEC flushing and padding */
	us_pay_total_bits = (us_psdu_len << 3) + uc_fec_flush_bits;
	us_pay_total_bits = div_ceil(us_pay_total_bits, us_buff_block_bits) * us_buff_block_bits;
	us_pay_total_octects = div8_ceil(us_pay_total_bits);

	/* Time of SPI transaction for 1 byte */
	uc_spi_byte_us_q5 = guc_spi_byte_time_us_q5;

	/* 500 us of margin between FBLI and RXFE in case there is another
	 * interrupt in between to avoid delaying RXFE interrupt */
	us_margin_us_q5 = (500 << 5);

	/* Time of SPI transactions before reading buffer in FBLI interrupt:
	 * 12 bytes (6 IRQS, 4 FBL, 2 SPI header) */
	us_margin_us_q5 += (uint16_t)uc_spi_byte_us_q5 * 12;

	/* Payload duration: PsduLen * TimeRfOctect */
	ul_pay_duration_us_q5 = (uint32_t)us_rf_octet_us_q5 * us_pay_total_octects;

	if (ul_pay_duration_us_q5 > us_margin_us_q5) {
		uint32_t ul_numerator;
		uint16_t us_denominator;
		uint16_t us_bits_fbli;
		uint16_t us_num_blocks;

		/* Compute number of bytes for FBLI interrupt as:
		 * (PayDuration - Margin) / (TimeByteSPI + TimeRfOctect) */
		ul_numerator = ul_pay_duration_us_q5 - us_margin_us_q5;
		us_denominator = us_rf_octet_us_q5 + uc_spi_byte_us_q5;
		us_buf_lvl_int = (uint16_t)(ul_numerator / us_denominator);

		/* Force FBLI to be multiple of block size (flooring) */
		us_bits_fbli = us_buf_lvl_int << 3;
		us_num_blocks = us_bits_fbli / us_buff_block_bits;
		us_bits_fbli = (us_num_blocks * us_buff_block_bits);

		/* Remove FEC "delay" */
		if (us_bits_fbli > uc_fec_delay_bits) {
			us_bits_fbli -= uc_fec_delay_bits;
		} else {
			us_bits_fbli = 0;
		}

		/* Convert to octects (flooring) */
		us_buf_lvl_int = us_bits_fbli >> 3;

		/* FBLI interrupt is triggered when Frame Buffer Level is higher
		 * than BBCn_FBLI */
		if (us_buf_lvl_int > 0) {
			us_buf_lvl_int -= 1;
		} else {
			/* FBLI interrupt not used: set to maximum */
			us_buf_lvl_int = 2047;
		}
	} else {
		/* Payload is too short: Read all PSDU in RXFE. FBLI interrupt
		 * not used, set to maximum */
		us_buf_lvl_int = 2047;
	}

	return us_buf_lvl_int;
}

/**
 * \brief Compute BBCn_FSKC0 register value
 *
 * \param px_fsk_cfg Pointer to FSK configuration
 *
 * \return BBCn_FSKC0 value
 */
__always_inline static uint8_t _bbc_fskc0(at86rf_fsk_cfg_t *px_fsk_cfg)
{
	const uint8_t puc_midx[RF215_NUM_FSK_MODIDX] = BBC_FSKC0_MIDX;
	uint8_t uc_fskc0;

	/* FSKC0.MIDX/MIDXS: FSK modulation index */
	uc_fskc0 = puc_midx[px_fsk_cfg->uc_modidx];

	/* FSKC0.MORD: FSK modulation order */
	uc_fskc0 |= RF215_BBCn_FSKC0_MORD(px_fsk_cfg->uc_modord);

	/* FSKC0.BT: FSK Bandwidth Time Product = 2.0. SUN FSK does not specify
	 * GFSK modulator, so set to the maximum value. Furthermore, we are
	 * using direct modulation with preemphasis and BT is ignored in that
	 * case (datasheet 6.10.4.2) */
	uc_fskc0 |= RF215_BBCn_FSKC0_BT_2_0;

	return uc_fskc0;
}

/**
 * \brief Compute BBCn_FSKC3 register value
 *
 * \param px_fsk_cfg Pointer to FSK configuration
 *
 * \return BBCn_FSKC3 value
 */
__always_inline static uint8_t _bbc_fskc3(at86rf_fsk_cfg_t *px_fsk_cfg)
{
	uint8_t uc_fskc3;
	uint8_t uc_pdt;

	/* FSKC3.SFDT: SFD Detection Threshold default value 8 */
	uc_fskc3 = RF215_BBCn_FSKC3_SFDT(8);

	/* FSKC3.PDT: Preamble Detection Threshold */
	if ((px_fsk_cfg->uc_modidx == AT86RF_FSK_MODIDX_0_5) && (px_fsk_cfg->uc_symrate >= AT86RF_FSK_SYMRATE_150kHz)) {
		/* Higher threshold to avoid false detections (spurious) */
		uc_pdt = 6;
	} else {
		/* Default value 5 */
		uc_pdt = 5;
	}

	uc_fskc3 |= RF215_BBCn_FSKC3_PDT(uc_pdt);

	return uc_fskc3;
}

/**
 * \brief Compute FSK configuration register values (BBCn_FSKC0/1/3, BBCn_FSKDM,
 * BBCn_FSKPE0/1/2).
 *
 * \param uc_trx_id TRX identifier
 * \param puc_fsk_regs_new Pointer to store reg values (4 bytes, local array)
 * \param puc_dm_pe_regs_new Pointer to store reg values (4 bytes, local array)
 */
static void _bbc_fsk_cfg_regs(uint8_t uc_trx_id, uint8_t *puc_fsk_regs_new, uint8_t *puc_dm_pe_regs_new)
{
	const uint8_t puc_fsk_dm_pe[RF215_NUM_FSK_SYMRATES][4] = BBC_FSK_DM_PE;
	at86rf_fsk_cfg_t *px_fsk_cfg;
	at86rf_fsk_symrate_t uc_symrate;

	/* BBCn_FSKC0 */
	px_fsk_cfg = &gpx_phy_cfg[uc_trx_id].u_mod_cfg.x_fsk;
	puc_fsk_regs_new[0] = _bbc_fskc0(px_fsk_cfg);

	/* BBCn_FSKC1
	 * SRATE: FSK symbol rate
	 * FSKPLH=0: Preamble length high byte (fixed to 8 octets)
	 * FI=0: Sign of FSK deviation frequency not inverted */
	uc_symrate = px_fsk_cfg->uc_symrate;
	puc_fsk_regs_new[1] = RF215_BBCn_FSKC1_SRATE(uc_symrate);

	/* BBCn_FSKC2: Not needed to modify default register value.
	 * FECIE=1: FEC interleaving enabled (NRNSC)
	 * FECS=0: FEC scheme NRNSC (phyFskFecScheme = 0)
	 * PRI=0: Preamble frequency deviation not inverted
	 * MSE=0: Mode Switch disabled
	 * RXPTO=0: Resinchronization if correlation is below threshold
	 * RXO=2: Receiver restarted by >18dB stronger frame
	 * PDTM=0: Enable only if preamble length is <8 (fixed to 8) */
	puc_fsk_regs_new[2] = RF215_BBCn_FSKC2_Rst;

	/* BBCn_FSKC3 */
	puc_fsk_regs_new[3] = _bbc_fskc3(px_fsk_cfg);

	/* BBCn_FSKC4: Not needed to modify default register value.
	 * CSFD0=0: SFD0 used for uncoded (FEC disabled) IEEE mode
	 * CSFD1=2: SFD1 used for coded (FEC enabled) IEEE mode
	 * RAWRBIT=1: RAW mode not used
	 * SFD32=0: Dual SFD mode
	 * SFDQ=0: Use soft decisions for SFD search */

	/* BBCn_FSKPLL: Not needed to modify default register value.
	 * FSKPLL=8: Preamble length fixed to 8 octets */

	/* BBCn_FSKSFD0L/H: Not needed to modify default register value.
	 * FSKSFD0L=0x09; FSKSFD0H=0x72: 802.15.4 SUN FSK SFD value for
	 * uncoded format (phySunFskSfd=0)  */

	/* BBCn_FSKSFD1L/H: Not needed to modify default register value.
	 * FSKSFD1L=0xF6; FSKSFD1H=0x72: 802.15.4 SUN FSK SFD value for
	 * coded format (phySunFskSfd=0)  */

	/* BBCn_FSKPHRTX: Not needed to modify default register value.
	 * RB1/2=0: PHR reserved bits set to 0
	 * DW=1: Data withening enabled
	 * SFD=0: SFD0 used for TX (uncoded, FEC disabled) */

	/* Copy BBCn_FSKDM, BBCn_FSKPE0/1/2 retister values */
	memcpy(puc_dm_pe_regs_new, puc_fsk_dm_pe[uc_symrate], 4);
}

/**
 * \brief Write FSK configuration registers (BBCn_FSKC0/1/3, BBCn_FSKDM,
 * BBCn_FSKPE0/1/2) to RF215 (only if values change from reset or
 * previous configuration). Register values computed with _bbc_fsk_cfg_regs().
 * Update baseband core processing delays.
 *
 * \param uc_trx_id TRX identifier
 * \param puc_fsk_regs_new Pointer to store reg values (4 bytes, local array)
 * \param puc_dm_pe_regs_new Pointer to store reg values (4 bytes, local array)
 */
static void _bbc_set_fsk_cfg(uint8_t uc_trx_id, uint8_t *puc_fsk_regs_new, uint8_t *puc_dm_pe_regs_new)
{
	const uint16_t pus_tx_bb_delay[RF215_NUM_FSK_SYMRATES] = BBC_TX_BB_DELAY_FSK;
	const uint8_t puc_rx_bb_delay[RF215_NUM_FSK_SYMRATES] = BBC_RX_BB_DELAY_FSK;
	uint16_t us_tx_bb_delay;
	uint16_t us_tx_pe_delay1;
	uint8_t uc_tx_pe_delay2;
	at86rf_fsk_symrate_t uc_symrate;

	/* Write up to 4 registers: BBCn_FSKC0/1/2/3. New values automatically
	 * updated in static array */
	rf215_spi_write_upd(RF215_ADDR_BBCn_FSKC0(uc_trx_id), puc_fsk_regs_new, spx_bbc_params[uc_trx_id].puc_fsk_cfg_regs, 4);

	uc_symrate = gpx_phy_cfg[uc_trx_id].u_mod_cfg.x_fsk.uc_symrate;
	if (puc_dm_pe_regs_new[0] & RF215_BBCn_FSKDM_PE) {
		const uint16_t pus_tx_pe_delay1[RF215_NUM_FSK_SYMRATES] = BBC_TX_PE_DELAY1_FSK;
		const uint8_t puc_tx_pe_delay2[RF215_NUM_FSK_SYMRATES] = BBC_TX_PE_DELAY2_FSK;
		/* Pre-emphasis enabled */
		/* Compensate pre-emphasis delay in tx_bb_delay */
		us_tx_pe_delay1 = pus_tx_pe_delay1[uc_symrate];
		uc_tx_pe_delay2 = puc_tx_pe_delay2[uc_symrate];
	} else {
		/* Pre-emphasis disabled */
		us_tx_pe_delay1 = 0;
		uc_tx_pe_delay2 = 0;
	}

	/* tx_bb_delay (FSK): depending on FSK symbol rate */
	us_tx_bb_delay = pus_tx_bb_delay[uc_symrate] - us_tx_pe_delay1 - uc_tx_pe_delay2;
	rf215_tx_set_bb_delay(uc_trx_id, us_tx_bb_delay, uc_tx_pe_delay2);

	/* RX delay (FSK): depending on FSK symbol rate */
	spx_bbc_params[uc_trx_id].us_rx_bb_delay_us_q5 = (uint16_t)puc_rx_bb_delay[uc_symrate];

	/* Turnaround time used for slotted CSMA-CA */
	gpx_phy_ctl[uc_trx_id].us_turnaround_time_us = FSK_TURNAROUND_TIME_US;

	/* Write up to 4 regs: BBCn_FSKDM, BBCn_FSKPE0/1/2. New values
	 * automatically updated in static array */
	rf215_spi_write_upd(RF215_ADDR_BBCn_FSKDM(uc_trx_id), puc_dm_pe_regs_new, spx_bbc_params[uc_trx_id].puc_fsk_dm_pe_regs, 4);
}

/**
 * \brief Compute BBCn_OFDMPHRRX register value
 *
 * \param uc_trx_id TRX identifier
 * \param px_ofdm_cfg Pointer to OFDM configuration
 *
 * \return BBCn_OFDMPHRRX value
 */
__always_inline static uint8_t _bbc_ofdmphrrx(uint8_t uc_trx_id, at86rf_ofdm_cfg_t *px_ofdm_cfg)
{
	uint32_t ul_freq_low, ul_freq_mid, ul_freq_high, ul_bw_half;
	uint8_t uc_div;
	uint8_t uc_ofdmphrrx;

	/* OFDMPHRRX.SPC: RX Spurious Compensation. Recommended to activate if
	 * the receive channel is a multiple of 26MHz or 32MHz.
	 * If activated, the AGC target level (AGCS.TGT) should be set to 0 */
	ul_freq_mid = rf215_pll_get_chn_freq(uc_trx_id);
	ul_bw_half = gpul_ofdm_bw_hz[px_ofdm_cfg->uc_opt] >> 1;
	ul_freq_low = ul_freq_mid - ul_bw_half;
	ul_freq_high = ul_freq_mid + ul_bw_half;

	uc_ofdmphrrx = RF215_BBCn_OFDMPHRRX_SPC_DIS;
	uc_div = (uint8_t)(ul_freq_high / 26000000UL);
	if ((26000000UL * uc_div) >= ul_freq_low) {
		/* Multiple of 26MHz falls in receive bandwidth */
		uc_ofdmphrrx = RF215_BBCn_OFDMPHRRX_SPC_EN;
	} else {
		uc_div = (uint8_t)(ul_freq_high / 32000000UL);
		if ((32000000UL * uc_div) >= ul_freq_low) {
			/* Multiple of 32MHz falls in receive bandwidth */
			uc_ofdmphrrx = RF215_BBCn_OFDMPHRRX_SPC_EN;
		}
	}

	return uc_ofdmphrrx;
}

/**
 * \brief Compute BBCn_OFDMC register value
 *
 * \param uc_trx_id TRX identifier
 * \param px_ofdm_cfg Pointer to OFDM configuration
 *
 * \return BBCn_OFDMC value
 */
__always_inline static uint8_t _bbc_ofdmc(uint8_t uc_trx_id, at86rf_ofdm_cfg_t *px_ofdm_cfg)
{
	uint32_t ul_fdelta;
	uint8_t uc_ofdmc;

	/* OFDMC.OPT: MR-OFDM Bandwidth Option */
	uc_ofdmc = RF215_BBCn_OFDMC_OPT(px_ofdm_cfg->uc_opt);

	/* OFDMC.POI: PIB Attribute phyOFDMInterleaving */
	uc_ofdmc |= RF215_BBCn_OFDMC_POI(px_ofdm_cfg->uc_itlv);

	/* OFDMC.LFO: Reception with Low Frequency Offset.
	 * Enable if it is guaranteed that the absolute frequency offset of the
	 * received OFDM signal is less than 57.3kHz.
	 * Get maximum frequency offset due to tolerance, depending on PHY and
	 * channel configuration. Multiply by 2 because offset is given for
	 * single-sided clock */
	ul_fdelta = rf215_pll_get_fdelta(uc_trx_id);
	ul_fdelta <<= 1;
	if (ul_fdelta < 57300) {
		uc_ofdmc |= RF215_BBCn_OFDMC_LFO_EN;
	}

	/* OFDMC.SSTX: Transmitter Scrambler Seed Configuration */
	uc_ofdmc |= RF215_BBCn_OFDMC_SSTX(0);

	return uc_ofdmc;
}

/**
 * \brief Compute BBCn_OFDMSW register value
 *
 * \param px_ofdm_cfg Pointer to OFDM configuration
 *
 * \return BBCn_OFDMSW value
 */
__always_inline static uint8_t _bbc_ofdmsw(at86rf_ofdm_cfg_t *px_ofdm_cfg)
{
	const uint8_t puc_pdt[RF215_NUM_OFDM_OPTIONS] = BBC_OFDMSW_PDT;
	uint8_t uc_ofdmsw;

	/* OFDMSW.PDT: Preamble Detection Threshold */
	uc_ofdmsw = puc_pdt[px_ofdm_cfg->uc_opt];

	/* OFDMSW.RXO = 1: Receiver restarted by >12dB stronger frame */
	uc_ofdmsw |= RF215_BBCn_OFDMSW_RXO_12dB;

	return uc_ofdmsw;
}

/**
 * \brief Compute OFDM configuration register values (BBCn_OFDMPHRRX,
 * BBCn_OFDMC, BBCn_OFDMSW).
 *
 * \param uc_trx_id TRX identifier
 * \param puc_ofdm_regs_new Pointer to store reg values (3 bytes, local array)
 */
static void _bbc_ofdm_cfg_regs(uint8_t uc_trx_id, uint8_t *puc_ofdm_regs_new)
{
	at86rf_ofdm_cfg_t *px_ofdm_cfg;

	/* BBCn_OFDMPHRTX: Not needed to modify default register value.
	 * MCS=0: MCS 0 used for TX by default
	 * RB5/17/18/21=0: PHR reserved bits set to 0 */

	/* BBCn_OFDMPHRRX */
	px_ofdm_cfg = &gpx_phy_cfg[uc_trx_id].u_mod_cfg.x_ofdm;
	puc_ofdm_regs_new[0] = _bbc_ofdmphrrx(uc_trx_id, px_ofdm_cfg);

	/* BBCn_OFDMC */
	puc_ofdm_regs_new[1] = _bbc_ofdmc(uc_trx_id, px_ofdm_cfg);

	/* BBCn_OFDMSW */
	puc_ofdm_regs_new[2] = _bbc_ofdmsw(px_ofdm_cfg);
}

/**
 * \brief Write OFDM configuration registers (BBCn_OFDMPHRRX, BBCn_OFDMC,
 * BBCn_OFDMSW) to RF215 (only if values change from reset or
 * previous configuration). Register values computed with _bbc_ofdm_cfg_regs().
 * Update baseband core processing delays.
 *
 * \param uc_trx_id TRX identifier
 * \param puc_ofdm_regs_new Pointer to register values (3 bytes, local array)
 */
static void _bbc_set_ofdm_cfg(uint8_t uc_trx_id, uint8_t *puc_ofdm_regs_new)
{
	const uint16_t pus_rx_bb_delay[2][RF215_NUM_OFDM_OPTIONS] = BBC_RX_BB_DELAY_OFDM;
	const uint16_t pus_tx_bb_delay[RF215_NUM_OFDM_OPTIONS] = BBC_TX_BB_DELAY_OFDM;
	at86rf_ofdm_cfg_t *px_ofdm_cfg;
	uint16_t us_tx_bb_delay;
	at86rf_ofdm_opt_t uc_opt;
	at86rf_ofdm_itlv_mode_t uc_itlv;

	/* Write up to 3 regs: BBCn_OFDMPHRRX, BBCn_OFDMC, BBCn_OFDMSW. New
	 * values automatically updated in static array */
	rf215_spi_write_upd(RF215_ADDR_BBCn_OFDMPHRRX(uc_trx_id), puc_ofdm_regs_new, spx_bbc_params[uc_trx_id].puc_ofdm_cfg_regs, 3);

	/* tx_bb_delay (OFDM) */
	px_ofdm_cfg = &gpx_phy_cfg[uc_trx_id].u_mod_cfg.x_ofdm;
	uc_opt = px_ofdm_cfg->uc_opt;
	us_tx_bb_delay = pus_tx_bb_delay[uc_opt];
	rf215_tx_set_bb_delay(uc_trx_id, us_tx_bb_delay, 0);

	/* RX delay (OFDM) */
	uc_itlv = px_ofdm_cfg->uc_itlv;
	spx_bbc_params[uc_trx_id].us_rx_bb_delay_us_q5 = pus_rx_bb_delay[uc_itlv][uc_opt];

	/* Turnaround time used for slotted CSMA-CA */
	gpx_phy_ctl[uc_trx_id].us_turnaround_time_us = OFDM_TURNAROUND_TIME_US;
}

/**
 * \brief RF215 Baseband Core TRX Reset Event. Initialize specific registers of
 * the corresponing transceiver Baseband Core PHY after TRX Reset
 * (Wake-up interrupt). This function is called from IRQ. Assumed that TRX is in
 * TRXOFF state, so register configuration is done without changing TRX state
 *
 * \param uc_trx_id TRX identifier
 */
void rf215_bbc_trx_reset_event(uint8_t uc_trx_id)
{
	uint8_t *puc_regs;
	at86rf_phy_cfg_t *px_phy_cfg;
	bbc_params_t *px_params;
	at86rf_phy_mod_t uc_phy_mod;
	uint8_t puc_bbc_irqm_pc[2];

	/* BBCn_IRQM: Enable Baseband Core interrupts */
	puc_bbc_irqm_pc[0] = BBC_IRQM_CFG;

	/* BBCn_PC: Enable baseband for configured PHY modulation */
	px_phy_cfg = &gpx_phy_cfg[uc_trx_id];
	uc_phy_mod = px_phy_cfg->uc_phy_mod;
	puc_bbc_irqm_pc[1] = BBC_PC_CFG_BBEN(uc_phy_mod);

	/* Write 2 registers: BBCn_IRQM, BBCn_PC */
	rf215_spi_write(RF215_ADDR_BBCn_IRQM(uc_trx_id), puc_bbc_irqm_pc, 2);

	/* Initial values of FSK configuration registers after reset */
	px_params = &spx_bbc_params[uc_trx_id];
	puc_regs = px_params->puc_fsk_cfg_regs;
	*puc_regs++ = RF215_BBCn_FSKC0_Rst;
	*puc_regs++ = RF215_BBCn_FSKC1_Rst;
	*puc_regs++ = RF215_BBCn_FSKC2_Rst;
	*puc_regs = RF215_BBCn_FSKC3_Rst;
	puc_regs = px_params->puc_fsk_dm_pe_regs;
	*puc_regs++ = RF215_BBCn_FSKDM_Rst;
	*puc_regs++ = RF215_BBCn_FSKPE0_Rst;
	*puc_regs++ = RF215_BBCn_FSKPE1_Rst;
	*puc_regs = RF215_BBCn_FSKPE2_Rst;

	/* Initial values of OFDM configuration registers after reset */
	puc_regs = px_params->puc_ofdm_cfg_regs;
	*puc_regs++ = RF215_BBCn_OFDMPHRRX_Rst;
	*puc_regs++ = RF215_BBCn_OFDMC_Rst;
	*puc_regs = RF215_BBCn_OFDMSW_Rst;

	if (uc_phy_mod == AT86RF_PHY_MOD_FSK) {
		/* FSK PHY configuration */
		uint8_t puc_fsk_regs_new[4];
		uint8_t puc_dm_pe_regs_new[4];
		_bbc_fsk_cfg_regs(uc_trx_id, puc_fsk_regs_new, puc_dm_pe_regs_new);
		_bbc_set_fsk_cfg(uc_trx_id, puc_fsk_regs_new, puc_dm_pe_regs_new);
	} else if (uc_phy_mod == AT86RF_PHY_MOD_OFDM) {
		/* OFDM PHY configuration */
		uint8_t puc_ofdm_regs_new[3];
		_bbc_ofdm_cfg_regs(uc_trx_id, puc_ofdm_regs_new);
		_bbc_set_ofdm_cfg(uc_trx_id, puc_ofdm_regs_new);
	}

	/* Initial values of TX automatic procedures registers after reset */
	puc_regs = px_params->puc_tx_auto_regs;
	*puc_regs++ = RF215_BBCn_AMCS_Rst;
	*puc_regs = RF215_BBCn_AMEDT_Rst;

	/* BBCn_CNTC: Enable counter with RX and TX start capture mode */
	rf215_bbc_cnt_capture(uc_trx_id);

	/* FSK FEC disabled by default */
	px_params->uc_fsk_tx_fec = AT86RF_FSK_FEC_OFF;

	/* OFDM MCS 0 by default */
	px_params->uc_ofdm_tx_mcs = AT86RF_OFDM_MCS_0;
}

/**
 * \brief Set PHY configuration. IRQ is disabled, TRX state changed to TRXOFF or
 * TXPREP (ongoing TX/RX aborted) and RF215 registers are updated if needed
 * (PLL, Baseband Core, TX/RX Frontend). After updating the configuration, the
 * TRX will start listening again (if not in sleep state) and IRQ enabled again.
 *
 * \param uc_trx_id TRX identifier
 * \param px_phy_cfg_new Pointer to new PHY configuration
 * \param us_chn_num_new New channel number
 *
 * \return Configuration result
 */
at86rf_res_t rf215_bbc_set_phy_cfg(uint8_t uc_trx_id, at86rf_phy_cfg_t *px_phy_cfg_new, uint16_t us_chn_num_new)
{
	at86rf_phy_mod_t uc_phy_mod_new;
	bool b_config_ok;
	bool b_upd_cfg;
	rf215_phy_ctl_t *px_phy_ctl = (rf215_phy_ctl_t *)&gpx_phy_ctl[uc_trx_id];
	at86rf_phy_cfg_t *px_phy_cfg = &gpx_phy_cfg[uc_trx_id];

	/* If channel 0, get first available channel */
	if (us_chn_num_new == 0) {
		us_chn_num_new = px_phy_cfg_new->x_chn_cfg.us_chn_num_min;
	}

	/* Check if PHY configuration changes */
	if ((memcmp(px_phy_cfg, px_phy_cfg_new, sizeof(at86rf_phy_cfg_t)) == 0) &&
			(px_phy_ctl->us_chn_num == us_chn_num_new)) {
		return AT86RF_SUCCESS;
	}

	/* Check correct PHY configuration */
	b_config_ok = _bbc_check_phy_cfg(px_phy_cfg_new);
	if (!b_config_ok) {
		return AT86RF_INVALID_PARAM;
	}

	/* Check correct channel configuration */
	b_config_ok = rf215_pll_check_chn_cfg(uc_trx_id, &px_phy_cfg_new->x_chn_cfg, us_chn_num_new);
	if (!b_config_ok) {
		return AT86RF_INVALID_PARAM;
	}

	/* Check transmission or reception in progress */
	if (px_phy_ctl->uc_phy_state > RF_PHY_STATE_RX_LISTEN) {
		px_phy_ctl->x_phy_cfg_pending = *px_phy_cfg_new;
		px_phy_ctl->us_chn_num_pending = us_chn_num_new;
		px_phy_ctl->b_phy_cfg_pending = true;
		return AT86RF_SUCCESS;
	}

	/* Wait for SPI free to avoid disabling interrupts more than needed */
	rf215_spi_wait_free();

	/* Critical region to avoid state changes from IRQ */
	gx_rf215_hal_wrp.rf_enable_int(false);
	gx_rf215_hal_wrp.timer_enable_int(false);

	/* Update channel configuration */
	rf215_pll_set_chn_cfg(uc_trx_id, &px_phy_cfg_new->x_chn_cfg, us_chn_num_new);
	px_phy_ctl->us_chn_num_listen = us_chn_num_new;

	/* If PHY modulation changes, BBCn_PC must be written in
	 * TRXOFF state. If state is reset or sleep, configuration is
	 * saved and it will be updated after TRX Wake-up interrupt */
	uc_phy_mod_new = px_phy_cfg_new->uc_phy_mod;
	if (px_phy_cfg->uc_phy_mod != uc_phy_mod_new) {
		b_upd_cfg = rf215_trx_switch_trxoff(uc_trx_id);
		px_phy_cfg->uc_phy_mod = uc_phy_mod_new;
		if (b_upd_cfg) {
			/* Send new configuration to RF215 */
			rf215_spi_reg_write(RF215_ADDR_BBCn_PC(uc_trx_id), BBC_PC_CFG_BBEN(uc_phy_mod_new));
		}
	}

	/* Store new modulation (FSK/OFDM) and CCA ED configuration */
	px_phy_cfg->u_mod_cfg = px_phy_cfg_new->u_mod_cfg;
	px_phy_cfg->x_cca_ed_cfg = px_phy_cfg_new->x_cca_ed_cfg;
	px_phy_ctl->b_phy_cfg_pending = false;

	if (uc_phy_mod_new == AT86RF_PHY_MOD_FSK) {
		uint8_t puc_fsk_regs_new[4];
		uint8_t puc_dm_pe_regs_new[4];

		/* Compute FSK configuration register values
		 * (BBCn_FSKC0/1/3, BBCn_FSKDM, BBCn_FSKPE0/1/2) */
		_bbc_fsk_cfg_regs(uc_trx_id, puc_fsk_regs_new, puc_dm_pe_regs_new);

		/* If FSK configuration changes, FSK registers must be
		 * written in TRXOFF state. If state is reset or sleep,
		 * configuration is saved and it will be updated after
		 * TRX Wake-up interrupt */
		if (memcmp(puc_fsk_regs_new, spx_bbc_params[uc_trx_id].puc_fsk_cfg_regs, 4) != 0) {
			b_upd_cfg = rf215_trx_switch_trxoff(uc_trx_id);
			if (b_upd_cfg) {
				/* Send new configuration to RF215 */
				_bbc_set_fsk_cfg(uc_trx_id, puc_fsk_regs_new, puc_dm_pe_regs_new);
			}
		}
	} else if (uc_phy_mod_new == AT86RF_PHY_MOD_OFDM) {
		uint8_t puc_ofdm_regs_new[3];

		/* Compute OFDM configuration register values
		 * (BBCn_OFDMPHRRX, BBCn_OFDMC, BBCn_OFDMSW) */
		_bbc_ofdm_cfg_regs(uc_trx_id, puc_ofdm_regs_new);

		/* If OFDM configuration changes, OFDM registers must be
		 * written in TRXOFF state. If state is reset or sleep,
		 * configuration is saved and it will be updated after
		 * TRX Wake-up interrupt */
		if (memcmp(puc_ofdm_regs_new, spx_bbc_params[uc_trx_id].puc_ofdm_cfg_regs, 3) != 0) {
			b_upd_cfg = rf215_trx_switch_trxoff(uc_trx_id);
			if (b_upd_cfg) {
				/* Send new configuration to RF215 */
				_bbc_set_ofdm_cfg(uc_trx_id, puc_ofdm_regs_new);
			}
		}
	}

	/* Update TX and RX Frontend configuration */
	rf215_fe_upd_phy_cfg(uc_trx_id);

	if ((px_phy_ctl->uc_trx_state == RF215_RFn_STATE_RF_TRXOFF) || (px_phy_ctl->uc_trx_state == RF215_RFn_STATE_RF_TXPREP)) {
		/* Clear TRXRDY flag to wait until PLL is locked (PLL.LS) */
		px_phy_ctl->b_trxrdy = false;
		/* Start listening (if TRX is not in reset or sleep state) */
		rf215_trx_rx_listen(uc_trx_id, false);
	}

	/* Leave critical region */
	gx_rf215_hal_wrp.timer_enable_int(true);
	gx_rf215_hal_wrp.rf_enable_int(true);

	return AT86RF_SUCCESS;
}

/**
 * \brief Update BBC (PHY) parameters for transmission. If register(s) need to
 * be updated, the TRX state will be changed to TRXOFF and TX/RX in progress
 * will be aborted. It is assumed that IRQ is disabled before calling this
 * function.
 *
 * \param[in] uc_trx_id TRX identifier
 * \param[in] px_tx_params Pointer to transmission parameters
 * \param[out] pus_pay_symbols Pointer to number of payload symbols
 *
 * \return Frame duration in us
 */
uint32_t rf215_bbc_upd_tx_params(uint8_t uc_trx_id,
		at86rf_tx_params_t *px_tx_params, uint16_t *pus_pay_symbols)
{
	at86rf_phy_cfg_t *px_phy_cfg;
	at86rf_mod_frame_params_t *px_mod_params;
	at86rf_phy_mod_t uc_phy_mod;

	/* Update TX power attenuation, if needed */
	px_mod_params = &px_tx_params->x_mod_params;
	rf215_fe_set_txpwr(uc_trx_id, px_tx_params->uc_txpwr_att, px_mod_params);

	px_phy_cfg = &gpx_phy_cfg[uc_trx_id];
	uc_phy_mod = px_phy_cfg->uc_phy_mod;
	if (uc_phy_mod == AT86RF_PHY_MOD_FSK) {
		/* FSK PHY parameters */
		at86rf_fsk_fec_t uc_fec_enabled = px_mod_params->x_fsk.uc_fec_enabled;
		if (uc_fec_enabled != spx_bbc_params[uc_trx_id].uc_fsk_tx_fec) {
			uint8_t uc_phrtx;

			/* FEC mode changes: register needs to be written. TRX
			 * must be in TRXOFF state to configure BBCn_FSKPHRTX */
			rf215_trx_switch_trxoff(uc_trx_id);
			spx_bbc_params[uc_trx_id].uc_fsk_tx_fec = uc_fec_enabled;

			if (uc_fec_enabled == AT86RF_FSK_FEC_OFF) {
				/* Use SFD0 (uncoded) */
				uc_phrtx = BBC_FSKPHRTX_FEC_OFF;
			} else {
				/* Use SFD1 (coded) */
				uc_phrtx = BBC_FSKPHRTX_FEC_ON;
			}

			/* Write BBCn_FSKPHRTX register */
			rf215_spi_reg_write(RF215_ADDR_BBCn_FSKPHRTX(uc_trx_id), uc_phrtx);
		}
	} else if (uc_phy_mod == AT86RF_PHY_MOD_OFDM) {
		/* OFDM PHY parameters */
		at86rf_ofdm_mcs_t uc_mcs = px_mod_params->x_ofdm.uc_mcs;
		if (uc_mcs != spx_bbc_params[uc_trx_id].uc_ofdm_tx_mcs) {
			/* MCS changes: register needs to be written. TRX must
			 * be in TRXOFF state to configure BBCn_OFDMPHRTX */
			rf215_trx_switch_trxoff(uc_trx_id);
			spx_bbc_params[uc_trx_id].uc_ofdm_tx_mcs = uc_mcs;

			/* Write BBCn_OFDMPHRTX register */
			rf215_spi_reg_write(RF215_ADDR_BBCn_OFDMPHRTX(uc_trx_id), BBC_OFDMPHRTX(uc_mcs));
		}
	}

	/* Compute frame duration in us for TX confirm */
	return _bbc_frame_duration(px_phy_cfg, px_mod_params, px_tx_params->us_psdu_len, pus_pay_symbols);
}

/**
 * \brief Configure BBC (PHY) for CCATX (CCA with Energy Detection) or TX2RX
 * (Transmit and switch to receive) automatic procedure. The TRX state should
 * be TRXOFF/TXPREP. It is assumed that IRQ is disabled before calling this
 * function.
 *
 * \param uc_trx_id TRX identifier
 * \param b_ccatx CCATX (true) / TX2RX (false)
 * \param px_cca_ed_cfg Pointer to CCA ED configuration (duration and threshold)
 * \param uc_cw Contention window length (for slotted CSMA-CA)
 */
void rf215_bbc_tx_auto_cfg(uint8_t uc_trx_id, bool b_ccatx, at86rf_cca_ed_cfg_t *px_cca_ed_cfg, uint8_t uc_cw)
{
	uint8_t puc_tx_auto_regs_new[2];
	uint8_t *puc_tx_auto_regs_prev = spx_bbc_params[uc_trx_id].puc_tx_auto_regs;

	if (b_ccatx) {
		at86rf_phy_cfg_t *px_phy_cfg;
		at86rf_phy_mod_t uc_phy_mod;

		/* BBCn_PC: Disable baseband. Avoid decoding during ED */
		px_phy_cfg = &gpx_phy_cfg[uc_trx_id];
		uc_phy_mod = px_phy_cfg->uc_phy_mod;
		rf215_spi_reg_write(RF215_ADDR_BBCn_PC(uc_trx_id), BBC_PC_CFG_BBDIS(uc_phy_mod));

		/* Set duration of Energy Detection */
		rf215_fe_set_edd(uc_trx_id, px_cca_ed_cfg->us_duration_us);

		if (uc_cw <= 1) {
			/* BBCn_AMCS: Enable CCATX auto procedure (disable TX2RX) */
			puc_tx_auto_regs_new[0] = RF215_BBCn_AMCS_CCATX;

			/* BBCn_AMEDT */
			puc_tx_auto_regs_new[1] = (uint8_t)px_cca_ed_cfg->sc_threshold_dBm;
		} else {
			/* BBCn_AMCS: Disable CCATX auto procedure */
			puc_tx_auto_regs_new[0] = 0;
			puc_tx_auto_regs_new[1] = puc_tx_auto_regs_prev[1];
		}
	} else {
		/* BBCn_AMCS: Enable TX2RX auto procedure (disable CCATX) */
		puc_tx_auto_regs_new[0] = RF215_BBCn_AMCS_TX2RX;
		puc_tx_auto_regs_new[1] = puc_tx_auto_regs_prev[1];
	}

	/* Write up to 2 registers: BBCn_AMCS, BBCn_AMEDT. New values
	 * automatically updated in static array */
	rf215_spi_write_upd(RF215_ADDR_BBCn_AMCS(uc_trx_id), puc_tx_auto_regs_new, puc_tx_auto_regs_prev, 2);
}

/**
 * \brief Disable automatic procedures (CCATX / TX2RX). It is assumed that IRQ
 * is disabled before calling this function.
 *
 * \param uc_trx_id TRX identifier
 */
void rf215_bbc_tx_auto_stop(uint8_t uc_trx_id)
{
	uint8_t puc_tx_auto_regs_new[1];
	uint8_t *puc_tx_auto_regs_prev = spx_bbc_params[uc_trx_id].puc_tx_auto_regs;

	/* BBCn_AMCS: Disable auto procedures */
	puc_tx_auto_regs_new[0] = 0;

	/* Write up to 1 registers: BBCn_AMCS. New values automatically updated
	 * in static array */
	rf215_spi_write_upd(RF215_ADDR_BBCn_AMCS(uc_trx_id), puc_tx_auto_regs_new, puc_tx_auto_regs_prev, 1);
}

/**
 * \brief Handle RFn_IRQS.EDC interrupt event. Check busy/clear channel and
 * update configuration. If busy channel, the baseband is enabled (changing TRX
 * state from RX to TXPREP). Energy detection duration is restored for Automatic
 * Energy Detection
 *
 * \param uc_trx_id TRX identifier
 * \param px_cca_ed_cfg Pointer to CCA ED configuration (duration and threshold)
 * \param uc_cw Contention window length (for slotted CSMA-CA)
 */
bool rf215_bbc_ccatx_edc_event(uint8_t uc_trx_id, at86rf_cca_ed_cfg_t *px_cca_ed_cfg, uint8_t uc_cw)
{
	bool b_busy_chn;
	bool b_bben;

	if (uc_cw <= 1) {
		/* No contention window: CCATX auto procedure enabled */
		/* Read BBCn_AMCS to check busy/clear channel */
		uint8_t uc_amcs = rf215_spi_reg_read(RF215_ADDR_BBCn_AMCS(uc_trx_id));
		if (uc_amcs & RF215_BBCn_AMCS_CCAED) {
			/* Busy channel. CCATX does not enable baseband
			 * automatically */
			b_busy_chn = true;
			b_bben = true;
		} else {
			/* Clear channel. CCATX enables baseband
			 * automatically */
			b_busy_chn = false;
			b_bben = false;
		}
	} else {
		/* Contention window: ED without CCATX */
		b_bben = true;
		/* Read RFn_EDV to check busy/clear channel */
		int8_t sc_edv = rf215_fe_get_edv(uc_trx_id);
		b_busy_chn = sc_edv > px_cca_ed_cfg->sc_threshold_dBm;
	}

	if (b_bben) {
		/* Enable baseband in TXPREP state */
		at86rf_phy_mod_t uc_phy_mod;
		rf215_trx_cmd_txprep(uc_trx_id);
		uc_phy_mod = gpx_phy_cfg[uc_trx_id].uc_phy_mod;
		rf215_spi_reg_write(RF215_ADDR_BBCn_PC(uc_trx_id), BBC_PC_CFG_BBEN(uc_phy_mod));
	}

	/* Set duration of Energy Detection for automatic ED mode */
	rf215_fe_set_edd_auto(uc_trx_id);

	return b_busy_chn;
}

/**
 * \brief Abort CCATX automatic procedure (before RFn_IRQS.EDC interrupt).
 * Enable baseband and restore energy detection duration for Automatic Energy
 * Detection Mode (triggered by reception).
 *
 * \param uc_trx_id TRX identifier
 */
void rf215_bbc_ccatx_abort(uint8_t uc_trx_id)
{
	at86rf_phy_mod_t uc_phy_mod;

	/* Channel is busy. CCATX automatic procedure doesn't enable it
	 * automatically. Enable baseband in TXPREP state */
	uc_phy_mod = gpx_phy_cfg[uc_trx_id].uc_phy_mod;
	rf215_spi_reg_write(RF215_ADDR_BBCn_PC(uc_trx_id), BBC_PC_CFG_BBEN(uc_phy_mod));

	/* Set duration and mode of Energy Detection for automatic mode */
	rf215_fe_set_edc_edd_auto(uc_trx_id);
}

/**
 * \brief Check PHY header parameters of reception in progress. If valid,
 * compute RF frame duration and set Frame Buffer Level Interrupt (BBCn_FBLI)
 * accordingly
 *
 * \param[in] uc_trx_id TRX identifier
 * \param[in] us_psdu_len PSDU length in bytes, including FCS
 * \param[out] px_rx_ind Pointer to RX indication
 * \param[out] pus_pay_symbols Pointer to number of payload symbols
 *
 *
 * \retval true Valid header received
 * \retval false Invalid header received
 */
bool rf215_bbc_check_rx_params(uint8_t uc_trx_id, uint16_t us_psdu_len,
		at86rf_rx_ind_t *px_rx_ind, uint16_t *pus_pay_symbols)
{
	bool b_valid = true;
	at86rf_phy_cfg_t *px_phy_cfg = &gpx_phy_cfg[uc_trx_id];
	at86rf_mod_frame_params_t *px_mod_params = &px_rx_ind->x_mod_params;

	if (px_phy_cfg->uc_phy_mod == AT86RF_PHY_MOD_FSK) {
		uint8_t uc_phrrx;

		/* FSK PHY parameters: Read BBCn_FSKPHRRX */
		uc_phrrx = rf215_spi_reg_read(RF215_ADDR_BBCn_FSKPHRRX(uc_trx_id));

		/* Check if PHR is valid (FEC enabled/disabled) or invalid */
		uc_phrrx &= BBC_FSKPHRRX_MASK;
		if (uc_phrrx == BBC_FSKPHRRX_FEC_OFF) {
			/* Valid PHR with SFD0 (uncoded) */
			px_mod_params->x_fsk.uc_fec_enabled = AT86RF_FSK_FEC_OFF;
		} else if (uc_phrrx == BBC_FSKPHRRX_FEC_ON) {
			/* Valid PHR with SFD1 (coded) */
			px_mod_params->x_fsk.uc_fec_enabled = AT86RF_FSK_FEC_ON;
		} else {
			/* Invalid PHR */
			b_valid = false;
		}
	} else if (px_phy_cfg->uc_phy_mod == AT86RF_PHY_MOD_OFDM) {
		uint8_t uc_phrrx;
		uint8_t uc_mcs;

		/* OFDM PHY parameters: Read BBCn_OFDMPHRRX */
		uc_phrrx = rf215_spi_reg_read(RF215_ADDR_BBCn_OFDMPHRRX(uc_trx_id));

		/* Check if PHR is valid (MCS 0 to 6) or invalid */
		uc_mcs = uc_phrrx & RF215_BBCn_OFDMPHRRX_MCS_Msk;
		if (uc_mcs > AT86RF_OFDM_MCS_6) {
			/* Invalid PHR */
			b_valid = false;
		} else {
			px_mod_params->x_ofdm.uc_mcs = (at86rf_ofdm_mcs_t)uc_mcs;
		}
	}

	if (b_valid) {
		uint32_t ul_rxfs_delay_q5;
		uint32_t ul_rx_time_trx;
		uint16_t us_buf_lvl_int;

		/* Compute number of bytes for FBLI interrupt */
		us_buf_lvl_int = _bbc_fbli(px_phy_cfg, px_mod_params,
				us_psdu_len);

		/* Write BBCn_FBLIH */
		_bbc_set_buf_lvl_int(uc_trx_id, us_buf_lvl_int);

		/* Compute frame duration in us for RX indication */
		px_rx_ind->us_psdu_len = us_psdu_len;
		px_rx_ind->ul_frame_duration = _bbc_frame_duration(px_phy_cfg,
				px_mod_params, us_psdu_len, pus_pay_symbols);

		/* Compute delay of RXFS IRQ in us [uQ14.5] */
		ul_rxfs_delay_q5 = _bbc_rxfs_delay_us_q5(px_phy_cfg, px_mod_params);

		/* Read counter (in capture mode). The RX frame start event
		 * complies to the interrupt RXFS */
		ul_rx_time_trx = rf215_bbc_get_cnt(uc_trx_id);

		/* Actual TRX time of frame start (compensate RXFS delay) */
		ul_rx_time_trx -= ul_rxfs_delay_q5;
		ul_rx_time_trx -= spx_bbc_params[uc_trx_id].us_rx_bb_delay_us_q5;
		ul_rx_time_trx -= spx_bbc_params[uc_trx_id].us_rx_proc_delay_us_q5;

		/* Convert time read from RF215 TRX to PHY time */
		rf215_trx_upd_sync(uc_trx_id);
		px_rx_ind->ul_rx_time_ini = rf215_trx_to_phy_time(uc_trx_id, ul_rx_time_trx);
	}

	return b_valid;
}

/**
 * \brief Get counter value (capture or free-running mode). The counter
 * frequency is 32MHz
 *
 * \param uc_trx_id TRX identifier
 *
 * \return Counter value
 */
uint32_t rf215_bbc_get_cnt(uint8_t uc_trx_id)
{
	uint32_t ul_counter;
	uint8_t puc_bbc_cnt[4] = {0};

	/* Read BBCn_CNT0..3 (4 bytes). BBCn_CNT0 is least significant byte */
	rf215_spi_read(RF215_ADDR_BBCn_CNT0(uc_trx_id), puc_bbc_cnt, 4);
	ul_counter = (uint32_t)puc_bbc_cnt[0];
	ul_counter += (uint32_t)puc_bbc_cnt[1] << 8;
	ul_counter += (uint32_t)puc_bbc_cnt[2] << 16;
	ul_counter += (uint32_t)puc_bbc_cnt[3] << 24;

	return ul_counter;
}

/**
 * \brief Set Receiver Frontend delay.
 *
 * \param uc_trx_id TRX identifier
 * \param us_proc_delay_us_q5 Receiver Frontend delay in us [uQ6.5]
 */
void rf215_bbc_set_rx_proc_delay(uint8_t uc_trx_id, uint16_t us_proc_delay_us_q5)
{
	spx_bbc_params[uc_trx_id].us_rx_proc_delay_us_q5 = us_proc_delay_us_q5;
}

/**
 * \brief Get OFDM Spurious Compensation (SPC) configuration.
 *
 * \param uc_trx_id TRX identifier
 *
 * \retval true SPC enabled
 * \retval false SPC disabled
 */
bool rf215_bbc_get_ofdm_scp(uint8_t uc_trx_id)
{
	if (spx_bbc_params[uc_trx_id].puc_ofdm_cfg_regs[0] & RF215_BBCn_OFDMPHRRX_SPC_EN) {
		return true;
	} else {
		return false;
	}
}

/**
 * \brief Function to get symbol duration in us [uQ14.5].
 *
 * \param uc_trx_id TRX identifier
 *
 * \return Symbol duration in us [uQ14.5]
 */
uint16_t rf215_bbc_get_symbol_duration_us_q5(uint8_t uc_trx_id)
{
	uint16_t us_symb_duration_us_q5 = 0;
	at86rf_phy_cfg_t *px_phy_cfg = &gpx_phy_cfg[uc_trx_id];

	if (px_phy_cfg->uc_phy_mod == AT86RF_PHY_MOD_FSK) {
		/* Symbol rate in kHz */
		uint16_t us_symb_rate_khz = gpus_fsk_symrate_khz[px_phy_cfg->u_mod_cfg.x_fsk.uc_symrate];

		/* Compute symbol duration in us [uQ14.5] */
		us_symb_duration_us_q5 = div_round(1000 << 5, us_symb_rate_khz);
	} else { /* AT86RF_PHY_MOD_OFDM */
		us_symb_duration_us_q5 = OFDM_SYMB_DURATION_US << 5;
	}

	return us_symb_duration_us_q5;
}

/**
 * \brief Function to get receiver sensitivity in dBm.
 *
 * \param uc_trx_id TRX identifier
 *
 * \return Receiver sensitivity in dBm
 */
int8_t rf215_bbc_get_sensitivity_dBm(uint8_t uc_trx_id)
{
	int8_t sc_sensitivity_dBm = 0;
	at86rf_phy_cfg_t *px_phy_cfg = &gpx_phy_cfg[uc_trx_id];

	if (px_phy_cfg->uc_phy_mod == AT86RF_PHY_MOD_FSK) {
		sc_sensitivity_dBm = spsc_fsk_sensitivity[px_phy_cfg->u_mod_cfg.x_fsk.uc_symrate];
	} else { /* AT86RF_PHY_MOD_OFDM */
		sc_sensitivity_dBm = spsc_ofdm_sensitivity[px_phy_cfg->u_mod_cfg.x_ofdm.uc_opt];
	}

	return sc_sensitivity_dBm;
}

/**
 * \brief Function to initialize Baseband Core module.
 *
 * \param uc_trx_id TRX identifier
 * \px_phy_cfg Pointer to initial PHY configuration
 * \param us_chn_num Channel number
 *
 * \retval true Correct configuration
 * \retval false Wrong configuration or not supported
 */
bool rf215_bbc_init(uint8_t uc_trx_id, at86rf_phy_cfg_t *px_phy_cfg, uint16_t us_chn_num)
{
	bool b_valid_cfg = _bbc_check_phy_cfg(px_phy_cfg);

	/* If channel 0, get first available channel */
	if (us_chn_num == 0) {
		us_chn_num = px_phy_cfg->x_chn_cfg.us_chn_num_min;
	}

	/* Initialize Frequency Synthesizer module (channel configuration) */
	if (b_valid_cfg) {
		b_valid_cfg = rf215_pll_init(uc_trx_id, &px_phy_cfg->x_chn_cfg, us_chn_num);
	}

	if (b_valid_cfg) {
		/* Store PHY configuration */
		gpx_phy_cfg[uc_trx_id] = *px_phy_cfg;
		gpx_phy_ctl[uc_trx_id].us_chn_num = us_chn_num;
		gpx_phy_ctl[uc_trx_id].us_chn_num_listen = us_chn_num;
	}

	return b_valid_cfg;
}

/**
 * \brief Compute message duration
 *
 * \param[in] uc_trx_id TRX identifier
 * \param[in] px_mod_params Pointer to frame parameters
 * \param[in] us_psdu_len PSDU length in bytes (octets), including FCS
 * \param[out] pul_duration Pointer to message duration
 *
 * \return Result
 */
at86rf_res_t at86rf_get_msg_duration(uint8_t uc_trx_id,
		at86rf_mod_frame_params_t *px_mod_params, uint16_t us_psdu_len,
		uint32_t *pul_duration)
{
	uint32_t ul_duration;
	uint16_t us_pay_symbols;

	/* Check TRX ID */
	if (uc_trx_id >= AT86RF_NUM_TRX) {
		return AT86RF_INVALID_TRX_ID;
	}

	/* Check pointer */
	if (pul_duration == NULL) {
		return AT86RF_INVALID_ATTR;
	}

	at86rf_phy_cfg_t *px_phy_cfg = &gpx_phy_cfg[uc_trx_id];

	ul_duration = _bbc_frame_duration(px_phy_cfg, px_mod_params, us_psdu_len, &us_pay_symbols);
	*pul_duration = ul_duration;

	return AT86RF_SUCCESS;
}

#ifdef __cplusplus
}
#endif
