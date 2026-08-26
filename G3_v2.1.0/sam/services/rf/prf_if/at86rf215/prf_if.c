/**
 *
 * \file
 *
 * \brief Proxy RF Controller interface layer implementation.
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

#include "prf_if.h"
#include "conf_prf_if.h"
#include "ioport.h"
#include "spi.h"
#include "pio.h"
#include "pio_handler.h"
#include "delay.h"
#include "string.h"
#if SAME70
# include "conf_board.h"
# include "xdmac.h"
#else
# include "pdc.h"
#endif

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* @endcond */

/* Interrupt, reset and LED pins definitions */
#define PRF_INT_PIO                    arch_ioport_pin_to_base(PRF_INT_GPIO)
#define PRF_INT_ID                     pio_get_pin_group_id(PRF_INT_GPIO)
#define PRF_INT_MASK                   ioport_pin_to_mask(PRF_INT_GPIO)
#define PRF_INT_IRQn                   (IRQn_Type)PRF_INT_ID
#define PRF_RESET_ID                   pio_get_pin_group_id(PRF_RESET_GPIO)
#define PRF_LED_TX_PIO                 arch_ioport_pin_to_base(PRF_LED_TX_GPIO)
#define PRF_LED_TX_ID                  pio_get_pin_group_id(PRF_LED_TX_GPIO)
#define PRF_LED_TX_MASK                ioport_pin_to_mask(PRF_LED_TX_GPIO)
#define PRF_LED_RX_PIO                 arch_ioport_pin_to_base(PRF_LED_RX_GPIO)
#define PRF_LED_RX_ID                  pio_get_pin_group_id(PRF_LED_RX_GPIO)
#define PRF_LED_RX_MASK                ioport_pin_to_mask(PRF_LED_RX_GPIO)
#define PRF_SPI_IRQn                   (IRQn_Type)PRF_SPI_ID

/* Reset pin pulse width From RF215 datasheet (Table 10-3): Min tRST = 625 ns */
#define PRF_RST_PULSE_US               10

/* Waiting time after reset RF transceiver
 * From RF215 datasheet (Table 10-7): Max tPOWERON = 500 us
 * From RF215 datasheet (Table 10-7): Max tRESET_TRXOFF =  1 us */
#define PRF_RST_WAIT_US                1000

#ifdef PRF_ENABLE_SPICLK_AUTO_CONFIGURE

/* Minimum time required between SPI LSB last byte to MSB next byte
 * From RF215 datasheet (Table 10-29): Min tSPI_5 = 125 ns */
# define PRF_SPI_MIN_TIME_BTW_BYTES_NS 125

/* Minimum time required between SPI LSB last byte to LSB next byte
 * Not in datasheet: 125 ns */
# define PRF_SPI_MIN_BYTE_DURATION_NS  875

/* Minimum time required between SPI CS falling edge to SCLK rising edge
 * From RF215 datasheet (Table 10-29): Min tSPI_0 = 50 ns */
# define PRF_SPI_MIN_TIME_FIRST_BIT_NS 50

/* Typical SPI clock frequency. This value is taken as maximum frequency
 * From RF215 datasheet (Table 10-29): Typ fSCLK = 25 MHz */
# define PRF_SPI_TYP_CLK_FREQ_HZ       25000000
#endif

/* RF SPI COMMAND definition (Table 4-4 in RF215 datasheet) */
/* COMMAND is 16 bits, containing MODE (read / write) and ADDRESS */
/* COMMAND[15:14] = MODE[1:0] */
/* COMMAND[13:0] = ADDRESS[13:0] */
#define PRF_SPI_CMD_MODE_POS           14
#define PRF_SPI_CMD_MODE_MASK          (3u << PRF_SPI_CMD_MODE_POS)
#define PRF_SPI_CMD_MODE_READ          (0u << PRF_SPI_CMD_MODE_POS)
#define PRF_SPI_CMD_MODE_WRITE         (2u << PRF_SPI_CMD_MODE_POS)
#define PRF_SPI_CMD_ADDR_MASK          0x3FFFF

/* Last RF transceiver register address */
/* RF215 datasheet Register Summary: 0x3FFE (BBC1_FBTXE) */
#define PRF_LAST_REG_ADDR              0x3FFFE

#ifndef PRF_TRX_MAX_MSG_LEN
/* Default maximum message length. RF215 buffer size is 2047 bytes. */
# define PRF_TRX_MAX_MSG_LEN           2047
#endif

/* SPI Header (COMMAND) size. */
#define PRF_SPI_COMMAND_SIZE           2

#if SAME70 && defined(CONF_BOARD_ENABLE_CACHE)

/* If cortex-M7 cache enabled, there are incoherency issues when using DMA.
 * They can be solved by using TCM, MPU to define non-cacheable region, or using
 * cache maintenance operations. If TCM or non-cacheable region is enabled in
 * conf_board.h, it is assumed that all data used by DMA (prf_if, at86rf215
 * component, stack) will be mapped accordingly by the linker script, and
 * therefore cache maintenance operations are not performed. */
# if (!defined(CONF_BOARD_ENABLE_TCM_AT_INIT)) && !(defined(CONF_BOARD_CONFIG_MPU_AT_INIT) && defined(MPU_HAS_NOCACHE_REGION))
#  define PRF_CM7_CACHE_MAINTENANCE
# endif
#endif

#ifdef PRF_CM7_CACHE_MAINTENANCE

/* When using cache maintenance operations, buffers used by DMA must be aligned
 * to cache line size (32 bytes). Consequently, an intermediate reception buffer
 * is needed to ensure 32-byte alignment and avoid issues */

