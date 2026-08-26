/**
 * \file
 *
 * \brief APP_PLC : Module to manage PL360 G3 MAC RT Layer.
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
#include "app_plc.h"
#include "coup_pl360_wrp.h"

/* Time in ms that LED is ON after message reception */
#define COUNT_MS_IND_LED        50

/* ATPL360 descriptor definition */
static atpl360_descriptor_t sx_atpl360_desc;

/* Exception counters */
static uint8_t suc_err_unexpected;
static uint8_t suc_err_critical;
static uint8_t suc_err_reset;
static uint8_t suc_err_none;
static bool sb_exception_pend;
static bool sb_enabling_pl360;

/* G3 Band identifier */
static uint8_t suc_phy_band;

/* G3 Extended Address */
static uint8_t spuc_extended_address[8] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};

/* G3 PIB management */
static struct TMacRtPibValue sx_g3_pib;

/* G3 Transmission management */
static uint8_t spuc_tx_buffer[MAC_RT_MAX_PAYLOAD_SIZE];
static struct TMacRtMhr sx_tx_mac_header;

/* G3 Reception management */
static struct TMacRtRxParameters sx_rx_parameters;

#ifdef CONF_TONEMASK_STATIC_NOTCHING
#define TONE_MASK_SIZE   (PROTOCOL_CARRIERS_MAX + 7) / 8
/* Tone Mask (Static Notching) array. Each carrier corresponding to the band can be notched (no energy is sent in those carriers) */
/* Each carrier is represented by one byte (0: carrier used; 1: carrier notched). By default it is all 0's in PL360 device */
/* The same Tone Mask must be set in both transmitter and receiver. Otherwise they don't understand each other */
static const uint8_t spuc_tone_mask[TONE_MASK_SIZE] = TONE_MASK_STATIC_NOTCHING;
#endif

/* LED management variables */
extern uint32_t sul_ind_count_ms;

static void _init_random(void)
{
#ifdef TRNG
	/* Configure PMC */
	pmc_enable_periph_clk(ID_TRNG);

#if (!PIC32CX)
	/* Enable TRNG */
	trng_enable(TRNG);
#else
	/* Get peripheral clock frequency */
	uint32_t ul_cpuhz = sysclk_get_peripheral_hz();

	/* Enable TRNG */
	trng_enable(TRNG, ul_cpuhz);
#endif
#else
	uint32_t ul_random_num;

	ul_random_num = DWT->CYCCNT;

	/* Initialize seed */
	srand(ul_random_num);
#endif
}

static uint32_t _get_random_32(void)
{
#ifdef TRNG
	while ((trng_get_interrupt_status(TRNG) & TRNG_ISR_DATRDY) != TRNG_ISR_DATRDY) {
	}

	return trng_read_output_data(TRNG);

#else
	uint32_t ul_random_num;

	ul_random_num = rand() & 0xFF;
	ul_random_num = (ul_random_num << 8) | (rand() & 0xFF);
	ul_random_num = (ul_random_num << 8) | (rand() & 0xFF);
	ul_random_num = (ul_random_num << 8) | (rand() & 0xFF);

	return ul_random_num;
#endif
}

/**
 * \brief Set PL360 configuration. Called at initialization (once the binary is loaded) to configure required parameters in PL360 device
 */
