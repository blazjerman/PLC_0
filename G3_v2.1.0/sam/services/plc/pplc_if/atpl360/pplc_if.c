/**
 * \file
 *
 * \brief Proxy PLC Controller interface layer implementation.
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

#include "string.h"
#include "pplc_if.h"
#include "conf_pplc_if.h"
#include "ioport.h"
#include "sysclk.h"
#include "spi.h"
#if SAME70
#include "conf_board.h"
#include "xdmac.h"
#else
#include "pdc.h"
#endif
#include "pio.h"
#include "pio_handler.h"
#include "delay.h"
#if defined(__RF_MAC__)
#include "conf_prf_if.h"
#endif

#if SAM4C || SAM4CM || PIC32CX
# include "adc.h"
#elif SAMG55
# include "adc2.h"
#elif SAME70
# include "afec.h"
#endif

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* @endcond */

/**
 * \weakgroup pplc_plc_group
 * @{
 */

#if defined(__RF_MAC__)
# if (PPLC_SPI_ID == PRF_SPI_ID)
#  define PPLC_SPI_SHARED_RF
# endif
#endif

#if SAME70 && defined(CONF_BOARD_ENABLE_CACHE)

/* If cortex-M7 cache enabled, there are incoherency issues when using DMA.
 * They can be solved by using TCM, MPU to define non-cacheable region, or using
 * cache maintenance operations. If TCM or non-cacheable region is enabled in
 * conf_board.h, it is assumed that all data used by DMA (pplc_if) will be
 * mapped accordingly by the linker script, and therefore cache maintenance
 * operations are not performed. */
# if (!defined(CONF_BOARD_ENABLE_TCM_AT_INIT)) && !(defined(CONF_BOARD_CONFIG_MPU_AT_INIT) && defined(MPU_HAS_NOCACHE_REGION))
#  define PPLC_CM7_CACHE_MAINTENANCE
# endif
#endif

/** SPI Header size. */
#define PPLC_SPI_HEADER_SIZE           4
/** SPI Max Msg_Data size. */
#define PPLC_SPI_MSG_DATA_SIZE         512
/** SPI Max Msg_Params size. */
#define PPLC_SPI_MSG_PARAMS_SIZE       118   /* Worst case = 118: sizeof(rx_msg_t) [G3] */

#ifdef PPLC_CM7_CACHE_MAINTENANCE

/* When using cache maintenance operations, buffers used by DMA must be aligned
 * to cache line size (32 bytes) */

/* DMA buffer size */
# define PPLC_BUFFER_SIZE              (div32_ceil(PPLC_SPI_HEADER_SIZE + PPLC_SPI_MSG_DATA_SIZE + PPLC_SPI_MSG_PARAMS_SIZE) << 5)

/* DMA Receive buffer */
COMPILER_ALIGNED(32)
static uint8_t gs_pplc_rx_buffer[PPLC_BUFFER_SIZE];
/* DMA Transmission buffer */
COMPILER_ALIGNED(32)
static uint8_t gs_pplc_tx_buffer[PPLC_BUFFER_SIZE];

#else
/** DMA buffer us_size. */
# define PPLC_BUFFER_SIZE              (PPLC_SPI_HEADER_SIZE + PPLC_SPI_MSG_DATA_SIZE + PPLC_SPI_MSG_PARAMS_SIZE)

/* DMA Receive buffer */
static uint8_t gs_pplc_rx_buffer[PPLC_BUFFER_SIZE];
/* DMA Transmission buffer */
static uint8_t gs_pplc_tx_buffer[PPLC_BUFFER_SIZE];
#endif

#if SAME70
/* XDMAC channels used for PPLC_IF SPI */
# define PPLC_XDMAC_CH_TX           0
# define PPLC_XDMAC_CH_RX           1
/* XDMAC channel status  */
# define PPLC_XDMAC_TX_STATUS       XDMAC_GS_ST0
# define PPLC_XDMAC_RX_STATUS       XDMAC_GS_ST1

/* XDMAC Peripheral IDs */
# ifndef PPLC_SPI_ID
#  define PPLC_XDMAC_SPI_TX_PERID   XDMAC_CHANNEL_HWID_SPI0_TX
#  define PPLC_XDMAC_SPI_RX_PERID   XDMAC_CHANNEL_HWID_SPI0_RX
# elif PPLC_SPI_ID == ID_SPI0
#  define PPLC_XDMAC_SPI_TX_PERID   XDMAC_CHANNEL_HWID_SPI0_TX
#  define PPLC_XDMAC_SPI_RX_PERID   XDMAC_CHANNEL_HWID_SPI0_RX
# elif PPLC_SPI_ID == ID_SPI1
#  define PPLC_XDMAC_SPI_TX_PERID   XDMAC_CHANNEL_HWID_SPI1_TX
#  define PPLC_XDMAC_SPI_RX_PERID   XDMAC_CHANNEL_HWID_SPI1_RX
# endif

/** XDMAC channel configuration. */
static xdmac_channel_config_t xdmac_tx_channel_cfg_boot;
static xdmac_channel_config_t xdmac_rx_channel_cfg_boot;
static xdmac_channel_config_t xdmac_tx_channel_cfg_cortex;
static xdmac_channel_config_t xdmac_rx_channel_cfg_cortex;
#else
/* PDC RX data packet */
pdc_packet_t g_pplc_rx_packet;
/* PDC TX data packet */
pdc_packet_t g_pplc_tx_packet;
/* Pointer to PDC register base */
Pdc *g_pplc_pdc;
#endif

/* PPLC chip select config value */
#define PPLC_PCS    spi_get_pcs(PPLC_CS)

/* Global SPI status */
static bool sb_spi_busy;

#ifdef PPLC_PVDD_MON_ADC_CHN

/** Resistor values (in ohms) of external voltage divider circuitry */
# ifndef PPLC_PVDD_MON_RUP_OHM
/* Rup = 36k by default */
#  define PPLC_PVDD_MON_RUP_OHM                36000
# endif
# ifndef PPLC_PVDD_MON_RDOWN_OHM
/* Rdown = 10k by default */
#  define PPLC_PVDD_MON_RDOWN_OHM              10000
# endif

/** Thresholds (in mV) for PVDD monitor */
# ifndef PPLC_PVDD_MON_HIGHTHRES_MV
/* High threshold: 13V by defult */
#  define PPLC_PVDD_MON_HIGHTHRES_MV           13000
# endif
# ifndef PPLC_PVDD_MON_LOWTHRES_MV
/* Low threshold: 10V by defult */
#  define PPLC_PVDD_MON_LOWTHRES_MV            10000
# endif
# ifndef PPLC_PVDD_MON_HIGHTHRES_HYST_MV
/* High threshold (hysteresis): 12.9V by defult */
#  define PPLC_PVDD_MON_HIGHTHRES_HYST_MV      12900
# endif
# ifndef PPLC_PVDD_MON_LOWTHRES_HYST_MV
/* Low threshold (hysteresis): 10.2V by defult */
#  define PPLC_PVDD_MON_LOWTHRES_HYST_MV       10200
# endif

/** Voltage reference (in mV) for ADC */
# ifndef PPLC_PVDD_MON_ADC_REF_MV
/* 3.3V by default */
#  define PPLC_PVDD_MON_ADC_REF_MV             3300
# endif

/* Voltage at ADC input: V_adc_in = PVDD * Rdown / (Rdown + Rup)
 * ADC converted value: (V_adc_in / V_adc_ref) * 2^Nbits */
# define PPLC_PVDD_MON_THRES_DEN               ((PPLC_PVDD_MON_RDOWN_OHM + PPLC_PVDD_MON_RUP_OHM) * PPLC_PVDD_MON_ADC_REF_MV)
# define PPLC_PVDD_MON_HIGHTHRES_NUM           (PPLC_PVDD_MON_HIGHTHRES_MV * PPLC_PVDD_MON_RDOWN_OHM)
# define PPLC_PVDD_MON_LOWTHRES_NUM            (PPLC_PVDD_MON_LOWTHRES_MV * PPLC_PVDD_MON_RDOWN_OHM)
# define PPLC_PVDD_MON_HIGHTHRES_HYST_NUM      (PPLC_PVDD_MON_HIGHTHRES_HYST_MV * PPLC_PVDD_MON_RDOWN_OHM)
# define PPLC_PVDD_MON_LOWTHRES_HYST_NUM       (PPLC_PVDD_MON_LOWTHRES_HYST_MV * PPLC_PVDD_MON_RDOWN_OHM)

# if SAM4C || SAM4CM
#  define PPLC_PVDD_MON_ADC_BITS               10
# elif SAMG55 || PIC32CX || SAME70
#  define PPLC_PVDD_MON_ADC_BITS               12
# endif

