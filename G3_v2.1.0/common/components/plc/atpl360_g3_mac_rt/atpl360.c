/**
 * \file
 *
 * \brief Management of the ATPL360 PLC transceiver.
 * This file manages the accesses to the ATPL360 component.
 *
 * Copyright (c) 2018 Atmel Corporation. All rights reserved.
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

#include "compiler.h"

/* ATPL360 includes */
#include "conf_atpl360.h"
#include "atpl360_hal_spi.h"
#include "atpl360_boot.h"
#include "atpl360.h"
#include "atpl360_comm.h"

/* HAL wrapper */
atpl360_hal_wrapper_t sx_atpl360_hal_wrapper;

/* Mac Rt includes */
#include "MacRt.h"
#include "MacRtDefs.h"
#include "MacRtMib.h"

#define ATPL360_MAX_STATUS_CHECK                  2

#define ATPL360_REG_CMD_RD                        0
#define ATPL360_REG_CMD_WR                        1

/* Buffer definition to communicate with ATPL360 */
static uint8_t spuc_tx_buf[div4_ceil(ATPL360_MAC_TX_PKT_SIZE) << 2];
static uint8_t spuc_data_ind_buf[div4_ceil(ATPL360_MAX_MAC_DATA_LENGTH) << 2];
static uint8_t spuc_mac_snf_buf[div4_ceil(ATPL360_MAX_MAC_DATA_LENGTH) << 2];
static uint8_t spuc_comm_status_buf[div4_ceil(ATPL360_MAC_COMM_STATUS_SIZE) << 2];
static uint8_t spuc_rx_par_buf[div4_ceil(ATPL360_MAC_RX_PAR_SIZE) << 2];
static uint8_t spuc_tx_cfm_buf[div4_ceil(ATPL360_MAC_TX_CMF_SIZE) << 2];
static uint8_t spuc_reg_buf[div4_ceil(ATPL360_REG_PKT_SIZE) << 2];

/* Flags to manage events */
static volatile bool sb_tx_cfm_event_enable;
static volatile bool sb_comm_status_event_enable;
static volatile bool sb_rx_par_ind_event_enable;
static volatile uint16_t sus_phy_snf_event_len;
static volatile uint16_t sus_data_ind_event_len;
static volatile uint16_t sus_mac_snf_event_len;
static volatile uint16_t sus_reg_event_len;

/* Device Enable/Disable Status */
static bool sb_component_enabled;

/* Sleep mode Status */
static bool sb_sleep_enabled;

/* Debug mode Status */
static bool sb_debug_enabled;
static uint32_t sul_dbg_addr;
static uint16_t sul_dbg_len;

/* Internal Handlers to manage ATPL360 event notifications */
static MacRtTxConfirm _tx_cfm_cb_handler;
static MacRtDataIndication _data_ind_cb_handler;
static MacRtSnifferIndication _mac_snf_cb_handler;
static MacRtCommStatusIndication _comm_status_cb_handler;
static MacRtRxParamsIndication _rx_par_ind_cb_handler;

static pf_exeption_event_t _exception_event_cb_handler;
static pf_handle_cb_t _sleep_mode_cb_handler;
static pf_handle_cb_t _debug_mode_cb_handler;

/* Callback for Phy Sniffer mode */
static pf_phy_snf_cb_t _phy_snf_cb_handler;
static uint8_t *spuc_phy_snf_buff;

/* SPI Commands to get/set spi data buffer */
#define SPI_RD_CMD      0
#define SPI_WR_CMD      1

/**
 * \brief Function to read/write through SPI
 *
 * \return  ATPL360_SUCCESS if there is no error, otherwise returns ATPL360_ERROR.
 */
static atpl360_res_t _spi_send_cmd(uint8_t uc_cmd, atpl360_spi_data_t *px_spi_data)
{
	uint8_t uc_cnt;
	atpl360_spi_status_t uc_status;

	if (uc_cmd == SPI_WR_CMD) {
		atpl360_spi_write_buf(px_spi_data);
	} else {
		atpl360_spi_read_buf(px_spi_data);
	}

	uc_cnt = ATPL360_MAX_STATUS_CHECK;
	uc_status = atpl360_spi_get_status();

	while (uc_status != ATPL360_SPI_STATUS_CORTEX) {
		if (uc_status == ATPL360_SPI_STATUS_UNKNOWN) {
			/* Safety condition in loop */
			if (_exception_event_cb_handler) {
				/* Report SPI Error to Application */
				_exception_event_cb_handler(ATPL360_EXCEPTION_UNEXPECTED_SPI_STATUS);
			}

			/* CRITICAL Safety condition in loop */
			if (!uc_cnt--) {
				if (_exception_event_cb_handler) {
					/* Report Critical SPI Error to Application */
					_exception_event_cb_handler(ATPL360_EXCEPTION_SPI_CRITICAL_ERROR);
				}

				uc_cnt = ATPL360_MAX_STATUS_CHECK;
				return ATPL360_ERROR;
			}

			/* Wait to system re-start */
			sx_atpl360_hal_wrapper.plc_delay(DELAY_TREF_MS, ATPL360_RST_WAIT_MS);

			/* ATPL360 hard reset */
			sx_atpl360_hal_wrapper.plc_reset();
		} else if (uc_status == ATPL360_SPI_STATUS_FW_VALIDATING) {
			/* Report Reset detection to Application */
			if (_exception_event_cb_handler) {
				_exception_event_cb_handler(ATPL360_EXCEPTION_RESET);
			}
		}

		/* Validate phase */
		if (uc_cmd == SPI_WR_CMD) {
			atpl360_spi_write_buf(px_spi_data);
		} else {
			atpl360_spi_read_buf(px_spi_data);
		}

		/* Update SPI status */
		uc_status = atpl360_spi_get_status();
	}

	return ATPL360_SUCCESS;
}

