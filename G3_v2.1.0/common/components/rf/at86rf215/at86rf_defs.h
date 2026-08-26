/**
 *
 * \file
 *
 * \brief Definitions for AT86RF215 RF transceiver component.
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

#ifndef AT86RF_DEFS_H_INCLUDED
#define AT86RF_DEFS_H_INCLUDED

/* AT86RF215 includes */
#include "conf_at86rf.h"

/* System includes */
#include "compiler.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

/* ! \name AT86RF215 Device Part Definition */
/* @{ */
/* Dual band transceiver and I/Q radio */
/* Embedded baseband supporting MR-FSK, MR-OFDM, MR-O-QPSK, and O-QPSK */
#define AT86RF_PART_AT86RF215                0
/* I/Q radio (13-bit I/Q low voltage differential signal (LVDS) interface) */
/* Not supported */
#define AT86RF_PART_AT86RF215Q               1
/* Sub-1GHz Transceiver and I/Q radio */
/* Embedded baseband supporting MR-FSK, MR-OFDM, MR-O-QPSK, and O-QPSK */
#define AT86RF_PART_AT86RF215M               2

#if (AT86RF_PART == AT86RF_PART_AT86RF215M)
# ifndef AT86RF215_DISABLE_RF24_TRX
/* AT86RF215M doesn't have the 2.4GHz Transceiver */
#  define AT86RF215_DISABLE_RF24_TRX
# endif
#endif
/* @} */

/* ! \name AT86RF215 TRX identifier (RF09, RF24). Used in AT86RF215 API to
 * select the corresponding TRX */
/* @{ */
/** Number of enabled Transceivers */
#if (defined(AT86RF215_DISABLE_RF09_TRX) || defined(AT86RF215_DISABLE_RF24_TRX))
/* Single TRX Mode */
# define AT86RF_NUM_TRX                      1
#else
/* Dual TRX Mode */
# define AT86RF_NUM_TRX                      2
#endif
/** RF09 (Sub-1GHz) TRX ID */
#ifndef AT86RF215_DISABLE_RF09_TRX
# define AT86RF_TRX_RF09_ID                  0
#endif
/** RF24 (2.4GHz) TRX ID */
#ifndef AT86RF215_DISABLE_RF24_TRX
# define AT86RF_TRX_RF24_ID                  (AT86RF_NUM_TRX - 1)
#endif
/* @} */

/* ! \name Define default AT86RF215 configurations if not defined in
 * conf_at86rf.h */
/* @{ */
/* Maximum PSDU data length, including FCS */
#ifndef AT86RF215_MAX_PSDU_LEN
# define AT86RF215_MAX_PSDU_LEN              2047
#endif

/* Number of reception buffers. Maximum number of RX indications that can be
 * stored without calling at86rf_event_handler() */
#ifndef AT86RF215_NUM_RX_BUFFERS
# define AT86RF215_NUM_RX_BUFFERS            1
#endif

/* Number of programmed TX buffers (instantaneous TX doesn't need buffer) */
#ifndef AT86RF215_NUM_TX_PROG_BUFFERS
# define AT86RF215_NUM_TX_PROG_BUFFERS       1
#endif
/* @} */

/* ! \name Definition of AT86RF215 exception types (bitmask) */
/* @{ */
/* Error in the SPI interface */
#define AT86RF_EXCEPTION_SPI_ERR             (1 << 0)
/* AT86RF215 chip didn't behave as expected during initialization (or reset) */
#define AT86RF_EXCEPTION_INIT_ERR            (1 << 1)
/* Unexpected chip reset. The transceiver was reinitialized correctly */
#define AT86RF_EXCEPTION_RESET               (1 << 2)
/* @} */

/* ! \name FCS length in bytes */
/* @{ */
#ifdef AT86RF215_ENABLE_AUTO_FCS
/* FCS type fixed to 32 bits */
# define AT86RF_FCS_LEN                      4
#else
/* Automatic calculation of FCS disabled */
# define AT86RF_FCS_LEN                      0
#endif
/* @} */

/* ! \name Enumeration of AT86RF215 interface results */
/* @{ */
typedef enum at86rf_res {
	AT86RF_SUCCESS = 0,
	AT86RF_INVALID_TRX_ID = 1,
	AT86RF_INVALID_PARAM = 2,
	AT86RF_INVALID_CFG = 3,
	AT86RF_INVALID_ATTR = 4,
	AT86RF_READ_ONLY = 5,
	AT86RF_WRITE_ONLY = 6,
	AT86RF_NOT_INIT = 7,
	AT86RF_NOT_ENABLED = 8,
	AT86RF_ERROR = 9,
} at86rf_res_t;