static void _set_pl360_configuration(void)
{
	uint8_t uc_band_coup;

	/********* The following lines show how to configure different parameters on PL360 device *********/
	/********* The user can customize it depending on the requirements ********************************/

	/* Select band for pl360_g3_coup_tx_config() */
	switch (suc_phy_band) {
	case ATPL360_WB_CENELEC_A:
		uc_band_coup = COUP_BAND_CEN_A;
		break;

	case ATPL360_WB_CENELEC_B:
		uc_band_coup = COUP_BAND_CEN_B;
		break;

	case ATPL360_WB_FCC:
		uc_band_coup = COUP_BAND_FCC;
		break;

	case ATPL360_WB_ARIB:
		uc_band_coup = COUP_BAND_ARIB;
		break;

	default:
		uc_band_coup = COUP_BAND_CEN_A;
		break;
	}

	/* Configure Coupling and TX parameters */
	pl360_g3_coup_tx_config(&sx_atpl360_desc, uc_band_coup);

	/* Force Transmission to VLO mode by default in order to maximize signal level in anycase */
	/* Disable autodetect mode */
	sx_g3_pib.m_au8Value[0] = 0;
	sx_g3_pib.m_u8Length = 1;
	sx_atpl360_desc.set_req(MAC_RT_PIB_MANUF_PHY_PARAM, ATPL360_PHY_CFG_AUTODETECT_IMPEDANCE, &sx_g3_pib);
	/* Set VLO mode */
	sx_g3_pib.m_au8Value[0] = 2;
	sx_g3_pib.m_u8Length = 1;
	sx_atpl360_desc.set_req(MAC_RT_PIB_MANUF_PHY_PARAM, ATPL360_PHY_CFG_IMPEDANCE, &sx_g3_pib);

#ifdef CONF_TONEMASK_STATIC_NOTCHING
	/* Example to configure Tone Mask (Static Notching). Each carrier corresponding to the band can be notched (no energy is sent in those carriers) */
	/* Each carrier is represented by one byte (0: carrier used; 1: carrier notched). By default it is all 0's in PL360 device */
	/* The length is the number of carriers corresponding to the band in use (see "general_defs.h") */
	/* The same Tone Mask must be set in both transmitter and receiver. Otherwise they don't understand each other */
	memcpy(sx_g3_pib.m_au8Value, (uint8_t *)spuc_tone_mask, TONE_MASK_SIZE);
	sx_g3_pib.m_u8Length = TONE_MASK_SIZE;
	sx_atpl360_desc.set_req(MAC_RT_PIB_TONE_MASK, 0, &sx_g3_pib);
#endif
}

/**
 * \brief Setup G3 MAC Real-Time header
 */
static void _setup_mac_rt_header(void)
{
	memset(&sx_tx_mac_header, 0, sizeof(sx_tx_mac_header));

	/* MAC HEADER. Set G3 header info */
	sx_tx_mac_header.m_Fc.m_nAckRequest = CONF_ACK_REQUEST;
	/* MAC HEADER. Set DATA message. This example application only use this kind of frame type. */
	sx_tx_mac_header.m_Fc.m_nFrameType = MAC_RT_FRAME_TYPE_DATA;
	/* MAC HEADER. Disable Security */
	sx_tx_mac_header.m_Fc.m_nSecurityEnabled = 0;
	/* MAC HEADER. Set Sequence number. It should be incremented after each transmission. */
	sx_tx_mac_header.m_u8SequenceNumber = (uint8_t)_get_random_32();

	/* MAC HEADER. Set SHORT addressing mode: Destination address */
	sx_tx_mac_header.m_DestinationAddress.m_eAddrMode = MAC_RT_ADDRESS_MODE_SHORT;
	sx_tx_mac_header.m_Fc.m_nDestAddressingMode = MAC_RT_ADDRESS_MODE_SHORT;

	/* MAC HEADER. Set SHORT addressing mode: Source address */
	sx_tx_mac_header.m_SourceAddress.m_eAddrMode = MAC_RT_ADDRESS_MODE_SHORT;
	sx_tx_mac_header.m_Fc.m_nSrcAddressingMode = MAC_RT_ADDRESS_MODE_SHORT;

	/* PL360 Set Addresses: Source and Destination */
	app_plc_set_mac_src_address((uint16_t)_get_random_32());
	app_plc_set_mac_dst_address(MAC_RT_SHORT_ADDRESS_BROADCAST);

	/* PL360 Set Personal Area Network Identification (PAN ID). See "conf_project.h" file. */
	app_plc_set_mac_panid(CONF_PAN_ID);

	printf("\r\nConfiguring G3 MAC RT Header");
	printf("\r\nG3 MAC PAN ID: 0x%04X", sx_tx_mac_header.m_nDestinationPanIdentifier);
	printf("\r\nG3 MAC Source Address: 0x%04X", sx_tx_mac_header.m_SourceAddress.m_nShortAddress);
	printf("\r\nG3 MAC Destination Address: 0x%04X\r\n", (unsigned int)sx_tx_mac_header.m_DestinationAddress.m_nShortAddress);
}

