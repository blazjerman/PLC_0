/**
 *
 * \file
 *
 * \brief RF215 SPI access functions.
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
#include "rf215_spi.h"

#ifdef __cplusplus
extern "C" {
#endif

/** RF215 internal global variables declared as extern */
extern volatile uint8_t guc_rf215_exception_mask;

/**
 * \brief Launch SPI transaction to RF215 transceiver. DMA is used to reduce CPU
 * overhead. In write mode the function is non-blocking. In read mode it can be
 * blocking (wait for the data to be read) or non-blocking. If something went
 * wrong in SPI transaction, SPI error exception flag is set.
 *
 * \param[in/out] puc_data_buf Pointer to data to write from or read to
 * \param[in] us_addr RF215 register address
 * \param[in] us_len Data length in bytes
 * \param[in] uc_mode Write or read with/without block (see enum rf_spi_mode)
 */
void rf215_spi_send_cmd(uint8_t *puc_data_buf, uint16_t us_addr, uint16_t us_len, uint8_t uc_mode)
{
	bool b_spi_result;

	b_spi_result = gx_rf215_hal_wrp.rf_send_spi_cmd(puc_data_buf, us_addr, us_len, uc_mode);

	if (!b_spi_result) {
		/* Something went wrong in SPI transaction. Set exception flag.
		 * Critical region is needed to avoid conflict in write access
		 * to guc_rf215_exception_mask */
		gx_rf215_hal_wrp.rf_enable_int(false);
		gx_rf215_hal_wrp.timer_enable_int(false);
		guc_rf215_exception_mask |= AT86RF_EXCEPTION_SPI_ERR;
		gx_rf215_hal_wrp.timer_enable_int(true);
		gx_rf215_hal_wrp.rf_enable_int(true);
	}
}

/**
 * \brief Write to RF transceiver efficiently. Compare the data to be written
 * with known values stored in RF215 (reset values or previous write). This is a
 * non-blocking function. The previous register values are stored in static
 * array and the content is automatically updated by this function.
 *
 * \param[in] us_addr RF215 register address
 * \param[in] puc_data_new Pointer to data to be written
 * \param[in/out] puc_data_old Pointer to previous data
 * \param[in] us_len Number of bytes to write
 */
void rf215_spi_write_upd(uint16_t us_addr, uint8_t *puc_data_new, uint8_t *puc_data_old, uint16_t us_len)
{
	uint8_t *puc_write_data = NULL;
	uint16_t us_write_len = 0;
	uint16_t us_write_addr = 0;
	uint8_t uc_same_data = 0;

	for (uint8_t uc_i = 0; uc_i < us_len; uc_i++) {
		if (puc_data_new[uc_i] != puc_data_old[uc_i]) {
			/* Different byte, needs to be written */
			/* Update static array to store register values */
			puc_data_old[uc_i] = puc_data_new[uc_i];

			/* Check start of SPI transaction block */
			if (us_write_len == 0) {
				us_write_addr = us_addr + uc_i;
				puc_write_data = &puc_data_new[uc_i];
			}

			/* Update write counters */
			us_write_len += (uc_same_data + 1);
			uc_same_data = 0;
		} else if (us_write_len != 0) {
			/* Same byte and SPI transaction block started. Split
			 * the SPI transaction only if it is worth: 3
			 * consecutive bytes not updated (2 SPI header bytes) */
			if (uc_same_data == 2) {
				rf215_spi_write(us_write_addr, puc_write_data, us_write_len);
				us_write_len = 0;
				uc_same_data = 0;
			} else {
				uc_same_data += 1;
			}
		}
	}

	if (us_write_len != 0) {
		/* Send the last SPI transaction */
		rf215_spi_write(us_write_addr, puc_write_data, us_write_len);
	}
}

#ifdef __cplusplus
}
#endif
