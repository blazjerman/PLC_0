/**
 * \file
 *
 * \brief ATMEL PLC PHY TX Test Console Application
 *
 * Copyright (c) 2020 Atmel Corporation. All rights reserved.
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

/**
 *  \mainpage ATMEL PLC PHY TX Test Console Application
 *
 *  \section Purpose
 *
 *  The PHY TX Test Console Application demonstrates how to configure some
 * parameters from the PHY layer on PLC boards.
 *
 *  \section Requirements
 *
 *  This package should be used with any PLC board on which there is PLC
 * hardware dedicated.
 *
 *  \section Description
 *
 *  This application can configure the PHY with a serial interface and test
 * PLC transmission/reception processes.
 *
 *  \section Usage
 *
 *  The tool is ready to configure, transmit and receive.
 */

#include <string.h>

/* Atmel boards includes. */
#include "board.h"

/* Atmel library includes. */
#include "asf.h"
#include "math.h"

/* G3 includes */
#include "conf_atpl360.h"
#include "phy_embedded_example.h"
#include "general_defs.h"
#include "atpl360_IB.h"
/* Example configuration */
#include "conf_project.h"

#ifndef SAM4C
/* random number library */
#include "stdlib.h"
#endif

#ifdef CONF_BOARD_CONSOLE_UDP
#define LED_NUM_BLINKS_RST      100
#else
#define LED_NUM_BLINKS_RST      30
#endif

/* Disable PCK external monitor */
#define CONF_PCK_DISABLE       0
/* Enable MCK external monitor */
#define CONF_PCK_MCK_EN        1
/* Enable SLCK external monitor */
#define CONF_PCK_SLCK_EN       2

/* PCK external monitor pin */
#if BOARD == PL360G55CB_EK || BOARD == PL360G55CF_EK
#include "genclk.h"
#define CONF_PCK_EXT           CONF_PCK_DISABLE
#define PCK_EXT_GPIO                     PIN_PCK1
#define PCK_EXT_FLAG                     PIN_PCK1_MUX
#define PCK_EXT_PCK_ID                   GENCLK_PCK_1
#else
#define CONF_PCK_EXT           CONF_PCK_DISABLE
#define PCK_EXT_GPIO                     PIO_PA0_IDX
#define PCK_EXT_FLAG                     IOPORT_MODE_MUX_B
#define PCK_EXT_PCK_ID                   PMC_PCK_2
#endif

/* Timers Configuration */
#if (!PIC32CX)
#define ID_TC_1MS                       ID_TC3
#define TC_1MS                          TC1
#define TC_1MS_CHN                      0
#define TC_1MS_IRQn                     TC3_IRQn
#define TC_1MS_Handler                  TC3_Handler
#else
#define ID_TC_1MS                       ID_TC1_CHANNEL0
#define TC_1MS                          TC1
#define TC_1MS_CHN                      0
#define TC_1MS_IRQn                     TC1_CHANNEL0_IRQn
#define TC_1MS_Handler                  TC1_CHANNEL0_Handler
#endif

void initTimer1ms(void);

#define COUNT_MS_SWAP_LED       500

static uint32_t ul_count_ms = COUNT_MS_SWAP_LED;
static uint8_t uc_led_swap = 0;
#if (BOARD == PIC32CXMTSH_DB) || (BOARD == PIC32CXMTC_DB)
#ifdef CONF_BOARD_LCD_EN
static bool b_led_swap_on = false;
#endif
#endif

/* Global milliseconds counter */
uint32_t g_ms_counter;

/* Function declarations */
static void prvSetupHardware(void);

#define STRING_EOL    "\r"
#define STRING_HEADER "\r\n-- ATMEL PLC Getting Started Application --\r\n" \
	"-- "BOARD_NAME " --\r\n" \
	"-- Compiled: "__DATE__ " "__TIME__ " --\r\n" \
	"-- PHY version: "ATPL360_HOST_DESCRIPTION " --\r\n"

#define MENU_HEADER "\n\r-- Menu Configuration --------------\n\r" \
	"0: Select TX Level\n\r" \
	"1: Select Modulation/Scheme\n\r" \
	"2: Select time period between messages to transmit(us.)\n\r" \
	"3: Select Data to transmit\n\r" \
	"4: Select TX tone Map\n\r" \
	"5: Select TX preemphasis\n\r" \
	"6: Select Branch Mode\n\r" \
	"7: Set/Clear Force No Output Signal\n\r" \
	"v: View TX configuration values\n\r" \
	"e: Execute transmission application\n\r" \
	"otherwise: Display this main menu\n\n\r"

#define MENU_SCHEME "\n\r-- Modulation Scheme --------------\r\n" \
	"0: DBPSK\n\r" \
	"1: DQPSK\n\r" \
	"2: D8PSK\n\r" \
	"3: Differential Robust\n\r" \
	"\n\r" \
	"4: Coherent BPSK\n\r" \
	"5: Coherent QPSK\n\r" \
	"6: Coherent 8PSK\n\r" \
	"7: Coherent Robust\n\r" \

#define MENU_MODE "\n\r-- Transmission Mode --------------\r\n"	\
	"0: Immediate and Not Forced\n\r" \
	"1: Immediate and Forced\n\r" \
	"2: Delayed and Not Forced\n\r"	\
	"3: Delayed and Forced\n\r"

#define MENU_DATA_MODE "\n\r-- Select Data Mode --------------\r\n" \
	"0: Random Data\n\r" \
	"1: Fixed Data\n\r" \
	"2: Manual Data\n\r" \
	"3: Manual Hex Data\n\r"

#define MENU_BRANCH_MODE "\n\r-- Select Branch Mode --------------\r\n"	\
	"0: Autodetect\n\r" \
	"1: High Impedance\n\r"	\
	"2: Very Low Impedance\n\r"

#define MENU_NO_OUTPUT "\n\r-- Force No Output Signal --------------\r\n" \
	"0: Clear\n\r" \
	"1: Set\n\r"

#define MENU_CONSOLE "\n\rPHY-Console>"

/* Phy data configuration */
extern txPhyEmbeddedConfig_t xAppPhyCfgTx;

/* Atpl360 instance  */
extern atpl360_descriptor_t sx_atpl360_desc;

/* G3 Band identifier */
extern uint8_t suc_phy_band;

/* Tx data buffer */
uint8_t ucv_tx_data_buffer[512];

/* { CEN_A, FCC, ARIB, CEN_B } */
static const uint8_t spuc_tonemap_size[4] = {TONE_MAP_SIZE_CENELEC, TONE_MAP_SIZE_FCC_ARIB, TONE_MAP_SIZE_FCC_ARIB, TONE_MAP_SIZE_CENELEC};
static const uint8_t spuc_numsubbands[4] = {NUM_SUBBANDS_CENELEC_A, NUM_SUBBANDS_FCC, NUM_SUBBANDS_ARIB, NUM_SUBBANDS_CENELEC_B};

/**
 * Read from Serial
 */
static uint32_t _read_char(uint8_t *c)
{
	/* Reset watchdog */
#if (!PIC32CX)
	WDT->WDT_CR = WDT_CR_KEY_PASSWD | WDT_CR_WDRSTT;
#else
	DWDT->WDT0_CR = WDT0_CR_KEY_PASSWD | WDT0_CR_WDRSTT;
	DWDT->WDT1_CR = WDT1_CR_KEY_PASSWD | WDT1_CR_WDRSTT;
#endif

#ifdef CONF_BOARD_CONSOLE_UDP
	uint16_t us_res;

	/*	*c = getchar(); */
	us_res = usb_wrp_udc_read_buf(c, 1) ? 0 : 1;
	return us_res;

#else
#if SAMG55 || PIC32CX
	uint32_t ul_char;
	uint32_t ul_res;

	ul_res = usart_read((Usart *)CONF_UART, (uint32_t *)&ul_char);
	*c = (uint8_t)ul_char;

	return ul_res;

#else
	return uart_read((Uart *)CONF_UART, c);
#endif
#endif
}

/**
 *  Configure UART console.
 */
static void configure_dbg_console(void)
{
#ifdef CONF_BOARD_CONSOLE_UDP
#if SAMG55
	stdio_udc_init(UDP, (void *)usb_wrp_udc_putchar, (void *)usb_wrp_udc_getchar, (void *)usb_wrp_udc_start);
#else
	stdio_udc_init(USBHS, (void *)usb_wrp_udc_putchar, (void *)usb_wrp_udc_getchar, (void *)usb_wrp_udc_start);
#endif
#else
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
#ifdef CONF_UART_CHAR_LENGTH
		.charlength = CONF_UART_CHAR_LENGTH,
#endif
		.paritytype = CONF_UART_PARITY,
#ifdef CONF_UART_STOP_BITS
		.stopbits = CONF_UART_STOP_BITS,
#endif
	};

	/* Configure console UART. */
	sysclk_enable_peripheral_clock(CONF_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
#endif
}

/* ************************************************************************** */

/** @brief	Interrupt handler for Timer 3
 *
 * Handler for Timer 3
 **************************************************************************/
