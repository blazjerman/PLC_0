/**
 * \file
 *
 * \brief atpl360_host G3 Physical layer
 *
 * Copyright (c) 2016 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#ifndef ATPL360_COMM_H_INCLUDED
#define ATPL360_COMM_H_INCLUDED

#include "general_defs.h"
#include "conf_atpl360.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

/* ! Tone Map size for Cenelec bandplan */
#define TONE_MAP_SIZE_CENELEC                  1
/* ! Tone Map size for FCC and ARIB bandplans */
#define TONE_MAP_SIZE_FCC_ARIB                 3
/* ! Maximum number of protocol carriers */
#define PROTOCOL_CARRIERS_MAX                  NUM_CARRIERS_FCC
/* ! Maximum number of tone map */
#define TONE_MAP_SIZE_MAX                      TONE_MAP_SIZE_FCC_ARIB
/* ! Maximum number of subbands */
#define NUM_SUBBANDS_MAX                       NUM_SUBBANDS_FCC
/* ! Maximum size of PL360 PHY register. Worst case: Predist Coef (FCC); 72 * 2 */
#define ATPL360_REG_PHY_PARAM_MAX_SIZE         (NUM_CARRIERS_FCC << 1)
/* ! Maximum length of PHY message (G3). Worts case: G3-FCC */
#define ATPL360_MAX_DATA_LENGTH                494

enum _imp_state {
	IMPEDANCE_STATE_HI,
	IMPEDANCE_STATE_LO,
	IMPEDANCE_STATE_VLO,
	IMPEDANCE_STATE_NUM,
};

/* ! Defines relatives to some ATPL360 registers */
#define ATPL360_REG_ADC_MASK                    0x1000
#define ATPL360_REG_DAC_MASK                    0x2000
#define ATPL360_REG_MASK                        0x4000
#define ATPL360_FUSES_MASK                      0x8000
#define ATPL360_REG_ADC_BASE                    0x40000000
#define ATPL360_REG_DAC_BASE                    0x40004000
#define ATPL360_REG_BASE                        0x80000000
#define ATPL360_FUSES_BASE                      0x400E1800