/* DMA buffer size */
# define PRF_SPI_BUFFER_SIZE           (div32_ceil(PRF_SPI_COMMAND_SIZE + PRF_TRX_MAX_MSG_LEN) << 5)

/* DMA Transmission buffer */
COMPILER_ALIGNED(32)
static uint8_t spuc_prf_tx_buffer[PRF_SPI_BUFFER_SIZE];

/* DMA Reception buffer */
COMPILER_ALIGNED(32)
static uint8_t spuc_prf_rx_buffer[PRF_SPI_BUFFER_SIZE];

/* Pointer and size of data to be read (from upper layer) */
static uint8_t *spuc_prf_rx_addr;
static uint16_t sus_prf_rx_len;
static bool sb_prf_read_pending;
#else
/** DMA buffer size. */
# define PRF_SPI_BUFFER_SIZE           (PRF_SPI_COMMAND_SIZE + PRF_TRX_MAX_MSG_LEN)

/* DMA Transmission buffer */
static uint8_t spuc_prf_tx_buffer[PRF_SPI_BUFFER_SIZE];

/* DMA Reception buffer (dummy). Used only to receive the first two bytes */
static uint8_t spuc_prf_rx_buffer_cmd_dummy[PRF_SPI_COMMAND_SIZE];
#endif

#if SAME70

/* XDMAC channels used for PRF_IF SPI */
# define PRF_XDMAC_CH_SPI_TX           14
# define PRF_XDMAC_CH_SPI_RX           15

/* XDMAC channel hardware interface number for corresponding SPI */
# if PRF_SPI_ID == ID_SPI0
#  define PRF_XDMAC_PERID_SPI_TX       XDMAC_CHANNEL_HWID_SPI0_TX
#  define PRF_XDMAC_PERID_SPI_RX       XDMAC_CHANNEL_HWID_SPI0_RX
# elif PRF_SPI_ID == ID_SPI1
#  define PRF_XDMAC_PERID_SPI_TX       XDMAC_CHANNEL_HWID_SPI1_TX
#  define PRF_XDMAC_PERID_SPI_RX       XDMAC_CHANNEL_HWID_SPI1_RX
# endif

# ifndef PRF_CM7_CACHE_MAINTENANCE
/* XDMAC RX descriptor (used in read mode for command reception) */
static lld_view0 sx_prf_rx_descr_cmd_dummy;
/* XDMAC RX descriptor (used in read mode for data reception) */
static lld_view0 sx_prf_rx_descr_data;
# endif
#else
/* PDC TX data packet */
static pdc_packet_t sx_prf_tx_packet;
/* PDC RX data packet (used in read mode for command reception) */
static pdc_packet_t sx_prf_rx_packet_cmd_dummy;
/* PDC RX data packet (used in read mode for data reception) */
static pdc_packet_t sx_prf_rx_packet_data;
/* Pointer to PDC register base */
static Pdc *spx_prf_pdc;
#endif

/* Critical section control (enable/disable IRQ) */
static uint32_t sul_prf_irq_crit_sect_count;

/* PRF interrupt handler */
static void (*prf_handler)(void);

/**
 * \brief RF IRQ pin handler.
 */
static void _prf_if_int_handler(uint32_t ul_id, uint32_t ul_mask)
{
	UNUSED(ul_id);
	UNUSED(ul_mask);
	if (prf_handler != NULL) {
		prf_handler();
	}

	/* Clear IRQ flags */
	pio_get_interrupt_status(PRF_INT_PIO);
}

#ifdef PRF_ENABLE_SPICLK_AUTO_CONFIGURE

/**
 * \brief Divider, DLYBS and DLYBCT SPI parameters configuration.
 * (Auto-configuration enabled). The best combination that fulfills the RF215
 * SPI timing requirements and maximizing the data rate.
 *
 * \param[out] puc_dlybs Pointer to DLYBS parameter
 * \param[out] puc_dlybct Pointer to DLYBCT parameter
 * \param[out] puc_byte_time_us_q5 Pointer to time of one byte transaction in us
 * with 5 comma bits [uQ3.5]
 *
 * \return SPI clock divider (SCBR)
 */