/** High and Low threshold ADC values (10 bits) */
# define PPLC_PVDD_MON_HIGHTHRES_ADC_VAL       ((uint16_t)div_round((uint64_t)PPLC_PVDD_MON_HIGHTHRES_NUM << PPLC_PVDD_MON_ADC_BITS, PPLC_PVDD_MON_THRES_DEN))
# define PPLC_PVDD_MON_LOWTHRES_ADC_VAL        ((uint16_t)div_round((uint64_t)PPLC_PVDD_MON_LOWTHRES_NUM << PPLC_PVDD_MON_ADC_BITS, PPLC_PVDD_MON_THRES_DEN))
# define PPLC_PVDD_MON_HIGHTHRES_HYST_ADC_VAL  ((uint16_t)div_round((uint64_t)PPLC_PVDD_MON_HIGHTHRES_HYST_NUM << PPLC_PVDD_MON_ADC_BITS, PPLC_PVDD_MON_THRES_DEN))
# define PPLC_PVDD_MON_LOWTHRES_HYST_ADC_VAL   ((uint16_t)div_round((uint64_t)PPLC_PVDD_MON_LOWTHRES_HYST_NUM << PPLC_PVDD_MON_ADC_BITS, PPLC_PVDD_MON_THRES_DEN))

/* ADC clock frequency, tracking time and startup time must be configured
 * according to datasheet recommendations.
 * For tracking time computations, source resistance of external circuitry is
 * Rsource = 1k + 10k||36k = 8.83k  */

# if SAM4C || SAM4CM
/** ADC clock frequency (in Hz)  */
#  ifndef PPLC_PVDD_MON_ADC_CLK_FREQ
/* 6 MHz by default */
#   define PPLC_PVDD_MON_ADC_CLK_FREQ          6000000
#  endif

/** ADC tracking time: 1-16 cycles of ADC clock.
 * From datasheet: tTRACKTIM (ns) >= 0.12 * Rsource (ohm) + 500 = 1.56 us */
#  ifndef PPLC_PVDD_MON_ADC_TRACK_TIME
/* 15 cycles by default (2.5 us at 6 MHz) */
#   define PPLC_PVDD_MON_ADC_TRACK_TIME        14
#  endif

/** ADC startup time in clycles of ADC clock.
 * From datasheet: tSTART >= 40 us */
#  ifndef PPLC_PVDD_MON_ADC_STARTUP_TIME
/* 512 cycles by default (85.3 us at 6 MHz) */
#   define PPLC_PVDD_MON_ADC_STARTUP_TIME      ADC_STARTUP_TIME_8
#  endif

# elif PIC32CX
/** ADC clock frequency (in Hz)  */
#  ifndef PPLC_PVDD_MON_ADC_CLK_FREQ
/* 10 MHz by default */
#   define PPLC_PVDD_MON_ADC_CLK_FREQ          10000000
#  endif

/** ADC tracking time: 1-246 cycles of ADC clock.
 * From datasheet: tTRACKTIM (ns) >= 0.024 * Rsource (ohm) + 192 = 0.404 us */
#  ifndef PPLC_PVDD_MON_ADC_TRACK_TIME
/* 7 cycles by default (0.7 us at 10 MHz; TRACKX = 0) */
#   define PPLC_PVDD_MON_ADC_TRACK_TIME        15
#  endif

/** ADC startup time in clycles of ADC clock.
 * From datasheet: tSTART >= 5 us */
#  ifndef PPLC_PVDD_MON_ADC_STARTUP_TIME
/* 80 cycles by default (8 us at 12.5 MHz) */
#   define PPLC_PVDD_MON_ADC_STARTUP_TIME      ADC_STARTUP_TIME_5
#  endif

# elif SAMG55
/** ADC clock frequency (in Hz)  */
#  ifndef PPLC_PVDD_MON_ADC_CLK_FREQ
/* 3.2 MHz by default */
#   define PPLC_PVDD_MON_ADC_CLK_FREQ          3200000
#  endif

/** ADC tracking time: 6 (0-14) or 7 (15) cycles of ADC clock.
 * From datasheet: tTRACKTIM (ns) >= 0.12 * Rsource (ohm) + 250 = 1.31 us */
#  ifndef PPLC_PVDD_MON_ADC_TRACK_TIME
/* 7 cycles by default (2.19 us at 3.2 MHz) */
#   define PPLC_PVDD_MON_ADC_TRACK_TIME        15
#  endif

/** ADC startup time in clycles of ADC clock.
 * From datasheet: tSTART >= 4 us */
#  ifndef PPLC_PVDD_MON_ADC_STARTUP_TIME
/* 24 cycles by default (7.5 us at 4 MHz) */
#   define PPLC_PVDD_MON_ADC_STARTUP_TIME      ADC_STARTUP_TIME_3
#  endif

# elif SAME70
/** AFEC clock frequency (in Hz)  */
#  ifndef PPLC_PVDD_MON_AFEC_CLK_FREQ
/* 7.5 MHz by default */
#   define PPLC_PVDD_MON_AFEC_CLK_FREQ         7500000
#  endif

/** AFEC tracking time: 15 cycles of AFEC clock (2 us at 7.5 MHz).
 * From datasheet: tTRACKTIM (ns) >= 0.077 * Rsource (ohm) + 614 = 1.29 us */

/** AFEC startup time in clycles of AFEC clock.
 * From datasheet: tSTART >= 4 us */
#  ifndef PPLC_PVDD_MON_AFEC_STARTUP_TIME
/* 64 cycles by default (8.53 us at 7.5 MHz) */
#   define PPLC_PVDD_MON_AFEC_STARTUP_TIME     AFEC_STARTUP_TIME_4
#  endif

/** AFEC bias current control, depending on sampling rate (fs). From datasheet:
 * IBCTL=01 for fs < 500 kHz
 * IBCTL=10 for fs < 1 MHz
 * IBCTL=11 for fs > 1 MHz */
#  ifndef PPLC_PVDD_MON_AFEC_IBCTL
/* fs = fAFEC / 23 = 326 kHz, with fAFEC = 7.5 MHz */
#   define PPLC_PVDD_MON_AFEC_IBCTL            1
#  endif
# endif

/* PVDD monitor interrupt handler */
static void (*pplc_pvdd_mon_handler)(bool);

/* PVDD good/bad flag */
static bool sb_pvdd_good;

/**
 * \brief Update comparison window mode and thresholds for PVDD monitor
 *
 * \param b_pvdd_good Current status of PVDD: Good (true) or bad (false)
 */
static void _pvdd_mon_set_comp_mode(bool b_pvdd_good)
{
	if (b_pvdd_good) {
		/* PVDD within allowed range: Set compare mode to OUT */
# if SAM4C || SAM4CM || PIC32CX
		adc_set_comparison_window(ADC, PPLC_PVDD_MON_LOWTHRES_ADC_VAL, PPLC_PVDD_MON_HIGHTHRES_ADC_VAL);
		adc_set_comparison_filter(ADC, 0);
		adc_set_comparison_mode(ADC, ADC_EMR_CMPMODE_OUT);
# elif SAMG55
		adc_set_comparison_window(ADC, PPLC_PVDD_MON_LOWTHRES_ADC_VAL, PPLC_PVDD_MON_HIGHTHRES_ADC_VAL);
		adc_set_comparison_mode(ADC, ADC_CMP_MODE_3, PPLC_PVDD_MON_ADC_CHN, 0);
# elif SAME70
		afec_set_comparison_window(PPLC_PVDD_MON_AFEC_MODULE, PPLC_PVDD_MON_LOWTHRES_ADC_VAL, PPLC_PVDD_MON_HIGHTHRES_ADC_VAL);
		afec_set_comparison_mode(PPLC_PVDD_MON_AFEC_MODULE, AFEC_CMP_MODE_3, PPLC_PVDD_MON_ADC_CHN, 0);
# endif
	} else {
		/* PVDD out of allowed range: Set compare mode to IN */
# if SAM4C || SAM4CM || PIC32CX
		adc_set_comparison_window(ADC, PPLC_PVDD_MON_LOWTHRES_HYST_ADC_VAL, PPLC_PVDD_MON_HIGHTHRES_HYST_ADC_VAL);
		adc_set_comparison_filter(ADC, 3);
		adc_set_comparison_mode(ADC, ADC_EMR_CMPMODE_IN);
# elif SAMG55
		adc_set_comparison_window(ADC, PPLC_PVDD_MON_LOWTHRES_HYST_ADC_VAL, PPLC_PVDD_MON_HIGHTHRES_HYST_ADC_VAL);
		adc_set_comparison_mode(ADC, ADC_CMP_MODE_2, PPLC_PVDD_MON_ADC_CHN, 3);
# elif SAME70
		afec_set_comparison_window(PPLC_PVDD_MON_AFEC_MODULE, PPLC_PVDD_MON_LOWTHRES_HYST_ADC_VAL, PPLC_PVDD_MON_HIGHTHRES_HYST_ADC_VAL);
		afec_set_comparison_mode(PPLC_PVDD_MON_AFEC_MODULE, AFEC_CMP_MODE_2, PPLC_PVDD_MON_ADC_CHN, 3);
# endif
	}
}

/**
 * \brief Handler of ADC interrupt. Check PVDD monitor ADC value. Use TX Enable
 * pin to enable/disable PLC TX depending on PVDD status. If PVDD status
 * changes, update comparison window mode and thresholds, and call upper layer
 * callback
 */
