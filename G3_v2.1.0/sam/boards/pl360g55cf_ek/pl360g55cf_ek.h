/**
 * \file
 *
 * \brief PL360G55CF_EK Board Definition.
 *
 * Copyright (c) 2018 Atmel Corporation. All rights reserved.
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

#ifndef PL360G55CF_EK_H_INCLUDED
#define PL360G55CF_EK_H_INCLUDED

#include "compiler.h"
#include "conf_board.h"

/**
 * \ingroup group_common_boards
 * \defgroup pl360g55cf_ek_group "PL360G55CF_EK"
 * Evaluation Module for pl360 devices.
 *
 * @{
 */

/**
 * \defgroup pl360g55cf_ek_board_info_group "PL360G55CF_EK - Board informations"
 * Definitions related to the board description.
 *
 * @{
 */

/** Name of the board */
#define BOARD_NAME "PL360G55CF_EK"
/** Board definition */
#define pl360g55cf_ek
/** Family definition (already defined) */
#define samg55
/** Core definition */
#define cortexm4
/** Board revision definition */
#define BOARD_REV_1     1

#ifndef BOARD_REV
#define BOARD_REV BOARD_REV_1
#endif

/* @} */

/**
 *  \defgroup pl360g55cf_ek_opfreq_group "PL360G55CF_EK - Operating frequencies"
 *  Definitions related to the board operating frequency.
 *
 *  @{
 */

/**
 * \name Board oscillator settings
 * @{
 */
#define BOARD_FREQ_SLCK_XTAL        (32768U)
#define BOARD_FREQ_SLCK_BYPASS      (32768U)
#define BOARD_FREQ_MAINCK_XTAL      0 /* Not Mounted */
#define BOARD_FREQ_MAINCK_BYPASS    0 /* Not Mounted */
#define BOARD_MCK                   CHIP_FREQ_CPU_MAX
#define BOARD_OSC_STARTUP_US        15625U
/* @} */

/**
 * \defgroup pl360g55cf_ek_features_group "PL360G55CF_EK - Features"
 * Symbols that describe features and capabilities of the board.
 *
 * @{
 */

/**
 * \name LED pin definition
 * @{
 */
#define LED0_GPIO            (PIO_PA12_IDX)
#define LED0_ACTIVE_LEVEL    IOPORT_PIN_LEVEL_LOW
#define LED0_INACTIVE_LEVEL  IOPORT_PIN_LEVEL_HIGH

/* Wrapper macros to ensure common naming across all boards */
#define LED_0_NAME           "user LED0"
#define PIN_LED_0            {PIO_PA12, PIOA, ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_LED_0_MASK       PIO_PA12
#define PIN_LED_0_PIO        PIOA
#define PIN_LED_0_ID         ID_PIOA
#define PIN_LED_0_TYPE       PIO_OUTPUT_1
#define PIN_LED_0_ATTR       PIO_DEFAULT
/* @} */

/**
 * \name LED1 pin definition
 * @{
 */
#define LED1_GPIO            (PIO_PA15_IDX)
#define LED1_ACTIVE_LEVEL    IOPORT_PIN_LEVEL_LOW
#define LED1_INACTIVE_LEVEL  IOPORT_PIN_LEVEL_HIGH

/* Wrapper macros to ensure common naming across all boards */
#define LED_1_NAME           "user LED1"
#define PIN_LED_1            {PIO_PA15, PIOA, ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_LED_1_MASK       PIO_PA15
#define PIN_LED_1_PIO        PIOA
#define PIN_LED_1_ID         ID_PIOA
#define PIN_LED_1_TYPE       PIO_OUTPUT_1
#define PIN_LED_1_ATTR       PIO_DEFAULT
/* @} */

/**
 * \name TC pins definition
 * @{
 */
#define TC_TIOA1_GPIO        (PIO_PA23_IDX)
#define TC_TIOA1_MUX         (IOPORT_MODE_MUX_B)
#define TC_TIOA1_FLAGS       (IOPORT_MODE_MUX_B)
/* @} */

