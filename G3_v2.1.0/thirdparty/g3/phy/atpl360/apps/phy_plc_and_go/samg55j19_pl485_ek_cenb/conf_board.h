/**
 * \file
 *
 * \brief PL485 EK board configuration
 *
 * Copyright (C) 2019 Atmel Corporation. All rights reserved.
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
 */

/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#ifndef CONF_BOARD_H_INCLUDED
#define CONF_BOARD_H_INCLUDED

/* Keep watchdog at board initialization (not disable it) */
#define CONF_BOARD_KEEP_WATCHDOG_AT_INIT

/* XPLAIN EXT CONNECTOR */
/* #define CONF_BOARD_XPLAIN_ADC_1 */
/* #define CONF_BOARD_XPLAIN_CONSOLE */
/* #define CONF_BOARD_XPLAIN_PWM_0 */
/* #define CONF_BOARD_XPLAIN_PWM_1 */
/* #define CONF_BOARD_XPLAIN_TWI */
/* #define CONF_BOARD_XPLAIN_UART */
/* #define CONF_BOARD_XPLAIN_SPI */

/* MIKRO BUS EXT CONNECTOR */
/* #define CONF_BOARD_MIKROBUS_AN */
/* #define CONF_BOARD_MIKROBUS_SPI */
/* #define CONF_BOARD_MIKROBUS_TWI */
/* #define CONF_BOARD_MIKROBUS_UART */
/* #define CONF_BOARD_MIKROBUS_PWM */

/* I2S */
/* #define CONF_BOARD_I2S0 */

/* VOLTAGE MONITOR */
/* #define CONF_VOLTAGE_MONITOR */

/* USB */
#define CONF_BOARD_USB_PORT
#define CONF_BOARD_USB_VBUS_DETECT

/* Configure UDC CONSOLE */
#define CONF_BOARD_UDC_CONSOLE

#endif /* CONF_BOARD_H_INCLUDED */