static inline uint8_t _prf_if_get_spi_params(uint8_t *puc_dlybs, uint8_t *puc_dlybct, uint8_t *puc_byte_time_us_q5)
{
	uint64_t ull_cycles;
	uint64_t ull_total_cycles_aux;
	uint32_t ul_periph_clk_hz;
	uint16_t us_cycles_best;
	uint8_t uc_div_min;
	uint8_t uc_div_best = 4;
	uint8_t uc_dlybct;
	uint8_t uc_dlybct_max;
	uint8_t uc_dlybct_min;
	uint8_t uc_dlybct_best = 0;
	uint8_t uc_cycles_btw_bytes_min;
	uint8_t uc_cycles_byte_duration_min;

	/* Get peripheral clock */
	ul_periph_clk_hz = sysclk_get_peripheral_hz();

	/* Compute minimum number of peripheral clock cycles between LSB last
	 * byte to MSB next byte */
	ull_cycles = (uint64_t)ul_periph_clk_hz * PRF_SPI_MIN_TIME_BTW_BYTES_NS;
	uc_cycles_btw_bytes_min = div_ceil(ull_cycles, 1000000000);

	/* Compute minimum number of peripheral clock cycles between LSB last
	 * byte to LSB next byte */
	ull_cycles = (uint64_t)ul_periph_clk_hz * PRF_SPI_MIN_BYTE_DURATION_NS;
	uc_cycles_byte_duration_min = div_ceil(ull_cycles, 1000000000);

	/* Compute needed number of peripheral clock cycles between between CS
	 * falling edge to SCLK rising edge. This is directly DLYBS */
	ull_cycles = (uint64_t)ul_periph_clk_hz * PRF_SPI_MIN_TIME_FIRST_BIT_NS;
	*puc_dlybs = div_ceil(ull_cycles, 1000000000);

	/* Compute minimum clock divider (maximum SPI CLK frequency) */
	uc_div_min = div_ceil(ul_periph_clk_hz, PRF_SPI_TYP_CLK_FREQ_HZ);

	if (uc_div_min >= uc_cycles_btw_bytes_min) {
		/* One SPCK cycle at maximum frequency is enough. No additional
		 * delay between bytes is needed */
		uc_dlybct_max = 0;
	} else {
		/* With maximum frequency, additional delay between bytes is
		 * needed. Compute that maximum DLYBCT needed.
		 * Delay is computed as 32*DLYCBT/FperiphClk */
		uc_dlybct_max = div32_ceil(uc_cycles_btw_bytes_min - uc_div_min);
	}

	/* CPU/DMA need >=32 cycles between bytes (DIV>=4 or DLYBCT>=1) */
	if (uc_div_min < 4) {
		uc_dlybct_min = 1;
		uc_dlybct_max = max(uc_dlybct_max, 1);
	} else {
		uc_dlybct_min = 0;
	}

	/* Find the best combination of divider/DLYBCT */
	us_cycles_best = UINT16_MAX;
	for (uc_dlybct = uc_dlybct_min; uc_dlybct <= uc_dlybct_max; uc_dlybct++) {
		uint16_t us_total_cycles;
		uint8_t uc_dlybct_cycles;
		uint8_t uc_div1;
		uint8_t uc_div2;
		uint8_t uc_div;

		/* Number of peripheral clock cycles for this DLYBCT */
		uc_dlybct_cycles = uc_dlybct << 5;

		/* Minimum divider allowed for this DLYBCT (1) */
		if (uc_cycles_btw_bytes_min > (uc_div_min + uc_dlybct_cycles)) {
			uc_div1 = uc_cycles_btw_bytes_min - uc_dlybct_cycles;
		} else {
			uc_div1 = uc_div_min;
		}

		/* Minimum divider allowed for this DLYBCT (2) */
		if (uc_cycles_byte_duration_min > ((uc_div_min << 3) + uc_dlybct_cycles)) {
			uc_div2 = div8_ceil(uc_cycles_byte_duration_min - uc_dlybct_cycles);
		} else {
			uc_div2 = uc_div_min;
		}

		/* Minimum divider allowed for this DLYBCT (worts case) */
		uc_div = max(uc_div1, uc_div2);

		/* Total number of peripheral clock cycles for one byte */
		/* 8 SPCK cycles + DLYBCT cycles */
		us_total_cycles = (uc_div << 3) + uc_dlybct_cycles;

		if (us_total_cycles < us_cycles_best) {
			/* This combination is the best one up to now */
			us_cycles_best = us_total_cycles;
			uc_div_best = uc_div;
			uc_dlybct_best = uc_dlybct;
		}
	}

	/* Return time in us [uQ3.5] for one byte */
	ull_total_cycles_aux = (uint64_t)us_cycles_best * (1000000 << 5);
	*puc_byte_time_us_q5 = (uint8_t)div_round(ull_total_cycles_aux, ul_periph_clk_hz);

	/* Return divider and DLYBCT */
	*puc_dlybct = uc_dlybct_best;
	return uc_div_best;
}

#else

/**
 * \brief Divider, DLYBS and DLYBCT SPI parameters configuration.
 * (Auto-configuration disabled). Parameters are taken from are taken from
 * conf_prf_if.h. It is left to the user to fulfills the RF215 SPI timing
 * requirements.
 *
 * \param[out] puc_dlybs Pointer to DLYBS parameter
 * \param[out] puc_dlybct Pointer to DLYBCT parameter
 * \param[out] puc_byte_time_us_q5 Pointer to time of one byte transaction in us
 * with 5 comma bits [uQ3.5]
 *
 * \return SPI clock divider (SCBR)
 */
static inline uint8_t _prf_if_get_spi_params(uint8_t *puc_dlybs, uint8_t *puc_dlybct, uint8_t *puc_byte_time_us_q5)
{
	uint64_t ull_total_cycles_aux;
	uint32_t ul_periph_clk_hz;
	uint32_t ul_div;
	uint16_t us_total_cycles;
	uint8_t uc_div;
	uint8_t uc_dlybs;
	uint8_t uc_dlybct;

	/* Get peripheral clock */
	ul_periph_clk_hz = sysclk_get_peripheral_hz();

	/* Auto configuration disabled */
	/* Divider, DLYBCT and DLYBS are taken from conf_prf_if.h */
	ul_div = div_round(ul_periph_clk_hz, PRF_SPI_CLOCK);

	/* Force divider, DLYBCT and DLYBS to be between 1 and 255 */
	uc_div = max(min(ul_div, 255), 1);
	uc_dlybs = min(PRF_SPI_DLYBS, 255);
	uc_dlybct = min(PRF_SPI_DLYBCT, 255);

	/* Return DLYBCT and DLYBS */
	*puc_dlybs = uc_dlybs;
	*puc_dlybct = uc_dlybct;

	/* Total number of peripheral clock cycles for one byte */
	/* 8 SPCK cycles + DLYBCT cycles */
	us_total_cycles = (uc_div << 3) + (uc_dlybct << 5);

	/* Return time in us [uQ3.5] for one byte */
	ull_total_cycles_aux = (uint64_t)us_total_cycles * (1000000 << 5);
	*puc_byte_time_us_q5 = (uint8_t)div_round(ull_total_cycles_aux, ul_periph_clk_hz);

	return uc_div;
}