/**
 * \brief Function to get Interrupt info
 *
 * \return Interrupt info in case of ATPl360 is enable. 0 in otherwise
 */
static void _get_interrupt_info(atpl360_events_t *px_events_info)
{
	atpl360_spi_data_t x_spi_data;
	atpl360_spi_status_info_t x_spi_status_info;
	uint8_t *puc_info;
	uint8_t puc_int_buffer[ATPL360_EVENT_DATA_LENGTH] = {0};

	puc_info = puc_int_buffer;

	/* Read Time Ref and Event flags */
	x_spi_data.us_mem_id = ATPL360_STATUS_INFO_ID;
	x_spi_data.us_len = ATPL360_EVENT_DATA_LENGTH;
	x_spi_data.puc_data_buf = puc_info;
	if (_spi_send_cmd(SPI_RD_CMD, &x_spi_data) == ATPL360_SUCCESS) {
		/* Extract events info */
		atpl360_spi_get_status_info(&x_spi_status_info);
		atpl360_comm_set_event_info(px_events_info, x_spi_status_info.ul_flags);

		/* Extract Timer info */
		px_events_info->ul_timer_ref = ((uint32_t)*puc_info++);
		px_events_info->ul_timer_ref += ((uint32_t)*puc_info++) << 8;
		px_events_info->ul_timer_ref += ((uint32_t)*puc_info++) << 16;
		px_events_info->ul_timer_ref += ((uint32_t)*puc_info++) << 24;

		/* Extract Event Length info */
		px_events_info->us_data_ind_len = ((uint16_t)*puc_info++);
		px_events_info->us_data_ind_len += ((uint16_t)*puc_info++) << 8;
		px_events_info->us_mac_snf_ind_len = ((uint16_t)*puc_info++);
		px_events_info->us_mac_snf_ind_len += ((uint16_t)*puc_info++) << 8;
		px_events_info->us_phy_snf_ind_len = ((uint16_t)*puc_info++);
		px_events_info->us_phy_snf_ind_len += ((uint16_t)*puc_info++) << 8;
		px_events_info->us_reg_rsp_len = ((uint16_t)*puc_info++);
		px_events_info->us_reg_rsp_len += ((uint16_t)*puc_info++) << 8;
	} else {
		/* Handle error in getting events info */
		memset(px_events_info, 0, sizeof(px_events_info));
	}
}

/**
 * \brief External interrupt handler
 */
