/**
 * \file
 *
 * \brief PL360G55CB_EK board configuration.
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

#ifndef CONF_BOARD_H_INCLUDED
#define CONF_BOARD_H_INCLUDED

#include "conf_project.h"

/* Keep watchdog at board initialization (not disable it) */
#define CONF_BOARD_KEEP_WATCHDOG_AT_INIT

/* Configure console, selection from conf_project.h */
#ifdef CONSOLE_ON_MIKROBUS_USART
#  define CONF_BOARD_UART_CONSOLE
#else
#  ifdef CONSOLE_ON_USB
#    define CONF_BOARD_UDC_CONSOLE
#  endif
#endif

/* Set UART/USART to use as console */
/* Only valid when CONF_BOARD_UART_CONSOLE is defined */
#define CONSOLE_UART      USART4
#define CONSOLE_UART_ID   ID_FLEXCOM4

/* Configure MIKROBUS_UART */
#ifdef USI_ON_MIKROBUS_USART
#  define CONF_BOARD_UART_MIKROBUS
#endif

/* Configure MIKROBUS_SPI (connected to ATREB215-XPRO SPI) */
#define CONF_BOARD_SPI_MIKROBUS

/* RF215 Reset pin definition */
#define RF215_RESET_GPIO               PIO_PA23_IDX
#define RF215_RESET_ACTIVE_LEVEL       IOPORT_PIN_LEVEL_LOW
#define RF215_RESET_INACTIVE_LEVEL     IOPORT_PIN_LEVEL_HIGH

/* RF215 interrupt pin definition */
#define RF215_INT_GPIO                 PIO_PA14_IDX
#define RF215_INT_ATTR                 (PIO_DEGLITCH | PIO_IT_HIGH_LEVEL)

/* ATREB215-XPRO board LEDs */
#define RF215_LED1                     PIO_PB3_IDX
#define RF215_LED2                     PIO_PB2_IDX

#endif /* CONF_BOARD_H_INCLUDED */
