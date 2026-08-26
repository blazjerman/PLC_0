/**
 * \file
 *
 * \brief PIC32CXMTSH_DB Board Definition.
 *
 * Copyright (c) 2020 Atmel Corporation. All rights reserved.
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

#ifndef PIC32CXMTSH_DB_H_INCLUDED
#define PIC32CXMTSH_DB_H_INCLUDED

#include "compiler.h"
#include "conf_board.h"

/**
 * \ingroup group_common_boards
 * \defgroup pic32cxmtsh_db_group "PIC32CXMTSH_DB"
 * Develop Board for metrology devices.
 *
 * @{
 */

/**
 * \defgroup PIC32CXMTSH_DB_board_info_group "PIC32CXMTSH_DB - Board informations"
 * Definitions related to the board description.
 *
 * @{
 */

/** Name of the board */
#define BOARD_NAME "PIC32CXMTSH_DB"
/** Board definition */
#define pic32cxmtsh_db /** Family definition (already defined) */
#define pic32cx
/** Core definition */
#define cortexm4
/** Board revision definition */
#define BOARD_REV_1     1

#ifndef BOARD_REV
#define BOARD_REV BOARD_REV_1
#endif

/* @} */

/**
 *  \defgroup PIC32CXMTSH_DB_opfreq_group "PIC32CXMTSH_DB - Operating frequencies"
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
#define BOARD_FREQ_MAINCK_XTAL      (16384000U)
#define BOARD_FREQ_MAINCK_BYPASS    (16384000U)
/* @} */

/** Master clock frequency */
#define BOARD_MCK                   CHIP_FREQ_CPU_MAX

/** board main clock xtal statup time */
#define BOARD_OSC_STARTUP_US        15625U

/* @} */

/**
 * \defgroup PIC32CXMTSH_DB_features_group "PIC32CXMTSH_DB - Features"
 * Symbols that describe features and capabilities of the board.
 *
 * @{
 */

/**
 * \name VISIBLE LED #0 pin definition
 * @{
 */
#define LED0_GPIO                        (PIO_PD17_IDX)
#define LED0_ACTIVE_LEVEL                IOPORT_PIN_LEVEL_HIGH
#define LED0_INACTIVE_LEVEL              IOPORT_PIN_LEVEL_LOW

/* Wrapper macros to ensure common naming across all boards */
#define LED_0_NAME                       "Visible LED"
#define PIN_LED_0                        {PIO_PD17, PIOD, ID_PIOD, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_LED_0_MASK                   PIO_PD17
#define PIN_LED_0_PIO                    PIOD
#define PIN_LED_0_ID                     ID_PIOD
#define PIN_LED_0_TYPE                   PIO_OUTPUT_1
#define PIN_LED_0_ATTR                   PIO_DEFAULT
/* @} */

/**
 * \name Push button #1 definition
 * Attributes = pull-up + debounce + interrupt on rising edge.
 * @{
 */
#define PUSHBUTTON_1_NAME        "SCROLL_DOWN"
#define GPIO_PUSH_BUTTON_1       (PIO_PA15_IDX)
#define GPIO_PUSH_BUTTON_1_FLAGS (IOPORT_MODE_PULLUP | IOPORT_MODE_DEBOUNCE)
#define GPIO_PUSH_BUTTON_1_SENSE (IOPORT_SENSE_FALLING)

#define PIN_PUSHBUTTON_1       {PIO_PA15, PIOA, ID_PIOA, PIO_INPUT, \
				PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_FALLING}
#define PIN_PUSHBUTTON_1_MASK  PIO_PA15
#define PIN_PUSHBUTTON_1_PIO   PIOA
#define PIN_PUSHBUTTON_1_ID    ID_PIOA
#define PIN_PUSHBUTTON_1_TYPE  PIO_INPUT
#define PIN_PUSHBUTTON_1_ATTR  (PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_FALLING)
#define PIN_PUSHBUTTON_1_IRQn  PIOA_IRQn
/* @} */

/**
 * \name Push button #2 definition
 * Attributes = pull-up + debounce + interrupt on rising edge.
 * @{
 */
#define PUSHBUTTON_2_NAME        "SCROLL_UP"
#define GPIO_PUSH_BUTTON_2       (PIO_PA14_IDX)
#define GPIO_PUSH_BUTTON_2_FLAGS (IOPORT_MODE_PULLUP | IOPORT_MODE_DEBOUNCE)
#define GPIO_PUSH_BUTTON_2_SENSE (IOPORT_SENSE_FALLING)

#define PIN_PUSHBUTTON_2       {PIO_PA14, PIOA, ID_PIOA, PIO_INPUT, \
				PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_FALLING}
