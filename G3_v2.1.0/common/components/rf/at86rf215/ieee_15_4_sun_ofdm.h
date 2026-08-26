/**
 *
 * \file
 *
 * \brief IEEE 802.15.4 (2020) SUN OFDM modes for AT86RF215.
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

#ifndef IEEE_15_4_SUN_OFDM_H_INCLUDED
#define IEEE_15_4_SUN_OFDM_H_INCLUDED

/* AT86RF includes */
#include "at86rf_defs.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

/************** CCA Energy Detection common configuration (FSK) **************/

/* Duration in us of CCA ED. From IEEE 802.15.4 section 10.2.6 Receiver ED: The
 * ED measurement time, to average over, shall be equal to 8 symbol periods.
 * For SUN OFDM, the symbol duration is fixed to 120 us */
#define SUN_OFDM_CCA_ED_DURATION                960

/* Threshold in dBm of CCA ED. From IEEE 802.15.4 section 10.2.8 Clear channel
 * assessment (CCA): Except for the SUN O-QPSK PHY, the ED threshold shall
 * correspond to a received signal power of at most 10 dB greater than the
 * specified receiver sensitivity for that PHY, or in accordance with local
 * regulations.
 * From IEEE 802.15.4 section 20.5.3 Receiver sensitivity:
 * Table 20-21—Sensitivity requirements for OFDM options and MCS levels
 * Sensitivity for MCS6 is selected (worst case) */
#define SUN_OFDM_OPT1_CCA_THRESHOLD             -75
#define SUN_OFDM_OPT2_CCA_THRESHOLD             -78
#define SUN_OFDM_OPT3_CCA_THRESHOLD             -81
#define SUN_OFDM_OPT4_CCA_THRESHOLD             -84

/********** SUN OFDM configurations for all bands and operating modes *********/

/* AT86RF215 PHY configuration of SUN OFDM modes as defined in 802.15.4 (2020).
 * Sections 10.1.3.9 Channel numbering for SUN and TVWS PHYs / 20.5.1 Operating
 * frequency range */

/************************* Frequency band 470-510 MHz *************************/

/* OFDM Bandwidth Option 4 */
#define SUN_OFDM_BAND_470_510_OPT4              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_4, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 470200000, \
			.ul_chn_spa_hz = 200000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 198, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT4_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 779-787 MHz *************************/

/* OFDM Bandwidth Option 4 */
#define SUN_OFDM_BAND_779_787_OPT4              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_4, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 779200000, \
			.ul_chn_spa_hz = 200000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 38, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT4_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 3 */
#define SUN_OFDM_BAND_779_787_OPT3              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_3, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 779400000, \
			.ul_chn_spa_hz = 400000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 18, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT3_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 2 */
#define SUN_OFDM_BAND_779_787_OPT2              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_2, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 779800000, \
			.ul_chn_spa_hz = 800000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 8, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT2_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 1 */
#define SUN_OFDM_BAND_779_787_OPT1              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_1, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 780200000, \
			.ul_chn_spa_hz = 1200000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 5, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT1_CCA_THRESHOLD, \
		} \
	}

/************* Frequency band 863-870 MHz (RF-PLC Hybrid default) *************/

/* OFDM Bandwidth Option 4 */
#define SUN_OFDM_BAND_863_870_OPT4              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_4, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 863100000, \
			.ul_chn_spa_hz = 200000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 34, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT4_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 865-867 MHz *************************/

/* OFDM Bandwidth Option 4 */
#define SUN_OFDM_BAND_865_867_OPT4              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_4, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 865100000, \
			.ul_chn_spa_hz = 200000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 9, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT4_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 866-869 MHz *************************/

/* OFDM Bandwidth Option 4 */
#define SUN_OFDM_BAND_866_869_OPT4              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_4, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 863100000, \
			.ul_chn_spa_hz = 200000, \
			.us_chn_num_min = 15, \
			.us_chn_num_max = 29, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT4_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 3 */