typedef enum at86rf_cca_res {
	AT86RF_CCA_FREE = 0,
	AT86RF_CCA_BUSY_RX = 1,
	AT86RF_CCA_BUSY_CHN = 2,
	AT86RF_CCA_BUSY_TX = 3,
	AT86RF_CCA_INVALID_TRX_ID = 4,
	AT86RF_CCA_INVALID_PARAM = 5,
	AT86RF_CCA_TRX_SLEPT = 6,
	AT86RF_CCA_NOT_ENABLED = 7,
} at86rf_cca_res_t;
/* @} */

/* ! \name AT86RF215 PHY parameters */
/* @{ */
/* Enumeration of AT86RF215 FSK symbol rates */
typedef enum at86rf_fsk_symrate {
	AT86RF_FSK_SYMRATE_50kHz = 0,
	AT86RF_FSK_SYMRATE_100kHz = 1,
	AT86RF_FSK_SYMRATE_150kHz = 2,
	AT86RF_FSK_SYMRATE_200kHz = 3,
	AT86RF_FSK_SYMRATE_300kHz = 4,
	AT86RF_FSK_SYMRATE_400kHz = 5,
} at86rf_fsk_symrate_t;

/* Enumeration of AT86RF215 FSK modulation indexes. It defines the maximum
 * frequency deviation, fdev = (SymbRate * ModIndex) / 2 */
typedef enum at86rf_fsk_modidx {
	/* ModIndex = 1.0 */
	AT86RF_FSK_MODIDX_1_0 = 0,
	/* ModIndex = 0.5 */
	AT86RF_FSK_MODIDX_0_5 = 1,
} at86rf_fsk_modidx_t;

/* Enumeration of AT86RF215 FSK modulation orders */
typedef enum at86rf_fsk_modord {
	/* 2-FSK. Frequency deviations: {-fdev, +fdev} */
	AT86RF_FSK_MODORD_2FSK = 0,

	/* 4-FSK. Frequency deviations: {-fdev, -fdev/3, +fdev/3, +fdev}.
	 * The interpretation of modulation index for 4-FSK is slightly
	 * different to 802.15.4 specification. 4-FSK & ModIndex=0.33 (802.15.4)
	 * would be 4-FSK & ModIndex=1.0 (AT86RF215)
	 * Restriction for AT86RF215: ModIndex>=1.0 for 4-FSK */
	AT86RF_FSK_MODORD_4FSK = 1,
} at86rf_fsk_modord_t;

/* Struct of AT86RF215 FSK configuration */
typedef struct at86rf_fsk_cfg {
	at86rf_fsk_symrate_t uc_symrate;
	at86rf_fsk_modidx_t uc_modidx;
	at86rf_fsk_modord_t uc_modord;
} at86rf_fsk_cfg_t;

/* Enumeration of AT86RF215 OFDM options */
typedef enum at86rf_ofdm_opt {
	AT86RF_OFDM_OPT_1 = 0,
	AT86RF_OFDM_OPT_2 = 1,
	AT86RF_OFDM_OPT_3 = 2,
	AT86RF_OFDM_OPT_4 = 3,
} at86rf_ofdm_opt_t;

/* Enumeration of AT86RF215 OFDM interleaving modes
 * (phyOFDMInterleaving from 802.15.4 specification) */
typedef enum at86rf_ofdm_itlv_mode {
	/* Interleaving depth: one symbol */
	AT86RF_OFDM_INTERLEAVING_0 = 0,
	/* Interleaving depth: frequency domain spreading factor symbols */
	AT86RF_OFDM_INTERLEAVING_1 = 1,
} at86rf_ofdm_itlv_mode_t;

/* Struct of AT86RF215 OFDM configuration */
typedef struct at86rf_ofmd_cfg {
	at86rf_ofdm_opt_t uc_opt;
	at86rf_ofdm_itlv_mode_t uc_itlv;
} at86rf_ofdm_cfg_t;

/* Union of AT86RF215 modulation (FSK/OFDM) configuration */
typedef union at86rf_mod_cfg {
	at86rf_fsk_cfg_t x_fsk;
	at86rf_ofdm_cfg_t x_ofdm;
} at86rf_mod_cfg_t;

/* Enumeration of AT86RF215 PHY modulations */
typedef enum at86rf_phy_mod {
	AT86RF_PHY_MOD_FSK = 0,
	AT86RF_PHY_MOD_OFDM = 1,
} at86rf_phy_mod_t;

