/**
 * \file
 *
 * \brief ATPL230ABN Board Definition.
 *
 * Copyright (c) 2015 Atmel Corporation. All rights reserved.
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

#ifndef PL360BN_H_INCLUDED
#define PL360BN_H_INCLUDED

#include "compiler.h"
#include "conf_board.h"
#include "system_same70.h"

/**
 * \ingroup group_common_boards
 * \defgroup atpl230ABN_group "ATPL230ABN"
 * Evaluation Board for atpl230a devices.
 *
 * @{
 */

/**
 * \defgroup atpl230ABN_board_info_group "ATPL230ABN - Board informations"
 * Definitions related to the board description.
 *
 * @{
 */

/** Name of the board */
#define BOARD_NAME "PL360BN"
/** Board definition */
#define atpl230abn_v2
/** Family definition (already defined) */
#define same70
/** Core definition */
#define cortexm7
/** Board revision definition */
#define BOARD_REV_1     1

#ifndef BOARD_REV
#define BOARD_REV BOARD_REV_1
#endif

/* @} */

/**
 *  \defgroup atpl230abn_opfreq_group "ATPL230ABN - Operating frequencies"
 *  Definitions related to the board operating frequency.
 *
 *  @{
 */
/** Board oscillator settings moved to conf_board.h in application examples */
/*
	#define BOARD_FREQ_SLCK_XTAL            (32768U)
	#define BOARD_FREQ_SLCK_BYPASS          (32768U)
	#define BOARD_FREQ_MAINCK_XTAL          (12000000U)
	#define BOARD_FREQ_MAINCK_BYPASS        (12000000U)
*/
/* @} */

/** Master clock frequency */
#define BOARD_MCK                   CHIP_FREQ_CPU_MAX

/** board main clock xtal statup time */
#define BOARD_OSC_STARTUP_US        15625U

/* @} */

/**
 * \defgroup atpl230abn_features_group "ATPL230ABN - Features"
 * Symbols that describe features and capabilities of the board.
 *
 * @{
 */

/**
* \name SDRAM pin definition
* @{
*/
#define PIN_SDRAM_D0_7    {0x000000FF, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_SDRAM_D8_13   {0x0000003F, PIOE, ID_PIOE, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_SDRAM_D14_15  {0x00018000, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_SDRAM_A0_9    {0x3FF00000, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_SDRAM_SDA10   {0x00002000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}
#define PIN_SDRAM_A11     {0x80000000, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_DEFAULT}

#define PIN_SDRAM_CAS     {0x00020000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}
#define PIN_SDRAM_RAS     {0x00010000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}
#define PIN_SDRAM_SDCKE   {0x00004000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}
#define PIN_SDRAM_SDCK    {0x00800000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}
#define PIN_SDRAM_SDSC    {0x00008000, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_SDRAM_NBS0    {0x00040000, PIOC, ID_PIOC, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_SDRAM_NBS1    {0x00008000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}
#define PIN_SDRAM_SDWE    {0x20000000, PIOD, ID_PIOD, PIO_PERIPH_C, PIO_DEFAULT}
#define PIN_SDRAM_BA0     {0x00100000, PIOA, ID_PIOA, PIO_PERIPH_C, PIO_DEFAULT}
#define PIN_SDRAM_BA1     {0x00000001, PIOA, ID_PIOA, PIO_PERIPH_C, PIO_DEFAULT}

#define BOARD_SDRAM_PINS PIN_SDRAM_D0_7, PIN_SDRAM_D8_13 , PIN_SDRAM_D14_15,\
						PIN_SDRAM_A0_9, PIN_SDRAM_SDA10, PIN_SDRAM_A11, PIN_SDRAM_BA0, PIN_SDRAM_BA1, \
						PIN_SDRAM_CAS, PIN_SDRAM_RAS, PIN_SDRAM_SDCKE,PIN_SDRAM_SDCK,\
						PIN_SDRAM_SDSC,PIN_SDRAM_NBS0 ,PIN_SDRAM_NBS1,PIN_SDRAM_SDWE
/* @} */

/**
* \name LED #0 pin definition
* @{
*/
#define LED0_GPIO            (PIO_PA21_IDX)
#define LED0_ACTIVE_LEVEL    IOPORT_PIN_LEVEL_LOW
#define LED0_INACTIVE_LEVEL  IOPORT_PIN_LEVEL_HIGH