#define PIN_PUSHBUTTON_2_MASK  PIO_PA14
#define PIN_PUSHBUTTON_2_PIO   PIOA
#define PIN_PUSHBUTTON_2_ID    ID_PIOA
#define PIN_PUSHBUTTON_2_TYPE  PIO_INPUT
#define PIN_PUSHBUTTON_2_ATTR  (PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_FALLING)
#define PIN_PUSHBUTTON_2_IRQn  PIOA_IRQn
/* @} */

/** List of all push button definitions. */
#define PINS_PUSHBUTTONS    {PIN_PUSHBUTTON_1, PIN_PUSHBUTTON_2}

/**
 * \PWM Pulse Output 0 Opto-Port pin definitions.
 * @{
 */
#define PIN_PWM_OUT0_GPIO                PIO_PD17_IDX
#define PIN_PWM_OUT0_FLAGS               (IOPORT_MODE_MUX_A)
#define PIN_PWM_OUT0_CHANNEL             PWM_CHANNEL_0
/* @} */

/**
 * \PWM Pulse Output 1 Opto-Port pin definitions.
 * @{
 */
#define PIN_PWM_OUT1_GPIO                PIO_PD18_IDX
#define PIN_PWM_OUT1_FLAGS               (IOPORT_MODE_MUX_A)
#define PIN_PWM_OUT1_CHANNEL             PWM_CHANNEL_1
/* @} */

/**
 * \PWM Pulse Output 2 Opto-Port pin definitions.
 * @{
 */
#define PIN_PWM_OUT2_GPIO                PIO_PD19_IDX
#define PIN_PWM_OUT2_FLAGS               (IOPORT_MODE_MUX_A)
#define PIN_PWM_OUT2_CHANNEL             PWM_CHANNEL_2
/* @} */

/**
 * \name TC pins definition
 * @{
 */
#define PIN_TC0_TIOA0_GPIO               (PIO_PA2_IDX)
#define PIN_TC0_TIOA0_MUX                (IOPORT_MODE_MUX_B)
#define PIN_TC0_TIOA0_FLAGS              (IOPORT_MODE_MUX_B)

#define PIN_TC0_TIOA1_GPIO               (PIO_PA31_IDX)
#define PIN_TC0_TIOA1_MUX                (IOPORT_MODE_MUX_D)
#define PIN_TC0_TIOA1_FLAGS              (IOPORT_MODE_MUX_D)

#define PIN_TC0_TIOA2_GPIO               (PIO_PB2_IDX)
#define PIN_TC0_TIOA2_MUX                (IOPORT_MODE_MUX_D)
#define PIN_TC0_TIOA2_FLAGS              (IOPORT_MODE_MUX_D)
/* @} */

/**
 * \name Console Port USART pins definitions
 * @{
 */
#define PINS_CONSOLE_UART                  (PIO_PA4A_FLEXCOM0_IO0 | PIO_PA5A_FLEXCOM0_IO1)
#define PINS_CONSOLE_UART_FLAGS            (IOPORT_MODE_MUX_A)

#define PINS_CONSOLE_UART_PORT             IOPORT_PIOA
#define PINS_CONSOLE_UART_MASK             (PIO_PA4A_FLEXCOM0_IO0 | PIO_PA5A_FLEXCOM0_IO1)
#define PINS_CONSOLE_UART_PIO              PIOA
#define PINS_CONSOLE_UART_ID               ID_PIOA
#define PINS_CONSOLE_UART_TYPE             IOPORT_MODE_MUX_A
#define PINS_CONSOLE_UART_ATTR             IOPORT_MODE_DEFAULT
/* @} */

/**
 * \name UART Opto-port pins (URXD, UTXD) definitions
 * @{
 */
#define PINS_OPTO_UART                  (PIO_PD1A_URXD | PIO_PD2A_UTXD)
#define PINS_OPTO_UART_FLAGS            (IOPORT_MODE_MUX_A)

#define PINS_OPTO_UART_PORT             IOPORT_PIOD
#define PINS_OPTO_UART_MASK             (PIO_PD1A_URXD | PIO_PD2A_UTXD)
#define PINS_OPTO_UART_PIO              PIOD
#define PINS_OPTO_UART_ID               ID_PIOD
#define PINS_OPTO_UART_TYPE             IOPORT_MODE_MUX_A
#define PINS_OPTO_UART_ATTR             IOPORT_MODE_DEFAULT
/* @} */

/**
 * \name QSPI pins definitions
 * @{
 */
