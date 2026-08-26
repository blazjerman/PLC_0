/**
 *
 * \file
 *
 * \brief IEEE 802.15.4 (2020 + 2022-aa amendment) SUN FSK modes for AT86RF215.
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

#ifndef IEEE_15_4_SUN_FSK_H_INCLUDED
#define IEEE_15_4_SUN_FSK_H_INCLUDED

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
 * For SUN FSK, the symbol duration is chosen from operating mode #1 */
#define SUN_FSK_50kHz_CCA_ED_DURATION           160
#define SUN_FSK_100kHz_CCA_ED_DURATION          80
#define SUN_FSK_150kHz_CCA_ED_DURATION          53
#define SUN_FSK_200kHz_CCA_ED_DURATION          40
#define SUN_FSK_300kHz_CCA_ED_DURATION          27
#define SUN_FSK_400kHz_CCA_ED_DURATION          20

/* Threshold in dBm of CCA ED. From IEEE 802.15.4 section 10.2.8 Clear channel
 * assessment (CCA): Except for the SUN O-QPSK PHY, the ED threshold shall
 * correspond to a received signal power of at most 10 dB greater than the
 * specified receiver sensitivity for that PHY, or in accordance with local
 * regulations.
 * From IEEE 802.15.4 section 19.6.7 Receiver sensitivity:
 * S = (S0 + 10log[R/R0]) dBm. Where S0 = -91 (without FEC) / -97 (with FEC);
 * R0 = 50 kb/s; R = bit rate in kb/s
 * Sensitivity without FEC is selected (worst case) */
#define SUN_FSK_50kHz_CCA_THRESHOLD             -81
#define SUN_FSK_100kHz_CCA_THRESHOLD            -78
#define SUN_FSK_150kHz_CCA_THRESHOLD            -76
#define SUN_FSK_200kHz_CCA_THRESHOLD            -75
#define SUN_FSK_300kHz_CCA_THRESHOLD            -73
#define SUN_FSK_400kHz_CCA_THRESHOLD            -72

/********** SUN FSK configurations for all bands and operating modes **********/

/* AT86RF215 PHY configuration of SUN FSK modes as defined in 802.15.4 (2020).
 * Sections 10.1.3.9 Channel numbering for SUN and TVWS PHYs / 19.3 Modulation
 * and coding for SUN FSK */

/************************* Frequency band 470-510 MHz *************************/

/* Operating mode #1 */
#define SUN_FSK_BAND_470_510_OPM1               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_50kHz,	\
			.uc_modidx = AT86RF_FSK_MODIDX_1_0, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_50kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #2 */
#define SUN_FSK_BAND_470_510_OPM2               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_100kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_100kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #3 */
#define SUN_FSK_BAND_470_510_OPM3               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_150kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_150kHz_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 779-787 MHz *************************/

/* Operating mode #1 */
#define SUN_FSK_BAND_779_787_OPM1               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_50kHz,	\
			.uc_modidx = AT86RF_FSK_MODIDX_1_0, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_50kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #2 */
#define SUN_FSK_BAND_779_787_OPM2               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_100kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_1_0, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_100kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #3 */
#define SUN_FSK_BAND_779_787_OPM3               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_100kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_1_0, \
			.uc_modord = AT86RF_FSK_MODORD_4FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_100kHz_CCA_THRESHOLD + 3, \
		} \
	}

/************* Frequency band 863-870 MHz (RF-PLC Hybrid default) *************/

/* Operating mode #1 */
#define SUN_FSK_BAND_863_870_OPM1               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_50kHz,	\
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 863100000, \
			.ul_chn_spa_hz = 100000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 68, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_50kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #2 */
#define SUN_FSK_BAND_863_870_OPM2               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_100kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_100kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #3 */
#define SUN_FSK_BAND_863_870_OPM3               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_150kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_150kHz_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 865-867 MHz *************************/

/* Operating mode #1 */
#define SUN_FSK_BAND_865_867_OPM1               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_50kHz,	\
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 865100000, \
			.ul_chn_spa_hz = 100000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 18, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_50kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #2 */