static void _pvdd_mon_adc_handler(void)
{
	uint16_t us_adc_value;
	bool b_pvdd_good;

	/* Read PVDD monitor ADC value */
# if SAM4C || SAM4CM || PIC32CX
	us_adc_value = (uint16_t)adc_get_channel_value(ADC, PPLC_PVDD_MON_ADC_CHN);
# elif SAMG55
	us_adc_value = (uint16_t)adc_channel_get_value(ADC, PPLC_PVDD_MON_ADC_CHN);
# elif SAME70
	us_adc_value = (uint16_t)afec_channel_get_value(PPLC_PVDD_MON_AFEC_MODULE, PPLC_PVDD_MON_ADC_CHN);
# endif

	/* Compare read value with thresholds */
	b_pvdd_good = sb_pvdd_good;
	if (sb_pvdd_good) {
		/* Thresholds from good to bad PVDD */
		if ((us_adc_value < PPLC_PVDD_MON_LOWTHRES_ADC_VAL) || (us_adc_value > PPLC_PVDD_MON_HIGHTHRES_ADC_VAL)) {
			b_pvdd_good = false;
		}
	} else {
		/* Thresholds from bad to good PVDD */
		if ((us_adc_value > PPLC_PVDD_MON_LOWTHRES_HYST_ADC_VAL) && (us_adc_value < PPLC_PVDD_MON_HIGHTHRES_HYST_ADC_VAL)) {
			b_pvdd_good = true;
		}
	}

# ifdef PPLC_TX_ENABLE_GPIO
	if (!b_pvdd_good) {
		/* Disable PLC TX with TX Enable pin */
		/* If there is ongoing TX it will be aborted */
		ioport_set_pin_level(PPLC_TX_ENABLE_GPIO, PPLC_TX_ENABLE_INACTIVE_LEVEL);
	} else {
		/* Enable PLC TX with TX Enable pin */
		ioport_set_pin_level(PPLC_TX_ENABLE_GPIO, PPLC_TX_ENABLE_ACTIVE_LEVEL);
	}
# endif

	if (b_pvdd_good != sb_pvdd_good) {
		/* PVDD status changed: call upper layer callback */
		if (pplc_pvdd_mon_handler != NULL) {
			pplc_pvdd_mon_handler(b_pvdd_good);
		}

		/* Update comparison window mode and thresholds */
		_pvdd_mon_set_comp_mode(b_pvdd_good);

		/* Update flag */
		sb_pvdd_good = b_pvdd_good;
	}
}

# if SAM4C || SAM4CM || PIC32CX

/**
 * \brief Interrupt handler for ADC.
 */
void ADC_Handler(void)
{
	if (adc_get_status(ADC) & ADC_ISR_COMPE) {
		_pvdd_mon_adc_handler();
	}
}

# endif

/**
 * \brief Initialize PVDD monitor. Configure ADC using Comparison Window.
 */
static void _pvdd_mon_init(void)
{
	/* Initialize PVDD status: Handler only called if PVDD is bad */
	sb_pvdd_good = true;

# if SAM4C || SAM4CM || PIC32CX
	/* Enable ADC peripheral */
	pmc_enable_periph_clk(ID_ADC);

	/* ADC configuration */
	adc_init(ADC, sysclk_get_peripheral_hz(), PPLC_PVDD_MON_ADC_CLK_FREQ, PPLC_PVDD_MON_ADC_STARTUP_TIME);
#  if PIC32CX
	adc_configure_timing(ADC, PPLC_PVDD_MON_ADC_TRACK_TIME, 2);
	adc_set_resolution(ADC, ADC_12_BITS);
#  else
	struct adc_internal_ref adc_int_ref;

	adc_configure_timing(ADC, PPLC_PVDD_MON_ADC_TRACK_TIME);
	adc_set_resolution(ADC, ADC_10_BITS);

	adc_int_ref.adc_internal_ref_change_enable = false;
	adc_int_ref.volt = ADC_INTERNAL_REF_3275MV;
	adc_int_ref.adc_force_internal_ref = true;
	adc_int_ref.adc_internal_ref_on = false;
	adc_set_internal_reference_voltage(ADC, &adc_int_ref);
#  endif

	/* Enable channel used for PVDD monitor */
	adc_enable_channel(ADC, PPLC_PVDD_MON_ADC_CHN);

	/* Set comparison window mode and thresholds */
	_pvdd_mon_set_comp_mode(true);
	adc_set_comparison_channel(ADC, PPLC_PVDD_MON_ADC_CHN);

	/* Configure ADC in Free-Running mode */
#  if PIC32CX
	adc_set_trigger(ADC, ADC_TRGR_TRGMOD_CONTINUOUS, 0);
#  else
	adc_configure_trigger(ADC, ADC_TRIG_SW, 1);
#  endif

	/* Enable ADC interrupt for PVDD monitor */
	NVIC_SetPriority(ADC_IRQn, PPLC_ADC_PRIO);
	NVIC_ClearPendingIRQ(ADC_IRQn);
	NVIC_EnableIRQ(ADC_IRQn);
	adc_enable_interrupt(ADC, ADC_ISR_COMPE);

# elif SAMG55
	struct adc_config adc_cfg;

	/* Enable ADC peripheral */
	adc_enable();

	/* ADC configuration */
	adc_cfg.resolution = ADC_12_BITS;
	adc_cfg.mck = sysclk_get_peripheral_hz();
	adc_cfg.adc_clock = PPLC_PVDD_MON_ADC_CLK_FREQ;
	adc_cfg.startup_time = PPLC_PVDD_MON_ADC_STARTUP_TIME;
	adc_cfg.tracktim = PPLC_PVDD_MON_ADC_TRACK_TIME;
	adc_cfg.transfer = 2;
	adc_cfg.useq = false;
	adc_cfg.tag = false;
	adc_cfg.aste = false;
	adc_init(ADC, &adc_cfg);

	/* Enable channel used for PVDD monitor */
	adc_channel_enable(ADC, PPLC_PVDD_MON_ADC_CHN);

	/* Set comparison window mode and thresholds */
	_pvdd_mon_set_comp_mode(true);

	/* Configure ADC in Free-Running mode */
	adc_set_trigger(ADC, ADC_TRIG_FREERUN);

	/* Set ADC callback for PVDD monitor */
	adc_set_callback(ADC, ADC_INTERRUPT_COMP_ERROR, _pvdd_mon_adc_handler, PPLC_ADC_PRIO);

# elif SAME70
	struct afec_config afec_cfg;
	struct afec_ch_config afec_ch_cfg;

	/* Enable AFEC peripheral */
	afec_enable(PPLC_PVDD_MON_AFEC_MODULE);

	/* AFEC configuration */
	afec_cfg.resolution = AFEC_12_BITS;
	afec_cfg.mck = sysclk_get_peripheral_hz();
	afec_cfg.afec_clock = PPLC_PVDD_MON_AFEC_CLK_FREQ;
	afec_cfg.startup_time = PPLC_PVDD_MON_AFEC_STARTUP_TIME;
	afec_cfg.tracktim = 0;
	afec_cfg.transfer = 2;
	afec_cfg.useq = false;
	afec_cfg.tag = false;
	afec_cfg.stm = false;
	afec_cfg.ibctl = PPLC_PVDD_MON_AFEC_IBCTL;
	afec_init(PPLC_PVDD_MON_AFEC_MODULE, &afec_cfg);

	/* Enable and configure channel used for PVDD monitor */
	afec_channel_enable(PPLC_PVDD_MON_AFEC_MODULE, PPLC_PVDD_MON_ADC_CHN);
	afec_ch_cfg.diff = false;
	afec_ch_cfg.gain = AFEC_GAINVALUE_0;
	afec_ch_set_config(PPLC_PVDD_MON_AFEC_MODULE, PPLC_PVDD_MON_ADC_CHN, &afec_ch_cfg);
	afec_channel_set_analog_offset(PPLC_PVDD_MON_AFEC_MODULE, PPLC_PVDD_MON_ADC_CHN, 512);

	/* Set comparison window mode and thresholds */
	_pvdd_mon_set_comp_mode(true);

	/* Configure AFEC in Free-Running mode */
	afec_set_trigger(PPLC_PVDD_MON_AFEC_MODULE, AFEC_TRIG_FREERUN);

	/* Set AFEC callback for PVDD monitor */
	afec_set_callback(PPLC_PVDD_MON_AFEC_MODULE, AFEC_INTERRUPT_COMP_ERROR, _pvdd_mon_adc_handler, PPLC_ADC_PRIO);
# endif
}

#endif /* PPLC_PVDD_MON_ADC_CHN */

/**
 * Describes an PPLC interrupt handler
 */
static void (*pplc_handler)(void);

static void pplc_if_int_handler(uint32_t ul_id, uint32_t ul_mask)
{
	UNUSED(ul_id);
	UNUSED(ul_mask);
	if (pplc_handler != NULL) {
		pplc_handler();
		/* Clear INT info */
		pio_get_interrupt_status(PPLC_INT_PIO);
	}
}

#if SAME70

/**
 * \brief Disable XDMAC for spi and forbidden transmit and receive by XDMAC.
 *
 */
static void _xdmac_disable(void)
{
	uint32_t xdmaint;

	xdmaint = (XDMAC_CIE_BIE |
			XDMAC_CIE_DIE   |
			XDMAC_CIE_FIE   |
			XDMAC_CIE_RBIE  |
			XDMAC_CIE_WBIE  |
			XDMAC_CIE_ROIE);

	xdmac_channel_disable_interrupt(XDMAC, PPLC_XDMAC_CH_RX, xdmaint);
	xdmac_channel_disable(XDMAC, PPLC_XDMAC_CH_RX);
	xdmac_disable_interrupt(XDMAC, PPLC_XDMAC_CH_RX);

	xdmac_channel_disable_interrupt(XDMAC, PPLC_XDMAC_CH_TX, xdmaint);
	xdmac_channel_disable(XDMAC, PPLC_XDMAC_CH_TX);
	xdmac_disable_interrupt(XDMAC, PPLC_XDMAC_CH_TX);

	NVIC_ClearPendingIRQ(XDMAC_IRQn);
	NVIC_DisableIRQ(XDMAC_IRQn);
}