#define SUN_OFDM_BAND_866_869_OPT3              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_3, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 863200000, \
			.ul_chn_spa_hz = 400000, \
			.us_chn_num_min = 8, \
			.us_chn_num_max = 14, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT3_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 870-876 MHz *************************/

/* OFDM Bandwidth Option 4 */
#define SUN_OFDM_BAND_870_876_OPT4              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_4, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 870200000, \
			.ul_chn_spa_hz = 200000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 29, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT4_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 902-928 MHz *************************/

/* OFDM Bandwidth Option 4 */
#define SUN_OFDM_BAND_902_928_OPT4              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_4, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902200000, \
			.ul_chn_spa_hz = 200000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 128, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT4_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 3 */
#define SUN_OFDM_BAND_902_928_OPT3              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_3, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902400000, \
			.ul_chn_spa_hz = 400000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 63, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT3_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 2 */
#define SUN_OFDM_BAND_902_928_OPT2              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_2, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902800000, \
			.ul_chn_spa_hz = 800000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 30, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT2_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 1 */
#define SUN_OFDM_BAND_902_928_OPT1              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_1, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 903200000, \
			.ul_chn_spa_hz = 1200000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 19, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT1_CCA_THRESHOLD, \
		} \
	}

/******************* Frequency band 902-928 MHz (alternate) *******************/

/* OFDM Bandwidth Option 4 */
#define SUN_OFDM_BAND_902_928_ALT_OPT4          (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_4, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902200000, \
			.ul_chn_spa_hz = 200000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 128, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT4_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 3 */
#define SUN_OFDM_BAND_902_928_ALT_OPT3          (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_3, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902400000, \
			.ul_chn_spa_hz = 400000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 63, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT3_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 2 */
#define SUN_OFDM_BAND_902_928_ALT_OPT2          (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_2, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902800000, \
			.ul_chn_spa_hz = 800000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 30, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT2_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 1 */
#define SUN_OFDM_BAND_902_928_ALT_OPT1          (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_1, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 903200000, \
			.ul_chn_spa_hz = 1200000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 19, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT1_CCA_THRESHOLD, \
		} \
	}

/******************* Frequency band 902–907.5 & 915–928 MHz *******************/

/* OFDM Bandwidth Option 4 */
#define SUN_OFDM_BAND_902_907_915_928_OPT4              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_4, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902200000, \
			.ul_chn_spa_hz = 200000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 26, \
			.us_chn_num_min2 = 65, \
			.us_chn_num_max2 = 128 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT4_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 3 */
#define SUN_OFDM_BAND_902_907_915_928_OPT3              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_3, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902400000, \
			.ul_chn_spa_hz = 400000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 12, \
			.us_chn_num_min2 = 32, \
			.us_chn_num_max2 = 63 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT3_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 2 */
#define SUN_OFDM_BAND_902_907_915_928_OPT2              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_2, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902800000, \
			.ul_chn_spa_hz = 800000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 5, \
			.us_chn_num_min2 = 16, \
			.us_chn_num_max2 = 30 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT2_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 1 */
#define SUN_OFDM_BAND_902_907_915_928_OPT1              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_1, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 903200000, \
			.ul_chn_spa_hz = 1200000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 3, \
			.us_chn_num_min2 = 11, \
			.us_chn_num_max2 = 19 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT1_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 915-928 MHz *************************/

/* OFDM Bandwidth Option 4 */
#define SUN_OFDM_BAND_915_928_OPT4              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_4, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902200000, \
			.ul_chn_spa_hz = 200000, \
			.us_chn_num_min = 65, \
			.us_chn_num_max = 128, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT4_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 3 */
#define SUN_OFDM_BAND_915_928_OPT3              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_3, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902400000, \
			.ul_chn_spa_hz = 400000, \
			.us_chn_num_min = 32, \
			.us_chn_num_max = 63, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT3_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 2 */
#define SUN_OFDM_BAND_915_928_OPT2              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_2, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902800000, \
			.ul_chn_spa_hz = 800000, \
			.us_chn_num_min = 16, \
			.us_chn_num_max = 30, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT2_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 1 */
