/**
 * \file
 *
 * \brief PL485 EK board definition
 *
 * Copyright (c) 2019 Atmel Corporation. All rights reserved.
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

/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#ifndef PL485_EK_H_INCLUDED
#define PL485_EK_H_INCLUDED

#include <conf_board.h>
#include <compiler.h>

/**
 * \ingroup group_common_boards
 * \defgroup pl485_ek_group PL485 EK board
 *
 * @{
 */

void system_board_init(void);

/**
 * \defgroup samg55_config_group Configuration
 *
 * Symbols to use for configuring the board and its initialization.
 *
 * @{
 */
#ifdef __DOXYGEN__

/* ! \name Initialization */
/* @{ */

/**
 * \def CONF_BOARD_KEEP_WATCHDOG_AT_INIT
 * \brief If defined, the watchdog will remain enabled
 *
 * If this symbol is defined, the watchdog is left running with its current
 * configuration. Otherwise, it is disabled during board initialization.
 */
# ifndef CONF_BOARD_KEEP_WATCHDOG_AT_INIT
#  define CONF_BOARD_KEEP_WATCHDOG_AT_INIT
# endif

/* @} */

#endif /* __DOXYGEN__ */
/**@} */

/**
 * \defgroup pl485_ek_features_group Features
 *
 * Symbols that describe features and capabilities of the board.
 *
 * @{
 */

/** Name string macro */
#define BOARD_NAME                "PL485_EK"

/** \name Resonator definitions
 *  @{ */
#define BOARD_FREQ_SLCK_XTAL      (32768U)
#define BOARD_FREQ_SLCK_BYPASS    (32768U)
#define BOARD_FREQ_MAINCK_XTAL    0 /* Not Mounted */
#define BOARD_FREQ_MAINCK_BYPASS  0 /* Not Mounted */
#define BOARD_MCK                 CHIP_FREQ_CPU_MAX
/*TBD startup time needs to be adjusted according to measurements */
#define BOARD_OSC_STARTUP_US      15625

/** @} */

/**
 * \name User LED0 pin definition
 * @{
 */
#define LED0_GPIO                      (PIO_PA31_IDX)
#define LED0_ACTIVE_LEVEL              IOPORT_PIN_LEVEL_LOW
#define LED0_INACTIVE_LEVEL            IOPORT_PIN_LEVEL_HIGH

/* Wrapper macros to ensure common naming across all boards */
#define LED_0_NAME                     "user LED0"
#define PIN_LED_0                      {PIO_PA31, PIOA, ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_LED_0_MASK                 PIO_PA31
#define PIN_LED_0_PIO                  PIOA
#define PIN_LED_0_ID                   ID_PIOA
#define PIN_LED_0_TYPE                 PIO_OUTPUT_1
#define PIN_LED_0_ATTR                 PIO_DEFAULT
/* @} */

/**
 * \name User LED1 pin definition
 * @{
 */
#define LED1_GPIO                      (PIO_PA19_IDX)
#define LED1_ACTIVE_LEVEL              IOPORT_PIN_LEVEL_LOW
#define LED1_INACTIVE_LEVEL            IOPORT_PIN_LEVEL_HIGH

/* Wrapper macros to ensure common naming across all boards */
#define LED_1_NAME                     "user LED1"
#define PIN_LED_1                      {PIO_PA19, PIOA, ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_LED_1_MASK                 PIO_PA19
#define PIN_LED_1_PIO                  PIOA
#define PIN_LED_1_ID                   ID_PIOA
#define PIN_LED_1_TYPE                 PIO_OUTPUT_1
#define PIN_LED_1_ATTR                 PIO_DEFAULT
/* @} */

