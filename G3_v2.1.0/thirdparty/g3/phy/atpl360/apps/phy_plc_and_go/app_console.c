/**
 * \file
 *
 * \brief PHY_CHAT : G3-PLC Phy PLC And Go Application. Module to manage console
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

/* Atmel library includes. */
#include "asf.h"
#include "conf_project.h"
#include "app_console.h"
#include "app_phy_ctl.h"

/* Maximum message data length allowed by G3-PLC Physical Layer. */
/* The maximum data length depends on: */
/*                 - G3 band (CENELEC-A, CENELEC-B, FCC or ARIB) */
/*                 - Modulation Type (BPSK Robust, BPSK, QPSK or 8PSK). See "enum mod_types" in "atpl360_comm.h" */
/*                 - Modulation Scheme (Differential of coherent). See "enum mod_schemes" in "atpl360_comm.h" */
/*                 - Number of Reed-Solomon blocks (uc_2_rs_blocks). Only applies to FCC band. 1 or 2 blocks. With 2 blocks the maximum length is the double */
/*                 - Tone Map (puc_tone_map). Dynamic notching: The subbands used to send the data can be chosen with Tone Map */
/*                 - Tone Mask (ATPL360_REG_TONE_MASK). Static notching: Carriers can be notched and no energy is sent in those carriers */
/* The absolute maximum corresponds to FCC band with 2 Reed-Solomon blocks and BPSK Robust modulation. 2 * (255 - 8) = 494 */
#define MAX_DATA_LEN        494

/* Time in ms that LED is ON after message reception */
#define COUNT_MS_IND_LED        50

/* Application state machine */
typedef enum app_state {
	APP_STATE_IDLE,
	APP_STATE_TYPING,
	APP_STATE_CONFIG_MENU,
	APP_STATE_CONFIG_MOD,
#if defined(CONF_MULTIBAND_FCC_CENA) || defined(CONF_MULTIBAND_FCC_CENB)
	APP_STATE_CONFIG_BAND,
#endif
	APP_STATE_TRANSMITTING,
} app_state_t;

static app_state_t suc_app_state;

/* Transmission data buffer */
static uint8_t spuc_tx_data_buff[MAX_DATA_LEN];

/* Index for transmission data buffer */
static uint16_t sus_tx_data_index;

/* Maximum length with configured Tx Parameters (see the above explanation) */
static uint16_t sus_max_data_len;

/* LED management variables */
extern uint32_t sul_ind_count_ms;

/**
 * \brief Shows maximum data rate that can be achieved depending on Tx modulation and band.
 * The data rate is computed for maximum data length, full Tone Map and no static notching.
 * data_rate = (max_data_len * 8) / frame_duration.
 *
 * \param uc_mod_scheme Modulation Scheme.
 * \param uc_mod_type Modulation Type.
 *
 */