#define SUN_FSK_BAND_865_867_OPM2               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_100kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_100kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #3 */
#define SUN_FSK_BAND_865_867_OPM3               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_150kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_150kHz_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 866-869 MHz *************************/

/* Operating mode #1 */
#define SUN_FSK_BAND_866_869_OPM1               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_50kHz,	\
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 863100000, \
			.ul_chn_spa_hz = 100000, \
			.us_chn_num_min = 30, \
			.us_chn_num_max = 58, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_50kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #2 */
#define SUN_FSK_BAND_866_869_OPM2               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_100kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_100kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #3 */
#define SUN_FSK_BAND_866_869_OPM3               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_150kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_150kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #4 */
#define SUN_FSK_BAND_866_869_OPM4               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_200kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_200kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #5 */
#define SUN_FSK_BAND_866_869_OPM5               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_300kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_300kHz_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 870-876 MHz *************************/

/* Operating mode #1 */
#define SUN_FSK_BAND_870_876_OPM1               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_50kHz,	\
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 870100000, \
			.ul_chn_spa_hz = 100000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 58, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_50kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #2 */
#define SUN_FSK_BAND_870_876_OPM2               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_100kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_100kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #3 */
#define SUN_FSK_BAND_870_876_OPM3               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_150kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_150kHz_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 902-928 MHz *************************/

/* Operating mode #1 */
#define SUN_FSK_BAND_902_928_OPM1               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_50kHz,	\
			.uc_modidx = AT86RF_FSK_MODIDX_1_0, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_50kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #2 */
#define SUN_FSK_BAND_902_928_OPM2               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_150kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_150kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #3 */
#define SUN_FSK_BAND_902_928_OPM3               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_200kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_200kHz_CCA_THRESHOLD, \
		} \
	}

/******************* Frequency band 902-928 MHz (alternate) *******************/

/* Operating mode #1 */
#define SUN_FSK_BAND_902_928_ALT_OPM1           (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_50kHz,	\
			.uc_modidx = AT86RF_FSK_MODIDX_1_0, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_50kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #2 */
#define SUN_FSK_BAND_902_928_ALT_OPM2           (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_100kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_100kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #3 */
#define SUN_FSK_BAND_902_928_ALT_OPM3           (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_150kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_150kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #4 */
#define SUN_FSK_BAND_902_928_ALT_OPM4           (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_200kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_200kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #5 */
#define SUN_FSK_BAND_902_928_ALT_OPM5           (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_300kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_300kHz_CCA_THRESHOLD, \
		} \
	}

/******************* Frequency band 902–907.5 & 915–928 MHz *******************/

/* Operating mode #1 */
#define SUN_FSK_BAND_902_907_915_928_OPM1           (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_50kHz,	\
			.uc_modidx = AT86RF_FSK_MODIDX_1_0, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_50kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #2 */
#define SUN_FSK_BAND_902_907_915_928_OPM2           (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_100kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_100kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #3 */
#define SUN_FSK_BAND_902_907_915_928_OPM3           (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_150kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_150kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #4 */
#define SUN_FSK_BAND_902_907_915_928_OPM4           (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_200kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_200kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #5 */
#define SUN_FSK_BAND_902_907_915_928_OPM5           (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_300kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_300kHz_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 915-928 MHz *************************/

/* Operating mode #1 */
#define SUN_FSK_BAND_915_928_OPM1               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_50kHz,	\
			.uc_modidx = AT86RF_FSK_MODIDX_1_0, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_50kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #2 */
#define SUN_FSK_BAND_915_928_OPM2               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_100kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_100kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #3 */
#define SUN_FSK_BAND_915_928_OPM3               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_150kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_150kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #4 */
#define SUN_FSK_BAND_915_928_OPM4               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_200kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_200kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #5 */
#define SUN_FSK_BAND_915_928_OPM5               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_300kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_300kHz_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 915-921 MHz *************************/

/* Operating mode #1 */
#define SUN_FSK_BAND_915_921_OPM1               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_50kHz,	\
			.uc_modidx = AT86RF_FSK_MODIDX_1_0, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_50kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #2 */