#define SUN_OFDM_BAND_915_928_OPT1              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_1, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 903200000, \
			.ul_chn_spa_hz = 1200000, \
			.us_chn_num_min = 11, \
			.us_chn_num_max = 19, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT1_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 915-921 MHz *************************/

/* OFDM Bandwidth Option 4 */
#define SUN_OFDM_BAND_915_921_OPT4              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_4, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902200000, \
			.ul_chn_spa_hz = 200000, \
			.us_chn_num_min = 65, \
			.us_chn_num_max = 93, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT4_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 3 */
#define SUN_OFDM_BAND_915_921_OPT3              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_3, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902400000, \
			.ul_chn_spa_hz = 400000, \
			.us_chn_num_min = 32, \
			.us_chn_num_max = 46, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT3_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 915-918 MHz *************************/

/* OFDM Bandwidth Option 4 */
#define SUN_OFDM_BAND_915_918_OPT4              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_4, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902200000, \
			.ul_chn_spa_hz = 200000, \
			.us_chn_num_min = 65, \
			.us_chn_num_max = 78, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT4_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 3 */
#define SUN_OFDM_BAND_915_918_OPT3              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_3, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902400000, \
			.ul_chn_spa_hz = 400000, \
			.us_chn_num_min = 32, \
			.us_chn_num_max = 38, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT3_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 2 */
#define SUN_OFDM_BAND_915_918_OPT2              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_2, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902800000, \
			.ul_chn_spa_hz = 800000, \
			.us_chn_num_min = 16, \
			.us_chn_num_max = 18, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT2_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 1 */
#define SUN_OFDM_BAND_915_918_OPT1              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_1, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 903200000, \
			.ul_chn_spa_hz = 1200000, \
			.us_chn_num_min = 11, \
			.us_chn_num_max = 11, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT1_CCA_THRESHOLD, \
		} \
	}

/************************ Frequency band 917-923.5 MHz ************************/

/* OFDM Bandwidth Option 4 */
#define SUN_OFDM_BAND_917_923_OPT4              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_4, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 917100000, \
			.ul_chn_spa_hz = 200000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 31, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT4_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 3 */
#define SUN_OFDM_BAND_917_923_OPT3              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_3, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 917300000, \
			.ul_chn_spa_hz = 400000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 15, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT3_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 2 */
#define SUN_OFDM_BAND_917_923_OPT2              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_2, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 917500000, \
			.ul_chn_spa_hz = 800000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 7, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT2_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 1 */
#define SUN_OFDM_BAND_917_923_OPT1              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_1, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 917900000, \
			.ul_chn_spa_hz = 1200000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 4, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT1_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 919-923 MHz *************************/

/* OFDM Bandwidth Option 4 */
#define SUN_OFDM_BAND_919_923_OPT4              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_4, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902200000, \
			.ul_chn_spa_hz = 200000, \
			.us_chn_num_min = 85, \
			.us_chn_num_max = 103, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT4_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 3 */
#define SUN_OFDM_BAND_919_923_OPT3              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_3, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902400000, \
			.ul_chn_spa_hz = 400000, \
			.us_chn_num_min = 42, \
			.us_chn_num_max = 51, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT3_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 2 */
#define SUN_OFDM_BAND_919_923_OPT2              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_2, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902800000, \
			.ul_chn_spa_hz = 800000, \
			.us_chn_num_min = 21, \
			.us_chn_num_max = 24, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT2_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 1 */
#define SUN_OFDM_BAND_919_923_OPT1              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_1, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 903200000, \
			.ul_chn_spa_hz = 1200000, \
			.us_chn_num_min = 14, \
			.us_chn_num_max = 16, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT1_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 920-928 MHz *************************/

/* OFDM Bandwidth Option 4 */
#define SUN_OFDM_BAND_920_928_OPT4              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_4, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 920200000, \
			.ul_chn_spa_hz = 200000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 38, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT4_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 3 */
#define SUN_OFDM_BAND_920_928_OPT3              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_3, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 920400000, \
			.ul_chn_spa_hz = 400000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 18, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT3_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 2 */