/* Enumeration of AT86RF215 FSK Forward Error Correction (FEC) mode */
typedef enum at86rf_fsk_fec {
	AT86RF_FSK_FEC_OFF = 0,
	AT86RF_FSK_FEC_ON = 1,
} at86rf_fsk_fec_t;

/* Struct of AT86RF215 FSK frame parameters */
typedef struct at86rf_fsk_frame_params {
	/* Forward Error Correction (FEC): Enabled / Disabled */
	at86rf_fsk_fec_t uc_fec_enabled;
} at86rf_fsk_frame_params_t;

/* Enumeration of AT86RF215 OFDM modulation and coding scheme (MCS) */
typedef enum at86rf_ofdm_mcs {
	/* BPSK, 1/2 convolutional encoder rate, 4x frequency repetition */
	AT86RF_OFDM_MCS_0 = 0,
	/* BPSK, 1/2 convolutional encoder rate, 2x frequency repetition */
	AT86RF_OFDM_MCS_1 = 1,
	/* QPSK, 1/2 convolutional encoder rate, 2x frequency repetition */
	AT86RF_OFDM_MCS_2 = 2,
	/* QPSK, 1/2 convolutional encoder rate */
	AT86RF_OFDM_MCS_3 = 3,
	/* QPSK, 3/4 convolutional encoder rate */
	AT86RF_OFDM_MCS_4 = 4,
	/* 16-QAM, 1/2 convolutional encoder rate */
	AT86RF_OFDM_MCS_5 = 5,
	/* 16-QAM, 3/4 convolutional encoder rate */
	AT86RF_OFDM_MCS_6 = 6,
} at86rf_ofdm_mcs_t;

#pragma pack(push,1)

/* Struct of AT86RF215 OFDM frame parameters */
typedef struct at86rf_ofdm_frame_params {
	at86rf_ofdm_mcs_t uc_mcs;
} at86rf_ofdm_frame_params_t;

/* Union of AT86RF215 modulation (FSK/OFDM) frame parameters */
typedef union at86rf_mod_frame_params {
	at86rf_fsk_frame_params_t x_fsk;
	at86rf_ofdm_frame_params_t x_ofdm;
} at86rf_mod_frame_params_t;

/* Sruct of AT86RF215 Channel Configuration */
typedef struct at86rf_chn_cfg {
	/* Channel center frequency F0 in Hz */
	uint32_t ul_f0_hz;
	/* Channel spacing in Hz */
	uint32_t ul_chn_spa_hz;
	/* Minimum channel number */
	uint16_t us_chn_num_min;
	/* Maximum channel number */
	uint16_t us_chn_num_max;
	/* Minimum channel number (second range) (0xFFFF for only one range) */
	uint16_t us_chn_num_min2;
	/* Maximum channel number (second range) (0x0000 for only one range) */
	uint16_t us_chn_num_max2;
} at86rf_chn_cfg_t;

/* Sruct of AT86RF215 Energy Detection for CCA Configuration */
typedef struct at86rf_cca_ed_cfg {
	/* Duration of Energy Detection for CCA in us */
	uint16_t us_duration_us;
	/* Threshold for CCA Energy Detection in dBm */
	int8_t sc_threshold_dBm;
} at86rf_cca_ed_cfg_t;

/* Struct of AT86RF215 PHY configuration */
typedef struct at86rf_phy_cfg {
	/* Channel configuration */
	at86rf_chn_cfg_t x_chn_cfg;
	/* Energy Detection for CCA configuration */
	at86rf_cca_ed_cfg_t x_cca_ed_cfg;
	/* PHY modulation (FSK / OFDM) */
	at86rf_phy_mod_t uc_phy_mod;
	/* Modulation-specific configuration */
	at86rf_mod_cfg_t u_mod_cfg;
} at86rf_phy_cfg_t;

#pragma pack(pop)

/* @} */

/* ! \name AT86RF215 PHY transmission parameters */
/* @{ */
/* Enumeration of AT86RF215 Clear Channel Assessment (CCA) modes */
typedef enum at86rf_cca {
	/* Energy above threshold only */
	AT86RF_CCA_MODE_1 = 0,
	/* Carrier sense only */
	AT86RF_CCA_MODE_2 = 1,
	/* Energy above threshold and carrier sense */
	AT86RF_CCA_MODE_3 = 2,
	/* ALOHA. CCA always reports idle medium */
	AT86RF_CCA_MODE_4 = 3,
	/* Transmit always, even if payload reception in progress */
	AT86RF_CCA_OFF = 4,
} at86rf_cca_t;