#endif

/**
 * \internal
 * \brief Initialize Proxy PLC controller.
 *
 * This function will change the system clock prescaler configuration to
 * match the parameters.
 *
 * \note The parameters to this function are device-specific.
 *
 */
void pplc_if_config(void)
{
	uint32_t ul_cpuhz;
	uint32_t ul_pplc_clock;
	uint8_t uc_div;

	ul_cpuhz = sysclk_get_peripheral_hz();
	ul_pplc_clock = PPLC_CLOCK;
	uc_div = ul_cpuhz / ul_pplc_clock;

	if (ul_cpuhz % ul_pplc_clock) {
		uc_div++;
	}

	/* Enable SPI peripheral. */
	spi_enable_clock(PPLC_SPI_MODULE);

#ifndef PPLC_SPI_SHARED_RF
	/* Reset SPI */
	spi_disable(PPLC_SPI_MODULE);
	spi_reset(PPLC_SPI_MODULE);

	/* Configure SPI */
	spi_set_master_mode(PPLC_SPI_MODULE);
	spi_disable_mode_fault_detect(PPLC_SPI_MODULE);
	spi_set_peripheral_chip_select_value(PPLC_SPI_MODULE, PPLC_PCS);
	spi_set_clock_polarity(PPLC_SPI_MODULE, PPLC_CS, 0);
	spi_set_clock_phase(PPLC_SPI_MODULE, PPLC_CS, 1);
	spi_set_bits_per_transfer(PPLC_SPI_MODULE, PPLC_CS, SPI_CSR_BITS_16_BIT);
	spi_set_fixed_peripheral_select(PPLC_SPI_MODULE);
	spi_set_baudrate_div(PPLC_SPI_MODULE, PPLC_CS, uc_div);
	spi_set_transfer_delay(PPLC_SPI_MODULE, PPLC_CS, PPLC_DLYBS, PPLC_DLYBCT);
	spi_configure_cs_behavior(PPLC_SPI_MODULE, PPLC_CS, SPI_CS_RISE_NO_TX);
	spi_enable(PPLC_SPI_MODULE);
#else
	if (spi_is_enabled(PPLC_SPI_MODULE) == 0) {
		/* Reset and configure SPI. Common registers for different Chip
		 * Selects are only written if SPI is not enabled yet */
		spi_disable(PPLC_SPI_MODULE);
		spi_reset(PPLC_SPI_MODULE);
		spi_set_master_mode(PPLC_SPI_MODULE);
		spi_disable_mode_fault_detect(PPLC_SPI_MODULE);
		spi_set_fixed_peripheral_select(PPLC_SPI_MODULE);
		spi_enable(PPLC_SPI_MODULE);
	}

	/* Configure SPI */
	spi_set_clock_polarity(PPLC_SPI_MODULE, PPLC_CS, 0);
	spi_set_clock_phase(PPLC_SPI_MODULE, PPLC_CS, 1);
	spi_set_baudrate_div(PPLC_SPI_MODULE, PPLC_CS, uc_div);
	spi_set_transfer_delay(PPLC_SPI_MODULE, PPLC_CS, PPLC_DLYBS, PPLC_DLYBCT);
	spi_configure_cs_behavior(PPLC_SPI_MODULE, PPLC_CS, SPI_CS_RISE_NO_TX);
#endif

#if SAME70
	/* Initialize and enable DMA controller */
	pmc_enable_periph_clk(ID_XDMAC);

	/* Turn off XDMAC initially */
	_xdmac_disable();

	/* Configure TX and RX XDMAC channels */
	xdmac_tx_channel_cfg_boot.mbr_sa = (uint32_t)gs_pplc_tx_buffer;
	xdmac_tx_channel_cfg_boot.mbr_da = (uint32_t)spi_get_tx_access(PPLC_SPI_MODULE);
	xdmac_tx_channel_cfg_boot.mbr_cfg = XDMAC_CC_TYPE_PER_TRAN |
			XDMAC_CC_DSYNC_MEM2PER |
			XDMAC_CC_PERID(PPLC_XDMAC_SPI_TX_PERID) |
			XDMAC_CC_CSIZE_CHK_1 |
			XDMAC_CC_MEMSET_NORMAL_MODE |
			XDMAC_CC_MBSIZE_SINGLE |
			XDMAC_CC_DWIDTH_BYTE |
			XDMAC_CC_SIF_AHB_IF0 |
			XDMAC_CC_DIF_AHB_IF1 |
			XDMAC_CC_SAM_INCREMENTED_AM |
			XDMAC_CC_DAM_FIXED_AM;
	xdmac_tx_channel_cfg_boot.mbr_bc = 0;
	xdmac_tx_channel_cfg_boot.mbr_ds = 0;
	xdmac_tx_channel_cfg_boot.mbr_sus = 0;
	xdmac_tx_channel_cfg_boot.mbr_dus = 0;

	xdmac_rx_channel_cfg_boot.mbr_sa = (uint32_t)spi_get_rx_access(PPLC_SPI_MODULE);
	xdmac_rx_channel_cfg_boot.mbr_da = (uint32_t)gs_pplc_rx_buffer;
	xdmac_rx_channel_cfg_boot.mbr_cfg = XDMAC_CC_TYPE_PER_TRAN |
			XDMAC_CC_DSYNC_PER2MEM |
			XDMAC_CC_PERID(PPLC_XDMAC_SPI_RX_PERID) |
			XDMAC_CC_CSIZE_CHK_1 |
			XDMAC_CC_MEMSET_NORMAL_MODE |
			XDMAC_CC_MBSIZE_SINGLE |
			XDMAC_CC_DWIDTH_BYTE |
			XDMAC_CC_SIF_AHB_IF1 |
			XDMAC_CC_DIF_AHB_IF0 |
			XDMAC_CC_SAM_FIXED_AM |
			XDMAC_CC_DAM_INCREMENTED_AM;
	xdmac_rx_channel_cfg_boot.mbr_bc = 0;
	xdmac_rx_channel_cfg_boot.mbr_ds = 0;
	xdmac_rx_channel_cfg_boot.mbr_sus = 0;
	xdmac_rx_channel_cfg_boot.mbr_dus = 0;

	xdmac_tx_channel_cfg_cortex.mbr_sa = (uint32_t)gs_pplc_tx_buffer;
	xdmac_tx_channel_cfg_cortex.mbr_da = (uint32_t)spi_get_tx_access(PPLC_SPI_MODULE);
	xdmac_tx_channel_cfg_cortex.mbr_cfg = XDMAC_CC_TYPE_PER_TRAN |
			XDMAC_CC_DSYNC_MEM2PER |
			XDMAC_CC_PERID(PPLC_XDMAC_SPI_TX_PERID) |
			XDMAC_CC_CSIZE_CHK_1 |
			XDMAC_CC_MEMSET_NORMAL_MODE |
			XDMAC_CC_MBSIZE_SINGLE |
			XDMAC_CC_DWIDTH_HALFWORD |
			XDMAC_CC_SIF_AHB_IF0 |
			XDMAC_CC_DIF_AHB_IF1 |
			XDMAC_CC_SAM_INCREMENTED_AM |
			XDMAC_CC_DAM_FIXED_AM;
	xdmac_tx_channel_cfg_cortex.mbr_bc = 0;
	xdmac_tx_channel_cfg_cortex.mbr_ds = 0;
	xdmac_tx_channel_cfg_cortex.mbr_sus = 0;
	xdmac_tx_channel_cfg_cortex.mbr_dus = 0;

	xdmac_rx_channel_cfg_cortex.mbr_sa = (uint32_t)spi_get_rx_access(PPLC_SPI_MODULE);
	xdmac_rx_channel_cfg_cortex.mbr_da = (uint32_t)gs_pplc_rx_buffer;
	xdmac_rx_channel_cfg_cortex.mbr_cfg = XDMAC_CC_TYPE_PER_TRAN |
			XDMAC_CC_DSYNC_PER2MEM |
			XDMAC_CC_PERID(PPLC_XDMAC_SPI_RX_PERID) |
			XDMAC_CC_CSIZE_CHK_1 |
			XDMAC_CC_MEMSET_NORMAL_MODE |
			XDMAC_CC_MBSIZE_SINGLE |
			XDMAC_CC_DWIDTH_HALFWORD |
			XDMAC_CC_SIF_AHB_IF1 |
			XDMAC_CC_DIF_AHB_IF0 |
			XDMAC_CC_SAM_FIXED_AM |
			XDMAC_CC_DAM_INCREMENTED_AM;
	xdmac_rx_channel_cfg_cortex.mbr_bc = 0;
	xdmac_rx_channel_cfg_cortex.mbr_ds = 0;
	xdmac_rx_channel_cfg_cortex.mbr_sus = 0;
	xdmac_rx_channel_cfg_cortex.mbr_dus = 0;
#else
	/* Get board PPLC PDC base address and enable receiver and transmitter */
	g_pplc_pdc = spi_get_pdc_base(PPLC_SPI_MODULE);
#endif
}

