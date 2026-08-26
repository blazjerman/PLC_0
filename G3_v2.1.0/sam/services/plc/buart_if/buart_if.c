/**
 * \file
 *
 * \brief PLC UART Buffered Interface
 *
 * Copyright (c) 2020 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
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

#include "asf.h"
#include "string.h"
#include "stdarg.h"
#include "stdio.h"
#include "sysclk.h"
#include "tc.h"
#include "pmc.h"
#include "buart_if.h"
#include "conf_buart_if.h"
#include "conf_board.h"

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* @endcond */

#if defined(CONF_BOARD_UART0) || defined(CONF_BOARD_UART1) || defined(CONF_BOARD_UART2) || defined(CONF_BOARD_UART4)
#include "uart.h"

/**
 * \weakgroup buart_plc_group
 * @{
 */
#if (SAMV71 || SAMV70 || SAME70)
/* XDMA channel used for BUART */
#define BUART_XDMAC_UART0_CH_TX 2
#define BUART_XDMAC_UART0_CH_RX 3
#define BUART_XDMAC_UART2_CH_TX 4
#define BUART_XDMAC_UART2_CH_RX 5
#define BUART_XDMAC_UART4_CH_TX 12
#define BUART_XDMAC_UART4_CH_RX 13
/* XDMAC channel status  */
#define BUART_XDMAC_UART0_TX_STATUS (1 << BUART_XDMAC_UART0_CH_TX)
#define BUART_XDMAC_UART0_RX_STATUS (1 << BUART_XDMAC_UART0_CH_RX)
#define BUART_XDMAC_UART2_TX_STATUS (1 << BUART_XDMAC_UART2_CH_TX)
#define BUART_XDMAC_UART2_RX_STATUS (1 << BUART_XDMAC_UART2_CH_RX)
#define BUART_XDMAC_UART4_TX_STATUS (1 << BUART_XDMAC_UART4_CH_TX)
#define BUART_XDMAC_UART4_RX_STATUS (1 << BUART_XDMAC_UART4_CH_RX)
#endif

#ifdef CONF_BOARD_UART0
/* Reception Buffer 0 */
COMPILER_ALIGNED(32)
static uint8_t rx_uart_buf0[RX_UART_BUF0_SIZE];
/* Transmission Buffer 0 */
COMPILER_ALIGNED(32)
static uint8_t tx_uart_buf0[TX_UART_BUF0_SIZE];
/* Pointers to Reception Buffer 0 */
uint8_t *const ptr_rx_uart_buf0 = &rx_uart_buf0[0];
/* Pointers to Transmission Buffer 0 */
uint8_t *const ptr_tx_uart_buf0 = &tx_uart_buf0[0];
#if (SAMV71 || SAMV70 || SAME70)
/* UART0 XDMAC peripheral IDs */
#define BUART_XDMAC_UART0_TX_PERID 20
#define BUART_XDMAC_UART0_RX_PERID 21
/** XDMA channel configuration. */
static xdmac_channel_config_t buart0_xdmac_tx_channel_cfg;
static xdmac_channel_config_t buart0_xdmac_rx_channel_cfg;
#endif
#endif

#ifdef CONF_BOARD_UART1
/* Reception Buffer 1 */
COMPILER_ALIGNED(32)
static uint8_t rx_uart_buf1[RX_UART_BUF1_SIZE];
/* Transmission Buffer 1 */
COMPILER_ALIGNED(32)
static uint8_t tx_uart_buf1[TX_UART_BUF1_SIZE];
/* Pointers to Reception Buffer 1 */
uint8_t *const ptr_rx_uart_buf1 = &rx_uart_buf1[0];
/* Pointers to Transmission Buffer 1 */
uint8_t *const ptr_tx_uart_buf1 = &tx_uart_buf1[0];
#if (SAMV71 || SAMV70 || SAME70)
/* UART1 XDMAC peripheral IDs */
#define BUART_XDMAC_UART1_TX_PERID 22
#define BUART_XDMAC_UART1_RX_PERID 23
/** XDMA channel configuration. */
static xdmac_channel_config_t buart1_xdmac_tx_channel_cfg;
static xdmac_channel_config_t buart1_xdmac_rx_channel_cfg;
#endif
#endif

#ifdef CONF_BOARD_UART2
/* Reception Buffer 2 */
COMPILER_ALIGNED(32)
static uint8_t rx_uart_buf2[RX_UART_BUF2_SIZE];
/* Transmission Buffer 2 */
COMPILER_ALIGNED(32)
static uint8_t tx_uart_buf2[TX_UART_BUF2_SIZE];
/* Pointers to Reception Buffer 2 */
uint8_t *const ptr_rx_uart_buf2 = &rx_uart_buf2[0];
/* Pointers to Transmission Buffer 1 */
uint8_t *const ptr_tx_uart_buf2 = &tx_uart_buf2[0];
#if (SAMV71 || SAMV70 || SAME70)
/* UART1 XDMAC peripheral IDs */
#define BUART_XDMAC_UART2_TX_PERID 24
#define BUART_XDMAC_UART2_RX_PERID 25
/** XDMA channel configuration. */
static xdmac_channel_config_t buart2_xdmac_tx_channel_cfg;
static xdmac_channel_config_t buart2_xdmac_rx_channel_cfg;
#endif
#endif

#ifdef CONF_BOARD_UART4
/* Reception Buffer 4 */
COMPILER_ALIGNED(32)
static uint8_t rx_uart_buf4[RX_UART_BUF4_SIZE];
/* Transmission Buffer 4 */
COMPILER_ALIGNED(32)
static uint8_t tx_uart_buf4[TX_UART_BUF4_SIZE];
/* Pointers to Reception Buffer 4 */
uint8_t *const ptr_rx_uart_buf4 = &rx_uart_buf4[0];
/* Pointers to Transmission Buffer 4 */
uint8_t *const ptr_tx_uart_buf4 = &tx_uart_buf4[0];
#if (SAMV71 || SAMV70 || SAME70)
/* UART0 XDMAC peripheral IDs */
#define BUART_XDMAC_UART4_TX_PERID 28
#define BUART_XDMAC_UART4_RX_PERID 29
/** XDMA channel configuration. */
static xdmac_channel_config_t buart4_xdmac_tx_channel_cfg;
static xdmac_channel_config_t buart4_xdmac_rx_channel_cfg;
#endif
#endif

/*! \brief Communications Queue Info */
typedef struct {
	/** Pointer to transmission queue. Buffer */
	uint8_t *puc_tq_buf;
	/** Pointer to reception queue. Buffer */
	uint8_t *puc_rq_buf;
	/** Reception queue. Read index */
	uint16_t us_rq_idx;
	/** Reception queue. Write index */
	uint16_t us_wq_idx;
	/** Reception queue. Occupation count */
	uint16_t us_rq_count;
} buart_comm_data_t;

/* \name Size of the receive buffer used by the PDC, in bytes */
/* @{ */
#define UART_BUFFER_SIZE                        1024
/* @} */

#ifdef CONF_BOARD_UART0
/* Data struct to use with UART0 */
COMPILER_ALIGNED(32)
static buart_comm_data_t buart_comm_data_0;
/* Receive buffer use 2 fast buffers */
COMPILER_ALIGNED(32)
static uint8_t gs_puc_uart_buf0[UART_BUFFER_SIZE];
/* Current bytes in buffer. */
static uint32_t gs_ul_size_uart_buf0 = UART_BUFFER_SIZE;
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
/* PDC RX data packet. */
pdc_packet_t g_st_uart_rx_packet0;
/* PDC TX data packet. */
pdc_packet_t g_st_uart_tx_packet0;
/* Pointer to PDC register base. */
Pdc *g_p_uart_pdc0;
#endif
/* Number of bytes received in UART0 */
static uint16_t num_bytes_rx_uart0;
#endif

#ifdef CONF_BOARD_UART1
/* Data struct to use with UART0 */
COMPILER_ALIGNED(32)
static buart_comm_data_t buart_comm_data_1;
/* Receive buffer use 2 fast buffers */
COMPILER_ALIGNED(32)
static uint8_t gs_puc_uart_buf1[UART_BUFFER_SIZE];
/* Current bytes in buffer. */
static uint32_t gs_ul_size_uart_buf1 = UART_BUFFER_SIZE;
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
/* PDC RX data packet. */
pdc_packet_t g_st_uart_rx_packet1;
/* PDC TX data packet. */
pdc_packet_t g_st_uart_tx_packet1;
/* Pointer to PDC register base. */
Pdc *g_p_uart_pdc1;
#endif
/* Number of bytes received in UART1 */
static uint16_t num_bytes_rx_uart1;
#endif

#ifdef CONF_BOARD_UART2
/* Data struct to use with UART2 */
COMPILER_ALIGNED(32)
static buart_comm_data_t buart_comm_data_2;
/* Receive buffer use 2 fast buffers */
COMPILER_ALIGNED(32)
static uint8_t gs_puc_uart_buf2[UART_BUFFER_SIZE];
/* Current bytes in buffer. */
static uint32_t gs_ul_size_uart_buf2 = UART_BUFFER_SIZE;
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
/* PDC RX data packet. */
pdc_packet_t g_st_uart_rx_packet2;
/* PDC TX data packet. */
pdc_packet_t g_st_uart_tx_packet2;
/* Pointer to PDC register base. */
Pdc *g_p_uart_pdc2;
#endif
/* Number of bytes received in UART2 */
static uint16_t num_bytes_rx_uart2;
#endif

#ifdef CONF_BOARD_UART4
/* Data struct to use with UART4 */
COMPILER_ALIGNED(32)
static buart_comm_data_t buart_comm_data_4;
/* Receive buffer use 2 fast buffers */
COMPILER_ALIGNED(32)
static uint8_t gs_puc_uart_buf4[UART_BUFFER_SIZE];
/* Current bytes in buffer. */
static uint32_t gs_ul_size_uart_buf4 = UART_BUFFER_SIZE;
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
/* PDC RX data packet. */
pdc_packet_t g_st_uart_rx_packet4;
/* PDC TX data packet. */
pdc_packet_t g_st_uart_tx_packet4;
/* Pointer to PDC register base. */
Pdc *g_p_uart_pdc4;
#endif
/* Number of bytes received in UART4 */
static uint16_t num_bytes_rx_uart4;
#endif

