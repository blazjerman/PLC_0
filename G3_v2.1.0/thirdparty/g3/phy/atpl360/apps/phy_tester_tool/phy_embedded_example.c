/**
 * \file
 *
 * \brief PHY_TESTER_TOOL : ATMEL PLC Phy Tester Example
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
 *  \mainpage ATMEL PLC Phy Tester Example
 *
 *  \section Purpose
 *
 *  The Phy Tester Tool example demonstrates how to use the PRIME PHY layer on
 * PLC boards.
 *
 *  \section Requirements
 *
 *  This package should be used with any PLC board on which there is PLC
 * hardware dedicated.
 *
 *  \section Description
 *
 *  This application will configure the PRIME PHY and its serial interface to
 * communicate with
 * ATMEL PLC Phy Tester Tool and test PLC transmission/reception processes.
 *
 *  \section Usage
 *
 *  The tool is ready for set up the device configuration and perform some
 * communications test.
 *
 */

/* Atmel library includes. */
#include "asf.h"
#include "board.h"
#include "conf_project.h"
#include "conf_usi.h"
#include "conf_pplc_if.h"

/* Function declarations */
static void prvSetupHardware(void);
void initTimer1ms(void);
void __user_low_level_init(void);

/* ATPL360 descriptor definition */
atpl360_descriptor_t sx_atpl360_desc;

#define COUNT_MS_SWAP_LED       500
#define COUNT_MS_IND_LED        50

#ifdef PL360_BIN_ADDR_FIXED
#  define PL360_BIN_ADDR  0x010E0000
#  define PL360_BIN_SIZE  (64 * 1024)
#endif

static uint32_t ul_count_ms = COUNT_MS_SWAP_LED;
static uint32_t ul_ind_count_ms = 0;
static bool b_led_swap = false;
static bool b_ind_led_swap = false;
#if (BOARD == PIC32CXMTSH_DB) || (BOARD == PIC32CXMTC_DB)
#ifdef CONF_BOARD_LCD_EN
static bool b_led_swap_on = false;
#endif
#endif

static uint8_t suc_err_unexpected;
static uint8_t suc_err_critical;
static uint8_t suc_err_reset;
static uint8_t suc_err_none;
static uint8_t sb_exception_pend;

/* G3 Band identifier */
uint8_t suc_phy_band;

#if (!PIC32CX)
#define ID_TC_1MS               ID_TC3
#define TC_1MS                  TC1
#define TC_1MS_CHN              0
#define TC_1MS_IRQn             TC3_IRQn
#define TC_1MS_Handler          TC3_Handler
#else
#define ID_TC_1MS               ID_TC1_CHANNEL0
#define TC_1MS                  TC1
#define TC_1MS_CHN              0
#define TC_1MS_IRQn             TC1_CHANNEL0_IRQn
#define TC_1MS_Handler          TC1_CHANNEL0_Handler
#endif

#ifdef CONF_BOARD_UART_CONSOLE
#define PRINT_DBG(x)            printf x
#else
#define PRINT_DBG(x)
#endif

#ifdef CONF_BOARD_UART_CONSOLE
#define STRING_EOL    "\r"
#define STRING_HEADER "\r\n-- Microchip PLC Phy Tester Tool Application --" \
	"\r\n-- "BOARD_NAME " --" \
	"\r\n-- Compiled: "__DATE__ " "__TIME__ " --"STRING_EOL
#endif

/**
 * \brief Set PL360 configuration
 */
static void _set_pl360_configuration(void)
{
	uint8_t uc_value;

	/* Configure Band in PL360 Host Controller */
	sx_atpl360_desc.set_config(ATPL360_HOST_BAND_ID, &suc_phy_band, 1);

	/* Configure Coupling and TX parameters */
	pl360_g3_coup_tx_config(&sx_atpl360_desc, suc_phy_band);

	/* Disable AUTO mode and set VLO behavior by default in order to maximize signal level in anycase */
	uc_value = 0;
	sx_atpl360_desc.set_config(ATPL360_REG_CFG_AUTODETECT_IMPEDANCE, &uc_value, 1);
	uc_value = 2;
	sx_atpl360_desc.set_config(ATPL360_REG_CFG_IMPEDANCE, &uc_value, 1);
}