/**
 * \brief Handler to manage confirmation of the last PLC transmission.
 *
 * \param eStatus Trasnsmission result
 * \param bUpdateTimestamp Flag to indicate whether timestamp has to be updated
 *
 */
static void _handler_data_cfm(enum EMacRtStatus eStatus, bool bUpdateTimestamp)
{
	/* Update Sequence Number for next transmission */
	sx_tx_mac_header.m_u8SequenceNumber++;

	switch (eStatus) {
	case MAC_RT_STATUS_SUCCESS:
		printf("...MAC_RT_STATUS_SUCCESS");
		break;

	case MAC_RT_STATUS_CHANNEL_ACCESS_FAILURE:
		printf("...MAC_RT_STATUS_CHANNEL_ACCESS_FAILURE");
		break;

	case MAC_RT_STATUS_NO_ACK:
		printf("...MAC_RT_STATUS_NO_ACK");
		break;

	case MAC_RT_STATUS_DENIED:
		printf("...MAC_RT_STATUS_DENIED");
		break;

	case MAC_RT_STATUS_INVALID_INDEX:
		printf("...MAC_RT_STATUS_INVALID_INDEX");
		break;

	case MAC_RT_STATUS_INVALID_PARAMETER:
		printf("...MAC_RT_STATUS_INVALID_PARAMETER");
		break;

	case MAC_RT_STATUS_TRANSACTION_OVERFLOW:
		printf("...MAC_RT_STATUS_TRANSACTION_OVERFLOW");
		break;

	case MAC_RT_STATUS_UNSUPPORTED_ATTRIBUTE:
		printf("...MAC_RT_STATUS_UNSUPPORTED_ATTRIBUTE");
		break;

	default:
		/* Unexpeted result */
		printf(" MAC RT UNEXPECTED STATUS");
		break;
	}

	printf(" UpdateTimestamp: %u", (unsigned int)bUpdateTimestamp);

	app_console_handle_tx_cfm();
}

static uint8_t _get_header_info(uint8_t *pFrame, uint16_t *us_src_address, uint16_t *us_dest_address)
{
	uint8_t *pData;
	uint16_t address;

	/* Frame Struct :
		* Frame Control(2b) + Seq Num(1b) + Dest PAN ID(2b) + Dest Addr(2b) + Src Addr(2b) */

	pData = pFrame;
	pData += 2;

	pData++; /* Sequence number */

	pData += 2; /* PAN ID */

	/* Destination address */
	address = *pData++;
	address += (uint16_t)*pData++ << 8;
	*us_dest_address = address;

	/* panIdCompression = 1 -> No Source PAN ID */

	/* Source address */
	address = *pData++;
	address += (uint16_t)*pData++ << 8;
	*us_src_address = address;

	/* Return Header length */
	return (uint8_t)(pData - pFrame);
}

/**
 * \brief Handler to manage data from a new received message.
 *
 * \param pMrtsdu Pointer to message data
 * \param u16MrtsduLen Length of the message data in bytes
 *
 */
static void _handler_new_data_received(uint8_t *pMrtsdu, uint16_t u16MrtsduLen)
{
	uint8_t *puc_data_buf;
	uint8_t uc_header_length;
	uint16_t us_src_address, us_dest_address;

#ifdef LED1_GPIO
	/* Turn on LED1 to indicate the reception of PLC message */
	sul_ind_count_ms = COUNT_MS_IND_LED;
	LED_On(LED1);
#endif

	/* Look for data. Skip MAC Header */
	puc_data_buf = pMrtsdu;
	uc_header_length = _get_header_info(puc_data_buf, &us_src_address, &us_dest_address);
	puc_data_buf += uc_header_length;

	/* Send Data to Console Application */
	app_console_handle_rx_msg(puc_data_buf, u16MrtsduLen - uc_header_length, sx_rx_parameters.m_eModulationScheme,
		sx_rx_parameters.m_eModulationType, sx_rx_parameters.m_u8PpduLinkQuality, us_src_address, us_dest_address);
}

/**
 * \brief Handler to manage parameters from a new received message.
 *
 * \param pParameters Pointer to struct containing message parameters
 *
 */