/**
 * \brief Initialize PPLC interface
 *
 */
void pplc_if_init(void)
{
	/* Init PPLC handler */
	pplc_handler = NULL;
	sb_spi_busy = false;

	/* Initialize PPLC */
	pplc_if_config();

#ifdef PPLC_STBY_GPIO
	/* Configure STBY pin */
	ioport_set_pin_level(PPLC_STBY_GPIO, PPLC_STBY_INACTIVE_LEVEL);
	ioport_set_pin_dir(PPLC_STBY_GPIO, IOPORT_DIR_OUTPUT);
#endif

	/* Configure LDO_EN pin */
	ioport_set_pin_level(PPLC_LDO_EN_GPIO, PPLC_LDO_EN_ACTIVE_LEVEL);
	ioport_set_pin_dir(PPLC_LDO_EN_GPIO, IOPORT_DIR_OUTPUT);

	/* Configure PLC reset pin */
	ioport_set_pin_level(PPLC_RESET_GPIO, PPLC_RESET_ACTIVE_LEVEL);
	ioport_set_pin_dir(PPLC_RESET_GPIO, IOPORT_DIR_OUTPUT);

#ifdef PPLC_CD_GPIO
	/* Configure CD pin */
	ioport_set_pin_dir(PPLC_CD_GPIO, IOPORT_DIR_INPUT);
#endif
#ifdef PPLC_NTHW0_GPIO
	/* Configure NTHW0 pin (only PL460/PL480) */
	ioport_set_pin_dir(PPLC_NTHW0_GPIO, IOPORT_DIR_INPUT);
	ioport_set_pin_mode(PPLC_NTHW0_GPIO, IOPORT_MODE_PULLUP);
#endif

#ifdef PPLC_TX_ENABLE_GPIO
# ifdef PPLC_CD_GPIO
#  error Carrier Detect (PL360) and TX Enable (PL460) cannot be used at the same time
# endif

	/* Configure TX Enable pin (only PL460/480) */
	ioport_set_pin_level(PPLC_TX_ENABLE_GPIO, PPLC_TX_ENABLE_ACTIVE_LEVEL);
	ioport_set_pin_dir(PPLC_TX_ENABLE_GPIO, IOPORT_DIR_OUTPUT);
#endif

#ifdef PPLC_PVDD_MON_ADC_CHN
	/* Initialize PVDD monitor */
	_pvdd_mon_init();
#endif
}

/**
 * \brief Reset PPLC interface
 *
 */
void pplc_if_reset(void)
{
	/* Enable LDO line */
	ioport_set_pin_level(PPLC_LDO_EN_GPIO, PPLC_LDO_EN_ACTIVE_LEVEL);
	ioport_set_pin_dir(PPLC_LDO_EN_GPIO, IOPORT_DIR_OUTPUT);
	delay_ms(1);

	/* Reset on RST of modem PLC */
	ioport_set_pin_level(PPLC_RESET_GPIO, PPLC_RESET_ACTIVE_LEVEL);
	delay_ms(1);
	/* Clear RST of modem PLC */
	ioport_set_pin_level(PPLC_RESET_GPIO, PPLC_RESET_INACTIVE_LEVEL);

	delay_ms(50);
}

/**
 * \brief Set an interrupt handler for the specified interrpupt source.
 */
void pplc_if_set_handler(void (*p_handler)(void))
{
	pplc_handler = p_handler;

	/* Configure PPLC interruption pin */
	ioport_set_pin_mode(PPLC_INT_GPIO, IOPORT_MODE_PULLDOWN);
	ioport_set_pin_dir(PPLC_INT_GPIO, IOPORT_DIR_INPUT);

	/* Configure PPLC Interruption */
	pio_handler_set(PPLC_INT_PIO, PPLC_INT_ID, PPLC_INT_MASK, PPLC_INT_ATTR, pplc_if_int_handler);

	NVIC_SetPriority(PPLC_INT_IRQn, PPLC_PRIO);
	NVIC_ClearPendingIRQ(PPLC_INT_IRQn);
	NVIC_EnableIRQ(PPLC_INT_IRQn);
}

/**
 * \brief Set callback for PVDD monitor. Callback handler will be used to notify
 * PVDD events (true: PVDD within allowed range; false: PVDD out of allowed
 * range). Handler is only called when PVDD status changes (from good to bad or
 * from bad to good). It will be called inside this function to indicate the
 * current PVDD status.
 *
 * \param p_handler Handler to notify PVDD event
 */
void pplc_if_pvdd_mon_set_handler(void (*p_handler)(bool))
{
#ifdef PPLC_PVDD_MON_ADC_CHN
	pplc_pvdd_mon_handler = p_handler;
	if (pplc_pvdd_mon_handler != NULL) {
		pplc_pvdd_mon_handler(sb_pvdd_good);
	}

#else
	UNUSED(p_handler);
#endif
}

#if SAME70

bool pplc_if_send_boot_cmd(uint16_t us_cmd, uint32_t ul_addr, uint32_t ul_data_len, uint8_t *puc_data_buf, uint8_t *puc_data_read)
{
	uint8_t *puc_tx_buf;
	uint32_t ul_spi_busy_cnt;
	uint16_t us_tx_size;

	/* Check length */
	if ((ul_data_len + 6) > PPLC_BUFFER_SIZE) {
		return false;
	}

	/* Disable PLC interrupt to avoid SPI access in the middle of a transaction */
	NVIC_DisableIRQ(PPLC_INT_IRQn);

	/* Check SPI status */
	if (sb_spi_busy) {
		return false;
	}

	/* Update SPI status */
	sb_spi_busy = true;

#ifdef PPLC_SPI_SHARED_RF

	/* Enter critical region. Disable all interrupts except highest priority
	 * (<2: 0, 1) to avoid wrong SPI transaction if same SPI (shared with
	 * RF) is used from IRQ */
	uint32_t ul_basepri_prev = __get_BASEPRI();
	if (ul_basepri_prev != (1 << (8 - __NVIC_PRIO_BITS))) {
		__set_BASEPRI(2 << (8 - __NVIC_PRIO_BITS));
	}
#endif

	/* Waiting transfer done while(pdc_read_tx_counter(g_pdc) > 0); */
	ul_spi_busy_cnt = 0;
	while ((xdmac_channel_get_status(XDMAC) & (PPLC_XDMAC_TX_STATUS | PPLC_XDMAC_RX_STATUS)) ||
			!(spi_read_status(PPLC_SPI_MODULE) & SPI_SR_TXEMPTY)) {
		ul_spi_busy_cnt++;
		if (ul_spi_busy_cnt > 5000000) {
			/* Update SPI status */
			sb_spi_busy = false;
			/* Enable PLC interrupt */
			NVIC_EnableIRQ(PPLC_INT_IRQn);
#ifdef PPLC_SPI_SHARED_RF
			__set_BASEPRI(ul_basepri_prev);
#endif
			return false;
		}
	}

	/* Disable the RX and TX XDMAC transfer requests */
	xdmac_channel_disable(XDMAC, PPLC_XDMAC_CH_RX);
	xdmac_channel_disable(XDMAC, PPLC_XDMAC_CH_TX);

	/* Set 8 bits per transfer */
	spi_set_bits_per_transfer(PPLC_SPI_MODULE, PPLC_CS, SPI_CSR_BITS_8_BIT);

#ifdef PPLC_SPI_SHARED_RF

	/* Set chip select. Done in every SPI transaction in case same SPI with
	 * another CS is used by another module */
	spi_set_peripheral_chip_select_value(PPLC_SPI_MODULE, PPLC_PCS);

	/* Read received data from SPI in order to clear RDRF flag (if previous
	 * transaction with RF215 was in write mode the received data is not
	 * read). Otherwise the DMA would read first the last received byte from
	 * previous transaction */
	spi_get(PPLC_SPI_MODULE);
#endif

	/* Configure Tx buffer */
	puc_tx_buf = gs_pplc_tx_buffer;

	memcpy(puc_tx_buf, &ul_addr, sizeof(uint32_t));
	puc_tx_buf +=  sizeof(uint32_t);
	memcpy(puc_tx_buf, &us_cmd, sizeof(uint16_t));
	puc_tx_buf +=  sizeof(uint16_t);

	memcpy(puc_tx_buf, puc_data_buf, ul_data_len);

	puc_tx_buf += ul_data_len;

	us_tx_size = puc_tx_buf - gs_pplc_tx_buffer;

#ifdef PPLC_CM7_CACHE_MAINTENANCE
	/* Clean DMA Tx buffer cache to avoid incoherency issues */
	SCB_CleanDCache_by_Addr((uint32_t *)gs_pplc_tx_buffer, us_tx_size);
#endif

	/* Configure TX and RX XDMAC channels */
	xdmac_rx_channel_cfg_boot.mbr_ubc = us_tx_size;
	xdmac_tx_channel_cfg_boot.mbr_ubc = us_tx_size;

	xdmac_configure_transfer(XDMAC, PPLC_XDMAC_CH_RX, &xdmac_rx_channel_cfg_boot);
	xdmac_channel_set_descriptor_control(XDMAC, PPLC_XDMAC_CH_RX, 0);

	xdmac_configure_transfer(XDMAC, PPLC_XDMAC_CH_TX, &xdmac_tx_channel_cfg_boot);
	xdmac_channel_set_descriptor_control(XDMAC, PPLC_XDMAC_CH_TX, 0);

	/* Enable the RX and TX XDMAC transfer requests */
	xdmac_channel_enable(XDMAC, PPLC_XDMAC_CH_RX);
	xdmac_channel_enable(XDMAC, PPLC_XDMAC_CH_TX);

#ifdef PPLC_SPI_SHARED_RF
	/* Protection: Wait until SPI is detected as busy */
	ul_spi_busy_cnt = 0;
	while (spi_read_status(PPLC_SPI_MODULE) & SPI_SR_TXEMPTY) {
		ul_spi_busy_cnt++;
		if (ul_spi_busy_cnt > 50) {
			/* It could have started and finished already */
			break;
		}
	}

	/* Leave critical region. The SPI transaction has started and interrupts
	 * can be enabled (RF SPI access function will block until this SPI
	 * transaction has finished) */
	__set_BASEPRI(ul_basepri_prev);
#endif

	/* Waiting transfer done and read */
	if (puc_data_read) {
#ifdef PPLC_CM7_CACHE_MAINTENANCE
		/* Invalidate DMA Rx buffer cache to avoid incoherency issues */
		SCB_InvalidateDCache_by_Addr((uint32_t *)gs_pplc_rx_buffer, us_tx_size);
#endif

		/* while(pdc_read_tx_counter(g_pdc) > 0); */
		ul_spi_busy_cnt = 0;
		while (xdmac_channel_get_status(XDMAC) & PPLC_XDMAC_RX_STATUS) {
			ul_spi_busy_cnt++;
			if (ul_spi_busy_cnt > 5000000) {
				/* Update SPI status */
				sb_spi_busy = false;
				/* Enable PLC interrupt */
				NVIC_EnableIRQ(PPLC_INT_IRQn);
				return false;
			}
		}

		memcpy(puc_data_read, &gs_pplc_rx_buffer[6], ul_data_len);
	}

	/* Update SPI status */
	sb_spi_busy = false;

	/* Enable PLC interrupt */
	NVIC_EnableIRQ(PPLC_INT_IRQn);

	return true;
}