#define SUN_FSK_BAND_915_921_OPM2               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_100kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_100kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #3 */
#define SUN_FSK_BAND_915_921_OPM3               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_150kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_150kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #4 */
#define SUN_FSK_BAND_915_921_OPM4               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_200kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_200kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #5 */
#define SUN_FSK_BAND_915_921_OPM5               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_300kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_300kHz_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 915-918 MHz *************************/

/* Operating mode #1 */
#define SUN_FSK_BAND_915_918_OPM1               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_50kHz,	\
			.uc_modidx = AT86RF_FSK_MODIDX_1_0, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_50kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #2 */
#define SUN_FSK_BAND_915_918_OPM2               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_100kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_100kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #3 */
#define SUN_FSK_BAND_915_918_OPM3               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_150kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_150kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #4 */
#define SUN_FSK_BAND_915_918_OPM4               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_200kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_200kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #5 */
#define SUN_FSK_BAND_915_918_OPM5               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_300kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_300kHz_CCA_THRESHOLD, \
		} \
	}

/************************ Frequency band 917-923.5 MHz ************************/

/* Operating mode #1 */
#define SUN_FSK_BAND_917_923_OPM1               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_50kHz,	\
			.uc_modidx = AT86RF_FSK_MODIDX_1_0, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_50kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #2 */
#define SUN_FSK_BAND_917_923_OPM2               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_150kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_150kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #3 */
#define SUN_FSK_BAND_917_923_OPM3               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_200kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_200kHz_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 919-923 MHz *************************/

/* Operating mode #1 */
#define SUN_FSK_BAND_919_923_OPM1               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_50kHz,	\
			.uc_modidx = AT86RF_FSK_MODIDX_1_0, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_50kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #2 */
#define SUN_FSK_BAND_919_923_OPM2               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_100kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_100kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #3 */
#define SUN_FSK_BAND_919_923_OPM3               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_150kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_150kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #4 */
#define SUN_FSK_BAND_919_923_OPM4               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_200kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_200kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #5 */
#define SUN_FSK_BAND_919_923_OPM5               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_300kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_300kHz_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 920-928 MHz *************************/

/* Operating mode #1 */
#define SUN_FSK_BAND_920_928_OPM1               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_50kHz,	\
			.uc_modidx = AT86RF_FSK_MODIDX_1_0, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 920600000, \
			.ul_chn_spa_hz = 200000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 36, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_50kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #2 */
#define SUN_FSK_BAND_920_928_OPM2               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_100kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_1_0, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 920900000, \
			.ul_chn_spa_hz = 400000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 17, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_100kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #3 */
#define SUN_FSK_BAND_920_928_OPM3               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_200kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_1_0, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 920800000, \
			.ul_chn_spa_hz = 600000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 11, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_200kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #4 */
#define SUN_FSK_BAND_920_928_OPM4               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_200kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_1_0, \
			.uc_modord = AT86RF_FSK_MODORD_4FSK \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 920800000, \
			.ul_chn_spa_hz = 600000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 11, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_200kHz_CCA_THRESHOLD + 3, \
		} \
	}

/* Operating mode #5 */
#define SUN_FSK_BAND_920_928_OPM5               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_200kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_1_0, \
			.uc_modord = AT86RF_FSK_MODORD_4FSK \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 920900000, \
			.ul_chn_spa_hz = 400000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 17, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_200kHz_CCA_THRESHOLD + 3, \
		} \
	}

/* Operating mode #6 */
#define SUN_FSK_BAND_920_928_OPM6               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_150kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 920900000, \
			.ul_chn_spa_hz = 400000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 17, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_150kHz_CCA_THRESHOLD + 3, \
		} \
	}

/* Operating mode #7 */
#define SUN_FSK_BAND_920_928_OPM7               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_300kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 920900000, \
			.ul_chn_spa_hz = 400000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 17, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_300kHz_CCA_THRESHOLD + 3, \
		} \
	}