static void _show_data_rate(enum mod_schemes uc_mod_scheme, enum mod_types uc_mod_type)
{
	uint8_t uc_phy_band;

	uc_phy_band = phy_ctl_get_band();

	if (uc_mod_scheme == MOD_SCHEME_DIFFERENTIAL) {
		switch (uc_mod_type) {
		case MOD_TYPE_BPSK:
			switch (uc_phy_band) {
			case ATPL360_WB_CENELEC_A:
				printf(" .......... 20.1 kbit/s\r\n");
				break;

			case ATPL360_WB_FCC:
				if (phy_ctl_get_2_rs_blocks() == 0) {
					/* 1 Reed-Solomon block */
					printf(" .......... 105.3 kbit/s\r\n");
				} else {
					/* 2 Reed-Solomon blocks */
					printf(" .......... 122 kbit/s\r\n");
				}

				break;

			case ATPL360_WB_ARIB:
				printf(" .......... 81.2 kbit/s\r\n");
				break;

			case ATPL360_WB_CENELEC_B:
				printf(" .......... 9.2 kbit/s\r\n");
				break;
			}
			break;

		case MOD_TYPE_QPSK:
			switch (uc_phy_band) {
			case ATPL360_WB_CENELEC_A:
				printf(" .......... 34.5 kbit/s\r\n");
				break;

			case ATPL360_WB_FCC:
				if (phy_ctl_get_2_rs_blocks() == 0) {
					/* 1 Reed-Solomon block */
					printf(" .......... 165.1 kbit/s\r\n");
				} else {
					/* 2 Reed-Solomon blocks */
					printf(" .......... 210.6 kbit/s\r\n");
				}

				break;

			case ATPL360_WB_ARIB:
				printf(" .......... 130.4 kbit/s\r\n");
				break;

			case ATPL360_WB_CENELEC_B:
				printf(" .......... 16.4 kbit/s\r\n");
				break;
			}
			break;

		case MOD_TYPE_8PSK:
			switch (uc_phy_band) {
			case ATPL360_WB_CENELEC_A:
				printf(" .......... 44.6 kbit/s\r\n");
				break;

			case ATPL360_WB_FCC:
				if (phy_ctl_get_2_rs_blocks() == 0) {
					/* 1 Reed-Solomon block */
					printf(" .......... 205.9 kbit/s\r\n");
				} else {
					/* 2 Reed-Solomon blocks */
					printf(" .......... 279.8 kbit/s\r\n");
				}

				break;

			case ATPL360_WB_CENELEC_B:
				printf(" .......... 21.8 kbit/s\r\n");
				break;
			}
			break;

		case MOD_TYPE_BPSK_ROBO:
			switch (uc_phy_band) {
			case ATPL360_WB_CENELEC_A:
				printf(" ... 5.5 kbit/s\r\n");
				break;

			case ATPL360_WB_FCC:
				if (phy_ctl_get_2_rs_blocks() == 0) {
					/* 1 Reed-Solomon block */
					printf(" ... 34 kbit/s\r\n");
				} else {
					/* 2 Reed-Solomon blocks */
					printf(" ... 35.6 kbit/s\r\n");
				}

				break;

			case ATPL360_WB_ARIB:
				printf(" ... 25.7 kbit/s\r\n");
				break;

			case ATPL360_WB_CENELEC_B:
				printf(" ... 2.1 kbit/s\r\n");
				break;
			}
			break;
		}
	} else if (uc_mod_scheme == MOD_SCHEME_COHERENT) {
		switch (uc_mod_type) {
		case MOD_TYPE_BPSK:
			switch (uc_phy_band) {
			case ATPL360_WB_CENELEC_A:
				printf(" .............. 18.5 kbit/s\r\n");
				break;

			case ATPL360_WB_FCC:
				if (phy_ctl_get_2_rs_blocks() == 0) {
					/* 1 Reed-Solomon block */
					printf(" .............. 96.6 kbit/s\r\n");
				} else {
					/* 2 Reed-Solomon blocks */
					printf(" .............. 112 kbit/s\r\n");
				}

				break;

			case ATPL360_WB_CENELEC_B:
				printf(" .............. 7.9 kbit/s\r\n");
				break;
			}
			break;

		case MOD_TYPE_QPSK:
			switch (uc_phy_band) {
			case ATPL360_WB_CENELEC_A:
				printf(" .............. 31.3 kbit/s\r\n");
				break;

			case ATPL360_WB_FCC:
				if (phy_ctl_get_2_rs_blocks() == 0) {
					/* 1 Reed-Solomon block */
					printf(" .............. 152.3 kbit/s\r\n");
				} else {
					/* 2 Reed-Solomon blocks */
					printf(" .............. 194 kbit/s\r\n");
				}

				break;

			case ATPL360_WB_CENELEC_B:
				printf(" .............. 14.5 kbit/s\r\n");
				break;
			}
			break;

		case MOD_TYPE_8PSK:
			switch (uc_phy_band) {
			case ATPL360_WB_CENELEC_A:
				printf(" .............. 41.2 kbit/s\r\n");
				break;

			case ATPL360_WB_FCC:
				if (phy_ctl_get_2_rs_blocks() == 0) {
					/* 1 Reed-Solomon block */
					printf(" .............. 184.2 kbit/s\r\n");
				} else {
					/* 2 Reed-Solomon blocks */
					printf(" .............. 252.1 kbit/s\r\n");
				}

				break;

			case ATPL360_WB_CENELEC_B:
				printf(" .............. 19.6 kbit/s\r\n");
				break;
			}
			break;

		case MOD_TYPE_BPSK_ROBO:
			switch (uc_phy_band) {
			case ATPL360_WB_CENELEC_A:
				printf(" ....... 5 kbit/s\r\n");
				break;

			case ATPL360_WB_FCC:
				if (phy_ctl_get_2_rs_blocks() == 0) {
					/* 1 Reed-Solomon block */
					printf(" ....... 31.2 kbit/s\r\n");
				} else {
					/* 2 Reed-Solomon blocks */
					printf(" ....... 32.8 kbit/s\r\n");
				}

				break;

			case ATPL360_WB_CENELEC_B:
				printf(" ....... 1.7 kbit/s\r\n");
				break;
			}
			break;
		}
	}
}

/**
 * \brief Shows Tx modulation (Modulation Type and Modulation Scheme) as string.
 *
 * \param uc_mod_scheme Modulation Scheme.
 * \param uc_mod_type Modulation Type.
 *
 */