static void _handler_new_parameters_received(struct TMacRtRxParameters *pParameters)
{
	/* Capture parameters of the new received message */
	sx_rx_parameters = *pParameters;
}

/**
 * \brief Handler to manage PL360 Exceptions. This callback is also called after loading binary at initization
 */
static void _handler_exception_event(atpl360_exception_t exception)
{
	printf("\r\n");

	switch (exception) {
	case ATPL360_EXCEPTION_UNEXPECTED_SPI_STATUS:
		/* SPI has detected an unexpected status, reset is recommended */
		suc_err_unexpected++;
		printf("ATPL360_EXCEPTION_UNEXPECTED_SPI_STATUS\r\n");
		break;

	case ATPL360_EXCEPTION_SPI_CRITICAL_ERROR:
		/* SPI critical error */
		suc_err_critical++;
		printf("ATPL360_EXCEPTION_SPI_CRITICAL_ERROR\r\n");
		break;

	case ATPL360_EXCEPTION_RESET:
		/* Device Reset */
		if (sb_enabling_pl360) {
			/* This callback is also called after loading binary at initization */
			/* This message is shown to indicate that the followitn exception is normal because PL360 binary has just been loaded */
			sb_enabling_pl360 = false;
			printf("PL360 initialization event: ");
		}

		suc_err_reset++;
		printf("ATPL360_EXCEPTION_RESET\r\n");
		break;

	default:
		printf("ATPL360_EXCEPTION_UNKNOWN\r\n");
		suc_err_none++;
	}

	/* Set flag to manage exception in app_plc_process() */
	sb_exception_pend = true;
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

#if defined(CONF_MULTIBAND_FCC_CENA)
	/* Multiband: FCC and CENELEC-A binaries are linked */
	if (suc_phy_band == ATPL360_WB_FCC) {
		/* Select FCC binary */
#    if defined (__CC_ARM)
		extern uint8_t atpl_bin_fcc_start[];
		extern uint8_t atpl_bin_fcc_end[];
		ul_bin_addr = (int)(atpl_bin_fcc_start - 1);
		puc_bin_start = atpl_bin_fcc_start - 1;
		puc_bin_end = atpl_bin_fcc_end;
#    elif defined (__GNUC__)
		extern uint8_t atpl_bin_fcc_start;
		extern uint8_t atpl_bin_fcc_end;
		ul_bin_addr = (int)&atpl_bin_fcc_start;
		puc_bin_start = (uint8_t *)&atpl_bin_fcc_start;
		puc_bin_end = (uint8_t *)&atpl_bin_fcc_end;
#    elif defined (__ICCARM__)
#      pragma section = "P_atpl_bin_fcc"
		extern uint8_t atpl_bin_fcc;
		ul_bin_addr = (int)&atpl_bin_fcc;
		puc_bin_start = __section_begin("P_atpl_bin_fcc");
		puc_bin_end = __section_end("P_atpl_bin_fcc");
#    else
#      error This compiler is not supported for now.
#    endif
	} else { /* ATPL360_WB_CENELEC_A */
		 /* Select CENELEC-A binary */
#    if defined (__CC_ARM)
		extern uint8_t atpl_bin_cena_start[];
		extern uint8_t atpl_bin_cena_end[];
		ul_bin_addr = (int)(atpl_bin_cena_start - 1);
		puc_bin_start = atpl_bin_cena_start - 1;
		puc_bin_end = atpl_bin_cena_end;
#    elif defined (__GNUC__)
		extern uint8_t atpl_bin_cena_start;
		extern uint8_t atpl_bin_cena_end;
		ul_bin_addr = (int)&atpl_bin_cena_start;
		puc_bin_start = (int)&atpl_bin_cena_start;
		puc_bin_end = (int)&atpl_bin_cena_end;
#    elif defined (__ICCARM__)
#      pragma section = "P_atpl_bin_cena"
		extern uint8_t atpl_bin_cena;
		ul_bin_addr = (int)&atpl_bin_cena;
		puc_bin_start = __section_begin("P_atpl_bin_cena");
		puc_bin_end = __section_end("P_atpl_bin_cena");
#    else
#      error This compiler is not supported for now.
#    endif
	}

#elif defined(CONF_MULTIBAND_FCC_CENB)
	/* Multiband: FCC and CENELEC-B binaries are linked */
	if (suc_phy_band == ATPL360_WB_FCC) {
		/* Select FCC binary */
#    if defined (__CC_ARM)
		extern uint8_t atpl_bin_fcc_start[];
		extern uint8_t atpl_bin_fcc_end[];
		ul_bin_addr = (int)(atpl_bin_fcc_start - 1);
		puc_bin_start = atpl_bin_fcc_start - 1;
		puc_bin_end = atpl_bin_fcc_end;
#    elif defined (__GNUC__)
		extern uint8_t atpl_bin_fcc_start;
		extern uint8_t atpl_bin_fcc_end;
		ul_bin_addr = (int)&atpl_bin_fcc_start;
		puc_bin_start = (uint8_t *)&atpl_bin_fcc_start;
		puc_bin_end = (uint8_t *)&atpl_bin_fcc_end;
#    elif defined (__ICCARM__)
#      pragma section = "P_atpl_bin_fcc"
		extern uint8_t atpl_bin_fcc;
		ul_bin_addr = (int)&atpl_bin_fcc;
		puc_bin_start = __section_begin("P_atpl_bin_fcc");
		puc_bin_end = __section_end("P_atpl_bin_fcc");
#    else
#      error This compiler is not supported for now.
#    endif
	} else { /* ATPL360_WB_CENELEC_B */
		 /* Select CENELEC-B binary */
#    if defined (__CC_ARM)
		extern uint8_t atpl_bin_cenb_start[];
		extern uint8_t atpl_bin_cenb_end[];
		ul_bin_addr = (int)(atpl_bin_cenb_start - 1);
		puc_bin_start = atpl_bin_cenb_start - 1;
		puc_bin_end = atpl_bin_cenb_end;
#    elif defined (__GNUC__)
		extern uint8_t atpl_bin_cenb_start;
		extern uint8_t atpl_bin_cenb_end;
		ul_bin_addr = (int)&atpl_bin_cenb_start;
		puc_bin_start = (int)&atpl_bin_cenb_start;
		puc_bin_end = (int)&atpl_bin_cenb_end;
#    elif defined (__ICCARM__)
#      pragma section = "P_atpl_bin_cenb"
		extern uint8_t atpl_bin_cenb;
		ul_bin_addr = (int)&atpl_bin_cenb;
		puc_bin_start = __section_begin("P_atpl_bin_cenb");
		puc_bin_end = __section_end("P_atpl_bin_cenb");
#    else
#      error This compiler is not supported for now.
#    endif
	}

#else /* ifdef CONF_MULTIBAND_FCC_CENA */
	/* Only one binary linked */
#  if defined (__CC_ARM)
	extern uint8_t atpl_bin_start[];
	extern uint8_t atpl_bin_end[];
	ul_bin_addr = (int)(atpl_bin_start - 1);
	puc_bin_start = atpl_bin_start - 1;
	puc_bin_end = atpl_bin_end;
#  elif defined (__GNUC__)
	extern uint8_t atpl_bin_start;
	extern uint8_t atpl_bin_end;
	ul_bin_addr = (int)&atpl_bin_start;
	puc_bin_start = (int)&atpl_bin_start;
	puc_bin_end = (int)&atpl_bin_end;
#  elif defined (__ICCARM__)
#    pragma section = "P_atpl_bin"
	extern uint8_t atpl_bin;
	ul_bin_addr = (int)&atpl_bin;
	puc_bin_start = __section_begin("P_atpl_bin");
	puc_bin_end = __section_end("P_atpl_bin");
#  else
#    error This compiler is not supported for now.
#  endif
#endif
	*pul_address = ul_bin_addr;
	/* cppcheck-suppress deadpointer */
	return ((uint32_t)puc_bin_end - (uint32_t)puc_bin_start);
}