/** @brief	Interrupt handler for Timer 3
 *
 * Handler for Timer 3 */
void TC_1MS_Handler(void)
{
	/* Clear status bit to acknowledge interrupt */
	tc_get_status(TC_1MS, TC_1MS_CHN);

	/* update count ms */
	if (!ul_count_ms--) {
		ul_count_ms = COUNT_MS_SWAP_LED;
		b_led_swap = true;
	}

	/* indication: update count ms */
	if (ul_ind_count_ms) {
		if (!--ul_ind_count_ms) {
			b_ind_led_swap = true;
		}
	}
}

/** @brief	Init Timer interrupt (1ms)
 *
 * Initialize 1mSec timer 3 interrupt */
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

	/** Start the timer. TC1, channel 0 = TC3 */
	tc_start(TC_1MS, TC_1MS_CHN);
}

#ifdef CONF_BOARD_UART_CONSOLE

/**
 *  Configure UART console.
 */
static void configure_dbg_console(void)
{
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
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}

#endif

/**
 * \brief Configure the hardware.
 */
static void prvSetupHardware(void)
{
#ifdef CONF_PCK_OUTPUT
	struct genclk_config gcfg;

	/* Configure specific CLKOUT pin */
	ioport_set_pin_mode(GCLK_PIN, GCLK_PIN_MUX);
	ioport_disable_pin(GCLK_PIN);

	/* Configure the output clock source and frequency */
	genclk_config_defaults(&gcfg, GCLK_ID);
	genclk_config_set_source(&gcfg, GENCLK_PCK_SRC_SLCK_XTAL);
	genclk_config_set_divider(&gcfg, GENCLK_PCK_PRES_1);
	genclk_enable(&gcfg, GCLK_ID);
#endif

	/* ASF function to setup clocking. */
	sysclk_init();

	/* Ensure all priority bits are assigned as preemption priority bits. */
	NVIC_SetPriorityGrouping(__NVIC_PRIO_BITS);

	/* Atmel library function to setup for the evaluation kit being used. */
	board_init();
}

/**
 * \brief Low level initialization called from Reset_Handler, before calling
 * to main and before data initialization. This function should not use
 * initialized data.
 */
void __user_low_level_init(void)
{
#if BOARD == PIC32CXMTSH_DB || BOARD == PIC32CXMTC_DB || BOARD == PIC32CXMTG_EK
	/* Enable coprocessor peripherals to allow access to PIOD */
	uint32_t read_reg;

	/* Assert coprocessor reset and reset its peripheral */
	read_reg = RSTC->RSTC_MR;
	read_reg &= ~(RSTC_MR_KEY_Msk | RSTC_MR_CPEREN | RSTC_MR_CPROCEN);
	read_reg |= RSTC_MR_KEY_PASSWD;
	RSTC->RSTC_MR = read_reg;

	/* Enables Coprocessor Bus Master Clock */
	PMC->PMC_SCER = PMC_SCER_CPBMCK | PMC_SCER_CPKEY_PASSWD;

	/* Set coprocessor clock prescaler */
	read_reg = PMC->PMC_CPU_CKR;
	read_reg &= ~PMC_CPU_CKR_CPPRES_Msk;
	PMC->PMC_CPU_CKR = read_reg | PMC_CPU_CKR_CPPRES(1);

	/*  Wait to PMC_SR_CPMCKRDY = 1 */
	while ((PMC->PMC_SR & PMC_SR_CPMCKRDY) == 0) {
	}

	/* Release coprocessor peripheral reset */
	RSTC->RSTC_MR |= (RSTC_MR_KEY_PASSWD | RSTC_MR_CPEREN);

	/* Set coprocessor clock prescaler */
	read_reg = PMC->PMC_CPU_CKR;
	read_reg &= ~PMC_CPU_CKR_CPPRES_Msk;
	PMC->PMC_CPU_CKR = read_reg | PMC_CPU_CKR_CPPRES(0);

	/*  Wait to PMC_SR_CPMCKRDY = 1 */
	while ((PMC->PMC_SR & PMC_SR_CPMCKRDY) == 0) {
	}
#endif

	/* Critical initialization of PLC pins */
	pplc_if_crit_init();
}