void TC_1MS_Handler(void)
{
	volatile uint32_t ul_dummy;
	/* Clear status bit to acknowledge interrupt */
	ul_dummy = tc_get_status(TC_1MS, TC_1MS_CHN);
	/* Avoid compiler warning */
	UNUSED(ul_dummy);

	/* update count ms */
	if (!ul_count_ms--) {
		ul_count_ms = COUNT_MS_SWAP_LED;
		uc_led_swap = true;
	}

	g_ms_counter++;
}

/* ************************************************************************** */

/** @brief	Init Timer interrupt (1ms)
 *
 * Initialize 1mSec timer 3 interrupt
 **************************************************************************/
void initTimer1ms(void)
{
	uint32_t ul_div, ul_tcclks;
	uint32_t ul_sysclk = 0;

	/* Configure PMC */
	pmc_enable_periph_clk(ID_TC_1MS);

	ul_tcclks = 2;

#if (!PIC32CX)
	ul_sysclk = sysclk_get_peripheral_hz();
	/* MCK = ul_sysclk -> tcclks = 2 : TCLK3 = MCK/32 Hz -> ul_div = (MCK/32)/1000 to get 1ms timer */
	ul_div = ul_sysclk / 32000;
	tc_init(TC_1MS, TC_1MS_CHN, ul_tcclks | TC_CMR_CPCTRG);
#else
	ul_sysclk = sysclk_get_cpu_hz();
	/* MCK = ul_sysclk -> tcclks = 2 : TCLK3 = MCK/32 Hz -> ul_div = (MCK/32)/1000 to get 1ms timer */
	ul_div = ul_sysclk / 32000;
	tc_init(TC_1MS, TC_1MS_CHN, ul_tcclks | TC_CMR_CPCTRG, 0);
#endif

	tc_write_rc(TC_1MS, TC_1MS_CHN, ul_div);

	/* Configure and enable interrupt on RC compare */
	NVIC_SetPriority((IRQn_Type)ID_TC_1MS, 0);
	NVIC_EnableIRQ((IRQn_Type)ID_TC_1MS);
	tc_enable_interrupt(TC_1MS, TC_1MS_CHN, TC_IER_CPCS);

	/** Start the timer. TC1, chanel 0 = TC3 */
	tc_start(TC_1MS, TC_1MS_CHN);
}

/**
 * \brief Calculates maximun length in bytes of psdu depending on Modulation, ToneMap, ToneMask and 1/2 RS blocks
 *
 * \return Max PSDU size in bytes
 *
 */
static uint16_t get_max_psdu_len(void)
{
	max_psdu_len_params_t x_max_psdu_len_params;
	uint16_t us_max_psdu_len;

	/** Fill message parameters to compute maximum PSDU length **/
	/* Modulation Type */
	x_max_psdu_len_params.uc_mod_type = xAppPhyCfgTx.xPhyMsg.uc_mod_type;
	/* Modulation scheme */
	x_max_psdu_len_params.uc_mod_scheme = xAppPhyCfgTx.xPhyMsg.uc_mod_scheme;
	/* 1 or 2 Reed-Solomon blocks (only for FCC) */
	if (suc_phy_band == ATPL360_WB_FCC) {
		x_max_psdu_len_params.uc_2_rs_blocks = xAppPhyCfgTx.xPhyMsg.uc_2_rs_blocks;
	}

	/* Tone Map */
	memset(x_max_psdu_len_params.puc_tone_map, 0, TONE_MAP_SIZE_MAX);
	memcpy(x_max_psdu_len_params.puc_tone_map, xAppPhyCfgTx.xPhyMsg.puc_tone_map, spuc_tonemap_size[suc_phy_band - 1]);

	/* Set parameters for MAX_PSDU_LEN */
	sx_atpl360_desc.set_config(ATPL360_REG_MAX_PSDU_LEN_PARAMS, &x_max_psdu_len_params, sizeof(max_psdu_len_params_t));

	/* Get MAX_PSDU_LEN */
	sx_atpl360_desc.get_config(ATPL360_REG_MAX_PSDU_LEN, &us_max_psdu_len, 2, true);

	return us_max_psdu_len;
}

/* StateMachine Events */
typedef enum {
	MAIN_MENU_OPT0,
	MAIN_MENU_OPT1,
	MAIN_MENU_OPT2,
	MAIN_MENU_OPT3,
	MAIN_MENU_OPT4,
	MAIN_MENU_OPT5,
	MAIN_MENU_OPT6,
	MAIN_MENU_OPT7,
	MAIN_MENU_VIEW_CFG,
	MAIN_MENU_EXECUTE,
	STOP_TEST,
	UNKNOW_CHAR,
	NO_CHAR
} EIndication;

/* StateMachine holds the current state pointer. */
typedef struct StateMachine TStateMachine;

/* StateProc function pointer type that represent each state of our machine. */
typedef void (*StateProc)(TStateMachine *sm, EIndication input);

struct StateMachine {
	StateProc curState;
};

void main_menu_show(TStateMachine *sm, EIndication input);
void main_menu_wait(TStateMachine *sm, EIndication input);
void opt0_menu(TStateMachine *sm, EIndication input);
void opt1_menu(TStateMachine *sm, EIndication input);
void opt2_menu(TStateMachine *sm, EIndication input);
void opt3_menu(TStateMachine *sm, EIndication input);
void opt4_menu(TStateMachine *sm, EIndication input);
void opt5_menu(TStateMachine *sm, EIndication input);
void opt6_menu(TStateMachine *sm, EIndication input);
void opt7_menu(TStateMachine *sm, EIndication input);
void show_cfg(TStateMachine *sm, EIndication input);
void executing(TStateMachine *sm, EIndication input);

EIndication processInput(void);

TStateMachine machine = { main_menu_show };

/**
 * Set configuration parameters in GPBR
 */
static void save_config(uint8_t cmd_start_mode)
{
	uint32_t ul_gpbr_value;

	ul_gpbr_value = cmd_start_mode;
	ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.uc_mod_type << 4; /* 3 bits */
	ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.uc_mod_scheme << 7; /* 1 bit */
	ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.uc_tx_power << 8; /* 5 bits */
	ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.uc_tx_mode << 13; /* 2 bits */
	ul_gpbr_value += xAppPhyCfgTx.uc_is_random << 15; /* 1 bit */
	ul_gpbr_value += xAppPhyCfgTx.uc_force_no_output << 16; /* 1 bit */

	gpbr_write(GPBR0, ul_gpbr_value);
	if ((suc_phy_band == ATPL360_WB_CENELEC_A) || (suc_phy_band == ATPL360_WB_CENELEC_B)) {
		gpbr_write(GPBR1, (uint32_t)xAppPhyCfgTx.xPhyMsg.puc_tone_map[0]);
	} else { /* ATPL360_WB_FCC / ATPL360_WB_ARIB */
		ul_gpbr_value = xAppPhyCfgTx.xPhyMsg.puc_tone_map[0];
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_tone_map[1] << 8;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_tone_map[2] << 16;
		gpbr_write(GPBR1, ul_gpbr_value);
	}

	gpbr_write(GPBR2, xAppPhyCfgTx.ul_tx_period);
	ul_gpbr_value = xAppPhyCfgTx.xPhyMsg.us_data_len;
	ul_gpbr_value += xAppPhyCfgTx.uc_tx_impedance << 16;
	ul_gpbr_value += xAppPhyCfgTx.uc_tx_auto << 24;
	gpbr_write(GPBR3, ul_gpbr_value);

#if (SAM4C || SAM4CP || SAM4CM)
	if (suc_phy_band == ATPL360_WB_CENELEC_A) {
		ul_gpbr_value = xAppPhyCfgTx.xPhyMsg.puc_preemphasis[0];
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[1] << 8;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[2] << 16;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[3] << 24;
		gpbr_write(GPBR8, ul_gpbr_value);

		ul_gpbr_value = xAppPhyCfgTx.xPhyMsg.puc_preemphasis[4];
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[5] << 8;
		gpbr_write(GPBR9, ul_gpbr_value);
	} else if (suc_phy_band == ATPL360_WB_CENELEC_B) {
		ul_gpbr_value = xAppPhyCfgTx.xPhyMsg.puc_preemphasis[0];
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[1] << 8;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[2] << 16;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[3] << 24;
		gpbr_write(GPBR8, ul_gpbr_value);
	} else if (suc_phy_band == ATPL360_WB_FCC) {
		ul_gpbr_value = xAppPhyCfgTx.xPhyMsg.puc_preemphasis[0];
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[1] << 8;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[2] << 16;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[3] << 24;
		gpbr_write(GPBR8, ul_gpbr_value);

		ul_gpbr_value = xAppPhyCfgTx.xPhyMsg.puc_preemphasis[4];
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[5] << 8;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[6] << 16;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[7] << 24;
		gpbr_write(GPBR9, ul_gpbr_value);

		ul_gpbr_value = xAppPhyCfgTx.xPhyMsg.puc_preemphasis[8];
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[9] << 8;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[10] << 16;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[11] << 24;
		gpbr_write(GPBR10, ul_gpbr_value);

		ul_gpbr_value = xAppPhyCfgTx.xPhyMsg.puc_preemphasis[12];
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[13] << 8;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[14] << 16;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[15] << 24;
		gpbr_write(GPBR11, ul_gpbr_value);

		ul_gpbr_value = xAppPhyCfgTx.xPhyMsg.puc_preemphasis[16];
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[17] << 8;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[18] << 16;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[19] << 24;
		gpbr_write(GPBR12, ul_gpbr_value);

		ul_gpbr_value = xAppPhyCfgTx.xPhyMsg.puc_preemphasis[20];
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[21] << 8;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[22] << 16;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[23] << 24;
		gpbr_write(GPBR13, ul_gpbr_value);
	} else if (suc_phy_band == ATPL360_WB_ARIB) {
		ul_gpbr_value = xAppPhyCfgTx.xPhyMsg.puc_preemphasis[0];
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[1] << 8;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[2] << 16;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[3] << 24;
		gpbr_write(GPBR8, ul_gpbr_value);

		ul_gpbr_value = xAppPhyCfgTx.xPhyMsg.puc_preemphasis[4];
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[5] << 8;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[6] << 16;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[7] << 24;
		gpbr_write(GPBR9, ul_gpbr_value);

		ul_gpbr_value = xAppPhyCfgTx.xPhyMsg.puc_preemphasis[8];
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[9] << 8;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[10] << 16;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[11] << 24;
		gpbr_write(GPBR10, ul_gpbr_value);

		ul_gpbr_value = xAppPhyCfgTx.xPhyMsg.puc_preemphasis[12];
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[13] << 8;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[14] << 16;
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[15] << 24;
		gpbr_write(GPBR11, ul_gpbr_value);

		ul_gpbr_value = xAppPhyCfgTx.xPhyMsg.puc_preemphasis[16];
		ul_gpbr_value += xAppPhyCfgTx.xPhyMsg.puc_preemphasis[17] << 8;
		gpbr_write(GPBR12, ul_gpbr_value);
	}
#endif
}