/**
 * \brief Enables PL360 device. Load PHY binary.
 *
 */
static void _pl360_enable(void)
{
	uint32_t ul_bin_addr;
	uint32_t ul_bin_size;
	uint8_t uc_ret;

	/* Get PL360 binary address and size */
	ul_bin_size = _get_pl360_bin_addressing(&ul_bin_addr);

	/* Enable PL360: Load binary */
	printf("\r\nEnabling PL360 device: Loading PHY binary\r\n");
	sb_enabling_pl360 = true;
	uc_ret = atpl360_enable(ul_bin_addr, ul_bin_size);
	if (uc_ret == ATPL360_ERROR) {
		printf("\r\nCRITICAL ERROR: PL360 binary load failed (%d)\r\n", uc_ret);
		while (1) {
		}
	}

	printf("\r\nPL360 binary loaded correctly\r\n");

	/* Get PHY version */
	sx_atpl360_desc.get_req(MAC_RT_PIB_MANUF_PHY_PARAM, PHY_PARAM_VERSION, &sx_g3_pib);

	printf("PHY version: 0x%08x (", *(uint32_t *)sx_g3_pib.m_au8Value);

	/* sx_g3_pib.m_au8Value[2] correspons to G3 band [0x01: CEN-A, 0x02: FCC, 0x03: ARIB, 0x04: CEN-B] */
	switch (sx_g3_pib.m_au8Value[2]) {
	case ATPL360_WB_CENELEC_A:
		printf("CENELEC-A band: 35 - 91 kHz)\r\n");
		break;

	case ATPL360_WB_FCC:
		printf("FCC band: 154 - 488 kHz)\r\n");
		break;

	case ATPL360_WB_ARIB:
		printf("ARIB band: 154 - 404 kHz)\r\n");
		break;

	case ATPL360_WB_CENELEC_B:
		printf("CENELEC-B band: 98 - 122 kHz)\r\n");
		break;

	default:
		printf("Unknown band)\r\n");
		break;
	}

	if (sx_g3_pib.m_au8Value[2] != suc_phy_band) {
		printf("ERROR: PHY band does not match with band configured in application\r\n");
	}
}