/**
 * \brief Handler to receive data from ATPL360 to serialize to APP.
 */
static void _handler_atpl360_serial_event(uint8_t *px_serial_data, uint16_t us_len)
{
	x_usi_serial_cmd_params_t x_usi_msg;

	x_usi_msg.uc_protocol_type = PROTOCOL_PHY_ATPL2X0;
	x_usi_msg.ptr_buf = px_serial_data;
	x_usi_msg.us_len = us_len;
	usi_send_cmd(&x_usi_msg);
}

/**
 * \brief Handler to receive serial data from APP.
 */
static uint8_t _handler_app_serial_event(uint8_t *px_serial_data, uint16_t us_len)
{
	sx_atpl360_desc.send_addons_cmd(px_serial_data, us_len);
	return USI_STATUS_OK;
}

/**
 * \brief Handler to phy data indicator.
 */
static void _handler_data_indicator(rx_msg_t *px_msg)
{
	(void)px_msg;
	ul_ind_count_ms = COUNT_MS_IND_LED;
#ifdef LED1_GPIO
	LED_On(LED1);
#else
#ifdef CONF_BOARD_LCD_EN
	cl010_show_icon(CL010_ICON_PHASE_2);
#endif
#endif
}

/**
 * \brief Handler to phy exceptions.
 */
static void _handler_exception_event(atpl360_exception_t exception)
{
	switch (exception) {
	case ATPL360_EXCEPTION_UNEXPECTED_SPI_STATUS:
		suc_err_unexpected++;
		break;

	case ATPL360_EXCEPTION_SPI_CRITICAL_ERROR:
		suc_err_critical++;
		break;

	case ATPL360_EXCEPTION_RESET:
		suc_err_reset++;
		break;

	default:
		suc_err_none++;
	}

	sb_exception_pend = true;
}

/**
 * \brief Handler to Sleep Mode resume event.
 */
static void _handler_sleep_mode_resume_event(void)
{
	/* Set PL360 specific configuration from application */
	_set_pl360_configuration();
}

/**
 * \brief Get PL360 binary addressing.
 *
 * \param pul_address   Pointer to store the initial address of PL360 binary data
 *
 * \return Size of PL360 binary file
 */
