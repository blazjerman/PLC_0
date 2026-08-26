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

/* System includes */
#include <stdio.h>
#include "string.h"

/* ASF includes */
#include "asf.h"

/* App includes */
#include "conf_project.h"
#include "conf_board.h"
#include "phy_embedded_example.h"

#ifdef PL360_BIN_ADDR_FIXED
#  define PL360_BIN_ADDR  0x010E0000
#  define PL360_BIN_SIZE  (64 * 1024)
#endif

static uint8_t uc_data_buf[800];    /*  Receive working buffer */

/* TX configuration parameters */
txPhyEmbeddedConfig_t xAppPhyCfgTx;

/* ATPL360 descriptor definition */
atpl360_descriptor_t sx_atpl360_desc;

/* PL360 exception pending to process */
static uint8_t sb_exception_pend;

/* G3 Band identifier */
uint8_t suc_phy_band;

static bool sb_high_temp_detected;
static uint32_t sul_high_temp_110_cnt;

/* Function declarations */
void __user_low_level_init(void);

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

/**
 * \internal
 * \brief Callback to capture exceptions form atpl360.
 *
 * \param px_tx_result Pointer to Transmission result struct
 */
static void _handler_atpl360_exceptions(atpl360_exception_t exception)
{
	UNUSED(exception);

	sb_exception_pend = true;
}

/**
 * \internal
 * \brief Callback to capture end of transmission and manage the result of transmission.
 *
 * \param px_tx_result Pointer to Transmission result struct
 */
static void _handler_atpl360_data_confirm(tx_cfm_t *x_write_result)
{
	uint32_t ul_tx_high_temp_120;

	xAppPhyCfgTx.uc_tx_result_flag = x_write_result->uc_tx_result;
	xAppPhyCfgTx.ul_tx_end_time = x_write_result->ul_tx_time;

	/* insert code to manage response */
	switch (x_write_result->uc_tx_result) {
	case TX_RESULT_PROCESS:
		printf("<-PHY_TX_RESULT_PROCESS\r\n");
		break;

	case TX_RESULT_SUCCESS:
		printf("<-PHY_TX_RESULT_SUCCESS\r\n");
		break;

	case TX_RESULT_INV_LENGTH:
		printf("<-PHY_TX_RESULT_INV_LENGTH\r\n");
		break;

	case TX_RESULT_BUSY_CH:
		printf("<-PHY_TX_RESULT_BUSY_CH\r\n");
		break;

	case TX_RESULT_BUSY_TX:
		printf("<-PHY_TX_RESULT_BUSY_TX\r\n");
		break;

	case TX_RESULT_BUSY_RX:
		printf("<-PHY_TX_RESULT_BUSY_RX\r\n");
		break;

	case TX_RESULT_INV_SCHEME:
		printf("<-PHY_TX_RESULT_INV_SCHEME\r\n");
		break;

	case TX_RESULT_TIMEOUT:
		printf("<-PHY_TX_RESULT_TIMEOUT\r\n");
		break;

	case TX_RESULT_INV_TONEMAP:
		printf("<-PHY_TX_RESULT_INV_TONEMAP\r\n");
		break;

	case TX_RESULT_INV_MODTYPE:
		printf("<-PHY_TX_RESULT_INV_MODTYPE\r\n");
		break;

	case TX_RESULT_INV_DT:
		printf("<-PHY_TX_RESULT_INV_DT\r\n");
		break;

	case TX_RESULT_CANCELLED:
		printf("<-TX_RESULT_CANCELLED\r\n");
		break;

	case TX_RESULT_HIGH_TEMP_120:
		printf("<-TX_RESULT_HIGH_TEMP_120\r\n");
		sb_high_temp_detected = true;
		break;

	case TX_RESULT_HIGH_TEMP_110:
		printf("<-TX_RESULT_HIGH_TEMP_110\r\n");
		sb_high_temp_detected = true;
		sul_high_temp_110_cnt++;
		break;

	case TX_RESULT_NO_TX:
		printf("<-PHY_TX_RESULT_NO_TX\r\n");
		break;

	default:
		printf("<-ERROR: NOT FOUND\r\n");
		break;
	}

	if (sb_high_temp_detected) {
		sx_atpl360_desc.get_config(ATPL360_REG_TX_HIGH_TEMP_120, &ul_tx_high_temp_120, 4, true);

		printf("Num TX_RESULT_HIGH_TEMP (>120ºC): %u\r\n", ul_tx_high_temp_120);
		printf("Num TX_RESULT_HIGH_TEMP (>110ºC): %u\r\n", sul_high_temp_110_cnt);
	}
}

/**
 * \internal
 * \brief Callback to capture frame reception and manage the serialization through USI.
 *
 * \param px_msg Pointer to Reception struct
 */