/* Operating mode #8 */
#define SUN_FSK_BAND_920_928_OPM8               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_300kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 920800000, \
			.ul_chn_spa_hz = 600000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 11, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_300kHz_CCA_THRESHOLD + 3, \
		} \
	}

/* Operating mode #9 */
#define SUN_FSK_BAND_920_928_OPM9               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_400kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 921000000, \
			.ul_chn_spa_hz = 1000000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 6, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_400kHz_CCA_THRESHOLD + 3, \
		} \
	}

/* Operating mode #12 */
#define SUN_FSK_BAND_920_928_OPM12               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_400kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_1_0, \
			.uc_modord = AT86RF_FSK_MODORD_4FSK \
		}, \
		.x_chn_cfg = { \
			.ul_f0_hz = 921000000, \
			.ul_chn_spa_hz = 1000000, \
			.us_chn_num_min = 0, \
			.us_chn_num_max = 6, \
			.us_chn_num_min2 = 0xFFFF, \
			.us_chn_num_max2 = 0 \
		}, \
		.x_cca_ed_cfg = { \
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_400kHz_CCA_THRESHOLD + 3, \
		} \
	}

/*********************** Frequency band 920.5-924.5MHz ************************/

/* Operating mode #1 */
#define SUN_FSK_BAND_920_924_OPM1               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_50kHz,	\
			.uc_modidx = AT86RF_FSK_MODIDX_1_0, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_50kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #2 */
#define SUN_FSK_BAND_920_924_OPM2               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_100kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_100kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #3 */
#define SUN_FSK_BAND_920_924_OPM3               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_150kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_150kHz_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 920-925 MHz *************************/

/* Operating mode #1 */
#define SUN_FSK_BAND_920_925_OPM1               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_50kHz,	\
			.uc_modidx = AT86RF_FSK_MODIDX_1_0, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_50kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #2 */
#define SUN_FSK_BAND_920_925_OPM2               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_100kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_100kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #3 */
#define SUN_FSK_BAND_920_925_OPM3               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_150kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_150kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #4 */
#define SUN_FSK_BAND_920_925_OPM4               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_200kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_200kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #5 */
#define SUN_FSK_BAND_920_925_OPM5               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_300kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_300kHz_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 920-923 MHz *************************/

/* Operating mode #1 */
#define SUN_FSK_BAND_920_923_OPM1               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_50kHz,	\
			.uc_modidx = AT86RF_FSK_MODIDX_1_0, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_50kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #2 */
#define SUN_FSK_BAND_920_923_OPM2               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_100kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_100kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #3 */
#define SUN_FSK_BAND_920_923_OPM3               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_150kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_150kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #4 */
#define SUN_FSK_BAND_920_923_OPM4               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_200kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_200kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #5 */
#define SUN_FSK_BAND_920_923_OPM5               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_300kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_300kHz_CCA_THRESHOLD, \
		} \
	}

/************************* Frequency band 2400-2483.5 MHz *********************/

/* Operating mode #1 */
#define SUN_FSK_BAND_2400_2483_OPM1               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_50kHz,	\
			.uc_modidx = AT86RF_FSK_MODIDX_1_0, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_50kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #2 */
#define SUN_FSK_BAND_2400_2483_OPM2               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_150kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_150kHz_CCA_THRESHOLD, \
		} \
	}

/* Operating mode #3 */
#define SUN_FSK_BAND_2400_2483_OPM3               (at86rf_phy_cfg_t) \
	{ \
		.uc_phy_mod = AT86RF_PHY_MOD_FSK, \
		.u_mod_cfg.x_fsk = { \
			.uc_symrate = AT86RF_FSK_SYMRATE_200kHz, \
			.uc_modidx = AT86RF_FSK_MODIDX_0_5, \
			.uc_modord = AT86RF_FSK_MODORD_2FSK \
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
			.us_duration_us = SUN_FSK_50kHz_CCA_ED_DURATION, \
			.sc_threshold_dBm = SUN_FSK_200kHz_CCA_THRESHOLD, \
		} \
	}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */

#endif /* IEEE_15_4_SUN_FSK_H_INCLUDED */
