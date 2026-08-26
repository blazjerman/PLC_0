/**
 *
 * \file
 *
 * \brief RF215 Baseband Core definitions.
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

#ifndef RF215_BBC_DEFS_H_INCLUDE
#define RF215_BBC_DEFS_H_INCLUDE

/* AT86RF215 includes */
#include "at86rf_defs.h"
#include "rf215_reg.h"

#ifdef __cplusplus
extern "C" {
#endif

/** FSK symbol rate in Hz */
#define FSK_SYMRATE_Hz           {50000, 100000, 150000, 200000, 300000, 400000}
/** FSK symbol rate in kHz */
#define FSK_SYMRATE_kHz          {50, 100, 150, 200, 300, 400}

/** OFDM bandwidth in Hz (bandwidth option) */
#define OFDM_BANDWIDTH_Hz        {1094000, 552000, 281000, 156000}
/** OFDM number of data subcarriers (bandwidth option) */
#define OFDM_DATA_CARRIERS       {96, 48, 24, 12}
/** OFDM number of PHR symbols (phyOfdmInterleaving, bandwidth option) */
#define OFDM_PHR_SYMBOLS         {{3, 6, 6, 6},	\
				  {4, 8, 6, 6} \
}
/** OFDM frequency spreading repetition factor shift: log2(factor) (MCS) */
#define OFDM_REP_FACT_SHIFT      {2, 1, 1, 0, 0, 0, 0}
/** OFDM bits per subcarrier shift: log2(bits_per_carrier) (MCS) */
#define OFDM_BITS_CARRIER_SHIFT  {0, 0, 1, 1, 1, 2, 2}
/** OFDM symbol duration in us */
#define OFDM_SYMB_DURATION_US    120

/** Receiver sensitivity for FSK in dBm (symbol rate).
 * From IEEE 802.15.4 section 19.6.7 Receiver sensitivity:
 * S = (S0 + 10log[R/R0]) dBm. Where S0 = -91 (without FEC) / -97 (with FEC);
 * R0 = 50 kb/s; R = bit rate in kb/s
 * Sensitivity without FEC is selected (worst case) */
#define FSK_SENSITIVITY_dBm      {-91, -88, -86, -85, -83, -82}

/** Receiver sensitivity for OFDM in dBm (bandwidth option).
 * From IEEE 802.15.4 section 20.5.3 Receiver sensitivity:
 * Table 20-21—Sensitivity requirements for OFDM options and MCS levels
 * Sensitivity for MCS6 is selected (worst case) */
#define OFDM_SENSITIVITY_dBm     {-85, -88, -91, -94}

/** aTurnaroundTime: RX-to-TX or TX-to-RX turnaround time. From IEEE
 * 802.15.4 Table 11-1—PHY constants: For the SUN, RS-GFSK, TVWS, and LECIM FSK
 * PHYs, the value is 1 ms expressed in symbol periods, rounded up to the next
 * integer number of symbol periods using the ceiling() function.
 * For all FSK symbol rates it is 1 ms exact.
 * For all OFDM options it is 1.08 ms (9 symbols of 120 us). */
#define FSK_TURNAROUND_TIME_US   1000
#define OFDM_TURNAROUND_TIME_US  1080

/** FSK modulation index subregisters values (FSKC0.MIDX/MIDXS) */
#define BBC_FSKC0_MIDX           {RF215_BBCn_FSKC0_MIDX_1_0 | \
				  RF215_BBCn_FSKC0_MIDXS_1_0, \
				  RF215_BBCn_FSKC0_MIDX_0_5 | \
				  RF215_BBCn_FSKC0_MIDXS_1_0}

/** OFDM preamble detection threshold subregisters values (OFDMSW.PDT)
 * Ordered by OFDM bandwith option (1, 2, 3, 4) [Table 6-93]
 * Option 1 and 4: higher treshold to avoid false detections (spurious) */
#define BBC_OFDMSW_PDT           {RF215_BBCn_OFDMSW_PDT(6), \
				  RF215_BBCn_OFDMSW_PDT(5), \
				  RF215_BBCn_OFDMSW_PDT(4), \
				  RF215_BBCn_OFDMSW_PDT(4)}

/** BBCn_IRQM register value */
#define BBC_IRQM_CFG             (RF215_BBCn_IRQ_RXFS | RF215_BBCn_IRQ_RXFE | \
	RF215_BBCn_IRQ_TXFE | RF215_BBCn_IRQ_AGCH | RF215_BBCn_IRQ_AGCR | \
	RF215_BBCn_IRQ_FBLI)