static void _handler_atpl360_data_indication(rx_msg_t *x_read_msg)
{
	/* set pointer to reception data buffer */
	x_read_msg->puc_data_buf = uc_data_buf;

	/* build response */
	if (x_read_msg->us_data_len) {
		/* blink Reception LED */
#if (BOARD == SAME70_XPLAINED) || (BOARD == SAMG55_XPLAINED_PRO)
		LED_On(LED0);
		delay_ms(50);
		LED_Off(LED0);
#elif (BOARD == SAM4CMS_DB)
		LED_On(LED4);
		delay_ms(50);
		LED_Off(LED4);
#elif (BOARD == PIC32CXMTSH_DB) || (BOARD == PIC32CXMTC_DB)
#ifdef CONF_BOARD_LCD_EN
		cl010_clear_icon(CL010_ICON_PHASE_1);
		delay_ms(50);
		cl010_show_icon(CL010_ICON_PHASE_1);
#endif
#else
		LED_On(LED1);
		delay_ms(50);
		LED_Off(LED1);
#endif
	}
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

void  app_init(void)
{
	uint32_t ul_bin_addr;
	uint32_t ul_bin_size;
	atpl360_hal_wrapper_t x_atpl360_hal_wrp;
	atpl360_dev_callbacks_t x_atpl360_cbs;
	uint8_t uc_ret;

	/* Initialize G3 band (defined in conf_atpl360.h) */
	suc_phy_band = ATPL360_WB;

#if ((defined(CONF_MULTIBAND_FCC_CENA) || defined(CONF_MULTIBAND_FCC_CENB)) && defined(SELECT_G3_FCC_PIN))
	ioport_set_pin_dir(SELECT_G3_FCC_PIN, IOPORT_DIR_INPUT);
	ioport_set_pin_mode(SELECT_G3_FCC_PIN, IOPORT_MODE_PULLUP);
	if (ioport_get_pin_level(SELECT_G3_FCC_PIN) == 0) {
		/* FCC selected with pin */
		suc_phy_band = ATPL360_WB_FCC;
	}
#endif

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
	x_atpl360_cbs.data_confirm = _handler_atpl360_data_confirm;
	x_atpl360_cbs.data_indication = _handler_atpl360_data_indication;
	x_atpl360_cbs.exception_event = _handler_atpl360_exceptions;
	x_atpl360_cbs.addons_event = NULL;
	x_atpl360_cbs.sleep_mode_cb = NULL;
	x_atpl360_cbs.debug_mode_cb = NULL;
	sx_atpl360_desc.set_callbacks(&x_atpl360_cbs);

	/* ATPL360 bin file addressing */
#ifndef PL360_BIN_ADDR_FIXED
	ul_bin_size = _get_pl360_bin_addressing(&ul_bin_addr);
#else
	ul_bin_addr = PL360_BIN_ADDR;
	ul_bin_size = PL360_BIN_SIZE;
#endif

	/* Init ATPL360 */
	uc_ret = atpl360_enable(ul_bin_addr, ul_bin_size);
	if (uc_ret == ATPL360_ERROR) {
		printf("\r\nmain: atpl360_enable call error!(%d)\r\n", uc_ret);
#if (BOARD != PIC32CXMTSH_DB) && (BOARD != PIC32CXMTC_DB) && (BOARD != SAMG55_XPLAINED_PRO) && (BOARD != SAM4CMS_DB)
		LED_On(LED0);
		LED_On(LED1);
#else
#ifdef CONF_BOARD_LCD_EN
		cl010_show_icon(CL010_ICON_PHASE_1);
		cl010_show_icon(CL010_ICON_PHASE_2);
#endif
#endif
		while (1) {
		}
	}

	/* Set PL360 configuration */
	_set_pl360_configuration();
	sb_exception_pend = false;
	sb_high_temp_detected = false;
	sul_high_temp_110_cnt = 0;

	xAppPhyCfgTx.ul_tx_end_time = 0;
}

void run_tx_task()
{
	uint32_t ul_curr_time;
	uint32_t ul_prog_time;
	uint8_t uc_tx_power_prev;
	tx_cfm_t x_tx_result;

	/* Manage PL360 exceptions */
	if (sb_exception_pend) {
		xAppPhyCfgTx.ul_tx_end_time = 0;

		sb_exception_pend = false;

		/* Set PL360 specific configuration from application */
		_set_pl360_configuration();
	}

	sx_atpl360_desc.get_config(ATPL360_TIME_REF_ID, &ul_curr_time, 4, true);
	ul_prog_time = xAppPhyCfgTx.ul_tx_end_time + xAppPhyCfgTx.ul_tx_period;

	if (xAppPhyCfgTx.ul_tx_end_time == 0) {
		xAppPhyCfgTx.xPhyMsg.ul_tx_time = ul_curr_time  + 100000;
	} else if (ul_curr_time > ul_prog_time) {
		xAppPhyCfgTx.xPhyMsg.ul_tx_time = ul_curr_time + xAppPhyCfgTx.ul_tx_period;
	} else {
		xAppPhyCfgTx.xPhyMsg.ul_tx_time = ul_prog_time;
	}

	/* printf("curr_time: %d \t prog_time:%d , xAppPhyCfgTx.xPhyMsg.ul_tx_time:%d\r\n", ul_curr_time, ul_prog_time, xAppPhyCfgTx.xPhyMsg.ul_tx_time); */

	if (xAppPhyCfgTx.uc_force_no_output == 1) {
		/* For zero gain: uc_tx_power = 0xFF */
		uc_tx_power_prev = xAppPhyCfgTx.xPhyMsg.uc_tx_power;
		xAppPhyCfgTx.xPhyMsg.uc_tx_power = 0xFF;
	}

	xAppPhyCfgTx.uc_tx_result_flag = sx_atpl360_desc.send_data(&xAppPhyCfgTx.xPhyMsg);
	printf("->Send message\r\n");
	fflush(stdout);

	if (xAppPhyCfgTx.uc_tx_result_flag != TX_RESULT_PROCESS) {
		x_tx_result.uc_tx_result = (enum tx_result_values)xAppPhyCfgTx.uc_tx_result_flag;
		x_tx_result.ul_tx_time = xAppPhyCfgTx.xPhyMsg.ul_tx_time;
		_handler_atpl360_data_confirm(&x_tx_result);
	}

	if (xAppPhyCfgTx.uc_force_no_output == 1) {
		/* Restore uc_tx_power in struct */
		xAppPhyCfgTx.xPhyMsg.uc_tx_power = uc_tx_power_prev;
	}
}