/* ! \name ATPL360 Internal Connection */
/* @{ */
/** ATPL360 MISO pin definition. */
#define ATPL360_MISO_GPIO                (PIO_PA27_IDX)
#define ATPL360_MISO_FLAGS               (IOPORT_MODE_MUX_B)
/** ATPL360 MOSI pin definition. */
#define ATPL360_MOSI_GPIO                (PIO_PA28_IDX)
#define ATPL360_MOSI_FLAGS               (IOPORT_MODE_MUX_B)
/** ATPL360 SPCK pin definition. */
#define ATPL360_SPCK_GPIO                (PIO_PA29_IDX)
#define ATPL360_SPCK_FLAGS               (IOPORT_MODE_MUX_B)
/** ATPL360 chip select pin definition. */
#define ATPL360_NPCS0_GPIO               (PIO_PA30_IDX)
#define ATPL360_NPCS0_FLAGS              (IOPORT_MODE_MUX_B)

#define BOARD_FLEXCOM_ATPL360            FLEXCOM7
#define ATPL360_SPI                      SPI7
#define ATPL360_SPI_CS                   0

/** ATPL360 Reset pin definition. */
#define ATPL360_RESET_GPIO               PIO_PA25_IDX
#define ATPL360_RESET_ACTIVE_LEVEL       IOPORT_PIN_LEVEL_LOW
#define ATPL360_RESET_INACTIVE_LEVEL     IOPORT_PIN_LEVEL_HIGH

/** ATPL360 LDO Enable pin definition. */
#define ATPL360_LDO_EN_GPIO              PIO_PB13_IDX
#define ATPL360_LDO_EN_ACTIVE_LEVEL      IOPORT_PIN_LEVEL_HIGH
#define ATPL360_LDO_EN_INACTIVE_LEVEL    IOPORT_PIN_LEVEL_LOW

/** ATPL360 TEST/STBY (Low Power Enable) pin definition. */
#define ATPL360_STBY_GPIO                PIO_PB15_IDX
#define ATPL360_STBY_ACTIVE_LEVEL        IOPORT_PIN_LEVEL_HIGH
#define ATPL360_STBY_INACTIVE_LEVEL      IOPORT_PIN_LEVEL_LOW

/** ATPL360 Carrier Detect Enable pin definition. */
#define ATPL360_CD_GPIO                  PIO_PA24_IDX

/** ATPL360 Interrupt pin definition. */
#define ATPL360_INT_GPIO                 PIO_PA26_IDX
#define ATPL360_INT_FLAGS                IOPORT_MODE_DEBOUNCE
#define ATPL360_INT_SENSE                IOPORT_SENSE_FALLING

#define ATPL360_INT                      {PIO_PA26, PIOA, ID_PIOA, PIO_INPUT, PIO_DEGLITCH | PIO_IT_LOW_LEVEL}
#define ATPL360_INT_MASK                 PIO_PA26
#define ATPL360_INT_PIO                  PIOA
#define ATPL360_INT_ID                   ID_PIOA
#define ATPL360_INT_TYPE                 PIO_INPUT
#define ATPL360_INT_ATTR                 (PIO_DEGLITCH | PIO_IT_LOW_LEVEL)
#define ATPL360_INT_IRQn                 PIOA_IRQn

/* @} */

/* ! \name I2S0 */
/* @{ */
/** I2S0 SCK pin definition. */
#define I2S0_SCK_GPIO                  (PIO_PA0_IDX)
#define I2S0_SCK_FLAGS                 (IOPORT_MODE_MUX_A)
/** I2S0 MCK pin definition. */
#define I2S0_MCK_GPIO                  (PIO_PA4_IDX)
#define I2S0_MCK_FLAGS                 (IOPORT_MODE_MUX_B)
/** I2S0 SDI pin definition. */
#define I2S0_SDI_GPIO                  (PIO_PA2_IDX)
#define I2S0_SDI_FLAGS                 (IOPORT_MODE_MUX_B)
/** I2S0 SDO pin definition. */
#define I2S0_SDO_GPIO                  (PIO_PA3_IDX)
#define I2S0_SDO_FLAGS                 (IOPORT_MODE_MUX_B)
/** I2S0 WS pin definition. */
#define I2S0_WS_GPIO                   (PIO_PA1_IDX)
#define I2S0_WS_FLAGS                  (IOPORT_MODE_MUX_A)
/* @} */

/** \name XPLAIN Extension header pin definitions
 *  @{
 */