static void _handler_atpl360_ext_int(void)
{
	atpl360_events_t x_events_info;
	atpl360_spi_data_t x_spi_data;

	if (sb_component_enabled) {
		/* Time guard */
		sx_atpl360_hal_wrapper.plc_delay(DELAY_TREF_US, 20);

		/* capture information relative to ATPL360 events */
		_get_interrupt_info(&x_events_info);

		/* Check Tx Cfm event */
		if (x_events_info.b_tx_cfm_ev) {
			/* Read confirm message */
			x_spi_data.us_mem_id = ATPL360_TX_CFM_ID;
			x_spi_data.puc_data_buf = spuc_tx_cfm_buf;
			x_spi_data.us_len = ATPL360_MAC_TX_CMF_SIZE;
			if (_spi_send_cmd(SPI_RD_CMD, &x_spi_data) == ATPL360_SUCCESS) {
				sb_tx_cfm_event_enable = true;
			}
		}

		/* Check Data indication event */
		if (x_events_info.b_data_ind_ev) {
			/* Extract Data indication length from event info */
			x_spi_data.us_len = x_events_info.us_data_ind_len;
			if ((x_spi_data.us_len == 0) || (x_spi_data.us_len > ATPL360_MAX_MAC_DATA_LENGTH)) {
				x_spi_data.us_len = 1;
			}

			/* Read DATA from indication message */
			x_spi_data.us_mem_id = ATPL360_DATA_IND_ID;
			x_spi_data.puc_data_buf = spuc_data_ind_buf;
			if (_spi_send_cmd(SPI_RD_CMD, &x_spi_data) == ATPL360_SUCCESS) {
				sus_data_ind_event_len = x_spi_data.us_len;
			}
		}

		/* Check MAC Sniffer event */
		if (x_events_info.b_mac_snf_ev) {
			/* Extract MAC Sniffer length from event info */
			x_spi_data.us_len = x_events_info.us_mac_snf_ind_len;
			if ((x_spi_data.us_len == 0) || (x_spi_data.us_len > ATPL360_MAX_MAC_DATA_LENGTH)) {
				x_spi_data.us_len = 1;
			}

			/* Read MAC SNIFFER DATA from indication message */
			x_spi_data.us_mem_id = ATPL360_MAC_SNIF_ID;
			x_spi_data.puc_data_buf = spuc_mac_snf_buf;

			if (_spi_send_cmd(SPI_RD_CMD, &x_spi_data) == ATPL360_SUCCESS) {
				sus_mac_snf_event_len = x_spi_data.us_len;
			}
		}

		/* Check COMM STATUS event */
		if (x_events_info.b_comm_status_ev) {
			/* Read COMM STATUS DATA from indication message */
			x_spi_data.us_mem_id = ATPL360_COMM_STATUS_ID;
			x_spi_data.puc_data_buf = spuc_comm_status_buf;
			x_spi_data.us_len = ATPL360_MAC_COMM_STATUS_SIZE;
			if (_spi_send_cmd(SPI_RD_CMD, &x_spi_data) == ATPL360_SUCCESS) {
				sb_comm_status_event_enable = true;
			}
		}

		/* Check MAC LME indication event */
		if (x_events_info.b_rx_par_ind_ev) {
			/* Read COMM STATUS DATA from indication message */
			x_spi_data.us_mem_id = ATPL360_RX_PAR_IND_ID;
			x_spi_data.puc_data_buf = spuc_rx_par_buf;
			x_spi_data.us_len = ATPL360_MAC_RX_PAR_SIZE;
			if (_spi_send_cmd(SPI_RD_CMD, &x_spi_data) == ATPL360_SUCCESS) {
				sb_rx_par_ind_event_enable = true;
			}
		}

		/* Check REG_RSP_EV_TYPE event */
		if (x_events_info.b_reg_data_ev) {
			/* Extract register length from event info */
			sus_reg_event_len = x_events_info.us_reg_rsp_len;
			if ((sus_reg_event_len == 0) || (sus_reg_event_len > MAC_RT_PIB_MAX_VALUE_LENGTH)) {
				sus_reg_event_len = 1;
			}

			x_spi_data.us_mem_id = ATPL360_REG_INFO_ID;
			x_spi_data.puc_data_buf = spuc_reg_buf;
			x_spi_data.us_len = 8 + sus_reg_event_len;
			if (_spi_send_cmd(SPI_RD_CMD, &x_spi_data) != ATPL360_SUCCESS) {
				spuc_reg_buf[0] = MAC_RT_STATUS_TRANSACTION_OVERFLOW;
			}
		}

		/* Check PHY Sniffer event */
		if (x_events_info.b_phy_snf_ev) {
			/* Extract PHY Sniffer length from event info */
			uint16_t us_data_len = x_events_info.us_phy_snf_ind_len;

			/* Read PHY sniffer frame */
			x_spi_data.us_mem_id = ATPL360_PHY_SNF_ID;
			x_spi_data.puc_data_buf = spuc_phy_snf_buff;
			if ((us_data_len == 0) || (us_data_len > sizeof(phy_snf_frm_t))) {
				us_data_len = 1;
			}

			/* Add length of sniffer fields */
			x_spi_data.us_len = us_data_len;
			if (_spi_send_cmd(SPI_RD_CMD, &x_spi_data) == ATPL360_SUCCESS) {
				sus_phy_snf_event_len = us_data_len;
			}
		}

		/* Time guard */
		sx_atpl360_hal_wrapper.plc_delay(DELAY_TREF_US, 20);
	} else {
		/* Disable EXT INT */
		sx_atpl360_hal_wrapper.plc_enable_int(false);
	}
}

/**
 * \brief Function to get PIB information
 *
 * \param eAttribute   Identification number of configuration parameter
 * \param u16Index     Offset index
 * \param pValue       Pointer to value to write PIB information
 *
 * \return Mac Rt Status
 */
static enum EMacRtStatus _get_req(enum EMacRtPibAttribute eAttribute, uint16_t u16Index, struct TMacRtPibValue *pValue)
{
	uint16_t us_sec_cnt;
	uint16_t us_reg_len;