/**
 * \name WAKEUP pin definitions [MIKRO BUS]
 * @{
 */
#define WKUP8_GPIO           (PIO_PA14_IDX)
/* @} */

/**
 * \name AD pin definitions [MIKRO BUS]
 * @{
 */
#define AD0_GPIO             (PIO_PA17_IDX)
/* @} */

/**
 * \name Voltage Monitor definitions
 * @{
 */
#define VOL_MON_GPIO         (PIO_PA19_IDX)
/* @} */

/**
 * \name SPI 3 pin definitions [ATPL360]
 * @{
 */
#define SPI3_MISO_GPIO       (PIO_PA4_IDX)
#define SPI3_MISO_FLAGS      (IOPORT_MODE_MUX_A)
#define SPI3_MOSI_GPIO       (PIO_PA3_IDX)
#define SPI3_MOSI_FLAGS      (IOPORT_MODE_MUX_A)
#define SPI3_SPCK_GPIO       (PIO_PB13_IDX)
#define SPI3_SPCK_FLAGS      (IOPORT_MODE_MUX_A)
#define SPI3_NPCS0_GPIO      (PIO_PB14_IDX)
#define SPI3_NPCS0_FLAGS     (IOPORT_MODE_MUX_A)

#define BOARD_FLEXCOM_SPI3   FLEXCOM3
#define ATPL360_SPI          SPI3
#define ATPL360_SPI_CS       0
/* @} */

/**
 * \name PL360 GPIO pin definitions
 * @{
 */
#define ATPL360_GPIO0        (PIO_PA31_IDX)
#define ATPL360_GPIO1        (PIO_PA5_IDX)
#define ATPL360_GPIO2        (PIO_PA1_IDX)
#define ATPL360_GPIO3        (PIO_PA0_IDX)
#define ATPL360_GPIO4        (PIO_PA16_IDX)
#define ATPL360_GPIO5        (PIO_PA06_IDX)
/* @} */

/**
 * \name ATPL360 Reset pin definition
 * @{
 */
#define ATPL360_RESET_GPIO               PIO_PA29_IDX
#define ATPL360_RESET_ACTIVE_LEVEL       IOPORT_PIN_LEVEL_LOW
#define ATPL360_RESET_INACTIVE_LEVEL     IOPORT_PIN_LEVEL_HIGH
/* @} */

/**
 * \name ATPL360 LDO Enable pin definition
 * @{
 */
#define ATPL360_LDO_EN_GPIO              PIO_PA30_IDX
#define ATPL360_LDO_EN_ACTIVE_LEVEL      IOPORT_PIN_LEVEL_HIGH
#define ATPL360_LDO_EN_INACTIVE_LEVEL    IOPORT_PIN_LEVEL_LOW
/* @} */

/**
 * \name ATPL360 Carrier Detect Enable pin definition
 * @{
 */
#define ATPL360_CD_GPIO      ATPL360_GPIO0
/* @} */

/**
 * \name ATPL360 interrupt pin definition
 * @{
 */
#define ATPL360_INT_GPIO     ATPL360_GPIO3
#define ATPL360_INT_FLAGS    IOPORT_MODE_DEBOUNCE
#define ATPL360_INT_SENSE    IOPORT_SENSE_FALLING

#define ATPL360_INT          {PIO_PA0, PIOA, ID_PIOA, PIO_INPUT, PIO_DEGLITCH | PIO_IT_LOW_LEVEL}
#define ATPL360_INT_MASK     PIO_PA0
#define ATPL360_INT_PIO      PIOA
#define ATPL360_INT_ID       ID_PIOA
#define ATPL360_INT_TYPE     PIO_INPUT
#define ATPL360_INT_ATTR     (PIO_DEGLITCH | PIO_IT_LOW_LEVEL)
#define ATPL360_INT_IRQn     PIOA_IRQn
/* @} */

/**
 * \name USART4 pins definition [MIKRO BUS]
 * @{
 */