static void _show_modulation(enum mod_schemes uc_mod_scheme, enum mod_types uc_mod_type)
{
	if (uc_mod_scheme == MOD_SCHEME_DIFFERENTIAL) {
		switch (uc_mod_type) {
		case MOD_TYPE_BPSK:
			printf("BPSK Differential");
			break;

		case MOD_TYPE_QPSK:
			printf("QPSK Differential");
			break;

		case MOD_TYPE_8PSK:
			printf("8PSK Differential");
			break;

		case MOD_TYPE_BPSK_ROBO:
			printf("BPSK Robust Differential");
			break;

		default:
			printf("Unknown");
			break;
		}
	} else if (uc_mod_scheme == MOD_SCHEME_COHERENT) {
		switch (uc_mod_type) {
		case MOD_TYPE_BPSK:
			printf("BPSK Coherent");
			break;

		case MOD_TYPE_QPSK:
			printf("QPSK Coherent");
			break;

		case MOD_TYPE_8PSK:
			printf("8PSK Coherent");
			break;

		case MOD_TYPE_BPSK_ROBO:
			printf("BPSK Robust Coherent");
			break;

		default:
			printf("Unknown");
			break;
		}
	} else {
		printf("Unknown");
	}
}

#if defined(CONF_MULTIBAND_FCC_CENA) || defined(CONF_MULTIBAND_FCC_CENB)

/**
 * \brief Shows Band as string.
 *
 * \param uc_band Band identifier (see "general_defs.h").
 *
 */
static void _show_band(uint8_t uc_band)
{
	switch (uc_band) {
	case ATPL360_WB_CENELEC_A:
		printf("CENELEC-A band (35 - 91 kHz)\r\n");
		break;

	case ATPL360_WB_FCC:
		printf("FCC band (154 - 488 kHz)\r\n");
		break;

	case ATPL360_WB_ARIB:
		printf("ARIB band (154 - 404 kHz)\r\n");
		break;

	case ATPL360_WB_CENELEC_B:
		printf("CENELEC-B band (98 - 122 kHz)\r\n");
		break;

	default:
		printf("Unknown band\r\n");
		break;
	}
}

#endif

/**
 * \brief Shows available options through console depending on app state.
 *
 * \param b_full_menu Show full menu (true). Only used in APP_STATE_IDLE.
 *
 */