/* Enumeration of AT86RF215 TX time modes */
typedef enum at86rf_tx_time_mode {
	/* Absolute time, referred to system time (32-bit counter of 1us) */
	AT86RF_TX_TIME_ABS = 0,
	/* Relative time, referred to current system time */
	AT86RF_TX_TIME_REL = 1,
	/* Instantaneous: Transmit as soon as possible */
	AT86RF_TX_TIME_INST = 2,
	/* Cancel transmission previously programmed */
	AT86RF_TX_CANCEL = 3,
} at86rf_tx_time_mode_t;

/* Struct of AT86RF215 PHY TX parameters */
typedef struct at86rf_tx_params {
	/* Pointer to data to be transmitted */
	uint8_t *puc_data;
	/* Transmission initial time in us (absolute/relative) */
	uint32_t ul_tx_time;
	/* PSDU length in bytes (including FCS) */
	uint16_t us_psdu_len;
	/* Frequency channel number used for transmission */
	uint16_t us_chn_num;
	/* Modulation specific parameters (FSK/OFDM) */
	at86rf_mod_frame_params_t x_mod_params;
	/* Clear Channel Assessment (CCA) mode */
	at86rf_cca_t uc_cca_mode;
	/* TX time mode */
	at86rf_tx_time_mode_t uc_time_mode;
	/* Cancel TX if valid header received before programmed TX time */
	bool b_cancel_by_rx;
	/* Transmitter power attenuation in dB (max 31 dB) */
	uint8_t uc_txpwr_att;
	/* TX identifier. Used to identify TX in confirm and reprogram/cancel */
	uint8_t uc_tx_id;
	/* Contention window length (CW in IEEE 802.15.4 slotted CSMA-CA) */
	/* Number of CCA for clear channel (backoff unit period interval) */
	uint8_t uc_cw;
} at86rf_tx_params_t;

/* Struct of AT86RF215 PHY TX parameters */
typedef struct at86rf_set_chn_req_params {
	/* Initial channel switch time in us (absolute/relative) */
	uint32_t ul_time;
	/* Frequency channel number to be updated */
	uint16_t us_chn_num;
	/* Channel switch time mode */
	at86rf_tx_time_mode_t uc_time_mode;
} at86rf_set_chn_req_params_t;
/* @} */

