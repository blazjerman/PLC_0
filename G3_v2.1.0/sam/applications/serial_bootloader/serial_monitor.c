/**
 * \file
 *
 * \brief Monitor functions for SERIAL on SAMG55
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
#include <string.h>
#include "serial_monitor.h"
#include "usart_serial.h"
#include "conf_board.h"
#include "conf_bootloader.h"

const char RomBOOT_Version[] = SERIAL_BOOT_VERSION;

/* Provides one common interface to handle both USART and USB-CDC */
typedef struct {
	/* send one byte of data */
	int (*putc)(int value);
	/* Get one byte */
	int (*getc)(void);
	/* Receive buffer not empty */
	bool (*is_rx_ready)(void);
	/* Send given data (polling) */
	uint32_t (*putdata)(void const *data, uint32_t length);
	/* Get data from comm. device */
	uint32_t (*getdata)(void *data, uint32_t length);
	/* Send given data (polling) using xmodem (if necessary) */
	uint32_t (*putdata_xmd)(void const *data, uint32_t length);
	/* Get data from comm. device using xmodem (if necessary) */
	uint32_t (*getdata_xmd)(void *data, uint32_t length);
} t_monitor_if;

/* Initialize structures with function pointers from supported interfaces */
const t_monitor_if uart_if =
{ usart_putc, usart_getc, usart_rx_is_ready, usart_putdata, usart_getdata,
  usart_putdata_xmd, usart_getdata_xmd };

#ifdef CONF_USBCDC_INTERFACE_SUPPORT
/* Please note that USB doesn't use Xmodem protocol, since USB already includes flow control and data verification */
/* Data are simply forwarded without further coding. */
const t_monitor_if usbcdc_if =
{ udi_cdc_putc, udi_cdc_getc, udi_cdc_is_rx_ready, udi_cdc_write_buf,
  udi_cdc_read_no_polling, udi_cdc_write_buf, udi_cdc_read_buf };
#endif

/* The pointer to the interface object use by the monitor */
t_monitor_if *ptr_monitor_if;

/* b_terminal_mode mode (ascii) or hex mode */
volatile bool b_terminal_mode = false;

/* Communication interface */
static uint8_t suc_com_interface;

/**
 * \brief This function initializes the SERIAL monitor
 *
 * \param com_interface  Communication interface to be used.
 */
void serial_monitor_init(uint8_t com_interface)
{
	suc_com_interface = com_interface;

	/* Selects the requested interface for future actions */
	if (com_interface == SERIAL_INTERFACE_USART) {
		ptr_monitor_if = (t_monitor_if *)&uart_if;
	}

#ifdef CONF_USBCDC_INTERFACE_SUPPORT
	if (com_interface == SERIAL_INTERFACE_USBCDC) {
		ptr_monitor_if = (t_monitor_if *)&usbcdc_if;
	}
#endif
}

/**
 * \brief This function allows data rx by USART
 *
 * \param *data  Data pointer
 * \param length Length of the data
 */
void serial_putdata_term(uint8_t *data, uint32_t length)
{
	uint8_t temp, buf[12], *data_ascii;
	uint32_t i, int_value;

	if (b_terminal_mode) {
		if (length == 4) {
			int_value = *(uint32_t *)data;
		} else if (length == 2) {
			int_value = *(uint16_t *)data;
		} else {
			int_value = *(uint8_t *)data;
		}

		data_ascii = buf + 2;
		data_ascii += length * 2 - 1;

		for (i = 0; i < length * 2; i++) {
			temp = (uint8_t)(int_value & 0xf);

			if (temp <= 0x9) {
				*data_ascii = temp | 0x30;
			} else {
				*data_ascii = temp + 0x37;
			}

			int_value >>= 4;
			data_ascii--;
		}
		buf[0] = '0';
		buf[1] = 'x';
		buf[length * 2 + 2] = '\n';
		buf[length * 2 + 3] = '\r';
		ptr_monitor_if->putdata(buf, length * 2 + 4);
	} else {
		ptr_monitor_if->putdata(data, length);
	}

	return;
}

uint32_t current_number;
uint32_t i, length;
static uint32_t sul_fragment_size;
uint8_t command, *ptr_address_cmd, *ptr, *ptr_data, data[SIZEBUFMAX];
static uint32_t ul_addr, ul_idx;

/**
 * \brief This function starts the SERIAL monitor.
 */