/**
 * Get configuration parameters from GPBR
 */
static uint8_t load_config(void)
{
	uint32_t ul_gpbr_value;
	uint8_t uc_start_mode;

	ul_gpbr_value = gpbr_read(GPBR0);
	uc_start_mode = ul_gpbr_value & 0x0F;
	if ((uc_start_mode == PHY_APP_CMD_MENU_START_MODE) || (uc_start_mode == PHY_APP_CMD_TX_START_MODE)) {
		xAppPhyCfgTx.xPhyMsg.uc_mod_type = (enum mod_types)((ul_gpbr_value >> 4) & 0x07);
		xAppPhyCfgTx.xPhyMsg.uc_mod_scheme = (enum mod_schemes)((ul_gpbr_value >> 7) & 0x01);
		xAppPhyCfgTx.xPhyMsg.uc_tx_power = (ul_gpbr_value >> 8) & 0x1F;
		xAppPhyCfgTx.xPhyMsg.uc_tx_mode = (ul_gpbr_value >> 13) & 0x03;
		xAppPhyCfgTx.uc_is_random = (ul_gpbr_value >> 15) & 0x01;
		xAppPhyCfgTx.uc_force_no_output = (ul_gpbr_value >> 16) & 0x01;

		ul_gpbr_value = gpbr_read(GPBR1);

		if ((suc_phy_band == ATPL360_WB_CENELEC_A) || (suc_phy_band == ATPL360_WB_CENELEC_B)) {
			xAppPhyCfgTx.xPhyMsg.puc_tone_map[0] = ul_gpbr_value & 0xFF;
		} else { /* ATPL360_WB_FCC / ATPL360_WB_ARIB */
			xAppPhyCfgTx.xPhyMsg.puc_tone_map[0] = ul_gpbr_value & 0xFF;
			xAppPhyCfgTx.xPhyMsg.puc_tone_map[1] = (ul_gpbr_value & 0xFF00) >> 8;
			xAppPhyCfgTx.xPhyMsg.puc_tone_map[2] = (ul_gpbr_value & 0xFF0000) >> 16;
		}

		xAppPhyCfgTx.ul_tx_period = gpbr_read(GPBR2);
		ul_gpbr_value = gpbr_read(GPBR3);
		xAppPhyCfgTx.xPhyMsg.us_data_len = ul_gpbr_value & 0xFFFF;
		xAppPhyCfgTx.uc_tx_impedance = (ul_gpbr_value & 0xFF0000) >> 16;
		xAppPhyCfgTx.uc_tx_auto = (ul_gpbr_value & 0xFF000000) >> 24;

		/* upload the content of data message from flash memroy */
		memcpy(ucv_tx_data_buffer, (uint8_t *)ADDR_APP_PHY_MESSAGE_DATA, xAppPhyCfgTx.xPhyMsg.us_data_len);

#if (SAM4C || SAM4CP || SAM4CM)
		if (suc_phy_band == ATPL360_WB_CENELEC_A) {
			ul_gpbr_value = gpbr_read(GPBR8);
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[0] = ul_gpbr_value & 0xFF;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[1] = (ul_gpbr_value & 0xFF00) >> 8;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[2] = (ul_gpbr_value & 0xFF0000) >> 16;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[3] = (ul_gpbr_value & 0xFF000000) >> 24;
			ul_gpbr_value = gpbr_read(GPBR9);
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[4] = ul_gpbr_value & 0xFF;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[5] = (ul_gpbr_value & 0xFF00) >> 8;
		} else if (suc_phy_band == ATPL360_WB_CENELEC_B) {
			ul_gpbr_value = gpbr_read(GPBR8);
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[0] = ul_gpbr_value & 0xFF;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[1] = (ul_gpbr_value & 0xFF00) >> 8;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[2] = (ul_gpbr_value & 0xFF0000) >> 16;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[3] = (ul_gpbr_value & 0xFF000000) >> 24;
		} else if (suc_phy_band == ATPL360_WB_FCC) {
			ul_gpbr_value = gpbr_read(GPBR8);
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[0] = ul_gpbr_value & 0xFF;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[1] = (ul_gpbr_value & 0xFF00) >> 8;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[2] = (ul_gpbr_value & 0xFF0000) >> 16;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[3] = (ul_gpbr_value & 0xFF000000) >> 24;
			ul_gpbr_value = gpbr_read(GPBR9);
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[4] = ul_gpbr_value & 0xFF;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[5] = (ul_gpbr_value & 0xFF00) >> 8;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[6] = (ul_gpbr_value & 0xFF0000) >> 16;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[7] = (ul_gpbr_value & 0xFF000000) >> 24;
			ul_gpbr_value = gpbr_read(GPBR10);
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[8] = ul_gpbr_value & 0xFF;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[9] = (ul_gpbr_value & 0xFF00) >> 8;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[10] = (ul_gpbr_value & 0xFF0000) >> 16;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[11] = (ul_gpbr_value & 0xFF000000) >> 24;
			ul_gpbr_value = gpbr_read(GPBR11);
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[12] = ul_gpbr_value & 0xFF;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[13] = (ul_gpbr_value & 0xFF00) >> 8;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[14] = (ul_gpbr_value & 0xFF0000) >> 16;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[15] = (ul_gpbr_value & 0xFF000000) >> 24;
			ul_gpbr_value = gpbr_read(GPBR12);
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[16] = ul_gpbr_value & 0xFF;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[17] = (ul_gpbr_value & 0xFF00) >> 8;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[18] = (ul_gpbr_value & 0xFF0000) >> 16;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[19] = (ul_gpbr_value & 0xFF000000) >> 24;
			ul_gpbr_value = gpbr_read(GPBR13);
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[20] = ul_gpbr_value & 0xFF;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[21] = (ul_gpbr_value & 0xFF00) >> 8;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[22] = (ul_gpbr_value & 0xFF0000) >> 16;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[23] = (ul_gpbr_value & 0xFF000000) >> 24;
		} else if (suc_phy_band == ATPL360_WB_ARIB) {
			ul_gpbr_value = gpbr_read(GPBR8);
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[0] = ul_gpbr_value & 0xFF;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[1] = (ul_gpbr_value & 0xFF00) >> 8;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[2] = (ul_gpbr_value & 0xFF0000) >> 16;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[3] = (ul_gpbr_value & 0xFF000000) >> 24;
			ul_gpbr_value = gpbr_read(GPBR9);
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[4] = ul_gpbr_value & 0xFF;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[5] = (ul_gpbr_value & 0xFF00) >> 8;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[6] = (ul_gpbr_value & 0xFF0000) >> 16;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[7] = (ul_gpbr_value & 0xFF000000) >> 24;
			ul_gpbr_value = gpbr_read(GPBR10);
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[8] = ul_gpbr_value & 0xFF;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[9] = (ul_gpbr_value & 0xFF00) >> 8;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[10] = (ul_gpbr_value & 0xFF0000) >> 16;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[11] = (ul_gpbr_value & 0xFF000000) >> 24;
			ul_gpbr_value = gpbr_read(GPBR11);
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[12] = ul_gpbr_value & 0xFF;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[13] = (ul_gpbr_value & 0xFF00) >> 8;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[14] = (ul_gpbr_value & 0xFF0000) >> 16;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[15] = (ul_gpbr_value & 0xFF000000) >> 24;
			ul_gpbr_value = gpbr_read(GPBR12);
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[16] = ul_gpbr_value & 0xFF;
			xAppPhyCfgTx.xPhyMsg.puc_preemphasis[17] = (ul_gpbr_value & 0xFF00) >> 8;
		}
#endif

		/* Set AUTO mode and Impedance */
		printf("set mode,impedance (%u,%u)\r\n", xAppPhyCfgTx.uc_tx_auto, xAppPhyCfgTx.uc_tx_impedance);
		sx_atpl360_desc.set_config(ATPL360_REG_CFG_AUTODETECT_IMPEDANCE, &xAppPhyCfgTx.uc_tx_auto, 1);
		sx_atpl360_desc.set_config(ATPL360_REG_CFG_IMPEDANCE, &xAppPhyCfgTx.uc_tx_impedance, 1);
	} else {
		uc_start_mode = PHY_APP_CMD_DEFAULT_MODE;
	}

	return uc_start_mode;
}