static void _show_menu(bool b_full_menu)
{
	enum mod_schemes uc_mod_scheme;
	enum mod_types uc_mod_type;
	uint8_t uc_phy_band;

	switch (suc_app_state) {
	case APP_STATE_IDLE:
		/* Idle state: Show available options */
		if (b_full_menu) {
			printf("\r\nPress 'CTRL+S' to enter configuration menu. ");
			printf("Enter text and press 'ENTER' to trigger transmission");
		}

		printf("\r\n>>> ");
		fflush(stdout);
		break;

	case APP_STATE_TYPING:
		/* Typing message to transmit */
		printf("\r\n>>> %.*s", sus_tx_data_index - 2, spuc_tx_data_buff + 2);
		fflush(stdout);
		break;

	case APP_STATE_CONFIG_MENU:
		/* Configuration Menu */
		printf("\r\n--- Configuration Menu ---\r\n");
		printf("Select parameter to configure: \r\n");
		printf("\t0: Tx Modulation\r\n");
#if defined(CONF_MULTIBAND_FCC_CENA) || defined(CONF_MULTIBAND_FCC_CENB)
		printf("\t1: Tx/Rx Band\r\n");
#endif
		printf(">>> ");
		fflush(stdout);
		break;

	case APP_STATE_CONFIG_MOD:
		/* Modulation Configuration */
		uc_mod_scheme = phy_ctl_get_mod_scheme();
		uc_mod_type = phy_ctl_get_mod_type();
		printf("\r\n--- Tx Modulation Configuration Menu ---\r\n");
		printf("Select Modulation:\r\n");

		if ((uc_mod_scheme == MOD_SCHEME_DIFFERENTIAL) && (uc_mod_type == MOD_TYPE_BPSK_ROBO)) {
			printf("->");
		}

		printf("\t0: ");
		_show_modulation(MOD_SCHEME_DIFFERENTIAL, MOD_TYPE_BPSK_ROBO);
		_show_data_rate(MOD_SCHEME_DIFFERENTIAL, MOD_TYPE_BPSK_ROBO);

		if ((uc_mod_scheme == MOD_SCHEME_DIFFERENTIAL) && (uc_mod_type == MOD_TYPE_BPSK)) {
			printf("->");
		}

		printf("\t1: ");
		_show_modulation(MOD_SCHEME_DIFFERENTIAL, MOD_TYPE_BPSK);
		_show_data_rate(MOD_SCHEME_DIFFERENTIAL, MOD_TYPE_BPSK);

		if ((uc_mod_scheme == MOD_SCHEME_DIFFERENTIAL) && (uc_mod_type == MOD_TYPE_QPSK)) {
			printf("->");
		}

		printf("\t2: ");
		_show_modulation(MOD_SCHEME_DIFFERENTIAL, MOD_TYPE_QPSK);
		_show_data_rate(MOD_SCHEME_DIFFERENTIAL, MOD_TYPE_QPSK);

		uc_phy_band = phy_ctl_get_band();

		if (uc_phy_band != ATPL360_WB_ARIB) {
			/* 8PSK modulation type and coherent modulation scheme not allowed in ARIB band */
			if ((uc_mod_scheme == MOD_SCHEME_DIFFERENTIAL) && (uc_mod_type == MOD_TYPE_8PSK)) {
				printf("->");
			}

			printf("\t3: ");
			_show_modulation(MOD_SCHEME_DIFFERENTIAL, MOD_TYPE_8PSK);
			_show_data_rate(MOD_SCHEME_DIFFERENTIAL, MOD_TYPE_8PSK);

			if ((uc_mod_scheme == MOD_SCHEME_COHERENT) && (uc_mod_type == MOD_TYPE_BPSK_ROBO)) {
				printf("->");
			}

			printf("\t4: ");
			_show_modulation(MOD_SCHEME_COHERENT, MOD_TYPE_BPSK_ROBO);
			_show_data_rate(MOD_SCHEME_COHERENT, MOD_TYPE_BPSK_ROBO);

			if ((uc_mod_scheme == MOD_SCHEME_COHERENT) && (uc_mod_type == MOD_TYPE_BPSK)) {
				printf("->");
			}

			printf("\t5: ");
			_show_modulation(MOD_SCHEME_COHERENT, MOD_TYPE_BPSK);
			_show_data_rate(MOD_SCHEME_COHERENT, MOD_TYPE_BPSK);

			if ((uc_mod_scheme == MOD_SCHEME_COHERENT) && (uc_mod_type == MOD_TYPE_QPSK)) {
				printf("->");
			}

			printf("\t6: ");
			_show_modulation(MOD_SCHEME_COHERENT, MOD_TYPE_QPSK);
			_show_data_rate(MOD_SCHEME_COHERENT, MOD_TYPE_QPSK);

			if ((uc_mod_scheme == MOD_SCHEME_COHERENT) && (uc_mod_type == MOD_TYPE_8PSK)) {
				printf("->");
			}

			printf("\t7: ");
			_show_modulation(MOD_SCHEME_COHERENT, MOD_TYPE_8PSK);
			_show_data_rate(MOD_SCHEME_COHERENT, MOD_TYPE_8PSK);
		}

		printf(">>> ");
		fflush(stdout);
		break;

#if defined(CONF_MULTIBAND_FCC_CENA)
	case APP_STATE_CONFIG_BAND:
		/* Band configuration */
		printf("\r\n--- Band Configuration Menu ---\r\n");
		printf("Select Band:\r\n");

		uc_phy_band = phy_ctl_get_band();

		if (uc_phy_band == ATPL360_WB_CENELEC_A) {
			printf("->");
		}

		printf("\t%u: ", (uint32_t)ATPL360_WB_CENELEC_A);
		_show_band(ATPL360_WB_CENELEC_A);

		if (uc_phy_band == ATPL360_WB_FCC) {
			printf("->");
		}

		printf("\t%u: ", (uint32_t)ATPL360_WB_FCC);
		_show_band(ATPL360_WB_FCC);

		printf(">>> ");
		fflush(stdout);
		break;

#elif defined(CONF_MULTIBAND_FCC_CENB)
	case APP_STATE_CONFIG_BAND:
		/* Band configuration */
		printf("\r\n--- Band Configuration Menu ---\r\n");
		printf("Select Band:\r\n");

		uc_phy_band = phy_ctl_get_band();

		if (uc_phy_band == ATPL360_WB_CENELEC_B) {
			printf("->");
		}

		printf("\t%u: ", (uint32_t)ATPL360_WB_CENELEC_B);
		_show_band(ATPL360_WB_CENELEC_B);

		if (uc_phy_band == ATPL360_WB_FCC) {
			printf("->");
		}

		printf("\t%u: ", (uint32_t)ATPL360_WB_FCC);
		_show_band(ATPL360_WB_FCC);

		printf(">>> ");
		fflush(stdout);
		break;
#endif
	}
}

/**
 * \brief Read one char from Serial port.
 *
 * \param c Pointer read char.
 *
 * \retval 0 Success. One char was read
 * \retval 1 There is no char to read.
 */
static uint32_t _read_char(uint8_t *c)
{
#ifdef CONF_BOARD_UDC_CONSOLE
	/* Read char through USB (SAMG55) */
	uint16_t us_res;
	us_res = usb_wrp_udc_read_buf(c, 1) ? 0 : 1;
	return us_res;

#else
#  if SAMG55 || PIC32CX
	/* Read char through USART (SAMG55) */
	uint32_t ul_char;
	uint32_t ul_res;

	ul_res = usart_read((Usart *)CONF_UART, (uint32_t *)&ul_char);
	*c = (uint8_t)ul_char;

	return ul_res;

#  else
	/* Read char through UART */
	return uart_read((Uart *)CONF_UART, c);
#  endif
#endif
}

