/**
 * \file
 *
 * \brief USART functions for SERIAL on SAM0
 *
 * Copyright (c) 2019 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 */

/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include <asf.h>
#include "usart_serial.h"
#include "conf_board.h"
#include "conf_clock.h"
#include "conf_bootloader.h"

/* Variable to let the main task select the appropriate communication interface */
volatile uint8_t b_sharp_received;

static bool sb_receiving_file = false;

/**
 * \brief Open the given USART
 */
void usart_open()
{
	const sam_usart_opt_t usart_settings = {
		.baudrate = BOOT_USART_BAUDRATE,
#ifdef BOOT_USART_CHAR_LENGTH
		.char_length = BOOT_USART_CHAR_LENGTH,
#endif
		.parity_type = BOOT_USART_PARITY,
#ifdef BOOT_USART_STOP_BITS
		.stop_bits = BOOT_USART_STOP_BITS,
#endif
		.channel_mode = US_MR_CHMODE_NORMAL,
	};

	/* Configure USART. */
#if (SAMG55)
	flexcom_enable(BOOT_FLEXCOM);
	flexcom_set_opmode(BOOT_FLEXCOM, FLEXCOM_USART);
#else
	sysclk_enable_peripheral_clock(BOOT_USART_ID);
#endif
	usart_init_rs232(BOOT_USART, &usart_settings, sysclk_get_peripheral_hz());

	/* Enable the receiver and transmitter. */
	usart_enable_tx(BOOT_USART);
	usart_enable_rx(BOOT_USART);

	/* Initialize flag */
	b_sharp_received = false;
}

/**
 * \brief Close the given USART
 */
void usart_close(void)
{
	usart_disable_tx(BOOT_USART);
	usart_disable_rx(BOOT_USART);
}

/**
 * \brief Puts a byte on usart line
 * The type int is used to support printf redirection from compiler LIB.
 *
 * \param value Value to put
 *
 * \return \c 1 if function was successfully done, otherwise \c 0.
 */
int usart_putc(int value)
{
	while (usart_write(BOOT_USART, value) != 0) {
	}
	return 1;
}

int usart_getc(void)
{
	uint32_t retval;

	/* Wait until input buffer is filled */
	while (usart_read(BOOT_USART, &retval)) {
	}
	return (int)retval;
}

int usart_sharp_received(void)
{
	if (usart_is_rx_ready(BOOT_USART)) {
		if (usart_getc() == SHARP_CHARACTER) {
			return (true);
		}
	}

	return (false);
}

bool usart_rx_is_ready(void)
{
	return (bool)usart_is_rx_ready(BOOT_USART);
}

/**
 * \brief Send given data (polling)
 */
uint32_t usart_putdata(void const *data, uint32_t length)
{
	uint32_t i;
	uint8_t *ptrdata;

	ptrdata = (uint8_t *)data;
	for (i = 0; i < length; i++) {
		usart_putc(*ptrdata);
		ptrdata++;
	}
	return (i);
}

/**
 * \brief Get data from com device
 */
uint32_t usart_getdata(void *data, uint32_t length)
{
	uint8_t *ptrdata;

	(void)length;
	ptrdata = (uint8_t *)data;
	*ptrdata = usart_getc();
	return (1);
}

/**
 * \brief Compute the CRC
 */
unsigned short add_crc(char ptr, unsigned short crc)
{
	unsigned short cmpt;

	crc = crc ^ (int)ptr << 8;

	for (cmpt = 0; cmpt < 8; cmpt++) {
		if (crc & 0x8000) {
			crc = crc << 1 ^ CRC16POLY;
		} else {
			crc = crc << 1;
		}
	}

	return (crc & 0xFFFF);
}

static uint16_t getbytes(uint8_t *ptr_data, uint16_t length)
{
	uint16_t crc = 0;
	uint16_t cpt;
	uint8_t c;

	for (cpt = 0; cpt < length; ++cpt) {
		c = usart_getc();
		crc = add_crc(c, crc);
		/* crc = (crc << 8) ^ xcrc16tab[(crc>>8) ^ c]; */
		*ptr_data++ = c;
	}

	return crc;
}

/**
 * \brief Used by Xup to send packets.
 */