/**
 * \brief Initialization of PL360 G3 MAC RT.
 *
 */
void app_plc_init(void)
{
	atpl360_dev_callbacks_t x_atpl360_cbs;
	atpl360_hal_wrapper_t x_atpl360_hal_wrp;

	/* Initialize G3 band static variable (ATPL360_WB defined in conf_atpl360.h) */
	suc_phy_band = ATPL360_WB;

	/* Initialize PL360 controller */
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

	/* Callback functions configuration. Set NULL as Not used */
	x_atpl360_cbs.tx_cfm = _handler_data_cfm;
	x_atpl360_cbs.data_ind = _handler_new_data_received;
	x_atpl360_cbs.rx_parameters_ind = _handler_new_parameters_received;
	x_atpl360_cbs.exception_cb = _handler_exception_event;
	x_atpl360_cbs.sleep_mode_cb = NULL;
	x_atpl360_cbs.debug_mode_cb = NULL;
	x_atpl360_cbs.mac_sniffer_ind = NULL;
	x_atpl360_cbs.comm_status_ind = NULL;
	sx_atpl360_desc.set_callbacks(&x_atpl360_cbs);

	/* Enable PL360 device: Load binary */
	_pl360_enable();

	/* Enable Random Generator */
	_init_random();
}

/**
 * \brief Builds Mac RT header in Tx buffer.
 *
 * \param pFrame Pointer to transmission buffer.
 * \param pHeader Pointer to Mac RT header.
 *
 */
static uint8_t _build_mac_rt_header(uint8_t *pFrame, struct TMacRtMhr *pHeader)
{
	uint8_t *pData;

	pData = pFrame;

	/* Copy Frame Control and Sequence number */
	memcpy(pData, pHeader, 3);
	pData += 3;
	/* Build Header to use MAC_RT_SHORT_ADDRESS mode */
	/* Destination PAN ID */
	*pData++ = (uint8_t)(pHeader->m_nDestinationPanIdentifier);
	*pData++ = (uint8_t)(pHeader->m_nDestinationPanIdentifier >> 8);
	/* Destination address */
	*pData++ = (uint8_t)(pHeader->m_DestinationAddress.m_nShortAddress);
	*pData++ = (uint8_t)(pHeader->m_DestinationAddress.m_nShortAddress >> 8);
	/* panIdCompression = 1 -> No Source PAN ID */
	/* Source Address */
	*pData++ = (uint8_t)(pHeader->m_SourceAddress.m_nShortAddress);
	*pData++ = (uint8_t)(pHeader->m_SourceAddress.m_nShortAddress >> 8);

	/* Return Header length */
	return (uint8_t)(pData - pFrame);
}