	if (sb_component_enabled) {
		/* Add Sleep Mode Protection */
		if (atpl360_get_sleep() && (eAttribute != MAC_RT_PIB_SLEEP_MODE)) {
			/* All attributes are disabled in sleep mode except Sleep Mode */
			return MAC_RT_STATUS_DENIED;
		}

		/* Add Debug Mode Protection */
		if (atpl360_get_debug()) {
			if ((eAttribute != MAC_RT_PIB_DEBUG_SET) && (eAttribute != MAC_RT_PIB_DEBUG_READ)) {
				/* All params are disabled in debug mode except Debug Mode IDs*/
				return MAC_RT_STATUS_DENIED;
			}
		} else {
			if (eAttribute == MAC_RT_PIB_DEBUG_READ) {
				/* DEBUG READ is only available when DEBUG MODE is enabled */
				pValue->m_u8Length = 1;
				memset(pValue->m_au8Value, 0, pValue->m_u8Length);

				return MAC_RT_STATUS_DENIED;
			}
		}

		/* Sleep Mode checking */
		if (eAttribute == MAC_RT_PIB_SLEEP_MODE) {
			*(uint8_t *)(pValue->m_au8Value) = atpl360_get_sleep();
			pValue->m_u8Length = 1;

			return MAC_RT_STATUS_SUCCESS;
		}

		/* Debug Mode checking */
		if (eAttribute == MAC_RT_PIB_DEBUG_SET) {
			uint8_t *puc_data;

			puc_data = (uint8_t *)(pValue->m_au8Value);

			/* Get Debug Mode */
			*puc_data++ = (uint8_t)atpl360_get_debug();
			*puc_data++ = (uint8_t)(sul_dbg_addr >> 24);
			*puc_data++ = (uint8_t)(sul_dbg_addr >> 16);
			*puc_data++ = (uint8_t)(sul_dbg_addr >> 8);
			*puc_data++ = (uint8_t)(sul_dbg_addr);
			*puc_data++ = (uint8_t)(sul_dbg_len >> 16);
			*puc_data++ = (uint8_t)(sul_dbg_len);
			pValue->m_u8Length = puc_data - (uint8_t *)(pValue->m_au8Value);

			return MAC_RT_STATUS_SUCCESS;
		}

		/* Debug Read checking */
		if (eAttribute == MAC_RT_PIB_DEBUG_READ) {
			/* Get Data debug data */
			atpl360_debug_read(sul_dbg_addr, pValue->m_au8Value, sul_dbg_len);
			pValue->m_u8Length = sul_dbg_len;

			return MAC_RT_STATUS_SUCCESS;
		}

		/* Protection: Disable external interrupt and reset event flag */
		sx_atpl360_hal_wrapper.plc_enable_int(false);
		sus_reg_event_len = 0;

		/* Build command */
		uint8_t *puc_buf = spuc_reg_buf;
		*puc_buf++ = ATPL360_REG_CMD_RD;
		*puc_buf++ = (uint8_t)(eAttribute >> 24);
		*puc_buf++ = (uint8_t)(eAttribute >> 16);
		*puc_buf++ = (uint8_t)(eAttribute >> 8);
		*puc_buf++ = (uint8_t)(eAttribute);
		*puc_buf++ = (uint8_t)(u16Index >> 8);
		*puc_buf++ = (uint8_t)(u16Index);

		/* Send command */
		atpl360_spi_data_t x_spi_data;
		x_spi_data.us_len = puc_buf - spuc_reg_buf;
		x_spi_data.us_mem_id = ATPL360_REG_INFO_ID;
		x_spi_data.puc_data_buf = spuc_reg_buf;
		atpl360_res_t uc_spi_res = _spi_send_cmd(SPI_WR_CMD, &x_spi_data);

		/* Enable external interrupt */
		sx_atpl360_hal_wrapper.plc_enable_int(true);

		/* Check SPI command result */
		if (uc_spi_res != ATPL360_SUCCESS) {
			return MAC_RT_STATUS_TRANSACTION_OVERFLOW;
		}

		/* Sync function: Wait to response from interrupt */
		us_sec_cnt = 100;
		us_reg_len = sus_reg_event_len;
		while (!us_reg_len) {
			/* Wait for Event. WFE returns immediately if an
			 * interrupt occurred since last WFE; otherwise, enter
			 * in sleep mode until an event occurs (interrupt) */
			__WFE();
			if (!us_sec_cnt--) {
				/* Error in get cmd */
				return MAC_RT_STATUS_TRANSACTION_OVERFLOW;
			}

			us_reg_len = sus_reg_event_len;
		}

		/* Reset event flag */
		sus_reg_event_len = 0;

		/* Check Response Content */
		enum EMacRtStatus uc_get_res = (enum EMacRtStatus)spuc_reg_buf[0];
		if (uc_get_res != MAC_RT_STATUS_SUCCESS) {
			/* Not success process */
			return uc_get_res;
		}

		/* copy reg info in data pointer */
		memcpy(pValue->m_au8Value, spuc_reg_buf + 8, us_reg_len);
		pValue->m_u8Length = us_reg_len;

		return MAC_RT_STATUS_SUCCESS;
	} else {
		return MAC_RT_STATUS_DENIED;
	}
}

/**
 * \brief Function to set PIB information
 *
 * \param eAttribute   Identification number of configuration parameter
 * \param u16Index     Offset index
 * \param pValue       Pointer to value to read PIB information
 *
 * \return Mac Rt Status
 */
static enum EMacRtStatus _set_req(enum EMacRtPibAttribute eAttribute, uint16_t u16Index, const struct TMacRtPibValue *pValue)
{
	uint16_t us_sec_cnt;
	uint16_t us_dummy;