/**
 * Display current information
 */
static void display_config(void)
{
	uint8_t uc_i;
	uint8_t *puc_tone_map;
	uint8_t uc_branch_mode;
	uint8_t uc_impedance_mode;
	uint8_t uc_size;

	sx_atpl360_desc.get_config(ATPL360_REG_CFG_AUTODETECT_IMPEDANCE, &uc_branch_mode, 1, true);
	sx_atpl360_desc.get_config(ATPL360_REG_CFG_IMPEDANCE, &uc_impedance_mode, 1, true);

	printf("get mode,impedance (%u,%u)\r\n", uc_branch_mode, uc_impedance_mode);

	printf("\n\r-- Configuration Info --------------\r\n");
	printf("-I- Tx Level: %u\n\r", (uint32_t)xAppPhyCfgTx.xPhyMsg.uc_tx_power);
	switch (xAppPhyCfgTx.xPhyMsg.uc_mod_type) {
	case MOD_TYPE_BPSK:
		if (xAppPhyCfgTx.xPhyMsg.uc_mod_scheme) {
			printf("-I- Modulation Scheme: Coherent BPSK \n\r");
		} else {
			printf("-I- Modulation Scheme: Differential BPSK \n\r");
		}

		break;

	case MOD_TYPE_QPSK:
		if (xAppPhyCfgTx.xPhyMsg.uc_mod_scheme) {
			printf("-I- Modulation Scheme: Coherent QPSK \n\r");
		} else {
			printf("-I- Modulation Scheme: Differential QPSK \n\r");
		}

		break;

	case MOD_TYPE_8PSK:
		if (xAppPhyCfgTx.xPhyMsg.uc_mod_scheme) {
			printf("-I- Modulation Scheme: Coherent 8PSK \n\r");
		} else {
			printf("-I- Modulation Scheme: Differential 8PSK \n\r");
		}

		break;

	case MOD_TYPE_BPSK_ROBO:
		if (xAppPhyCfgTx.xPhyMsg.uc_mod_scheme) {
			printf("-I- Modulation Scheme: Coherent Robust\n\r");
		} else {
			printf("-I- Modulation Scheme: Differential Robust\n\r");
		}

		break;

	default:
		printf("-I-  Modulation Scheme: ERROR CFG\n\r");
	}
	puc_tone_map = &xAppPhyCfgTx.xPhyMsg.puc_tone_map[0];

	uc_size = spuc_tonemap_size[suc_phy_band - 1];
	printf("-I- Tone Map: ");
	for (uc_i = 0; uc_i < uc_size; uc_i++) {
		if (uc_i > 0) {
			printf(":");
		}

		printf("%02X", *(puc_tone_map + uc_i));
	}
	printf("\r\n");

	uc_size = spuc_numsubbands[suc_phy_band - 1];
	printf("-I- Preemphasis: ");
	for (uc_i = 0; uc_i < uc_size; uc_i++) {
		if (uc_i > 0) {
			printf(":");
		}

		printf("%02X", *(&xAppPhyCfgTx.xPhyMsg.puc_preemphasis[0] + uc_i));
	}
	printf("\r\n");

	if (uc_branch_mode) {
		printf("-I- Branch Mode : Autodetect - ");
	} else {
		printf("-I- Branch Mode : Fixed - ");
	}

	if (uc_impedance_mode == HI_STATE) {
		printf("High Impedance \r\n");
	} else {
		printf("Very Low Impedance \r\n");
	}

	printf("-I- Forced No Output Signal: %u\n\r", xAppPhyCfgTx.uc_force_no_output);
	printf("-I- Time Period: %u\n\r", xAppPhyCfgTx.ul_tx_period);
	printf("-I- Data Len: %u\n\r", (uint32_t)xAppPhyCfgTx.xPhyMsg.us_data_len);

#if ((defined(CONF_MULTIBAND_FCC_CENA) || defined(CONF_MULTIBAND_FCC_CENB)) && defined(SELECT_G3_FCC_PIN))
	if (ioport_get_pin_level(SELECT_G3_FCC_PIN) == 0) {
		/* G3 FCC */
		printf("-I- Workband: G3 FCC\n\r");
	} else {
		/* G3 CEN A */
		printf("-I- Workband: G3 CEN A\n\r");
	}
#endif

	printf(MENU_CONSOLE);
	fflush(stdout);
}

/**
 * Get ID of transmission level.
 */
static void get_transmission_level(void)
{
	uint8_t uc_char;
	uint16_t us_level;

	printf("Enter transmission level using 2 digits [00..31] : ");
	fflush(stdout);
	while (1) {
		while (_read_char((uint8_t *)&uc_char)) {
		}
		printf("%c", uc_char);
		fflush(stdout);
		us_level = (uc_char - 0x30) * 10;

		while (_read_char((uint8_t *)&uc_char)) {
		}

		printf("%c\r\n", uc_char);
		us_level += (uc_char - 0x30);
		if (us_level < 32) {
			printf("->Attenuation level %u ok\r\n", (uint32_t)us_level);
			xAppPhyCfgTx.xPhyMsg.uc_tx_power = us_level;
			save_config(PHY_APP_CMD_MENU_START_MODE);
			printf(MENU_CONSOLE);
			fflush(stdout);
			return;
		} else {
			printf("ERROR: Attenuation level not permitted [0..31]. Try again.\n\r");
		}
	}
}

/**
 * Get scheme of modulation.
 */
static void get_transmission_scheme(void)
{
	uint8_t uc_char;

	puts(MENU_SCHEME);
	fflush(stdout);
	while (1) {
		while (_read_char((uint8_t *)&uc_char)) {
		}

		switch (uc_char) {
		case '0':
			xAppPhyCfgTx.xPhyMsg.uc_mod_type = MOD_TYPE_BPSK;
			xAppPhyCfgTx.xPhyMsg.uc_mod_scheme = MOD_SCHEME_DIFFERENTIAL;
			break;

		case '1':
			xAppPhyCfgTx.xPhyMsg.uc_mod_type = MOD_TYPE_QPSK;
			xAppPhyCfgTx.xPhyMsg.uc_mod_scheme = MOD_SCHEME_DIFFERENTIAL;
			break;

		case '2':
			xAppPhyCfgTx.xPhyMsg.uc_mod_type = MOD_TYPE_8PSK;
			xAppPhyCfgTx.xPhyMsg.uc_mod_scheme = MOD_SCHEME_DIFFERENTIAL;
			break;

		case '3':
			xAppPhyCfgTx.xPhyMsg.uc_mod_type = MOD_TYPE_BPSK_ROBO;
			xAppPhyCfgTx.xPhyMsg.uc_mod_scheme = MOD_SCHEME_DIFFERENTIAL;
			break;

		case '4':
			xAppPhyCfgTx.xPhyMsg.uc_mod_type = MOD_TYPE_BPSK;
			xAppPhyCfgTx.xPhyMsg.uc_mod_scheme = MOD_SCHEME_COHERENT;
			break;

		case '5':
			xAppPhyCfgTx.xPhyMsg.uc_mod_type = MOD_TYPE_QPSK;
			xAppPhyCfgTx.xPhyMsg.uc_mod_scheme = MOD_SCHEME_COHERENT;
			break;

		case '6':
			xAppPhyCfgTx.xPhyMsg.uc_mod_type = MOD_TYPE_8PSK;
			xAppPhyCfgTx.xPhyMsg.uc_mod_scheme = MOD_SCHEME_COHERENT;
			break;

		case '7':
			xAppPhyCfgTx.xPhyMsg.uc_mod_type = MOD_TYPE_BPSK_ROBO;
			xAppPhyCfgTx.xPhyMsg.uc_mod_scheme = MOD_SCHEME_COHERENT;
			break;

		default:
			return;
		}
		save_config(PHY_APP_CMD_MENU_START_MODE);
		printf("->Scheme %c ok\r\n", uc_char);

		printf(MENU_CONSOLE);
		fflush(stdout);
		break;
	}
}

/**
 * Get branch Tx mode.
 */