static uint32_t _get_pl360_bin_addressing(uint32_t *pul_address)
{
	uint32_t ul_bin_addr;
	uint8_t *puc_bin_start;
	uint8_t *puc_bin_end;

#if ((ATPL360_WB != ATPL360_WB_CENELEC_A) && (ATPL360_WB != ATPL360_WB_CENELEC_B) && (ATPL360_WB != ATPL360_WB_FCC) && (ATPL360_WB != ATPL360_WB_ARIB))
  #error ERROR in PHY band definition
#endif

#if defined(CONF_MULTIBAND_FCC_CENA)
  #if ((ATPL360_WB == ATPL360_WB_FCC) || (ATPL360_WB == ATPL360_WB_CENELEC_A))
	/* FCC and CENELEC-A binaries are linked */
	if (suc_phy_band == ATPL360_WB_FCC) {
		/* Select FCC binary */
    #if defined (__CC_ARM)
		extern uint8_t atpl_bin_fcc_start[];
		extern uint8_t atpl_bin_fcc_end[];
		ul_bin_addr = (int)(atpl_bin_fcc_start - 1);
		puc_bin_start = atpl_bin_fcc_start - 1;
		puc_bin_end = atpl_bin_fcc_end;
    #elif defined (__GNUC__)
		extern uint8_t atpl_bin_fcc_start;
		extern uint8_t atpl_bin_fcc_end;
		ul_bin_addr = (int)&atpl_bin_fcc_start;
		puc_bin_start = (uint8_t *)&atpl_bin_fcc_start;
		puc_bin_end = (uint8_t *)&atpl_bin_fcc_end;
    #elif defined (__ICCARM__)
      #pragma section = "P_atpl_bin_fcc"
		extern uint8_t atpl_bin_fcc;
		ul_bin_addr = (int)&atpl_bin_fcc;
		puc_bin_start = __section_begin("P_atpl_bin_fcc");
		puc_bin_end = __section_end("P_atpl_bin_fcc");
    #else
      #error This compiler is not supported for now.
    #endif
	} else { /* ATPL360_WB_CENELEC_A */
		 /* Select CENELEC-A binary */
    #if defined (__CC_ARM)
		extern uint8_t atpl_bin_cena_start[];
		extern uint8_t atpl_bin_cena_end[];
		ul_bin_addr = (int)(atpl_bin_cena_start - 1);
		puc_bin_start = atpl_bin_cena_start - 1;
		puc_bin_end = atpl_bin_cena_end;
    #elif defined (__GNUC__)
		extern uint8_t atpl_bin_cena_start;
		extern uint8_t atpl_bin_cena_end;
		ul_bin_addr = (int)&atpl_bin_cena_start;
		puc_bin_start = (int)&atpl_bin_cena_start;
		puc_bin_end = (int)&atpl_bin_cena_end;
    #elif defined (__ICCARM__)
      #pragma section = "P_atpl_bin_cena"
		extern uint8_t atpl_bin_cena;
		ul_bin_addr = (int)&atpl_bin_cena;
		puc_bin_start = __section_begin("P_atpl_bin_cena");
		puc_bin_end = __section_end("P_atpl_bin_cena");
    #else
      #error This compiler is not supported for now.
    #endif
	}

  #else
    #error Work-band not supported for this board
  #endif
#elif defined(CONF_MULTIBAND_FCC_CENB)
  #if ((ATPL360_WB == ATPL360_WB_FCC) || (ATPL360_WB == ATPL360_WB_CENELEC_B))
	/* FCC and CENELEC-B binaries are linked */
	if (suc_phy_band == ATPL360_WB_FCC) {
		/* Select FCC binary */
    #if defined (__CC_ARM)
		extern uint8_t atpl_bin_fcc_start[];
		extern uint8_t atpl_bin_fcc_end[];
		ul_bin_addr = (int)(atpl_bin_fcc_start - 1);
		puc_bin_start = atpl_bin_fcc_start - 1;
		puc_bin_end = atpl_bin_fcc_end;
    #elif defined (__GNUC__)
		extern uint8_t atpl_bin_fcc_start;
		extern uint8_t atpl_bin_fcc_end;
		ul_bin_addr = (int)&atpl_bin_fcc_start;
		puc_bin_start = (uint8_t *)&atpl_bin_fcc_start;
		puc_bin_end = (uint8_t *)&atpl_bin_fcc_end;
    #elif defined (__ICCARM__)
      #pragma section = "P_atpl_bin_fcc"
		extern uint8_t atpl_bin_fcc;
		ul_bin_addr = (int)&atpl_bin_fcc;
		puc_bin_start = __section_begin("P_atpl_bin_fcc");
		puc_bin_end = __section_end("P_atpl_bin_fcc");
    #else
      #error This compiler is not supported for now.
    #endif
	} else { /* ATPL360_WB_CENELEC_B */
		 /* Select CENELEC-B binary */
    #if defined (__CC_ARM)
		extern uint8_t atpl_bin_cenb_start[];
		extern uint8_t atpl_bin_cenb_end[];
		ul_bin_addr = (int)(atpl_bin_cenb_start - 1);
		puc_bin_start = atpl_bin_cenb_start - 1;
		puc_bin_end = atpl_bin_cenb_end;
    #elif defined (__GNUC__)
		extern uint8_t atpl_bin_cenb_start;
		extern uint8_t atpl_bin_cenb_end;
		ul_bin_addr = (int)&atpl_bin_cenb_start;
		puc_bin_start = (int)&atpl_bin_cenb_start;
		puc_bin_end = (int)&atpl_bin_cenb_end;
    #elif defined (__ICCARM__)
      #pragma section = "P_atpl_bin_cenb"
		extern uint8_t atpl_bin_cenb;
		ul_bin_addr = (int)&atpl_bin_cenb;
		puc_bin_start = __section_begin("P_atpl_bin_cenb");
		puc_bin_end = __section_end("P_atpl_bin_cenb");
    #else
      #error This compiler is not supported for now.
    #endif
	}

  #else
    #error Work-band not supported for this board
  #endif
#else /* #if defined(CONF_MULTIBAND_FCC_CENA) */
	/* Only one binary linked */
    #if defined (__CC_ARM)
	extern uint8_t atpl_bin_start[];
	extern uint8_t atpl_bin_end[];
	ul_bin_addr = (int)(atpl_bin_start - 1);
	puc_bin_start = atpl_bin_start - 1;
	puc_bin_end = atpl_bin_end;
    #elif defined (__GNUC__)
	extern uint8_t atpl_bin_start;
	extern uint8_t atpl_bin_end;
	ul_bin_addr = (int)&atpl_bin_start;
	puc_bin_start = (int)&atpl_bin_start;
	puc_bin_end = (int)&atpl_bin_end;
    #elif defined (__ICCARM__)
    #pragma section = "P_atpl_bin"
	extern uint8_t atpl_bin;
	ul_bin_addr = (int)&atpl_bin;
	puc_bin_start = __section_begin("P_atpl_bin");
	puc_bin_end = __section_end("P_atpl_bin");
    #else
    #error This compiler is not supported for now.
  #endif
#endif /* #if defined(CONF_MULTIBAND_FCC_CENA) */
	*pul_address = ul_bin_addr;
	/* cppcheck-suppress deadpointer */
	return ((uint32_t)puc_bin_end - (uint32_t)puc_bin_start);
}