/* Wrapper macros to ensure common naming across all boards */
#define LED_0_NAME      "green LED (D5)"
#define PIN_LED_0       {PIO_PA21, PIOA, ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_LED_0_MASK   PIO_PA21
#define PIN_LED_0_PIO    PIOA
#define PIN_LED_0_ID     ID_PIOA
#define PIN_LED_0_TYPE   PIO_OUTPUT_1
#define PIN_LED_0_ATTR   PIO_DEFAULT
/* @} */

/**
 * \name LED #1 pin definition (only support BOARD_REV_4)
 * @{
 */
#define LED1_GPIO            (PIO_PA22_IDX)
#define LED1_ACTIVE_LEVEL    IOPORT_PIN_LEVEL_LOW
#define LED1_INACTIVE_LEVEL  IOPORT_PIN_LEVEL_HIGH

/* Wrapper macros to ensure common naming across all boards */
#define LED_1_NAME      "red LED (D6)"
#define PIN_LED_1       {PIO_PA22, PIOA, ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_LED_1_MASK   PIO_PA22
#define PIN_LED_1_PIO    PIOA
#define PIN_LED_1_ID     ID_PIOA
#define PIN_LED_1_TYPE   PIO_OUTPUT_1
#define PIN_LED_1_ATTR   PIO_DEFAULT
/* @} */

/* Number of on-board LEDs */
#define BOARD_NUM_OF_LED 2

/**
 * \name TC -- Timer Counter
 * @{
 */
#define PIN_TC0_TIOA0            (PIO_PA4_IDX)
#define PIN_TC0_TIOA0_MUX        (IOPORT_MODE_MUX_B)
#define PIN_TC0_TIOA0_FLAGS      (IOPORT_MODE_MUX_B)

#define PIN_TC0_TIOA0_PIO        PIOA
#define PIN_TC0_TIOA0_MASK       PIO_PA4
#define PIN_TC0_TIOA0_ID         ID_PIOA
#define PIN_TC0_TIOA0_TYPE       PIO_PERIPH_B
#define PIN_TC0_TIOA0_ATTR       PIO_DEFAULT

#define PIN_TC3_TIOA11	         (PIO_PC25_IDX)
#define PIN_TC3_TIOA11_MUX	 (IOPORT_MODE_MUX_B)
#define PIN_TC3_TIOA11_FLAGS     (IOPORT_MODE_MUX_B)

#define PIN_TC3_TIOA11_PIO	 PIOC
#define PIN_TC3_TIOA11_MASK	 PIO_PC25
#define PIN_TC3_TIOA11_ID	 ID_PIOC
#define PIN_TC3_TIOA11_TYPE	 PIO_PERIPH_B
#define PIN_TC3_TIOA11_ATTR	 PIO_DEFAULT
/* @} */

/**
 * \name Console UART definitions defined in conf_board.h of the application
 * @{
 */
//#define CONSOLE_UART      UART2
//#define CONSOLE_UART_ID   ID_UART2
/* @} */

/**
 * \name UART0 pins (UTXD0 and URXD0) definitions
 * @{
 */

#define PINS_UART0        (PIO_PA9A_URXD0 | PIO_PA10A_UTXD0)
#define PINS_UART0_FLAGS  (IOPORT_MODE_MUX_A)

#define PINS_UART0_PORT   IOPORT_PIOA
#define PINS_UART0_MASK   (PIO_PA9A_URXD0 | PIO_PA10A_UTXD0)
#define PINS_UART0_PIO    PIOA
#define PINS_UART0_ID     ID_PIOA
#define PINS_UART0_TYPE   PIO_PERIPH_A
#define PINS_UART0_ATTR   PIO_DEFAULT

/* @} */

/**
 * \name UART2 pins (UTXD2 and URXD2) definitions PD25,PD26
	 Data Concentrator Expansion Connector
 * @{
 */
#define PINS_UART2        (PIO_PD25C_URXD2 | PIO_PD26C_UTXD2)
#define PINS_UART2_FLAGS  (IOPORT_MODE_MUX_C)

#define PINS_UART2_PORT   IOPORT_PIOD
#define PINS_UART2_MASK   (PIO_PD25C_URXD2 | PIO_PD26C_UTXD2)
#define PINS_UART2_PIO    PIOD
#define PINS_UART2_ID     ID_PIOD
#define PINS_UART2_TYPE   PIO_PERIPH_C
#define PINS_UART2_ATTR   PIO_DEFAULT
/* @} */

