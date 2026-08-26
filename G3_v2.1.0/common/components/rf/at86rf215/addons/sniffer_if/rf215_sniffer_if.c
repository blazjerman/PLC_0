/**
 *
 * \file
 *
 * \brief RF215 sniffer addon.
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
#include "rf215_addon.h"
#include "rf215_sniffer_if.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Number of programmed buffers plus instantaneous TX */
#define RF215_SNIFFER_MAX_TX_BUF     (AT86RF215_NUM_TX_PROG_BUFFERS + 1)

/* RF215 sniffer TX control struct */
typedef struct rf215_snif_tx_ctl {
	uint16_t us_len;
	uint8_t uc_tx_id;
	bool b_free;
	uint8_t puc_buf[RF215_SNIFFER_MSG_MAX_LEN];
} rf215_snif_tx_ctl_t;

/* RF215 sniffer control struct */
typedef struct rf215_snif_ctl {
	rf215_snif_tx_ctl_t px_buf_tx[RF215_SNIFFER_MAX_TX_BUF];
	uint8_t puc_buf_rx[RF215_SNIFFER_MSG_MAX_LEN];
} rf215_snif_ctl_t;

#ifdef AT86RF_ADDONS_ENABLE

/* RF215 sniffer control struct (one per TRX) */
static rf215_snif_ctl_t spx_snif_ctl[AT86RF_NUM_TRX];

/**
 * \brief Converts PHY RX indication struct to byte buffer for sniffer
 *
 * \param[in] uc_trx_id TRX identifier
 * \param[in] px_rx_ind Pointer to RX indication struct
 * \pus_len[out] pus_len Length of sniffer data buffer
 *
 * \return Pointer to sniffer data buffer
 */
uint8_t *rf215_addon_stringify_ind(uint8_t uc_trx_id, at86rf_rx_ind_t *px_rx_ind, uint16_t *pus_len)
{
	uint8_t *puc_dst_buf;
	uint16_t us_len;

	/* Select buffer to stringify RX indication */
	puc_dst_buf = spx_snif_ctl[uc_trx_id].puc_buf_rx;

	/* Stringify RX indication */
	us_len = rf215_sniffer_if_stringify_ind(uc_trx_id, puc_dst_buf, px_rx_ind);

	/* Return pointer to buffer and length */
	*pus_len = us_len;
	return puc_dst_buf;
}

/**
 * \brief Converts PHY TX confirm struct to byte buffer for sniffer
 *
 * \param[in] uc_trx_id TRX identifier
 * \param[in] px_tx_cfm Pointer to TX confirm struct
 * \pus_len[out] pus_len Length of sniffer data buffer (0 if not successful TX)
 *
 * \return Pointer to sniffer data buffer (NULL if not successful TX)
 */
uint8_t *rf215_addon_stringify_cfm(uint8_t uc_trx_id, at86rf_tx_cfm_t *px_tx_cfm, uint16_t *pus_len)
{
	uint8_t *puc_dst_buf = NULL;
	uint16_t us_len = 0;

	/* Look for buffer with same TX ID */
	for (uint8_t uc_i = 0; uc_i < RF215_SNIFFER_MAX_TX_BUF; uc_i++) {
		rf215_snif_tx_ctl_t *px_tx_buf = &spx_snif_ctl[uc_trx_id].px_buf_tx[uc_i];
		if ((!px_tx_buf->b_free) && (px_tx_buf->uc_tx_id == px_tx_cfm->uc_tx_id)) {
			/* Free buffer */
			px_tx_buf->b_free = true;

			if (px_tx_cfm->uc_tx_res == AT86RF_TX_SUCCESS) {
				/* Select buffer to stringify TX confirm */
				puc_dst_buf = px_tx_buf->puc_buf;

				/* Stringify TX confirm */
				rf215_sniffer_if_stringify_cfm(uc_trx_id, puc_dst_buf, px_tx_cfm);
				us_len = px_tx_buf->us_len;
			}
		}
	}

	/* Return pointer to buffer and length */
	*pus_len = us_len;
	return puc_dst_buf;
}

/**
 * \brief Converts PHY TX request struct to byte buffer for sniffer
 *
 * \param[in] uc_trx_id TRX identifier
 * \param[in] px_tx_params Pointer to TX parameters struct
 */
void rf215_addon_stringify_tx(uint8_t uc_trx_id, at86rf_tx_params_t *px_tx_params)
{
	/* Look for free buffer */
	for (uint8_t uc_i = 0; uc_i < RF215_SNIFFER_MAX_TX_BUF; uc_i++) {
		rf215_snif_tx_ctl_t *px_tx_buf = &spx_snif_ctl[uc_trx_id].px_buf_tx[uc_i];
		if (px_tx_buf->b_free) {
			uint8_t *puc_dst_buf;
			uint16_t us_len;

			/* Select buffer to stringify TX request */
			puc_dst_buf = px_tx_buf->puc_buf;

			/* Stringify TX confirm */
			us_len = rf215_sniffer_if_stringify_tx(puc_dst_buf, px_tx_params);

			/* Set buffer as used */
			px_tx_buf->b_free = false;

			/* Store parameters for TX confirm */
			px_tx_buf->uc_tx_id = px_tx_params->uc_tx_id;
			px_tx_buf->us_len = us_len;

			break;
		}
	}
}

/**
 * \brief Sniffer addon initialization
 */
void rf215_addon_init(void)
{
	for (uint8_t uc_trx_id = 0; uc_trx_id < AT86RF_NUM_TRX; uc_trx_id++) {
		for (uint8_t uc_i = 0; uc_i < (AT86RF215_NUM_TX_PROG_BUFFERS + 1); uc_i++) {
			spx_snif_ctl[uc_trx_id].px_buf_tx[uc_i].b_free = true;
		}
	}
}

#endif

#ifdef __cplusplus
}
#endif
