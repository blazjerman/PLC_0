/**
 * \file
 *
 * \brief PIC32CXMTSH_DB board init.
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

#include "compiler.h"

#include "board.h"
#include "conf_board.h"
#include "conf_clock.h"
#include "asf.h"

/**
 * \addtogroup pic32cxmtsh_db_group
 * @{
 */

/**
 * \brief Set peripheral mode for IOPORT pins.
 * It will configure port mode and disable pin mode (but enable peripheral).
 * \param port IOPORT port to configure
 * \param masks IOPORT pin masks to configure
 * \param mode Mode masks to configure for the specified pin (\ref ioport_modes)
 */
#define ioport_set_port_peripheral_mode(port, masks, mode) \
	do { \
		ioport_set_port_mode(port, masks, mode); \
		ioport_disable_port(port, masks); \
	} \
	while (0)

/**
 * \brief Set peripheral mode for one single IOPORT pin.
 * It will configure port mode and disable pin mode (but enable peripheral).
 * \param pin IOPORT pin to configure
 * \param mode Mode masks to configure for the specified pin (\ref ioport_modes)
 */
#define ioport_set_pin_peripheral_mode(pin, mode) \
	do { \
		ioport_set_pin_mode(pin, mode);	\
		ioport_disable_pin(pin); \
	} \
	while (0)

/**
 * \brief Set input mode for one single IOPORT pin.
 * It will configure port mode and disable pin mode (but enable peripheral).
 * \param pin IOPORT pin to configure
 * \param mode Mode masks to configure for the specified pin (\ref ioport_modes)
 * \param sense Sense for interrupt detection (\ref ioport_sense)
 */
#define ioport_set_pin_input_mode(pin, mode, sense) \
	do { \
		ioport_set_pin_dir(pin, IOPORT_DIR_INPUT); \
		ioport_set_pin_mode(pin, mode);	\
		ioport_set_pin_sense_mode(pin, sense); \
	} \
	while (0)

#ifdef CONF_BOARD_TCM_INIT
int __tcm_initialization(void);

#pragma section = "code_TCM"
#pragma section = "data_TCM"
#pragma section = "DTCMInRom"
#pragma section = "ITCMInRom"
volatile uint32_t srcData, dstData, sizeData;
volatile uint32_t srcCode, dstCode, sizeCode;
/**
 * \brief TCM manual initialization. This function replaces to the empty function of startup file.
 */
int __tcm_initialization(void)
{
	srcData = (uint32_t)__section_begin("DTCMInRom");
	dstData = (uint32_t)__section_begin("data_TCM");
	sizeData = (uint32_t)__section_end("data_TCM") - dstData;
	srcCode = (uint32_t)__section_begin("ITCMInRom");
	dstCode = (uint32_t)__section_begin("code_TCM");
	sizeCode = (uint32_t)__section_end("code_TCM") - dstCode;

	__asm volatile (
		/* Init DTCM region */
		"LDR R0, =sizeData\n"
		"LDR R0, [R0]\n"
		"LDR R1, =dstData\n"
		"LDR R1, [R1]\n"
		"LDR R2, =srcData\n"
		"LDR R2, [R2]\n"
		"COPY_DATA_LOOP:\n"
		"CMP R0, #0\n"
		"BEQ COPY_DATA_END\n"
		"LDR R3, [R2]\n"
		"STR R3, [R1]\n"
		"ADD R1, R1, #4\n"
		"ADD R2, R2, #4\n"
		"SUB R0, R0, #1\n"
		"B COPY_DATA_LOOP\n"
		"COPY_DATA_END:\n"
		/* Init ITCM region */
		"LDR R0, =sizeCode\n"
		"LDR R0, [R0]\n"
		"LDR R1, =dstCode\n"
		"LDR R1, [R1]\n"
		"LDR R2, =srcCode\n"
		"LDR R2, [R2]\n"
		"COPY_CODE_LOOP:\n"
		"CMP R0, #0\n"
		"BEQ COPY_CODE_END\n"
		"LDR R3, [R2]\n"
		"STR R3, [R1]\n"
		"ADD R1, R1, #4\n"
		"ADD R2, R2, #4\n"
		"SUB R0, R0, #1\n"
		"B COPY_CODE_LOOP\n"
		"COPY_CODE_END:\n"
	);

	return 0;
}
#endif

