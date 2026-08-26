/**
 *
 * \file
 *
 * \brief Proxy RF Controller interface layer implementation.
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

#ifndef PRF_IF_H_INCLUDED
#define PRF_IF_H_INCLUDED

#include "compiler.h"

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* @endcond */

/**
 * \ingroup rf_group
 * \defgroup prf_rf_group Proxy RF Controller
 *
 * This module provides configuration and utils to control the AT86RF215
 * device through SPI and RST and IRQ pins.
 *
 * @{
 */

/* \name PRF interrupt priority
 * \note Highest priority allowed is 2 (the lowest value is 2) */
/* @{ */
#define PRF_PRIO          2
/* @} */

/* \name PRF SPI modes */
/* @{ */
enum ESpiMode {
	PRF_SPI_WRITE = 0,
	PRF_SPI_READ_BLOCK = 1,
	PRF_SPI_READ_NO_BLOCK = 2
};
/* @} */

/* \name Proxy RF Controller interface
 * \note These API functions must not be called from interrupt priority higher
 * than 2, i.e. they must be called from interrupt with priority value >= 2 (or
 * not from interrupt) */
/* @{ */
uint8_t prf_if_init(void);
void prf_if_reset(void);
void prf_if_enable_interrupt(bool b_enable);
void prf_if_set_handler(void (*p_handler)(void));
bool prf_if_send_spi_cmd(uint8_t *puc_data_buf, uint16_t us_addr, uint16_t us_len, uint8_t uc_mode);
bool prf_if_is_spi_busy(void);
void prf_if_led(uint8_t uc_led_id, bool b_led_on);

/* @} */

/* @} */

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* @endcond */

#endif /* PRF_IF_H_INCLUDED */