#define QSPI_QSCK_GPIO                  PIO_PC15_IDX
#define QSPI_QSCK_FLAGS                 (IOPORT_MODE_MUX_A)
#define QSPI_QCS_GPIO                   PIO_PC14_IDX
#define QSPI_QCS_FLAGS                  (IOPORT_MODE_MUX_A)
#define QSPI_QIO0_GPIO                  PIO_PC13_IDX
#define QSPI_QIO0_FLAGS                 (IOPORT_MODE_MUX_A)
#define QSPI_QIO1_GPIO                  PIO_PC12_IDX
#define QSPI_QIO1_FLAGS                 (IOPORT_MODE_MUX_A)
#define QSPI_QIO2_GPIO                  PIO_PC11_IDX
#define QSPI_QIO2_FLAGS                 (IOPORT_MODE_MUX_A)
#define QSPI_QIO3_GPIO                  PIO_PC10_IDX
#define QSPI_QIO3_FLAGS                 (IOPORT_MODE_MUX_A | IOPORT_MODE_PULLUP)
/* @} */

/* ! \name Xplain PRO Extension header pin definitions */
/* @{ */
#define XPLAIN_PRO_PIN_3                PIO_PA30_IDX
#define XPLAIN_PRO_PIN_4                PIO_PB1_IDX
#define XPLAIN_PRO_PIN_5                PIO_PA18_IDX
#define XPLAIN_PRO_PIN_6                PIO_PA20_IDX
#define XPLAIN_PRO_PIN_7                PIO_PD3_IDX
#define XPLAIN_PRO_PIN_8                PIO_PD16_IDX
#define XPLAIN_PRO_PIN_9                PIO_PA3_IDX
#define XPLAIN_PRO_PIN_10               PIO_PA2_IDX
#define XPLAIN_PRO_PIN_11               PIO_PA16_IDX
#define XPLAIN_PRO_PIN_12               PIO_PA17_IDX
#define XPLAIN_PRO_PIN_13               PIO_PA13_IDX
#define XPLAIN_PRO_PIN_14               PIO_PA12_IDX
#define XPLAIN_PRO_PIN_15               PIO_PA11_IDX
#define XPLAIN_PRO_PIN_16               PIO_PA8_IDX
#define XPLAIN_PRO_PIN_17               PIO_PA9_IDX
#define XPLAIN_PRO_PIN_18               PIO_PA10_IDX
/* @} */

/* ! \name Xplain PRO Extension header pin definitions by function */
/* @{ */
#define XPLAIN_PRO_PIN_ADC_0            XPLAIN_PRO_PIN_3
#define XPLAIN_PRO_PIN_ADC_1            XPLAIN_PRO_PIN_4
#define XPLAIN_PRO_PIN_GPIO_1           XPLAIN_PRO_PIN_5
#define XPLAIN_PRO_PIN_GPIO_2           XPLAIN_PRO_PIN_6
#define XPLAIN_PRO_PIN_PWM_0            XPLAIN_PRO_PIN_7
#define XPLAIN_PRO_PIN_PWM_1            XPLAIN_PRO_PIN_8
#define XPLAIN_PRO_PIN_IRQ              XPLAIN_PRO_PIN_9
#define XPLAIN_PRO_PIN_TWI_SDA          XPLAIN_PRO_PIN_11
#define XPLAIN_PRO_PIN_TWI_SCL          XPLAIN_PRO_PIN_12
#define XPLAIN_PRO_PIN_UART_RX          XPLAIN_PRO_PIN_13
#define XPLAIN_PRO_PIN_UART_TX          XPLAIN_PRO_PIN_14
#define XPLAIN_PRO_PIN_SPI_SS_1         XPLAIN_PRO_PIN_10
#define XPLAIN_PRO_PIN_SPI_SS_0         XPLAIN_PRO_PIN_15
#define XPLAIN_PRO_PIN_SPI_MOSI         XPLAIN_PRO_PIN_16
#define XPLAIN_PRO_PIN_SPI_MISO         XPLAIN_PRO_PIN_17
#define XPLAIN_PRO_PIN_SPI_SCK          XPLAIN_PRO_PIN_18
/* @} */

/**
 * \name XPLAIN Port USART pins definitions
 * @{
 */
#define PINS_XPLAIN_USART                 (PIO_PA12A_FLEXCOM2_IO0 | PIO_PA13A_FLEXCOM2_IO1)
#define PINS_XPLAIN_USART_FLAGS           (IOPORT_MODE_MUX_A)