/** PC.TXAFCS/FCSFE subregister values */
#ifdef AT86RF215_ENABLE_AUTO_FCS
/* PC.TXAFCS: Enable Transmitter Auto Frame Check Sequence */
# define BBC_PC_TXAFCS           RF215_BBCn_PC_TXAFCS_EN
/* PC.FCSFE:: Enable Frame Check Sequence Filter */
# define BBC_PC_FCSFE            RF215_BBCn_PC_FCSFE_EN
#else
/* PC.TXAFCS: Disable Transmitter Auto Frame Check Sequence */
# define BBC_PC_TXAFCS           RF215_BBCn_PC_TXAFCS_DIS
/* PC.FCSFE: Disable Frame Check Sequence Filter */
# define BBC_PC_FCSFE            RF215_BBCn_PC_FCSFE_DIS
#endif

/** BBCn_PC register value (common for baseband enabled/disabled)
 * PT: PHY Type (pt enum at86rf_phy_mod_t)
 * FCST: Only 32-bit FCS supported by now */
#define BBC_PC_COMMON(pt)        (RF215_BBCn_PC_PT(pt + 1) | \
	RF215_BBCn_PC_FCST_32 | BBC_PC_TXAFCS | BBC_PC_FCSFE)

/** BBCn_PC register value (Baseband enabled)
 * BBEN: Baseband Core Enable */
#define BBC_PC_CFG_BBEN(pt)      (RF215_BBCn_PC_BBEN_ON | BBC_PC_COMMON(pt))

/** BBCn_PC register value (Baseband disabled)
 * BBEN: Baseband Core Disabled */
#define BBC_PC_CFG_BBDIS(pt)     (RF215_BBCn_PC_BBEN_OFF | BBC_PC_COMMON(pt))

/** BBCn_FSKPHRTX register value (common for FEC enabled/disabled)
 * RB1/2: Reserved bits to 0
 * DW: Data Whitening enabled */
#define BBC_FSKPHRTX_COMMON      (RF215_BBCn_FSKPHRTX_DW_EN | \
	RF215_BBCn_FSKPHRTX_RB1(0) | RF215_BBCn_FSKPHRTX_RB2(0))

/** BBCn_FSKPHRTX register value (FEC disabled)
 * RB0/1: Reserved bits to 0 0
 * DW: Data Whitening enabled
 * SFD: SFD0 used (uncoded) */
#define BBC_FSKPHRTX_FEC_OFF      (RF215_BBCn_FSKPHRTX_SFD_0 | \
	BBC_FSKPHRTX_COMMON)

/** BBCn_FSKPHRTX register value (FEC enabled)
 * RB0/1: Reserved bits to 0 0
 * DW: Data Whitening enabled
 * SFD: SFD1 used (coded) */
#define BBC_FSKPHRTX_FEC_ON       (RF215_BBCn_FSKPHRTX_SFD_1 | \
	BBC_FSKPHRTX_COMMON)

/** FSKPHRRX.FCST mask and value to check received PHR */
#ifdef AT86RF215_ENABLE_AUTO_FCS
/* Check FCST subregister */
# define BBC_FSKPHRRX_FCST_MASK   RF215_BBCn_FSKPHRRX_FCST_Msk
# define BBC_FSKPHRRX_FCST_VAL    RF215_BBCn_FSKPHRRX_FCST_32
#else
/* Not check FCST subregister */
# define BBC_FSKPHRRX_FCST_MASK   0
# define BBC_FSKPHRRX_FCST_VAL    0
#endif

/** BBCn_FSKPHRRX mask to check received PHR */
#define BBC_FSKPHRRX_MASK         (RF215_BBCn_FSKPHRRX_DW_Msk |	\
	RF215_BBCn_FSKPHRRX_SFD_Msk | RF215_BBCn_FSKPHRRX_MS | \
	BBC_FSKPHRRX_FCST_MASK)

/** BBCn_FSKPHRRX register value (common for FEC enabled/disabled)
 * DW: Data Whitening enabled
 * MS: Mode switch not used (0)
 * FCST: FCS type 32-bit ( only if AT86RF215_ENABLE_AUTO_FCS) */
#define BBC_FSKPHRRX_COMMON      (RF215_BBCn_FSKPHRRX_DW_EN | \
	BBC_FSKPHRRX_FCST_VAL)

/** BBCn_FSKPHRRX register value (FEC disabled)
 * DW: Data Whitening enabled
 * SFD: SFD0 used (uncoded)
 * MS: Mode switch not used (0)
 * FCST: FCS type 32-bit ( only if AT86RF215_ENABLE_AUTO_FCS) */
#define BBC_FSKPHRRX_FEC_OFF      (RF215_BBCn_FSKPHRRX_SFD_0 | \
	BBC_FSKPHRRX_COMMON)

/** BBCn_FSKPHRRX register value (FEC enabled)
 * DW: Data Whitening enabled
 * SFD: SFD1 used (coded)
 * MS: Mode switch not used (0)
 * FCST: FCS type 32-bit ( only if AT86RF215_ENABLE_AUTO_FCS) */
#define BBC_FSKPHRRX_FEC_ON       (RF215_BBCn_FSKPHRRX_SFD_1 | \
	BBC_FSKPHRRX_COMMON)

