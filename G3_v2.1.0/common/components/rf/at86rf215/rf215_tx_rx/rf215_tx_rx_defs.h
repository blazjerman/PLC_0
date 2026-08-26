/**
 *
 * \file
 *
 * \brief RF215 TX/RX controller definitions.
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

#ifndef RF215_TX_RX_DEFS_H_INCLUDE
#define RF215_TX_RX_DEFS_H_INCLUDE

/* RF215 includes */
#include "at86rf_defs.h"
#if SAME70
# include "conf_board.h"
#endif

/* Minimum / maximum delay in the future for programmed TX in us */
#define RF215_TX_PROG_MIN_DELAY_US        (-10000)
#define RF215_TX_PROG_MAX_DELAY_US        600000000

/* Delays and margins for programmed TX (FW delay compensation) in us. Not
 * critical, but they can affect the TX time accuaracy due to higher/same
 * priority IRQs and depend on the FW (compiler, optimizations, CPU frequency).
 * Measured for SAMG55 @96MHz and IAR compiler with High Optimization (Size) */
#define RF215_TX_PROG_INT_MARGIN_US       75
#define RF215_TIMER_PROG_INT_MARGIN_US    20
#define RF215_TX_PROG_CFG_DELAY_US        12
#define RF215_TX_PROG_TCHECK_DELAY_US     6
#define RF215_TX_PROG_TXPREP_DELAY_US     12
#define RF215_TX_PROG_TXPREP_CCA_DELAY_US 17
#define RF215_RF_INT_DELAY_US             4
#define RF215_TIMER_INT_DELAY_US          8

/** Offset to adjust TX time (FW delay compensation) in us [uQ3.5].
 * This offset affects the TX time accuaracy and depends on the core and FW used
 * (compiler, optimizations, CPU frequency).
 * To achieve best accuracy it should be adjusted for every core and
 * compiler/CPU freq.
 * Anyway, the error shouldn't be > +/- ~10us */
#if SAME70
# ifdef CONF_BOARD_ENABLE_TCM_AT_INIT

/* Measured for SAME70 @300MHz with TCM and cache enabled and IAR compiler with
 * High Optimization (Size)
 * It is assummed that rf215_rx, prf_if and STACK data is mapped to TCM region
 * by linker script. */
#  define RF215_TX_TIME_OFFSET_US_Q5      16 /* 0.5 us */
# elif defined(CONF_BOARD_ENABLE_CACHE)
#  if defined(CONF_BOARD_CONFIG_MPU_AT_INIT) && defined(MPU_HAS_NOCACHE_REGION)

/* Measured for SAME70 @300MHz with cache enabled (with non-cacheable region
 * configured by MPU) and IAR compiler with High Optimization (Size).
 * It is assummed that rf215_rx, prf_if and STACK data is mapped to
 * non-cacheable region by linker script.
 * Time offset very similar to cache disabled because of MPU configuration in
 * board init (flash non-cacheable). */
#   define RF215_TX_TIME_OFFSET_US_Q5     128 /* 4 us */
#  else

/* Measured for SAME70 @300MHz with cache enabled (with cache maintenance
 * operations) and IAR compiler with High Optimization (Size) */
#   define RF215_TX_TIME_OFFSET_US_Q5     24 /* 0.75 us */
#  endif
# else
/* Measured for SAME70 @300MHz and IAR compiler with High Optimization (Size) */
#  define RF215_TX_TIME_OFFSET_US_Q5      128 /* 4 us */
# endif
#else
/* Measured for SAMG55 @96MHz and IAR compiler with High Optimization (Size) */
# define RF215_TX_TIME_OFFSET_US_Q5       120 /* 3.75 us */
#endif

/** Depth of TX confirm queue (Number of programmed TX + 1 instantaneous) */
#define RF215_NUM_TX_CONFIRMS             (AT86RF215_NUM_TX_PROG_BUFFERS + 1)

/* RF215 Programmed TX request & buffer struct */
typedef struct rf215_tx_prog {
	uint32_t ul_time_int_id;
	at86rf_tx_params_t x_params;
	bool b_free;
	uint8_t puc_buf[AT86RF215_MAX_PSDU_LEN];
} rf215_tx_prog_t;

/* RF215 RX indication & buffer struct */
typedef struct rf215_rx_ind {
	at86rf_rx_ind_t x_ind;
	uint8_t puc_buf[AT86RF215_MAX_PSDU_LEN];
} rf215_rx_ind_t;

/* RF215 TX control struct */
typedef struct rf215_tx_ctl {
	at86rf_tx_cfm_t px_tx_cfm[RF215_NUM_TX_CONFIRMS];
	rf215_tx_prog_t px_tx_prog[AT86RF215_NUM_TX_PROG_BUFFERS];
	at86rf_cca_ed_cfg_t x_cca_ed_cfg;
	uint32_t ul_tx_time_cmd;
	uint32_t ul_frame_duration;
	uint16_t us_psdu_len;
	uint16_t us_tx_cmd_delay_us;
	uint16_t us_tx_param_cfg_delay_us;
	uint16_t us_trxrdy_delay_us;
	uint16_t us_trxoff_txprep_delay_us;
	uint16_t us_bb_delay_us_q5;
	uint16_t us_proc_delay_us_q5;
	uint8_t uc_pe_delay_us_q5;
	uint8_t uc_tx_id;
	uint8_t uc_cw;
	uint8_t uc_tx_cfm_wr;
	uint8_t uc_tx_cfm_rd;
	uint8_t uc_tx_cfm_pending;
	bool b_cca_ed;
	bool b_tx_on;
	bool b_ongoing_tx_aborted;
} rf215_tx_ctl_t;

/* RF215 RX control struct */
typedef struct rf215_rx_ctl {
	uint16_t us_rx_buf_offset;
	rf215_rx_ind_t px_rx_ind[AT86RF215_NUM_RX_BUFFERS];
	uint8_t uc_rx_ind_wr;
	uint8_t uc_rx_ind_rd;
	uint8_t uc_rx_ind_pending;
	bool b_rx_abort;
} rf215_rx_ctl_t;

/** RF215 internal global variables declared as extern */
extern at86rf_callbacks_t gx_rf215_callbacks;
extern uint8_t guc_spi_byte_time_us_q5;

#ifdef __cplusplus
}
#endif

#endif  /* RF215_TX_RX_DEFS_H_INCLUDE */