	if (sb_component_enabled) {
		/* Add Sleep Mode Protection */
		if (atpl360_get_sleep() && (eAttribute != MAC_RT_PIB_SLEEP_MODE)) {
			/* All attributes are disabled in sleep mode except Sleep Mode */
			return MAC_RT_STATUS_DENIED;
		}

		/* Sleep Mode checking */
		if (eAttribute == MAC_RT_PIB_SLEEP_MODE) {
			atpl360_set_sleep(*(bool *)(pValue->m_au8Value));
			return MAC_RT_STATUS_SUCCESS;
		}

		/* Add Debug Mode Protection */
		if (atpl360_get_debug()) {
			if ((eAttribute != MAC_RT_PIB_DEBUG_SET) && (eAttribute != MAC_RT_PIB_DEBUG_READ)) {
				/* All params are disabled in debug mode except Debug IDs */
				return MAC_RT_STATUS_DENIED;
			}
		} else {
			if (eAttribute == MAC_RT_PIB_DEBUG_READ) {
				/* DEBUG READ is only available when DEBUG MODE is enabled */
				return MAC_RT_STATUS_DENIED;
			}
		}

		/* Debug Mode checking */
		if (eAttribute == MAC_RT_PIB_DEBUG_SET) {
			/* Set Debug Mode */
			uint8_t *puc_data;

			/* Only used to catch debug parametes */
			puc_data = (uint8_t *)(pValue->m_au8Value);

			/* Set Debug Mode */
			if (*puc_data++) {
				sul_dbg_addr = ((uint32_t)*puc_data++) << 24;
				sul_dbg_addr += ((uint32_t)*puc_data++) << 16;
				sul_dbg_addr += ((uint32_t)*puc_data++) << 8;
				sul_dbg_addr += ((uint32_t)*puc_data++);
				sul_dbg_len = ((uint32_t)*puc_data++) << 8;
				sul_dbg_len += ((uint32_t)*puc_data++);

				atpl360_set_debug(true);
			} else {
				atpl360_set_debug(false);
				sul_dbg_addr = 0;
				sul_dbg_len = 0;
			}

			return MAC_RT_STATUS_SUCCESS;
		}

		/* Debug Read checking */
		if (eAttribute == MAC_RT_PIB_DEBUG_READ) {
			/* Nothing to do */
			return MAC_RT_STATUS_INVALID_PARAMETER;
		}

		/* Check Length */
		if (pValue->m_u8Length > MAC_RT_PIB_MAX_VALUE_LENGTH) {
			/* len error */
			return MAC_RT_STATUS_INVALID_PARAMETER;
		}

		/* Protection: Disable external interrupt and reset event flag */
		sx_atpl360_hal_wrapper.plc_enable_int(false);
		sus_reg_event_len = 0;

		/* Build command */
		uint8_t *puc_buf = spuc_reg_buf;
		*puc_buf++ = ATPL360_REG_CMD_WR;
		*puc_buf++ = (uint8_t)(eAttribute >> 24);
		*puc_buf++ = (uint8_t)(eAttribute >> 16);
		*puc_buf++ = (uint8_t)(eAttribute >> 8);
		*puc_buf++ = (uint8_t)(eAttribute);
		*puc_buf++ = (uint8_t)(u16Index >> 8);
		*puc_buf++ = (uint8_t)(u16Index);
		*puc_buf++ = pValue->m_u8Length;
		memcpy(puc_buf, pValue->m_au8Value, pValue->m_u8Length);
		puc_buf += pValue->m_u8Length;

		/* Send command */
		atpl360_spi_data_t x_spi_data;
		x_spi_data.us_len = puc_buf - spuc_reg_buf;
		x_spi_data.us_mem_id = ATPL360_REG_INFO_ID;
		x_spi_data.puc_data_buf = spuc_reg_buf;
		atpl360_res_t uc_spi_res = _spi_send_cmd(SPI_WR_CMD, &x_spi_data);

		/* Enable external interrupt */
		sx_atpl360_hal_wrapper.plc_enable_int(true);

		/* Check SPI command result */
		if (uc_spi_res != ATPL360_SUCCESS) {
			return MAC_RT_STATUS_TRANSACTION_OVERFLOW;
		}

		/* Sync function: Wait to response from interrupt */
		us_sec_cnt = 100;
		us_dummy = sus_reg_event_len;
		while (!us_dummy) {
			/* Wait for Event. WFE returns immediately if an
			 * interrupt occurred since last WFE; otherwise, enter
			 * in sleep mode until an event occurs (interrupt) */
			__WFE();
			if (!us_sec_cnt--) {
				/* Error in get cmd */
				return MAC_RT_STATUS_TRANSACTION_OVERFLOW;
			}

			us_dummy = sus_reg_event_len;
		}

		/* Reset event flag */
		sus_reg_event_len = 0;

		return (enum EMacRtStatus)spuc_reg_buf[0];
	} else {
		return MAC_RT_STATUS_DENIED;
	}
}

/**
 * \brief Function to transmit message through ATPL360 device
 *
 * \param pTxRequest  Pointer to MAC message information
 * \param pMhr        Pointer to MAC header information
 *
 */