bool pplc_if_send_wrrd_cmd(uint8_t uc_cmd, void *px_spi_data, void *px_spi_status_info)
{
	uint8_t *puc_tx_buf;
	uint32_t ul_spi_busy_cnt;
	uint16_t us_tx_size;
	uint16_t us_len_wr_rd;
	spi_data_t *px_data;
	spi_status_info_t *px_status_info;

	px_data = (spi_data_t *)px_spi_data;
	px_status_info = (spi_status_info_t *)px_spi_status_info;

	/* Check length */
	if ((px_data->us_len == 0) || ((px_data->us_len + PPLC_SPI_HEADER_SIZE) > PPLC_BUFFER_SIZE)) {
		return false;
	}

	/* Disable PLC interrupt to avoid SPI access in the middle of a transaction */
	NVIC_DisableIRQ(PPLC_INT_IRQn);

	/* Check SPI status */
	if (sb_spi_busy) {
		return false;
	}

	/* Update SPI status */
	sb_spi_busy = true;

#ifdef PPLC_SPI_SHARED_RF

	/* Enter critical region. Disable all interrupts except highest priority
	 * (<2: 0, 1) to avoid wrong SPI transaction if same SPI (shared with
	 * RF) is used from IRQ */
	uint32_t ul_basepri_prev = __get_BASEPRI();
	if (ul_basepri_prev != (1 << (8 - __NVIC_PRIO_BITS))) {
		__set_BASEPRI(2 << (8 - __NVIC_PRIO_BITS));
	}
#endif

	/* Waiting transfer done while(pdc_read_tx_counter(g_pdc) > 0); */
	ul_spi_busy_cnt = 0;
	while ((xdmac_channel_get_status(XDMAC) & (PPLC_XDMAC_TX_STATUS | PPLC_XDMAC_RX_STATUS)) ||
			!(spi_read_status(PPLC_SPI_MODULE) & SPI_SR_TXEMPTY)) {
		ul_spi_busy_cnt++;
		if (ul_spi_busy_cnt > 5000000) {
			/* Update SPI status */
			sb_spi_busy = false;
			/* Enable PLC interrupt */
			NVIC_EnableIRQ(PPLC_INT_IRQn);
#ifdef PPLC_SPI_SHARED_RF
			__set_BASEPRI(ul_basepri_prev);
#endif
			return false;
		}
	}

	/* Disable the RX and TX XDMAC transfer requests */
	xdmac_channel_disable(XDMAC, PPLC_XDMAC_CH_RX);
	xdmac_channel_disable(XDMAC, PPLC_XDMAC_CH_TX);

	/* Set 16 bits per transfer */
	spi_set_bits_per_transfer(PPLC_SPI_MODULE, PPLC_CS, SPI_CSR_BITS_16_BIT);

#ifdef PPLC_SPI_SHARED_RF

	/* Set chip select. Done in every SPI transaction in case same SPI with
	 * another CS is used by another module */
	spi_set_peripheral_chip_select_value(PPLC_SPI_MODULE, PPLC_PCS);

	/* Read received data from SPI in order to clear RDRF flag (if previous
	 * transaction with RF215 was in write mode the received data is not
	 * read). Otherwise the DMA would read first the last received byte from
	 * previous transaction */
	spi_get(PPLC_SPI_MODULE);
#endif

	/** Configure PPLC Tx buffer **/
	puc_tx_buf = gs_pplc_tx_buffer;
	/* Address */
	*puc_tx_buf++ = (uint8_t)(px_data->us_address & 0xFF);
	*puc_tx_buf++ = (uint8_t)(px_data->us_address >> 8);
	/* Length & read/write */
	us_len_wr_rd = (((px_data->us_len + 1) / 2) & PPLC_LEN_MASK) | (uc_cmd << PPLC_WR_RD_POS);
	*puc_tx_buf++ = (uint8_t)(us_len_wr_rd & 0xFF);
	*puc_tx_buf++ = (uint8_t)(us_len_wr_rd >> 8);

	if (uc_cmd == PPLC_CMD_WRITE) {
		memcpy(puc_tx_buf, px_data->puc_data_buf, px_data->us_len);
	} else {
		memset(puc_tx_buf, 0, px_data->us_len);
	}

	puc_tx_buf += px_data->us_len;

	us_tx_size = puc_tx_buf - gs_pplc_tx_buffer;
	if (us_tx_size % 2) {
		/* Add 1 padding byte */
		*puc_tx_buf++ = 0;
		us_tx_size++;
	}

#ifdef PPLC_CM7_CACHE_MAINTENANCE
	/* Clean DMA Tx buffer cache to avoid incoherency issues */
	SCB_CleanDCache_by_Addr((uint32_t *)gs_pplc_tx_buffer, us_tx_size);
#endif

	/* Configure TX and RX XDMAC channels */
	xdmac_rx_channel_cfg_cortex.mbr_ubc = us_tx_size / 2;
	xdmac_tx_channel_cfg_cortex.mbr_ubc = us_tx_size / 2;

	xdmac_configure_transfer(XDMAC, PPLC_XDMAC_CH_RX, &xdmac_rx_channel_cfg_cortex);
	xdmac_channel_set_descriptor_control(XDMAC, PPLC_XDMAC_CH_RX, 0);

	xdmac_configure_transfer(XDMAC, PPLC_XDMAC_CH_TX, &xdmac_tx_channel_cfg_cortex);
	xdmac_channel_set_descriptor_control(XDMAC, PPLC_XDMAC_CH_TX, 0);

	/* Enable the RX and TX XDMAC transfer requests */
	xdmac_channel_enable(XDMAC, PPLC_XDMAC_CH_RX);
	xdmac_channel_enable(XDMAC, PPLC_XDMAC_CH_TX);

#ifdef PPLC_SPI_SHARED_RF
	/* Protection: Wait until SPI is detected as busy */
	ul_spi_busy_cnt = 0;
	while (spi_read_status(PPLC_SPI_MODULE) & SPI_SR_TXEMPTY) {
		ul_spi_busy_cnt++;
		if (ul_spi_busy_cnt > 50) {
			/* It could have started and finished already */
			break;
		}
	}

	/* Leave critical region. The SPI transaction has started and interrupts
	 * can be enabled (RF SPI access function will block until this SPI
	 * transaction has finished) */
	__set_BASEPRI(ul_basepri_prev);
#endif

#ifdef PPLC_CM7_CACHE_MAINTENANCE
	/* Invalidate DMA Rx buffer cache to avoid incoherency issues */
	SCB_InvalidateDCache_by_Addr((uint32_t *)gs_pplc_rx_buffer, us_tx_size);
#endif

	/* Waiting transfer done*/
	/* while(pdc_read_tx_counter(g_pdc) > 0); */
	ul_spi_busy_cnt = 0;
	while (xdmac_channel_get_status(XDMAC) & PPLC_XDMAC_RX_STATUS) {
		ul_spi_busy_cnt++;
		if (ul_spi_busy_cnt > 5000000) {
			/* Update SPI status */
			sb_spi_busy = false;
			/* Enable PLC interrupt */
			NVIC_EnableIRQ(PPLC_INT_IRQn);
			return false;
		}
	}

	if (uc_cmd == PPLC_CMD_READ) {
		memcpy(px_data->puc_data_buf, &gs_pplc_rx_buffer[PPLC_SPI_HEADER_SIZE], px_data->us_len);
	}

	px_status_info->us_header_id = PPLC_GET_ID_HEADER(gs_pplc_rx_buffer[0], gs_pplc_rx_buffer[1]);
	if (PPLC_CHECK_ID_BOOT_HEADER(px_status_info->us_header_id)) {
		px_status_info->ul_flags = PPLC_GET_FLAGS_FROM_BOOT(gs_pplc_rx_buffer[0], gs_pplc_rx_buffer[2], gs_pplc_rx_buffer[3]);
	} else if (PPLC_CHECK_ID_CORTEX_HEADER(px_status_info->us_header_id)) {
		px_status_info->ul_flags = PPLC_GET_FLAGS_FROM_CORTEX(gs_pplc_rx_buffer[2], gs_pplc_rx_buffer[3]);
	} else {
		px_status_info->ul_flags = 0;
	}

	/* Update SPI status */
	sb_spi_busy = false;

	/* Enable PLC interrupt */
	NVIC_EnableIRQ(PPLC_INT_IRQn);

	return true;
}