static void get_branch_mode(void)
{
	uint8_t uc_char;
	uint8_t uc_branch_mode;
	uint8_t uc_impedance_mode;

	puts(MENU_BRANCH_MODE);
	fflush(stdout);

	while (1) {
		while (_read_char((uint8_t *)&uc_char)) {
		}

		switch (uc_char) {
		case '0':
			uc_impedance_mode = HI_STATE;
			uc_branch_mode = 0x01;
			printf("-->Branch Mode: Autodetect OK \n\r");
			break;

		case '1':
			uc_impedance_mode = HI_STATE;
			uc_branch_mode = 0x00;
			printf("-->Branch Mode: High Impedance OK \n\r");
			break;

		case '2':
			uc_impedance_mode = VLO_STATE;
			uc_branch_mode = 0x00;
			printf("-->Branch Mode: Low Impedance OK \n\r");
			break;

		default:
			return;
		}

		xAppPhyCfgTx.uc_tx_auto = uc_branch_mode;
		xAppPhyCfgTx.uc_tx_impedance = uc_impedance_mode;

		printf("set mode,impedance (%u,%u)\r\n", xAppPhyCfgTx.uc_tx_auto, xAppPhyCfgTx.uc_tx_impedance);
		sx_atpl360_desc.set_config(ATPL360_REG_CFG_AUTODETECT_IMPEDANCE, &uc_branch_mode, 1);
		sx_atpl360_desc.set_config(ATPL360_REG_CFG_IMPEDANCE, &uc_impedance_mode, 1);

		save_config(PHY_APP_CMD_MENU_START_MODE);

		printf(MENU_CONSOLE);
		fflush(stdout);
		break;
	}
}

/**
 * Get Tx Output Signal state.
 */
static void get_forced_output(void)
{
	uint8_t uc_char;
	uint8_t uc_force_no_output;

	puts(MENU_NO_OUTPUT);
	fflush(stdout);

	while (1) {
		while (_read_char((uint8_t *)&uc_char)) {
		}

		switch (uc_char) {
		case '0':
			uc_force_no_output = 0;
			printf("-->Forced No Output Signal CLEARED OK \n\r");
			break;

		case '1':
			uc_force_no_output = 1;
			printf("-->Forced No Output Signal SET OK \n\r");
			break;

		default:
			printf("\r\n->ERROR Invalid char \r\n");
			printf(MENU_CONSOLE);
			fflush(stdout);
			return;
		}

		xAppPhyCfgTx.uc_force_no_output = uc_force_no_output;

		save_config(PHY_APP_CMD_MENU_START_MODE);

		printf(MENU_CONSOLE);
		fflush(stdout);
		break;
	}
}

/**
 * Fill data message for tone map.
 */
static void get_tone_map(void)
{
	uint16_t uc_i;
	uint8_t uc_char;
	uint8_t *p_data_buf;
	uint8_t uc_tone_map_len;

	if (suc_phy_band == ATPL360_WB_CENELEC_A) {
		printf("Please enter value for tone map (from 01 to 3F): ");
	} else if (suc_phy_band == ATPL360_WB_CENELEC_B) {
		printf("Please enter value for tone map (from 01 to 0F): ");
	} else if (suc_phy_band == ATPL360_WB_FCC) {
		printf("Please enter value for tone map (from 000001 to FFFFFF): ");
	} else if (suc_phy_band == ATPL360_WB_ARIB) {
		printf("Please enter value for tone map (from 000001 to 03FFFF): ");
	}

	uc_tone_map_len = spuc_tonemap_size[suc_phy_band - 1];

	/* Asgin pointer to tx data buffer */
	p_data_buf = &xAppPhyCfgTx.xPhyMsg.puc_tone_map[0];

	fflush(stdout);

	while (1) {
		for (uc_i = 0; uc_i < uc_tone_map_len * 2 + 1; uc_i++) {
			while (_read_char((uint8_t *)&uc_char)) {
			}

			if (uc_char == 0x0D) {
				printf("\r\n->Manual message ready.\r\n");
				printf(MENU_CONSOLE);
				fflush(stdout);
				return;
			} else {
				printf("%c", uc_char);
				fflush(stdout);
				if (uc_char >= 0x30 && uc_char < 0x40) { /* 0 to 9 */
					if (uc_i % 2) {
						*p_data_buf++ += uc_char - 0x30;
					} else {
						*p_data_buf = (uc_char - 0x30) << 4;
					}
				} else if (uc_char >= 0x41 && uc_char < 0x47) {
					if (uc_i % 2) {
						*p_data_buf++ += uc_char - 0x37;
					} else {
						*p_data_buf = (uc_char - 0x37) << 4;
					}
				} else if (uc_char >= 0x61 && uc_char < 0x67) {
					if (uc_i % 2) {
						*p_data_buf++ += uc_char - 0x57;
					} else {
						*p_data_buf = (uc_char - 0x57) << 4;
					}
				} else {
					printf("\r\n->ERROR Invalid char \r\n");
					return;
				}
			}
		}

		save_config(PHY_APP_CMD_MENU_START_MODE);

		printf("\r\n->End: Maximum Length is %d bytes\r\n", uc_tone_map_len);
		fflush(stdout);
	}
}

/**
 * Fill data message for preemphasis
 */
static void get_preemphasis(void)
{
	uint16_t uc_i;
	uint8_t uc_char;
	uint8_t *p_data_buf;
	uint8_t uc_preemphasis_len;

	/* Asgin pointer to tx data buffer */
	p_data_buf = &xAppPhyCfgTx.xPhyMsg.puc_preemphasis[0];
	uc_preemphasis_len = spuc_numsubbands[suc_phy_band - 1];

	printf("Please enter value for tone map (%u bytes default ", uc_preemphasis_len);

	for (uc_i = 0; uc_i < uc_preemphasis_len; uc_i++) {
		printf("00");
	}

	printf("): ");

	fflush(stdout);

	while (1) {
		for (uc_i = 0; uc_i < uc_preemphasis_len * 2 + 1; uc_i++) {
			while (_read_char((uint8_t *)&uc_char)) {
			}

			if (uc_char == 0x0D) {
				printf("\r\n->Manual message ready.\r\n");
				printf(MENU_CONSOLE);
				fflush(stdout);
				return;
			} else {
				printf("%c", uc_char);
				fflush(stdout);
				if (uc_char >= 0x30 && uc_char < 0x40) { /* 0 to 9 */
					if (uc_i % 2) {
						*p_data_buf++ += uc_char - 0x30;
					} else {
						*p_data_buf = (uc_char - 0x30) << 4;
					}
				} else if (uc_char >= 0x41 && uc_char < 0x47) {
					if (uc_i % 2) {
						*p_data_buf++ += uc_char - 0x37;
					} else {
						*p_data_buf = (uc_char - 0x37) << 4;
					}
				} else if (uc_char >= 0x61 && uc_char < 0x67) {
					if (uc_i % 2) {
						*p_data_buf++ += uc_char - 0x57;
					} else {
						*p_data_buf = (uc_char - 0x57) << 4;
					}
				} else {
					printf("\r\n->ERROR Invalid char \r\n");
					return;
				}
			}
		}
		save_config(PHY_APP_CMD_MENU_START_MODE);

		printf("\r\n->End: Maximum Length is %d bytes\r\n", uc_preemphasis_len);
		fflush(stdout);
	}
}

/**
 * Get Period of transmission.
 */
static void get_transmission_period(void)
{
	uint8_t uc_char;
	uint8_t ucv_period[10];
	uint8_t i, c;
	uint32_t ul_mul, ul_unit;

	printf("Enter transmission period in us. (max. 10 digits and value min 2100 us): ");
	fflush(stdout);
	while (1) {
		for (i = 0; i < 10; i++) {
			while (_read_char((uint8_t *)&uc_char)) {
			}

			if (uc_char == 0x0D) {
				xAppPhyCfgTx.ul_tx_period = 0;
				for (c = i; c > 0; c--) {
					ul_mul = (uint32_t)pow(10, (i - c));
					ul_unit = ucv_period[c - 1];
					xAppPhyCfgTx.ul_tx_period += ul_unit * ul_mul;
				}
				save_config(PHY_APP_CMD_MENU_START_MODE);
				printf("\r\n->Transmission period %u us\r\n",
						(uint32_t)xAppPhyCfgTx.ul_tx_period);
				printf(MENU_CONSOLE);
				return;
			} else if ((uc_char >= '0') && (uc_char <= '9')) {
				printf("%c", uc_char);
				fflush(stdout);
				ucv_period[i] = (uc_char - 0x30);
			} else {
				printf("Error. Try again\r\n");
				break;
			}

			fflush(stdout);
		}
	}
}

/**
 * Get Length of data to transmit.
 */
static void get_data_len(void)
{
	uint8_t uc_char;
	uint8_t ucv_len[4];
	uint8_t i, c, uc_unit;
	uint16_t us_mul;
	uint16_t us_max_pdu_len;

	us_max_pdu_len = get_max_psdu_len();

	printf("Enter length of data to transmit in bytes. (max. %d bytes; min. 1 byte): ",
			us_max_pdu_len);
	fflush(stdout);

	xAppPhyCfgTx.xPhyMsg.us_data_len = 0;
	while (1) {
		for (i = 0; i < 4; i++) {
			while (_read_char((uint8_t *)&uc_char)) {
			}

			if (uc_char == 0x0D) {
				xAppPhyCfgTx.xPhyMsg.us_data_len = 0;
				for (c = i; c > 0; c--) {
					us_mul = (uint16_t)pow(10, (i - c));
					uc_unit = ucv_len[c - 1];
					xAppPhyCfgTx.xPhyMsg.us_data_len += uc_unit * us_mul;
				}

				if ((xAppPhyCfgTx.xPhyMsg.us_data_len <= us_max_pdu_len) && (xAppPhyCfgTx.xPhyMsg.us_data_len >= 1)) {
					printf("\r\n->Message Data length %u bytes\r\n",
							(uint32_t)xAppPhyCfgTx.xPhyMsg.us_data_len);
					fflush(stdout);
					return;
				} else {
					printf("\r\n->Error. Invalid Length, try again:\r\n");
					fflush(stdout);
					break;
				}
			} else if ((uc_char >= '0') && (uc_char <= '9')) {
				printf("%c", uc_char);
				fflush(stdout);
				ucv_len[i] = (uc_char - 0x30);
			} else {
				printf("Error. Try again\r\n");
				break;
			}

			save_config(PHY_APP_CMD_MENU_START_MODE);

			fflush(stdout);
		}
	}
}