/* Uart channel open / closed */
static uint8_t buart_chn_open[5] = {
	false,
	false,
	false,
	false,
	false
};

#if defined(CONF_BOARD_UART0) || defined(CONF_BOARD_UART1) || defined(CONF_BOARD_UART2) || defined(CONF_BOARD_UART4)

/**
 * \brief Configure Timer Counter to generate an interrupt every 10ms.
 * This interrupt will be used to flush UART input and echo back.
 */
static void _configure_TC_uart(void)
{
	uint32_t ul_div = 1;
	uint32_t ul_tcclks = 0;
	uint32_t ul_sysclk = 0;
	uint32_t ul_frec_hz = (uint32_t)FREQ_TIMER_POLL_UART;

	/* Get system clock. */
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM)
	ul_sysclk = sysclk_get_cpu_hz();
#elif (PIC32CX)
	ul_sysclk = sysclk_get_peripheral_bus_hz(TC_UART);
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
	ul_sysclk = sysclk_get_peripheral_hz();
#endif

	/* Configure PMC. */
	pmc_enable_periph_clk(ID_TC_UART);

	/* Configure TC for a TC_FREQ frequency and trigger on RC compare. */
	tc_find_mck_divisor(ul_frec_hz, ul_sysclk, &ul_div, &ul_tcclks,
			ul_sysclk);
	tc_init(TC_UART, TC_UART_CHN, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC_UART, TC_UART_CHN, (ul_sysclk / ul_div) / ul_frec_hz);

	/* Configure and enable interrupt on RC compare. */
	NVIC_SetPriority((IRQn_Type)ID_TC_UART, TIMER_UART_PRIO);
	NVIC_EnableIRQ((IRQn_Type)ID_TC_UART);
	tc_enable_interrupt(TC_UART, TC_UART_CHN, TC_IER_CPCS);
}

#ifdef CONF_BOARD_UART0

/** @brief	Interrupt handler for UART0
 *
 */
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
void UART0_Handler(void)
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
static void _BUART_RX0_buffer_proc(void)
#endif
{
	uint16_t us_wr_idx, us_data_count;
	uint16_t us_end_size, us_free_size, us_part_size;

#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
	uint32_t ul_status;
	/* Read UART Status. */
	ul_status = uart_get_status(UART0);
	if (ul_status & UART_SR_ENDRX) {
#endif
		/* manage data */
		us_wr_idx = buart_comm_data_0.us_wq_idx;
		us_data_count = buart_comm_data_0.us_rq_count;
		us_free_size = RX_UART_BUF0_SIZE - us_data_count;
		if (gs_ul_size_uart_buf0 <= us_free_size) {
			/* there is enough space to write all data */
			us_end_size = RX_UART_BUF0_SIZE - us_wr_idx;
			/* there is no overflow of us_wq_idx */
			if (us_end_size >= gs_ul_size_uart_buf0) {
#ifdef CONF_BOARD_ENABLE_CACHE
				SCB_InvalidateDCache_by_Addr ((uint32_t*)gs_puc_uart_buf0, gs_ul_size_uart_buf0);                           
#endif
				memcpy(&buart_comm_data_0.puc_rq_buf[us_wr_idx],
						gs_puc_uart_buf0, gs_ul_size_uart_buf0);
				/* update counters */
				buart_comm_data_0.us_rq_count += gs_ul_size_uart_buf0;
				buart_comm_data_0.us_wq_idx += gs_ul_size_uart_buf0;
			} else { /* there is overflow of us_wq_idx -> write in 2
				  * steps	*/
				memcpy(&buart_comm_data_0.puc_rq_buf[us_wr_idx],
						gs_puc_uart_buf0, us_end_size);
				us_part_size = gs_ul_size_uart_buf0 - us_end_size;
				memcpy(&buart_comm_data_0.puc_rq_buf[0],
						&gs_puc_uart_buf0[us_end_size],	us_part_size);
				/* update counters */
				buart_comm_data_0.us_rq_count += gs_ul_size_uart_buf0;
				buart_comm_data_0.us_wq_idx = us_part_size;
			}
		} else { /* there is not enough space to write all data */
			tc_start(TC_UART, TC_UART_CHN);
		}

		/* change RX buffer */
		gs_ul_size_uart_buf0 = UART_BUFFER_SIZE;

		/* Restart read on buffer. */
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		g_st_uart_rx_packet0.ul_addr = (uint32_t)gs_puc_uart_buf0;
		g_st_uart_rx_packet0.ul_size = UART_BUFFER_SIZE;
		pdc_rx_init(g_p_uart_pdc0, &g_st_uart_rx_packet0, NULL);
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
		buart0_xdmac_rx_channel_cfg.mbr_ubc = UART_BUFFER_SIZE;
		xdmac_configure_transfer(XDMAC, BUART_XDMAC_UART0_CH_RX, &buart0_xdmac_rx_channel_cfg);
		xdmac_channel_set_descriptor_control(XDMAC, BUART_XDMAC_UART0_CH_RX, 0);
		xdmac_channel_enable(XDMAC, BUART_XDMAC_UART0_CH_RX);
#endif

		/* Restart timer. */
		tc_start(TC_UART, TC_UART_CHN);
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
	}
#endif
}

#endif /* #ifdef CONF_BOARD_UART0 */

#ifdef CONF_BOARD_UART1

/** @brief	Interrupt handler for UART1
 *
 */
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
void UART1_Handler(void)
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
static void _BUART_RX1_buffer_proc(void)
#endif
{
	uint16_t us_wr_idx, us_data_count;
	uint16_t us_end_size, us_free_size, us_part_size;

#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
	uint32_t ul_status;
	/* Read UART Status. */
	ul_status = uart_get_status(UART1);
	if (ul_status & UART_SR_ENDRX) {
#endif
		/* manage data */
		us_wr_idx = buart_comm_data_1.us_wq_idx;
		us_data_count = buart_comm_data_1.us_rq_count;
		us_free_size = RX_UART_BUF1_SIZE - us_data_count;
		if (gs_ul_size_uart_buf1 <= us_free_size) {
			/* there is enough space to write all data */
			us_end_size = RX_UART_BUF1_SIZE - us_wr_idx;
			/* there is no overflow of us_wq_idx */
			if (us_end_size >= gs_ul_size_uart_buf1) {
#ifdef CONF_BOARD_ENABLE_CACHE
				SCB_InvalidateDCache_by_Addr ((uint32_t*)gs_puc_uart_buf1, gs_ul_size_uart_buf1);
#endif
				memcpy(&buart_comm_data_1.puc_rq_buf[us_wr_idx],
						gs_puc_uart_buf1, gs_ul_size_uart_buf1);
				/* update counters */
				buart_comm_data_1.us_rq_count += gs_ul_size_uart_buf1;
				buart_comm_data_1.us_wq_idx += gs_ul_size_uart_buf1;
			} else { /* there is overflow of us_wq_idx -> write in 2
				  * steps	*/
				memcpy(&buart_comm_data_1.puc_rq_buf[us_wr_idx],
						gs_puc_uart_buf1, us_end_size);
				us_part_size = gs_ul_size_uart_buf1 - us_end_size;
				memcpy(&buart_comm_data_1.puc_rq_buf[0],
						&gs_puc_uart_buf1[us_end_size],	us_part_size);
				/* update counters */
				buart_comm_data_1.us_rq_count += gs_ul_size_uart_buf1;
				buart_comm_data_1.us_wq_idx = us_part_size;
			}
		} else { /* there is not enough space to write all data */
			tc_start(TC_UART, TC_UART_CHN);
		}

		/* change RX buffer */
		gs_ul_size_uart_buf1 = UART_BUFFER_SIZE;

		/* Restart read on buffer. */
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		g_st_uart_rx_packet1.ul_addr = (uint32_t)gs_puc_uart_buf1;
		g_st_uart_rx_packet1.ul_size = UART_BUFFER_SIZE;
		pdc_rx_init(g_p_uart_pdc1, &g_st_uart_rx_packet1, NULL);
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
		buart1_xdmac_rx_channel_cfg.mbr_ubc = UART_BUFFER_SIZE;
		xdmac_configure_transfer(XDMAC, BUART_XDMAC_UART1_CH_RX, &buart1_xdmac_rx_channel_cfg);
		xdmac_channel_set_descriptor_control(XDMAC, BUART_XDMAC_UART1_CH_RX, 0);
		xdmac_channel_enable(XDMAC, BUART_XDMAC_UART1_CH_RX);
#endif

		/* Restart timer. */
		tc_start(TC_UART, TC_UART_CHN);
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
	}
#endif
}
#endif /* #ifdef CONF_BOARD_UART1 */

#ifdef CONF_BOARD_UART2