static void _tx_request(uint8_t *pTxSdu, uint16_t u16TxSduLen)
{
	enum EMacRtStatus eStatus = MAC_RT_STATUS_SUCCESS;

	if (sb_component_enabled) {
		/* Add Sleep Mode Protection */
		if (atpl360_get_sleep()) {
			/* Not available in Sleep Mode */
			eStatus = MAC_RT_STATUS_DENIED;
		} else if (sx_atpl360_hal_wrapper.plc_get_thw != NULL) {
			/* Check thermal warning (>110�C). Only for PL460/PL480 */
			if (sx_atpl360_hal_wrapper.plc_get_thw()) {
				/* High temperature: TX error */
				eStatus = MAC_RT_STATUS_DENIED;
			}
		}

		if (eStatus == MAC_RT_STATUS_SUCCESS) {
			uint16_t us_len = atpl360_comm_tx_stringify(spuc_tx_buf, pTxSdu, u16TxSduLen);
			if (us_len > 0) {
				atpl360_spi_data_t x_spi_data;

				/* Send tx msg */
				x_spi_data.us_len = us_len;
				x_spi_data.us_mem_id = ATPL360_TX_REQ_ID;
				x_spi_data.puc_data_buf = spuc_tx_buf;
				if (_spi_send_cmd(SPI_WR_CMD, &x_spi_data) != ATPL360_SUCCESS) {
					eStatus = MAC_RT_STATUS_TRANSACTION_OVERFLOW;
				}
			} else {
				eStatus = MAC_RT_STATUS_INVALID_PARAMETER;
			}
		}
	} else {
		eStatus = MAC_RT_STATUS_DENIED;
	}

	if (eStatus != MAC_RT_STATUS_SUCCESS) {
		if (_tx_cfm_cb_handler) {
			_tx_cfm_cb_handler(eStatus, false);
		}
	}
}

/**
 * \brief Function to get Timer reference from ATPL360 device
 *
 * \return ATPL360 Timer reference in microseconds
 *
 */
static uint32_t _get_timer_ref(void)
{
	uint32_t phy_time = 0;
	atpl360_spi_data_t x_spi_data;

	/* Add Sleep Mode Protection */
	if (atpl360_get_sleep()) {
		/* Not available in Sleep Mode */
		return 0;
	}

	x_spi_data.us_len = 4;
	x_spi_data.us_mem_id = ATPL360_STATUS_INFO_ID;
	x_spi_data.puc_data_buf = (uint8_t *)&phy_time;
	/* Send SPI command */
	_spi_send_cmd(SPI_RD_CMD, &x_spi_data);

	return phy_time;
}

/**
 * \brief Function to set ATPL360 device in Coordinator Mode
 *
 */
static void _set_coordinator(void)
{
	atpl360_spi_data_t x_spi_data;
	uint8_t uc_dummy;

	/* Add Sleep Mode Protection */
	if (atpl360_get_sleep()) {
		/* Not available in Sleep Mode */
		return;
	}

	x_spi_data.us_len = 1;
	x_spi_data.us_mem_id = ATPL360_SET_COORD_ID;
	x_spi_data.puc_data_buf = &uc_dummy;

	/* Send SPI command */
	_spi_send_cmd(SPI_WR_CMD, &x_spi_data);
}

/**
 * \brief Function to enable/disable PHY sniffer
 *
 */
static void _set_sniffer_mode(bool b_enable)
{
	atpl360_spi_data_t x_spi_data;
	uint8_t uc_value;

	/* Add Sleep Mode Protection */
	if (atpl360_get_sleep()) {
		/* Not available in Sleep Mode */
		return;
	}

	x_spi_data.us_len = 1;
	x_spi_data.us_mem_id = ATPL360_PHY_SNF_ID;
	uc_value = (uint8_t)b_enable;
	x_spi_data.puc_data_buf = &uc_value;

	/* Send SPI command */
	_spi_send_cmd(SPI_WR_CMD, &x_spi_data);
}

/**
 * \brief Function to set callbacks to call on upper layer
 *
 * \param px_cbs  Pointer to ATPL360 Callback struct
 *
 */
static void _set_callbacks(atpl360_dev_callbacks_t *px_cbs)
{
	_tx_cfm_cb_handler = px_cbs->tx_cfm;
	_data_ind_cb_handler = px_cbs->data_ind;
	_mac_snf_cb_handler = px_cbs->mac_sniffer_ind;
	_comm_status_cb_handler = px_cbs->comm_status_ind;
	_rx_par_ind_cb_handler = px_cbs->rx_parameters_ind;

	_exception_event_cb_handler = px_cbs->exception_cb;
	_sleep_mode_cb_handler = px_cbs->sleep_mode_cb;
	_debug_mode_cb_handler = px_cbs->debug_mode_cb;
}

/**
 * \brief Function to init internal system
 *
 */
static void _init_system(void)
{
	/* Init Callback function pointers */
	_tx_cfm_cb_handler = NULL;
	_data_ind_cb_handler = NULL;
	_mac_snf_cb_handler = NULL;
	_comm_status_cb_handler = NULL;
	_rx_par_ind_cb_handler = NULL;

	_exception_event_cb_handler = NULL;
	_sleep_mode_cb_handler = NULL;
	_debug_mode_cb_handler = NULL;

	/* Init PHY sniffer callback */
	_phy_snf_cb_handler = NULL;
	spuc_phy_snf_buff = NULL;

	/* Init internal SPI controller */
	atpl360_spi_initialize();

	/* Initialize HAL SPI */
	sx_atpl360_hal_wrapper.plc_init();
	sx_atpl360_hal_wrapper.plc_reset();

	/* Set ATPl360 handler and enable int */
	sx_atpl360_hal_wrapper.plc_set_handler(_handler_atpl360_ext_int);

	/* Init event indicators */
	sb_tx_cfm_event_enable = false;
	sb_comm_status_event_enable = false;
	sb_rx_par_ind_event_enable = false;
	sus_phy_snf_event_len = 0;
	sus_data_ind_event_len = 0;
	sus_mac_snf_event_len = 0;
	sus_reg_event_len = 0;

	/* Init internal buffers */
	memset(spuc_tx_buf, 0, sizeof(spuc_tx_buf));
	memset(spuc_data_ind_buf, 0, sizeof(spuc_data_ind_buf));
	memset(spuc_mac_snf_buf, 0, sizeof(spuc_mac_snf_buf));
	memset(spuc_comm_status_buf, 0, sizeof(spuc_comm_status_buf));
	memset(spuc_rx_par_buf, 0, sizeof(spuc_rx_par_buf));
	memset(spuc_tx_cfm_buf, 0, sizeof(spuc_tx_cfm_buf));
	memset(spuc_reg_buf, 0, sizeof(spuc_reg_buf));
}