/* Defines relatives to configuration parameter (G3) */
typedef enum atpl360_reg_id {
	ATPL360_REG_PRODID = 0x4000,
	ATPL360_REG_MODEL,
	ATPL360_REG_VERSION_STR,
	ATPL360_REG_VERSION_NUM,
	ATPL360_REG_TONE_MASK,
	ATPL360_REG_TONE_MAP_RSP_DATA,
	ATPL360_REG_TX_TOTAL,
	ATPL360_REG_TX_TOTAL_BYTES,
	ATPL360_REG_TX_TOTAL_ERRORS,
	ATPL360_REG_TX_BAD_BUSY_TX,
	ATPL360_REG_TX_BAD_BUSY_CHANNEL,
	ATPL360_REG_TX_BAD_LEN,
	ATPL360_REG_TX_BAD_FORMAT,
	ATPL360_REG_TX_TIMEOUT,
	ATPL360_REG_RX_TOTAL,
	ATPL360_REG_RX_TOTAL_BYTES,
	ATPL360_REG_RX_RS_ERRORS,
	ATPL360_REG_RX_EXCEPTIONS,
	ATPL360_REG_RX_BAD_LEN,
	ATPL360_REG_RX_BAD_CRC_FCH,
	ATPL360_REG_RX_FALSE_POSITIVE,
	ATPL360_REG_RX_BAD_FORMAT,
	ATPL360_REG_ENABLE_AUTO_NOISE_CAPTURE,
	ATPL360_REG_TIME_BETWEEN_NOISE_CAPTURES,
	ATPL360_REG_DELAY_NOISE_CAPTURE_AFTER_RX,
	ATPL360_REG_RRC_NOTCH_ACTIVE,
	ATPL360_REG_RRC_NOTCH_INDEX,
	ATPL360_REG_NOISE_PEAK_POWER,
	ATPL360_REG_CRC_TX_RX_CAPABILITY,
	ATPL360_REG_RX_BAD_CRC_PAY,
	ATPL360_REG_CFG_AUTODETECT_IMPEDANCE,
	ATPL360_REG_CFG_IMPEDANCE,
	ATPL360_REG_ZC_PERIOD,
	ATPL360_REG_FCH_SYMBOLS,
	ATPL360_REG_PAY_SYMBOLS_TX,
	ATPL360_REG_PAY_SYMBOLS_RX,
	ATPL360_REG_RRC_NOTCH_AUTODETECT,
	ATPL360_REG_MAX_RMS_TABLE_HI,
	ATPL360_REG_MAX_RMS_TABLE_VLO,
	ATPL360_REG_THRESHOLDS_TABLE_HI,
	ATPL360_REG_THRESHOLDS_TABLE_LO,
	ATPL360_REG_THRESHOLDS_TABLE_VLO,
	ATPL360_REG_PREDIST_COEF_TABLE_HI,
	ATPL360_REG_PREDIST_COEF_TABLE_LO,
	ATPL360_REG_PREDIST_COEF_TABLE_VLO,
	ATPL360_REG_GAIN_TABLE_HI,
	ATPL360_REG_GAIN_TABLE_LO,
	ATPL360_REG_GAIN_TABLE_VLO,
	ATPL360_REG_DACC_TABLE_CFG,
	ATPL360_REG_RSV0,
	ATPL360_REG_NUM_TX_LEVELS,
	ATPL360_REG_CORRECTED_RMS_CALC,
	ATPL360_REG_RRC_NOTCH_THR_ON,
	ATPL360_REG_RRC_NOTCH_THR_OFF,
	ATPL360_REG_CURRENT_GAIN,
	ATPL360_REG_ZC_CONF_INV,
	ATPL360_REG_ZC_CONF_FREQ,
	ATPL360_REG_ZC_CONF_DELAY,
	ATPL360_REG_NOISE_PER_CARRIER,
	ATPL360_REG_SYNC_XCORR_THRESHOLD,
	ATPL360_REG_SYNC_XCORR_PEAK_VALUE,
	ATPL360_REG_SYNC_SYNCM_THRESHOLD,
	ATPL360_REG_TONE_MAP_RSP_ENABLED_MODS,
	ATPL360_REG_PPM_CALIB_ON,
	ATPL360_REG_SFO_ESTIMATION_LAST_RX,
	ATPL360_REG_PDC_TX_LAST_RX,
	ATPL360_REG_MAX_PSDU_LEN_PARAMS,
	ATPL360_REG_MAX_PSDU_LEN,
	ATPL360_REG_RESET_STATS,
	ATPL360_REG_PLC_IC_DRIVER_CFG,
	ATPL360_REG_RX_CHN_EST_REAL,
	ATPL360_REG_RX_CHN_EST_IMAG,
	ATPL360_REG_TX_DISABLE,
	ATPL360_REG_TX_HIGH_TEMP_120,
	ATPL360_REG_TX_CANCELLED,
	ATPL360_REG_ZC_VALUE,
	ATPL360_REG_PDC_RX_LAST_RX,
	ATPL360_REG_PDC_ZC_TIME_LAST_RX,
	ATPL360_REG_PDC_ZC_PERIOD_LAST_RX,
	ATPL360_REG_PREAMBLE_NUM_SYNCP,
	ATPL360_REG_END_ID,
} atpl360_reg_id_t;

/* ! Internal Memory Map */
typedef enum atpl360_mem_id {
	ATPL360_STATUS_INFO_ID = 0,
	ATPL360_TX_PARAM_ID,
	ATPL360_TX_DATA_ID,
	ATPL360_TX_CFM_ID,
	ATPL360_RX_PARAM_ID,
	ATPL360_RX_DATA_ID,
	ATPL360_REG_INFO_ID,
	ATPL360_IDS,
} atpl360_mem_id_t;

/* Defines refering to Impedance Configuration */
#define HI_STATE                                        0x00
#define LOW_STATE                                       0x01
#define VLO_STATE                                       0x02