/** @brief	Interrupt handler for UART1
 *
 */
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
void UART2_Handler(void)
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
static void _BUART_RX2_buffer_proc(void)
#endif
{
	uint16_t us_wr_idx, us_data_count;
	uint16_t us_end_size, us_free_size, us_part_size;

#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
	uint32_t ul_status;
	/* Read UART Status. */
	ul_status = uart_get_status(UART2);
	if (ul_status & UART_SR_ENDRX) {
#endif
		/* manage data */
		us_wr_idx = buart_comm_data_2.us_wq_idx;
		us_data_count = buart_comm_data_2.us_rq_count;
		us_free_size = RX_UART_BUF2_SIZE - us_data_count;
		if (gs_ul_size_uart_buf2 <= us_free_size) {
			/* there is enough space to write all data */
			us_end_size = RX_UART_BUF2_SIZE - us_wr_idx;
			/* there is no overflow of us_wq_idx */
			if (us_end_size >= gs_ul_size_uart_buf2) {
#ifdef CONF_BOARD_ENABLE_CACHE
				SCB_InvalidateDCache_by_Addr ((uint32_t*)gs_puc_uart_buf2, gs_ul_size_uart_buf2);
#endif
				memcpy(&buart_comm_data_2.puc_rq_buf[us_wr_idx],
						gs_puc_uart_buf2, gs_ul_size_uart_buf2);
				/* update counters */
				buart_comm_data_2.us_rq_count += gs_ul_size_uart_buf2;
				buart_comm_data_2.us_wq_idx += gs_ul_size_uart_buf2;
			} else { /* there is overflow of us_wq_idx -> write in 2
				  * steps	*/
				memcpy(&buart_comm_data_2.puc_rq_buf[us_wr_idx],
						gs_puc_uart_buf2, us_end_size);
				us_part_size = gs_ul_size_uart_buf2 - us_end_size;
				memcpy(&buart_comm_data_2.puc_rq_buf[0],
						&gs_puc_uart_buf2[us_end_size],	us_part_size);
				/* update counters */
				buart_comm_data_2.us_rq_count += gs_ul_size_uart_buf2;
				buart_comm_data_2.us_wq_idx = us_part_size;
			}
		} else { /* there is not enough space to write all data */
			tc_start(TC_UART, TC_UART_CHN);
		}

		/* change RX buffer */
		gs_ul_size_uart_buf2 = UART_BUFFER_SIZE;

		/* Restart read on buffer. */
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		g_st_uart_rx_packet2.ul_addr = (uint32_t)gs_puc_uart_buf2;
		g_st_uart_rx_packet2.ul_size = UART_BUFFER_SIZE;
		pdc_rx_init(g_p_uart_pdc2, &g_st_uart_rx_packet2, NULL);
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
		buart2_xdmac_rx_channel_cfg.mbr_ubc = UART_BUFFER_SIZE;
		xdmac_configure_transfer(XDMAC, BUART_XDMAC_UART2_CH_RX, &buart2_xdmac_rx_channel_cfg);
		xdmac_channel_set_descriptor_control(XDMAC, BUART_XDMAC_UART2_CH_RX, 0);
		xdmac_channel_enable(XDMAC, BUART_XDMAC_UART2_CH_RX);
#endif

		/* Restart timer. */
		tc_start(TC_UART, TC_UART_CHN);
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
	}
#endif
}
#endif /* #ifdef CONF_BOARD_UART2 */

#ifdef CONF_BOARD_UART4

/** @brief	Interrupt handler for UART4
 *
 */
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
void UART4_Handler(void)
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
static void _BUART_RX4_buffer_proc(void)
#endif
{
	uint16_t us_wr_idx, us_data_count;
	uint16_t us_end_size, us_free_size, us_part_size;

#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
	uint32_t ul_status;
	/* Read UART Status. */
	ul_status = uart_get_status(UART4);
	if (ul_status & UART_SR_ENDRX) {
#endif
		/* manage data */
		us_wr_idx = buart_comm_data_4.us_wq_idx;
		us_data_count = buart_comm_data_4.us_rq_count;
		us_free_size = RX_UART_BUF4_SIZE - us_data_count;
		if (gs_ul_size_uart_buf4 <= us_free_size) {
			/* there is enough space to write all data */
			us_end_size = RX_UART_BUF4_SIZE - us_wr_idx;
			/* there is no overflow of us_wq_idx */
			if (us_end_size >= gs_ul_size_uart_buf4) {
#ifdef CONF_BOARD_ENABLE_CACHE
				SCB_InvalidateDCache_by_Addr ((uint32_t*)gs_puc_uart_buf4, gs_ul_size_uart_buf4);
#endif
				memcpy(&buart_comm_data_4.puc_rq_buf[us_wr_idx],
						gs_puc_uart_buf4, gs_ul_size_uart_buf4);
				/* update counters */
				buart_comm_data_4.us_rq_count += gs_ul_size_uart_buf4;
				buart_comm_data_4.us_wq_idx += gs_ul_size_uart_buf4;
			} else { /* there is overflow of us_wq_idx -> write in 2
				  * steps	*/
				memcpy(&buart_comm_data_4.puc_rq_buf[us_wr_idx],
						gs_puc_uart_buf4, us_end_size);
				us_part_size = gs_ul_size_uart_buf4 - us_end_size;
				memcpy(&buart_comm_data_4.puc_rq_buf[0],
						&gs_puc_uart_buf4[us_end_size],	us_part_size);
				/* update counters */
				buart_comm_data_4.us_rq_count += gs_ul_size_uart_buf4;
				buart_comm_data_4.us_wq_idx = us_part_size;
			}
		} else { /* there is not enough space to write all data */
			tc_start(TC_UART, TC_UART_CHN);
		}

		/* change RX buffer */
		gs_ul_size_uart_buf4 = UART_BUFFER_SIZE;

		/* Restart read on buffer. */
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		g_st_uart_rx_packet4.ul_addr = (uint32_t)gs_puc_uart_buf4;
		g_st_uart_rx_packet4.ul_size = UART_BUFFER_SIZE;
		pdc_rx_init(g_p_uart_pdc4, &g_st_uart_rx_packet4, NULL);
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
		buart4_xdmac_rx_channel_cfg.mbr_ubc = UART_BUFFER_SIZE;
		xdmac_configure_transfer(XDMAC, BUART_XDMAC_UART4_CH_RX, &buart4_xdmac_rx_channel_cfg);
		xdmac_channel_set_descriptor_control(XDMAC, BUART_XDMAC_UART4_CH_RX, 0);
		xdmac_channel_enable(XDMAC, BUART_XDMAC_UART4_CH_RX);
#endif

		/* Restart timer. */
		tc_start(TC_UART, TC_UART_CHN);
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
	}
#endif
}

#endif /* #ifdef CONF_BOARD_UART4 */
#endif  /* #if defined(CONF_BOARD_UART0) || defined(CONF_BOARD_UART1) || defined(CONF_BOARD_UART2) || defined(CONF_BOARD_UART4) */

/**
 * \brief Interrupt handler. Record the number of bytes received,
 * and then restart a read transfer on the UART if the transfer was stopped.
 */
void TC_UART_Handler(void)
{
	uint32_t ul_status;
#if defined(CONF_BOARD_UART0) || defined(CONF_BOARD_UART1) || defined(CONF_BOARD_UART2) || defined(CONF_BOARD_UART4)
	uint32_t ul_byte_total = 0;
#endif

	/* Read TC_UART Status. */
	ul_status = tc_get_status(TC_UART, TC_UART_CHN);

	/* RC compare. */
	if ((ul_status & TC_SR_CPCS) == TC_SR_CPCS) {
#ifdef CONF_BOARD_UART0
		if (buart_chn_open[0]) {
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
			/* Flush PDC buffer. */
			ul_byte_total = UART_BUFFER_SIZE - pdc_read_rx_counter(g_p_uart_pdc0);
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
			/* Read received bytes from uB ctrl register */
			ul_byte_total = UART_BUFFER_SIZE - XDMAC->XDMAC_CHID[BUART_XDMAC_UART0_CH_RX].XDMAC_CUBC & 0x00FFFFFF;
#endif
			if (ul_byte_total > 0) {
#if (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
				/* Flush the received bytes buffer */
				xdmac_channel_software_flush_request(XDMAC, BUART_XDMAC_UART0_CH_RX);
				/* Read received bytes from uB ctrl register */
				ul_byte_total = UART_BUFFER_SIZE - XDMAC->XDMAC_CHID[BUART_XDMAC_UART0_CH_RX].XDMAC_CUBC & 0x00FFFFFF;
#endif
				if (ul_byte_total == num_bytes_rx_uart0) {
					/* Disable timer. */
					tc_stop(TC_UART, TC_UART_CHN);

					/* Log current size */
					gs_ul_size_uart_buf0 = ul_byte_total;

#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
					/* Stop DMA UART_RX -> force Uart Handler*/
					g_st_uart_rx_packet0.ul_size = 0;
					pdc_rx_init(g_p_uart_pdc0, &g_st_uart_rx_packet0, NULL);
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
					/* Process RX buffer */
					_BUART_RX0_buffer_proc();
#endif
				} else {
					num_bytes_rx_uart0 = ul_byte_total;
				}
			} else {
				num_bytes_rx_uart0 = 0;
			}
		}
#endif
#ifdef CONF_BOARD_UART1
		if (buart_chn_open[1]) {
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
			/* Flush PDC buffer. */
			ul_byte_total = UART_BUFFER_SIZE - pdc_read_rx_counter(g_p_uart_pdc1);
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
			/* Read received bytes from uB ctrl register */
			ul_byte_total = UART_BUFFER_SIZE - XDMAC->XDMAC_CHID[BUART_XDMAC_UART1_CH_RX].XDMAC_CUBC & 0x00FFFFFF;
#endif
			if (ul_byte_total > 0) {
#if (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
				/* Flush the received bytes buffer */
				xdmac_channel_software_flush_request(XDMAC, BUART_XDMAC_UART1_CH_RX);
				/* Read received bytes from uB ctrl register */
				ul_byte_total = UART_BUFFER_SIZE - XDMAC->XDMAC_CHID[BUART_XDMAC_UART1_CH_RX].XDMAC_CUBC & 0x00FFFFFF;
#endif
				if (ul_byte_total == num_bytes_rx_uart1) {
					/* Disable timer. */
					tc_stop(TC_UART, TC_UART_CHN);

					/* Log current size */
					gs_ul_size_uart_buf1 = ul_byte_total;

#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
					/* Stop DMA UART_RX -> force Uart Handler*/
					g_st_uart_rx_packet1.ul_size = 0;
					pdc_rx_init(g_p_uart_pdc1, &g_st_uart_rx_packet1, NULL);
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
					/* Process RX buffer */
					_BUART_RX1_buffer_proc();
#endif
				} else {
					num_bytes_rx_uart1 = ul_byte_total;
				}
			} else {
				num_bytes_rx_uart1 = 0;
			}
		}
#endif
#ifdef CONF_BOARD_UART2
		if (buart_chn_open[2]) {
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
			/* Flush PDC buffer. */
			ul_byte_total = UART_BUFFER_SIZE - pdc_read_rx_counter(g_p_uart_pdc2);
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
			/* Read received bytes from uB ctrl register */
			ul_byte_total = UART_BUFFER_SIZE - XDMAC->XDMAC_CHID[BUART_XDMAC_UART2_CH_RX].XDMAC_CUBC & 0x00FFFFFF;
#endif
			if (ul_byte_total > 0) {
#if (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
				/* Flush the received bytes buffer */
				xdmac_channel_software_flush_request(XDMAC, BUART_XDMAC_UART2_CH_RX);
				/* Read received bytes from uB ctrl register */
				ul_byte_total = UART_BUFFER_SIZE - XDMAC->XDMAC_CHID[BUART_XDMAC_UART2_CH_RX].XDMAC_CUBC & 0x00FFFFFF;
#endif
				if (ul_byte_total == num_bytes_rx_uart2) {
					/* Disable timer. */
					tc_stop(TC_UART, TC_UART_CHN);

					/* Log current size */
					gs_ul_size_uart_buf2 = ul_byte_total;

#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
					/* Stop DMA UART_RX -> force Uart Handler*/
					g_st_uart_rx_packet2.ul_size = 0;
					pdc_rx_init(g_p_uart_pdc2, &g_st_uart_rx_packet2, NULL);
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
					/* Process RX buffer */
					_BUART_RX2_buffer_proc();
#endif
				} else {
					num_bytes_rx_uart2 = ul_byte_total;
				}
			} else {
				num_bytes_rx_uart2 = 0;
			}
		}
#endif
#ifdef CONF_BOARD_UART4
		if (buart_chn_open[4]) {
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
			/* Flush PDC buffer. */
			ul_byte_total = UART_BUFFER_SIZE - pdc_read_rx_counter(g_p_uart_pdc4);
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
			/* Read received bytes from uB ctrl register */
			ul_byte_total = UART_BUFFER_SIZE - XDMAC->XDMAC_CHID[BUART_XDMAC_UART4_CH_RX].XDMAC_CUBC & 0x00FFFFFF;
#endif
			if (ul_byte_total > 0) {
#if (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
				/* Flush the received bytes buffer */
				xdmac_channel_software_flush_request(XDMAC, BUART_XDMAC_UART4_CH_RX);
				/* Read received bytes from uB ctrl register */
				ul_byte_total = UART_BUFFER_SIZE - XDMAC->XDMAC_CHID[BUART_XDMAC_UART4_CH_RX].XDMAC_CUBC & 0x00FFFFFF;
#endif
				if (ul_byte_total == num_bytes_rx_uart4) {
					/* Disable timer. */
					tc_stop(TC_UART, TC_UART_CHN);

					/* Log current size */
					gs_ul_size_uart_buf4 = ul_byte_total;

#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
					/* Stop DMA UART_RX -> force Uart Handler*/
					g_st_uart_rx_packet4.ul_size = 0;
					pdc_rx_init(g_p_uart_pdc0, &g_st_uart_rx_packet0, NULL);
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
					/* Process RX buffer */
					_BUART_RX4_buffer_proc();
#endif
				} else {
					num_bytes_rx_uart4 = ul_byte_total;
				}
			} else {
				num_bytes_rx_uart4 = 0;
			}
		}
#endif
	}
}