/**
 * \brief Function to initialize ATPL360 instance
 *
 * \param descr           Pointer to ATPL360 descriptor
 * \param px_hal_wrapper  Pointer to HAL wrapper (hardware abstraction layer functions)
 *
 */
void atpl360_init(atpl360_descriptor_t *const px_descr, atpl360_hal_wrapper_t *px_hal_wrapper)
{
	/* Fill HAL wrapper functions to access hardware peripherals */
	memcpy(&sx_atpl360_hal_wrapper, px_hal_wrapper, sizeof(atpl360_hal_wrapper_t));

	px_descr->set_callbacks = _set_callbacks;

	px_descr->tx_request = _tx_request;
	px_descr->set_coordinator = _set_coordinator;
	px_descr->get_req = _get_req;
	px_descr->set_req = _set_req;
	px_descr->get_timer_ref = _get_timer_ref;

	/* component must be explicitly enabled */
	atpl360_disable();

	/* init internal vars */
	_init_system();
}

/**
 * \brief Function to enable ATPL360 instance
 *
 * \param ul_binary_address   Address where binary file for ATPL360 is stored in internal flash memory
 * \param ul_binary_len       Size of binary file for ATPl360
 *
 * \return Result of enable operation
 */
atpl360_res_t atpl360_enable(uint32_t ul_binary_address, uint32_t ul_binary_len)
{
	atpl360_spi_data_t x_spi_data;
	uint8_t puc_int_buffer[ATPL360_EVENT_DATA_LENGTH];

	/* Disable EXT INT */
	sx_atpl360_hal_wrapper.plc_enable_int(false);

	atpl360_boot_init(ul_binary_address, ul_binary_len);

	/* Read Time Ref to get SPI status */
	x_spi_data.us_mem_id = ATPL360_STATUS_INFO_ID;
	x_spi_data.us_len = sizeof(puc_int_buffer);
	x_spi_data.puc_data_buf = puc_int_buffer;
	if (_spi_send_cmd(SPI_RD_CMD, &x_spi_data) == ATPL360_ERROR) {
		return ATPL360_ERROR;
	}

	/* Enable component */
	sb_component_enabled = true;

	/* Disable Sleep mode */
	sb_sleep_enabled = false;

	/* Disable Debug mode */
	sb_debug_enabled = false;
	sul_dbg_addr = 0;
	sul_dbg_len = 0;

	/* Enable EXT INT */
	sx_atpl360_hal_wrapper.plc_enable_int(true);

	return ATPL360_SUCCESS;
}

void atpl360_disable(void)
{
	/* Add Sleep Mode Protection */
	if (atpl360_get_sleep()) {
		/* Not available in Sleep Mode */
		return;
	}

	sb_component_enabled = false;

	/* Disable EXT INT by default */
	sx_atpl360_hal_wrapper.plc_enable_int(false);

	/* PL360 reset */
	sx_atpl360_hal_wrapper.plc_reset();
}

void atpl360_sniffer_mode_enable(uint8_t *puc_buffer, pf_phy_snf_cb_t pf_callback)
{
	_phy_snf_cb_handler = pf_callback;
	spuc_phy_snf_buff = puc_buffer;
	_set_sniffer_mode(true);
}

void atpl360_sniffer_mode_disable(void)
{
	_set_sniffer_mode(false);
	_phy_snf_cb_handler = NULL;
	spuc_phy_snf_buff = NULL;
}

/**
 * \brief Function to Check ATPL360 pending events
 *
 */
void atpl360_handle_events(void)
{
	if (sb_component_enabled) {
		/* Add Sleep Mode Protection */
		if (atpl360_get_sleep()) {
			/* Not available in Sleep Mode */
			return;
		}

		/* Check Tx Cfm event */
		if (sb_tx_cfm_event_enable) {
			/* Reset event flag */
			sb_tx_cfm_event_enable = false;

			if (_tx_cfm_cb_handler) {
				_tx_cfm_cb_handler((enum EMacRtStatus)spuc_tx_cfm_buf[0],
						(bool)spuc_tx_cfm_buf[1]);
			}
		}

		/* Check MAC LME indication event */
		if (sb_rx_par_ind_event_enable) {
			/* Reset event flag */
			sb_rx_par_ind_event_enable = 0;

			if (_rx_par_ind_cb_handler) {
				_rx_par_ind_cb_handler((struct TMacRtRxParameters *)spuc_rx_par_buf);
			}
		}

		/* Check Data indication event */
		if (sus_data_ind_event_len) {
			if (_data_ind_cb_handler) {
				_data_ind_cb_handler(spuc_data_ind_buf, sus_data_ind_event_len);
			}

			/* Reset event flag */
			sus_data_ind_event_len = 0;
		}

		/* Check MAC sniffer event */
		if (sus_mac_snf_event_len) {
			if (_mac_snf_cb_handler) {
				_mac_snf_cb_handler(spuc_mac_snf_buf, sus_mac_snf_event_len);
			}

			/* Reset event flag */
			sus_mac_snf_event_len = 0;
		}

		/* Check MAC COMM status event */
		if (sb_comm_status_event_enable) {
			/* Reset event flag */
			sb_comm_status_event_enable = 0;

			if (_comm_status_cb_handler) {
				_comm_status_cb_handler(spuc_comm_status_buf);
			}
		}

		/* Check Phy Sniffer event */
		if (sus_phy_snf_event_len) {
			if (_phy_snf_cb_handler) {
				_phy_snf_cb_handler(sus_phy_snf_event_len);
			}

			/* Reset event flag */
			sus_phy_snf_event_len = 0;
		}
	}
}