/**
 * Fill data message in random mode.
 */
static void fill_msg_random(void)
{
	uint8_t *p_data_buf;
	uint16_t us_len;
	uint32_t ul_random_num;

	/* Asgin pointer to tx data buffer */
	p_data_buf = ucv_tx_data_buffer;

	/* init vars */
	us_len = xAppPhyCfgTx.xPhyMsg.us_data_len;

#if (SAM4C)
	/* Configure PMC */
	pmc_enable_periph_clk(ID_TRNG);
	/* Enable TRNG */
	trng_enable(TRNG);
#else
	srand(0xFFFFFF);
#endif
	/* fill message */
	while (us_len) {
#if (SAM4C)
		while ((trng_get_interrupt_status(TRNG) & TRNG_ISR_DATRDY)
				!= TRNG_ISR_DATRDY) {
		}
		ul_random_num = trng_read_output_data(TRNG);
#else
		ul_random_num = rand();
#endif

		*p_data_buf++ = (uint8_t)ul_random_num;
		if (!us_len--) {
			break;
		}

		*p_data_buf++ = (uint8_t)(ul_random_num >> 8);
		if (!us_len--) {
			break;
		}

		*p_data_buf++ = (uint8_t)(ul_random_num >> 16);
		if (!us_len--) {
			break;
		}

		*p_data_buf++ = (uint8_t)(ul_random_num >> 24);
		if (!us_len--) {
			break;
		}
	}
}

/**
 * Fill data message in fixed mode.
 */
static void fill_msg_fixed(void)
{
	uint8_t uc_i;
	uint8_t *p_data_buf;
	uint16_t us_len;

	/* Assign pointer to tx data buffer */
	p_data_buf = ucv_tx_data_buffer;

	/* Get length in multiples of 8 bytes  */
	us_len = xAppPhyCfgTx.xPhyMsg.us_data_len >> 3;
	if (xAppPhyCfgTx.xPhyMsg.us_data_len & 0x07) {
		us_len++;
	}
	us_len <<= 3;

	/* fill message */
	uc_i = 0;
	while (us_len--) {
		*p_data_buf++ = 0x30 + uc_i++;
		if (uc_i == 10) {
			uc_i = 0;
		}
	}

	/* store the content of message in flash memory */
	flash_unlock((uint32_t)ADDR_APP_PHY_MESSAGE_DATA,
			(uint32_t)ADDR_APP_PHY_MESSAGE_DATA	+ us_len, 0, 0);
	flash_erase_page((uint32_t)ADDR_APP_PHY_MESSAGE_DATA, IFLASH_ERASE_PAGES_8);
	flash_write((uint32_t)ADDR_APP_PHY_MESSAGE_DATA, ucv_tx_data_buffer, us_len, 0);

	printf("->Fixed message ready\r\n");
	printf(MENU_CONSOLE);
	fflush(stdout);
}

/**
 * Fill data message in manual mode (hex).
 */
static void fill_msg_manual_hex(void)
{
	uint16_t uc_i;
	uint8_t uc_char;
	uint8_t *p_data_buf;
	uint16_t us_max_pdu_len;
	uint16_t us_len;

	uint8_t uc_byte_content = 0x00;
	uint8_t uc_byte_upper_4_bits = 0x01; /* indicate if we are processing upper 4 bits of the byte */

	us_max_pdu_len = get_max_psdu_len();
	printf(
			"Enter bytes data of message to transmit in hex format, i.e. 000102030405060708090A0B0C0D0E0F10... (max. %d bytes; min. 1 byte): ",
			us_max_pdu_len);
	fflush(stdout);

	/* Asign pointer to tx data buffer */
	p_data_buf = ucv_tx_data_buffer;

	xAppPhyCfgTx.xPhyMsg.us_data_len = 0;
	while (1) {
		for (uc_i = 0; uc_i < us_max_pdu_len; uc_i++) {
			while (_read_char((uint8_t *)&uc_char)) {
			}

			if (uc_char == 0x0D) {
				/* Get length in multiples of 8 bytes  */
				us_len = xAppPhyCfgTx.xPhyMsg.us_data_len >> 3;
				if (xAppPhyCfgTx.xPhyMsg.us_data_len & 0x07) {
					us_len++;
				}
				us_len <<= 3;

				/* store the content of message in flash memory */
				flash_unlock((uint32_t)ADDR_APP_PHY_MESSAGE_DATA,
						(uint32_t)ADDR_APP_PHY_MESSAGE_DATA	+ us_len, 0, 0);
				flash_erase_page((uint32_t)ADDR_APP_PHY_MESSAGE_DATA, IFLASH_ERASE_PAGES_8);
				flash_write((uint32_t)ADDR_APP_PHY_MESSAGE_DATA, ucv_tx_data_buffer, us_len, 0);

				printf("\r\n->Manual message ready.\r\n");
				printf(MENU_CONSOLE);
				fflush(stdout);
				return;
			} else {
				printf("%c", uc_char);
				fflush(stdout);
				switch (uc_char) {
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					if (uc_byte_upper_4_bits) {
						uc_byte_content += (uc_char - 0x30) * 16;
						uc_byte_upper_4_bits = 0x00;
					} else {
						uc_byte_content += (uc_char - 0x30);
						*p_data_buf++ = uc_byte_content;
						xAppPhyCfgTx.xPhyMsg.us_data_len++;
						uc_byte_upper_4_bits = 0x01;
						uc_byte_content = 0x00;
					}

					break;

				case 'a':
				case 'A':
					if (uc_byte_upper_4_bits) {
						uc_byte_content += 10 * 16;
						uc_byte_upper_4_bits = 0x00;
					} else {
						uc_byte_content += 10;
						*p_data_buf++ = uc_byte_content;
						xAppPhyCfgTx.xPhyMsg.us_data_len++;
						uc_byte_upper_4_bits = 0x01;
						uc_byte_content = 0x00;
					}

					break;

				case 'b':
				case 'B':
					if (uc_byte_upper_4_bits) {
						uc_byte_content += 11 * 16;
						uc_byte_upper_4_bits = 0x00;
					} else {
						uc_byte_content += 11;
						*p_data_buf++ = uc_byte_content;
						xAppPhyCfgTx.xPhyMsg.us_data_len++;
						uc_byte_upper_4_bits = 0x01;
						uc_byte_content = 0x00;
					}

					break;

				case 'c':
				case 'C':
					if (uc_byte_upper_4_bits) {
						uc_byte_content += 12 * 16;
						uc_byte_upper_4_bits = 0x00;
					} else {
						uc_byte_content += 12;
						*p_data_buf++ = uc_byte_content;
						xAppPhyCfgTx.xPhyMsg.us_data_len++;
						uc_byte_upper_4_bits = 0x01;
						uc_byte_content = 0x00;
					}

					break;

				case 'd':
				case 'D':
					if (uc_byte_upper_4_bits) {
						uc_byte_content += 13 * 16;
						uc_byte_upper_4_bits = 0x00;
					} else {
						uc_byte_content += 13;
						*p_data_buf++ = uc_byte_content;
						xAppPhyCfgTx.xPhyMsg.us_data_len++;
						uc_byte_upper_4_bits = 0x01;
						uc_byte_content = 0x00;
					}

					break;

				case 'e':
				case 'E':
					if (uc_byte_upper_4_bits) {
						uc_byte_content += 14 * 16;
						uc_byte_upper_4_bits = 0x00;
					} else {
						uc_byte_content += 14;
						*p_data_buf++ = uc_byte_content;
						xAppPhyCfgTx.xPhyMsg.us_data_len++;
						uc_byte_upper_4_bits = 0x01;
						uc_byte_content = 0x00;
					}

					break;

				case 'f':
				case 'F':
					if (uc_byte_upper_4_bits) {
						uc_byte_content += 15 * 16;
						uc_byte_upper_4_bits = 0x00;
					} else {
						uc_byte_content += 15;
						*p_data_buf++ = uc_byte_content;
						xAppPhyCfgTx.xPhyMsg.us_data_len++;
						uc_byte_upper_4_bits = 0x01;
						uc_byte_content = 0x00;
					}

					break;

				default:
					break;
				}
				fflush(stdout);
			}
		}
		printf("\r\n->End: Maximum Length is %d bytes\r\n", us_max_pdu_len);
		fflush(stdout);
	}
}

/**
 * Fill data message in manual mode.
 */