#endif

/**
 * \brief Initialize and configure SPI peripheral.
 *
 * \return Time of one byte transaction in us with 5 comma bits [uQ3.5]
 */
static inline uint8_t _prf_if_spi_init(void)
{
	uint8_t uc_byte_time_us_q5;
	uint8_t uc_div;
	uint8_t uc_dlybs;
	uint8_t uc_dlybct;

	/* Get SPI parameters: Divider, DLYBS, DLYBCT */
	uc_div = _prf_if_get_spi_params(&uc_dlybs, &uc_dlybct, &uc_byte_time_us_q5);

	/* Enable SPI peripheral clock */
	spi_enable_clock(PRF_SPI_MODULE);

	if (spi_is_enabled(PRF_SPI_MODULE) == 0) {
		/* Reset and configure SPI. Common registers for different Chip
		 * Selects are only written if SPI is not enabled yet */
		spi_disable(PRF_SPI_MODULE);
		spi_reset(PRF_SPI_MODULE);
		spi_set_master_mode(PRF_SPI_MODULE);
		spi_disable_mode_fault_detect(PRF_SPI_MODULE);
		spi_set_fixed_peripheral_select(PRF_SPI_MODULE);
		spi_enable(PRF_SPI_MODULE);
	}

	/* Configure SPI (registers for specific Chip Select) */
	spi_set_clock_polarity(PRF_SPI_MODULE, PRF_SPI_CS, 0);
	spi_set_clock_phase(PRF_SPI_MODULE, PRF_SPI_CS, 1);
	spi_set_bits_per_transfer(PRF_SPI_MODULE, PRF_SPI_CS, SPI_CSR_BITS_8_BIT);
	spi_set_baudrate_div(PRF_SPI_MODULE, PRF_SPI_CS, uc_div);
	spi_set_transfer_delay(PRF_SPI_MODULE, PRF_SPI_CS, uc_dlybs, uc_dlybct);
	spi_configure_cs_behavior(PRF_SPI_MODULE, PRF_SPI_CS, SPI_CS_RISE_NO_TX);
	spi_disable_interrupt(PRF_SPI_MODULE, SPI_IDR_TXEMPTY);

	return uc_byte_time_us_q5;
}

#if SAME70

/**
 * \brief Initialize and configure SPI XDMAC (SAME70).
 */
static inline void _prf_if_dma_spi_init(void)
{
	/* Initialize and enable DMA controller */
	sysclk_enable_peripheral_clock(ID_XDMAC);

	/* Configure DMA Tx channel for single transaction (without descriptor) */
	xdmac_channel_set_destination_addr(XDMAC, PRF_XDMAC_CH_SPI_TX, (uint32_t)spi_get_tx_access(PRF_SPI_MODULE));
	xdmac_channel_set_descriptor_control(XDMAC, PRF_XDMAC_CH_SPI_TX, XDMAC_CNDC_NDE_DSCR_FETCH_DIS);
	xdmac_channel_set_block_control(XDMAC, PRF_XDMAC_CH_SPI_TX, 0);
	xdmac_channel_set_config(XDMAC, PRF_XDMAC_CH_SPI_TX, XDMAC_CC_TYPE_PER_TRAN | XDMAC_CC_DSYNC_MEM2PER |
			XDMAC_CC_CSIZE_CHK_1 | XDMAC_CC_DWIDTH_BYTE | XDMAC_CC_SIF_AHB_IF0 |
			XDMAC_CC_DIF_AHB_IF1 | XDMAC_CC_SAM_INCREMENTED_AM | XDMAC_CC_DAM_FIXED_AM |
			XDMAC_CC_PERID(PRF_XDMAC_PERID_SPI_TX));

# ifdef PRF_CM7_CACHE_MAINTENANCE
	/* Configure DMA Rx channel for single transaction (without descriptor) */
	xdmac_channel_set_source_addr(XDMAC, PRF_XDMAC_CH_SPI_RX, (uint32_t)spi_get_rx_access(PRF_SPI_MODULE));
	xdmac_channel_set_descriptor_control(XDMAC, PRF_XDMAC_CH_SPI_RX, XDMAC_CNDC_NDE_DSCR_FETCH_DIS);
	xdmac_channel_set_block_control(XDMAC, PRF_XDMAC_CH_SPI_RX, 0);
	xdmac_channel_set_config(XDMAC, PRF_XDMAC_CH_SPI_RX, XDMAC_CC_TYPE_PER_TRAN | XDMAC_CC_DSYNC_PER2MEM |
			XDMAC_CC_CSIZE_CHK_1 | XDMAC_CC_DWIDTH_BYTE | XDMAC_CC_SIF_AHB_IF1 |
			XDMAC_CC_DIF_AHB_IF0 | XDMAC_CC_SAM_FIXED_AM | XDMAC_CC_DAM_INCREMENTED_AM |
			XDMAC_CC_PERID(PRF_XDMAC_PERID_SPI_RX));
# else
	/* Configure DMA Rx channel */
	xdmac_channel_set_source_addr(XDMAC, PRF_XDMAC_CH_SPI_RX, (uint32_t)spi_get_rx_access(PRF_SPI_MODULE));
	xdmac_channel_set_block_control(XDMAC, PRF_XDMAC_CH_SPI_RX, 0);
	xdmac_channel_set_config(XDMAC, PRF_XDMAC_CH_SPI_RX, XDMAC_CC_TYPE_PER_TRAN | XDMAC_CC_DSYNC_PER2MEM |
			XDMAC_CC_CSIZE_CHK_1 | XDMAC_CC_DWIDTH_BYTE | XDMAC_CC_SIF_AHB_IF1 |
			XDMAC_CC_DIF_AHB_IF0 | XDMAC_CC_SAM_FIXED_AM | XDMAC_CC_DAM_INCREMENTED_AM |
			XDMAC_CC_PERID(PRF_XDMAC_PERID_SPI_RX));

	/* Configure DMA Rx descriptor for header reception in read mode (dummy, constant) */
	sx_prf_rx_descr_cmd_dummy.mbr_nda = (uint32_t)&sx_prf_rx_descr_data;
	sx_prf_rx_descr_cmd_dummy.mbr_ubc = XDMAC_UBC_UBLEN(PRF_SPI_COMMAND_SIZE) | XDMAC_UBC_NDE_FETCH_EN |
			XDMAC_UBC_NSEN_UNCHANGED | XDMAC_UBC_NDEN_UPDATED | XDMAC_UBC_NVIEW_NDV0;
	sx_prf_rx_descr_cmd_dummy.mbr_da = (uint32_t)spuc_prf_rx_buffer_cmd_dummy;
# endif
}