/**
 * \brief This function opens an UART
 *
 * \note Opening of the specified UART implies initializing local variables and
 * opening required hardware with the following configuration:
 * - bauds as specified
 * - 8 bits, no parity, 1 stop bit
 * - enable interrupts
 *
 * \param chn			Communication channel [0, 1]
 * \param bauds			Communication speed in bauds
 *
 * \retval true on success.
 * \retval false on failure.
 */
int8_t buart_if_open(uint8_t chn, uint32_t bauds)
{
#if defined(CONF_BOARD_UART0) || defined(CONF_BOARD_UART1) || defined(CONF_BOARD_UART2) || defined(CONF_BOARD_UART4)
	sam_uart_opt_t uart_console_settings;

	/* MCK for UART */
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM)
	uart_console_settings.ul_mck = sysclk_get_cpu_hz();
#elif (PIC32CX)
	uart_console_settings.ul_mck = sysclk_get_peripheral_bus_hz(UART);
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
        /* Initialize and enable DMA controller */
	pmc_enable_periph_clk(ID_XDMAC);
	uart_console_settings.ul_mck = sysclk_get_peripheral_hz();
#endif

	/* Expected baud rate. */
	uart_console_settings.ul_baudrate = bauds;
	/* Initialize value for UART mode register */
	uart_console_settings.ul_mode = UART_MR_PAR_NO;
#else
	UNUSED(bauds);
