/**
 * \file
 *
 * \brief Meter Demo : Command module
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

#ifndef COMMAND_H_INCLUDED
#define COMMAND_H_INCLUDED

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

#include "pdc.h"
#include "metrology.h"

/* Secure Password (3 bytes) */
#define COMPROC_SEC_PWD                 "PIC"

/* Enable ECHO for Console port */
#define COMPROC_ECHO_ON                 true

/* PDC transfer TIMEOUT */
#define COMPROC_TXFER_TIMEOUT           100000

#define BUF_CMD_OUT_iSDEC

#define UPPER(c)                        ((c >= 'a') ? (c - ('a' - 'A')) : c)
#define char_to_digit(c)                ((c) - '0')
#define is_num(c)                       ((c) >= '0' && (c) <= '9')
#define ISCHAR(c)                      (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define is_hex(c)                       (((c) >= '0' && (c) <= '9') || ((c) >= 'A' && (c) <= 'F'))
#define is_cmd(c)                       (((c) == '!' || (c) == '?' || (c) == ';') || ((c) >= 'A' && (c) <= 'Z'))
#define is_blank(c)                     (0x20 == (c) || '\t' == (c))
#define is_help(c)                      ((c) == '?')

  
#define macro_pio_config(pio_group, pio_pin, pio_cfgr_options) do {\
    (pio_group)->PIO_MSKR = (pio_pin);\
    (pio_group)->PIO_CFGR = (pio_cfgr_options);\
    } while(0)
// EXAMPLE: macro_pio_config(PIOD, EMAFE_PD20_PerA_MCSPI_SPCK_MSK, PIO_CFGR__FUNC_PERIPH_A | PIO_CFGR__SR_Medium);
      
      
typedef enum {
	TER_CMD_BUF = 0,      /* Hecho */
	TER_CMD_CAL,          /* Hecho */
	TER_CMD_CNF,          /* Hecho */
	TER_CMD_DAR,          /* Hecho */
	TER_CMD_DCB,          /* Hecho */
	TER_CMD_DCD,          /* Hecho */
	TER_CMD_DCM, 	      /* Hecho */
	TER_CMD_DCR,          /* Hecho */
	TER_CMD_DCS,          /* Hecho */
	TER_CMD_DCW,          /* Hecho */
	TER_CMD_DSR,          /* Hecho */
	TER_CMD_ENC,          /* Hecho */
	TER_CMD_ENR,          /* Hecho */
	TER_CMD_EVEC,         /* Hecho */
	TER_CMD_EVER,         /* Hecho */
	TER_CMD_HAR,          /* Hecho */
	TER_CMD_HRR,          /* Hecho */
	TER_CMD_IDR,          /* Hecho */
	TER_CMD_IDW,          /* Hecho */
	TER_CMD_MDC,          /* Hecho */
	TER_CMD_MDR,          /* Hecho */
	TER_CMD_PAR,          /* Hecho */
	TER_CMD_RTCR,         /* Hecho */
	TER_CMD_RTCW,         /* Hecho */
	TER_CMD_TOUR,         /* Hecho */
	TER_CMD_TOUW,         /* Hecho */
	TER_CMD_RST,          /* Hecho */
	TER_CMD_RLD,          /* Hecho */
	TER_CMD_INVALID = 0xFF,
} terminal_cmd_t;

typedef enum {
	IEC_PROTOCOL = 0,               /* Reserved for future uses */
	ASCII_PROTOCOL  = 1,
	DLT645_PROTOCOL                 /* Reserved for future uses */
} PROTOCOL_TYPE;

typedef enum {
	FREE        = 0,
	RECDATA,
	PROCESS,
	SEND
} UART_STATUS_TYPE;

#define FRAME_MAX_RCV_LEN           110
#define FRAME_MAX_SEND_LEN          255

typedef struct {
	Usart *p_usart;
	Pdc *pdc_base;
	uint8_t send_buff[FRAME_MAX_SEND_LEN];
	uint8_t send_len;
	uint32_t rcv_data;
	uint16_t overtimer;
	uint8_t protocolmode;
	uint8_t sendframenum;
	uint8_t sendframeid;
	uint8_t sendtimer;
	uint8_t offset[2];
	uint8_t state[2];
	uint8_t echo_pending[2];
	uint8_t bufidx;
	uint8_t rcv_buff[2][FRAME_MAX_RCV_LEN];
} com_port_data_t;

typedef enum {
	COMPROC_CONSOLE_ID = 0,
	COMPROC_OPTO_ID,
//	COMPROC_XPLAIN_ID,     /* Reserved for future uses */
//	COMPROC_MIKROBUS_ID,   /* Reserved for future uses */
	COMPROC_NUM_IDS
} com_port_id_t;

#define METER_ID_SIZE           6

typedef struct {
	/* Display Comm Icon timer */
	uint8_t lamptimer;
	/* Meter id */
	uint8_t meterID[METER_ID_SIZE];
	/* Console ECHO */
	bool console_echo_on;
	/* Console Port */
	com_port_data_t comport[COMPROC_NUM_IDS];
} command_t;

extern command_t VCom;

void CommandInit(void);
void CommandConsoleProcess(void);
void CommandOptoProcess(void);
void CommandSendConsoleMsg(const char *msg);

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */

#endif /* COMMAND_H_INCLUDED // */