#else

/**
 * \brief Initialize and configure SPI PDC (SAMG55, SAM4C, PIC32CX).
 */
static inline void _prf_if_dma_spi_init(void)
{
	/* Get board PRF PDC base address and enable receiver and transmitter */
	spx_prf_pdc = spi_get_pdc_base(PRF_SPI_MODULE);

	/* Configure DMA Tx channel. Only address (constant) */
	sx_prf_tx_packet.ul_addr = (uint32_t)spuc_prf_tx_buffer;

	/* Configure DMA Rx channel for header reception in read mode (dummy, constant) */
	sx_prf_rx_packet_cmd_dummy.ul_addr = (uint32_t)spuc_prf_rx_buffer_cmd_dummy;
	sx_prf_rx_packet_cmd_dummy.ul_size = PRF_SPI_COMMAND_SIZE;
}

#endif

/**
 * \brief PRF SPI interrupt handler (TXEMPTY). Disable SPI TXEMPTY interrupt and
 * enable PRF pin interrupt
 *
 */
void PRF_SPI_Handler(void)
{
	uint32_t ul_spi_busy_cnt;

	/* Disable SPI TXEMPTY interrupt in SPI peripheral */
	spi_disable_interrupt(PRF_SPI_MODULE, SPI_IDR_TXEMPTY);

	/* Enable RF interrupt pin in PIO */
	pio_get_interrupt_status(PRF_INT_PIO);
	pio_enable_interrupt(PRF_INT_PIO, PRF_INT_MASK);

	/* Wait until SPI is detected as free to avoid issues with WFE */
	ul_spi_busy_cnt = 0;
	while (prf_if_is_spi_busy()) {
		ul_spi_busy_cnt++;
		if (ul_spi_busy_cnt > 50) {
			break;
		}
	}
}

/**
 * \brief Initialize PRF interface
 *
 * \return Time of one byte SPI transaction in us with 5 comma bits [uQ3.5]
 */
uint8_t prf_if_init(void)
{
	uint8_t uc_byte_time_us_q5;

	/* Init PRF handler */
	prf_handler = NULL;

	/* Initialize SPI peripheral */
	uc_byte_time_us_q5 = _prf_if_spi_init();

	/* Initialize DMA for SPI */
	_prf_if_dma_spi_init();

	/* Configure RF reset pin */
	ioport_set_pin_level(PRF_RESET_GPIO, PRF_RESET_ACTIVE_LEVEL);
	ioport_set_pin_dir(PRF_RESET_GPIO, IOPORT_DIR_OUTPUT);

	/* Configure RF interrupt pin */
	ioport_set_pin_mode(PRF_INT_GPIO, IOPORT_MODE_PULLUP);
	ioport_set_pin_dir(PRF_INT_GPIO, IOPORT_DIR_INPUT);

#ifdef PRF_LED_TX_GPIO
	/* Configure RF LED1 pin */
	pio_set_output(PRF_LED_TX_PIO, PRF_LED_TX_MASK, PRF_LED_INACTIVE_LEVEL, 0, 0);
#endif

#ifdef PRF_LED_RX_GPIO
	/* Configure RF LED2 pin */
	pio_set_output(PRF_LED_RX_PIO, PRF_LED_RX_MASK, PRF_LED_INACTIVE_LEVEL, 0, 0);
#endif

	/* Disable RF interrupt pin IRQ in PIO */
	pio_disable_interrupt(PRF_INT_PIO, PRF_INT_MASK);

	/* Configure PRF interrupt priority (if it was not configured before) */
	if (NVIC_GetEnableIRQ(PRF_INT_IRQn) == 0) {
		NVIC_ClearPendingIRQ(PRF_INT_IRQn);
		NVIC_SetPriority(PRF_INT_IRQn, PRF_PRIO);
	} else if (PRF_PRIO < NVIC_GetPriority(PRF_INT_IRQn)) {
		/* If interrupt priority was configured before (same priority is
		 * shared for all pins in the same PIO block), set priority only
		 * if higher priority (lower value) */
		NVIC_SetPriority(PRF_INT_IRQn, PRF_PRIO);
	}

	/* Configure PRF SPI interrupt. Priority set to PRF_PRIO in order to
	 * avoid SPI interrupt in the middle of RF IRQ pin interrupt */
	NVIC_ClearPendingIRQ(PRF_SPI_IRQn);
	NVIC_SetPriority(PRF_SPI_IRQn, PRF_PRIO);
	NVIC_EnableIRQ(PRF_SPI_IRQn);

	/* Initialize critical section counter (number of times that IRQ is
	 * disabled) */
	sul_prf_irq_crit_sect_count = 0;

	return uc_byte_time_us_q5;
}