/**
 * \name UART4 pins (UTXD4 and URXD4) definitions PD18,PD19
	 PP Expansion Connector
 * @{
 */
#define PINS_UART4        (PIO_PD18C_URXD4 | PIO_PD19C_UTXD4)
#define PINS_UART4_FLAGS  (IOPORT_MODE_MUX_C)

#define PINS_UART4_PORT   IOPORT_PIOD
#define PINS_UART4_MASK   (PIO_PD18C_URXD4 | PIO_PD19C_UTXD4)
#define PINS_UART4_PIO    PIOD
#define PINS_UART4_ID     ID_PIOD
#define PINS_UART4_TYPE   PIO_PERIPH_C
#define PINS_UART4_ATTR   PIO_DEFAULT

/**
 * \name USARTx pin definitions
 * @{
 */
/** USART0 pin RTS */
#define PIN_USART0_RTS        {PIO_PB3C_RTS0, PIOB, ID_PIOB, PIO_PERIPH_C, PIO_DEFAULT}
#define PIN_USART0_RTS_IDX    (PIO_PB3_IDX)
#define PIN_USART0_RTS_FLAGS  (IOPORT_MODE_MUX_C)
/** USART0 pin RX */
#define PIN_USART0_RXD        {PIO_PB0C_RXD0, PIOB, ID_PIOB, PIO_PERIPH_C, PIO_DEFAULT}
#define PIN_USART0_RXD_IDX    (PIO_PB0_IDX)
#define PIN_USART0_RXD_FLAGS  (IOPORT_MODE_MUX_C)
/** USART0 pin SCK */
#define PIN_USART0_SCK        {PIO_PB13C_SCK0, PIOB, ID_PIOB, PIO_PERIPH_C, PIO_DEFAULT}
#define PIN_USART0_SCK_IDX    (PIO_PB13_IDX)
#define PIN_USART0_SCK_FLAGS  (IOPORT_MODE_MUX_C)
/** USART0 pin TX */
#define PIN_USART0_TXD        {PIO_PB1C_TXD0, PIOB, ID_PIOB, PIO_PERIPH_C, PIO_DEFAULT}
#define PIN_USART0_TXD_IDX    (PIO_PB1_IDX)
#define PIN_USART0_TXD_FLAGS  (IOPORT_MODE_MUX_C)


/** USART1 pin RX */
#define PIN_USART1_RXD        {PIO_PD18A_RXD1, PIOD, ID_PIOD, PIO_PERIPH_D, PIO_DEFAULT}
#define PIN_USART1_RXD_IDX    (PIO_PD18_IDX)
#define PIN_USART1_RXD_FLAGS  (IOPORT_MODE_MUX_A)
/** USART1 pin TX */
#define PIN_USART1_TXD        {PIO_PD19A_TXD1, PIOD, ID_PIOD, PIO_PERIPH_D, PIO_DEFAULT}
#define PIN_USART1_TXD_IDX    (PIO_PD19_IDX)
#define PIN_USART1_TXD_FLAGS  (IOPORT_MODE_MUX_A)
/** USART1 pin CTS */
#define PIN_USART1_CTS        {PIO_PA25A_CTS1, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART1_CTS_IDX    (PIO_PA25_IDX)
#define PIN_USART1_CTS_FLAGS  (IOPORT_MODE_MUX_A)
/** USART1 pin RTS */
#define PIN_USART1_RTS        {PIO_PA24A_RTS1, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART1_RTS_IDX    (PIO_PA24_IDX)
#define PIN_USART1_RTS_FLAGS  (IOPORT_MODE_MUX_A)
/** USART1 pin SCK */
#define PIN_USART1_SCK        {PIO_PA23A_SCK1, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_USART1_SCK_IDX    (PIO_PA23_IDX)
#define PIN_USART1_SCK_FLAGS  (IOPORT_MODE_MUX_A)
/** USART1 pin ENABLE */
#define PIN_USART1_EN         {PIO_PA23, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define PIN_USART1_EN_IDX     (PIO_PA23_IDX)
#define PIN_USART1_EN_FLAGS   (0)
#define PIN_USART1_EN_ACTIVE_LEVEL   IOPORT_PIN_LEVEL_LOW
#define PIN_USART1_EN_INACTIVE_LEVEL IOPORT_PIN_LEVEL_HIGH