#endif

	/* check uart and it is close */
	if (chn >= 5) {
		return false;
	}

	if (buart_chn_open[chn]) {
		return false;
	}

	switch (chn) {
#ifdef CONF_BOARD_UART0
	case 0:
	{
		/* Configure PMC. */
		pmc_enable_periph_clk(ID_UART0);
		/* Configure UART. */
		uart_init(UART0, &uart_console_settings);

		/* Assign buffers to pointers */
		buart_comm_data_0.puc_tq_buf = ptr_tx_uart_buf0;
		buart_comm_data_0.puc_rq_buf = ptr_rx_uart_buf0;
		buart_comm_data_0.us_rq_count = 0;
		buart_comm_data_0.us_rq_idx = 0;
		buart_comm_data_0.us_wq_idx = 0;

#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		/* Get board UART0 PDC base address and enable receiver and
		 * transmitter. */
		g_p_uart_pdc0 = uart_get_pdc_base(UART0);
		pdc_enable_transfer(g_p_uart_pdc0,
				PERIPH_PTCR_RXTEN | PERIPH_PTCR_TXTEN);

		/* Start receiving data and start timer. */
		g_st_uart_rx_packet0.ul_addr = (uint32_t)gs_puc_uart_buf0;
		g_st_uart_rx_packet0.ul_size = UART_BUFFER_SIZE;
		pdc_rx_init(g_p_uart_pdc0, &g_st_uart_rx_packet0, NULL);

		/* Stop transmitting data */
		g_st_uart_tx_packet0.ul_addr = (uint32_t)buart_comm_data_0.puc_tq_buf;
		g_st_uart_tx_packet0.ul_size = 0;
		pdc_tx_init(g_p_uart_pdc0, &g_st_uart_tx_packet0, NULL);

		gs_ul_size_uart_buf0 = UART_BUFFER_SIZE;

		/* Transfer to PDC communication mode, disable RXRDY interrupt
		 * and enable RXBUFF interrupt. */
		uart_disable_interrupt(UART0, UART_IDR_RXRDY);
		uart_enable_interrupt(UART0, UART_IER_RXBUFF);
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
		/* Configure TX and RX XDMAC channels */
		buart0_xdmac_tx_channel_cfg.mbr_ubc = 0;
		buart0_xdmac_tx_channel_cfg.mbr_sa = (uint32_t)buart_comm_data_0.puc_tq_buf;
		buart0_xdmac_tx_channel_cfg.mbr_da = (uint32_t)(&(UART0->UART_THR));
		buart0_xdmac_tx_channel_cfg.mbr_cfg = XDMAC_CC_TYPE_PER_TRAN |
			XDMAC_CC_DSYNC_MEM2PER |
			XDMAC_CC_PERID(BUART_XDMAC_UART0_TX_PERID) |
			XDMAC_CC_CSIZE_CHK_1 |
			XDMAC_CC_MEMSET_NORMAL_MODE |
			XDMAC_CC_MBSIZE_SINGLE |
			XDMAC_CC_DWIDTH_BYTE |
			XDMAC_CC_SIF_AHB_IF0 |
			XDMAC_CC_DIF_AHB_IF1 |
			XDMAC_CC_SAM_INCREMENTED_AM |
			XDMAC_CC_DAM_FIXED_AM;
		buart0_xdmac_tx_channel_cfg.mbr_bc = 0;
		buart0_xdmac_tx_channel_cfg.mbr_ds = 0;
		buart0_xdmac_tx_channel_cfg.mbr_sus = 0;
		buart0_xdmac_tx_channel_cfg.mbr_dus = 0;

		buart0_xdmac_rx_channel_cfg.mbr_ubc = UART_BUFFER_SIZE;
		buart0_xdmac_rx_channel_cfg.mbr_sa = (uint32_t)(&(UART0->UART_RHR));
		buart0_xdmac_rx_channel_cfg.mbr_da = (uint32_t)gs_puc_uart_buf0;
		buart0_xdmac_rx_channel_cfg.mbr_cfg = XDMAC_CC_TYPE_PER_TRAN |
			XDMAC_CC_DSYNC_PER2MEM |
			XDMAC_CC_PERID(BUART_XDMAC_UART0_RX_PERID) |
			XDMAC_CC_CSIZE_CHK_1 |
			XDMAC_CC_MEMSET_NORMAL_MODE |
			XDMAC_CC_MBSIZE_SINGLE |
			XDMAC_CC_DWIDTH_BYTE |
			XDMAC_CC_SIF_AHB_IF1 |
			XDMAC_CC_DIF_AHB_IF0 |
			XDMAC_CC_SAM_FIXED_AM |
			XDMAC_CC_DAM_INCREMENTED_AM;
		buart0_xdmac_rx_channel_cfg.mbr_bc = 0;
		buart0_xdmac_rx_channel_cfg.mbr_ds =  0;
		buart0_xdmac_rx_channel_cfg.mbr_sus = 0;
		buart0_xdmac_rx_channel_cfg.mbr_dus = 0;

		xdmac_configure_transfer(XDMAC, BUART_XDMAC_UART0_CH_TX, &buart0_xdmac_tx_channel_cfg);
		xdmac_channel_set_descriptor_control(XDMAC, BUART_XDMAC_UART0_CH_TX, 0);
		xdmac_configure_transfer(XDMAC, BUART_XDMAC_UART0_CH_RX, &buart0_xdmac_rx_channel_cfg);
		xdmac_channel_set_descriptor_control(XDMAC, BUART_XDMAC_UART0_CH_RX, 0);

		/* Transfer to XDMAC communication mode, disable RXRDY interrupt. */
		uart_disable_interrupt(UART0, UART_IDR_RXRDY);

		xdmac_channel_enable(XDMAC, BUART_XDMAC_UART0_CH_TX);
		xdmac_channel_enable(XDMAC, BUART_XDMAC_UART0_CH_RX);
#endif

		/* Enable the receiver and transmitter. */
		uart_enable_tx(UART0);
		uart_enable_rx(UART0);

#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		/* Configure and enable interrupt of UART. */
		NVIC_SetPriority((IRQn_Type)UART0_IRQn, UART0_PRIO);
		NVIC_EnableIRQ(UART0_IRQn);
#endif

		buart_chn_open[chn] = true;
		num_bytes_rx_uart0 = 0;

		/* Configure TC uart */
		_configure_TC_uart();
		tc_start(TC_UART, TC_UART_CHN);
	}
	break;
#endif

#ifdef CONF_BOARD_UART1
	case 1:
	{
		/* Configure PMC. */
		pmc_enable_periph_clk(ID_UART1);
		/* Configure UART. */
		uart_init(UART1, &uart_console_settings);

		/* Assign buffers to pointers */
		buart_comm_data_1.puc_tq_buf = ptr_tx_uart_buf1;
		buart_comm_data_1.puc_rq_buf = ptr_rx_uart_buf1;
		buart_comm_data_1.us_rq_count = 0;
		buart_comm_data_1.us_rq_idx = 0;
		buart_comm_data_1.us_wq_idx = 0;

#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		/* Get board UART1 PDC base address and enable receiver and
		 * transmitter. */
		g_p_uart_pdc1 = uart_get_pdc_base(UART1);
		pdc_enable_transfer(g_p_uart_pdc1,
				PERIPH_PTCR_RXTEN | PERIPH_PTCR_TXTEN);

		/* Start receiving data and start timer. */
		g_st_uart_rx_packet1.ul_addr = (uint32_t)gs_puc_uart_buf1;
		g_st_uart_rx_packet1.ul_size = UART_BUFFER_SIZE;
		pdc_rx_init(g_p_uart_pdc1, &g_st_uart_rx_packet1, NULL);

		/* Stop transmitting data */
		g_st_uart_tx_packet1.ul_addr = (uint32_t)buart_comm_data_1.puc_tq_buf;
		g_st_uart_tx_packet1.ul_size = 0;
		pdc_tx_init(g_p_uart_pdc1, &g_st_uart_tx_packet1, NULL);

		gs_ul_size_uart_buf1 = UART_BUFFER_SIZE;

		/* Transfer to PDC communication mode, disable RXRDY interrupt
		 * and enable RXBUFF interrupt. */
		uart_disable_interrupt(UART1, UART_IDR_RXRDY);
		uart_enable_interrupt(UART1, UART_IER_RXBUFF);
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
		/* Configure TX and RX XDMAC channels */
		buart1_xdmac_tx_channel_cfg.mbr_ubc = 0;
		buart1_xdmac_tx_channel_cfg.mbr_sa = (uint32_t)buart_comm_data_1.puc_tq_buf;
		buart1_xdmac_tx_channel_cfg.mbr_da = (uint32_t)(&(UART1->UART_THR));
		buart1_xdmac_tx_channel_cfg.mbr_cfg = XDMAC_CC_TYPE_PER_TRAN |
			XDMAC_CC_DSYNC_MEM2PER |
			XDMAC_CC_PERID(BUART_XDMAC_UART1_TX_PERID) |
			XDMAC_CC_CSIZE_CHK_1 |
			XDMAC_CC_MEMSET_NORMAL_MODE |
			XDMAC_CC_MBSIZE_SINGLE |
			XDMAC_CC_DWIDTH_BYTE |
			XDMAC_CC_SIF_AHB_IF0 |
			XDMAC_CC_DIF_AHB_IF1 |
			XDMAC_CC_SAM_INCREMENTED_AM |
			XDMAC_CC_DAM_FIXED_AM;
		buart1_xdmac_tx_channel_cfg.mbr_bc = 0;
		buart1_xdmac_tx_channel_cfg.mbr_ds = 0;
		buart1_xdmac_tx_channel_cfg.mbr_sus = 0;
		buart1_xdmac_tx_channel_cfg.mbr_dus = 0;

		buart1_xdmac_rx_channel_cfg.mbr_ubc = UART_BUFFER_SIZE;
		buart1_xdmac_rx_channel_cfg.mbr_sa = (uint32_t)(&(UART1->UART_RHR));
		buart1_xdmac_rx_channel_cfg.mbr_da = (uint32_t)gs_puc_uart_buf1;
		buart1_xdmac_rx_channel_cfg.mbr_cfg = XDMAC_CC_TYPE_PER_TRAN |
			XDMAC_CC_DSYNC_PER2MEM |
			XDMAC_CC_PERID(BUART_XDMAC_UART1_RX_PERID) |
			XDMAC_CC_CSIZE_CHK_1 |
			XDMAC_CC_MEMSET_NORMAL_MODE |
			XDMAC_CC_MBSIZE_SINGLE |
			XDMAC_CC_DWIDTH_BYTE |
			XDMAC_CC_SIF_AHB_IF1 |
			XDMAC_CC_DIF_AHB_IF0 |
			XDMAC_CC_SAM_FIXED_AM |
			XDMAC_CC_DAM_INCREMENTED_AM;
		buart1_xdmac_rx_channel_cfg.mbr_bc = 0;
		buart1_xdmac_rx_channel_cfg.mbr_ds =  0;
		buart1_xdmac_rx_channel_cfg.mbr_sus = 0;
		buart1_xdmac_rx_channel_cfg.mbr_dus = 0;

		xdmac_configure_transfer(XDMAC, BUART_XDMAC_UART1_CH_TX, &buart1_xdmac_tx_channel_cfg);
		xdmac_channel_set_descriptor_control(XDMAC, BUART_XDMAC_UART1_CH_TX, 0);
		xdmac_configure_transfer(XDMAC, BUART_XDMAC_UART1_CH_RX, &buart1_xdmac_rx_channel_cfg);
		xdmac_channel_set_descriptor_control(XDMAC, BUART_XDMAC_UART1_CH_RX, 0);

		/* Transfer to XDMAC communication mode, disable RXRDY interrupt. */
		uart_disable_interrupt(UART1, UART_IDR_RXRDY);

		xdmac_channel_enable(XDMAC, BUART_XDMAC_UART1_CH_TX);
		xdmac_channel_enable(XDMAC, BUART_XDMAC_UART1_CH_RX);
#endif

		/* Enable the receiver and transmitter. */
		uart_enable_tx(UART1);
		uart_enable_rx(UART1);

#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		/* Configure and enable interrupt of UART. */
		NVIC_SetPriority((IRQn_Type)UART1_IRQn, UART1_PRIO);
		NVIC_EnableIRQ(UART1_IRQn);
#endif

		buart_chn_open[chn] = true;
		num_bytes_rx_uart1 = 0;

		/* Configure TC uart */
		_configure_TC_uart();
		tc_start(TC_UART, TC_UART_CHN);
	}
	break;
#endif

#ifdef CONF_BOARD_UART2
	case 2:
	{
		/* Configure PMC. */
		pmc_enable_periph_clk(ID_UART2);
		/* Configure UART. */
		uart_init(UART2, &uart_console_settings);

		/* Assign buffers to pointers */
		buart_comm_data_2.puc_tq_buf = ptr_tx_uart_buf2;
		buart_comm_data_2.puc_rq_buf = ptr_rx_uart_buf2;
		buart_comm_data_2.us_rq_count = 0;
		buart_comm_data_2.us_rq_idx = 0;
		buart_comm_data_2.us_wq_idx = 0;

#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		/* Get board UART1 PDC base address and enable receiver and
		 * transmitter. */
		g_p_uart_pdc2 = uart_get_pdc_base(UART2);
		pdc_enable_transfer(g_p_uart_pdc2,
				PERIPH_PTCR_RXTEN | PERIPH_PTCR_TXTEN);

		/* Start receiving data and start timer. */
		g_st_uart_rx_packet2.ul_addr = (uint32_t)gs_puc_uart_buf2;
		g_st_uart_rx_packet2.ul_size = UART_BUFFER_SIZE;
		pdc_rx_init(g_p_uart_pdc2, &g_st_uart_rx_packet2, NULL);

		/* Stop transmitting data */
		g_st_uart_tx_packet2.ul_addr = (uint32_t)buart_comm_data_2.puc_tq_buf;
		g_st_uart_tx_packet2.ul_size = 0;
		pdc_tx_init(g_p_uart_pdc2, &g_st_uart_tx_packet2, NULL);

		gs_ul_size_uart_buf2 = UART_BUFFER_SIZE;

		/* Transfer to PDC communication mode, disable RXRDY interrupt
		 * and enable RXBUFF interrupt. */
		uart_disable_interrupt(UART2, UART_IDR_RXRDY);
		uart_enable_interrupt(UART2, UART_IER_RXBUFF);
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
		/* Configure TX and RX XDMAC channels */
		buart2_xdmac_tx_channel_cfg.mbr_ubc = 0;
		buart2_xdmac_tx_channel_cfg.mbr_sa = (uint32_t)buart_comm_data_2.puc_tq_buf;
		buart2_xdmac_tx_channel_cfg.mbr_da = (uint32_t)(&(UART2->UART_THR));
		buart2_xdmac_tx_channel_cfg.mbr_cfg = XDMAC_CC_TYPE_PER_TRAN |
			XDMAC_CC_DSYNC_MEM2PER |
			XDMAC_CC_PERID(BUART_XDMAC_UART2_TX_PERID) |
			XDMAC_CC_CSIZE_CHK_1 |
			XDMAC_CC_MEMSET_NORMAL_MODE |
			XDMAC_CC_MBSIZE_SINGLE |
			XDMAC_CC_DWIDTH_BYTE |
			XDMAC_CC_SIF_AHB_IF0 |
			XDMAC_CC_DIF_AHB_IF1 |
			XDMAC_CC_SAM_INCREMENTED_AM |
			XDMAC_CC_DAM_FIXED_AM;
		buart2_xdmac_tx_channel_cfg.mbr_bc = 0;
		buart2_xdmac_tx_channel_cfg.mbr_ds = 0;
		buart2_xdmac_tx_channel_cfg.mbr_sus = 0;
		buart2_xdmac_tx_channel_cfg.mbr_dus = 0;

		buart2_xdmac_rx_channel_cfg.mbr_ubc = UART_BUFFER_SIZE;
		buart2_xdmac_rx_channel_cfg.mbr_sa = (uint32_t)(&(UART2->UART_RHR));
		buart2_xdmac_rx_channel_cfg.mbr_da = (uint32_t)gs_puc_uart_buf2;
		buart2_xdmac_rx_channel_cfg.mbr_cfg = XDMAC_CC_TYPE_PER_TRAN |
			XDMAC_CC_DSYNC_PER2MEM |
			XDMAC_CC_PERID(BUART_XDMAC_UART2_RX_PERID) |
			XDMAC_CC_CSIZE_CHK_1 |
			XDMAC_CC_MEMSET_NORMAL_MODE |
			XDMAC_CC_MBSIZE_SINGLE |
			XDMAC_CC_DWIDTH_BYTE |
			XDMAC_CC_SIF_AHB_IF1 |
			XDMAC_CC_DIF_AHB_IF0 |
			XDMAC_CC_SAM_FIXED_AM |
			XDMAC_CC_DAM_INCREMENTED_AM;
		buart2_xdmac_rx_channel_cfg.mbr_bc = 0;
		buart2_xdmac_rx_channel_cfg.mbr_ds =  0;
		buart2_xdmac_rx_channel_cfg.mbr_sus = 0;
		buart2_xdmac_rx_channel_cfg.mbr_dus = 0;

		xdmac_configure_transfer(XDMAC, BUART_XDMAC_UART2_CH_TX, &buart2_xdmac_tx_channel_cfg);
		xdmac_channel_set_descriptor_control(XDMAC, BUART_XDMAC_UART2_CH_TX, 0);
		xdmac_configure_transfer(XDMAC, BUART_XDMAC_UART2_CH_RX, &buart2_xdmac_rx_channel_cfg);
		xdmac_channel_set_descriptor_control(XDMAC, BUART_XDMAC_UART2_CH_RX, 0);

		/* Transfer to XDMAC communication mode, disable RXRDY interrupt. */
		uart_disable_interrupt(UART2, UART_IDR_RXRDY);

		xdmac_channel_enable(XDMAC, BUART_XDMAC_UART2_CH_TX);
		xdmac_channel_enable(XDMAC, BUART_XDMAC_UART2_CH_RX);
#endif

		/* Enable the receiver and transmitter. */
		uart_enable_tx(UART2);
		uart_enable_rx(UART2);

#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		/* Configure and enable interrupt of UART. */
		NVIC_SetPriority((IRQn_Type)UART2_IRQn, UART2_PRIO);
		NVIC_EnableIRQ(UART2_IRQn);
#endif

		buart_chn_open[chn] = true;
		num_bytes_rx_uart2 = 0;

		/* Configure TC uart */
		_configure_TC_uart();
		tc_start(TC_UART, TC_UART_CHN);
	}
	break;
#endif

#ifdef CONF_BOARD_UART4
	case 4:
	{
		/* Configure PMC. */
		pmc_enable_periph_clk(ID_UART4);
		/* Configure UART. */
		uart_init(UART4, &uart_console_settings);

		/* Assign buffers to pointers */
		buart_comm_data_4.puc_tq_buf = ptr_tx_uart_buf4;
		buart_comm_data_4.puc_rq_buf = ptr_rx_uart_buf4;
		buart_comm_data_4.us_rq_count = 0;
		buart_comm_data_4.us_rq_idx = 0;
		buart_comm_data_4.us_wq_idx = 0;

#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		/* Get board UART0 PDC base address and enable receiver and
		 * transmitter. */
		g_p_uart_pdc4 = uart_get_pdc_base(UART4);
		pdc_enable_transfer(g_p_uart_pdc4,
				PERIPH_PTCR_RXTEN | PERIPH_PTCR_TXTEN);

		/* Start receiving data and start timer. */
		g_st_uart_rx_packet4.ul_addr = (uint32_t)gs_puc_uart_buf4;
		g_st_uart_rx_packet4.ul_size = UART_BUFFER_SIZE;
		pdc_rx_init(g_p_uart_pdc4, &g_st_uart_rx_packet4, NULL);

		/* Stop transmitting data */
		g_st_uart_tx_packet4.ul_addr = (uint32_t)buart_comm_data_4.puc_tq_buf;
		g_st_uart_tx_packet4.ul_size = 0;
		pdc_tx_init(g_p_uart_pdc4, &g_st_uart_tx_packet4, NULL);

		gs_ul_size_uart_buf4 = UART_BUFFER_SIZE;

		/* Transfer to PDC communication mode, disable RXRDY interrupt
		 * and enable RXBUFF interrupt. */
		uart_disable_interrupt(UART4, UART_IDR_RXRDY);
		uart_enable_interrupt(UART4, UART_IER_RXBUFF);
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
		/* Configure TX and RX XDMAC channels */
		buart4_xdmac_tx_channel_cfg.mbr_ubc = 0;
		buart4_xdmac_tx_channel_cfg.mbr_sa = (uint32_t)buart_comm_data_4.puc_tq_buf;
		buart4_xdmac_tx_channel_cfg.mbr_da = (uint32_t)(&(UART4->UART_THR));
		buart4_xdmac_tx_channel_cfg.mbr_cfg = XDMAC_CC_TYPE_PER_TRAN |
			XDMAC_CC_DSYNC_MEM2PER |
			XDMAC_CC_PERID(BUART_XDMAC_UART4_TX_PERID) |
			XDMAC_CC_CSIZE_CHK_1 |
			XDMAC_CC_MEMSET_NORMAL_MODE |
			XDMAC_CC_MBSIZE_SINGLE |
			XDMAC_CC_DWIDTH_BYTE |
			XDMAC_CC_SIF_AHB_IF0 |
			XDMAC_CC_DIF_AHB_IF1 |
			XDMAC_CC_SAM_INCREMENTED_AM |
			XDMAC_CC_DAM_FIXED_AM;
		buart4_xdmac_tx_channel_cfg.mbr_bc = 0;
		buart4_xdmac_tx_channel_cfg.mbr_ds = 0;
		buart4_xdmac_tx_channel_cfg.mbr_sus = 0;
		buart4_xdmac_tx_channel_cfg.mbr_dus = 0;

		buart4_xdmac_rx_channel_cfg.mbr_ubc = UART_BUFFER_SIZE;
		buart4_xdmac_rx_channel_cfg.mbr_sa = (uint32_t)(&(UART4->UART_RHR));
		buart4_xdmac_rx_channel_cfg.mbr_da = (uint32_t)gs_puc_uart_buf4;
		buart4_xdmac_rx_channel_cfg.mbr_cfg = XDMAC_CC_TYPE_PER_TRAN |
			XDMAC_CC_DSYNC_PER2MEM |
			XDMAC_CC_PERID(BUART_XDMAC_UART4_RX_PERID) |
			XDMAC_CC_CSIZE_CHK_1 |
			XDMAC_CC_MEMSET_NORMAL_MODE |
			XDMAC_CC_MBSIZE_SINGLE |
			XDMAC_CC_DWIDTH_BYTE |
			XDMAC_CC_SIF_AHB_IF1 |
			XDMAC_CC_DIF_AHB_IF0 |
			XDMAC_CC_SAM_FIXED_AM |
			XDMAC_CC_DAM_INCREMENTED_AM;
		buart4_xdmac_rx_channel_cfg.mbr_bc = 0;
		buart4_xdmac_rx_channel_cfg.mbr_ds =  0;
		buart4_xdmac_rx_channel_cfg.mbr_sus = 0;
		buart4_xdmac_rx_channel_cfg.mbr_dus = 0;

		xdmac_configure_transfer(XDMAC, BUART_XDMAC_UART4_CH_TX, &buart4_xdmac_tx_channel_cfg);
		xdmac_channel_set_descriptor_control(XDMAC, BUART_XDMAC_UART4_CH_TX, 0);
		xdmac_configure_transfer(XDMAC, BUART_XDMAC_UART4_CH_RX, &buart4_xdmac_rx_channel_cfg);
		xdmac_channel_set_descriptor_control(XDMAC, BUART_XDMAC_UART4_CH_RX, 0);

		/* Transfer to XDMAC communication mode, disable RXRDY interrupt. */
		uart_disable_interrupt(UART4, UART_IDR_RXRDY);

		xdmac_channel_enable(XDMAC, BUART_XDMAC_UART4_CH_TX);
		xdmac_channel_enable(XDMAC, BUART_XDMAC_UART4_CH_RX);
#endif

		/* Enable the receiver and transmitter. */
		uart_enable_tx(UART4);
		uart_enable_rx(UART4);

#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		/* Configure and enable interrupt of UART. */
		NVIC_SetPriority((IRQn_Type)UART4_IRQn, UART4_PRIO);
		NVIC_EnableIRQ(UART4_IRQn);
#endif

		buart_chn_open[chn] = true;
		num_bytes_rx_uart4 = 0;

		/* Configure TC uart */
		_configure_TC_uart();
		tc_start(TC_UART, TC_UART_CHN);
	}
	break;
#endif
	default:
		return false;
	}
	/* At least one UART is open */
	return true;
}

