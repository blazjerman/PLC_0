/**
 *
 * \file
 *
 * \brief PRF interface Configuration.
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

#ifndef CONF_PRF_IF_H_INCLUDE
#define CONF_PRF_IF_H_INCLUDE

#include "conf_board.h"

/* Select the SPI module and Chip Select that PRF is connected to */
#define PRF_SPI_MODULE             SPI3
#define PRF_SPI_CS                 0
#define PRF_SPI_ID                 ID_FLEXCOM3
#define PRF_SPI_Handler            FLEXCOM3_Handler

/* Interrupt pin definition */
#define PRF_INT_GPIO               MIKROBUS1_PIN_INT
#define PRF_INT_ATTR               PIO_IT_HIGH

/* Reset pin definition */
#define PRF_RESET_GPIO             MIKROBUS1_PIN_RST
#define PRF_RESET_ACTIVE_LEVEL     IOPORT_PIN_LEVEL_LOW
#define PRF_RESET_INACTIVE_LEVEL   IOPORT_PIN_LEVEL_HIGH

/* LED pins definition */
#define PRF_LED_TX_GPIO            MIKROBUS1_PIN_TWI_SDA
#define PRF_LED_RX_GPIO            MIKROBUS1_PIN_TWI_SCL
#define PRF_LED_ACTIVE_LEVEL       IOPORT_PIN_LEVEL_HIGH
#define PRF_LED_INACTIVE_LEVEL     IOPORT_PIN_LEVEL_LOW

/* Maximum message length to be used by upper layers */
/* RF215 buffer size is 2047 bytes */
/* It should be the same as AT86RF215_MAX_PSDU_LEN */
#define PRF_TRX_MAX_MSG_LEN        571

/* Enable auto configuration of SPI clock frequency and delays */
/* The auto configuration finds best choice to comply with RF215 requirements */
#define PRF_ENABLE_SPICLK_AUTO_CONFIGURE

#ifndef PRF_ENABLE_SPICLK_AUTO_CONFIGURE
/* SPI clock frequency and delay if auto configuration is disabled */
# define PRF_SPI_CLOCK             13700000
# define PRF_SPI_DLYBS             5
# define PRF_SPI_DLYBCT            1
#endif

#endif  /* CONF_PRF_IF_H_INCLUDE */
