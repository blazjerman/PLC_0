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

#ifndef RF215_BBC_H_INCLUDE
#define RF215_BBC_H_INCLUDE

/* RF215 includes */
#include "rf215_bbc_defs.h"
#include "at86rf_defs.h"
#include "rf215_phy_defs.h"
#include "rf215_reg.h"
#include "rf215_spi.h"

#ifdef __cplusplus
extern "C" {
#endif

/** RF215 internal global variables declared as extern */
extern const uint32_t gpul_fsk_symrate_hz[RF215_NUM_FSK_SYMRATES];
extern const uint32_t gpul_ofdm_bw_hz[RF215_NUM_OFDM_OPTIONS];

/** RF215 Baseband Core inline function definition */

/**
 * \brief Set TX Frame Length in BBC frame buffer registers
 *
 * \param uc_trx_id TRX identifier
 * \param us_len TX frame length (PSDU, including FCS)
 */
__always_inline void rf215_bbc_set_tx_len(uint8_t uc_trx_id, uint16_t us_len)
{
	uint8_t puc_txfl_regs[2];

	/* BBCn_TXFLL � TX Frame Length Low Byte */
	puc_txfl_regs[0] = (uint8_t)(us_len & 0xFF);
	/* BBCn_TXFLH � TX Frame Length High Byte */
	puc_txfl_regs[1] = (uint8_t)RF215_BBCn_TXFLH_TXFLH(us_len >> 8);

	/* Write 2 registers: BBCn_TXFLL, BBCn_TXFLH */
	rf215_spi_write(RF215_ADDR_BBCn_TXFLL(uc_trx_id), puc_txfl_regs, 2);
}

/**
 * \brief Write TX Frame Buffer in BBC frame buffer registers
 *
 * \param uc_trx_id TRX identifier
 * \param puc_data Pointer to data to be sent
 * \param us_len TX frame length (PSDU, including FCS)
 */
__always_inline void rf215_bbc_write_tx_buf(uint8_t uc_trx_id, uint8_t *puc_data, uint16_t us_len)
{
	/* Write data to TX frame buffer */
	rf215_spi_write(RF215_ADDR_BBCn_FBTXS(uc_trx_id), puc_data, us_len);
}

/**
 * \brief Get RX Frame Length from BBC frame buffer registers
 *
 * \param uc_trx_id TRX identifier
 *
 * \return RX frame length (PSDU, including FCS)
 */
__always_inline uint16_t rf215_bbc_get_rx_len(uint8_t uc_trx_id)
{
	uint16_t us_len;
	uint8_t puc_rxfl_regs[2] = {0};

	/* Read 2 registers: BBCn_RXFLL, BBCn_RXFLH */
	rf215_spi_read(RF215_ADDR_BBCn_RXFLL(uc_trx_id), puc_rxfl_regs, 2);

	/* BBCn_RXFLL � RX Frame Length Low Byte */
	us_len = (uint16_t)puc_rxfl_regs[0];
	/* BBCn_RXFLH � RX Frame Length High Byte */
	us_len += ((uint16_t)(puc_rxfl_regs[1] & RF215_BBCn_RXFLH_RXFLH_Msk) << 8);

	return us_len;
}

/**
 * \brief Read RX Frame Buffer from BBC frame buffer registers. Function is
 * non-blocking because it's not needed to have the data when returning from
 * this funcion. The SPI DMA transaction is launched and IRQ is left (leaving
 * the CPU free for other tasks).
 *
 * \param uc_trx_id TRX identifier
 * \param puc_data Pointer to store received data
 * \param us_len RX frame remaining length (PSDU, with FCS)
 * \param us_offset RX frame buffer offset
 */
__always_inline void rf215_bbc_read_rx_buf(uint8_t uc_trx_id, uint8_t *puc_data,
		uint16_t us_len, uint16_t us_offset)
{
	/* Read data from RX frame buffer */
	rf215_spi_read_no_block(RF215_ADDR_BBCn_FBRXS(uc_trx_id) + us_offset, puc_data, us_len);
}

/**
 * \brief Get Frame Buffer Level from BBC frame buffer registers. During frame
 * receive the FBL indicates the number of bytes stored from the baseband core
 * to the receive frame buffer. During frame transmit the FBL indicates the
 * number of octets already read by the baseband core from the transmit frame
 * buffer.
 *
 * \param uc_trx_id TRX identifier
 *
 * \return Frame Buffer Level
 */
__always_inline uint16_t rf215_bbc_get_buf_lvl(uint8_t uc_trx_id)
{
	uint16_t us_len;
	uint8_t puc_rxfl_regs[2] = {0};

	/* Read 2 registers: BBCn_FBLL, BBCn_FBLH */
	rf215_spi_read(RF215_ADDR_BBCn_FBLL(uc_trx_id), puc_rxfl_regs, 2);

	/* BBCn_FBLL � RX Frame Length Low Byte */
	us_len = (uint16_t)puc_rxfl_regs[0];
	/* BBCn_FBLH � RX Frame Length High Byte */
	us_len += ((uint16_t)(puc_rxfl_regs[1] & RF215_BBCn_FBLH_FBLH_Msk) << 8);

	return us_len;
}

/**
 * \brief Configure Timestap Counter in Capture Mode (RX and TX)
 *
 * \param uc_trx_id TRX identifier
 */
__always_inline void rf215_bbc_cnt_capture(uint8_t uc_trx_id)
{
	/* BBCn_CNTC: Enable counter with RX and TX start capture mode
	 * EN: Enable counter
	 * CAPRXS: Capture of Counter Values at RX Start Event
	 * CAPTXS: Capture of Counter Values at TX Start Event */
	rf215_spi_reg_write(RF215_ADDR_BBCn_CNTC(uc_trx_id), RF215_BBCn_CNTC_EN |
			RF215_BBCn_CNTC_CAPRXS | RF215_BBCn_CNTC_CAPTXS);
}

/**
 * \brief Configure Timestap Counter in Free-Running Mode
 *
 * \param uc_trx_id TRX identifier
 */
__always_inline void rf215_bbc_cnt_free_run(uint8_t uc_trx_id)
{
	/* BBCn_CNTC: Enable counter with free-running mode
	 * EN: Enable counter */
	rf215_spi_reg_write(RF215_ADDR_BBCn_CNTC(uc_trx_id), RF215_BBCn_CNTC_EN);
}

/**
 * \brief Compute FSK frequency deviation in Hz;
 * fdev = (SymbRate * ModIndex) / 2
 *
 * \param px_fsk_cfg Pointer to FSK configuration
 *
 * \return FSK frequency deviation in Hz
 */
__always_inline uint32_t rf215_bbc_fsk_fdev(at86rf_fsk_cfg_t *px_fsk_cfg)
{
	uint32_t ul_symb_rate;
	uint32_t ul_fdev;

	ul_symb_rate = gpul_fsk_symrate_hz[px_fsk_cfg->uc_symrate];

	if (px_fsk_cfg->uc_modidx == AT86RF_FSK_MODIDX_1_0) {
		/* fdev = (SymbRate * 1.0) / 2 */
		ul_fdev = ul_symb_rate >> 1;
	} else { /* AT86RF_FSK_MODIDX_0_5 */
		 /* fdev = (SymbRate * 0.5) / 2 */
		ul_fdev = ul_symb_rate >> 2;
	}

	return ul_fdev;
}

/** RF215 Baseband Core function declaration */
bool rf215_bbc_init(uint8_t uc_trx_id, at86rf_phy_cfg_t *px_phy_cfg, uint16_t us_chn_num);
void rf215_bbc_trx_reset_event(uint8_t uc_trx_id);
at86rf_res_t rf215_bbc_set_phy_cfg(uint8_t uc_trx_id, at86rf_phy_cfg_t *px_phy_cfg, uint16_t us_chn_num_new);
uint32_t rf215_bbc_upd_tx_params(uint8_t uc_trx_id, at86rf_tx_params_t *px_tx_params, uint16_t *pus_pay_symbols);
void rf215_bbc_tx_auto_cfg(uint8_t uc_trx_id, bool b_ccatx, at86rf_cca_ed_cfg_t *px_cca_ed_cfg, uint8_t uc_cw);
bool rf215_bbc_ccatx_edc_event(uint8_t uc_trx_id, at86rf_cca_ed_cfg_t *px_cca_ed_cfg, uint8_t uc_cw);
void rf215_bbc_ccatx_abort(uint8_t uc_trx_id);
bool rf215_bbc_check_rx_params(uint8_t uc_trx_id, uint16_t us_psdu_len, at86rf_rx_ind_t *px_rx_ind, uint16_t *pus_pay_symbols);
uint32_t rf215_bbc_get_cnt(uint8_t uc_trx_id);
void rf215_bbc_set_rx_proc_delay(uint8_t uc_trx_id, uint16_t us_proc_delay_us_q5);
bool rf215_bbc_get_ofdm_scp(uint8_t uc_trx_id);
void rf215_bbc_tx_auto_stop(uint8_t uc_trx_id);
uint16_t rf215_bbc_get_symbol_duration_us_q5(uint8_t uc_trx_id);
int8_t rf215_bbc_get_sensitivity_dBm(uint8_t uc_trx_id);

#ifdef __cplusplus
}
#endif

#endif  /* RF215_BBC_H_INCLUDE */