/* @} */

/**
 * \name SPI pin definitions [ATPL360]
 * @{
 */
/** SPI MISO pin definition. */
#define SPI_MISO_GPIO         (PIO_PD20_IDX)
#define SPI_MISO_FLAGS        (IOPORT_MODE_MUX_B)
/** SPI MOSi pin definition. */
#define SPI_MOSI_GPIO         (PIO_PD21_IDX)
#define SPI_MOSI_FLAGS        (IOPORT_MODE_MUX_B)
/** SPI SPCK pin definition. */
#define SPI_SPCK_GPIO         (PIO_PD22_IDX)
#define SPI_SPCK_FLAGS        (IOPORT_MODE_MUX_B)
/** SPI chip select 0 pin definition. (Only one configuration is possible) */
#define SPI_NPCS2_GPIO        (PIO_PD12_IDX)
#define SPI_NPCS2_FLAGS       (IOPORT_MODE_MUX_C)

#define ATPL360_SPI           SPI0
#define ATPL360_SPI_CS        2
/* @} */

/**
 * \name PL360 GPIO pin definitions
 * @{
 */
#define ATPL360_GPIO0                    (PIO_PA19_IDX)
#define ATPL360_GPIO1                    (PIO_PA18_IDX)
#define ATPL360_GPIO2                    (PIO_PA17_IDX)
#define ATPL360_GPIO3                    (PIO_PD24_IDX)
#define ATPL360_GPIO4                    (PIO_PD27_IDX)
#define ATPL360_GPIO5                    (PIO_PA5_IDX)
/* @} */

/**
 * \name ATPL360 Reset pin definition
 * @{
 */
#define ATPL360_RESET_GPIO               PIO_PA23_IDX
#define ATPL360_RESET_ACTIVE_LEVEL       IOPORT_PIN_LEVEL_LOW
#define ATPL360_RESET_INACTIVE_LEVEL     IOPORT_PIN_LEVEL_HIGH
/* @} */

/**
 * \name ATPL360 LDO Enable pin definition
 * @{
 */
#define ATPL360_LDO_EN_GPIO              PIO_PA24_IDX
#define ATPL360_LDO_EN_ACTIVE_LEVEL      IOPORT_PIN_LEVEL_HIGH
#define ATPL360_LDO_EN_INACTIVE_LEVEL    IOPORT_PIN_LEVEL_LOW
/* @} */

/**
 * \name ATPL360 Carrier Detect Enable pin definition
 * @{
 */
#define ATPL360_CD_GPIO                  ATPL360_GPIO0
/* @} */

/**
 * \name ATPL360 interrupt pin definition
 * @{
 */
#define ATPL360_INT_GPIO                 PIO_PD24_IDX
#define ATPL360_INT_FLAGS                IOPORT_MODE_DEBOUNCE
#define ATPL360_INT_SENSE                IOPORT_SENSE_FALLING

#define ATPL360_INT                      {PIO_PD24, PIOD, ID_PIOD, PIO_INPUT, PIO_DEGLITCH | PIO_IT_LOW_LEVEL}
#define ATPL360_INT_MASK                 PIO_PD24
#define ATPL360_INT_PIO                  PIOD
#define ATPL360_INT_ID                   ID_PIOD
#define ATPL360_INT_TYPE                 PIO_INPUT
#define ATPL360_INT_ATTR                 (PIO_DEGLITCH | PIO_IT_LOW_LEVEL)
#define ATPL360_INT_IRQn                 PIOD_IRQn
/* @} */

/**
 * \name TWIx pin definitions
 * @{
 */
/* ------------------------------------------------------------------------
 * TWI0 pins definition  AT24C02D SSHM-T 2kb eeprom
 * ------------------------------------------------------------------------
 */
#define TWI0_DATA_GPIO   PIO_PA3_IDX
#define TWI0_DATA_FLAGS  (PIO_PERIPH_A | PIO_DEFAULT)
#define TWI0_CLK_GPIO    PIO_PA4_IDX
#define TWI0_CLK_FLAGS   (PIO_PERIPH_A | PIO_DEFAULT)