void serial_monitor_run(void)
{
	/* SERIAL command: Set Terminal Mode by default */
	b_terminal_mode = 1;
	ptr_monitor_if->putdata(">", 1);
	ptr_address_cmd = NULL;
	command = 'z';

	/* Start waiting some cmd */
	while (1) {
		length = ptr_monitor_if->getdata(data, SIZEBUFMAX);
		ptr = data;
		for (i = 0; i < length; i++) {
			if (b_terminal_mode) {
				ptr_monitor_if->putdata(data, length);

				if (*ptr == 0x1B) {
					/* Escape: cancel command */
					ptr_monitor_if->putdata("\n\r", 2);
					command = 'z';
					current_number = 0;
					ptr_monitor_if->putdata(">", 1);
					ptr_monitor_if->putdata(">", 1);
					break;
				}
			}

			if (*ptr != 0xff) {
				if (*ptr == '#') {
					if (b_terminal_mode) {
						ptr_monitor_if->putdata("\n\r", 2);
					}

					if (command == 'S') {
						/* SERIAL command: Send a file */
						/* Erase Flash memory */
						ul_addr = APP_START_ADDRESS + 0x1000;
						for (ul_idx = 0; ul_idx < 4; ul_idx++) {
							flash_erase_sector(ul_addr + (0x20000 * ul_idx));
						}

						/* Capture address to write binary file in flash memory */
						ptr_data = ptr_address_cmd;
						/* Waiting to Receive file */
						ul_idx = 0;
						ul_addr = APP_START_ADDRESS;
						ptr = ptr_data;
						while (ul_idx < current_number) {
							sul_fragment_size = current_number - ul_idx;
							/* Check size of fragment received (128 KB max) */
							if (sul_fragment_size > APP_FRAG_MAX_SIZE) {
								sul_fragment_size = APP_FRAG_MAX_SIZE;
							}

							/* read fragment */
							if (suc_com_interface == SERIAL_INTERFACE_USBCDC) {
								sul_fragment_size = ptr_monitor_if->getdata(ptr, sul_fragment_size);
							} else {
								sul_fragment_size = ptr_monitor_if->getdata_xmd(ptr, sul_fragment_size);
							}

							if (sul_fragment_size) {
								/* Update counters and pointers */
								ptr += sul_fragment_size;
								/* ul_addr += sul_fragment_size; */
								ul_idx += sul_fragment_size;
							}

							if (ul_idx >= APP_FRAG_MAX_SIZE) {
								/* Write flash memory */
								flash_write(ul_addr, (const void *)ptr_data, ul_idx, 0);
								current_number -= ul_idx;
								ptr_data += ul_idx;
								ul_addr += ul_idx;
								ul_idx = 0;

								ptr_data = ptr_address_cmd;
								ptr = ptr_data;
							}
						}

						/* Write flash memory */
						flash_write(ul_addr, (const void *)ptr_data, ul_idx, 0);

						__asm("nop");
					} else if (command == 'R') {
						/* SERIAL command: Receive a file */
						ptr_monitor_if->putdata_xmd(ptr_address_cmd, current_number);
					} else if (command == 'O') {
						/* SERIAL command: Write a byte */
						*ptr_address_cmd = (char)current_number;
					} else if (command == 'H') {
						/* SERIAL command: Write a half word */
						*((uint16_t *)ptr_address_cmd) = (uint16_t)current_number;
					} else if (command == 'W') {
						/* SERIAL command: Write a word */
						*((int *)ptr_address_cmd) = current_number;
					} else if (command == 'o') {
						/* SERIAL command: Read a byte */
						serial_putdata_term(ptr_address_cmd, 1);
					} else if (command == 'h') {
						/* SERIAL command: Read a half word */
						current_number = *((uint16_t *)ptr_address_cmd);
						serial_putdata_term((uint8_t *)&current_number, 2);
					} else if (command == 'w') {
						/* SERIAL command: Read a word */
						current_number = *((uint32_t *)ptr_address_cmd);
						serial_putdata_term((uint8_t *)&current_number, 4);
					} else if (command == 'G') {
						/* Go */
						delay_ms(100);
						call_app(APP_START_ADDRESS);
					} else if (command == 'T') {
						/* SERIAL command: Set Terminal Mode */
						b_terminal_mode = 1;
						ptr_monitor_if->putdata("\n\r", 2);
					} else if (command == 'N') {
						/* SERIAL command: Set Normal Mode */
						if (b_terminal_mode == 0) {
							ptr_monitor_if->putdata("\n\r", 2);
						}

						b_terminal_mode = 0;
					} else if (command == 'V') {
						/* SERIAL command: Display version */
						ptr_monitor_if->putdata("v", 1);
						ptr_monitor_if->putdata((uint8_t *)RomBOOT_Version,
								strlen(RomBOOT_Version));
						ptr_monitor_if->putdata(" ", 1);
						ptr = (uint8_t *)&(__DATE__);
						i = 0;
						while (*ptr++ != '\0') {
							i++;
						}
						ptr_monitor_if->putdata((uint8_t *)&(__DATE__), i);
						ptr_monitor_if->putdata(" ", 1);
						i = 0;
						ptr = (uint8_t *)&(__TIME__);
						while (*ptr++ != '\0') {
							i++;
						}
						ptr_monitor_if->putdata((uint8_t *)&(__TIME__), i);
						ptr_monitor_if->putdata("\n\r", 2);
					}

					command = 'z';
					current_number = 0;

					if (b_terminal_mode) {
						ptr_monitor_if->putdata(">", 1);
					}
				} else {
					if (('0' <= *ptr) && (*ptr <= '9')) {
						current_number = (current_number << 4) | (*ptr - '0');
					} else if (('A' <= *ptr) && (*ptr <= 'F')) {
						current_number = (current_number << 4) | (*ptr - 'A' + 0xa);
					} else if (('a' <= *ptr) && (*ptr <= 'f')) {
						current_number = (current_number << 4) | (*ptr - 'a' + 0xa);
					} else if (*ptr == ',') {
						ptr_address_cmd = (uint8_t *)current_number;
						current_number = 0;
					} else {
						command = *ptr;
						current_number = 0;
					}
				}

				ptr++;
			}
		}
	}
}