static int putPacket(uint8_t *tmppkt, uint8_t sno)
{
	uint32_t i;
	uint16_t chksm;
	uint8_t data;

	chksm = 0;

	usart_putc(SOH);

	usart_putc(sno);
	usart_putc((uint8_t) ~(sno));

	for (i = 0; i < PKTLEN_128; i++) {
		data = *tmppkt++;
		usart_putc(data);

		chksm = add_crc(data, chksm);
	}

	/* An "endian independent way to extract the CRC bytes. */
	usart_putc((uint8_t)(chksm >> 8));
	usart_putc((uint8_t)chksm);

	return (usart_getc()); /* Wait for ack */
}

/**
 * \brief Used by Xdown to retrieve packets.
 */
uint8_t getPacket(uint8_t *ptr_data, uint8_t sno)
{
	uint8_t seq[2];
	uint16_t crc, xcrc;

	getbytes(seq, 2);
	xcrc = getbytes(ptr_data, PKTLEN_128);

	/* An "endian independent way to combine the CRC bytes. */
	crc = (uint16_t)usart_getc() << 8;
	crc += (uint16_t)usart_getc();

	if ((crc != xcrc) || (seq[0] != sno) || (seq[1] != (uint8_t)(~sno))) {
		usart_putc(NAK);
		return (false);
	}

	usart_putc(ACK);
	return (true);
}

/**
 * \brief Called when a transfer from target to host is being made(considered an upload).
 */
uint32_t usart_putdata_xmd(void const *data, uint32_t length)
{
	uint8_t c, sno = 1;
	uint8_t done;
	uint8_t *ptr_data = (uint8_t *)data;

	if (!length) {
		return (1);
	}

	if (length & (PKTLEN_128 - 1)) {
		length += PKTLEN_128;
		length &= ~(PKTLEN_128 - 1);
	}

	/* Startup synchronization... */
	/* Wait to receive a NAK or 'C' from receiver. */
	done = 0;
	while (!done) {
		c = (uint8_t)usart_getc();

		switch (c) {
		case NAK:
			done = 1;
			break;

		case 'C':
			done = 1;
			break;

		case 'q': /* ELS addition, not part of XMODEM spec. */
			return (0);

		default:
			break;
		}
	}

	done = 0;
	sno = 1;
	while (!done) {
		c = (uint8_t)putPacket((uint8_t *)ptr_data, sno);

		switch (c) {
		case ACK:
			++sno;
			length -= PKTLEN_128;
			ptr_data += PKTLEN_128;
			break;

		case NAK:
			break;

		case CAN:
		case EOT:
		default:
			done = 0;
			break;
		}
		if (!length) {
			usart_putc(EOT);
			usart_getc(); /* Flush the ACK */
			break;
		}
	}

	return (1);
}

/**
 * \brief Called when a transfer from host to target is being made (considered an download).
 */
uint32_t usart_getdata_xmd(void *data, uint32_t length)
{
	uint32_t timeout;
	char c;
	uint8_t *ptr_data = (uint8_t *)data;
	uint32_t ul_data_nb = 0;
	uint32_t b_run, nbr_of_timeout = 100;
	uint8_t sno = 0x01;

	/* Copied from legacy source code ... might need some tweaking */
	uint32_t loops_per_second = sysclk_get_peripheral_hz() / 10;

	if (!length) {
		return (0);
	}

	if (!sb_receiving_file) {
		/* Startup synchronization... */
		/* Continuously send NAK or 'C' until sender responds. */
		while (1) {
			usart_putc('C');
			timeout = loops_per_second;
			while (!(usart_is_rx_ready(BOOT_USART)) && timeout) {
				timeout--;
			}
			if (timeout) {
				break;
			}

			if (!(--nbr_of_timeout)) {
				return (0);
			}
		}
		sb_receiving_file = true;
	} else {
		/* Assume that last packet has been lost. Send NACK to retransmit */
		usart_putc(NAK);
	}

	b_run = true;
	while (b_run != false) {
		c = (char)usart_getc();

		switch (c) {
		case SOH: /* 128-byte incoming packet */
			if (getPacket(ptr_data, sno)) {
				++sno;
				ptr_data += PKTLEN_128;
				ul_data_nb += PKTLEN_128;
			}

			/* Check length of fragment */
			if (ul_data_nb >= length) {
				b_run = false;

				/* Check if it is the last EOT */
				c = (char)usart_getc();
				if (c == EOT) {
					usart_putc(ACK);
					sb_receiving_file = false;
				}
			}

			break;

		case EOT:
			usart_putc(ACK);
			b_run = false;
			sb_receiving_file = false;
			break;

		case CAN:
		/* "X" User-invoked abort */
		case ESC:
		default:
			b_run = false;
			sb_receiving_file = false;
			break;
		}
	}

	return (ul_data_nb);
}
