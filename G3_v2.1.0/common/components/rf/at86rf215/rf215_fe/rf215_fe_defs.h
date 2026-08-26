/**
 *
 * \file
 *
 * \brief RF215 Transmitter and Receiver Frontend definitions.
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

#ifndef RF215_FE_DEFS_H_INCLUDE
#define RF215_FE_DEFS_H_INCLUDE

/* AT86RF215 includes */
#include "rf215_reg.h"

#ifdef __cplusplus
extern "C" {
#endif

/** TX/RX DFE sampling rate (TXDFE.SR/RXDFE.SR) [Table 6-51]. */
/* For FSK PHY: Ordered by FSK symbol rate (50, 100, 150, 200, 300, 400 kHz) */
#define TXDFE_SR_FSK         {RF215_RFn_TXDFE_SR_500kHz, \
			      RF215_RFn_TXDFE_SR_1000kHz, \
			      RF215_RFn_TXDFE_SR_2000kHz, \
			      RF215_RFn_TXDFE_SR_2000kHz, \
			      RF215_RFn_TXDFE_SR_4000kHz, \
			      RF215_RFn_TXDFE_SR_4000kHz}
#define RXDFE_SR_FSK         {RF215_RFn_TXDFE_SR_400kHz, \
			      RF215_RFn_TXDFE_SR_800kHz, \
			      RF215_RFn_TXDFE_SR_1000kHz, \
			      RF215_RFn_TXDFE_SR_1000kHz, \
			      RF215_RFn_TXDFE_SR_2000kHz, \
			      RF215_RFn_TXDFE_SR_2000kHz}
/* For OFDM PHY: Ordered by OFMD Option (1, 2, 3, 4) */
#define TXDFE_SR_OFDM        {RF215_RFn_TXDFE_SR_1333kHz, \
			      RF215_RFn_TXDFE_SR_1333kHz, \
			      RF215_RFn_TXDFE_SR_667kHz, \
			      RF215_RFn_TXDFE_SR_667kHz}
#define RXDFE_SR_OFDM        {RF215_RFn_TXDFE_SR_1333kHz, \
			      RF215_RFn_TXDFE_SR_1333kHz, \
			      RF215_RFn_TXDFE_SR_667kHz, \
			      RF215_RFn_TXDFE_SR_667kHz}

/** Transmitter frontend delay in us [uQ6.5] [Figure 6-3] */
/* tx_start_delay: Typ. 4 us [Table 10-7] */
#define TXFE_START_DELAY      (4 << 5)
/* tx_proc_delay:  Ordered by TXDFE.SR (1, 2, 3, 4, 5, 6, 8, 10) [Table 6-1] */
/* Delay wih TXDFE.RCUT=4: 2.0, 4.0, 5.25, 6.25, 8.3, 8.5, 10.25, 13.75 us */
#define TXDFE_PROC_DELAY      {64, 128, 168, 200, 266, 272, 0, 328, 0, 440}
/* Delay wih TXDFE.RCUT!=4: 4.0, 8.5, 11.0, 15.0, 18.5, 20.5, 28.5, 34.0 us */
#define TXDFE_PROC_DELAY_RCUT {128, 272, 352, 480, 592, 656, 0, 912, 0, 1088}

/** AGC Update Time Tu in us [uQ8.5] [Figure 6-7] */
/** AGCC.AGCI=0: Ordered by TXDFE.SR (1, 2, 3, 4, 5, 6, 8, 10) [Table 6-12] */
/* AGCC.AVGS=0: 8.75, 14.5, 21.75, 29, 36.25, 43.5, 48, 65 us */
/* AGCC.AVGS=1: 10.75, 18.5, 27.75, 37, 46.25, 55.5, 74, 85 us */
/* AGCC.AVGS=2: 14.75, 26.5, 39.75, 53, 66.25, 79.5, 106, 125 us */
/* AGCC.AVGS=3: 22.75, 42.5, 63.75, 85, 106.25, 127.5, 170, 205 us */
#define RXFE_AGC_UPD_TIME_0   {{280, 464, 696, 928, 1160, 1392, 0, 1536, 0, 2080}, \
			       {344, 592, 888, 1184, 1480, 1776, 0, 2368, 0, 2720}, \
			       {472, 848, 1272, 1696, 2120, 2544, 0, 3392, 0, 4000}, \
			       {728, 1360, 2040, 2720, 3400, 4080, 0, 5440, 0, 6560} \
}
/** AGCC.AGCI=1: Ordered by TXDFE.SR (1, 2, 3, 4, 5, 6, 8, 10) [Table 6-12] */
/* AGCC.AVGS=0: 6.75, 10.5, 15.75, 21, 26.25, 31.5, 42, 45 us */
/* AGCC.AVGS=1: 8.75, 14.5, 21.75, 29, 36.25, 43.5, 58, 65 us */
/* AGCC.AVGS=2: 12.75, 22.5, 33.75, 45, 56.25, 67.5, 90, 105 us */
/* AGCC.AVGS=3: 20.75, 38.5, 57.75, 77, 96.25, 115.5, 154, 185 us */
#define RXFE_AGC_UPD_TIME_1   {{216, 336, 504, 672, 840, 1008, 0, 1344, 0, 1440}, \
			       {280, 464, 696, 928, 1160, 1392, 0, 1856, 0, 2080}, \
			       {408, 720, 1080, 1440, 1800, 2160, 0, 2880, 0, 3360}, \
			       {664, 1232, 1848, 2464, 3080, 3696, 0, 4928, 0, 5920} \
}