/* Enumeration of AT86RF215 PHY PIB attributes */
typedef enum at86rf_pib_attr {
	/* RF device identifier. 16 bits */
	AT86RF_PIB_DEVICE_ID                        = 0x0000,
	/* RF PHY layer firmware version number. 6 bytes (see "at86rf_fw_version_t") */
	AT86RF_PIB_FW_VERSION                       = 0x0001,
	/* RF device reset (write-only) */
	AT86RF_PIB_DEVICE_RESET                     = 0x0002,
	/* RF transceiver (RF09 or RF24) reset (write-only) */
	AT86RF_PIB_TRX_RESET                        = 0x0080,
	/* RF transceiver (RF09 or RF24) sleep (write-only) */
	AT86RF_PIB_TRX_SLEEP                        = 0x0081,
	/* RF PHY configuration (see "at86rf_phy_cfg_t") */
	AT86RF_PIB_PHY_CONFIG                       = 0x0100,
	/* RF PHY band and operating mode. 16 bits (see "at86rf_phy_band_opm_t") */
	AT86RF_PIB_PHY_BAND_OPERATING_MODE          = 0x0101,
	/* RF channel number used for transmission and reception. 16 bits */
	AT86RF_PIB_PHY_CHANNEL_NUM                  = 0x0120,
	/* RF frequency in Hz used for transmission and reception. 32 bits (read-only) */
	AT86RF_PIB_PHY_CHANNEL_FREQ_HZ              = 0x0121,
	/* Configuration of Energy Detection for CCA. 3 bytes (see "at86rf_cca_ed_cfg_t") */
	AT86RF_PIB_PHY_CCA_ED_CONFIG                = 0x0140,
	/* Duration in us of Energy Detection for CCA. 16 bits */
	AT86RF_PIB_PHY_CCA_ED_DURATION_US           = 0x0141,
	/* Threshold in dBm for CCA with Energy Detection. 16 bits */
	AT86RF_PIB_PHY_CCA_ED_THRESHOLD_DBM         = 0x0142,
	/* Duration in symbols of Energy Detection for CCA. 16 bits */
	AT86RF_PIB_PHY_CCA_ED_DURATION_SYMBOLS      = 0x0143,
	/* Threshold in dB above sensitivity for CCA with Energy Detection. 16 bits */
	AT86RF_PIB_PHY_CCA_ED_THRESHOLD_SENSITIVITY = 0x0144,
	/* Turnaround time in us (aTurnaroundTime in IEEE 802.15.4). 16 bits (read-only) */
	AT86RF_PIB_PHY_TURNAROUND_TIME              = 0x0160,
	/* Number of payload symbols in last transmitted message. 16 bits */
	AT86RF_PIB_PHY_TX_PAY_SYMBOLS               = 0x0180,
	/* Number of payload symbols in last received message. 16 bits */
	AT86RF_PIB_PHY_RX_PAY_SYMBOLS               = 0x0181,
	/* Successfully transmitted messages count. 32 bits */
	AT86RF_PIB_PHY_TX_TOTAL                     = 0x01A0,
	/* Successfully transmitted bytes count. 32 bits */
	AT86RF_PIB_PHY_TX_TOTAL_BYTES               = 0x01A1,
	/* Transmission errors count. 32 bits */
	AT86RF_PIB_PHY_TX_ERR_TOTAL                 = 0x01A2,
	/* Transmission errors count due to already in transmission. 32 bits */
	AT86RF_PIB_PHY_TX_ERR_BUSY_TX               = 0x01A3,
	/* Transmission errors count due to already in reception. 32 bits */
	AT86RF_PIB_PHY_TX_ERR_BUSY_RX               = 0x01A4,
	/* Transmission errors count due to busy channel. 32 bits */
	AT86RF_PIB_PHY_TX_ERR_BUSY_CHN              = 0x01A5,
	/* Transmission errors count due to bad message length. 32 bits */
	AT86RF_PIB_PHY_TX_ERR_BAD_LEN               = 0x01A6,
	/* Transmission errors count due to bad format. 32 bits */
	AT86RF_PIB_PHY_TX_ERR_BAD_FORMAT            = 0x01A7,
	/* Transmission errors count due to timeout. 32 bits */
	AT86RF_PIB_PHY_TX_ERR_TIMEOUT               = 0x01A8,
	/* Transmission aborted count. 32 bits */
	AT86RF_PIB_PHY_TX_ERR_ABORTED               = 0x01A9,
	/* Transmission confirms not handled by upper layer count. 32 bits */
	AT86RF_PIB_PHY_TX_CFM_NOT_HANDLED           = 0x01AA,
	/* Successfully received messages count. 32 bits */
	AT86RF_PIB_PHY_RX_TOTAL                     = 0x01B0,
	/* Successfully received bytes count. 32 bits */
	AT86RF_PIB_PHY_RX_TOTAL_BYTES               = 0x01B1,
	/* Reception errors count. 32 bits */
	AT86RF_PIB_PHY_RX_ERR_TOTAL                 = 0x01B2,
	/* Reception false positive count. 32 bits */
	AT86RF_PIB_PHY_RX_ERR_FALSE_POSITIVE        = 0x01B3,
	/* Reception errors count due to bad message length. 32 bits */
	AT86RF_PIB_PHY_RX_ERR_BAD_LEN               = 0x01B4,
	/* Reception errors count due to bad format or bad FCS in header. 32 bits */
	AT86RF_PIB_PHY_RX_ERR_BAD_FORMAT            = 0x01B5,
	/* Reception errors count due to bad FCS in payload. 32 bits */
	AT86RF_PIB_PHY_RX_ERR_BAD_FCS_PAY           = 0x01B6,
	/* Reception aborted count. 32 bits */
	AT86RF_PIB_PHY_RX_ERR_ABORTED               = 0x01B7,
	/* Reception overrided (another message with higher signal level) count. 32 bits */
	AT86RF_PIB_PHY_RX_OVERRIDE                  = 0x01B8,
	/* Reception indications not handled by upper layer count. 32 bits */
	AT86RF_PIB_PHY_RX_IND_NOT_HANDLED           = 0x01B9,
	/* Reset Phy Statistics (write-only) */
	AT86RF_PIB_PHY_STATS_RESET                  = 0x01C0,
	/* Continuous Tx Mode. 8 bits */
	AT86RF_PIB_PHY_TX_CONTINUOUS                = 0x01C1,
	/* Backoff period unit in us (aUnitBackoffPeriod in IEEE 802.15.4) used for CSMA-CA . 16 bits (read-only) */
	AT86RF_PIB_MAC_UNIT_BACKOFF_PERIOD          = 0x0200,
} at86rf_pib_attr_t;