/**
 * \brief This function closes and disables communication in the specified UART.
 *
 * \param chn  Communication channel [0, 1]
 *
 * \retval true on success.
 * \retval false on failure.
 */
int8_t buart_if_close(uint8_t chn)
{
	if (!buart_chn_open[chn]) {
		return false;
	}

	switch (chn) {
#ifdef CONF_BOARD_UART0
	case 0:
	{
		uart_disable(UART0);
		uart_disable_interrupt(UART0, US_IDR_RXRDY);
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		uart_disable_interrupt(UART0, US_IER_ENDRX);
#endif
		/* Stop TC */
		if (buart_if_get_num_active_channels() == 1) {
			tc_stop(TC_UART, TC_UART_CHN);
		}
		buart_chn_open[chn] = false;
	}
	break;
#endif

#ifdef CONF_BOARD_UART1
	case 1:
	{
		uart_disable(UART1);
		uart_disable_interrupt(UART1, US_IDR_RXRDY);
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		uart_disable_interrupt(UART1, US_IER_ENDRX);
#endif

		/* Stop TC */
		if (buart_if_get_num_active_channels() == 1) {
			tc_stop(TC_UART, TC_UART_CHN);
		}
		buart_chn_open[chn] = false;
	}
	break;
#endif

#ifdef CONF_BOARD_UART2
	case 2:
	{
		uart_disable(UART2);
		uart_disable_interrupt(UART2, US_IDR_RXRDY);
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		uart_disable_interrupt(UART2, US_IER_ENDRX);
#endif

		/* Stop TC */
		if (buart_if_get_num_active_channels() == 1) {
			tc_stop(TC_UART, TC_UART_CHN);
		}
		buart_chn_open[chn] = false;
	}
	break;
#endif
#ifdef CONF_BOARD_UART4
	case 4:
	{
		uart_disable(UART4);
		uart_disable_interrupt(UART4, US_IDR_RXRDY);
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		uart_disable_interrupt(UART4, US_IER_ENDRX);
#endif
		/* Stop TC */
		if (buart_if_get_num_active_channels() == 1) {
			tc_stop(TC_UART, TC_UART_CHN);
		}
		buart_chn_open[chn] = false;
	}
	break;
#endif
	default:
		return false;
	}
	/* At least one UART is closed */
	return true;
}

