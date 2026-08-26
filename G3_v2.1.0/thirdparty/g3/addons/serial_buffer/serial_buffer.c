/**
 * \file
 *
 * \brief Example for a Serial Interface
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

#include "asf.h"
#include "stdio_serial.h"
#include "conf_board.h"
#include "conf_clock.h"
#include "logger.h"
#include "hdlc.h"
#include "serial_buffer.h"

void serial_if_init(void)
{
  #ifdef BUART_METER_MODEM
	buart_if_open(BUART_METER_MODEM, 9600U);
  #else
	busart_if_open(BUSART_METER_MODEM, 9600U);
  #endif
}

/* ************************************************************************** */

/** @brief	Process reception machine
 *
 **************************************************************************/
uint8_t serial_if_RxProcess(struct serial_msg *ptr_rx_serial)
{
	ptr_rx_serial->todo = 0;

      #ifdef BUART_METER_MODEM
	ptr_rx_serial->length = buart_if_read(BUART_METER_MODEM, &ptr_rx_serial->buf, SERIAL_BUF_SIZE);
      #else
	ptr_rx_serial->length = busart_if_read(BUSART_METER_MODEM, &ptr_rx_serial->buf, SERIAL_BUF_SIZE);
      #endif
	if (ptr_rx_serial->length) {
      #ifdef SERIAL_DEBUG_CONSOLE
		LogBuffer(&ptr_rx_serial->buf, ptr_rx_serial->length, "\r\nserial_if_RxProcess: ");
		printf("\r\n");
      #endif
		ptr_rx_serial->todo = 1;
	}

	return ptr_rx_serial->todo;
}

/* ************************************************************************** */

/** @brief	Process transmission machine
 *
 **************************************************************************/
uint8_t serial_if_TxProcess(struct serial_msg *ptr_tx_serial)
{
	uint16_t u16_written;
	if (ptr_tx_serial->todo) {
    #ifdef BUART_METER_MODEM
		u16_written = buart_if_write(BUART_METER_MODEM, &ptr_tx_serial->buf, ptr_tx_serial->length);
    #else
		u16_written = busart_if_write(BUSART_METER_MODEM, &ptr_tx_serial->buf, ptr_tx_serial->length);
    #endif

		if (u16_written < ptr_tx_serial->length) {
			printf("Serial Tx error: sent %hu bytes from %hu.\r\n", u16_written, ptr_tx_serial->length);
		}

    #ifdef SERIAL_DEBUG_CONSOLE
		if (u16_written > 0) {
			LogBuffer(&ptr_tx_serial->buf, u16_written, "\r\nserial_if_TxProcess: ");
			printf("\r\n");
		}
    #endif

		ptr_tx_serial->todo = 0;
	}

	return ptr_tx_serial->todo;
}

/* ************************************************************************** */

/** @brief	SERIAL_IF Process
 *
 **************************************************************************/
void serial_if_Process(struct buff_msg *rx_serial, struct buff_msg *tx_serial, uint16_t *ptr_HDLC_iframe_tout)
{
	/* uint8_t *rx = &rx_serial->ptr_in; */
	if (serial_if_RxProcess(&rx_serial->msg[rx_serial->ptr_wr])) {      /* serial_if Rx */
		if (++rx_serial->ptr_wr == MSG_BUF_SIZE) {
			rx_serial->ptr_wr = 0;
		}
	}

	if (tx_serial->ptr_rd != tx_serial->ptr_wr) {   /* serial_if Tx */
		if (*ptr_HDLC_iframe_tout == 0) {       /* Check HDLC Interframe Timeout */
			if (!serial_if_TxProcess(&tx_serial->msg[tx_serial->ptr_rd])) {
				HDLC_iframe_tout_init(ptr_HDLC_iframe_tout);
				/* tx_serial->ptr_rd++; */
				if (++tx_serial->ptr_rd == MSG_BUF_SIZE) {
					tx_serial->ptr_rd = 0;
				}
			}
		}
	}
}