/*! TWI0 Data pin for EEPROM */
#define BOARD_CLK_TWI_EEPROM      TWI0_CLK_GPIO
#define BOARD_CLK_TWI_MUX_EEPROM  TWI0_CLK_FLAG
/* @} */

/**
 * \name Voltage Monitor pins definition
 * @{
 */
#define VZ_CROSS_GPIO    PIO_PC30_IDX
#define V5V_SENSE_GPIO   PIO_PB2_IDX
#define VDD_SENSE_GPIO   PIO_PD30_IDX
/* @} */

/**
 * \name Xplain PRO pins definition
 * @{
 */
/* @} */

/**
 * \name GPIO EXPANSIONS pins definition
 * @{
 */
#define PIN_GPIO_EXT_PIO0 PIO_PA17_IDX
#define PIN_GPIO_EXT_PIO1 PIO_PA19_IDX
#define PIN_GPIO_EXT_PIO2 PIO_PA18_IDX
#define PIN_GPIO_EXT_PIO3 PIO_PB3_IDX
#define PIN_GPIO_EXT_PIO4 PIO_PD28_IDX
#define PIN_GPIO_EXT_PIO5 PIO_PA5_IDX
#define PIN_GPIO_EXT_PIO6 PIO_PC19_IDX
#define PIN_GPIO_EXT_PIO7 PIO_PC11_IDX
#define PIN_GPIO_EXT_PIO8 PIO_PC8_IDX
#define PIN_GPIO_EXT_PIO9 PIO_PC14_IDX
#define PIN_GPIO_EXT_PIO10 PIO_PD11_IDX
#define PIN_GPIO_EXT_PIO11 PIO_PC13_IDX
#define PIN_GPIO_EXT_PIO12 PIO_PC12_IDX
#define PIN_GPIO_EXT_PIO13 PIO_PC10_IDX
#define PIN_GPIO_EXT_PIO14 PIO_PC9_IDX
#define PIN_GPIO_EXT_PIO15 PIO_PB0_IDX
#define PIN_GPIO_EXT_PIO16 PIO_PD27_IDX
#define PIN_GPIO_EXT_PIO17 PIO_PA2_IDX

/* @} */

/** HSMCI pins definition. */
/*! Number of slot connected on HSMCI interface */
#define SD_MMC_HSMCI_MEM_CNT      1
#define SD_MMC_HSMCI_SLOT_0_SIZE  4
#define PINS_HSMCI   {0x3fUL << 26, PIOA, ID_PIOA, PIO_PERIPH_C, PIO_PULLUP}
/** HSMCI MCCDA pin definition. */
#define PIN_HSMCI_MCCDA_GPIO            (PIO_PA28_IDX)
#define PIN_HSMCI_MCCDA_FLAGS           (IOPORT_MODE_MUX_C)
/** HSMCI MCCK pin definition. */
#define PIN_HSMCI_MCCK_GPIO             (PIO_PA25_IDX)
#define PIN_HSMCI_MCCK_FLAGS            (IOPORT_MODE_MUX_D)
/** HSMCI MCDA0 pin definition. */
#define PIN_HSMCI_MCDA0_GPIO            (PIO_PA30_IDX)
#define PIN_HSMCI_MCDA0_FLAGS           (IOPORT_MODE_MUX_C)
/** HSMCI MCDA1 pin definition. */
#define PIN_HSMCI_MCDA1_GPIO            (PIO_PA31_IDX)
#define PIN_HSMCI_MCDA1_FLAGS           (IOPORT_MODE_MUX_C)
/** HSMCI MCDA2 pin definition. */
#define PIN_HSMCI_MCDA2_GPIO            (PIO_PA26_IDX)
#define PIN_HSMCI_MCDA2_FLAGS           (IOPORT_MODE_MUX_C)
/** HSMCI MCDA3 pin definition. */
#define PIN_HSMCI_MCDA3_GPIO            (PIO_PA27_IDX)
#define PIN_HSMCI_MCDA3_FLAGS           (IOPORT_MODE_MUX_C)

/** SD/MMC card detect pin definition. */
#define PIN_HSMCI_CD                    {PIO_PA29, PIOA, ID_PIOA, PIO_INPUT, PIO_PULLUP}
#define SD_MMC_0_CD_GPIO                (PIO_PA29_IDX)
#define SD_MMC_0_CD_PIO_ID              ID_PIOA
#define SD_MMC_0_CD_FLAGS               (IOPORT_MODE_PULLUP)
#define SD_MMC_0_CD_DETECT_VALUE        0
/**  Board SDRAM size for IS42S16100E-7B */
#define BOARD_SDRAM_SIZE        (8 * 1024 * 1024)