/**
 * \brief This function receives a message.
 *
 * \note This function receives a given number of characters from the specified
 * UART. If so configured, the function waits until all characters are received.
 * In this case, the watchdog timer must be reloaded to avoid a program reset.
 *
 * \param  chn     Communication channel [0, 1]
 * \param  buffer  Pointer to buffer for information
 * \param  len     Number of characters to receive
 *
 * \retval Number of received characters
 */
uint16_t buart_if_read(uint8_t chn, void *buffer, uint16_t len)
{
#if defined(CONF_BOARD_UART0) || defined(CONF_BOARD_UART1) || defined(CONF_BOARD_UART2) || defined(CONF_BOARD_UART4)
	uint16_t us_rd_idx = 0;
	uint16_t us_num_bytes_read, us_num_bytes_to_end, us_num_bytes_to_start;
	uint16_t us_total_pos;
	uint16_t us_buf_size;

	uint8_t *msg = (uint8_t *)buffer;
#else
	UNUSED(buffer);
	UNUSED(len);
#endif

	/* check uart is open */
	if (!buart_chn_open[chn]) {
		return 0;
	}

	switch (chn) {
#ifdef CONF_BOARD_UART0
	case 0:
		us_buf_size = RX_UART_BUF0_SIZE;
		/* check if there is any byte in rx queue */
		if (buart_comm_data_0.us_rq_count == 0) {
			return 0;
		}

		/* get counters */
		us_rd_idx = buart_comm_data_0.us_rq_idx;
		/* get number of bytes to read */
		if (buart_comm_data_0.us_rq_count >= len) {
			us_num_bytes_read = len;
		} else {
			us_num_bytes_read = buart_comm_data_0.us_rq_count;
		}

		/* check overflow us_rd_idx counter */
		us_total_pos = us_rd_idx + us_num_bytes_read;
		/* copy data to buffer */
#ifdef CONF_BOARD_ENABLE_CACHE
                SCB_InvalidateDCache_by_Addr ((uint32_t*)buart_comm_data_0.puc_rq_buf+us_rd_idx, us_num_bytes_read);
#endif
		if (us_total_pos <= us_buf_size) {
			memcpy(msg, &buart_comm_data_0.puc_rq_buf[us_rd_idx],
					us_num_bytes_read);
			/* update counters */
			buart_comm_data_0.us_rq_count -= us_num_bytes_read;
			buart_comm_data_0.us_rq_idx += us_num_bytes_read;
		} else {
			/* copy data to buffer in fragments -> overflow
			 * us_rq_idx counter */
			us_num_bytes_to_start = us_total_pos - us_buf_size;
			us_num_bytes_to_end = us_num_bytes_read - us_num_bytes_to_start;
			memcpy(msg, &buart_comm_data_0.puc_rq_buf[us_rd_idx],
					us_num_bytes_to_end);
			msg += us_num_bytes_to_end;
			memcpy(msg, &buart_comm_data_0.puc_rq_buf[0],
					us_num_bytes_to_start);
			/* update counters */
			buart_comm_data_0.us_rq_count -= us_num_bytes_read;
			buart_comm_data_0.us_rq_idx = us_num_bytes_to_start;
		}

		break;
#endif

#ifdef CONF_BOARD_UART1
	case 1:
		us_buf_size = RX_UART_BUF1_SIZE;
		/* check if there is any byte in rx queue */
		if (buart_comm_data_1.us_rq_count == 0) {
			return 0;
		}

		/* get counters */
		us_rd_idx = buart_comm_data_1.us_rq_idx;
		/* get number of bytes to read */
		if (buart_comm_data_1.us_rq_count >= len) {
			us_num_bytes_read = len;
		} else {
			us_num_bytes_read = buart_comm_data_1.us_rq_count;
		}

		/* check overflow us_rd_idx counter */
		us_total_pos = us_rd_idx + us_num_bytes_read;
		if (us_total_pos <= us_buf_size) {
			/* copy data to buffer */
			memcpy(msg, &buart_comm_data_1.puc_rq_buf[us_rd_idx],
					us_num_bytes_read);
			/* update counters */
			buart_comm_data_1.us_rq_count -= us_num_bytes_read;
			buart_comm_data_1.us_rq_idx += us_num_bytes_read;
		} else {
			/* copy data to buffer in fragments -> overflow
			 * us_rq_idx counter */
			us_num_bytes_to_start = us_total_pos - us_buf_size;
			us_num_bytes_to_end = us_num_bytes_read - us_num_bytes_to_start;
			memcpy(msg, &buart_comm_data_1.puc_rq_buf[us_rd_idx],
					us_num_bytes_to_end);
			msg += us_num_bytes_to_end;
			memcpy(msg, &buart_comm_data_1.puc_rq_buf[0],
					us_num_bytes_to_start);
			/* update counters */
			buart_comm_data_1.us_rq_count -= us_num_bytes_read;
			buart_comm_data_1.us_rq_idx = us_num_bytes_to_start;
		}

		break;
#endif

#ifdef CONF_BOARD_UART2
	case 2:
		us_buf_size = RX_UART_BUF2_SIZE;
		/* check if there is any byte in rx queue */
		if (buart_comm_data_2.us_rq_count == 0) {
			return 0;
		}

		/* get counters */
		us_rd_idx = buart_comm_data_2.us_rq_idx;
		/* get number of bytes to read */
		if (buart_comm_data_2.us_rq_count >= len) {
			us_num_bytes_read = len;
		} else {
			us_num_bytes_read = buart_comm_data_2.us_rq_count;
		}

		/* check overflow us_rd_idx counter */
		us_total_pos = us_rd_idx + us_num_bytes_read;
#ifdef CONF_BOARD_ENABLE_CACHE
                SCB_InvalidateDCache_by_Addr ((uint32_t*)buart_comm_data_2.puc_rq_buf+us_rd_idx, us_num_bytes_read);
#endif
		if (us_total_pos <= us_buf_size) {
			/* copy data to buffer */
			memcpy(msg, &buart_comm_data_2.puc_rq_buf[us_rd_idx],
					us_num_bytes_read);
			/* update counters */
			buart_comm_data_2.us_rq_count -= us_num_bytes_read;
			buart_comm_data_2.us_rq_idx += us_num_bytes_read;
		} else {
			/* copy data to buffer in fragments -> overflow
			 * us_rq_idx counter */
			us_num_bytes_to_start = us_total_pos - us_buf_size;
			us_num_bytes_to_end = us_num_bytes_read - us_num_bytes_to_start;
			memcpy(msg, &buart_comm_data_2.puc_rq_buf[us_rd_idx],
					us_num_bytes_to_end);
			msg += us_num_bytes_to_end;
			memcpy(msg, &buart_comm_data_2.puc_rq_buf[0],
					us_num_bytes_to_start);
			/* update counters */
			buart_comm_data_2.us_rq_count -= us_num_bytes_read;
			buart_comm_data_2.us_rq_idx = us_num_bytes_to_start;
		}

		break;
#endif


#ifdef CONF_BOARD_UART4
	case 4:
		us_buf_size = RX_UART_BUF4_SIZE;
		/* check if there is any byte in rx queue */
		if (buart_comm_data_4.us_rq_count == 0) {
			return 0;
		}

		/* get counters */
		us_rd_idx = buart_comm_data_4.us_rq_idx;
		/* get number of bytes to read */
		if (buart_comm_data_4.us_rq_count >= len) {
			us_num_bytes_read = len;
		} else {
			us_num_bytes_read = buart_comm_data_4.us_rq_count;
		}

		/* check overflow us_rd_idx counter */
		us_total_pos = us_rd_idx + us_num_bytes_read;
#ifdef CONF_BOARD_ENABLE_CACHE
                SCB_InvalidateDCache_by_Addr ((uint32_t*)buart_comm_data_4.puc_rq_buf+us_rd_idx, us_num_bytes_read);
#endif
		if (us_total_pos <= us_buf_size) {
			memcpy(msg, &buart_comm_data_4.puc_rq_buf[us_rd_idx],
					us_num_bytes_read);
			/* update counters */
			buart_comm_data_4.us_rq_count -= us_num_bytes_read;
			buart_comm_data_4.us_rq_idx += us_num_bytes_read;
		} else {
			/* copy data to buffer in fragments -> overflow
			 * us_rq_idx counter */
			us_num_bytes_to_start = us_total_pos - us_buf_size;
			us_num_bytes_to_end = us_num_bytes_read - us_num_bytes_to_start;
			memcpy(msg, &buart_comm_data_4.puc_rq_buf[us_rd_idx],
					us_num_bytes_to_end);
			msg += us_num_bytes_to_end;
			memcpy(msg, &buart_comm_data_4.puc_rq_buf[0],
					us_num_bytes_to_start);
			/* update counters */
			buart_comm_data_4.us_rq_count -= us_num_bytes_read;
			buart_comm_data_4.us_rq_idx = us_num_bytes_to_start;
		}

		break;
#endif
	default:
		return 0;
	}
	/* Bytes read from UART */
	return us_num_bytes_read;
}

/**
 * \brief This function transmits a message.
 *
 * \note This function transmits characters via the specified UART.
 * If so configured, the function waits until all characters are inserted
 * in the transmission queue. In this case, the watchdog timer must be
 * reloaded to avoid a program reset.
 *
 * \param  chn     Communication channel [0, 1]
 * \param  buffer  Pointer to information to transmit
 * \param  len     Number of characters to transmit
 *
 * \retval Number of characters sent
 */
