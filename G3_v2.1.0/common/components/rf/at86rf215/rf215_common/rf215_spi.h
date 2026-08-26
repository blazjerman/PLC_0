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

#ifndef RF215_SPI_H_INCLUDE
#define RF215_SPI_H_INCLUDE

/* RF215 includes */
#include "at86rf_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/** RF SPI modes */
enum rf_spi_mode {
	RF_SPI_WRITE = 0,
	RF_SPI_READ_BLOCK = 1,
	RF_SPI_READ_NO_BLOCK = 2
};

/** RF215 internal global variables declared as extern */
extern at86rf_hal_wrapper_t gx_rf215_hal_wrp;

/** RF215 SPI internal function declaration */
void rf215_spi_send_cmd(uint8_t *puc_data_buf, uint16_t us_addr, uint16_t us_len, uint8_t uc_mode);
void rf215_spi_write_upd(uint16_t us_addr, uint8_t *puc_data_new, uint8_t *puc_data_old, uint16_t us_len);

/** RF215 SPI inline function definition */

/**
 * \brief Read register from RF transceiver. This is a blocking function.
 *
 * \param[in] us_addr RF215 register address
 *
 * \return Register value
 */
__always_inline uint8_t rf215_spi_reg_read(uint16_t us_addr)
{
	uint8_t uc_reg_value = 0;
	rf215_spi_send_cmd(&uc_reg_value, us_addr, 1, RF_SPI_READ_BLOCK);
	return uc_reg_value;
}

/**
 * \brief Read from RF transceiver. This is a blocking function.
 *
 * \param[in] us_addr RF215 register address
 * \param[out] puc_data Pointer to store read data
 * \param[in] us_len Number of bytes to read
 */
__always_inline void rf215_spi_read(uint16_t us_addr, uint8_t *puc_data, uint16_t us_len)
{
	rf215_spi_send_cmd(puc_data, us_addr, us_len, RF_SPI_READ_BLOCK);
}

/**
 * \brief Read from RF transceiver. This is a non-blocking function.
 *
 * \param[in] us_addr RF215 register address
 * \param[out] puc_data Pointer to store read data
 * \param[in] us_len Number of bytes to read
 */
__always_inline void rf215_spi_read_no_block(uint16_t us_addr, uint8_t *puc_data, uint16_t us_len)
{
	rf215_spi_send_cmd(puc_data, us_addr, us_len, RF_SPI_READ_NO_BLOCK);
}

/**
 * \brief Write register to RF transceiver. This is a non-blocking function.
 *
 * \param[in] us_addr RF215 register address
 * \param[in] uc_reg_value Value to be written
 */
__always_inline void rf215_spi_reg_write(uint16_t us_addr, uint8_t uc_reg_value)
{
	rf215_spi_send_cmd(&uc_reg_value, us_addr, 1, RF_SPI_WRITE);
}

/**
 * \brief Write to RF transceiver. This is a non-blocking function.
 *
 * \param[in] us_addr RF215 register address
 * \param[in] puc_data Pointer to data to be written
 * \param[in] us_len Number of bytes to write
 */
__always_inline void rf215_spi_write(uint16_t us_addr, uint8_t *puc_data, uint16_t us_len)
{
	rf215_spi_send_cmd(puc_data, us_addr, us_len, RF_SPI_WRITE);
}

/**
 * \brief Blocking function to wait for SPI to be free.
 */
__always_inline void rf215_spi_wait_free(void)
{
	uint32_t ul_timeout = 500000;
	while (gx_rf215_hal_wrp.rf_is_spi_busy() && (ul_timeout > 0)) {
		ul_timeout--;
	}
}

#ifdef __cplusplus
}
#endif

#endif  /* RF215_SPI_H_INCLUDE */
