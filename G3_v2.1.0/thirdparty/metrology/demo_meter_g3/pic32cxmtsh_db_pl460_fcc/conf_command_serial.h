/**
 * \file
 *
 * \brief Command USART service configuration.
 *
 * Copyright (c) 2020 Microchip Technology Inc. and its subsidiaries.
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

#ifndef CONF_COMMAND_USART_SERIAL_H
#define CONF_COMMAND_USART_SERIAL_H

/** Command UART Interface */
#define CONF_COMMAND_UART               USART0
/** Command UART Interface */
#define CONF_COMMAND_UART_ID            ID_FLEXCOM0
/** Command UART Baudrate setting */
#define CONF_COMMAND_UART_BAUDRATE      (115200UL)
/** Command UART Parity setting */
#define CONF_COMMAND_UART_PARITY        US_MR_PAR_NO
/** Command UART Char Length setting */
#define CONF_COMMAND_UART_CHAR_LENGTH   US_MR_CHRL_8_BIT
/** Command UART Stop bits setting */
#define CONF_COMMAND_UART_STOP_BITS     US_MR_NBSTOP_1_BIT
/** Command UART IRQn */
#define CONF_COMMAND_UART_IRQn          FLEXCOM0_IRQn
/** Command UART Handler */
#define CONF_COMMAND_UART_Handler       FLEXCOM0_Handler

/** Opto UART Interface */
#define CONF_OPTO_UART                  UART
/** Opto UART Interface */
#define CONF_OPTO_UART_ID               ID_UART
/** Opto UART Baudrate setting */
#define CONF_OPTO_UART_BAUDRATE         (1200UL)
/** Opto UART Parity setting */
#define CONF_OPTO_UART_PARITY           US_MR_PAR_EVEN
/** Opto UART IRQn */
#define CONF_OPTO_UART_IRQn             UART_IRQn
/** Opto UART Handler */
#define CONF_OPTO_UART_Handler          UART_Handler

#endif /* CONF_COMMAND_USART_SERIAL_H */