/**
 * \brief Main code entry point.
 */
int main(void)
{
	uint32_t ul_bin_addr;
	uint32_t ul_bin_size;
#ifdef CONF_BOARD_LCD_EN
	status_code_t status;
#endif

	atpl360_dev_callbacks_t x_atpl360_cbs;
	atpl360_hal_wrapper_t x_atpl360_hal_wrp;
	uint8_t uc_ret;

	/* Init counter in ms to blink led */
	ul_count_ms = 500;

	/* Prepare the hardware */
	prvSetupHardware();

#ifdef CONF_BOARD_UART_CONSOLE
	/* UART debug */
	configure_dbg_console();
	puts(STRING_HEADER);
#endif

#ifdef CONF_BOARD_LCD_EN
#if BOARD == ATPL360ASB
	/* Initialize the vim878 LCD glass component. */
	status = (status_code_t)vim878_init();
	if (status != STATUS_OK) {
		puts("-- LCD Initialization fails! --\r\n");
		while (1) {
		}
	}

	vim878_set_contrast(1);
	vim878_clear_all();
	vim878_show_text((const uint8_t *)"phytst");
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
	c0216CiZ_show((const char *)"Phy tester tool");
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
	c0216CiZ_show((const char *)"Phy tester tool");
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

	/* Init process timers */
	initTimer1ms();

	/* Init USI */
	usi_init();

	/* Initialize G3 band (defined in conf_atpl360.h) */
	suc_phy_band = ATPL360_WB;

	/* Init ATPL360 */
	x_atpl360_hal_wrp.plc_init = pplc_if_init;
	x_atpl360_hal_wrp.plc_reset = pplc_if_reset;
	x_atpl360_hal_wrp.plc_set_stby_mode = pplc_if_set_stby_mode;
	x_atpl360_hal_wrp.plc_set_handler = pplc_if_set_handler;
	x_atpl360_hal_wrp.plc_send_boot_cmd = pplc_if_send_boot_cmd;
	x_atpl360_hal_wrp.plc_write_read_cmd = pplc_if_send_wrrd_cmd;
	x_atpl360_hal_wrp.plc_enable_int = pplc_if_enable_interrupt;
	x_atpl360_hal_wrp.plc_delay = pplc_if_delay;
	x_atpl360_hal_wrp.plc_get_thw = pplc_if_get_thermal_warning;
	atpl360_init(&sx_atpl360_desc, &x_atpl360_hal_wrp);

	/* Callback configuration. Set NULL as Not used */
	x_atpl360_cbs.data_confirm = NULL;
	x_atpl360_cbs.data_indication = _handler_data_indicator;
	x_atpl360_cbs.exception_event = _handler_exception_event;
	x_atpl360_cbs.addons_event = _handler_atpl360_serial_event;
	x_atpl360_cbs.sleep_mode_cb = _handler_sleep_mode_resume_event;
	x_atpl360_cbs.debug_mode_cb = NULL;
	sx_atpl360_desc.set_callbacks(&x_atpl360_cbs);

	/* ATPL360 bin file addressing */
#ifndef PL360_BIN_ADDR_FIXED
	ul_bin_size = _get_pl360_bin_addressing(&ul_bin_addr);
#else
	ul_bin_addr = PL360_BIN_ADDR;
	ul_bin_size = PL360_BIN_SIZE;
#endif

	/* Enable ATPL360 */
	uc_ret = atpl360_enable(ul_bin_addr, ul_bin_size);
	if (uc_ret == ATPL360_ERROR) {
		PRINT_DBG(("\r\nmain: atpl360_enable call error!(%d)\r\n", uc_ret));
#if (BOARD != PIC32CXMTSH_DB) && (BOARD != SAMG55_XPLAINED_PRO) && (BOARD != SAM4CMS_DB) && (BOARD != PIC32CXMTC_DB)
		LED_On(LED0);
#ifdef LED1_GPIO
		LED_On(LED1);
#endif
#else
#ifdef CONF_BOARD_LCD_EN
		cl010_show_icon(CL010_ICON_PHASE_1);
		cl010_show_icon(CL010_ICON_PHASE_2);
#endif
#endif
		while (1) {
		}
	}

	PRINT_DBG(("\r\nmain: atpl360_enable ok\r\n"));

	/* Set USI callback : Phy Tester Tool iface */
	usi_set_callback(PROTOCOL_PHY_ATPL2X0, _handler_app_serial_event, PHY_IFACE_SERIAL_PORT);

	while (1) {
		/* Reset watchdog */
#if (!PIC32CX)
		WDT->WDT_CR = WDT_CR_KEY_PASSWD | WDT_CR_WDRSTT;
#else
		DWDT->WDT0_CR = WDT0_CR_KEY_PASSWD | WDT0_CR_WDRSTT;
		DWDT->WDT1_CR = WDT1_CR_KEY_PASSWD | WDT1_CR_WDRSTT;
#endif

		/* Manage PL360 exceptions */
		if (sb_exception_pend) {
			sb_exception_pend = false;

			/* Set PL360 specific configuration from application */
			_set_pl360_configuration();
		}

		/* blink led 0 */
		if (b_led_swap) {
			b_led_swap = false;
#if (BOARD != PIC32CXMTSH_DB) && (BOARD != PIC32CXMTC_DB)
	#if (BOARD == SAM4CMS_DB)
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

		/* Data indication led 1 */
		if (b_ind_led_swap) {
			b_ind_led_swap = false;
#ifdef LED1_GPIO
			LED_Off(LED1);
#else
#ifdef CONF_BOARD_LCD_EN
			cl010_clear_icon(CL010_ICON_PHASE_2);
#endif
#endif
		}

		/* USI process */
		usi_process();

		/* Check ATPL360 pending events */
		atpl360_handle_events();
	}
}
