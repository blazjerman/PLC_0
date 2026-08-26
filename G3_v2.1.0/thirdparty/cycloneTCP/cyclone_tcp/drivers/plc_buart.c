/**
 * @file plc_buart.c
 * @brief Serial port control used by cycloneTCP
 *
 * @section License
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
 
#define TRACE_LEVEL PPP_TRACE_LEVEL

#include "board.h"
#include "error.h"
#include "plc_buart.h"
#include "ppp/ppp_hdlc.h"
#include "debug.h"

#if BOARD == PL360G55CB_EK || BOARD == PL360G55CF_EK || BOARD == PL485_VB || BOARD == PL485_EK || BOARD == PL460_VB || BOARD == PL480_VB || BOARD == PL480_EK
#include "conf_usb.h"
#endif

/* Check TCP/IP stack configuration */
#if (PPP_SUPPORT == ENABLED)
#if PIC32CX
#include "busart_if.h"
#else
#include "buart_if.h"
#endif

const UartDriver plcBuartDriver = {
	plc_buart_init,
	plc_buart_tick,
	plc_buart_enable_irq,
	plc_buart_disable_irq,
	plc_buart_start_tx
};

/* UART abstraction layer */
error_t plc_buart_init(void)
{
#ifdef UDI_CDC_PORT_NB
	usb_wrp_udc_start();
#else
	#if PIC32CX
	if (busart_if_open(PLC_BUART_CHANNEL, PLC_BUART_SPEED)) {
		return NO_ERROR;
	}
	#else
	if (buart_if_open(PLC_BUART_CHANNEL, PLC_BUART_SPEED)) {
		return NO_ERROR;
	}
	#endif
#endif

	return ERROR_FAILURE;
}

void plc_buart_tick(NetInterface *interface)
{
	unsigned char puc_buff[1024];
	unsigned short us_read;

#ifdef UDI_CDC_PORT_NB
	us_read = usb_wrp_udc_read_buf(puc_buff, 1024);
	if (us_read > 0) {
		printf("UDP Frame rcv  (%d bytes)...\r\n", us_read);
	}
#else
	#if PIC32CX
	us_read = busart_if_read(PLC_BUART_CHANNEL, puc_buff, 1024);
	if (us_read > 0) {
		printf("BUART Frame rcv  (%d bytes)...\r\n", us_read);
	}
	#else
	us_read = buart_if_read(PLC_BUART_CHANNEL, puc_buff, 1024);
	if (us_read > 0) {
		printf("BUART Frame rcv  (%d bytes)...\r\n", us_read);
	}
	#endif
#endif

	if (us_read > 0) {
		int i;
		TRACE_DEBUG("BUART%d Frame rcv  (%d bytes)...\r\n", PLC_BUART_CHANNEL, us_read);
		for (i = 0; i < us_read; i++) {
			pppHdlcDriverWriteRxQueue(interface, puc_buff[i]);
		}
		interface->nicEvent = TRUE;
		osSetEvent(&netEvent);
	}
}

void plc_buart_enable_irq(void)
{
	/* Do notihing*/
}

void plc_buart_disable_irq(void)
{
	/* Do notihing*/
}

void plc_buart_start_tx(NetInterface *interface)
{
	char puc_buff[1024];
	int length = 0;
	int i_ch;

	do {
		pppHdlcDriverReadTxQueue(interface, &i_ch);
		puc_buff[length++] = i_ch;
	} while (i_ch != EOF);

	if (length) {
#ifdef UDI_CDC_PORT_NB
		printf("UDP Frame TX  (%d bytes)...\r\n", length - 1);
		while (!usb_wrp_udc_is_tx_ready()) {
			/* Wait last tx end */
		}
		usb_wrp_udc_write_buf((uint8_t *)puc_buff, length - 1);
#else
	#if PIC32CX
		busart_if_write(PLC_BUART_CHANNEL, puc_buff, length - 1);
		printf("BUART Frame TX  (%d bytes)...\r\n", length - 1);
	#else
		buart_if_write(PLC_BUART_CHANNEL, puc_buff, length - 1);
		printf("BUART Frame TX  (%d bytes)...\r\n", length - 1);
	#endif
#endif
		TRACE_DEBUG("BUART%d Frame TX  (%d bytes)...\r\n", PLC_BUART_CHANNEL, length - 1);
	}
}

#endif