/**
 * \brief Function to set ATPL360 sleep mode
 *
 * \param sleep   Set True to enter in sleep mode. Set False to exit from sleep mode.
 *
 * \note This function disables PLC interrupts before going to sleep, and will leave
 * them disabled upon return.
 */
void atpl360_set_sleep(bool sleep)
{
	bool b_result;

	/* Check HAL availability */
	if (sx_atpl360_hal_wrapper.plc_set_stby_mode == NULL)
	{
		return;
	}

	if (sleep) {
		/* Enable Sleep Mode */
		if (sb_sleep_enabled) {
			/* Sleep mode is already enabled */
			return;
		}

		/* Disable PLC interrupts */
		sx_atpl360_hal_wrapper.plc_enable_int(false);

		/* Set PL360 STBY pin */
		b_result = sx_atpl360_hal_wrapper.plc_set_stby_mode(true);

		if (b_result) {
			/* STBY pin set successfully: Update status var */
			sb_sleep_enabled = true;
		} else {
			/* STBY pin not available */
			sx_atpl360_hal_wrapper.plc_enable_int(true);
		}
	} else {
		/* Disable Sleep Mode */
		if (!sb_sleep_enabled) {
			/* Sleep mode is already disabled */
			return;
		}

		/* Clear PL360 STBY pin */
		sx_atpl360_hal_wrapper.plc_set_stby_mode(false);

		/* Disable CPU Wait */
		atpl360_spi_boot_write_cmd_enable();
		atpl360_spi_boot_config_sys();
		atpl360_boot_without_load();

		/* Enable PLC interrupts */
		sx_atpl360_hal_wrapper.plc_enable_int(true);

		/* Update status var */
		sb_sleep_enabled = false;

		/* Rise Sleep Mode Callback */
		if (_sleep_mode_cb_handler) {
			_sleep_mode_cb_handler();
		}
	}
}

/**
 * \brief Function to get ATPL360 sleep mode
 *
 * \return sleep  True if PL360 is in sleep mode. False otherwise.
 */
bool atpl360_get_sleep(void)
{
	return sb_sleep_enabled;
}

/**
 * \brief Function to set ATPL360 debug mode
 *
 * \param sleep   Set True to enter in debug mode. Set False to exit from debug mode.
 *
 * \note This function disables PLC interrupts before going to debug, and will leave
 * them disabled upon return.
 */
void atpl360_set_debug(bool debug)
{
	if (debug) {
		/* Enable Debug Mode */
		if (sb_debug_enabled) {
			/* Debug mode is already enabled */
			return;
		}

		/* Disable PLC interrupts */
		sx_atpl360_hal_wrapper.plc_enable_int(false);

		/* Reset PL360 */
		sx_atpl360_hal_wrapper.plc_reset();

		/* Enable CPU Wait */
		atpl360_spi_boot_write_cmd_enable();
		atpl360_spi_boot_config_sys();

		/* Update status var */
		sb_debug_enabled = true;
	} else {
		/* Disable Debug Mode */
		if (!sb_debug_enabled) {
			/* Debug mode is already disabled */
			return;
		}

		/* Disable CPU Wait */
		atpl360_boot_without_load();

		/* Enable PLC interrupts */
		sx_atpl360_hal_wrapper.plc_enable_int(true);

		/* Update status var */
		sb_debug_enabled = false;

		/* Rise Debug Mode Callback */
		if (_debug_mode_cb_handler) {
			_debug_mode_cb_handler();
		}
	}
}

/**
 * \brief Function to get ATPL360 debug mode
 *
 * \return True if PL360 is in debug mode. False otherwise.
 */
bool atpl360_get_debug(void)
{
	return sb_debug_enabled;
}

/**
 * \brief Function to get ATPL360 debug mode
 *
 * \param puc_data  Pointer of data buffer to write with debug data
 * \param us_len    Length of the debug data to read
 *
 * \return Length of the read data.
 */
uint16_t atpl360_debug_read(uint32_t ul_address, uint8_t *puc_data, uint16_t us_len)
{
	if (sb_debug_enabled) {
		return atpl360_spi_boot_read_mem(ul_address, puc_data, us_len);
	}

	return 0;
}
