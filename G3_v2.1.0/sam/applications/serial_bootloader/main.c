/**
 * \file
 *
 * \brief SAM SERIAL Bootloader
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
 *
 */

/**
 * \mainpage SERIAL Bootloader Application
 *
 * \section Purpose
 *
 * The example will help new users get familiar with Microchip's SAMG55 microcontrollers
 * bootloader for SERIAL.
 *
 * \section Requirements
 *
 * This package can be used with PL360G55CB_EK and PL360G55CF_EK.
 *
 * \section Description
 *
 * The bootloader code will be located at 0x0 and executed before any applicative code.
 * Applications compiled to be executed along with the bootloader will start at 0x4000.
 * Before jumping to the application, the bootloader changes the VTOR register
 * to use the interrupt vectors of the application.
 *
 * \section Usage
 *
 * -# Build the program and download it inside the evaluation board.
 * -# Start the application.
 * -# Setting Boot Jumper the board will enter SERIAL monitor mode.
 *
 */

/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include "asf.h"
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <string.h>
#include "conf_board.h"
#include "conf_clock.h"
#include "conf_bootloader.h"
#include "serial_monitor.h"
#include "usart_serial.h"

static void check_start_application(void);

#ifdef CONF_USBCDC_INTERFACE_SUPPORT
static volatile bool main_b_cdc_enable = false;
static volatile bool main_b_dtr_enable = false;
#endif

/**
 * \brief Execute an application from the specified address
 *
 * \param address Application address
 */
void call_app(uint32_t address)
{
	uint8_t i;

	__disable_irq();
	/* Disable SysTick */
	SysTick->CTRL = 0;
	/* Disable IRQs & clear pending IRQs */
	for (i = 0; i < 8; i++) {
		NVIC->ICER[i] = 0xFFFFFFFF;
		NVIC->ICPR[i] = 0xFFFFFFFF;
	}

	/* Modify vector table location */
	__DSB();
	__ISB();
	/* set the stack pointer also to the start of the firmware app */
	__set_MSP(*(int *)(address));
	/* offset the start of the vector table (first 6 bits must be zero) */
	SCB->VTOR = ((uint32_t)address & SCB_VTOR_TBLOFF_Msk);
	__DSB();
	__ISB();
	__enable_irq();

	/* jump to the start of the firmware, casting the address as function
	 * pointer to the start of the firmware. We want to jump to the address
	 * of the reset */

	/* handler function, that is the value that is being pointed at position */
	void (*runFirmware)(void) = NULL;
	runFirmware = (void (*)(void))(*(uint32_t *)(address + 4));
	runFirmware();
}

/**
 * \brief Check the application startup condition
 *
 */
static void check_start_application(void)
{
	uint32_t app_start_address;

        /**
         * Test reset vector of application @APP_START_ADDRESS+4
         * Stay in SERIAL if *(APP_START+0x4) == 0xFFFFFFFF
         * Application erased condition
         */
	app_start_address = *(uint32_t *)(APP_START_ADDRESS + 4);
	if (app_start_address == 0xFFFFFFFF) {
                /* Stay in bootloader */
		return;
	}

        /* Check Boot Jumper */
	ioport_init();
	ioport_set_pin_dir(BOOT_LOAD_PIN, IOPORT_DIR_INPUT);
	ioport_set_pin_mode(BOOT_LOAD_PIN, IOPORT_MODE_PULLUP);
	if (ioport_get_pin_level(BOOT_LOAD_PIN) == 0) {
		return;
	}

	call_app(APP_START_ADDRESS);
}

/**
 *  \brief SERIAL Main loop.
 *  \return Unused (ANSI-C compatibility).
 */
int main(void)
{
        /* Jump in application if condition is satisfied */
	check_start_application();

        /* ASF function to setup clocking. */
	sysclk_init();

        /* Atmel library function to setup for the evaluation kit being used. */
	board_init();

	LED_On(LED0);

#ifdef CONF_USBCDC_INTERFACE_SUPPORT
        /* Start USB stack */
	udc_start();
#endif
        /* UART is enabled in all cases */
	usart_open();

	LED_Off(LED0);
        /* Wait for a complete enum on usb or a '#' char on serial line */
	while (1) {
#ifdef CONF_USBCDC_INTERFACE_SUPPORT
        /* Check if a USB enumeration has succeeded and com port was opened */
		if (main_b_cdc_enable && main_b_dtr_enable) {
			serial_monitor_init(SERIAL_INTERFACE_USBCDC);
            /* SERIAL on USB loop */
			while (1) {
				serial_monitor_run();
			}
		}
#endif
                /* Check if a '#' has been received */
		if (usart_sharp_received()) {
			serial_monitor_init(SERIAL_INTERFACE_USART);
            /* SERIAL on UART loop */
			while (1) {
				serial_monitor_run();
			}
		}
	}
}

#ifdef CONF_USBCDC_INTERFACE_SUPPORT
#ifdef USB_DEVICE_LPM_SUPPORT
void main_suspend_lpm_action(void)
{
}

void main_remotewakeup_lpm_disable(void)
{
}

void main_remotewakeup_lpm_enable(void)
{
}

#endif

bool main_cdc_enable(uint8_t port)
{
	(void)port;
	main_b_cdc_enable = true;
	return true;
}

void main_cdc_disable(uint8_t port)
{
	(void)port;
	main_b_cdc_enable = false;
}

void main_cdc_set_dtr(uint8_t port, bool b_enable)
{
	(void)port;
	main_b_dtr_enable = b_enable;
}

void main_cdc_rx_notify(uint8_t port)
{
	(void)port;
}

void main_cdc_set_coding(uint8_t port, usb_cdc_line_coding_t *cfg)
{
	(void)port;
	(void)cfg;
}

#endif