/** Table of configuration parameters */
typedef struct {
	/** (ro) Version number */
	uint32_t ul_version;
	/** (ro) Model identifier */
	uint16_t us_model_id;
	/** (ro) Product identifier */
	uint16_t us_product_id;
	/** (ro) Product description */
	char puc_description[10];
} atpl360_id_t;

/* ! \name TX Mode Bit Mask */
/* @{ */
/* ! TX Mode: Forced transmission */
#define TX_MODE_FORCED               (1 << 0)
/* ! TX Mode: Absolute transmission */
#define TX_MODE_ABSOLUTE             (0 << 1)
/* ! TX Mode: Delayed transmission */
#define TX_MODE_RELATIVE             (1 << 1)
/* ! TX Mode: SYNCP Continuous transmission */
#define TX_MODE_SYNCP_CONTINUOUS     (1 << 2)
/* ! TX Mode: Syimbols Continuous transmission */
#define TX_MODE_SYMBOLS_CONTINUOUS   (1 << 3)
/* ! TX Mode: Cancel transmission */
#define TX_MODE_CANCEL               (1 << 4)
/* @} */

/* ! \name G3 Modulation types */
enum mod_types {
	MOD_TYPE_BPSK = 0,
	MOD_TYPE_QPSK = 1,
	MOD_TYPE_8PSK = 2,
	MOD_TYPE_QAM = 3,
	MOD_TYPE_BPSK_ROBO = 4,
};

/* ! \name G3 Modulation schemes */
enum mod_schemes {
	MOD_SCHEME_DIFFERENTIAL = 0,
	MOD_SCHEME_COHERENT = 1,
};

/* ! \name G3 Frame Delimiter Types */
enum delimiter_types {
	DT_SOF_NO_RESP = 0,
	DT_SOF_RESP = 1,
	DT_ACK = 2,
	DT_NACK = 3,
};

/* ! \name G3 TX Result values */
enum tx_result_values {
	TX_RESULT_PROCESS = 0,                  /* Transmission result: already in process */
	TX_RESULT_SUCCESS = 1,                  /* Transmission result: end successfully */
	TX_RESULT_INV_LENGTH = 2,               /* Transmission result: invalid length error */
	TX_RESULT_BUSY_CH = 3,                  /* Transmission result: busy channel error */
	TX_RESULT_BUSY_TX = 4,                  /* Transmission result: busy in transmission error */
	TX_RESULT_BUSY_RX = 5,                  /* Transmission result: busy in reception error */
	TX_RESULT_INV_SCHEME = 6,               /* Transmission result: invalid modulation scheme error */
	TX_RESULT_TIMEOUT = 7,                  /* Transmission result: timeout error */
	TX_RESULT_INV_TONEMAP = 8,              /* Transmission result: invalid tone map error */
	TX_RESULT_INV_MODTYPE = 9,              /* Transmission result: invalid modulation type error */
	TX_RESULT_INV_DT = 10,                  /* Transmission result: invalid delimiter type error */
	TX_RESULT_CANCELLED = 11,               /* Transmission result: transmission cancelled */
	TX_RESULT_HIGH_TEMP_120 = 12,           /* Transmission result: high temperature (>120ºC) error (only with PL460/PL480) */
	TX_RESULT_HIGH_TEMP_110 = 13,           /* Transmission result: high temperature (>110ºC) error (only with PL460/PL480) */
	TX_RESULT_NO_TX = 255,                  /* Transmission result: No transmission ongoing */
};

/* ! \name G3 Event types */
enum atpl360_event_type {
	MSG_IND_DATA_EV_TYPE = 0,               /* Message data indication event */
	MSG_IND_PARAM_EV_TYPE,                  /* Message parameters indication event */
	MSG_CFM_EV_TYPE,                        /* Message confirm event */
	REG_EV_TYPE,                            /* Registers event */
	NUM_EV_TYPES,                           /* Number of event types */
};

/* ! \name G3 communication results */
typedef enum atpl360_comm_status {
	ATPL360_COMM_SUCCESS = 0,               /* Current operation successful */
	ATPL360_COMM_ERROR,                     /* Current operation failed */
} atpl360_comm_status_t;