#define PINS_USART4             (PIO_PB11A_RXD4 | PIO_PB10A_TXD4)
#define PINS_USART4_FLAGS       (IOPORT_MODE_MUX_A)

#define PINS_USART4_PORT        IOPORT_PIOB
#define PINS_USART4_MASK        (PIO_PB11A_RXD4 | PIO_PB10A_TXD4)
#define PINS_USART4_PIO         PIOB
#define PINS_USART4_ID          ID_PIOB
#define PINS_USART4_TYPE        PIO_PERIPH_A
#define PINS_USART4_ATTR        PIO_DEFAULT

#define BOARD_FLEXCOM_USART4    FLEXCOM4
#define MIKRO_BUS_USART         USART4
/* @} */

/**
 * \name SPI 0 pin definitions [MIKRO BUS]
 * @{
 */
#define SPI0_MISO_GPIO       (PIO_PA9_IDX)
#define SPI0_MISO_FLAGS      (IOPORT_MODE_MUX_A)
#define SPI0_MOSI_GPIO       (PIO_PA10_IDX)
#define SPI0_MOSI_FLAGS      (IOPORT_MODE_MUX_A)
#define SPI0_SPCK_GPIO       (PIO_PB0_IDX)
#define SPI0_SPCK_FLAGS      (IOPORT_MODE_MUX_A)
#define SPI0_NPCS0_GPIO      (PIO_PA25_IDX)
#define SPI0_NPCS0_FLAGS     (IOPORT_MODE_MUX_A)

#define BOARD_FLEXCOM_SPI0   FLEXCOM0
#define MIKROBUS_SPI         SPI0
#define MIKROBUS_SPI_CS      0
/* @} */

/**
 * \name TWI 1 pin definitions [MIKRO BUS]
 * @{
 */
#define TWI1_DATA_GPIO       (PIO_PB3_IDX)
#define TWI1_DATA_FLAG       (IOPORT_MODE_MUX_A)
#define TWI1_CLK_GPIO        (PIO_PB2_IDX)
#define TWI1_CLK_FLAG        (IOPORT_MODE_MUX_A)

#define BOARD_FLEXCOM_TWI1   FLEXCOM1
#define MIKRO_BUS_TWI        TWI1
/* @} */

/** \name USB definitions
 * @{
 */
#define PIN_USB_VBUS         {PIO_PA11, PIOA, ID_PIOA, PIO_INPUT, PIO_PULLUP}
#define USB_VBUS_FLAGS       (PIO_INPUT | PIO_DEBOUNCE | PIO_IT_EDGE)
#define USB_VBUS_PIN_IRQn    (PIOA_IRQn)
#define USB_VBUS_PIN         (PIO_PA11_IDX)
#define USB_VBUS_PIO_ID      (ID_PIOA)
#define USB_VBUS_PIO_MASK    (PIO_PA11)

/** USB D- pin (System function) */
#define PIN_USB_DM           {PIO_PA21}
/** USB D+ pin (System function) */
#define PIN_USB_DP           {PIO_PA22}
/** @} */

/** PCK1 pin definition (PA17) : Use AD0 on MIKRO BUS connector */
#define PIN_PCK1         (PIO_PA17_IDX)
#define PIN_PCK1_MUX     (IOPORT_MODE_MUX_B)
#define PIN_PCK1_FLAGS   (IOPORT_MODE_MUX_B)
#define PIN_PCK1_PORT    IOPORT_PIOA
#define PIN_PCK1_MASK    PIO_PA17B_PCK1
#define PIN_PCK1_PIO     PIOA
#define PIN_PCK1_ID      ID_PIOA
#define PIN_PCK1_TYPE    PIO_PERIPH_B
#define PIN_PCK1_ATTR    PIO_DEFAULT

/* @} */ /* End of pl360g55cb_features_group */

/* @} */ /* End of pl360g55cf_ek_group */

#endif  /* PL360G55CF_EK_H_INCLUDED */