/**
 * \brief Send PLC message.
 *
 * \param puc_data_buff Pointer to buffer containing the data to send.
 * \param us_data_len Data length in bytes.
 *
 */
bool app_plc_send_msg(uint8_t *puc_data_buff, uint16_t us_data_len)
{
	uint8_t *puc_data;
	uint8_t uc_header_len;

	if (us_data_len > MAC_RT_MAX_PAYLOAD_SIZE) {
		return false;
	}

	/* Build MAC RT DATA Frame */
	puc_data = spuc_tx_buffer;

	/* Build MAC RT Frame */
	uc_header_len = _build_mac_rt_header(puc_data, &sx_tx_mac_header);
	puc_data += uc_header_len;

	/* Fill Payload */
	memcpy(puc_data, puc_data_buff, us_data_len);
	puc_data += us_data_len;

	/* Send MAC RT Frame */
	sx_atpl360_desc.tx_request(spuc_tx_buffer, puc_data - spuc_tx_buffer);

	printf("\r\nTx (%u bytes): ", (unsigned int)(puc_data - spuc_tx_buffer));

	return true;
}

#if defined(CONF_MULTIBAND_FCC_CENA)

/**
 * \brief Change G3 band. In this example it is only possible to change between CENELEC-A and FCC.
 * CONF_MULTIBAND_FCC_CENA must be defined to enable Multiband capability.
 * PLCOUP011 should be used for Multiband purposes (Branch 0 for CENELEC-A and Branch 1 for FCC)
 *
 * \param uc_band Band to be configured (see band definitions in "general_defs.h").
 *
 * \retval false. Invalid configuration
 * \retval true. Band reconfigured successfully
 */
bool app_plc_set_band(uint8_t uc_band)
{
	if (uc_band == suc_phy_band) {
		printf("\r\nBand has not changed: Skipping Band Reconfiguration\r\n");
		return false;
	}

	if ((uc_band == ATPL360_WB_CENELEC_A) || (uc_band == ATPL360_WB_FCC)) {
		/* Reset PL360 device to load new binary */
		atpl360_disable();

		/* Set static Band variable */
		suc_phy_band = uc_band;

		/* Enable PL360 device: Load binary to change Band */
		_pl360_enable();

		return true;
	} else {
		printf("Band not supported: Skipping Band Configuration\r\n");
		return false;
	}
}

#elif defined(CONF_MULTIBAND_FCC_CENB)

/**
 * \brief Change G3 band. In this example it is only possible to change between CENELEC-B and FCC.
 * CONF_MULTIBAND_FCC_CENB must be defined to enable Multiband capability.
 *
 * \param uc_band Band to be configured (see band definitions in "general_defs.h").
 *
 * \retval false. Invalid configuration
 * \retval true. Band reconfigured successfully
 */
bool app_plc_set_band(uint8_t uc_band)
{
	if (uc_band == suc_phy_band) {
		printf("\r\nBand has not changed: Skipping Band Reconfiguration\r\n");
		return false;
	}

	if ((uc_band == ATPL360_WB_CENELEC_B) || (uc_band == ATPL360_WB_FCC)) {
		/* Reset PL360 device to load new binary */
		atpl360_disable();

		/* Set static Band variable */
		suc_phy_band = uc_band;

		/* Enable PL360 device: Load binary to change Band */
		_pl360_enable();

		return true;
	} else {
		printf("Band not supported: Skipping Band Configuration\r\n");
		return false;
	}
}

#endif

/**
 * \brief Get current G3 band configured.
 *
 * \return Current G3 band configured.
 */
uint8_t app_plc_get_phy_band(void)
{
	/* Return G3 band configured */
	return suc_phy_band;
}

/**
 * \brief Get current G3 MAC Source Address.
 *
 * \return Current MAC Source Address.
 */