/**
 * \brief Shows Tx parameters through console.
 *
 * \param uc_mod_scheme Modulation Scheme.
 * \param uc_mod_type Modulation Type.
 *
 */
static void _show_tx_parameters(enum mod_schemes uc_mod_scheme, enum mod_types uc_mod_type)
{
	/* Modulation Type & Modulation Scheme */
	printf("\r\nTx Modulation: ");
	_show_modulation(uc_mod_scheme, uc_mod_type);

	/* Maximum data length with configured Tx Parameters */
	printf(" (Max data length = %u bytes)\r\n", (uint32_t)sus_max_data_len);
}

/**
 * \brief Handles received message with CRC ok. Prints message parameters and data.
 *
 * \param puc_data_buf Pointer to buffer containing received data.
 * \param us_data_len Length of received data in bytes. It may contain padding length.
 * \param uc_mod_scheme Modulation Scheme of received frame.
 * \param uc_mod_type Modulation Type of received frame.
 * \param us_rssi Received Signal Strength Indicator in dBuV.
 * \param uc_lqi Link Quality Indicator in quarters of dB and 10-dB offset (LQI = 0 means -10 dB).
 *
 */
static void app_console_handle_rx_msg(uint8_t *puc_data_buf, uint16_t us_data_len, enum mod_schemes uc_mod_scheme, enum mod_types uc_mod_type, uint16_t us_rssi, uint8_t uc_lqi)
{
	uint16_t us_len;

	/* Extract real data length from two first bytes */
	/* It does not have to be the same as us_data_len because us_data_len may contain padding length */
	us_len = puc_data_buf[0] << 8;
	us_len += puc_data_buf[1];

	if (us_len > us_data_len) {
		/* Length error: length in message content should never be more than total data length from PHY */
		printf("\rRx ERROR: length error\r\n");
	} else {
		printf("\rRx (");
		/* Show Modulation of received frame */
		_show_modulation(uc_mod_scheme, uc_mod_type);
		/* Show RSSI (Received Signal Strength Indicator) in dBuV */
		printf(", RSSI %udBuV", (uint32_t)us_rssi);
		/* Show LQI (Link Quality Indicator). It is in quarters of dB and 10-dB offset: SNR(dB) = (LQI - 40) / 4 */
		printf(", LQI %ddB): ", div_round((int16_t)uc_lqi - 40, 4));
		/* Show received message */
		printf("%.*s", us_len - 2, puc_data_buf + 2);
	}

	_show_menu(false);

	/* Turn on LED1 to indicate the reception of PLC message */
	sul_ind_count_ms = COUNT_MS_IND_LED;
#ifdef LED1_GPIO
	LED_On(LED1);
#endif
}

/**
 * \brief Handles received message with bad CRC (corrupted message).
 */
static void app_console_handle_rx_msg_discarded(void)
{
	printf("\rRx ERROR: CRC error\r\n");
	_show_menu(false);

	/* Turn on LED1 to indicate the reception of PLC message */
	sul_ind_count_ms = COUNT_MS_IND_LED;
#ifdef LED1_GPIO
	LED_On(LED1);
#endif
}

/**
 * \brief Handles a modification of the configuration of transmission.
 *
 * \param uc_mod_scheme Configured Tx Modulation Scheme.
 * \param uc_mod_type Configured Tx Modulation Type.
 * \param us_max_data_len Maximum length of data that can be sent with the configured Tx parameters.
 *
 */
static void app_console_handle_upd_tx_cfg(enum mod_schemes uc_mod_scheme, enum mod_types uc_mod_type, uint16_t us_max_data_len)
{
	/* Store maximum data length that can be sent with current configured Tx parameters */
	sus_max_data_len = us_max_data_len;

	_show_tx_parameters(uc_mod_scheme, uc_mod_type);

	/* Initialize data buffer index to point to third byte */
	/* First 2 bytes are used to store message length because Physical Layer may add padding in order to complete symbols with data */
	/* Data length reported in received frames includes such padding length */
	/* Therefore, it is needed to include data length in message content in order to know the real data length in reception */
	sus_tx_data_index = 2;

	/* Reset Chat App state */
	suc_app_state = APP_STATE_IDLE;

	_show_menu(true);
}

/**
 * \brief Handles confirm of transmitted message.
 *
 */