static void fill_msg_manual(void)
{
	uint16_t uc_i;
	uint8_t uc_char;
	uint8_t *p_data_buf;
	uint16_t us_max_pdu_len;
	uint16_t us_len;

	us_max_pdu_len = get_max_psdu_len();
	printf("Enter data message to transmit (max. %d bytes; min. 1 byte): ", us_max_pdu_len);
	fflush(stdout);

	/* Asgin pointer to tx data buffer */
	p_data_buf = ucv_tx_data_buffer;

	xAppPhyCfgTx.xPhyMsg.us_data_len = 0;
	while (1) {
		for (uc_i = 0; uc_i < us_max_pdu_len; uc_i++) {
			while (_read_char((uint8_t *)&uc_char)) {
			}

			if (uc_char == 0x0D) {
				/* Get length in multiples of 8 bytes  */
				us_len = xAppPhyCfgTx.xPhyMsg.us_data_len >> 3;
				if (xAppPhyCfgTx.xPhyMsg.us_data_len & 0x07) {
					us_len++;
				}
				us_len <<= 3;

				/* set header type to generic message */
				xAppPhyCfgTx.xPhyMsg.puc_data_buf[0] = 0;

				/* store the content of message in flash memory */
				flash_unlock((uint32_t)ADDR_APP_PHY_MESSAGE_DATA,
						(uint32_t)ADDR_APP_PHY_MESSAGE_DATA	+ us_len, 0, 0);
				flash_erase_page((uint32_t)ADDR_APP_PHY_MESSAGE_DATA, IFLASH_ERASE_PAGES_8);
				flash_write((uint32_t)ADDR_APP_PHY_MESSAGE_DATA, ucv_tx_data_buffer, us_len, 0);

				printf("\r\n->Manual message ready.\r\n");
				printf(MENU_CONSOLE);
				fflush(stdout);
				return;
			} else {
				printf("%c", uc_char);
				fflush(stdout);
				*p_data_buf++ = uc_char;
				xAppPhyCfgTx.xPhyMsg.us_data_len++;
				fflush(stdout);
			}
		}
		printf("\r\n->End: Maximum Length is %d bytes\r\n", us_max_pdu_len);
		fflush(stdout);
	}
}

/**
 * Get Transmission Data.
 */
static void get_transmission_data(void)
{
	uint8_t uc_char;

	puts(MENU_DATA_MODE);
	fflush(stdout);

	while (1) {
		while (_read_char((uint8_t *)&uc_char)) {
		}

		switch (uc_char) {
		case '0':
			printf("%c\r\n", uc_char);
			get_data_len();
			xAppPhyCfgTx.uc_is_random = 1;
			fill_msg_random();
			break;

		case '1':
			printf("%c\r\n", uc_char);
			xAppPhyCfgTx.uc_is_random = 0;
			get_data_len();
			fill_msg_fixed();
			break;

		case '2':
			printf("%c\r\n", uc_char);
			xAppPhyCfgTx.uc_is_random = 0;
			fill_msg_manual();
			break;

		case '3':
			printf("%c\r\n", uc_char);
			xAppPhyCfgTx.uc_is_random = 0;
			fill_msg_manual_hex();
			break;

		default:
			return;
		}
		break;
	}

	save_config(PHY_APP_CMD_MENU_START_MODE);
}

/**
 * \brief Main code entry point.0
 */
int main(void)
{
	uint8_t uc_start_mode;
	EIndication input;

	/* Prepare the hardware */
	prvSetupHardware();

	/* Init process timers */
	g_ms_counter = 0;
	initTimer1ms();

	puts(STRING_HEADER);
	app_init();

	/* Configuration management */
	xAppPhyCfgTx.xPhyMsg.puc_data_buf = ucv_tx_data_buffer;

	uc_start_mode = load_config();
	if (uc_start_mode == PHY_APP_CMD_DEFAULT_MODE) {
		uint8_t uc_size;

		sx_atpl360_desc.get_config(ATPL360_REG_CFG_AUTODETECT_IMPEDANCE, &xAppPhyCfgTx.uc_tx_auto, 1, true);
		sx_atpl360_desc.get_config(ATPL360_REG_CFG_IMPEDANCE, &xAppPhyCfgTx.uc_tx_impedance, 1, true);

		xAppPhyCfgTx.xPhyMsg.uc_delimiter_type = DT_SOF_NO_RESP;
		xAppPhyCfgTx.xPhyMsg.uc_mod_scheme = MOD_SCHEME_DIFFERENTIAL;
		xAppPhyCfgTx.xPhyMsg.uc_mod_type = MOD_TYPE_BPSK;
		if (suc_phy_band == ATPL360_WB_CENELEC_A) {
			xAppPhyCfgTx.xPhyMsg.puc_tone_map[2] = 0x00;
			xAppPhyCfgTx.xPhyMsg.puc_tone_map[1] = 0x00;
			xAppPhyCfgTx.xPhyMsg.puc_tone_map[0] = 0x3F;
		} else if (suc_phy_band == ATPL360_WB_CENELEC_B) {
			xAppPhyCfgTx.xPhyMsg.puc_tone_map[2] = 0x00;
			xAppPhyCfgTx.xPhyMsg.puc_tone_map[1] = 0x00;
			xAppPhyCfgTx.xPhyMsg.puc_tone_map[0] = 0x0F;
		} else if (suc_phy_band == ATPL360_WB_FCC) {
			xAppPhyCfgTx.xPhyMsg.puc_tone_map [2] = 0xff;
			xAppPhyCfgTx.xPhyMsg.puc_tone_map [1] = 0xff;
			xAppPhyCfgTx.xPhyMsg.puc_tone_map [0] = 0xff;
			xAppPhyCfgTx.xPhyMsg.uc_2_rs_blocks = 0;
		} else { /* ATPL360_WB_ARIB */
			xAppPhyCfgTx.xPhyMsg.puc_tone_map [2] = 0x03;
			xAppPhyCfgTx.xPhyMsg.puc_tone_map [1] = 0xff;
			xAppPhyCfgTx.xPhyMsg.puc_tone_map [0] = 0xff;
			xAppPhyCfgTx.xPhyMsg.uc_2_rs_blocks = 0;
		}

		xAppPhyCfgTx.xPhyMsg.uc_pdc = 0;
		uc_size = spuc_numsubbands[ATPL360_WB_FCC - 1];
		memset(xAppPhyCfgTx.xPhyMsg.puc_preemphasis, 0x00, uc_size);
		xAppPhyCfgTx.xPhyMsg.uc_tx_mode = TX_MODE_ABSOLUTE | TX_MODE_FORCED;
		xAppPhyCfgTx.xPhyMsg.us_data_len = 133;
		xAppPhyCfgTx.xPhyMsg.ul_tx_time = 5400;

		xAppPhyCfgTx.ul_tx_period = 5400;
		xAppPhyCfgTx.uc_is_random = 1;

		xAppPhyCfgTx.uc_force_no_output = 0;

		/* Fill Data of message: Fixed by default */
		save_config(PHY_APP_CMD_MENU_START_MODE);
		fill_msg_fixed();
		machine.curState = main_menu_show;
	} else if (uc_start_mode == PHY_APP_CMD_TX_START_MODE) {
		/* execute test */
		printf("Press 'x' to finish transmission...\r\n");
		machine.curState = executing;
	}

	xAppPhyCfgTx.uc_tx_result_flag = TX_RESULT_SUCCESS;

	while (1) {
		/* Reset watchdog */
#if (!PIC32CX)
		WDT->WDT_CR = WDT_CR_KEY_PASSWD | WDT_CR_WDRSTT;
#else
		DWDT->WDT0_CR = WDT0_CR_KEY_PASSWD | WDT0_CR_WDRSTT;
		DWDT->WDT1_CR = WDT1_CR_KEY_PASSWD | WDT1_CR_WDRSTT;
#endif

		input = processInput();
		(machine.curState)(&machine, input);
		if (machine.curState == executing) {
			/* blink led 0 */
			if (uc_led_swap) {
				uc_led_swap = false;
#if (BOARD != PIC32CXMTSH_DB) && (BOARD != PIC32CXMTC_DB)
	#if  (BOARD == SAM4CMS_DB)
				LED_Toggle(LED4);
	#else
				LED_Toggle(LED0);
	#endif
#else
#ifdef CONF_BOARD_LCD_EN
				if (b_led_swap_on) {
					cl010_clear_icon(CL010_ICON_PHASE_1);
					b_led_swap_on = false;
				} else {
					cl010_show_icon(CL010_ICON_PHASE_1);
					b_led_swap_on = true;
				}
#endif
#endif
			}
		}

		/* Check ATPL360 pending events */
		atpl360_handle_events();
	}
}

/**
 * \brief Configure the hardware.
 */