#define XPLAIN_PIN_3                   IOPORT_CREATE_PIN(PIOA, 18)
#define XPLAIN_PIN_4                   IOPORT_CREATE_PIN(PIOA, 19)
#define XPLAIN_PIN_5                   IOPORT_CREATE_PIN(PIOB, 10)
#define XPLAIN_PIN_6                   IOPORT_CREATE_PIN(PIOB, 11)
#define XPLAIN_PIN_7                   IOPORT_CREATE_PIN(PIOA, 0)
#define XPLAIN_PIN_8                   IOPORT_CREATE_PIN(PIOA, 1)
#define XPLAIN_PIN_9                   IOPORT_CREATE_PIN(PIOA, 2)
#define XPLAIN_PIN_10                  IOPORT_CREATE_PIN(PIOB, 14)
#define XPLAIN_PIN_11                  IOPORT_CREATE_PIN(PIOA, 3)
#define XPLAIN_PIN_12                  IOPORT_CREATE_PIN(PIOA, 4)
#define XPLAIN_PIN_13                  IOPORT_CREATE_PIN(PIOB, 1)
#define XPLAIN_PIN_14                  IOPORT_CREATE_PIN(PIOB, 0)
#define XPLAIN_PIN_15                  IOPORT_CREATE_PIN(PIOA, 16)
#define XPLAIN_PIN_16                  IOPORT_CREATE_PIN(PIOA, 6)
#define XPLAIN_PIN_17                  IOPORT_CREATE_PIN(PIOA, 5)
#define XPLAIN_PIN_18                  IOPORT_CREATE_PIN(PIOA, 15)
/** @} */

/** \name XPLAIN Extension header pin definitions by function
 *  @{
 */
#define XPLAIN_PIN_ADC_1               XPLAIN_PIN_3
#define XPLAIN_PIN_ADC_2               XPLAIN_PIN_4    /* USER LED 1 used by default */
#define XPLAIN_PIN_GPIO_1              XPLAIN_PIN_5    /* Could be used as Xplain Console UART */
#define XPLAIN_PIN_GPIO_2              XPLAIN_PIN_6    /* Could be used as Xplain Console UART */
#define XPLAIN_PIN_CONSOLE_UART_RX     XPLAIN_PIN_5    /* Could be used as Xplain GPIO 1 */
#define XPLAIN_PIN_CONSOLE_UART_TX     XPLAIN_PIN_6    /* Could be used as Xplain GPIO 2 */
#define XPLAIN_PIN_PWM_0               XPLAIN_PIN_7
#define XPLAIN_PIN_PWM_1               XPLAIN_PIN_8
#define XPLAIN_PIN_IRQ                 XPLAIN_PIN_9
#define XPLAIN_PIN_GPIO                XPLAIN_PIN_10   /* PIN USB VBUS used by default */
#define XPLAIN_PIN_I2C_SDA             XPLAIN_PIN_11
#define XPLAIN_PIN_I2C_SCL             XPLAIN_PIN_12
#define XPLAIN_PIN_UART_RX             XPLAIN_PIN_13
#define XPLAIN_PIN_UART_TX             XPLAIN_PIN_14
#define XPLAIN_PIN_SPI_SS              XPLAIN_PIN_15
#define XPLAIN_PIN_SPI_MOSI            XPLAIN_PIN_16
#define XPLAIN_PIN_SPI_MISO            XPLAIN_PIN_17
#define XPLAIN_PIN_SPI_SCK             XPLAIN_PIN_18
/** @} */

/** \name XPLAIN Extension header ADC definitions
 *  @{
 */
#define XPLAIN_ADC_MODULE              ADC
#define XPLAIN_ADC_1_CHANNEL           1
#define XPLAIN_ADC_2_CHANNEL           2
/** @} */

/** \name XPLAIN Extension header PWM definitions
 *  @{
 */
#define XPLAIN_PWM_MODULE              TC0
#define XPLAIN_PWM_0_CHANNEL           0
#define XPLAIN_PWM_0_MUX               IOPORT_MODE_MUX_B
#define XPLAIN_PWM_1_CHANNEL           0
#define XPLAIN_PWM_1_MUX               IOPORT_MODE_MUX_B
/** @} */