/**
 * \brief Reset RF transceiver
 */
void prf_if_reset(void)
{
	/* Activate RST pin of RF transceiver */
	ioport_set_pin_level(PRF_RESET_GPIO, PRF_RESET_ACTIVE_LEVEL);

	/* Delay to generate RST pin pulse */
	delay_us(PRF_RST_PULSE_US);

	/* Deactivate RST pin of RF transceiver */
	ioport_set_pin_level(PRF_RESET_GPIO, PRF_RESET_INACTIVE_LEVEL);

	/* delay_us(PRF_RST_WAIT_US); */
}

/**
 * \brief Set an interrupt handler for the specified interrput source.
 */
void prf_if_set_handler(void (*p_handler)(void))
{
	/* Save pointer to upper layer callback */
	prf_handler = p_handler;

	/* Configure RF interrupt pin handler */
	pio_handler_set(PRF_INT_PIO, PRF_INT_ID, PRF_INT_MASK, PRF_INT_ATTR,
			_prf_if_int_handler);

	/* Enable RF interrupt pin in PIO */
	pio_enable_interrupt(PRF_INT_PIO, PRF_INT_MASK);
}

/**
 * \brief Enable/Disable RF interrupt
 *
 * \param[in] b_enable Enable (true) or disable (false)
 */
void prf_if_enable_interrupt(bool b_enable)
{
	if (b_enable) {
		/* Leave critical section */
		if (sul_prf_irq_crit_sect_count > 0) {
			/* Decrement critical section counter */
			sul_prf_irq_crit_sect_count--;
		}

		if (sul_prf_irq_crit_sect_count == 0) {
			/* Enable IRQ in NVIC */
			NVIC_EnableIRQ(PRF_INT_IRQn);
		}
	} else {
		/* Enter critical section: Disable IRQ in NVIC */
		NVIC_DisableIRQ(PRF_INT_IRQn);

		/* Increment critical section counter */
		sul_prf_irq_crit_sect_count++;
	}
}

#if SAME70

/**
 * \brief Check if SPI is busy (read or write transaction in progress).
 * Implementation for devices with XDMAC (SAME70).
 *
 * \retval true SPI is busy
 * \retval false SPI is free
 */
bool prf_if_is_spi_busy(void)
{
	uint32_t ul_spi_status;
	uint32_t ul_xdmac_status;
# ifdef PRF_CM7_CACHE_MAINTENANCE
	uint32_t ul_basepri_prev;
# endif

	/* Get SPI and XDMAC status */
	ul_spi_status = spi_read_status(PRF_SPI_MODULE);
	ul_xdmac_status = xdmac_channel_get_status(XDMAC);

	/* Check TXEMPTY (0 during the entire SPI transaction and 1 when SPI is
	 * not being used) flag and XDMAC SPI_RX finished */
	if ((ul_spi_status & SPI_SR_TXEMPTY) && (!(ul_xdmac_status & (1 << PRF_XDMAC_CH_SPI_RX)))) {
# ifdef PRF_CM7_CACHE_MAINTENANCE

		/* Enter critical region. Disable all interrupts except highest
		 * priority (<2: 0, 1) to avoid memcpy conflict */
		ul_basepri_prev = __get_BASEPRI();
		if (ul_basepri_prev != (1 << (8 - __NVIC_PRIO_BITS))) {
			__set_BASEPRI(2 << (8 - __NVIC_PRIO_BITS));
		}

		if (sb_prf_read_pending) {
			/* Copy read data to address from upper layer */
			memcpy(spuc_prf_rx_addr, spuc_prf_rx_buffer + PRF_SPI_COMMAND_SIZE, sus_prf_rx_len);
			sb_prf_read_pending = false;
		}

		/* Leave critical region */
		__set_BASEPRI(ul_basepri_prev);
# endif
		return false;
	} else {
		return true;
	}
}

#else

/**
 * \brief Check if SPI is busy (read or write transaction in progress).
 * Implementation for devices with PDC (SAMG55, SAM4C, PIC32CX).
 *
 * \retval true SPI is busy
 * \retval false SPI is free
 */
bool prf_if_is_spi_busy(void)
{
	uint32_t ul_spi_status;

	/* Get SPI status */
	ul_spi_status = spi_read_status(PRF_SPI_MODULE);

	/* Check TXEMPTY (0 during the entire SPI transaction and 1 when SPI is
	 * not being used) and RXBUFF (read DMA transaction finished) flags */
	if ((ul_spi_status & (SPI_SR_TXEMPTY | SPI_SR_RXBUFF)) == (SPI_SR_TXEMPTY | SPI_SR_RXBUFF)) {
		return false;
	} else {
		return true;
	}
}

#endif