static void prvSetupHardware(void)
{
#ifdef CONF_BOARD_LCD_EN
	status_code_t status;
#endif
	uint8_t uc_num_blinks;

	SystemCoreClockUpdate();

	/* ASF function to setup clocking. */
	sysclk_init();

	/* Ensure all priority bits are assigned as preemption priority bits. */
	NVIC_SetPriorityGrouping(__NVIC_PRIO_BITS);

	/* Atmel library function to setup for the evaluation kit being used. */
	board_init();

#if (BOARD != SAME70_XPLAINED) && (BOARD != PL360BN)
	#if !PIC32CX
	/* Initialize flash: 6 wait states for flash writing. */
	flash_init(FLASH_ACCESS_MODE_128, CHIP_FLASH_WRITE_WAIT_STATE);
	#endif
#endif

	/* UART debug */
	configure_dbg_console();

#if CONF_PCK_EXT == CONF_PCK_MCK_EN
	ioport_set_pin_mode(PCK_EXT_GPIO, PCK_EXT_FLAG);
	ioport_disable_pin(PCK_EXT_GPIO);
	pmc_disable_pck(PCK_EXT_PCK_ID);
	pmc_switch_pck_to_mainck(PCK_EXT_PCK_ID, 0);
	pmc_enable_pck(PCK_EXT_PCK_ID);
#elif CONF_PCK_EXT == CONF_PCK_SLCK_EN
	ioport_set_pin_mode(PCK_EXT_GPIO, PCK_EXT_FLAG);
	ioport_disable_pin(PCK_EXT_GPIO);
	pmc_disable_pck(PCK_EXT_PCK_ID);
	pmc_switch_pck_to_sclk(PCK_EXT_PCK_ID, 0);
	pmc_enable_pck(PCK_EXT_PCK_ID);
#endif

	/* LED signalling */
	for (uc_num_blinks = 0; uc_num_blinks < LED_NUM_BLINKS_RST; uc_num_blinks++) {
		delay_ms(100);
#if (BOARD != PIC32CXMTSH_DB) && (BOARD != PIC32CXMTC_DB)
	#if  (BOARD == SAM4CMS_DB)
		LED_Toggle(LED4);
	#else
		LED_Toggle(LED0);
	#endif
	#if (BOARD != SAME70_XPLAINED) && (BOARD != SAMG55_XPLAINED_PRO) && (BOARD != SAM4CMS_DB)
		LED_Toggle(LED1);
	#endif
#else
	#ifdef CONF_BOARD_LCD_EN
		if (b_led_swap_on) {
			cl010_clear_icon(CL010_ICON_PHASE_1);
			b_led_swap_on = false;
		} else {
			cl010_show_icon(CL010_ICON_PHASE_1);
			b_led_swap_on = true;
		}
	#endif
#endif
	}

#ifdef CONF_BOARD_LCD_EN
#if BOARD == ATPL360ASB
	/* Initialize the vim878 LCD glass component. */
	status = vim878_init();
	if (status != STATUS_OK) {
		puts("-- LCD Initialization fails! --\r\n");
		while (1) {
		}
	}

	vim878_set_contrast(1);
	vim878_clear_all();
	vim878_show_text((const uint8_t *)"phycon");
#elif BOARD == ATPL360AMB
	status = (status_code_t)c0216CiZ_init();
	if (status != STATUS_OK) {
		puts("-- LCD Initialization fails! --\r\n");
		while (1) {
		}
	}

	c0216CiZ_set_cursor(C0216CiZ_LINE_UP, 0);
	c0216CiZ_show((const char *)"ATPL360AMB    G3");
	c0216CiZ_set_cursor(C0216CiZ_LINE_DOWN, 0);
	c0216CiZ_show((const char *)"Phy TX console");
#elif BOARD == ATPL360MB
	status = (status_code_t)c0216CiZ_init();
	if (status != STATUS_OK) {
		puts("-- LCD Initialization fails! --\r\n");
		while (1) {
		}
	}

	c0216CiZ_set_cursor(C0216CiZ_LINE_UP, 0);
	c0216CiZ_show((const char *)"ATPL360MB    G3");
	c0216CiZ_set_cursor(C0216CiZ_LINE_DOWN, 0);
	c0216CiZ_show((const char *)"Phy TX console");
#elif (BOARD == PIC32CXMTSH_DB) || (BOARD == PIC32CXMTC_DB)
	/* Initialize the CL010 LCD glass component. */
	status = cl010_init();
	if (status != STATUS_OK) {
		puts("-- LCD Initialization fails! --\r\n");
		while (1) {
		}
	}

	cl010_clear_all();
	cl010_show_icon(CL010_ICON_MICROCHIP);
	cl010_show_numeric_string(CL010_LINE_DOWN, (const uint8_t *)"0000360");
	cl010_show_icon(CL010_ICON_DOT_5);
	cl010_show_icon(CL010_ICON_P_PLUS);
	cl010_show_icon(CL010_ICON_P_MINUS);
	b_led_swap_on = false;
#else
#error ERROR in board definition
#endif
#endif
}

void main_menu_show(TStateMachine *sm, EIndication input)
{
	UNUSED(input);
	/* Console Application menu */
	puts(MENU_HEADER);
	printf(MENU_CONSOLE);
	fflush(stdout);
	sm->curState = main_menu_wait;
}

void main_menu_wait(TStateMachine *sm, EIndication input)
{
	switch (input) {
	case MAIN_MENU_OPT0:
		sm->curState = opt0_menu;
		break;

	case MAIN_MENU_OPT1:
		sm->curState = opt1_menu;
		break;

	case MAIN_MENU_OPT2:
		sm->curState = opt2_menu;
		break;

	case MAIN_MENU_OPT3:
		sm->curState = opt3_menu;
		break;

	case MAIN_MENU_OPT4:
		sm->curState = opt4_menu;
		break;

	case MAIN_MENU_OPT5:
		sm->curState = opt5_menu;
		break;

	case MAIN_MENU_OPT6:
		sm->curState = opt6_menu;
		break;

	case MAIN_MENU_OPT7:
		sm->curState = opt7_menu;
		break;

	case MAIN_MENU_VIEW_CFG:
		sm->curState = show_cfg;
		break;

	case MAIN_MENU_EXECUTE:
		/* save configuration parameters */
		save_config(PHY_APP_CMD_TX_START_MODE);
		printf("Press 'x' to finish transmission...\r\n");
		sm->curState = executing;
		break;

	case UNKNOW_CHAR:
		sm->curState = main_menu_show;
		break;

	default: /* DO NOTHING */
		break;
	}
}

void opt0_menu(TStateMachine *sm, EIndication input)
{
	UNUSED(input);
	get_transmission_level();
	sm->curState = main_menu_show;
}

void opt1_menu(TStateMachine *sm, EIndication input)
{
	UNUSED(input);
	get_transmission_scheme();
	sm->curState = main_menu_show;
}

void opt2_menu(TStateMachine *sm, EIndication input)
{
	UNUSED(input);
	get_transmission_period();
	sm->curState = main_menu_show;
}

void opt3_menu(TStateMachine *sm, EIndication input)
{
	UNUSED(input);
	get_transmission_data();
	sm->curState = main_menu_show;
}

void opt5_menu(TStateMachine *sm, EIndication input)
{
	UNUSED(input);
	get_preemphasis();
	sm->curState = main_menu_show;
}

void opt6_menu(TStateMachine *sm, EIndication input)
{
	UNUSED(input);
	get_branch_mode();
	sm->curState = main_menu_show;
}

void opt7_menu(TStateMachine *sm, EIndication input)
{
	UNUSED(input);
	get_forced_output();
	sm->curState = main_menu_show;
}

void opt4_menu(TStateMachine *sm, EIndication input)
{
	UNUSED(input);
	get_tone_map();
	sm->curState = main_menu_show;
}

void show_cfg(TStateMachine *sm, EIndication input)
{
	UNUSED(input);
	display_config();
	sm->curState = main_menu_show;
}

void executing(TStateMachine *sm, EIndication input)
{
	uint32_t ul_gpbr_value;
	/* fflush(stdout); */
	if (input == STOP_TEST) {
		sm->curState = main_menu_show;
		ul_gpbr_value = gpbr_read(GPBR0);
		ul_gpbr_value &= 0xFFFFFFF0;
		ul_gpbr_value |= PHY_APP_CMD_MENU_START_MODE;
		gpbr_write(GPBR0, ul_gpbr_value);
		xAppPhyCfgTx.uc_tx_result_flag = TX_RESULT_NO_TX;
	}

	/* else maintain state */

	if (xAppPhyCfgTx.uc_tx_result_flag != TX_RESULT_PROCESS) {
		run_tx_task();
		if (xAppPhyCfgTx.uc_is_random) {
			fill_msg_random();
		}
	}
}

EIndication processInput(void)
{
	uint8_t uc_choice;

	EIndication e_result = NO_CHAR;

	if (!_read_char((uint8_t *)&uc_choice)) {
		switch (uc_choice) {
		case '0':
			e_result = MAIN_MENU_OPT0;
			break;

		case '1':
			e_result = MAIN_MENU_OPT1;
			break;

		case '2':
			e_result = MAIN_MENU_OPT2;
			break;

		case '3':
			e_result = MAIN_MENU_OPT3;
			break;

		case '4':
			e_result = MAIN_MENU_OPT4;
			break;

		case '5':
			e_result = MAIN_MENU_OPT5;
			break;

		case '6':
			e_result = MAIN_MENU_OPT6;
			break;

		case '7':
			e_result = MAIN_MENU_OPT7;
			break;

		case 'v':
		case 'V':
			e_result = MAIN_MENU_VIEW_CFG;
			break;

		case 'e':
		case 'E':
			e_result = MAIN_MENU_EXECUTE;
			break;

		case 'x':
		case 'X':
			e_result = STOP_TEST;
			break;

		case 0x00:
			e_result = NO_CHAR;
			break;

		default:
			e_result = UNKNOW_CHAR;
			break;
		}
	}

	return e_result;
}