#else

bool pplc_if_send_boot_cmd(uint16_t us_cmd, uint32_t ul_addr, uint32_t ul_data_len, uint8_t *puc_data_buf, uint8_t *puc_data_read)
{
	uint8_t *puc_tx_buf;
	uint32_t ul_spi_busy_cnt;
	uint16_t us_tx_size;

	/* Check length */
	if ((ul_data_len + 6) > PPLC_BUFFER_SIZE) {
		return false;
	}

	/* Disable PLC interrupt to avoid SPI access in the middle of a transaction */
	NVIC_DisableIRQ(PPLC_INT_IRQn);

	/* Check SPI status */
	if (sb_spi_busy) {
		return false;
	}

	/* Update SPI status */
	sb_spi_busy = true;

#ifdef PPLC_SPI_SHARED_RF

	/* Enter critical region. Disable all interrupts except highest priority
	 * (<2: 0, 1) to avoid wrong SPI transaction if same SPI (shared with
	 * RF) is used from IRQ */
	uint32_t ul_basepri_prev = __get_BASEPRI();
	if (ul_basepri_prev != (1 << (8 - __NVIC_PRIO_BITS))) {
		__set_BASEPRI(2 << (8 - __NVIC_PRIO_BITS));
	}
#endif

	/* Waiting transfer done while(pdc_read_tx_counter(g_pdc) > 0); */
	ul_spi_busy_cnt = 0;
	while ((spi_read_status(PPLC_SPI_MODULE) & (SPI_SR_TXEMPTY | SPI_SR_RXBUFF)) != (SPI_SR_TXEMPTY | SPI_SR_RXBUFF)) {
		ul_spi_busy_cnt++;
		if (ul_spi_busy_cnt > 5000000) {
			/* Update SPI status */
			sb_spi_busy = false;
			/* Enable PLC interrupt */
			NVIC_EnableIRQ(PPLC_INT_IRQn);
#ifdef PPLC_SPI_SHARED_RF
			__set_BASEPRI(ul_basepri_prev);
#endif
			return false;
		}
	}

	/* Disable the RX and TX PDC transfer requests */
	pdc_disable_transfer(g_pplc_pdc, PERIPH_PTCR_RXTDIS | PERIPH_PTCR_TXTDIS);

	/* Set 8 bits per transfer */
	spi_set_bits_per_transfer(PPLC_SPI_MODULE, PPLC_CS, SPI_CSR_BITS_8_BIT);

#ifdef PPLC_SPI_SHARED_RF

	/* Set chip select. Done in every SPI transaction in case same SPI with
	 * another CS is used by another module */
	spi_set_peripheral_chip_select_value(PPLC_SPI_MODULE, PPLC_PCS);

	/* Read received data from SPI in order to clear RDRF flag (if previous
	 * transaction with RF215 was in write mode the received data is not
	 * read). Otherwise the DMA would read first the last received byte from
	 * previous transaction */
	spi_get(PPLC_SPI_MODULE);
#endif

	/* Configure Tx buffer */
	puc_tx_buf = gs_pplc_tx_buffer;

	memcpy(puc_tx_buf, &ul_addr, sizeof(uint32_t));
	puc_tx_buf +=  sizeof(uint32_t);
	memcpy(puc_tx_buf, &us_cmd, sizeof(uint16_t));
	puc_tx_buf +=  sizeof(uint16_t);

	memcpy(puc_tx_buf, puc_data_buf, ul_data_len);

	puc_tx_buf += ul_data_len;

	us_tx_size = puc_tx_buf - gs_pplc_tx_buffer;

	/* Configure DMA channels */
	g_pplc_rx_packet.ul_addr = (uint32_t)gs_pplc_rx_buffer;
	g_pplc_rx_packet.ul_size = us_tx_size;
	pdc_rx_init(g_pplc_pdc, &g_pplc_rx_packet, NULL);

	g_pplc_tx_packet.ul_addr = (uint32_t)gs_pplc_tx_buffer;
	g_pplc_tx_packet.ul_size = us_tx_size;
	pdc_tx_init(g_pplc_pdc, &g_pplc_tx_packet, NULL);

	/* Enable the RX and TX PDC transfer requests */
	pdc_enable_transfer(g_pplc_pdc, PERIPH_PTCR_RXTEN | PERIPH_PTCR_TXTEN);

#ifdef PPLC_SPI_SHARED_RF
	/* Protection: Wait until SPI is detected as busy */
	ul_spi_busy_cnt = 0;
	while ((spi_read_status(PPLC_SPI_MODULE) & (SPI_SR_TXEMPTY | SPI_SR_RXBUFF)) == (SPI_SR_TXEMPTY | SPI_SR_RXBUFF)) {
		ul_spi_busy_cnt++;
		if (ul_spi_busy_cnt > 50) {
			/* It could have started and finished already */
			break;
		}
	}

	/* Leave critical region. The SPI transaction has started and interrupts
	 * can be enabled (RF SPI access function will block until this SPI
	 * transaction has finished) */
	__set_BASEPRI(ul_basepri_prev);
#endif

	/* Waiting transfer done and read */
	if (puc_data_read) {
		/* while(pdc_read_tx_counter(g_pdc) > 0); */
		ul_spi_busy_cnt = 0;
		while ((spi_read_status(PPLC_SPI_MODULE) & SPI_SR_RXBUFF) == 0) {
			ul_spi_busy_cnt++;
			if (ul_spi_busy_cnt > 5000000) {
				/* Update SPI status */
				sb_spi_busy = false;
				/* Enable PLC interrupt */
				NVIC_EnableIRQ(PPLC_INT_IRQn);
				return false;
			}
		}

		memcpy(puc_data_read, &gs_pplc_rx_buffer[6], ul_data_len);
	}

	/* Update SPI status */
	sb_spi_busy = false;

	/* Enable PLC interrupt */
	NVIC_EnableIRQ(PPLC_INT_IRQn);

	return true;
}