uint16_t buart_if_write(uint8_t chn, const void *buffer, uint16_t len)
{
#if !defined(CONF_BOARD_UART0) || !defined(CONF_BOARD_UART1) || defined(CONF_BOARD_UART2) || defined(CONF_BOARD_UART4)
	UNUSED(buffer);
	UNUSED(len);
#endif

	/* check uart is open */
	if (!buart_chn_open[chn]) {
		return 0;
	}

	switch (chn) {
#ifdef CONF_BOARD_UART0
	case 0:
		if (len > TX_UART_BUF0_SIZE) {
			return 0;
		}

		/* Wait previous TX transfer finish */
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		while (pdc_read_tx_counter(g_p_uart_pdc0) > 0) {
		}
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
		while (xdmac_channel_get_status(XDMAC) & BUART_XDMAC_UART0_TX_STATUS) {
		}
#endif
		memcpy(&buart_comm_data_0.puc_tq_buf[0], buffer, len);
#ifdef CONF_BOARD_ENABLE_CACHE
		SCB_InvalidateDCache_by_Addr ((uint32_t*)buart_comm_data_0.puc_tq_buf, len);                
#endif
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		g_st_uart_tx_packet0.ul_addr = (uint32_t)&buart_comm_data_0.puc_tq_buf[0];
		g_st_uart_tx_packet0.ul_size = len;
		pdc_tx_init(g_p_uart_pdc0, &g_st_uart_tx_packet0, NULL);
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
		buart0_xdmac_tx_channel_cfg.mbr_ubc = len;
		xdmac_configure_transfer(XDMAC, BUART_XDMAC_UART0_CH_TX, &buart0_xdmac_tx_channel_cfg);
		xdmac_channel_enable(XDMAC, BUART_XDMAC_UART0_CH_TX);                
#endif
		return len;
#endif

#ifdef CONF_BOARD_UART1
	case 1:
		if (len > TX_UART_BUF1_SIZE) {
			return 0;
		}

		/* Wait previous TX transfer finish */
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		while (pdc_read_tx_counter(g_p_uart_pdc1) > 0) {
		}
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
		while (xdmac_channel_get_status(XDMAC) & BUART_XDMAC_UART1_TX_STATUS) {
		}
#endif
		memcpy(&buart_comm_data_1.puc_tq_buf[0], buffer, len);
#ifdef CONF_BOARD_ENABLE_CACHE
		SCB_InvalidateDCache_by_Addr ((uint32_t*)buart_comm_data_1.puc_tq_buf, len);                
#endif
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		g_st_uart_tx_packet1.ul_addr = (uint32_t)&buart_comm_data_1.puc_tq_buf[0];
		g_st_uart_tx_packet1.ul_size = len;
		pdc_tx_init(g_p_uart_pdc1, &g_st_uart_tx_packet1, NULL);
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
		buart1_xdmac_tx_channel_cfg.mbr_ubc = len;
		xdmac_configure_transfer(XDMAC, BUART_XDMAC_UART1_CH_TX, &buart1_xdmac_tx_channel_cfg);
		xdmac_channel_enable(XDMAC, BUART_XDMAC_UART1_CH_TX);
#endif
		return len;
#endif


#ifdef CONF_BOARD_UART2
	case 2:
		if (len > TX_UART_BUF2_SIZE) {
			return 0;
		}

		/* Wait previous TX transfer finish */
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		while (pdc_read_tx_counter(g_p_uart_pdc2) > 0) {
		}
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
		while (xdmac_channel_get_status(XDMAC) & BUART_XDMAC_UART2_TX_STATUS) {
		}
#endif
		memcpy(&buart_comm_data_2.puc_tq_buf[0], buffer, len);
#ifdef CONF_BOARD_ENABLE_CACHE
		SCB_InvalidateDCache_by_Addr ((uint32_t*)buart_comm_data_2.puc_tq_buf, len);
#endif
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		g_st_uart_tx_packet2.ul_addr = (uint32_t)&buart_comm_data_2.puc_tq_buf[0];
		g_st_uart_tx_packet2.ul_size = len;
		pdc_tx_init(g_p_uart_pdc2, &g_st_uart_tx_packet2, NULL);
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
		buart2_xdmac_tx_channel_cfg.mbr_ubc = len;
		xdmac_configure_transfer(XDMAC, BUART_XDMAC_UART2_CH_TX, &buart2_xdmac_tx_channel_cfg);
		xdmac_channel_enable(XDMAC, BUART_XDMAC_UART2_CH_TX);
#endif
		return len;
#endif

#ifdef CONF_BOARD_UART4
	case 4:
		if (len > TX_UART_BUF4_SIZE) {
			return 0;
		}

		/* Wait previous TX transfer finish */
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		while (pdc_read_tx_counter(g_p_uart_pdc4) > 0) {
		}
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
		while (xdmac_channel_get_status(XDMAC) & BUART_XDMAC_UART4_TX_STATUS) {
		}
#endif
		memcpy(&buart_comm_data_4.puc_tq_buf[0], buffer, len);
#ifdef CONF_BOARD_ENABLE_CACHE
		SCB_InvalidateDCache_by_Addr ((uint32_t*)buart_comm_data_4.puc_tq_buf, len);
#endif
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		g_st_uart_tx_packet4.ul_addr = (uint32_t)&buart_comm_data_4.puc_tq_buf[0];
		g_st_uart_tx_packet4.ul_size = len;
		pdc_tx_init(g_p_uart_pdc4, &g_st_uart_tx_packet4, NULL);
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
		buart4_xdmac_tx_channel_cfg.mbr_ubc = len;
		xdmac_configure_transfer(XDMAC, BUART_XDMAC_UART4_CH_TX, &buart4_xdmac_tx_channel_cfg);
		xdmac_channel_enable(XDMAC, BUART_XDMAC_UART4_CH_TX);
#endif
		return len;
#endif
	default:
		return 0;
	}
}

/**
 * \brief Get byte from UART.
 *
 * \param  chn  Communication channel [0, 1]
 *
 * \retval Byte received
 * \retval -1 in case of no reception
 */
int buart_if_rx_char(uint8_t chn)
{
	uint8_t buf[4] = {0, 0, 0, 0};

	if (buart_if_read(chn, buf, 1) <= 0) {
		return (-1);
	}

	return buf[0];
}

/**
 * \brief Sent byte to UART.
 *
 * \param  chn   Communication channel [0, 1]
 * \param  data  Data to sent
 *
 * \retval Number of characters sent
 */
uint16_t buart_if_tx_char(uint8_t chn, char data)
{
	return (buart_if_write(chn, &data, 1));
}

/**
 * \brief Check UART PDC transmission in course.
 *
 * \param  chn   Communication channel [0, 1]
 *
 * \retval true is UART is free to tx, false in otherwise
 */
bool buart_if_is_free(uint8_t chn)
{
	/* check uart is open */
	if (!buart_chn_open[chn]) {
		return false;
	}

	switch (chn) {
#ifdef CONF_BOARD_UART0
	case 0:
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		if (pdc_read_tx_counter(g_p_uart_pdc0)) {
			return false;
		} else {
			return true;
		}
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
		if (xdmac_channel_get_status(XDMAC) & BUART_XDMAC_UART0_TX_STATUS) {
			return false;
		} else {
			return true;
		}
#endif
#endif

#ifdef CONF_BOARD_UART1
	case 1:
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		if (pdc_read_tx_counter(g_p_uart_pdc1)) {
			return false;
		} else {
			return true;
		}
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
		if (xdmac_channel_get_status(XDMAC) & BUART_XDMAC_UART1_TX_STATUS) {
			return false;
		} else {
			return true;
		}
#endif
#endif


#ifdef CONF_BOARD_UART2
	case 2:
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		if (pdc_read_tx_counter(g_p_uart_pdc2)) {
			return false;
		} else {
			return true;
		}
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
		if (xdmac_channel_get_status(XDMAC) & BUART_XDMAC_UART2_TX_STATUS) {
			return false;
		} else {
			return true;
		}
#endif
#endif

#ifdef CONF_BOARD_UART4
	case 4:
#if (SAM4S || SAM4E || SAM4N || SAM4C || SAMG || SAM4CP || SAM4CM || PIC32CX)
		if (pdc_read_tx_counter(g_p_uart_pdc4)) {
			return false;
		} else {
			return true;
		}
#elif (SAM3U  || SAM3XA || SAMV71 || SAMV70 || SAME70 || SAMS70)
		if (xdmac_channel_get_status(XDMAC) & BUART_XDMAC_UART4_TX_STATUS) {
			return false;
		} else {
			return true;
		}
#endif
#endif
	default:
		return false;
	}
}

/**
 * \brief Get number of active channels.
 *
 * \retval number of active channels
 */
uint8_t buart_if_get_num_active_channels(void){
  
  uint8_t uc_result = 0;
  uint8_t uc_i;
  
  for(uc_i = 0; uc_i < 5 ; uc_i ++){
    if( buart_chn_open[uc_i] == true ){
      uc_result++;
    }
  }    
  return uc_result;
}
/* @} */
#else

int8_t buart_if_open(uint8_t chn, uint32_t bauds) {
	(void)chn;
	(void)bauds;
	return false;
}

int8_t buart_if_close(uint8_t chn) {
	(void)chn;
	return false;
}

uint16_t buart_if_read(uint8_t chn, void *buffer, uint16_t len) {
	(void)chn;
	(void)buffer;
	(void)len;
	return 0;
}

uint16_t buart_if_write(uint8_t chn, const void *buffer, uint16_t len) {
	(void)chn;
	(void)buffer;
	(void)len;
	return 0;
}

int buart_if_rx_char(uint8_t chn) {
	(void)chn;
	return -1;
}

uint16_t buart_if_tx_char(uint8_t chn, char data) {
	(void)chn;
	(void)data;
	return false;
}

bool buart_if_is_free(uint8_t chn) {
	(void)chn;
	return false;
}


/**
 * \brief Get number of active channels.
 *
 * \retval number of active channels
 */
uint8_t buart_if_get_num_active_channels(void){
  return 0;
}

#endif



/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* @endcond */
