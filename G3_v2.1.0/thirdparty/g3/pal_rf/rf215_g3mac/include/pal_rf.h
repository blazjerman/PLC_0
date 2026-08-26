/**
 *
 * \file
 *
 * \brief G3 RF Phy Abstraction Layer for RF215
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

#ifndef PAL_RF_H_INCLUDED
#define PAL_RF_H_INCLUDED

/* #define PAL_RF_DEBUG_ENABLE */

enum EPalRFStatus {
	PAL_RF_SUCCESS,
	PAL_RF_CHANNEL_ACCESS_FAILURE,
	PAL_RF_BUSY_TX,
	PAL_RF_TIMEOUT,
	PAL_RF_INVALID_PARAM,
	PAL_RF_TX_CANCELLED,
	PAL_RF_TRX_OFF,
	PAL_RF_ERROR,
};

enum EPalRfGetSetResult {
	PAL_RF_GETSET_RESULT_OK,
	PAL_RF_GETSET_RESULT_INVALID_PARAM,
	PAL_RF_GETSET_RESULT_READ_ONLY,
	PAL_RF_GETSET_RESULT_WRITE_ONLY,
	PAL_RF_GETSET_RESULT_ERROR
};

enum EPalRfPhyParam {
	/* RF device identifier. 16 bits */
	PAL_RF_PHY_PARAM_DEVICE_ID                        = 0x0000,
	/* RF PHY layer firmware version number. 6 bytes (see "at86rf_fw_version_t") */
	PAL_RF_PHY_PARAM_FW_VERSION                       = 0x0001,
	/* RF device reset (write-only) */
	PAL_RF_PHY_PARAM_DEVICE_RESET                     = 0x0002,
	/* RF transceiver (RF09 or RF24) reset (write-only) */
	PAL_RF_PHY_PARAM_TRX_RESET                        = 0x0080,
	/* RF transceiver (RF09 or RF24) sleep (write-only) */
	PAL_RF_PHY_PARAM_TRX_SLEEP                        = 0x0081,
	/* RF PHY configuration (see "at86rf_phy_cfg_t") */
	PAL_RF_PHY_PARAM_PHY_CONFIG                       = 0x0100,
	/* RF PHY band and operating mode. 16 bits (see "at86rf_phy_band_opm_t") */
	PAL_RF_PHY_PARAM_PHY_BAND_OPERATING_MODE          = 0x0101,
	/* RF channel number used for transmission and reception. 16 bits */
	PAL_RF_PHY_PARAM_PHY_CHANNEL_NUM                  = 0x0120,
	/* RF frequency in Hz used for transmission and reception. 32 bits (read-only) */
	PAL_RF_PHY_PARAM_PHY_CHANNEL_FREQ_HZ              = 0x0121,
	/* Configuration of Energy Detection for CCA. 3 bytes (see "at86rf_cca_ed_cfg_t") */
	PAL_RF_PHY_PARAM_PHY_CCA_ED_CONFIG                = 0x0140,
	/* Duration in us of Energy Detection for CCA. 16 bits */
	PAL_RF_PHY_PARAM_PHY_CCA_ED_DURATION_US           = 0x0141,
	/* Threshold in dBm for CCA with Energy Detection. 16 bits */
	PAL_RF_PHY_PARAM_PHY_CCA_ED_THRESHOLD_DBM         = 0x0142,
	/* Duration in symbols of Energy Detection for CCA. 16 bits */
	PAL_RF_PHY_PARAM_PHY_CCA_ED_DURATION_SYMBOLS      = 0x0143,
	/* Threshold in dB above sensitivity for CCA with Energy Detection. 16 bits */
	PAL_RF_PHY_PARAM_PHY_CCA_ED_THRESHOLD_SENSITIVITY = 0x0144,
	/* Turnaround time in us (aTurnaroundTime in IEEE 802.15.4). 16 bits (read-only) */
	PAL_RF_PHY_PARAM_PHY_TURNAROUND_TIME              = 0x0160,
	/* Number of payload symbols in last transmitted message. 16 bits */
	PAL_RF_PHY_PARAM_PHY_TX_PAY_SYMBOLS               = 0x0180,
	/* Number of payload symbols in last received message. 16 bits */
	PAL_RF_PHY_PARAM_PHY_RX_PAY_SYMBOLS               = 0x0181,
	/* Successfully transmitted messages count. 32 bits */
	PAL_RF_PHY_PARAM_PHY_TX_TOTAL                     = 0x01A0,
	/* Successfully transmitted bytes count. 32 bits */
	PAL_RF_PHY_PARAM_PHY_TX_TOTAL_BYTES               = 0x01A1,
	/* Transmission errors count. 32 bits */
	PAL_RF_PHY_PARAM_PHY_TX_ERR_TOTAL                 = 0x01A2,
	/* Transmission errors count due to already in transmission. 32 bits */
	PAL_RF_PHY_PARAM_PHY_TX_ERR_BUSY_TX               = 0x01A3,
	/* Transmission errors count due to already in reception. 32 bits */
	PAL_RF_PHY_PARAM_PHY_TX_ERR_BUSY_RX               = 0x01A4,
	/* Transmission errors count due to busy channel. 32 bits */
	PAL_RF_PHY_PARAM_PHY_TX_ERR_BUSY_CHN              = 0x01A5,
	/* Transmission errors count due to bad message length. 32 bits */
	PAL_RF_PHY_PARAM_PHY_TX_ERR_BAD_LEN               = 0x01A6,
	/* Transmission errors count due to bad format. 32 bits */
	PAL_RF_PHY_PARAM_PHY_TX_ERR_BAD_FORMAT            = 0x01A7,
	/* Transmission errors count due to timeout. 32 bits */
	PAL_RF_PHY_PARAM_PHY_TX_ERR_TIMEOUT               = 0x01A8,
	/* Transmission aborted count. 32 bits */
	PAL_RF_PHY_PARAM_PHY_TX_ERR_ABORTED               = 0x01A9,
	/* Transmission confirms not handled by upper layer count. 32 bits */
	PAL_RF_PHY_PARAM_PHY_TX_CFM_NOT_HANDLED           = 0x01AA,
	/* Successfully received messages count. 32 bits */
	PAL_RF_PHY_PARAM_PHY_RX_TOTAL                     = 0x01B0,
	/* Successfully received bytes count. 32 bits */
	PAL_RF_PHY_PARAM_PHY_RX_TOTAL_BYTES               = 0x01B1,
	/* Reception errors count. 32 bits */
	PAL_RF_PHY_PARAM_PHY_RX_ERR_TOTAL                 = 0x01B2,
	/* Reception false positive count. 32 bits */
	PAL_RF_PHY_PARAM_PHY_RX_ERR_FALSE_POSITIVE        = 0x01B3,
	/* Reception errors count due to bad message length. 32 bits */
	PAL_RF_PHY_PARAM_PHY_RX_ERR_BAD_LEN               = 0x01B4,
	/* Reception errors count due to bad format or bad FCS in header. 32 bits */
	PAL_RF_PHY_PARAM_PHY_RX_ERR_BAD_FORMAT            = 0x01B5,
	/* Reception errors count due to bad FCS in payload. 32 bits */
	PAL_RF_PHY_PARAM_PHY_RX_ERR_BAD_FCS_PAY           = 0x01B6,
	/* Reception aborted count. 32 bits */
	PAL_RF_PHY_PARAM_PHY_RX_ERR_ABORTED               = 0x01B7,
	/* Reception overrided (another message with higher signal level) count. 32 bits */
	PAL_RF_PHY_PARAM_PHY_RX_OVERRIDE                  = 0x01B8,
	/* Reception indications not handled by upper layer count. 32 bits */
	PAL_RF_PHY_PARAM_PHY_RX_IND_NOT_HANDLED           = 0x01B9,
	/* Reset Phy Statistics (write-only) */
	PAL_RF_PHY_PARAM_PHY_STATS_RESET                  = 0x01C0,
	/* Continuous Tx Mode. 8 bits */
	PAL_RF_PHY_PARAM_CONTINUOUS_TX_MODE               = 0x01C1,
	/* Backoff period unit in us (aUnitBackoffPeriod in IEEE 802.15.4) used for CSMA-CA . 16 bits (read-only) */
	PAL_RF_PHY_PARAM_MAC_UNIT_BACKOFF_PERIOD          = 0x0200,
	/* SUN FSK FEC enabled or disabled for transmission (phyFskFecEnabled in IEEE 802.15.4). 8 bits */
	PAL_RF_PHY_PARAM_TX_FSK_FEC                       = 0x8000,
	/* SUN OFDM MCS (Modulation and coding scheme) used for transmission. 8 bits */
	PAL_RF_PHY_PARAM_TX_OFDM_MCS                      = 0x8001,
};