/** Address for transferring command bytes to the SDRAM. */
#define BOARD_SDRAM_ADDR     0x70000000

/**  SDRAM pins definitions */
#define SDRAM_BA0_PIO        PIO_PA20_IDX
#define SDRAM_BA1_PIO        PIO_PA0_IDX
#define SDRAM_SDCK_PIO       PIO_PD23_IDX
#define SDRAM_SDCKE_PIO      PIO_PD14_IDX
#define SDRAM_SDCS_PIO       PIO_PC15_IDX
#define SDRAM_RAS_PIO        PIO_PD16_IDX
#define SDRAM_CAS_PIO        PIO_PD17_IDX
#define SDRAM_SDWE_PIO       PIO_PD29_IDX
#define SDRAM_NBS0_PIO       PIO_PC18_IDX
#define SDRAM_NBS1_PIO       PIO_PD15_IDX
#define SDRAM_A2_PIO         PIO_PC20_IDX
#define SDRAM_A3_PIO         PIO_PC21_IDX
#define SDRAM_A4_PIO         PIO_PC22_IDX
#define SDRAM_A5_PIO         PIO_PC23_IDX
#define SDRAM_A6_PIO         PIO_PC24_IDX
#define SDRAM_A7_PIO         PIO_PC25_IDX
#define SDRAM_A8_PIO         PIO_PC26_IDX
#define SDRAM_A9_PIO         PIO_PC27_IDX
#define SDRAM_A10_PIO        PIO_PC28_IDX
#define SDRAM_A11_PIO        PIO_PC29_IDX
#define SDRAM_SDA10_PIO      PIO_PD13_IDX
#define SDRAM_A13_PIO        PIO_PC31_IDX

#define SDRAM_D0_PIO         PIO_PC0_IDX
#define SDRAM_D1_PIO         PIO_PC1_IDX
#define SDRAM_D2_PIO         PIO_PC2_IDX
#define SDRAM_D3_PIO         PIO_PC3_IDX
#define SDRAM_D4_PIO         PIO_PC4_IDX
#define SDRAM_D5_PIO         PIO_PC5_IDX
#define SDRAM_D6_PIO         PIO_PC6_IDX
#define SDRAM_D7_PIO         PIO_PC7_IDX
#define SDRAM_D8_PIO         PIO_PE0_IDX
#define SDRAM_D9_PIO         PIO_PE1_IDX
#define SDRAM_D10_PIO        PIO_PE2_IDX
#define SDRAM_D11_PIO        PIO_PE3_IDX
#define SDRAM_D12_PIO        PIO_PE4_IDX
#define SDRAM_D13_PIO        PIO_PE5_IDX
#define SDRAM_D14_PIO        PIO_PA15_IDX
#define SDRAM_D15_PIO        PIO_PA16_IDX

#define SDRAM_BA0_FLAGS      PIO_PERIPH_C
#define SDRAM_BA1_FLAGS      PIO_PERIPH_C
#define SDRAM_SDCK_FLAGS     PIO_PERIPH_C
#define SDRAM_SDCKE_FLAGS    PIO_PERIPH_C
#define SDRAM_SDCS_FLAGS     PIO_PERIPH_A
#define SDRAM_RAS_FLAGS      PIO_PERIPH_C
#define SDRAM_CAS_FLAGS      PIO_PERIPH_C
#define SDRAM_SDWE_FLAGS     PIO_PERIPH_C
#define SDRAM_NBS0_FLAGS     PIO_PERIPH_A
#define SDRAM_NBS1_FLAGS     PIO_PERIPH_C
#define SDRAM_A_FLAGS        PIO_PERIPH_A
#define SDRAM_SDA10_FLAGS    PIO_PERIPH_C
#define SDRAM_D_FLAGS        PIO_PERIPH_A

/* GMAC HW configurations */
#define BOARD_GMAC_PHY_ADDR 0

