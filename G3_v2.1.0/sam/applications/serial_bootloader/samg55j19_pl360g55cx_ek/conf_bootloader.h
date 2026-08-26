/**
 * \file
 *
 * \brief Bootloader specific configuration.
 *
 * Copyright (c) 2015-2018 Microchip Technology Inc. and its subsidiaries.
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

/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#ifndef CONF_BOOTLOADER_H_INCLUDED
#define CONF_BOOTLOADER_H_INCLUDED

#include "conf_board.h"

#define APP_START_ADDRESS          (IFLASH_ADDR + 0x4000) /* 16 KB for Bootloader */
#define APP_FRAG_MAX_SIZE          0x10000 /* 64 KB */

#define BOOT_LOAD_PIN              IOPORT_CREATE_PIN(PIOA, 18)

#define CONF_USBCDC_INTERFACE_SUPPORT

#define BOOT_USART                 CONSOLE_UART
#define BOOT_USART_ID              CONSOLE_UART_ID
#define BOOT_USART_BAUDRATE        115200
#define BOOT_USART_PARITY          US_MR_PAR_NO
#define BOOT_USART_CHAR_LENGTH     US_MR_CHRL_8_BIT
#define BOOT_USART_STOP_BITS       US_MR_NBSTOP_1_BIT
#define BOOT_FLEXCOM               BOARD_FLEXCOM_USART4

#endif /* CONF_BOOTLOADER_H_INCLUDED */