struct TPalRfTxParams {
	uint32_t m_u32Time;
	uint8_t m_u8TxPowerAtt;
	bool m_bCSMA;
};

struct TPalRfRxParams {
	uint32_t m_u32TimeIni;
	uint32_t m_u32TimeEnd;
	int8_t m_s8RSSI;
	bool m_bFCSOK;
};

typedef void (*PalRFDataConfirm)(enum EPalRFStatus eStatus, uint32_t u32TimeIni, uint32_t u32TimeEnd);
typedef void (*PalRFDataIndication)(uint8_t *pPsdu, uint16_t u16PsduLength, struct TPalRfRxParams *pParameters);

struct TPalRfNotifications {
	PalRFDataConfirm m_pPalRFDataConfirm;
	PalRFDataIndication m_pPalRFDataIndication;
};

bool PalRfInitialize(struct TPalRfNotifications *pNotifications);
void PalRfEventHandler(void);
void PalRfTxRequest(uint8_t *pPsdu, uint16_t u16PsduLength, struct TPalRfTxParams *pParameters);
void PalRfTxCancel(void);
void PalRfResetRequest(void);
uint32_t PalRfGetPhyTime(void);
enum EPalRfGetSetResult PalRfGetParam(uint16_t us_id, void *p_val);
enum EPalRfGetSetResult PalRfSetParam(uint16_t us_id, void *p_val);
uint8_t PalRfGetParamLen(uint16_t us_id);

#endif /* PAL_RF_H_INCLUDED */
