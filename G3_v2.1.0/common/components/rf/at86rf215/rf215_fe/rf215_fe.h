/**
 *
 * \file
 *
 * \brief RF215 Transmitter and Receiver Frontend.
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

#ifndef RF215_FE_H_INCLUDE
#define RF215_FE_H_INCLUDE

/* RF215 includes */
#include "rf215_fe_defs.h"
#include "rf215_reg.h"
#include "rf215_spi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Get Receiver Energy Detection Value (RFn_EDV)
 *
 * \param uc_trx_id TRX identifier
 *
 * \return Energy detection value in dBm (-127 to +4, or +127 if invalid)
 */
__always_inline int8_t rf215_fe_get_edv(uint8_t uc_trx_id)
{
	int8_t sc_edv;

	/* Read RFn_EDV register */
	sc_edv = (int8_t)rf215_spi_reg_read(RF215_ADDR_RFn_EDV(uc_trx_id));

	return sc_edv;
}

/**
 * \brief Configure RFn_EDD register value for desired duration given in us
 *
 * \param uc_trx_id TRX identifier
 * \param us_duration_us Energy Detection duration in us
 */
__always_inline void rf215_fe_set_edd(uint8_t uc_trx_id, uint16_t us_duration_us)
{
	uint8_t uc_df;
	uint8_t uc_edd;

	if (us_duration_us <= (63 << 1)) {
		/* EDD.DTB: 2us */
		uc_edd = RF215_RFn_EDD_DTB_2us;
		uc_df = (uint8_t)(us_duration_us >> 1);
	} else if (us_duration_us <= (63 << 3)) {
		/* EDD.DTB: 8us */
		uc_edd = RF215_RFn_EDD_DTB_8us;
		uc_df = (uint8_t)(us_duration_us >> 3);
	} else if (us_duration_us <= (63 << 5)) {
		/* EDD.DTB: 32us */
		uc_edd = RF215_RFn_EDD_DTB_32us;
		uc_df = (uint8_t)(us_duration_us >> 5);
	} else {
		/* EDD.DTB: 128us */
		uc_edd = RF215_RFn_EDD_DTB_128us;
		us_duration_us = min(us_duration_us, 63 << 7);
		uc_df = (uint8_t)(us_duration_us >> 7);
	}

	/* EDD.DF */
	uc_edd |= RF215_RFn_EDD_DF(uc_df);

	/* Write RFn_EDD */
	rf215_spi_reg_write(RF215_ADDR_RFn_EDD(uc_trx_id), uc_edd);
}

/**
 * \brief Configure RFn_EDD register value for for automatic mode
 *
 * \param uc_trx_id TRX identifier
 */
__always_inline void rf215_fe_set_edd_auto(uint8_t uc_trx_id)
{
	/* Write RFn_EDD */
	rf215_spi_reg_write(RF215_ADDR_RFn_EDD(uc_trx_id), RXFE_EDD_AUTO);
}

/**
 * \brief Start a single Energy Detection measurement
 *
 * \param uc_trx_id TRX identifier
 */
__always_inline void rf215_fe_ed_single(uint8_t uc_trx_id)
{
	/* Write RFn_EDC */
	rf215_spi_reg_write(RF215_ADDR_RFn_EDC(uc_trx_id), RF215_RFn_EDC_EDM_SINGLE);
}

/**
 * \brief Configure Automatic Energy Detection Mode (triggered by reception)
 *
 * \param uc_trx_id TRX identifier
 */
__always_inline void rf215_fe_set_edc_edd_auto(uint8_t uc_trx_id)
{
	/* Write RFn_EDC, RFn_EDD */
	const uint8_t puc_edc_edd[2] = {RF215_RFn_EDC_EDM_AUTO, RXFE_EDD_AUTO};
	rf215_spi_write(RF215_ADDR_RFn_EDC(uc_trx_id), (uint8_t *)puc_edc_edd, 2);
}

/** RF215 Transmitter and Receiver Frontend function declaration */
void rf215_fe_trx_reset_event(uint8_t uc_trx_id);
void rf215_fe_upd_phy_cfg(uint8_t uc_trx_id);
void rf215_fe_set_txpwr(uint8_t uc_trx_id, uint8_t uc_txpwr_att, at86rf_mod_frame_params_t *px_mod_params);

#ifdef __cplusplus
}
#endif

#endif  /* RF215_FE_H_INCLUDE */