/* Enumeration of bands and operating modes (IEEE 802.15.4-2020) */
typedef enum at86rf_phy_band_opm {
	/* 863 (863-870 MHz) band. Europe, India */
	AT86RF_SUN_FSK_BAND_863_OPM1   = 0x0401,
	AT86RF_SUN_FSK_BAND_863_OPM2   = 0x0402,
	AT86RF_SUN_FSK_BAND_863_OPM3   = 0x0403,
	AT86RF_SUN_OFDM_BAND_863_OPT4  = 0x0440,
	/* 866 (865-867 MHz) band. */
	AT86RF_SUN_FSK_BAND_866_OPM1   = 0x0E01,
	AT86RF_SUN_FSK_BAND_866_OPM2   = 0x0E02,
	AT86RF_SUN_FSK_BAND_866_OPM3   = 0x0E03,
	AT86RF_SUN_OFDM_BAND_866_OPT4  = 0x0E40,
	/* 870 (870-876 MHz) band. Europe */
	AT86RF_SUN_FSK_BAND_870_OPM1   = 0x0F01,
	AT86RF_SUN_FSK_BAND_870_OPM2   = 0x0F02,
	AT86RF_SUN_FSK_BAND_870_OPM3   = 0x0F03,
	AT86RF_SUN_OFDM_BAND_870_OPT4  = 0x0F40,
	/* 915 (902-928 MHz) band. USA, Canada, Mexico, Colombia */
	AT86RF_SUN_FSK_BAND_915_OPM1   = 0x0701,
	AT86RF_SUN_FSK_BAND_915_OPM2   = 0x0702,
	AT86RF_SUN_FSK_BAND_915_OPM3   = 0x0703,
	AT86RF_SUN_OFDM_BAND_915_OPT4  = 0x0740,
	AT86RF_SUN_OFDM_BAND_915_OPT3  = 0x0730,
	AT86RF_SUN_OFDM_BAND_915_OPT2  = 0x0720,
	AT86RF_SUN_OFDM_BAND_915_OPT1  = 0x0710,
	/* 915-a (902-928 MHz alternate) band. USA, Canada, Mexico */
	AT86RF_SUN_FSK_BAND_915A_OPM1  = 0x1001,
	AT86RF_SUN_FSK_BAND_915A_OPM2  = 0x1002,
	AT86RF_SUN_FSK_BAND_915A_OPM3  = 0x1003,
	AT86RF_SUN_FSK_BAND_915A_OPM4  = 0x1004,
	AT86RF_SUN_FSK_BAND_915A_OPM5  = 0x1005,
	AT86RF_SUN_OFDM_BAND_915A_OPT4 = 0x1040,
	AT86RF_SUN_OFDM_BAND_915A_OPT3 = 0x1030,
	AT86RF_SUN_OFDM_BAND_915A_OPT2 = 0x1020,
	AT86RF_SUN_OFDM_BAND_915A_OPT1 = 0x1010,
	/* 915-b (902-907.5 & 915-928 MHz) band. Brazil */
	AT86RF_SUN_FSK_BAND_915B_OPM1  = 0x1101,
	AT86RF_SUN_FSK_BAND_915B_OPM2  = 0x1102,
	AT86RF_SUN_FSK_BAND_915B_OPM3  = 0x1103,
	AT86RF_SUN_FSK_BAND_915B_OPM4  = 0x1104,
	AT86RF_SUN_FSK_BAND_915B_OPM5  = 0x1105,
	AT86RF_SUN_OFDM_BAND_915B_OPT4 = 0x1140,
	AT86RF_SUN_OFDM_BAND_915B_OPT3 = 0x1130,
	AT86RF_SUN_OFDM_BAND_915B_OPT2 = 0x1120,
	AT86RF_SUN_OFDM_BAND_915B_OPT1 = 0x1110,
	/* 915-c (915-928 MHz) band. Argentina, Australia, New Zeland */
	AT86RF_SUN_FSK_BAND_915C_OPM1  = 0x1201,
	AT86RF_SUN_FSK_BAND_915C_OPM2  = 0x1202,
	AT86RF_SUN_FSK_BAND_915C_OPM3  = 0x1203,
	AT86RF_SUN_FSK_BAND_915C_OPM4  = 0x1204,
	AT86RF_SUN_FSK_BAND_915C_OPM5  = 0x1205,
	AT86RF_SUN_OFDM_BAND_915C_OPT4 = 0x1240,
	AT86RF_SUN_OFDM_BAND_915C_OPT3 = 0x1230,
	AT86RF_SUN_OFDM_BAND_915C_OPT2 = 0x1220,
	AT86RF_SUN_OFDM_BAND_915C_OPT1 = 0x1210,
	/* 919 (919-923 MHz) band. Malaysia */
	AT86RF_SUN_FSK_BAND_919_OPM1   = 0x1501,
	AT86RF_SUN_FSK_BAND_919_OPM2   = 0x1502,
	AT86RF_SUN_FSK_BAND_919_OPM3   = 0x1503,
	AT86RF_SUN_FSK_BAND_919_OPM4   = 0x1504,
	AT86RF_SUN_FSK_BAND_919_OPM5   = 0x1505,
	AT86RF_SUN_OFDM_BAND_919_OPT4  = 0x1540,
	AT86RF_SUN_OFDM_BAND_919_OPT3  = 0x1530,
	AT86RF_SUN_OFDM_BAND_919_OPT2  = 0x1520,
	AT86RF_SUN_OFDM_BAND_919_OPT1  = 0x1510,
	/* 920 (920-928 MHz) band. Japan */
	AT86RF_SUN_FSK_BAND_920_OPM1   = 0x0901,
	AT86RF_SUN_FSK_BAND_920_OPM2   = 0x0902,
	AT86RF_SUN_FSK_BAND_920_OPM3   = 0x0903,
	AT86RF_SUN_FSK_BAND_920_OPM4   = 0x0904,
	AT86RF_SUN_FSK_BAND_920_OPM5   = 0x0905,
	AT86RF_SUN_FSK_BAND_920_OPM6   = 0x0906,
	AT86RF_SUN_FSK_BAND_920_OPM7   = 0x0907,
	AT86RF_SUN_FSK_BAND_920_OPM8   = 0x0908,
	AT86RF_SUN_FSK_BAND_920_OPM9   = 0x0909,
	AT86RF_SUN_FSK_BAND_920_OPM12  = 0x090C,
	AT86RF_SUN_OFDM_BAND_920_OPT4  = 0x0940,
	AT86RF_SUN_OFDM_BAND_920_OPT3  = 0x0930,
	AT86RF_SUN_OFDM_BAND_920_OPT2  = 0x0920,
	AT86RF_SUN_OFDM_BAND_920_OPT1  = 0x0910,
	/* 920-b (920-923 MHz) band. Indonesia */
	AT86RF_SUN_FSK_BAND_920B_INDONESIA_OPM1  = 0x1701,
	AT86RF_SUN_FSK_BAND_920B_INDONESIA_OPM2  = 0x1702,
	AT86RF_SUN_FSK_BAND_920B_INDONESIA_OPM3  = 0x1703,
	AT86RF_SUN_FSK_BAND_920B_INDONESIA_OPM4  = 0x1704,
	AT86RF_SUN_FSK_BAND_920B_INDONESIA_OPM5  = 0x1705,
	AT86RF_SUN_OFDM_BAND_920B_INDONESIA_OPT4 = 0x1740,
	AT86RF_SUN_OFDM_BAND_920B_INDONESIA_OPT3 = 0x1730,
	AT86RF_SUN_OFDM_BAND_920B_INDONESIA_OPT2 = 0x1720,
	AT86RF_SUN_OFDM_BAND_920B_INDONESIA_OPT1 = 0x1710,
	/* Custom band and operating mode (at86rf_phy_cfg_t, AT86RF_PIB_PHY_CONFIG) */
	AT86RF_BAND_OPM_CUSTOM         = 0x0000
} at86rf_phy_band_opm_t;