/**
 * \brief Launch SPI transaction to RF transceiver. DMA is used to reduce CPU
 * overhead. In write mode the function is non-blocking. In read mode it can be
 * blocking (wait for the data to be read) or non-blocking.
 *
 * \param[in/out] puc_data_buf Pointer to data to write from or read to
 * \param[in] us_addr RF215 register address
 * \param[in] us_len Data length in bytes
 * \param[in] uc_mode Write or read with/without block (see enum ESpiMode)
 *
 * \retval true Success
 * \retval false Error
 */
bool prf_if_send_spi_cmd(uint8_t *puc_data_buf, uint16_t us_addr, uint16_t us_len, uint8_t uc_mode)
{
	uint8_t *puc_tx_buf;
	uint32_t ul_spi_busy_cnt;
	uint32_t ul_final_addr;
	uint32_t ul_basepri_prev;
	uint16_t us_tx_size;
	uint16_t us_command;
#ifdef PRF_CM7_CACHE_MAINTENANCE
	uint16_t us_tx_cache_clean_size;
#endif

	/* Check length */
	if ((us_len == 0) || (us_len > PRF_TRX_MAX_MSG_LEN)) {
		return false;
	}

	/* Check address */
	ul_final_addr = us_addr + us_len;
	if (ul_final_addr > (PRF_LAST_REG_ADDR + 1)) {
		return false;
	}

	/* Build 16 bits corresponding to COMMAND */
	/* COMMAND[15:14] = MODE[1:0]; COMMAND[13:0] = ADDRESS[13:0] */
	us_command = us_addr;
	if (uc_mode == PRF_SPI_WRITE) {
		/* Write MODE */
		us_command |= PRF_SPI_CMD_MODE_WRITE;
	} else {
		/* Read MODE */
		us_command |= PRF_SPI_CMD_MODE_READ;
	}

	/* Pointer to DMA Tx buffer */
	puc_tx_buf = spuc_prf_tx_buffer;

	/* Total number of bytes */
	us_tx_size = us_len + PRF_SPI_COMMAND_SIZE;

	/* Enter critical region. Disable all interrupts except highest priority
	 * (<2: 0, 1) to avoid wrong SPI transaction if same SPI (RF or shared
	 * with PLC) is used from IRQ */
	ul_basepri_prev = __get_BASEPRI();
	if (ul_basepri_prev != (1 << (8 - __NVIC_PRIO_BITS))) {
		__set_BASEPRI(2 << (8 - __NVIC_PRIO_BITS));
	}

	/* Wait for previous transfer to finish. The previous transaction can be
	 * in progress if the previous call was non-blocking (write or read
	 * without block) or if this call comes from interrupt. */
	ul_spi_busy_cnt = 0;
	while (prf_if_is_spi_busy()) {
		ul_spi_busy_cnt++;
		if (ul_spi_busy_cnt > 500000) {
			/* Timeout error */
			__set_BASEPRI(ul_basepri_prev);
			return false;
		}
	}

	/* Set chip select. Done in every SPI transaction in case same SPI with
	 * another CS is used by another module */
	spi_set_peripheral_chip_select_value(PRF_SPI_MODULE, spi_get_pcs(PRF_SPI_CS));

	/* Write COMMAND to Tx buffer: MSB first */
	*puc_tx_buf++ = (uint8_t)(us_command >> 8);
	*puc_tx_buf++ = (uint8_t)(us_command & 0xFF);

	if (uc_mode == PRF_SPI_WRITE) {
		/* Copy data to Tx buffer */
		memcpy(puc_tx_buf, puc_data_buf, us_len);
#if !SAME70
		/* Disable PDC Rx channel. Received data unused in write mode */
		pdc_disable_transfer(spx_prf_pdc, PERIPH_PTCR_RXTDIS);
		pdc_enable_transfer(spx_prf_pdc, PERIPH_PTCR_TXTEN);
#endif
#ifdef PRF_CM7_CACHE_MAINTENANCE
		/* All Tx data cache must be cleaned */
		us_tx_cache_clean_size = us_tx_size;
#endif
	} else {
		/* Read received data from SPI in order to clear RDRF flag
		 * (if previous transaction was in write mode the received data
		 * is not read). Otherwise the DMA would read first the last
		 * received byte from previous transaction */
		spi_get(PRF_SPI_MODULE);

#ifdef PRF_CM7_CACHE_MAINTENANCE
		/* Configure DMA Rx channel with single descriptor */
		xdmac_channel_set_destination_addr(XDMAC, PRF_XDMAC_CH_SPI_RX, (uint32_t)spuc_prf_rx_buffer);
		xdmac_channel_set_microblock_control(XDMAC, PRF_XDMAC_CH_SPI_RX, us_tx_size);
		xdmac_channel_enable(XDMAC, PRF_XDMAC_CH_SPI_RX);

		/* Only COMMAND Tx data cache needs to be cleaned */
		us_tx_cache_clean_size = PRF_SPI_COMMAND_SIZE;

		/* Save pointer and length from upper layer */
		spuc_prf_rx_addr = puc_data_buf;
		sus_prf_rx_len = us_len;
		sb_prf_read_pending = true;
#else

		/* Configure DMA Rx channel to receive header (command) and data
		 * with 2 different descriptors. The read data is directly
		 * written to desired address (from upper layer) by DMA */
# if SAME70
		sx_prf_rx_descr_data.mbr_ubc = XDMAC_UBC_UBLEN(us_len) | XDMAC_UBC_NDE_FETCH_DIS;
		sx_prf_rx_descr_data.mbr_da = (uint32_t)puc_data_buf;
		__DMB();
		xdmac_channel_set_descriptor_addr(XDMAC, PRF_XDMAC_CH_SPI_RX, (uint32_t)&sx_prf_rx_descr_cmd_dummy, 0);
		xdmac_channel_set_descriptor_control(XDMAC, PRF_XDMAC_CH_SPI_RX, XDMAC_CNDC_NDE_DSCR_FETCH_EN |
				XDMAC_CNDC_NDSUP_SRC_PARAMS_UNCHANGED | XDMAC_CNDC_NDDUP_DST_PARAMS_UPDATED |
				XDMAC_CNDC_NDVIEW_NDV0);
		xdmac_channel_enable(XDMAC, PRF_XDMAC_CH_SPI_RX);
# else
		sx_prf_rx_packet_data.ul_addr = (uint32_t)puc_data_buf;
		sx_prf_rx_packet_data.ul_size = us_len;
		pdc_rx_init(spx_prf_pdc, &sx_prf_rx_packet_cmd_dummy, &sx_prf_rx_packet_data);
		pdc_enable_transfer(spx_prf_pdc, PERIPH_PTCR_RXTEN | PERIPH_PTCR_TXTEN);
# endif
#endif
	}

#ifdef PRF_CM7_CACHE_MAINTENANCE
	/* Clean DMA Tx buffer cache to avoid incoherency issues */
	SCB_CleanDCache_by_Addr((uint32_t *)spuc_prf_tx_buffer, us_tx_cache_clean_size);
#else
	/* Data memory barrier to ensure DMA Tx buffer is completely written */
	__DMB();
#endif

	/* Configure DMA Tx channel with single descriptor */
	/* This will trigger the SPI transaction */
#if SAME70
	xdmac_channel_set_source_addr(XDMAC, PRF_XDMAC_CH_SPI_TX, (uint32_t)spuc_prf_tx_buffer);
	xdmac_channel_set_microblock_control(XDMAC, PRF_XDMAC_CH_SPI_TX, us_tx_size);
	xdmac_channel_enable(XDMAC, PRF_XDMAC_CH_SPI_TX);
#else
	sx_prf_tx_packet.ul_size = us_tx_size;
	pdc_tx_init(spx_prf_pdc, &sx_prf_tx_packet, NULL);
#endif

	/* For non-blocking modes, disable RF interrupt pin in PIO to avoid IRQ
	 * when SPI is busy. Enable SPI TXEMPTY interrupt to enable RF interrupt
	 * when transaction is finished */
	if (uc_mode != PRF_SPI_READ_BLOCK) {
		pio_disable_interrupt(PRF_INT_PIO, PRF_INT_MASK);
		NVIC_ClearPendingIRQ(PRF_SPI_IRQn);
		spi_enable_interrupt(PRF_SPI_MODULE, SPI_IER_TXEMPTY);
	}

#ifdef PRF_CM7_CACHE_MAINTENANCE
	if (sb_prf_read_pending) {
		/* Invalidate DMA Rx buffer cache to avoid incoherency issues */
		SCB_InvalidateDCache_by_Addr((uint32_t *)spuc_prf_rx_buffer, sus_prf_rx_len + PRF_SPI_COMMAND_SIZE);
	}
#endif

	/* Protection: Wait until SPI is detected as busy */
	ul_spi_busy_cnt = 0;
	while (!prf_if_is_spi_busy()) {
		ul_spi_busy_cnt++;
		if (ul_spi_busy_cnt > 50) {
			/* It could have started and finished already */
			break;
		}
	}

	/* Leave critical region. The SPI transaction has started and interrupts
	 * can be enabled (this function will block at the beginning until the
	 * SPI transaction has finished) */
	__set_BASEPRI(ul_basepri_prev);

	/* If read with block mode, wait for transfer to finish. Otherwise, we
	 * take advantage of DMA and we don't wait. */
	if (uc_mode == PRF_SPI_READ_BLOCK) {
		/* Check for RXBUFF flag. It is not needed to copy the data.
		 * Data was directly written to desired address (from upper
		 * layer) by DMA */
		ul_spi_busy_cnt = 0;
		while (prf_if_is_spi_busy()) {
			ul_spi_busy_cnt++;
			if (ul_spi_busy_cnt > 500000) {
				/* Timeout error */
				return false;
			}
		}
	}

	return true;
}