static void app_console_handle_tx_cfm(enum tx_result_values uc_tx_result)
{
	switch (uc_tx_result) {
	case TX_RESULT_SUCCESS:
		/* PLC message was successfully transmitted */
		printf(" TX_RESULT_SUCCESS");
		break;

	case TX_RESULT_INV_LENGTH:
		/* Data length is invalid */
		printf(" TX_RESULT_INV_LENGTH");
		break;

	case TX_RESULT_INV_SCHEME:
		/* Modulation Scheme is invalid */
		printf(" TX_RESULT_INV_SCHEME");
		break;

	case TX_RESULT_INV_TONEMAP:
		/* Tone Map is invalid */
		printf(" TX_RESULT_INV_TONEMAP");
		break;

	case TX_RESULT_INV_MODTYPE:
		/* Modulation Type is invalid */
		printf(" TX_RESULT_INV_MODTYPE");
		break;

	case TX_RESULT_INV_DT:
		/* Delimiter Type is invalid */
		printf(" TX_RESULT_INV_DT");
		break;

	case TX_RESULT_BUSY_CH:
		/* Transmission aborted because there is a reception in progress (PLC channel is busy) */
		/* If TX_MODE_FORCED is used in Tx parameters, transmission is never aborted by reception */
		printf(" TX_RESULT_BUSY_CH");
		break;

	case TX_RESULT_BUSY_RX:
		/* Transmission aborted because there is a reception in progress (PLC channel is busy) */
		/* If TX_MODE_FORCED is used in Tx parameters, transmission is never aborted by reception */
		printf(" TX_RESULT_BUSY_RX");
		break;

	case TX_RESULT_BUSY_TX:
		/* There is another transmission that has not been transmitted yet */
		printf(" TX_RESULT_BUSY_TX");
		break;

	case TX_RESULT_TIMEOUT:
		/* Timeout Error */
		printf(" TX_RESULT_TIMEOUT");
		break;

	case TX_RESULT_CANCELLED:
		/* Transmission cancelled */
		printf(" TX_RESULT_CANCELLED");
		break;

	case TX_RESULT_HIGH_TEMP_120:
		/* High temperature Error (>120ºC) */
		printf(" TX_RESULT_HIGH_TEMP_120");
		break;

	case TX_RESULT_NO_TX:
		/* No transmission ongoing */
		printf(" TX_RESULT_NO_TX");
		break;
	}

	/* Initialize data buffer index to point to third byte */
	/* First 2 bytes are used to store message length because Physical Layer may add padding in order to complete symbols with data */
	/* Data length reported in received frames includes such padding length */
	/* Therefore, it is needed to include data length in message content in order to know the real data length in reception */
	sus_tx_data_index = 2;

	/* Reset Chat App state */
	suc_app_state = APP_STATE_IDLE;

	_show_menu(false);
}

/**
 * \brief Initialization of Chat module.
 *
 */
void app_console_init(void)
{
	phy_ctl_callbacks_t x_phy_ctl_cbs;

	x_phy_ctl_cbs.phy_ctl_data_confirm = app_console_handle_tx_cfm;
	x_phy_ctl_cbs.phy_ctl_rx_msg_discarded = app_console_handle_rx_msg_discarded;
	x_phy_ctl_cbs.phy_ctl_rx_msg = app_console_handle_rx_msg;
	x_phy_ctl_cbs.phy_ctl_update_tx_configuration = app_console_handle_upd_tx_cfg;

	/* Set PHY controller callbacks */
	phy_ctl_set_callbacks(&x_phy_ctl_cbs);

	/* Initialize Chat App state */
	suc_app_state = APP_STATE_IDLE;
}

/**
 * \brief Chat module process. Process Serial port input.
 */