/* ! \name AT86RF215 PHY initialization parameters parameters */
/* @{ */
/* Struct of AT86RF215 PHY initialization parameters (at86rf_enable) */
typedef struct at86rf_phy_ini_params {
	/* Pointer to PHY configuration (only used if us_band_opm=AT86RF_BAND_OPM_CUSTOM) */
	at86rf_phy_cfg_t *px_phy_cfg;
	/* Band and operating mode */
	at86rf_phy_band_opm_t us_band_opm;
	/* Initial channel number (0 to use first available channel) */
	uint16_t us_chn_num_ini;
} at86rf_phy_ini_params_t;
/* @} */

/* ! \name AT86RF215 Hardware wrapper */
/* @{ */
/* Function type definition */
typedef uint8_t (*pf_rf_init)(void);
typedef void (*pf_rf_reset)(void);
typedef void (*pf_rf_enable_interrupt)(bool b_enable);
typedef void (*pf_rf_set_handler)(void (*p_handler)(void));
typedef bool (*pf_rf_send_spi_cmd)(uint8_t *puc_data_buf, uint16_t us_addr, uint16_t us_len, uint8_t uc_mode);
typedef bool (*pf_rf_is_spi_busy)(void);
typedef void (*pf_rf_led)(uint8_t uc_led_id, bool b_led_on);
typedef uint32_t (*pf_timer_get)(void);
typedef void (*pf_timer_enable_interrupt)(bool b_enable);
typedef bool (*pf_timer_set_int)(uint32_t ul_time_us, bool b_relative, void (*p_handler)(uint32_t), uint32_t *pul_int_id);
typedef bool (*pf_timer_cancel_int)(uint32_t ul_int_id);