#define PINS_XPLAIN_USART_PORT            IOPORT_PIOA
#define PINS_XPLAIN_USART_MASK            (PIO_PA12A_FLEXCOM2_IO0 | PIO_PA13A_FLEXCOM2_IO1)
#define PINS_XPLAIN_USART_PIO             PIOA
#define PINS_XPLAIN_USART_ID              ID_PIOA
#define PINS_XPLAIN_USART_TYPE            IOPORT_MODE_MUX_A
#define PINS_XPLAIN_USART_ATTR            IOPORT_MODE_DEFAULT
/* @} */

/**
 * \name XPLAIN Port TWI pins definitions
 * @{
 */
#define PINS_XPLAIN_TWI                   (PIO_PA16A_FLEXCOM3_IO0 | PIO_PA17A_FLEXCOM3_IO1)
#define PINS_XPLAIN_TWI_FLAGS             (IOPORT_MODE_MUX_A | IOPORT_MODE_PULLUP)

#define PINS_XPLAIN_TWI_PORT              IOPORT_PIOA
#define PINS_XPLAIN_TWI_MASK              (PIO_PA16A_FLEXCOM3_IO0 | PIO_PA17A_FLEXCOM3_IO1)
#define PINS_XPLAIN_TWI_PIO               PIOA
#define PINS_XPLAIN_TWI_ID                ID_PIOA
#define PINS_XPLAIN_TWI_TYPE              IOPORT_MODE_MUX_A
#define PINS_XPLAIN_TWI_ATTR              IOPORT_MODE_PULLUP
/* @} */

/**
 * \name XPLAIN Port SPI pins definitions
 * @{
 */
#define XPLAIN_SPI_MOSI_GPIO    (XPLAIN_PRO_PIN_SPI_MOSI)
#define XPLAIN_SPI_MOSI_FLAGS   (IOPORT_MODE_MUX_A)
#define XPLAIN_SPI_MISO_GPIO    (XPLAIN_PRO_PIN_SPI_MISO)
#define XPLAIN_SPI_MISO_FLAGS   (IOPORT_MODE_MUX_A)
#define XPLAIN_SPI_SPCK_GPIO    (XPLAIN_PRO_PIN_SPI_SCK)
#define XPLAIN_SPI_SPCK_FLAGS   (IOPORT_MODE_MUX_A)
#define XPLAIN_SPI_NPCS0_GPIO   (XPLAIN_PRO_PIN_SPI_SS_0)
#define XPLAIN_SPI_NPCS0_FLAGS  (IOPORT_MODE_MUX_A)
#define XPLAIN_SPI_NPCS1_GPIO   (XPLAIN_PRO_PIN_SPI_SS_1)
#define XPLAIN_SPI_NPCS1_FLAGS  (IOPORT_MODE_MUX_B)
/* @} */

/* ! \name mikroBUS Extension header pin definitions */
/* @{ */
#define MIKROBUS_PIN_1                  PIO_PA31_IDX
#define MIKROBUS_PIN_2                  PIO_PB26_IDX
#define MIKROBUS_PIN_3                  PIO_PC6_IDX
#define MIKROBUS_PIN_4                  PIO_PC7_IDX
#define MIKROBUS_PIN_5                  PIO_PC8_IDX
#define MIKROBUS_PIN_6                  PIO_PC9_IDX
#define MIKROBUS_PIN_11                 PIO_PC21_IDX
#define MIKROBUS_PIN_12                 PIO_PC20_IDX
#define MIKROBUS_PIN_13                 PIO_PC16_IDX
#define MIKROBUS_PIN_14                 PIO_PC17_IDX
#define MIKROBUS_PIN_15                 PIO_PB25_IDX
#define MIKROBUS_PIN_16                 PIO_PD16_IDX
/* @} */

/* ! \name mikroBUS Extension header pin definitions by function */
/* @{ */
#define MIKROBUS_PIN_AN                 MIKROBUS_PIN_1
#define MIKROBUS_PIN_PWM                MIKROBUS_PIN_16
#define MIKROBUS_PIN_RST                MIKROBUS_PIN_2
#define MIKROBUS_PIN_INT                MIKROBUS_PIN_15
#define MIKROBUS_PIN_SPI_CS             MIKROBUS_PIN_3
#define MIKROBUS_PIN_SPI_SCK            MIKROBUS_PIN_4
#define MIKROBUS_PIN_SPI_MISO           MIKROBUS_PIN_5
#define MIKROBUS_PIN_SPI_MOSI           MIKROBUS_PIN_6
#define MIKROBUS_PIN_UART_RX            MIKROBUS_PIN_14
#define MIKROBUS_PIN_UART_TX            MIKROBUS_PIN_13
#define MIKROBUS_PIN_TWI_SCL            MIKROBUS_PIN_12
#define MIKROBUS_PIN_TWI_SDA            MIKROBUS_PIN_11
/* @} */