uint16_t app_plc_get_mac_src_address(void)
{
	/* Return G3 Source Address */
	return sx_tx_mac_header.m_SourceAddress.m_nShortAddress;
}

/**
 * \brief Get current G3 MAC Destination Address.
 *
 * \return Current MAC Destination Address.
 */
uint16_t app_plc_get_mac_dst_address(void)
{
	/* Return G3 Destination Address */
	return (uint16_t)sx_tx_mac_header.m_DestinationAddress.m_nShortAddress;
}

/**
 * \brief Get current ACK request flag.
 *
 * \return Current ACK request flag.
 */
uint8_t app_plc_get_ack_request(void)
{
	/* Return ACK flag */
	return (uint8_t)sx_tx_mac_header.m_Fc.m_nAckRequest;
}

/**
 * \brief Set G3 MAC Source Address.
 *
 * \parameter address MAC Source Address.
 */
void app_plc_set_mac_src_address(uint16_t address)
{
	/* Set Source Address */
	sx_tx_mac_header.m_SourceAddress.m_nShortAddress = address;

	/* Set PIB Short Address */
	memcpy(sx_g3_pib.m_au8Value, (uint8_t *)&sx_tx_mac_header.m_SourceAddress.m_nShortAddress, 2);
	sx_g3_pib.m_u8Length = 2;
	sx_atpl360_desc.set_req(MAC_RT_PIB_SHORT_ADDRESS, 0, &sx_g3_pib);

	/* Update Extended Address */
	spuc_extended_address[6] = (uint8_t)(address >> 8);
	spuc_extended_address[7] = (uint8_t)address;

	/* Set PIB Extended Address */
	memcpy(sx_g3_pib.m_au8Value, (uint8_t *)spuc_extended_address, 8);
	sx_g3_pib.m_u8Length = 8;
	sx_atpl360_desc.set_req(MAC_RT_PIB_MANUF_EXTENDED_ADDRESS, 0, &sx_g3_pib);
}

/**
 * \brief Set G3 MAC Personal Area Network Identifier (PAN ID).
 *
 * \parameter panid Personal Area Network Identifier (PAN ID)
 */
void app_plc_set_mac_panid(uint16_t pan_id)
{
	/* Set PanId */
	sx_tx_mac_header.m_nSourcePanIdentifier = pan_id;
	sx_tx_mac_header.m_nDestinationPanIdentifier = pan_id;

	/* Set PIB PanId */
	memcpy(sx_g3_pib.m_au8Value, (uint8_t *)&sx_tx_mac_header.m_nSourcePanIdentifier, 2);
	sx_g3_pib.m_u8Length = 2;
	sx_atpl360_desc.set_req(MAC_RT_PIB_PAN_ID, 0, &sx_g3_pib);
}

/**
 * \brief Set G3 MAC Destination Address.
 *
 * \parameter address MAC Destination Address.
 */
void app_plc_set_mac_dst_address(uint16_t address)
{
	/* Set Destination Address */
	sx_tx_mac_header.m_DestinationAddress.m_nShortAddress = address;
}

/**
 * \brief Set G3 ACK request flag.
 *
 * \parameter ack_request ACK request flag.
 */
void app_plc_set_ack_request(uint8_t ack_request)
{
	/* Set ACK flag */
	sx_tx_mac_header.m_Fc.m_nAckRequest = ack_request;
}

/**
 * \brief Phy Controller module process.
 */
void app_plc_process(void)
{
	/* Manage PL360 exceptions. At initialization ATPL360_EXCEPTION_RESET is reported */
	if (sb_exception_pend) {
		/* Clear exception flag */
		sb_exception_pend = false;

		/* Set PL360 specific configuration from application */
		/* Called at initialization and if an exception ocurrs */
		/* If an exception occurs, PL360 is reset and some parameters may have to be reconfigured */
		_set_pl360_configuration();

		/* Setup G3 MAC RT header */
		_setup_mac_rt_header();

		/* Initialize Chat App */
		app_console_handle_pl360_reset();
	}

	/* Check ATPL360 pending events. It must be called from application periodically to handle G3 MAC RT events */
	atpl360_handle_events();
}
