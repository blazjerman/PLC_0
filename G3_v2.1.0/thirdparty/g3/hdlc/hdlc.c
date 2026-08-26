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

//*** Includes ************************************************************
#include "compiler.h"
#include "Logger.h"
#include "serial_buffer.h"
#include "hdlc.h"

/* Global variables */
uint16_t hcs, fcs;
uint8_t hdlc_tail_pos;


/*
 * FCS Look up table
 */
uint16_t FCSTable[ 256 ] =
    {
        0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
        0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
        0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
        0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
        0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
        0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
        0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
        0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
        0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
        0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
        0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
        0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
        0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
        0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
        0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
        0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
        0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
        0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
        0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
        0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
        0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
        0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
        0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
        0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
        0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
        0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
        0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
        0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
        0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
        0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
        0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
        0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
    };

//*** Functions  **********************************************************
/*
 * Function: _hdlcChksumCalculate
 *  Description: This function calculates the check sum, both header
 *  checksum and frame check sum
 * Arguments:
 *  unsigned short fcs - 0xFFFF
 *  unsigned char *cp_p - pointer to buffer containing data, for which checksum is
 *  to be calculated
 *  unsigned short len - length of data, for which checksum is to be calculated
 * Returns:
 *  static - checksum
 */
static uint16_t hdlc_ChksumCalculate( uint16_t _fcs, uint8_t *cp_p, uint8_t len )
{
    while ( len != 0 )
    {
        _fcs = (uint16_t) ((_fcs >> 8) ^ FCSTable[ ( _fcs ^ *cp_p++ ) & 0xff ]);
        len--;
    }

    return _fcs;
}

/*
 * Function: HDLC_iframe_tout_init
 *  Description: Initialize Hdlc Interframe Timeout
 * Arguments:
 *  void
 * Returns:
 *  void
 */
void HDLC_iframe_tout_init(uint16_t *ptr_HDLC_iframe_tout)
{
  	*ptr_HDLC_iframe_tout = HDLC_INTERFRAME_TIMEOUT;
}

/*
 * Function: hdlc_init
 *  Description: Initialize Hdlc Layer config parameters
 * Arguments:
 *  void
 * Returns:
 *  void
 */
void hdlc_init(void)
{
	/* DO NOTHING */
}

uint8_t hdlc_RxProcess(struct dlms_msg *ptr_rx_dlms_msg, struct serial_msg *ptr_rx_serial)
{
	uint16_t rx_hcs, rx_fcs;

	ptr_rx_dlms_msg->todo = 0;

	if (ptr_rx_serial->length != 0){
#ifdef DUMP_HDLC
    LogDump(ptr_rx_serial->buf, ptr_rx_serial->length);
#endif /* DUMP_HDLC */

    /* Length_DLMS_APDU =  LENGTH HDLC - HDLC_header(12bytes) [Excluded FLAGs] */
		ptr_rx_dlms_msg->length = (uint16_t) ptr_rx_serial->buf[2] - 12;

		/* CHECK HCS */
		rx_hcs = (uint16_t) ptr_rx_serial->buf[6];
		rx_hcs = (uint16_t)(ptr_rx_serial->buf[7] << 8) | rx_hcs;
		hcs = hdlc_ChksumCalculate(0xFFFF, &ptr_rx_serial->buf[1], 5);
		hcs ^= 0xffff;		/* Complement */

		if (hcs != rx_hcs){
		  	/* ERROR HCS */
		  	ptr_rx_serial->todo = 0;
			return ptr_rx_dlms_msg->todo;
		}

		/* CHECK FCS */
		hdlc_tail_pos = ptr_rx_serial->length - 3;

		rx_fcs = (uint16_t) ptr_rx_serial->buf[hdlc_tail_pos];
		rx_fcs = (uint16_t)(ptr_rx_serial->buf[hdlc_tail_pos + 1] << 8) | rx_fcs;
		fcs = hdlc_ChksumCalculate(0xFFFF, &ptr_rx_serial->buf[1], ptr_rx_serial->length - 4);
		fcs ^= 0xffff;		/* Complement */

		if (fcs != rx_fcs){
		 	/* ERROR FCS */
		  	ptr_rx_serial->todo = 0;
			return ptr_rx_dlms_msg->todo;
		}

		memcpy(&ptr_rx_dlms_msg->buf, &ptr_rx_serial->buf[11], ptr_rx_dlms_msg->length);
		ptr_rx_dlms_msg->todo = 1;
	}

	ptr_rx_serial->todo = 0;
	return ptr_rx_dlms_msg->todo;
}

uint8_t hdlc_TxProcess(struct serial_msg *ptr_tx_serial, struct dlms_msg *ptr_tx_dlms_msg)
{

  if (ptr_tx_dlms_msg->todo){
    /* FILL SERIAL DATA BUFFER */
    /* HDLC FLAG */
    ptr_tx_serial->buf[0] = HDLC_START_END_FLAG;

    /* FRAME FORMAT */
    ptr_tx_serial->buf[1] = HDLC_FRAME_FORMAT_WITHOUT_SEGMENTATION;

    /* LENGTH = Length_DLMS_APDU + HDLC_header(14bytes) */
    ptr_tx_serial->length = ptr_tx_dlms_msg->length + 14;    // FILL SERIAL DATA LENGTH
    ptr_tx_serial->buf[2] = ptr_tx_serial->length - 2;  // Excluding HDLC_START_END_FLAGs

    /* HDLC ADDRESSES */
    ptr_tx_serial->buf[3] = HDLC_ADDR_METER; /* DEST ADDR */
    ptr_tx_serial->buf[4] = HDLC_ADDR_MODEM; /* SOURCE ADDR */

    /* CONTROL (0x13)*/
    ptr_tx_serial->buf[5] = HDLC_CONTROL;

    /* HCS */
    hcs = hdlc_ChksumCalculate(0xFFFF, &ptr_tx_serial->buf[1], 5);
    hcs ^= 0xffff;		/* Complement */
    ptr_tx_serial->buf[6] = (uint8_t) (hcs & 0x00FF);
    ptr_tx_serial->buf[7] = (uint8_t) ((hcs & 0xFF00) >> 8);

    /* LLC */
    ptr_tx_serial->buf[8] = HDLC_LLC_DESTINATION_LSAP;
    ptr_tx_serial->buf[9] = HDLC_LLC_SOURCE_COMMAND_LSAP;
    ptr_tx_serial->buf[10] = HDLC_LLC_CONTROL;

    /* HDCL DATA. DLMS APDU */
    memcpy(&ptr_tx_serial->buf[11], ptr_tx_dlms_msg->buf, ptr_tx_dlms_msg->length);

    hdlc_tail_pos = ptr_tx_dlms_msg->length + 11;

    /* FCS */
    fcs = hdlc_ChksumCalculate(0xFFFF, &ptr_tx_serial->buf[1], ptr_tx_serial->length - 4);
    fcs ^= 0xffff;		/* Complement */
    ptr_tx_serial->buf[hdlc_tail_pos] = (uint8_t) (fcs & 0x00FF);
    ptr_tx_serial->buf[hdlc_tail_pos + 1] = (uint8_t) ((fcs & 0xFF00) >> 8);

    /* HDLC FLAG */
    ptr_tx_serial->buf[hdlc_tail_pos + 2] = HDLC_START_END_FLAG;

    // FILL SERIAL TODO FLAG
    ptr_tx_serial->todo = 1;
#ifdef DUMP_HDLC
    LogDump(ptr_tx_serial->buf, ptr_tx_serial->length);
#endif /* DUMP_HDLC */
  }

  return ptr_tx_serial->todo;

}






