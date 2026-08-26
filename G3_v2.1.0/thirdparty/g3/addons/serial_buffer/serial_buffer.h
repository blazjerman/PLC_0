/* \file
 *
 * \brief Example for a Serial Interface header file
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

/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */

#ifndef __SERIAL_IF_TYPES_H__
#define __SERIAL_IF_TYPES_H__

#include <stdio.h>
#include "conf_board.h"

/* Activate serial debug */
/* #define SERIAL_DEBUG_CONSOLE */

#define MSG_BUF_SIZE            20

#if defined(BUART_METER_MODEM)
	#include "buart_if.h"
	#include "conf_buart_if.h"
	#define SERIAL_BUF_SIZE RX_UART_BUF0_SIZE
#elif defined(BUSART_METER_MODEM)
	#include "busart_if.h"
	#include "conf_busart_if.h"
	#define SERIAL_BUF_SIZE RX_BUSART0_SIZE
#else
	#error "To use serial buffer, either BUART_METER_MODEM or BUSART_METER_MODEM must be defined; check conf_board.h."
#endif

/* *** Structures ************************************************************ */
struct serial_msg {
	uint8_t todo;            /* Flag to indicate that msg has to be served */
	uint8_t buf[SERIAL_BUF_SIZE];    /* Serial data buffer */
	uint16_t length;        /* Length of the data buffer */
};

struct buff_msg {
	struct serial_msg msg[MSG_BUF_SIZE];    /* Round buffer */
	uint8_t ptr_wr; /* Input Buffer Index */
	uint8_t ptr_rd; /* Output Buffer Index */
};

/* extern struct serial_msg rx_serial, tx_serial; */

/* *** Functions prototypes ************************************************** */
void serial_if_init(void);              /* serial_if Initialization */
/* serial_if Processs */
void serial_if_Process(struct buff_msg *rx_serial, struct buff_msg *tx_serial, uint16_t *ptr_HDLC_iframe_tout);
uint8_t serial_if_RxProcess(struct serial_msg *ptr_rx_serial);
uint8_t serial_if_TxProcess(struct serial_msg *ptr_tx_serial);

#endif