/* Hardware wrapper structure */
typedef struct at86rf_hal_wrapper {
	pf_rf_init rf_init;
	pf_rf_reset rf_reset;
	pf_rf_enable_interrupt rf_enable_int;
	pf_rf_set_handler rf_set_handler;
	pf_rf_send_spi_cmd rf_send_spi_cmd;
	pf_rf_is_spi_busy rf_is_spi_busy;
	pf_rf_led rf_led;
	pf_timer_get timer_get;
	pf_timer_enable_interrupt timer_enable_int;
	pf_timer_set_int timer_set_int;
	pf_timer_cancel_int timer_cancel_int;
} at86rf_hal_wrapper_t;
/* @} */

/* ! \name AT86RF215 Callbacks */
/* @{ */
/* Enumeration of AT86RF215 TX confirm results */
typedef enum at86rf_tx_cfm_res {
	AT86RF_TX_SUCCESS = 0,
	AT86RF_TX_ERROR_UNDERRUN = 1,
	AT86RF_TX_ABORTED = 2,
	AT86RF_TX_BUSY_TX = 3,
	AT86RF_TX_BUSY_RX = 4,
	AT86RF_TX_BUSY_CHN = 5,
	AT86RF_TX_TRX_SLEPT = 6,
	AT86RF_TX_CANCEL_BY_RX = 7,
	AT86RF_TX_TIMEOUT = 8,
	AT86RF_TX_FULL_BUFFERS = 9,
	AT86RF_TX_INVALID_TRX_ID = 10,
	AT86RF_TX_INVALID_LEN = 11,
	AT86RF_TX_INVALID_PARAM = 12,
	AT86RF_TX_CANCELLED = 13,
} at86rf_tx_cfm_res_t;

/* Struct of AT86RF215 PHY TX confirm */
typedef struct at86rf_tx_cfm {
	/* TX time (frame start), referred to system 32-bit counter of 1us */
	uint32_t ul_tx_time_ini;
	/* Frame duration in us */
	uint32_t ul_frame_duration;
	/* TX identifier for the corresponding confirm */
	uint8_t uc_tx_id;
	/* TX result */
	at86rf_tx_cfm_res_t uc_tx_res;
} at86rf_tx_cfm_t;

/* Struct of AT86RF215 PHY RX indication */
typedef struct at86rf_rx_ind {
	/* Pointer to received data */
	uint8_t *puc_data;
	/* RX time (frame start), referred to system 32-bit counter of 1us */
	uint32_t ul_rx_time_ini;
	/* Frame duration in us */
	uint32_t ul_frame_duration;
	/* PSDU length in bytes (including FCS) */
	uint16_t us_psdu_len;
	/* Modulation specific parameters (FSK/OFDM) */
	at86rf_mod_frame_params_t x_mod_params;
	/* RSSI in dBm */
	int8_t sc_rssi_dBm;
	/* Correct FCS flag (only relevant if AT86RF215_ENABLE_AUTO_FCS) */
	bool b_fcs_ok;
} at86rf_rx_ind_t;

/* Function type definition */
typedef void (*pf_at86rf_exception_cb)(uint8_t uc_exception_mask);
typedef void (*pf_at86rf_tx_cfm_cb)(uint8_t uc_trx_id, at86rf_tx_cfm_t *px_tx_cfm);
typedef void (*pf_at86rf_rx_ind_cb)(uint8_t uc_trx_id, at86rf_rx_ind_t *px_rx_ind);
typedef void (*pf_at86rf_chn_switch_cb)(uint8_t uc_trx_id, at86rf_tx_cfm_res_t uc_result);
typedef void (*pf_at86rf_addon_cb)(uint8_t *puc_msg, uint16_t us_len);

/* AT86RF215 Callbacks structure */
typedef struct at86rf_callbacks {
	pf_at86rf_exception_cb rf_exception_cb;
	pf_at86rf_tx_cfm_cb rf_tx_cfm_cb;
	pf_at86rf_rx_ind_cb rf_rx_ind_cb;
	pf_at86rf_chn_switch_cb rf_chn_switch_cb;
#ifdef AT86RF_ADDONS_ENABLE
	pf_at86rf_addon_cb rf_addon_event_cb;
#endif
} at86rf_callbacks_t;
/* @} */

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */

#endif /* AT86RF_DEFS_H_INCLUDED */