void app_console_process(void)
{
	/* Process Chat input */
	uint8_t uc_char;
	bool b_send_plc_msg;
	enum mod_schemes uc_mod_scheme;
	enum mod_types uc_mod_type;
	bool b_valid_modulation;
	uint8_t uc_result;
	uint8_t uc_phy_band;
#if defined(CONF_MULTIBAND_FCC_CENA) || defined(CONF_MULTIBAND_FCC_CENB)
	bool b_valid_band;
	uint8_t uc_band;
#endif

	if (suc_app_state == APP_STATE_TRANSMITTING) {
		/* If transmitting, wait to end transmission to process incoming characters */
		return;
	}

	if (!_read_char((uint8_t *)&uc_char)) {
		/* Show received character if it is printable (ASCII) */
		if (((uc_char >= 32) && (uc_char <= 126)) || ((uc_char >= 128) && (uc_char <= 254)) || (uc_char == '\t') || (uc_char == '\r') || (uc_char == '\n')) {
			printf("%c", uc_char);
			fflush(stdout);
		}

		switch (suc_app_state) {
		case APP_STATE_IDLE:
			switch (uc_char) {
			case 0x13:
				/* Special character: 0x13 (CTRL+S). Enter configuration menu */
#if defined(CONF_MULTIBAND_FCC_CENA) || defined(CONF_MULTIBAND_FCC_CENB)
				/* Multiband supported: Go to configuration menu to select Band or Modulation configuration */
				suc_app_state = APP_STATE_CONFIG_MENU;
#else
				/* Multiband not supported: Go directly to Modulation configuration */
				suc_app_state = APP_STATE_CONFIG_MOD;
#endif
				_show_menu(true);
				break;

			case '\r':
			case '\n':
				/* Special character: '\r' (Carriage Return) or '\n' (Line Feed). Show menu */
				_show_menu(true);
				break;

			default:
				/* Normal character: Start to type message if it is printable (ASCII) */
				if (((uc_char >= 32) && (uc_char <= 126)) || ((uc_char >= 128) && (uc_char <= 254)) || (uc_char == '\t')) {
					suc_app_state = APP_STATE_TYPING;
				}
			}

			if (suc_app_state != APP_STATE_TYPING) {
				break;
			}

		case APP_STATE_TYPING:
			b_send_plc_msg = false;

			switch (uc_char) {
			case '\r':
				/* Special character: '\r' (carriage return). Send message through PLC */
				b_send_plc_msg = true;
				break;

			case '\b':
			case 0x7F:
				/* Special character: '\b' (backspace) or 0x7F (DEL). Remove character from Tx data buffer */
				if (sus_tx_data_index > 2) {
					sus_tx_data_index--;
					printf("\b \b");
					fflush(stdout);
				}

				if (sus_tx_data_index == 2) {
					/* All text has been removed: Go back to idle state */
					suc_app_state = APP_STATE_IDLE;
				}

				break;

			default:
				/* Normal character: Add to Tx data buffer if it is printable (ASCII) */
				if (((uc_char >= 32) && (uc_char <= 126)) || ((uc_char >= 128) && (uc_char <= 254)) || (uc_char == '\t') || (uc_char == '\n')) {
					spuc_tx_data_buff[sus_tx_data_index++] = uc_char;
					if (sus_tx_data_index == sus_max_data_len) {
						/* Maximum data length reached: Send message through PLC */
						printf("\r\nMax data length reached... Message will be sent\r\n");
						b_send_plc_msg = true;
					}
				}
			}

			if (b_send_plc_msg) {
				/* Fill 2 first bytes with data length */
				/* Physical Layer may add padding bytes in order to complete symbols with data */
				/* It is needed to include real data length in the message because otherwise at reception is not possible to know if there is padding or not */
				spuc_tx_data_buff[0] = sus_tx_data_index >> 8;
				spuc_tx_data_buff[1] = sus_tx_data_index & 0xFF;

				/* Send PLC message */
				uc_result = phy_ctl_send_msg(spuc_tx_data_buff, sus_tx_data_index);
				switch (uc_result) {
				case TX_RESULT_PROCESS:
					printf("\r\nTx (%u bytes): ", (uint32_t)sus_tx_data_index);
					suc_app_state = APP_STATE_TRANSMITTING;
					break;

				case TX_RESULT_INV_LENGTH:
					printf("\r\nError sending PLC message: TX_RESULT_INV_LENGTH\r\n");
					suc_app_state = APP_STATE_IDLE;
					break;

				case TX_RESULT_NO_TX:
					printf("\r\nError sending PLC message: TX_RESULT_NO_TX\r\n");
					suc_app_state = APP_STATE_IDLE;
					break;

				case TX_RESULT_HIGH_TEMP_110:
					printf("\r\nError sending PLC message: TX_RESULT_HIGH_TEMP_110\r\n");
					suc_app_state = APP_STATE_IDLE;
					break;

				default:
					printf("\r\nError sending PLC message: Unknown Error\r\n");
					suc_app_state = APP_STATE_IDLE;
					break;
				}
			}

			break;

		case APP_STATE_CONFIG_MENU:
			switch (uc_char) {
			case '0':
				/* Modulation configuration */
				suc_app_state = APP_STATE_CONFIG_MOD;
				_show_menu(true);
				break;

#if defined(CONF_MULTIBAND_FCC_CENA) || defined(CONF_MULTIBAND_FCC_CENB)
			case '1':
				/* Band configuration */
				suc_app_state = APP_STATE_CONFIG_BAND;
				_show_menu(true);
				break;
#endif

			default:
				printf("\r\nUnknown command. Skipping configuration\r\n");
				suc_app_state = APP_STATE_IDLE;
				break;
			}

			break;

		case APP_STATE_CONFIG_MOD:
			b_valid_modulation = true;
			uc_phy_band = phy_ctl_get_band();
			switch (uc_char) {
			case '0':
				/* BPSK_ROBO_D */
				uc_mod_type = MOD_TYPE_BPSK_ROBO;
				uc_mod_scheme = MOD_SCHEME_DIFFERENTIAL;
				break;

			case '1':
				/* BPSK_D */
				uc_mod_type = MOD_TYPE_BPSK;
				uc_mod_scheme = MOD_SCHEME_DIFFERENTIAL;
				break;

			case '2':
				/* QPSK_D */
				uc_mod_type = MOD_TYPE_QPSK;
				uc_mod_scheme = MOD_SCHEME_DIFFERENTIAL;
				break;

			case '3':
				/* 8PSK_D */
				uc_mod_type = MOD_TYPE_8PSK;
				uc_mod_scheme = MOD_SCHEME_DIFFERENTIAL;
				if (uc_phy_band == ATPL360_WB_ARIB) {
					/* 8PSK Modulation Type not supported in ARIB band */
					printf("\r\n8PSK modulation not supported in ARIB band. Skipping configuration\r\n");
					b_valid_modulation = false;
				}

				break;

			case '4':
				/* BPSK_ROBO_C */
				uc_mod_type = MOD_TYPE_BPSK_ROBO;
				uc_mod_scheme = MOD_SCHEME_COHERENT;
				if (uc_phy_band == ATPL360_WB_ARIB) {
					/* Coherent Modulation Scheme not supported in ARIB band */
					printf("\r\nCoherent modulation not supported in ARIB band. Skipping configuration\r\n");
					b_valid_modulation = false;
				}

				break;

			case '5':
				/* BPSK_C */
				uc_mod_type = MOD_TYPE_BPSK;
				uc_mod_scheme = MOD_SCHEME_COHERENT;
				if (uc_phy_band == ATPL360_WB_ARIB) {
					/* Coherent Modulation Scheme not supported in ARIB band */
					printf("\r\nCoherent modulation not supported in ARIB band. Skipping configuration\r\n");
					b_valid_modulation = false;
				}

				break;

			case '6':
				/* QPSK_C */
				uc_mod_type = MOD_TYPE_QPSK;
				uc_mod_scheme = MOD_SCHEME_COHERENT;
				if (uc_phy_band == ATPL360_WB_ARIB) {
					/* Coherent Modulation Scheme not supported in ARIB band */
					printf("\r\nCoherent modulation not supported in ARIB band. Skipping configuration\r\n");
					b_valid_modulation = false;
				}

				break;

			case '7':
				/* 8PSK_C */
				uc_mod_type = MOD_TYPE_8PSK;
				uc_mod_scheme = MOD_SCHEME_COHERENT;
				if (uc_phy_band == ATPL360_WB_ARIB) {
					/* Coherent Modulation Scheme not supported in ARIB band */
					printf("\r\nCoherent modulation not supported in ARIB band. Skipping configuration\r\n");
					b_valid_modulation = false;
				}

				break;

			default:
				printf("\r\nUnknown command. Skipping configuration\r\n");
				b_valid_modulation = false;
				break;
			}

			if (b_valid_modulation) {
				/* Set new Modulation Type & Modulation Scheme. Get maximum data length allowed with configured Tx Parameters */
				phy_ctl_set_mod_scheme(uc_mod_scheme);
				sus_max_data_len = phy_ctl_set_mod_type(uc_mod_type);
				_show_tx_parameters(uc_mod_scheme, uc_mod_type);
			}

			suc_app_state = APP_STATE_IDLE;
			_show_menu(false);
			break;

#if defined(CONF_MULTIBAND_FCC_CENA)
		case APP_STATE_CONFIG_BAND:
			b_valid_band = true;
			switch (uc_char) {
			case '1':
				/* CENELEC-A */
				uc_band = ATPL360_WB_CENELEC_A;
				break;

			case '2':
				/* FCC */
				uc_band = ATPL360_WB_FCC;
				break;

			default:
				printf("\r\nUnknown command. Skipping configuration\r\n");
				b_valid_band = false;
				break;
			}

			if (b_valid_band) {
				/* Set new Band on PL360 */
				b_valid_band = phy_ctl_set_band(uc_band);
			}

			suc_app_state = APP_STATE_IDLE;

			if (!b_valid_band) {
				_show_menu(false);
			}

			break;

#elif defined(CONF_MULTIBAND_FCC_CENB)
		case APP_STATE_CONFIG_BAND:
			b_valid_band = true;
			switch (uc_char) {
			case '1':
				/* CENELEC-B */
				uc_band = ATPL360_WB_CENELEC_B;
				break;

			case '2':
				/* FCC */
				uc_band = ATPL360_WB_FCC;
				break;

			default:
				printf("\r\nUnknown command. Skipping configuration\r\n");
				b_valid_band = false;
				break;
			}

			if (b_valid_band) {
				/* Set new Band on PL360 */
				b_valid_band = phy_ctl_set_band(uc_band);
			}

			suc_app_state = APP_STATE_IDLE;

			if (!b_valid_band) {
				_show_menu(false);
			}

			break;
#endif
		}
	}
}