/* Number of transmission buffers */
#define NUM_TX_BUFFERS                      1

#pragma pack(push,1)

/* ! \name G3 Structure defining Rx message */
typedef struct rx_msg {
	uint32_t ul_rx_time;                            /* /< Instant when frame was received (end of message) referred to 1us PHY counter */
	uint32_t ul_frame_duration;                     /* /< Frame duration referred to 1us PHY counter (Preamble + FCH + Payload) */
	uint16_t us_rssi;                               /* /< Reception RSSI in dBuV */
	uint16_t us_data_len;                           /* /< Length of the data buffer in bytes */
	uint8_t uc_zct_diff;                            /* /< ZCT info */
	uint8_t uc_rs_corrected_errors;                 /* /< Errors corrected by Reed-Solomon */
	enum mod_types uc_mod_type;                     /* /< Modulation type of the received message */
	enum mod_schemes uc_mod_scheme;                 /* /< Modulation scheme of the received message */
	uint32_t ul_agc_factor;                         /* /< Test data information */
	uint16_t us_agc_fine;                           /* /< Test data information */
	int16_t ss_agc_offset_meas;                     /* /< Test data information */
	uint8_t uc_agc_active;                          /* /< Test data information */
	uint8_t uc_agc_pga_value;                       /* /< Test data information */
	int16_t ss_snr_fch;                             /* /< Test data information */
	int16_t ss_snr_pay;                             /* /< Test data information */
	uint16_t us_payload_corrupted_carriers;         /* /< Number of corrupted carriers */
	uint16_t us_payload_noised_symbols;             /* /< Number of noised symbols */
	uint8_t uc_payload_snr_worst_carrier;           /* /< SNR of the worst carrier */
	uint8_t uc_payload_snr_worst_symbol;            /* /< SNR of the worst symbol */
	uint8_t uc_payload_snr_impulsive;               /* /< SNR of impulsive noise */
	uint8_t uc_payload_snr_band;                    /* /< SNR of Narrowband noise */
	uint8_t uc_payload_snr_background;              /* /< Background SNR */
	uint8_t uc_lqi;                                 /* /< Link Quality Indicator */
	enum delimiter_types uc_delimiter_type;         /* /< DT field coming in header */
	uint8_t uc_crc_ok;                              /* /< MAC CRC. 1: OK; 0: BAD; 0xFE: Timeout Error; 0xFF: CRC capability disabled (ATPL360_REG_CRC_TX_RX_CAPABILITY) */
	uint8_t puc_tone_map[TONE_MAP_SIZE_MAX];        /* /< Reception Tone Map */
	uint8_t puc_carrier_snr[PROTOCOL_CARRIERS_MAX]; /* /< SNR per carrier */
	uint8_t *puc_data_buf;                          /* /< Pointer to data buffer */
} rx_msg_t;

/* ! \name G3 Structure defining Tx message */
typedef struct tx_msg {
	uint8_t *puc_data_buf;                         /* Pointer to data buffer */
	uint32_t ul_tx_time;                           /* Instant when transmission has to start referred to 1us PHY counter */
	uint16_t us_data_len;                          /* Length of the data buffer */
	uint8_t puc_preemphasis[NUM_SUBBANDS_MAX];     /* Preemphasis for transmission */
	uint8_t puc_tone_map[TONE_MAP_SIZE_MAX];       /* Tone Map to use on transmission */
	uint8_t uc_tx_mode;                            /* Transmission Mode (absolute, relative, forced, continuous, cancel). Constants above */
	uint8_t uc_tx_power;                           /* Power to transmit */
	enum mod_types uc_mod_type;                    /* Modulation type */
	enum mod_schemes uc_mod_scheme;                /* Modulation scheme */
	uint8_t uc_pdc;                                /* Phase Detector Counter */
	uint8_t uc_2_rs_blocks;                        /* Flag to indicate whether 2 RS blocks have to be used (only used for FCC) */
	enum delimiter_types uc_delimiter_type;        /* DT field to be used in header */
} tx_msg_t;