#define PIN_GMAC_RESET_MASK   PIO_PC10
#define PIN_GMAC_RESET_PIO    PIOC
#define PIN_GMAC_INT_MASK     PIO_PA14
#define PIN_GMAC_INT_PIO      PIOA
#define PIN_GMAC_PERIPH       PIO_PERIPH_A
#define PIN_GMAC_PIO          PIOD
#define PIN_GMAC_MASK         (PIO_PD0A_GTXCK | PIO_PD1A_GTXEN | PIO_PD2A_GTX0 | \
							   PIO_PD3A_GTX1 | PIO_PD4A_GRXDV | PIO_PD5A_GRX0 |  \
							   PIO_PD6A_GRX1 | PIO_PD7A_GRXER | PIO_PD8A_GMDC | \
							   PIO_PD9A_GMDIO)

/** USB VBus monitoring pin definition. */
#define PIN_USB_VBUS    {PIO_PE0, PIOE, ID_PIOE, PIO_INPUT, PIO_PULLUP}
#define USB_VBUS_FLAGS    (PIO_INPUT | PIO_DEBOUNCE | PIO_IT_EDGE)
#define USB_VBUS_PIN_IRQn (PIOE_IRQn)
#define USB_VBUS_PIN      (PIO_PE0_IDX)
#define USB_VBUS_PIO_ID   (ID_PIOE)
#define USB_VBUS_PIO_MASK (PIO_PE2)
/* This pin can not be used as fast wakeup source such as
 * USB_VBUS_WKUP PMC_FSMR_FSTT7 */

/** USB D- pin (System function) */
#define PIN_USB_DM      {PIO_PB10}
/** USB D+ pin (System function) */
#define PIN_USB_DP      {PIO_PB11}

/*----------------------------------------------------------------------------*/
/**
 * \page sam4e_SD_CARD "ATPL230ABN - External components"
 * This page lists the definitions related to external on-board components
 * located in the board.h file for the SAM4E-XPRO.
 *
 * SD Card
 * - \ref BOARD_SD_PINS
 * - \ref BOARD_SD_PIN_CD
 *

 */

/** HSMCI pins that shall be configured to access the SD card. */
#define BOARD_SD_PINS               PINS_HSMCI
#define MICRO_SD_DETECT 	    PIO_PD30_IDX

/*----------------------------------------------------------------------------*/
/**
 * \page sam4e_usb "ATPL230ABN - USB device"
 *
 * \section Definitions
 * - \ref BOARD_USB_BMATTRIBUTES
 * - \ref CHIP_USB_UDP
 * - \ref CHIP_USB_PULLUP_INTERNAL
 * - \ref CHIP_USB_NUMENDPOINTS
 * - \ref CHIP_USB_ENDPOINTS_MAXPACKETSIZE
 * - \ref CHIP_USB_ENDPOINTS_BANKS
 */

/**
 * USB attributes configuration descriptor (bus or self powered,
 * remote wakeup)
 */
#define BOARD_USB_BMATTRIBUTES  USBConfigurationDescriptor_SELFPOWERED_RWAKEUP

/** Indicates chip has an UDP Full Speed. */
#define CHIP_USB_UDP

/** Indicates chip has an internal pull-up. */
#define CHIP_USB_PULLUP_INTERNAL

/** Number of USB endpoints */
#define CHIP_USB_NUMENDPOINTS 8

/** Endpoints max packet size */
#define CHIP_USB_ENDPOINTS_MAXPACKETSIZE(i) \
   ((i == 0) ? 64 : \
   ((i == 1) ? 64 : \
   ((i == 2) ? 64 : \
   ((i == 3) ? 64 : \
   ((i == 4) ? 512 : \
   ((i == 5) ? 512 : \
   ((i == 6) ? 64 : \
   ((i == 7) ? 64 : 0 ))))))))

/** Endpoints Number of Bank */
#define CHIP_USB_ENDPOINTS_BANKS(i) \
   ((i == 0) ? 1 : \
   ((i == 1) ? 2 : \
   ((i == 2) ? 2 : \
   ((i == 3) ? 1 : \
   ((i == 4) ? 2 : \
   ((i == 5) ? 2 : \
   ((i == 6) ? 2 : \
   ((i == 7) ? 2 : 0 ))))))))

#define USB_DEVICE_DETECT 	PIO_PE0_IDX

/* @} *//* End of atpl230abn_features_group */

/* @} *//* End of atpl230abn_group */

#endif  /* ATPL230ABN_V2_H_INCLUDED */