/** \name XPLAIN Extension header IRQ/External interrupt definitions
 *  @{
 */
#define XPLAIN_IRQ_MODULE              SUPC
#define XPLAIN_IRQ_INPUT               2
/** @} */

/** \name XPLAIN Extension header I2C definitions
 *  @{
 */
#define XPLAIN_TWI_MODULE              TWI3
#define XPLAIN_TWI_TWD_MUX             IOPORT_MODE_MUX_A
#define XPLAIN_TWI_TWCK_MUX            IOPORT_MODE_MUX_A

#define BOARD_FLEXCOM_XPLAIN_TWI       FLEXCOM3
/** @} */

/** \name XPLAIN Extension header UART definitions
 *  @{
 */
#define XPLAIN_UART_MODULE             USART6
#define XPLAIN_UART_MODULE_ID          ID_FLEXCOM6
#define XPLAIN_UART_RXD_MUX            IOPORT_MODE_MUX_B
#define XPLAIN_UART_TXD_MUX            IOPORT_MODE_MUX_B

#define BOARD_FLEXCOM_XPLAIN_UART      FLEXCOM6
/** @} */

/** \name XPLAIN Extension header CONSOLE UART definitions
 *  @{
 */
#define XPLAIN_CONSOLE_UART_MODULE     USART4
#define XPLAIN_CONSOLE_UART_MODULE_ID  ID_FLEXCOM4
#define XPLAIN_CONSOLE_UART_RXD_MUX    IOPORT_MODE_MUX_A
#define XPLAIN_CONSOLE_UART_TXD_MUX    IOPORT_MODE_MUX_A

#define BOARD_FLEXCOM_CONSOLE_UART     FLEXCOM4
/** @} */

/** \name XPLAIN Extension header SPI definitions
 *  @{
 */
#define XPLAIN_SPI_MODULE              SPI2
#define XPLAIN_SPI_MISO_MUX            IOPORT_MODE_MUX_A
#define XPLAIN_SPI_MOSI_MUX            IOPORT_MODE_MUX_A
#define XPLAIN_SPI_SPCK_MUX            IOPORT_MODE_MUX_B
#define XPLAIN_SPI_NPCS0_MUX           IOPORT_MODE_MUX_A

#define BOARD_FLEXCOM_XPLAIN_SPI       FLEXCOM2
/** @} */

/** \name mikroBUS Extension header pin definitions
 *  @{
 */
#define MIKROBUS_PIN_1                 IOPORT_CREATE_PIN(PIOA, 17)
#define MIKROBUS_PIN_2                 IOPORT_CREATE_PIN(PIOA, 10)
#define MIKROBUS_PIN_3                 IOPORT_CREATE_PIN(PIOA, 11)
#define MIKROBUS_PIN_4                 IOPORT_CREATE_PIN(PIOA, 14)
#define MIKROBUS_PIN_5                 IOPORT_CREATE_PIN(PIOA, 12)
#define MIKROBUS_PIN_6                 IOPORT_CREATE_PIN(PIOA, 13)
#define MIKROBUS_PIN_11                IOPORT_CREATE_PIN(PIOB, 3)
#define MIKROBUS_PIN_12                IOPORT_CREATE_PIN(PIOB, 2)
#define MIKROBUS_PIN_13                IOPORT_CREATE_PIN(PIOB, 8)
#define MIKROBUS_PIN_14                IOPORT_CREATE_PIN(PIOB, 9)
#define MIKROBUS_PIN_15                IOPORT_CREATE_PIN(PIOA, 9)
#define MIKROBUS_PIN_16                IOPORT_CREATE_PIN(PIOA, 23)
/** @} */

/** \name mikroBUS Extension header pin definitions by function
 *  @{
 */