/**
 * \brief Turn on/off LED
 *
 * \param uc_led_id LED identifier [1, 2]
 * \param b_led_on Turn on (true) / off (false)
 */
void prf_if_led(uint8_t uc_led_id, bool b_led_on)
{
#ifdef PRF_LED_TX_GPIO
	if (uc_led_id == 1) {
		if (b_led_on) {
			ioport_set_pin_level(PRF_LED_TX_GPIO, PRF_LED_ACTIVE_LEVEL);
		} else {
			ioport_set_pin_level(PRF_LED_TX_GPIO, PRF_LED_INACTIVE_LEVEL);
		}
	}
#endif

#ifdef PRF_LED_RX_GPIO
	if (uc_led_id == 2) {
		if (b_led_on) {
			ioport_set_pin_level(PRF_LED_RX_GPIO, PRF_LED_ACTIVE_LEVEL);
		} else {
			ioport_set_pin_level(PRF_LED_RX_GPIO, PRF_LED_INACTIVE_LEVEL);
		}
	}
#endif

#if ((!defined(PRF_LED_TX_GPIO)) && (!defined(PRF_LED_RX_GPIO)))
	/* Avoid warning */
	UNUSED(uc_led_id);
	UNUSED(b_led_on);
#endif
}

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* @endcond */
