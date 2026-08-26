/**
 *
 * \file
 *
 * \brief Example for a IEC 62056 Server stack - implementation of HDLC layer
 *
 * Copyright (c) 2023 Microchip Technology Inc. and its subsidiaries.
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

#ifndef __HDLC_TYPES_H__
#define __HDLC_TYPES_H__


/*** Includes **************************************************************/
#include <stdio.h>
#include <string.h>

#include "serial_buffer.h"

/* #define DUMP_HDLC */

/*** Constants *************************************************************/
#define HDLC_START_END_FLAG 			0x7E
#define HDLC_FRAME_FORMAT_WITHOUT_SEGMENTATION 	0xA0
#define HDLC_ADDR_METER 			0x03
#define HDLC_ADDR_MODEM 			0xFD
#define HDLC_CONTROL 				0x13

#define HDLC_LLC_DESTINATION_LSAP 		0xE6
#define HDLC_LLC_SOURCE_COMMAND_LSAP  		0xE6
#define HDLC_LLC_SOURCE_RESPONSE_LSAP 		0xE7
#define HDLC_LLC_CONTROL 			0x00

#define MAX_LENGTH_HDLC_PDU       		SERIAL_BUF_SIZE

#define HDLC_INTERFRAME_TIMEOUT			200

/* Maximum length of DLMS APDU over serial_if */
#define OVERHEAD_HDLC 			14
#define DLMS_BUF_SIZE   (SERIAL_BUF_SIZE - OVERHEAD_HDLC)

/*** Structs *************************************************************/
struct dlms_msg
{
          uint8_t todo;          /* Flag to indicate that msg has to be served */
          uint8_t buf[DLMS_BUF_SIZE];  /* Serial data buffer */
          uint16_t length;      /* Length of the data buffer */
};

/*** Functions prototypes **************************************************/
void HDLC_iframe_tout_init(uint16_t *ptr_HDLC_iframe_tout);
void hdlc_init(void);
uint8_t hdlc_RxProcess(struct dlms_msg *ptr_rx_dlms_msg, struct serial_msg *ptr_rx_serial);
uint8_t hdlc_TxProcess(struct serial_msg *ptr_tx_serial, struct dlms_msg *ptr_tx_dlms_msg);

#endif


