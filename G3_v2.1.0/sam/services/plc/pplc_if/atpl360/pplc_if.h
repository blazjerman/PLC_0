/**
 * \file
 *
 * \brief Proxy PLC Controller interface layer implementation.
 *
 * Copyright (c) 2014 Atmel Corporation. All rights reserved.
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

#ifndef CHIP_PPLC_IF_H_INCLUDED
#define CHIP_PPLC_IF_H_INCLUDED

#include "compiler.h"
#include "board.h"

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* @endcond */

/**
 * \ingroup plc_group
 * \defgroup pplc_plc_group Proxy PLC Controller
 *
 * This module provides configuration and utils to control the PLC interface
 * with the ATPL230 PHY layer.
 *
 * @{
 */

#include "atpl360_hal_spi.h"

/** \brief PLC commands */
/* @{ */
#define PPLC_CMD_READ                            ATPL360_CMD_READ
#define PPLC_CMD_WRITE                           ATPL360_CMD_WRITE
#define PPLC_WR_RD_POS                           ATPL360_WR_RD_POS
#define PPLC_LEN_MASK                            ATPL360_LEN_MASK

/** SPI Header field when bootload is in the other side of spi*/
#define PPLC_SPI_HEADER_BOOT                     ATPL360_SPI_HEADER_BOOT
/** SPI Header MASK for bootloader heade*/
#define PPLC_SPI_HEADER_BOOT_MASK                ATPL360_SPI_HEADER_BOOT_MASK
/** SPI Header field when atpl360 is in the other side of spi*/
#define PPLC_SPI_HEADER_CORTEX                   ATPL360_SPI_HEADER_CORTEX

#define PPLC_GET_ID_HEADER(b0, b1)               ATPL360_GET_ID_HEADER(b0, b1)
#define PPLC_GET_FLAGS_FROM_BOOT(b0, b2, b3)     ATPL360_GET_FLAGS_FROM_BOOT(b0, b2, b3)
#define PPLC_GET_FLAGS_FROM_CORTEX(b2, b3)       ATPL360_GET_FLAGS_FROM_CORTEX(b2, b3)

#define PPLC_CHECK_ID_BOOT_HEADER(val)           ATPL360_CHECK_ID_BOOT_HEADER(val)
#define PPLC_CHECK_ID_CORTEX_HEADER(val)         ATPL360_CHECK_ID_CORTEX_HEADER(val)
/* @} */

typedef struct spi_data {
	uint16_t us_len;
	uint16_t us_address;
	uint8_t *puc_data_buf;
} spi_data_t;

typedef struct spi_status_info {
	uint32_t ul_flags;
	uint16_t us_header_id;
} spi_status_info_t;

/* \name PPLC interrupt priority */

/* \note In case of use of FreeRTOS, GROUP_PRIO is greater value than
 * configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY */
/* @{ */
#define PPLC_PRIO          9 /* < PPLC interrupt group priority */
#define PPLC_ADC_PRIO      1 /* < PPLC PVDD monitor (ADC) interrupt group priority */
/* @} */

/* \name PPLC communication parameters */
/* @{ */
#define PPLC_DLYBS         ATPL360_SPI_DLYBS   /* < Delay before SPCK */
#define PPLC_DLYBCT        ATPL360_SPI_DLYBCT  /* < Delay between consecutive transfers */
/* @} */

/* \name PPLC Time reference delay */
/* @{ */
enum ETrefDelayMode {
	PPLC_DELAY_TREF_SEC = 0,
	PPLC_DELAY_TREF_MS,
	PPLC_DELAY_TREF_US
};
/* @} */

/* \name Proxy PLC Controller interface */
/* @{ */
void pplc_if_init(void);
void pplc_if_reset(void);
void pplc_if_config(void);
void pplc_if_set_handler(void (*p_handler)(void));
bool pplc_if_send_boot_cmd(uint16_t us_cmd, uint32_t ul_addr, uint32_t ul_data_len, uint8_t *puc_data_buf, uint8_t *puc_data_read);
bool pplc_if_send_wrrd_cmd(uint8_t uc_cmd, void *px_spi_data, void *px_spi_status_info);
void pplc_if_enable_interrupt(bool enable);
void pplc_if_delay(uint8_t uc_tref, uint32_t ul_delay);
bool pplc_if_set_stby_mode(bool sleep);
bool pplc_if_get_thermal_warning(void);
void pplc_if_crit_init(void);
void pplc_if_pvdd_mon_set_handler(void (*p_handler)(bool));

/* @} */
/* @} */

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* @endcond */

#endif /* CHIP_PPLC_IF_H_INCLUDED */
