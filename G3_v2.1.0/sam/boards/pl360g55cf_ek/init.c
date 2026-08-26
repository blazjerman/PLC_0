/**
 * \file
 *
 * \brief PL360G55CF_EK board init.
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

#include "compiler.h"
#include "board.h"
#include "conf_board.h"
#include "conf_clock.h"
#include "asf.h"

/**
 * \addtogroup pl360g55cf_ek_group
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

void board_init(void)
{
#ifndef CONF_BOARD_KEEP_WATCHDOG_AT_INIT
	/* Disable the watchdog */
	WDT->WDT_MR = WDT_MR_WDDIS;
#endif

	/* GPIO has been deprecated, the old code just keeps it for compatibility.
	 * In new designs IOPORT is used instead.
	 * Here IOPORT must be initialized for others to use before setting up IO.
	 */
	ioport_init();

	/* Configure the pins connected to LEDs as output and set their
	 * default initial state to high (LEDs off).
	 */
	ioport_set_pin_dir(LED0_GPIO, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(LED0_GPIO, LED0_INACTIVE_LEVEL);
	ioport_set_pin_dir(LED1_GPIO, IOPORT_DIR_OUTPUT);
	ioport_set_pin_level(LED1_GPIO, LED0_INACTIVE_LEVEL);

	/* Configure ATPL360 SPI pins */
	ioport_set_pin_peripheral_mode(SPI3_MISO_GPIO, SPI3_MISO_FLAGS);
	ioport_set_pin_peripheral_mode(SPI3_MOSI_GPIO, SPI3_MOSI_FLAGS);
	ioport_set_pin_peripheral_mode(SPI3_SPCK_GPIO, SPI3_SPCK_FLAGS);
	ioport_set_pin_peripheral_mode(SPI3_NPCS0_GPIO, SPI3_NPCS0_FLAGS);
	/* Enable FLEXCOM */
	flexcom_enable(BOARD_FLEXCOM_SPI3);
	flexcom_set_opmode(BOARD_FLEXCOM_SPI3, FLEXCOM_SPI);

#ifdef CONF_BOARD_SPI_MIKROBUS
	/* Configure SPI pins */
	ioport_set_pin_peripheral_mode(SPI0_MISO_GPIO, SPI0_MISO_FLAGS);
	ioport_set_pin_peripheral_mode(SPI0_MOSI_GPIO, SPI0_MOSI_FLAGS);
	ioport_set_pin_peripheral_mode(SPI0_SPCK_GPIO, SPI0_SPCK_FLAGS);
	ioport_set_pin_peripheral_mode(SPI0_NPCS0_GPIO, SPI0_NPCS0_FLAGS);
	/* Enable FLEXCOM */
	flexcom_enable(BOARD_FLEXCOM_SPI0);
	flexcom_set_opmode(BOARD_FLEXCOM_SPI0, FLEXCOM_SPI);
#endif

#if defined(CONF_BOARD_UART_MIKROBUS) || defined(CONF_BOARD_UART_CONSOLE)
	/* Configure USART 4 */
	ioport_set_port_peripheral_mode(PINS_USART4_PORT, PINS_USART4, PINS_USART4_FLAGS);
	/* Enable FLEXCOM */
	flexcom_enable(BOARD_FLEXCOM_USART4);
	flexcom_set_opmode(BOARD_FLEXCOM_USART4, FLEXCOM_USART);
#endif

#ifdef CONF_BOARD_TWI1_MIKROBUS
	ioport_set_pin_peripheral_mode(TWI1_DATA_GPIO, TWI1_DATA_FLAG);
	ioport_set_pin_peripheral_mode(TWI1_CLK_GPIO, TWI1_CLK_FLAG);
	/* Enable FLEXCOM */
	flexcom_enable(BOARD_FLEXCOM_TWI1);
	flexcom_set_opmode(BOARD_FLEXCOM_TWI1, FLEXCOM_TWI);
#endif

#ifdef CONF_BOARD_USB_PORT
	gpio_configure_pin(USB_VBUS_PIN, USB_VBUS_FLAGS);
#endif
}

/* @} */