/**
 * \name MIKROBUS Port PWM pin definitions
 * @{
 */
#define PIN_MIKROBUS_PWM_GPIO                MIKROBUS_PIN_PWM
#define PIN_MIKROBUS_PWM_FLAGS               (IOPORT_MODE_MUX_B)
#define PIN_MIKROBUS_PWM_CHANNEL             PWM_CHANNEL_2
/* @} */

/**
 * \name MIKROBUS Port USART pins definitions
 * @{
 */
#define PINS_MIKROBUS_USART                 (PIO_PC16A_FLEXCOM6_IO0 | PIO_PC17A_FLEXCOM6_IO1)
#define PINS_MIKROBUS_USART_FLAGS           (IOPORT_MODE_MUX_A)

#define PINS_MIKROBUS_USART_PORT            IOPORT_PIOC
#define PINS_MIKROBUS_USART_MASK            (PIO_PC16A_FLEXCOM6_IO0 | PIO_PC17A_FLEXCOM6_IO1)
#define PINS_MIKROBUS_USART_PIO             PIOC
#define PINS_MIKROBUS_USART_ID              ID_PIOC
#define PINS_MIKROBUS_USART_TYPE            IOPORT_MODE_MUX_A
#define PINS_MIKROBUS_USART_ATTR            IOPORT_MODE_DEFAULT
/* @} */

/**
 * \name MIKROBUS Port TWI pins definitions
 * @{
 */
#define PINS_MIKROBUS_TWI                   (PIO_PC21A_FLEXCOM7_IO0 | PIO_PC20A_FLEXCOM7_IO1)
#define PINS_MIKROBUS_TWI_FLAGS             (IOPORT_MODE_MUX_A)

#define PINS_MIKROBUS_TWI_PORT              IOPORT_PIOC
#define PINS_MIKROBUS_TWI_MASK              (PIO_PC21A_FLEXCOM7_IO0 | PIO_PC20A_FLEXCOM7_IO1)
#define PINS_MIKROBUS_TWI_PIO               PIOC
#define PINS_MIKROBUS_TWI_ID                ID_PIOC
#define PINS_MIKROBUS_TWI_TYPE              IOPORT_MODE_MUX_A
#define PINS_MIKROBUS_TWI_ATTR              IOPORT_MODE_DEFAULT
/* @} */

/**
 * \name MIKROBUS Port SPI pins definitions
 * @{
 */
#define MIKROBUS_SPI_MOSI_GPIO        (MIKROBUS_PIN_SPI_MOSI)
#define MIKROBUS_SPI_MOSI_FLAGS       (IOPORT_MODE_MUX_A)
#define MIKROBUS_SPI_MISO_GPIO        (MIKROBUS_PIN_SPI_MISO)
#define MIKROBUS_SPI_MISO_FLAGS       (IOPORT_MODE_MUX_A)
#define MIKROBUS_SPI_SPCK_GPIO        (MIKROBUS_PIN_SPI_SCK)
#define MIKROBUS_SPI_SPCK_FLAGS       (IOPORT_MODE_MUX_A)
#define MIKROBUS_SPI_NPCS0_GPIO       (MIKROBUS_PIN_SPI_CS)
#define MIKROBUS_SPI_NPCS0_FLAGS      (IOPORT_MODE_MUX_A)
/* @} */

/**
 * \name PCK pin definitions
 * @{
 */
#define PIN_PCK0        (PIO_PC22_IDX)
#define PIN_PCK0_MUX    (IOPORT_MODE_MUX_A)
/* #define PIN_PCK0        (PIO_PA20_IDX) */
/* #define PIN_PCK0_MUX    (IOPORT_MODE_MUX_A) */

/* #define PIN_PCK1        (PIO_PB1_IDX) */
/* #define PIN_PCK1_MUX    (IOPORT_MODE_MUX_C) */
#define PIN_PCK1        (PIO_PA2_IDX)
#define PIN_PCK1_MUX    (IOPORT_MODE_MUX_A)

#define PIN_PCK2        (PIO_PC5_IDX)
#define PIN_PCK2_MUX    (IOPORT_MODE_MUX_A)
/* @} */

/**
 * \name Voltage Monitor pins definition
 * @{
 */
#define VDD_SENSE_GPIO                  PIO_PB1_IDX
/* @} */

/* @} */ /* End of PIC32CXMTSH_DB_features_group */

/* @} */ /* End of PIC32CXMTSH_DB_group */

#endif  /* PIC32CXMTSH_DB_H_INCLUDED */