bool pplc_if_send_wrrd_cmd(uint8_t uc_cmd, void *px_spi_data, void *px_spi_status_info)
{
	uint8_t *puc_tx_buf;
	uint32_t ul_spi_busy_cnt;
	uint16_t us_tx_size;
	uint16_t us_len_wr_rd;
	spi_data_t *px_data;
	spi_status_info_t *px_status_info;

	px_data = (spi_data_t *)px_spi_data;
	px_status_info = (spi_status_info_t *)px_spi_status_info;

	/* Check length */
	if ((px_data->us_len == 0) || ((px_data->us_len + PPLC_SPI_HEADER_SIZE) > PPLC_BUFFER_SIZE)) {
		return false;
	}

	/* Disable PLC interrupt to avoid SPI access in the middle of a transaction */
	NVIC_DisableIRQ(PPLC_INT_IRQn);

	/* Check SPI status */
	if (sb_spi_busy) {
		return false;
	}

	/* Update SPI status */
	sb_spi_busy = true;

#ifdef PPLC_SPI_SHARED_RF

	/* Enter critical region. Disable all interrupts except highest priority
	 * (<2: 0, 1) to avoid wrong SPI transaction if same SPI (shared with
	 * RF) is used from IRQ */
	uint32_t ul_basepri_prev = __get_BASEPRI();
	if (ul_basepri_prev != (1 << (8 - __NVIC_PRIO_BITS))) {
		__set_BASEPRI(2 << (8 - __NVIC_PRIO_BITS));
	}
#endif

	/* Waiting transfer done while(pdc_read_tx_counter(g_pdc) > 0); */
	ul_spi_busy_cnt = 0;
	while ((spi_read_status(PPLC_SPI_MODULE) & (SPI_SR_TXEMPTY | SPI_SR_RXBUFF)) != (SPI_SR_TXEMPTY | SPI_SR_RXBUFF)) {
		ul_spi_busy_cnt++;
		if (ul_spi_busy_cnt > 5000000) {
			/* Update SPI status */
			sb_spi_busy = false;
			/* Enable PLC interrupt */
			NVIC_EnableIRQ(PPLC_INT_IRQn);
#ifdef PPLC_SPI_SHARED_RF
			__set_BASEPRI(ul_basepri_prev);
#endif
			return false;
		}
	}

	/* Disable the RX and TX PDC transfer requests */
	pdc_disable_transfer(g_pplc_pdc, PERIPH_PTCR_RXTDIS | PERIPH_PTCR_TXTDIS);

	/* Set 16 bits per transfer */
	spi_set_bits_per_transfer(PPLC_SPI_MODULE, PPLC_CS, SPI_CSR_BITS_16_BIT);

#ifdef PPLC_SPI_SHARED_RF

	/* Set chip select. Done in every SPI transaction in case same SPI with
	 * another CS is used by another module */
	spi_set_peripheral_chip_select_value(PPLC_SPI_MODULE, PPLC_PCS);

	/* Read received data from SPI in order to clear RDRF flag (if previous
	 * transaction with RF215 was in write mode the received data is not
	 * read). Otherwise the DMA would read first the last received byte from
	 * previous transaction */
	spi_get(PPLC_SPI_MODULE);
#endif

	/** Configure PPLC Tx buffer **/
	puc_tx_buf = gs_pplc_tx_buffer;
	/* Address */
	*puc_tx_buf++ = (uint8_t)(px_data->us_address & 0xFF);
	*puc_tx_buf++ = (uint8_t)(px_data->us_address >> 8);
	/* Length & read/write */
	us_len_wr_rd = (((px_data->us_len + 1) / 2) & PPLC_LEN_MASK) | (uc_cmd << PPLC_WR_RD_POS);
	*puc_tx_buf++ = (uint8_t)(us_len_wr_rd & 0xFF);
	*puc_tx_buf++ = (uint8_t)(us_len_wr_rd >> 8);

	if (uc_cmd == PPLC_CMD_WRITE) {
		memcpy(puc_tx_buf, px_data->puc_data_buf, px_data->us_len);
	} else {
		memset(puc_tx_buf, 0, px_data->us_len);
	}

	puc_tx_buf += px_data->us_len;

	us_tx_size = puc_tx_buf - gs_pplc_tx_buffer;
	if (us_tx_size % 2) {
		/* Add 1 padding byte */
		*puc_tx_buf++ = 0;
		us_tx_size++;
	}

	/* Configure DMA channels */
	g_pplc_rx_packet.ul_addr = (uint32_t)gs_pplc_rx_buffer;
	g_pplc_rx_packet.ul_size = us_tx_size / 2;
	pdc_rx_init(g_pplc_pdc, &g_pplc_rx_packet, NULL);

	g_pplc_tx_packet.ul_addr = (uint32_t)gs_pplc_tx_buffer;
	g_pplc_tx_packet.ul_size = us_tx_size / 2;
	pdc_tx_init(g_pplc_pdc, &g_pplc_tx_packet, NULL);

	/* Enable the RX and TX PDC transfer requests */
	pdc_enable_transfer(g_pplc_pdc, PERIPH_PTCR_RXTEN | PERIPH_PTCR_TXTEN);

#ifdef PPLC_SPI_SHARED_RF
	/* Protection: Wait until SPI is detected as busy */
	ul_spi_busy_cnt = 0;
	while ((spi_read_status(PPLC_SPI_MODULE) & (SPI_SR_TXEMPTY | SPI_SR_RXBUFF)) == (SPI_SR_TXEMPTY | SPI_SR_RXBUFF)) {
		ul_spi_busy_cnt++;
		if (ul_spi_busy_cnt > 50) {
			/* It could have started and finished already */
			break;
		}
	}

	/* Leave critical region. The SPI transaction has started and interrupts
	 * can be enabled (RF SPI access function will block until this SPI
	 * transaction has finished) */
	__set_BASEPRI(ul_basepri_prev);
#endif

	/* Waiting transfer done */
	ul_spi_busy_cnt = 0;
	while ((spi_read_status(PPLC_SPI_MODULE) & SPI_SR_RXBUFF) == 0) {
		ul_spi_busy_cnt++;
		if (ul_spi_busy_cnt > 5000000) {
			/* Update SPI status */
			sb_spi_busy = false;
			/* Enable PLC interrupt */
			NVIC_EnableIRQ(PPLC_INT_IRQn);
			return false;
		}
	}

	if (uc_cmd == PPLC_CMD_READ) {
		memcpy(px_data->puc_data_buf, &gs_pplc_rx_buffer[PPLC_SPI_HEADER_SIZE], px_data->us_len);
	}

	px_status_info->us_header_id = PPLC_GET_ID_HEADER(gs_pplc_rx_buffer[0], gs_pplc_rx_buffer[1]);
	if (PPLC_CHECK_ID_BOOT_HEADER(px_status_info->us_header_id)) {
		px_status_info->ul_flags = PPLC_GET_FLAGS_FROM_BOOT(gs_pplc_rx_buffer[0], gs_pplc_rx_buffer[2], gs_pplc_rx_buffer[3]);
	} else if (PPLC_CHECK_ID_CORTEX_HEADER(px_status_info->us_header_id)) {
		px_status_info->ul_flags = PPLC_GET_FLAGS_FROM_CORTEX(gs_pplc_rx_buffer[2], gs_pplc_rx_buffer[3]);
	} else {
		px_status_info->ul_flags = 0;
	}

	/* Update SPI status */
	sb_spi_busy = false;

	/* Enable PLC interrupt */
	NVIC_EnableIRQ(PPLC_INT_IRQn);

	return true;
}

#endif

void pplc_if_enable_interrupt(bool enable)
{
	if (enable) {
		pio_enable_interrupt(PPLC_INT_PIO, PPLC_INT_MASK);
	} else {
		pio_disable_interrupt(PPLC_INT_PIO, PPLC_INT_MASK);
	}
}

void pplc_if_delay(uint8_t uc_tref, uint32_t ul_delay)
{
	if (uc_tref == PPLC_DELAY_TREF_SEC) {
		delay_s(ul_delay);
	} else if (uc_tref == PPLC_DELAY_TREF_MS) {
		delay_ms(ul_delay);
	} else if (uc_tref == PPLC_DELAY_TREF_US) {
		delay_us(ul_delay);
	}
}

bool pplc_if_set_stby_mode(bool sleep)
{
#ifdef PPLC_STBY_GPIO
	if (sleep) {
		/* Set RESET pin */
		ioport_set_pin_level(PPLC_RESET_GPIO, PPLC_RESET_ACTIVE_LEVEL);
		/* Set STBY pin */
		ioport_set_pin_level(PPLC_STBY_GPIO, PPLC_STBY_ACTIVE_LEVEL);
	} else {
		/* Clear STBY pin */
		ioport_set_pin_level(PPLC_STBY_GPIO, PPLC_STBY_INACTIVE_LEVEL);
		/* Wait STBY disabled */
		delay_us(100);
		/* Clear RESET pin */
		ioport_set_pin_level(PPLC_RESET_GPIO, PPLC_RESET_INACTIVE_LEVEL);
		/* Wait RESET disabled */
		delay_us(750);
	}

	return true;

#else
	/* STBY pin not available */
	UNUSED(sleep);
	return false;
#endif
}

bool pplc_if_get_thermal_warning(void)
{
#ifdef PPLC_NTHW0_GPIO
	if (ioport_get_pin_level(PPLC_NTHW0_GPIO)) {
		/* NTHW0 high level: Normal condition */
		return false;
	} else {
		/* NTHW0 low level: High temperature (>110�C) condition */
		return true;
	}

#else
	/* NTHW0 pin not available */
	return false;
#endif
}

/**
 * \brief Critical initialization of PPLC interface. PL360 control pins are
 * initialized (otherwise all GPIO with pull-up at reset). This function should
 * be called as soon as possible after Reset_Handler to avoid issues in
 * PL460/480 without pull-down resistor in the PLC reset pin.
 * This initialization is not critical for PL360/485 or PL460/480 with pull-down
 * resistor in the PLC reset pin.
 */
void pplc_if_crit_init(void)
{
	/* Configure PLC reset pin */
	pmc_enable_periph_clk(pio_get_pin_group_id(PPLC_RESET_GPIO));
	ioport_set_pin_level(PPLC_RESET_GPIO, PPLC_RESET_ACTIVE_LEVEL);
	ioport_set_pin_dir(PPLC_RESET_GPIO, IOPORT_DIR_OUTPUT);

#ifdef PPLC_STBY_GPIO
	/* Configure STBY pin */
	pmc_enable_periph_clk(pio_get_pin_group_id(PPLC_STBY_GPIO));
	ioport_set_pin_level(PPLC_STBY_GPIO, PPLC_STBY_INACTIVE_LEVEL);
	ioport_set_pin_dir(PPLC_STBY_GPIO, IOPORT_DIR_OUTPUT);
#endif

	/* Configure LDO_EN pin */
	pmc_enable_periph_clk(pio_get_pin_group_id(PPLC_LDO_EN_GPIO));
	ioport_set_pin_level(PPLC_LDO_EN_GPIO, PPLC_LDO_EN_INACTIVE_LEVEL);
	ioport_set_pin_dir(PPLC_LDO_EN_GPIO, IOPORT_DIR_OUTPUT);
}

/* @} */

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* @endcond */