#define SUN_OFDM_BAND_920_928_OPT2              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_2, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 920800000, \
			.ul_chn_spa_hz = 800000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 8, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT2_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 1 */
#define SUN_OFDM_BAND_920_928_OPT1              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_1, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 921200000, \
			.ul_chn_spa_hz = 1200000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 4, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT1_CCA_THRESHOLD, \
		} \
	}

/*********************** Frequency band 920.5-924.5MHz ************************/

/* OFDM Bandwidth Option 4 */
#define SUN_OFDM_BAND_920_924_OPT4              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_4, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902200000, \
			.ul_chn_spa_hz = 200000, \
			.us_chn_num_min = 92, \
			.us_chn_num_max = 111, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT4_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 920-925 MHz *************************/

/* OFDM Bandwidth Option 4 */
#define SUN_OFDM_BAND_920_925_OPT4              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_4, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902200000, \
			.ul_chn_spa_hz = 200000, \
			.us_chn_num_min = 90, \
			.us_chn_num_max = 113, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT4_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 3 */
#define SUN_OFDM_BAND_920_925_OPT3              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_3, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902400000, \
			.ul_chn_spa_hz = 400000, \
			.us_chn_num_min = 45, \
			.us_chn_num_max = 56, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT3_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 2 */
#define SUN_OFDM_BAND_920_925_OPT2              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_2, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902800000, \
			.ul_chn_spa_hz = 800000, \
			.us_chn_num_min = 22, \
			.us_chn_num_max = 27, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT2_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 1 */
#define SUN_OFDM_BAND_920_925_OPT1              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_1, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 903200000, \
			.ul_chn_spa_hz = 1200000, \
			.us_chn_num_min = 15, \
			.us_chn_num_max = 17, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT1_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 920-923 MHz *************************/

/* OFDM Bandwidth Option 4 */
#define SUN_OFDM_BAND_920_923_OPT4              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_4, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902200000, \
			.ul_chn_spa_hz = 200000, \
			.us_chn_num_min = 90, \
			.us_chn_num_max = 103, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT4_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 3 */
#define SUN_OFDM_BAND_920_923_OPT3              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_3, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902400000, \
			.ul_chn_spa_hz = 400000, \
			.us_chn_num_min = 45, \
			.us_chn_num_max = 51, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT3_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 2 */
#define SUN_OFDM_BAND_920_923_OPT2              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_2, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 902800000, \
			.ul_chn_spa_hz = 800000, \
			.us_chn_num_min = 22, \
			.us_chn_num_max = 24, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT2_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 1 */
#define SUN_OFDM_BAND_920_923_OPT1              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_1, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 903200000, \
			.ul_chn_spa_hz = 1200000, \
			.us_chn_num_min = 15, \
			.us_chn_num_max = 16, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT1_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 2400-2483.5 MHz *********************/

/* OFDM Bandwidth Option 4 */
#define SUN_OFDM_BAND_2400_2483_OPT4              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_4, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 2400200000,	\
			.ul_chn_spa_hz = 200000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 415, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT4_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 3 */
#define SUN_OFDM_BAND_2400_2483_OPT3              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_3, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 2400400000,	\
			.ul_chn_spa_hz = 400000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 206, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT3_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 2 */
#define SUN_OFDM_BAND_2400_2483_OPT2              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_2, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 2400800000,	\
			.ul_chn_spa_hz = 800000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 96, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT2_CCA_THRESHOLD, \
		} \
	}

/* OFDM Bandwidth Option 1 */
#define SUN_OFDM_BAND_2400_2483_OPT1              (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_OFDM, \
		.u_mod_cfg.x_ofdm = { \
			.uc_opt = AT86RF_OFDM_OPT_1, \
			.uc_itlv = AT86RF_OFDM_INTERLEAVING_1 \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 2401200000,	\
			.ul_chn_spa_hz = 1200000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 63, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_OFDM_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_OFDM_OPT1_CCA_THRESHOLD, \
		} \
	}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */

#endif /* IEEE_15_4_SUN_OFDM_H_INCLUDED */