void board_init(void)
{
#ifndef CONF_BOARD_KEEP_WATCHDOG_AT_INIT
	/* Disable the watchdog */
	DWDT->WDT0_MR |= WDT0_MR_WDDIS;
#endif
	DWDT->WDT1_MR |= WDT1_MR_WDDIS;

	/* GPIO has been deprecated, the old code just keeps it for compatibility.
	 * In new designs IOPORT is used instead.
	 * Here IOPORT must be initialized for others to use before setting up IO.
	 */
	ioport_init();

#ifdef CONF_VISIBLE_LED

	/* Configure the pins connected to LEDs as output and set their
	 * default initial state to high (LEDs off).
	 */
	ioport_set_pin_dir(LED0_GPIO, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(LED0_GPIO, LED0_INACTIVE_LEVEL);
#endif

	/* Configure Push Button pins */
	ioport_set_pin_input_mode(GPIO_PUSH_BUTTON_1, GPIO_PUSH_BUTTON_1_FLAGS,
			GPIO_PUSH_BUTTON_1_SENSE);
	ioport_set_pin_input_mode(GPIO_PUSH_BUTTON_2, GPIO_PUSH_BUTTON_2_FLAGS,
			GPIO_PUSH_BUTTON_2_SENSE);

	/* Configure USART Console pins */
#ifdef CONF_BOARD_UART_CONSOLE
	ioport_set_port_peripheral_mode(PINS_CONSOLE_UART_PORT, PINS_CONSOLE_UART, PINS_CONSOLE_UART_FLAGS);
	/* Enable the peripheral and set USART mode. */
	flexcom_enable(FLEXCOM0);
	flexcom_set_opmode(FLEXCOM0, FLEXCOM_USART);
#endif

	/* Configure UART Opto-port pins */
#ifdef CONF_BOARD_UART_OPTO
	ioport_set_port_peripheral_mode(PINS_OPTO_UART_PORT, PINS_OPTO_UART, PINS_OPTO_UART_FLAGS);
#endif

#ifdef CONF_BOARD_QSPI
	ioport_set_pin_peripheral_mode(QSPI_QSCK_GPIO, QSPI_QSCK_FLAGS);
	ioport_set_pin_peripheral_mode(QSPI_QCS_GPIO, QSPI_QCS_FLAGS);
	ioport_set_pin_peripheral_mode(QSPI_QIO0_GPIO, QSPI_QIO0_FLAGS);
	ioport_set_pin_peripheral_mode(QSPI_QIO1_GPIO, QSPI_QIO1_FLAGS);
	ioport_set_pin_peripheral_mode(QSPI_QIO2_GPIO, QSPI_QIO2_FLAGS);
	ioport_set_pin_peripheral_mode(QSPI_QIO3_GPIO, QSPI_QIO3_FLAGS);
#endif

	/* Configure Xplain Pro USART pins */
#ifdef CONF_BOARD_XPLAIN_USART
	ioport_set_port_peripheral_mode(PINS_XPLAIN_USART_PORT, PINS_XPLAIN_USART, PINS_XPLAIN_USART_FLAGS);
	/* Enable the peripheral and set USART mode. */
	flexcom_enable(FLEXCOM2);
	flexcom_set_opmode(FLEXCOM2, FLEXCOM_USART);
#endif
	/* Configure Xplain Pro TWI pins */
#ifdef CONF_BOARD_XPLAIN_TWI
	ioport_set_port_peripheral_mode(PINS_XPLAIN_TWI_PORT, PINS_XPLAIN_TWI, PINS_XPLAIN_TWI_FLAGS);
	/* Enable the peripheral and set TWI mode. */
	flexcom_enable(FLEXCOM3);
	flexcom_set_opmode(FLEXCOM3, FLEXCOM_TWI);
#endif
	/* Configure Xplain Pro SPI pins */
#ifdef CONF_BOARD_XPLAIN_SPI
	ioport_set_pin_peripheral_mode(XPLAIN_SPI_MISO_GPIO, XPLAIN_SPI_MISO_FLAGS);
	ioport_set_pin_peripheral_mode(XPLAIN_SPI_MOSI_GPIO, XPLAIN_SPI_MOSI_FLAGS);
	ioport_set_pin_peripheral_mode(XPLAIN_SPI_SPCK_GPIO, XPLAIN_SPI_SPCK_FLAGS);
	ioport_set_pin_peripheral_mode(XPLAIN_SPI_NPCS0_GPIO, XPLAIN_SPI_NPCS0_FLAGS);

	/* Enable the peripheral and set SPI mode. */
	flexcom_enable(FLEXCOM1);
	flexcom_set_opmode(FLEXCOM1, FLEXCOM_SPI);
#endif

	/* Configure MikroBus PWM pins */
#ifdef CONF_BOARD_MIKROBUS_PWM
	/* Configure PWM pin */
	ioport_set_pin_peripheral_mode(PIN_MIKROBUS_PWM_GPIO, PIN_MIKROBUS_PWM_FLAGS);
#endif

	/* Configure MikroBus USART pins */
#ifdef CONF_BOARD_MIKROBUS_USART
	ioport_set_port_peripheral_mode(PINS_MIKROBUS_USART_PORT, PINS_MIKROBUS_USART, PINS_MIKROBUS_USART_FLAGS);
	/* Enable the peripheral and set USART mode. */
	flexcom_enable(FLEXCOM6);
	flexcom_set_opmode(FLEXCOM6, FLEXCOM_USART);
#endif
	/* Configure MikroBus TWI pins */
#ifdef CONF_BOARD_MIKROBUS_TWI
	ioport_set_port_peripheral_mode(PINS_MIKROBUS_TWI_PORT, PINS_MIKROBUS_TWI, PINS_MIKROBUS_TWI_FLAGS);
	/* Enable the peripheral and set TWI mode. */
	flexcom_enable(FLEXCOM7);
	flexcom_set_opmode(FLEXCOM7, FLEXCOM_TWI);
#endif
	/* Configure MikroBus SPI pins */
#ifdef CONF_BOARD_MIKROBUS_SPI
	ioport_set_pin_peripheral_mode(MIKROBUS_SPI_MISO_GPIO, MIKROBUS_SPI_MISO_FLAGS);
	ioport_set_pin_peripheral_mode(MIKROBUS_SPI_MOSI_GPIO, MIKROBUS_SPI_MOSI_FLAGS);
	ioport_set_pin_peripheral_mode(MIKROBUS_SPI_SPCK_GPIO, MIKROBUS_SPI_SPCK_FLAGS);
	ioport_set_pin_peripheral_mode(MIKROBUS_SPI_NPCS0_GPIO, MIKROBUS_SPI_NPCS0_FLAGS);

	/* Enable the peripheral and set SPI mode. */
	flexcom_enable(FLEXCOM5);
	flexcom_set_opmode(FLEXCOM5, FLEXCOM_SPI);
#endif

	/* Configure PWM LED pins */
#ifdef CONF_BOARD_PWM_LED0
	/* Configure PWM LED0 pin */
	ioport_set_pin_peripheral_mode(PIN_PWM_OUT0_GPIO, PIN_PWM_OUT0_FLAGS);
#endif

#ifdef CONF_BOARD_PWM_LED1
	/* Configure PWM LED1 pin */
	ioport_set_pin_peripheral_mode(PIN_PWM_OUT1_GPIO, PIN_PWM_OUT1_FLAGS);
#endif

#ifdef CONF_BOARD_PWM_LED2
	/* Configure PWM LED2 pin */
	ioport_set_pin_peripheral_mode(PIN_PWM_OUT2_GPIO, PIN_PWM_OUT2_FLAGS);
#endif

#ifdef CONF_BOARD_PCK0
	/* Configure PCK0 pin */
	ioport_set_pin_peripheral_mode(PIN_PCK0, PIN_PCK0_MUX);
#endif

#ifdef CONF_BOARD_PCK1
	/* Configure PCK1 pin */
	ioport_set_pin_peripheral_mode(PIN_PCK1, PIN_PCK1_MUX);
#endif

#ifdef CONF_BOARD_PCK2
	/* Configure PCK2 pin */
	ioport_set_pin_peripheral_mode(PIN_PCK2, PIN_PCK2_MUX);
#endif
}

/* @} */
