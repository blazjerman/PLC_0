/**
 * \file
 *
 * \brief PL485 EK board initialization
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

#include <compiler.h>
#include <board.h>
#include <conf_board.h>
#include <ioport.h>
#include "asf.h"

/**
 * \addtogroup pl485_ek_group
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

#if defined(__GNUC__)
void board_init(void) WEAK __attribute__((alias("system_board_init")));

#elif defined(__ICCARM__)
void board_init(void);

#  pragma weak board_init=system_board_init
#endif

void system_board_init(void)
{
#ifndef CONF_BOARD_KEEP_WATCHDOG_AT_INIT
	WDT->WDT_MR = WDT_MR_WDDIS;
#endif
	ioport_init();

	/* Configure the pins connected to LEDs as output and set their
	 * default initial state to high (LEDs off).
	 */
	ioport_set_pin_dir(LED0_GPIO, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(LED0_GPIO, LED0_INACTIVE_LEVEL);
	ioport_set_pin_dir(LED1_GPIO, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(LED1_GPIO, LED0_INACTIVE_LEVEL);

	/* Configure ATPL360 connection */
	ioport_set_pin_peripheral_mode(ATPL360_MISO_GPIO, ATPL360_MISO_FLAGS);
	ioport_set_pin_peripheral_mode(ATPL360_MOSI_GPIO, ATPL360_MOSI_FLAGS);
	ioport_set_pin_peripheral_mode(ATPL360_SPCK_GPIO, ATPL360_SPCK_FLAGS);
	ioport_set_pin_peripheral_mode(ATPL360_NPCS0_GPIO, ATPL360_NPCS0_FLAGS);
	ioport_set_pin_dir(ATPL360_STBY_GPIO, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(ATPL360_STBY_GPIO, ATPL360_STBY_INACTIVE_LEVEL);
	ioport_set_pin_dir(ATPL360_RESET_GPIO, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(ATPL360_RESET_GPIO, ATPL360_RESET_ACTIVE_LEVEL);
	ioport_set_pin_dir(ATPL360_LDO_EN_GPIO, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(ATPL360_LDO_EN_GPIO, ATPL360_LDO_EN_INACTIVE_LEVEL);
	ioport_set_pin_dir(ATPL360_CD_GPIO, IOPORT_DIR_INPUT);
	ioport_set_pin_mode(ATPL360_CD_GPIO, IOPORT_MODE_PULLDOWN);
	ioport_set_pin_dir(ATPL360_INT_GPIO, IOPORT_DIR_INPUT);
	ioport_set_pin_mode(ATPL360_INT_GPIO, IOPORT_MODE_PULLDOWN);
	/* Enable FLEXCOM */
	flexcom_enable(BOARD_FLEXCOM_ATPL360);
	flexcom_set_opmode(BOARD_FLEXCOM_ATPL360, FLEXCOM_SPI);

#ifdef CONF_BOARD_XPLAIN_CONSOLE
	ioport_set_pin_peripheral_mode(XPLAIN_PIN_CONSOLE_UART_RX, XPLAIN_CONSOLE_UART_RXD_MUX);
	ioport_set_pin_peripheral_mode(XPLAIN_PIN_CONSOLE_UART_TX, XPLAIN_CONSOLE_UART_TXD_MUX);
#endif

#ifdef CONF_BOARD_XPLAIN_PWM_0
	ioport_set_pin_peripheral_mode(XPLAIN_PIN_PWM_0, XPLAIN_PWM_0_MUX);
#endif

#ifdef CONF_BOARD_XPLAIN_PWM_1
	ioport_set_pin_peripheral_mode(XPLAIN_PIN_PWM_1, XPLAIN_PWM_1_MUX);
#endif

#ifdef CONF_BOARD_XPLAIN_TWI
	ioport_set_pin_peripheral_mode(XPLAIN_PIN_I2C_SDA, XPLAIN_TWI_TWD_MUX);
	ioport_set_pin_peripheral_mode(XPLAIN_PIN_I2C_SCL, XPLAIN_TWI_TWCK_MUX);
	/* Enable FLEXCOM */
	flexcom_enable(BOARD_FLEXCOM_XPLAIN_TWI);
	flexcom_set_opmode(BOARD_FLEXCOM_XPLAIN_TWI, FLEXCOM_TWI);
#endif

#ifdef CONF_BOARD_XPLAIN_UART
	ioport_set_pin_peripheral_mode(XPLAIN_PIN_UART_RX, XPLAIN_UART_RXD_MUX);
	ioport_set_pin_peripheral_mode(XPLAIN_PIN_UART_TX, XPLAIN_UART_TXD_MUX);
	/* Enable FLEXCOM */
	flexcom_enable(BOARD_FLEXCOM_XPLAIN_UART);
	flexcom_set_opmode(BOARD_FLEXCOM_XPLAIN_UART, FLEXCOM_USART);
#endif

#ifdef CONF_BOARD_XPLAIN_SPI
	ioport_set_pin_peripheral_mode(XPLAIN_PIN_SPI_MISO, XPLAIN_SPI_MISO_MUX);
	ioport_set_pin_peripheral_mode(XPLAIN_PIN_SPI_MOSI, XPLAIN_SPI_MOSI_MUX);
	ioport_set_pin_peripheral_mode(XPLAIN_PIN_SPI_SCK, XPLAIN_SPI_SPCK_MUX);
	ioport_set_pin_peripheral_mode(XPLAIN_PIN_SPI_SS, XPLAIN_SPI_NPCS0_MUX);
	/* Enable FLEXCOM */
	flexcom_enable(BOARD_FLEXCOM_XPLAIN_SPI);
	flexcom_set_opmode(BOARD_FLEXCOM_XPLAIN_SPI, FLEXCOM_SPI);
#endif

#ifdef CONF_BOARD_MIKROBUS_SPI
	ioport_set_pin_peripheral_mode(MIKROBUS_PIN_SPI_MISO, MIKROBUS_SPI_MISO_MUX);
	ioport_set_pin_peripheral_mode(MIKROBUS_PIN_SPI_MOSI, MIKROBUS_SPI_MOSI_MUX);
	ioport_set_pin_peripheral_mode(MIKROBUS_PIN_SPI_SCK, MIKROBUS_SPI_SPCK_MUX);
	ioport_set_pin_peripheral_mode(MIKROBUS_PIN_SPI_SS, MIKROBUS_SPI_NPCS0_MUX);
	/* Enable FLEXCOM */
	flexcom_enable(BOARD_FLEXCOM_MIKROBUS_SPI);
	flexcom_set_opmode(BOARD_FLEXCOM_MIKROBUS_SPI, FLEXCOM_SPI);
#endif

#ifdef CONF_BOARD_MIKROBUS_TWI
	ioport_set_pin_peripheral_mode(MIKROBUS_PIN_SDA, MIKROBUS_TWI_TWD_MUX);
	ioport_set_pin_peripheral_mode(MIKROBUS_PIN_SCL, MIKROBUS_TWI_TWCK_MUX);
	/* Enable FLEXCOM */
	flexcom_enable(BOARD_FLEXCOM_MIKROBUS_TWI);
	flexcom_set_opmode(BOARD_FLEXCOM_MIKROBUS_TWI, FLEXCOM_TWI);
#endif

#ifdef CONF_BOARD_MIKROBUS_UART
	ioport_set_pin_peripheral_mode(MIKROBUS_PIN_UART_RX, MIKROBUS_UART_RXD_MUX);
	ioport_set_pin_peripheral_mode(MIKROBUS_PIN_UART_TX, MIKROBUS_UART_TXD_MUX);
	/* Enable FLEXCOM */
	flexcom_enable(BOARD_FLEXCOM_MIKROBUS_UART);
	flexcom_set_opmode(BOARD_FLEXCOM_MIKROBUS_UART, FLEXCOM_USART);
#endif

#ifdef CONF_BOARD_MIKROBUS_PWM
	ioport_set_pin_peripheral_mode(MIKROBUS_PIN_PWM, MIKROBUS_PWM_1_MUX);
#endif

#ifdef CONF_BOARD_I2S0
	ioport_set_pin_peripheral_mode(I2S0_SCK_GPIO, I2S0_SCK_FLAGS);
	ioport_set_pin_peripheral_mode(I2S0_MCK_GPIO, I2S0_MCK_FLAGS);
	ioport_set_pin_peripheral_mode(I2S0_SDI_GPIO, I2S0_SDI_FLAGS);
	ioport_set_pin_peripheral_mode(I2S0_SDO_GPIO, I2S0_SDO_FLAGS);
	ioport_set_pin_peripheral_mode(I2S0_WS_GPIO, I2S0_WS_FLAGS);
#endif

#if defined(CONF_BOARD_USB_PORT)
#  if defined(CONF_BOARD_USB_VBUS_DETECT)
	ioport_set_pin_dir(USB_VBUS_PIN, IOPORT_DIR_INPUT);
#  endif
#endif
}

/** @} */