#define MIKROBUS_PIN_AN                MIKROBUS_PIN_1
#define MIKROBUS_PIN_RST               MIKROBUS_PIN_2
#define MIKROBUS_PIN_SPI_SS            MIKROBUS_PIN_3
#define MIKROBUS_PIN_SPI_SCK           MIKROBUS_PIN_4
#define MIKROBUS_PIN_SPI_MISO          MIKROBUS_PIN_5
#define MIKROBUS_PIN_SPI_MOSI          MIKROBUS_PIN_6
#define MIKROBUS_PIN_SDA               MIKROBUS_PIN_11
#define MIKROBUS_PIN_SCL               MIKROBUS_PIN_12
#define MIKROBUS_PIN_UART_TX           MIKROBUS_PIN_13
#define MIKROBUS_PIN_UART_RX           MIKROBUS_PIN_14
#define MIKROBUS_PIN_INT               MIKROBUS_PIN_15
#define MIKROBUS_PIN_PWM               MIKROBUS_PIN_16
/** @} */

/** \name mikroBUS Extension header ADC definitions
 *  @{
 */
#define MIKROBUS_ADC_MODULE            ADC
#define MIKROBUS_ADC_0_CHANNEL         0
/** @} */

/** \name mikroBUS Extension header PWM definitions
 *  @{
 */
#define MIKROBUS_PWM_MODULE            TC0
#define MIKROBUS_PWM_1_CHANNEL         1
#define MIKROBUS_PWM_1_MUX             IOPORT_MODE_MUX_B
/** @} */

/** \name mikroBUS Extension header IRQ/External interrupt definitions
 *  @{
 */
#define MIKROBUS_IRQ_MODULE            SUPC
#define MIKROBUS_IRQ_INPUT             6
/** @} */

/** \name mikroBUS Extension header I2C definitions
 *  @{
 */
#define MIKROBUS_TWI_MODULE            TWI1
#define MIKROBUS_TWI_TWD_MUX           IOPORT_MODE_MUX_A
#define MIKROBUS_TWI_TWCK_MUX          IOPORT_MODE_MUX_A

#define BOARD_FLEXCOM_MIKROBUS_TWI     FLEXCOM1
/** @} */

/** \name mikroBUS Extension header UART definitions
 *  @{
 */
#define MIKROBUS_UART_MODULE           USART4
#define MIKROBUS_UART_MODULE_ID        ID_FLEXCOM4
#define MIKROBUS_UART_RXD_MUX          IOPORT_MODE_MUX_A
#define MIKROBUS_UART_TXD_MUX          IOPORT_MODE_MUX_A

#define BOARD_FLEXCOM_MIKROBUS_UART    FLEXCOM4
/** @} */

/** \name mikroBUS Extension header SPI definitions
 *  @{
 */
#define MIKROBUS_SPI_MODULE            SPI5
#define MIKROBUS_SPI_MISO_MUX          IOPORT_MODE_MUX_A
#define MIKROBUS_SPI_MOSI_MUX          IOPORT_MODE_MUX_A
#define MIKROBUS_SPI_SPCK_MUX          IOPORT_MODE_MUX_A
#define MIKROBUS_SPI_NPCS0_MUX         IOPORT_MODE_MUX_A

#define BOARD_FLEXCOM_MIKROBUS_SPI     FLEXCOM5
/** @} */

/** \name USB definitions
 * @{
 */
#define PIN_USB_VBUS                   {PIO_PB14, PIOB, ID_PIOB, PIO_INPUT, PIO_PULLUP}
#define USB_VBUS_FLAGS                 (PIO_INPUT | PIO_DEBOUNCE | PIO_IT_EDGE)
#define USB_VBUS_PIN_IRQn              (PIOB_IRQn)
#define USB_VBUS_PIN                   (PIO_PB14_IDX)
#define USB_VBUS_PIO_ID                (ID_PIOB)
#define USB_VBUS_PIO_MASK              (PIO_PB14)

/** USB D- pin (System function) */
#define PIN_USB_DM                     {PIO_PA21}
/** USB D+ pin (System function) */
#define PIN_USB_DP                     {PIO_PA22}
/** @} */

/**
 * \name Voltage Monitor definitions
 * @{
 */
#define VOL_MON_GPIO                   (PIO_PA20_IDX)
#define VOL_MON_MODULE                 ADC
#define VOL_MON_CHANNEL                3
/* @} */

/** @} */

#endif  /* PL485_EK_H_INCLUDED */