/* ! \name G3 Structure defining result of a transmission */
typedef struct tx_cfm {
	uint32_t ul_rms_calc;                          /* RMS_CALC it allows to estimate tx power injected */
	uint32_t ul_tx_time;                           /* Instant when frame transmission ended referred to 1us PHY counter */
	enum tx_result_values uc_tx_result;            /* Tx Result (see "TX Result values" above) */
} tx_cfm_t;

/* ! \name G3 Structure defining tone map response data */
typedef struct tm_rsp_data {
	enum mod_types uc_mod_type;                    /* Modulation type */
	enum mod_schemes uc_mod_scheme;                /* Modulation scheme */
	uint8_t puc_tone_map[TONE_MAP_SIZE_MAX];       /* Tone Map */
} tm_rsp_data_t;

/* ! \name G3 Structure defining parameters for maximum PSDU length computation */
typedef struct max_psdu_len_params {
	enum mod_types uc_mod_type;                    /* Modulation type */
	enum mod_schemes uc_mod_scheme;                /* Modulation scheme */
	uint8_t uc_2_rs_blocks;                        /* Flag to indicate whether 2 RS blocks have to be used (only used for FCC) */
	uint8_t puc_tone_map[TONE_MAP_SIZE_MAX];       /* Tone Map */
} max_psdu_len_params_t;

/* ! \name G3 Structure defining events */
typedef struct atpl360_events {
	/* HW Timer reference */
	uint32_t ul_timer_ref;
	/* Info relative to events */
	uint32_t ul_event_info;
	/* Flag to indicate if CONFIRMATION MSG event is enable */
	bool b_cfm_event_enable[NUM_TX_BUFFERS];
	/* Flag to indicate if DATA INDICATION MSG event is enable */
	bool b_data_ind_event_enable;
	/* Flag to indicate if QPAR INDICATION MSG event is enable */
	bool b_qpar_ind_event_enable;
	/* Flag to indicate if REGISTER DATA RESPONSE event is enable */
	bool b_reg_data_enable;
} atpl360_events_t;

#pragma pack(pop)

#define ATPL360_DELAY_TX_DATA_US                 200

#define ATPL360_EVENT_DATA_LENGTH                8

/* ! FLAG MASKs for set events */
#define ATPL360_TX_CFM_FLAG_MASK                 0x0001
#define ATPL360_RX_DATA_IND_FLAG_MASK            0x0002
#define ATPL360_CD_FLAG_MASK                     0x0004
#define ATPL360_REG_RSP_MASK                     0x0008
#define ATPL360_RX_QPAR_IND_FLAG_MASK            0x0010

/* ! Event Info MASKs */
#define ATPL360_EV_DAT_LEN_MASK                  0x0000FFFF
#define ATPL360_EV_REG_LEN_MASK                  0xFFFF0000
#define ATPL360_GET_EV_DAT_LEN_INFO(x)           ((uint32_t)x & ATPL360_EV_DAT_LEN_MASK)
#define ATPL360_GET_EV_REG_LEN_INFO(x)           (((uint32_t)x & ATPL360_EV_REG_LEN_MASK) >> 16)

uint16_t atpl360_comm_stringify(uint8_t *pv_dst, void *pv_src, uint16_t us_src_size);
atpl360_comm_status_t atpl360_comm_parse(void *pv_dst, uint8_t *puc_src, uint16_t us_dst_size);
uint16_t atpl360_comm_get_event_id(enum atpl360_event_type ev_type, uint16_t us_int_flags);
uint16_t atpl360_comm_get_tx_params_id(tx_msg_t *px_msg);
uint16_t atpl360_comm_get_tx_data_id(tx_msg_t *px_msg);
void atpl360_comm_set_event_info(atpl360_events_t *px_events_info, uint16_t us_int_flags);
uint32_t atpl360_comm_get_cfg_param_access_type(uint16_t us_param_id);
uint32_t atpl360_comm_get_cfg_param_delay_us(uint16_t us_param_id);

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */

#endif /* ATPL360_COMM_H_INCLUDED */