/** Receiver frontend delay in us [uQ5.5] (not in datasheet) */
/* Measured empirically to adjust TX+RX delay */
/* Ordered by RXDFE.SR (1, 2, 3, 4, 5, 6, 8, 10) */
/* Delay wih RXDFE.RCUT=4: 0.125 us */
#define RXDFE_PROC_DELAY      4
/* Delay wih RXDFE.RCUT!=4: 1.75, 3.5, 5.25, 7.0, 8.75, 10.5, 14, 17.5 us */
#define RXDFE_PROC_DELAY_RCUT {56, 112, 168, 224, 280, 336, 0, 448, 0, 560}

/** Power Amplifier Ramp Time (TXCUTC.PARAMP) [Table 6-53] */
/* For FSK PHY: Ordered by FSK symbol rate (50, 100, 150, 200, 300, 400 kHz) */
#define TXPA_PARAMP_FSK       {RF215_RFn_TXCUTC_PARAMP_32us, \
			       RF215_RFn_TXCUTC_PARAMP_16us, \
			       RF215_RFn_TXCUTC_PARAMP_16us, \
			       RF215_RFn_TXCUTC_PARAMP_16us, \
			       RF215_RFn_TXCUTC_PARAMP_8us, \
			       RF215_RFn_TXCUTC_PARAMP_8us}
/* For OFDM PHY: Fixed */
#define TXPA_PARAMP_OFDM       RF215_RFn_TXCUTC_PARAMP_4us

/** Transmitter Analog Frontend low-pass cut-off frequencies (TXCUTC.LPFCUT) or
 * Receiver Analog Frontend band-pass half bandwidth (RXBWC.BW) in Hz */
#define TXRXAFE_CUTOFF_Hz     {80000, 100000, 125000, 160000, 200000, 250000, \
			       315000, 400000, 500000, 625000, 800000, 1000000}

/** TX/RX DFE maximum sampling rate frequency in Hz. fs = 4/SR MHz */
#define TXRXDFE_SR_MAX_Hz     4000000

/** RX Automatic Gain Control (AGC) configuration for FSK PHY */

/* AGCC.EN: Enable AGC
 * AGCC.AVGS: 8 samples for averaging [Tables 6-60 to 6-63]
 * AGCC.AGCI: use x0 signal (post-filtered) for AGC
 * AGCC.RSV: keep initial reset value 1 ?? */
#define RXFE_AGCC_FSK         (RF215_RFn_AGCC_EN | RF215_RFn_AGCC_AVGS_8SAMP | \
	RF215_RFn_AGCC_AGCI_x0 | RF215_RFn_AGCC_RSV)

/* AGCS.TGT: AGC target level -24dB [Tables 6-60 to 6-63]
 * AGCS.GCW: keep initial reset value 23 */
#define RXFE_AGCS_FSK         (RF215_RFn_AGCS_GCW(23) | RF215_RFn_AGCS_TGT_24dB)

/** RX Automatic Gain Control (AGC) configuration for OFDM PHY */

/* AGCC.EN: Enable AGC
 * AGCC.AVGS: 8 samples for averaging [Table 6-93]
 * AGCC.AGCI: use x0 signal (post-filtered) for AGC
 * AGCC.RSV: keep initial reset value 1 ?? */
#define RXFE_AGCC_OFDM        (RF215_RFn_AGCC_EN | RF215_RFn_AGCC_AVGS_8SAMP | \
	RF215_RFn_AGCC_AGCI_x0 | RF215_RFn_AGCC_RSV)

/* AGCS.TGT: AGC target level -30dB [Table 6-92]
 * AGCS.GCW: keep initial reset value 23 */
#define RXFE_AGCS_OFDM        (RF215_RFn_AGCS_GCW(23) | RF215_RFn_AGCS_TGT_30dB)

/* If Spurious Compensation activated (OFDMPHRRX.SPC = 1)
 * AGCS.TGT: AGC target level -21dB
 * AGCS.GCW: keep initial reset value 23 */
#define RXFE_AGCS_OFDM_SPC    (RF215_RFn_AGCS_GCW(23) | RF215_RFn_AGCS_TGT_21dB)

/** Transmitter Power Amplifier Control (RFn_PAC)
 * PAC.PACUR: Power Amplifier Current Control. No power amplifier current
 * reduction to achieve maximum output power
 * PAC.TXPWR: Transmitter Output Power. Assumed that 0<=att<=31 */
#define TXPA_PAC(att)         (RF215_RFn_PAC_PACUR_0mA | \
	RF215_RFn_PAC_TXPWR(31 - att))

/** Minimum TX Power attenuation (PAC.TXPWR) to comply with EVM requeriments [Table 6-91] */
/* For OFDM PHY: Ordered by OFDM MCS (0, 1, 2, 3, 4, 5, 6) */
#define TXPA_TX_ATT_MIN_OFDM  {0, 0, 0, 0, 1, 3, 5}

/** RFn_EDD register value for Automatic Energy Detection Mode (triggered by
 * reception). Set to maximum duration in order to have the best energy
 * averaging. It is not a problem if the frame duration is less than energy
 * duration, the value is still correct (although the datasheet says that it is
 * incorrect or invalid).
 * DTB: 128 us
 * DF: 63 (6 bits) */
#define RXFE_EDD_AUTO     (RF215_RFn_EDD_DTB_128us | RF215_RFn_EDD_DF(63))

/** Struct definition for BBC configuration parameters */
typedef struct fe_params {
	uint16_t us_agc_upd_time_us_q5;
	uint8_t puc_txfe_regs[2];
	uint8_t puc_rxfe_regs[7];
	uint8_t uc_txpwr_att;
} fe_params_t;

#ifdef __cplusplus
}
#endif

#endif  /* RF215_FE_DEFS_H_INCLUDE */
