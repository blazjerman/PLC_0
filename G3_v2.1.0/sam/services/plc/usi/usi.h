/**
 * \file
 *
 * \brief USI: PLC Service Universal Serial Interface Header
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

#ifndef USI_H_INCLUDE
#define USI_H_INCLUDE

/* System includes */
#include <stdint.h>
#include <stdbool.h>

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* @endcond */

/**
 * \ingroup plc_group
 * \defgroup usi_plc_group PLC Universal Serial Interface
 *
 * This module provides configuration and utils for the serialization of
 * protocols in PLC.
 *
 * @{
 */

/** \brief PLC Universal Serial Interface */
/* @{ */
/** Management Plane Protocol Spec and ATMEL serialized protocols */
typedef enum {
	PROTOCOL_MNGP_PRIME               = 0x00,
	PROTOCOL_MNGP_PRIME_GETQRY        = 0x00,
	PROTOCOL_MNGP_PRIME_GETRSP        = 0x01,
	PROTOCOL_MNGP_PRIME_SET           = 0x02,
	PROTOCOL_MNGP_PRIME_RESET         = 0x03,
	PROTOCOL_MNGP_PRIME_REBOOT        = 0x04,
	PROTOCOL_MNGP_PRIME_FU            = 0x05,
	PROTOCOL_MNGP_PRIME_GETQRY_EN     = 0x06,
	PROTOCOL_MNGP_PRIME_GETRSP_EN     = 0x07,
	PROTOCOL_SNIF_PRIME               = 0x13,
	PROTOCOL_PHY_SERIAL_PRIME         = 0x1F,
	PROTOCOL_PHY_ATPL2X0              = 0x22,
	PROTOCOL_SNIF_G3                  = 0x23,
	PROTOCOL_MAC_G3                   = 0x24,
	PROTOCOL_ADP_G3                   = 0x25,
	PROTOCOL_COORD_G3                 = 0x26,
	PROTOCOL_PHY_MICROPLC             = 0x27,
	PROTOCOL_PHY_RF215                = 0x28,
	PROTOCOL_PRIME_API                = 0x30,
	PROTOCOL_INTERNAL                 = 0x3F,
	PROTOCOL_USER_DEFINED             = 0xFE,
	PROTOCOL_INVALID                  = 0xFF
} usi_protocol_t;

/** Number of USI supported protocols */
#define USI_NUMBER_OF_PROTOCOLS            12

/** USI operation results */
typedef enum {
	USI_STATUS_PROTOCOL_NOT_FOUND,
	USI_STATUS_PROTOCOL_NOT_REGISTERED,
	USI_STATUS_TX_BUFFER_OVERFLOW,
	USI_STATUS_RX_BUFFER_OVERFLOW,
	USI_STATUS_UART_ERROR,
	USI_STATUS_FORMAT_ERROR,
	USI_STATUS_OK,
	USI_STATUS_INVALID
} usi_status_t;

/** Message Structure to communicate with USI layer */
typedef struct {
	uint8_t uc_protocol_type;    /* Protocol Type */
	uint8_t *ptr_buf;            /* Pointer to data buffer */
	uint16_t us_len;             /* Length of data */
} x_usi_serial_cmd_params_t;

/** \brief Universal Serial Interface */
/* @{ */
void usi_init(void);

usi_status_t usi_set_callback(usi_protocol_t protocol_id, uint8_t (*p_handler)(uint8_t *puc_rx_msg, uint16_t us_len), uint8_t serial_port);
void usi_process(void);
usi_status_t usi_send_cmd(void *msg);

/* @} */

/* @} */

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* @endcond */
#endif /* USI_H_INCLUDE */