/** FSK Direct modulation and pre-emphashis settings. Recommended to enable
 * direct modulation and pre-emphasis filtering to improve the modulation
 * quality [Table 6-57].
 * 4 register values: BBCn_FSKDM, BBCn_FSKPE0, BBCn_FSKPE1, BBCn_FSKPE2
 * Ordered by FSK symbol rate (50, 100, 150, 200, 300, 400 kHz) */
#define BBC_FSKDM                (RF215_BBCn_FSKDM_EN | RF215_BBCn_FSKDM_PE)
#define BBC_FSK_DM_PE            {{BBC_FSKDM, 0x02, 0x03, 0xFC}, \
				  {BBC_FSKDM, 0x0E, 0x0F, 0xF0}, \
				  {BBC_FSKDM, 0x3C, 0x3F, 0xC0}, \
				  {BBC_FSKDM, 0x74, 0x7F, 0x80}, \
				  {BBC_FSKDM, 0x05, 0x3C, 0xC3}, \
				  {BBC_FSKDM, 0x13, 0x29, 0xC7}	\
}

/** BBCn_OFDMPHRTX register value (FEC disabled)
 * RB5/17/RB18/RB21: Reserved bits to 0 0
 * MCS: Modulation and coding shceme (assumed that 0<=mcs<=6 */
#define BBC_OFDMPHRTX(mcs)       RF215_BBCn_OFDMPHRTX_MCS(mcs)

/** Transmitter baseband processing delay in us [uQ6.5] [Table 6-2] */

/* tx_bb_delay FSK: Ordered by FSK symbol rate (50, 100, 150, 200, 300, 400 kHz)
 * 42.0, 21.0, 19.0, 11.0, 9.5, 5.5 us */
#define BBC_TX_BB_DELAY_FSK      {1344, 672, 608, 352, 304, 176}

/* tx_bb_delay OFDM: <3.5 us. Ordered by OFDM bandwith option (1, 2, 3, 4)
 * 0.5, 0.5, 0.5, 2.5 us */
#define BBC_TX_BB_DELAY_OFDM     {16, 16, 16, 80}

/** FSK pre-emphasis processing delay in us [uQ5.5] (not in datasheet).
 * If pre-emphasis is enabled (FSKDM.PE=1), tx_bb_delay is reduced because
 * FSKC0.BT has no effect (GFSK modulator disabled). Delay1 and Delay2 reduce
 * tx_bb_delay. Delay2 has to be compensated in TX confirm time.
 * Ordered by FSK symbol rate (50, 100, 150, 200, 300, 400 kHz)
 * Delay1: 20.0, 10.0, 5.0, 5.0, 2.75, 2.75 us
 * Delay2: 7.66, 3.5, 4.66, 1.0, 1.75, 1.0 us */
#define BBC_TX_PE_DELAY1_FSK     {640, 320, 160, 160, 88, 88}
#define BBC_TX_PE_DELAY2_FSK     {245, 112, 149, 32, 56, 32}

/** Receiver baseband processing delay in us [uQ7.5] (not in datasheet) */

/* FSK: Ordered by FSK symbol rate (50, 100, 150, 200, 300, 400 kHz)
 * Common: 0.31, 2.34, 7.97, 2.34, 4.0, 1.31 us
 * FEC enabled: 1.66, 1.34, 1.22, 1.16, 1.25, 1.66 us */
#define BBC_RX_BB_DELAY_FSK      {10, 75, 255, 75, 128, 42}
#define BBC_RX_BB_DELAY_FSK_FEC  {53, 44, 39, 37, 40, 53}

/* OFDM: Ordered by OFDM bandwith option (1, 2, 3, 4)
 * phyOfdmInterleaving=0: 91.25, 61.75, 52, 48.5 us
 * phyOfdmInterleaving=1: 97.25, 63.63, 54.31, 50.38 us */
#define BBC_RX_BB_DELAY_OFDM     {{2920, 1976, 1664, 1552}, \
				  {3112, 2036, 1738, 1612} \
}

/** Struct definition for BBC configuration parameters */
typedef struct bbc_params {
	uint16_t us_rx_proc_delay_us_q5;
	uint16_t us_rx_bb_delay_us_q5;
	uint8_t puc_fsk_cfg_regs[4];
	uint8_t puc_fsk_dm_pe_regs[4];
	uint8_t puc_ofdm_cfg_regs[3];
	uint8_t puc_tx_auto_regs[2];
	at86rf_fsk_fec_t uc_fsk_tx_fec;
	at86rf_ofdm_mcs_t uc_ofdm_tx_mcs;
} bbc_params_t;

/** RF215 internal global variables declared as extern */
extern uint8_t guc_spi_byte_time_us_q5;

#ifdef __cplusplus
}
#endif

#endif  /* RF215_BBC_DEFS_H_INCLUDE */
