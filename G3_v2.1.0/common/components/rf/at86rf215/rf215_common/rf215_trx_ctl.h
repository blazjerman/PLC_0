/**
 *
 * \file
 *
 * \brief RF215 TRX control.
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

#ifndef RF215_TRX_CTL_H_INCLUDE
#define RF215_TRX_CTL_H_INCLUDE

/* RF215 includes */
#include "at86rf_defs.h"
#include "rf215_spi.h"
#include "rf215_reg.h"
#include "rf215_phy_defs.h"
#if SAME70
# include "conf_board.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** RF_PN – Device Part Number */
#if (AT86RF_PART == AT86RF_PART_AT86RF215)
# define RF215_PART_NUMBER            RF215_RF_PN_AT86RF215
#elif (AT86RF_PART == AT86RF_PART_AT86RF215Q)
# define RF215_PART_NUMBER            RF215_RF_PN_AT86RF215Q
#elif (AT86RF_PART == AT86RF_PART_AT86RF215M)
/* Errata #9: The RF215M device part number is 0x34 instead of 0x36 */
# define RF215_PART_NUMBER            RF215_RF_PN_AT86RF215 /* RF215_RF_PN_AT86RF215M */
#endif

/** RF_CLKO register value
 * CLKO.OS: Disable clock output; CLKO.DRV: minimum driving */
#define RF_CLKO_CFG                   (RF215_RF_CLKO_OS_OFF | RF215_RF_CLKO_DRV_2mA)

/** RF_AUXS register value. Disable clock output
 * AVEN=1: State transition time reduced, power consumption in TRXOFF increased
 * PAVC: 2.4V Power amplifier volgate control
 * AVEXT: Internal analog supply voltage is used
 * AGCMAP: Internal AGC, no external LNA used
 * EXTLNABYP: Bypass of external LNA not available */
#define RF_AUXS_CFG                   (RF215_RFn_AUXS_AVEN_EN |	\
	RF215_RFn_AUXS_PAVC_2_4V | RF215_RFn_AUXS_AVEXT_INT | \
	RF215_RFn_AUXS_AGCMAP_INT | RF215_RFn_AUXS_EXTLNABYP_DIS)

/** RFn_IRQM register value */
#define RF_IRQM_CFG                   (RF215_RFn_IRQ_WAKEUP | \
	RF215_RFn_IRQ_TRXRDY | RF215_RFn_IRQ_TRXERR | RF215_RFn_IRQ_EDC)

/** TRX transition times [Table 10-7].
 * From TRXOFF to TXPREP in us. Max. 200us
 * From TXPREP to TX in us [uQ0.5]. Max. 200ns
 * From RX to TX (CCATX) in us [uQ0.5]. Max. 400ns
 * Energy detection (EDM_SINGLE) delay. 4.75us [uQ0.5] (not in datasheet) */
#define RF215_TRXOFF_TXPREP_TIME_US     200
#define RF215_TXPREP_TX_TIME_US_Q5      6
#define RF215_RX_TX_TIME_US_Q5          13
#define RF215_RX_CCA_ED_TIME_US_Q5      152

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
#  define RF215_SYNC_TIME_OFFSET_US_Q5  8 /* 0.25 us */
# elif defined(CONF_BOARD_ENABLE_CACHE)
#  if defined(CONF_BOARD_CONFIG_MPU_AT_INIT) && defined(MPU_HAS_NOCACHE_REGION)

/* Measured for SAME70 @300MHz with cache enabled (with non-cacheable region
 * configured by MPU) and IAR compiler with High Optimization (Size).
 * It is assummed that rf215_rx, prf_if and STACK data is mapped to
 * non-cacheable region by linker script.
 * Time offset very similar to cache disabled because of MPU configuration in
 * board init (flash non-cacheable). */
#   define RF215_SYNC_TIME_OFFSET_US_Q5 92 /* 2.875 us */
#  else

/* Measured for SAME70 @300MHz with cache enabled (with cache maintenance
 * operations) and IAR compiler with High Optimization (Size) */
#   define RF215_SYNC_TIME_OFFSET_US_Q5 16 /* 0.5 us */
#  endif
# else
/* Measured for SAME70 @300MHz and IAR compiler with High Optimization (Size) */
#  define RF215_SYNC_TIME_OFFSET_US_Q5  92 /* 2.875 us */
# endif
#else
/* Measured for SAMG55 @96MHz and IAR compiler with High Optimization (Size) */
# define RF215_SYNC_TIME_OFFSET_US_Q5   92 /* 2.875 us */
#endif

/** Macro to convert time in us [uQ11.5] (@32MHz) to us [uQ11.0] */
#define RF215_TIME_US_Q5_TO_US(x)       ((x + 16) >> 5)

/** RF215 TRX control inline function definition */

/**
 * \brief Send SPI command to change TRX state
 *
 * \param uc_trx_id TRX identifier
 * \param uc_cmd TRX state command
 */
__always_inline void rf215_trx_cmd(uint8_t uc_trx_id, uint8_t uc_cmd)
{
	rf215_spi_reg_write(RF215_ADDR_RFn_CMD(uc_trx_id), uc_cmd);
}

/**
 * \brief Send SLEEP command to TRX through SPI
 *
 * \param uc_trx_id TRX identifier
 */
__always_inline void rf215_trx_cmd_sleep(uint8_t uc_trx_id)
{
	rf215_trx_cmd(uc_trx_id, RF215_RFn_CMD_RF_SLEEP);

	/* Update TRX and PHY state variables */
	gpx_phy_ctl[uc_trx_id].uc_phy_state = RF_PHY_STATE_SLEPT;
	gpx_phy_ctl[uc_trx_id].uc_trx_state = RF215_RFn_STATE_RF_RESET;
}

/**
 * \brief Send TRXOFF command to TRX through SPI
 *
 * \param uc_trx_id TRX identifier
 */
__always_inline void rf215_trx_cmd_trxoff(uint8_t uc_trx_id)
{
	rf215_trx_cmd(uc_trx_id, RF215_RFn_CMD_RF_TRXOFF);

	/* Update TRX state variable */
	gpx_phy_ctl[uc_trx_id].uc_trx_state = RF215_RFn_STATE_RF_TRXOFF;
}

/**
 * \brief Send TXPREP command to TRX through SPI
 *
 * \param uc_trx_id TRX identifier
 */
__always_inline void rf215_trx_cmd_txprep(uint8_t uc_trx_id)
{
	rf215_trx_cmd(uc_trx_id, RF215_RFn_CMD_RF_TXPREP);

	/* Clear TRXRDY flag. It will be set in TRXRDY IRQ */
	gpx_phy_ctl[uc_trx_id].b_trxrdy = false;
	/* Update TRX state variable */
	gpx_phy_ctl[uc_trx_id].uc_trx_state = RF215_RFn_STATE_RF_TXPREP;
}

/**
 * \brief Send TX command to TRX through SPI
 *
 * \param uc_trx_id TRX identifier
 */
__always_inline void rf215_trx_cmd_tx(uint8_t uc_trx_id)
{
	rf215_trx_cmd(uc_trx_id, RF215_RFn_CMD_RF_TX);

	/* Turn on TX LED */
	gx_rf215_hal_wrp.rf_led(RF215_LED_TX, true);
	/* Update TRX and PHY state variables */
	gpx_phy_ctl[uc_trx_id].uc_trx_state = RF215_RFn_STATE_RF_TX;
	gpx_phy_ctl[uc_trx_id].uc_phy_state = RF_PHY_STATE_TX;
}

/**
 * \brief Send RX command to TRX through SPI
 *
 * \param uc_trx_id TRX identifier
 */
__always_inline void rf215_trx_cmd_rx(uint8_t uc_trx_id)
{
	rf215_trx_cmd(uc_trx_id, RF215_RFn_CMD_RF_RX);

	/* Update TRX and PHY state variables */
	gpx_phy_ctl[uc_trx_id].uc_trx_state = RF215_RFn_STATE_RF_RX;
}

/**
 * \brief Send RESET command to TRX through SPI
 *
 * \param uc_trx_id TRX identifier
 */
__always_inline void rf215_trx_cmd_reset(uint8_t uc_trx_id)
{
	rf215_trx_cmd(uc_trx_id, RF215_RFn_CMD_RF_RESET);

	/* Update TRX state variable */
	gpx_phy_ctl[uc_trx_id].uc_trx_state = RF215_RFn_STATE_RF_TRXOFF;
}

/**
 * \brief Convert RX/TX RF event time to PHY time counter of 1 us.
 * Synchronization times must be previously obtained (rf215_trx_upd_sync)
 *
 * \param uc_trx_id TRX identifier
 * \param ul_ev_time_trx Time of RX/TX RF event (RF215 counter @32MHz)
 *
 * \return RX/TX RF event time, referred to PHY time counter of 1 us
 */
__always_inline uint32_t rf215_trx_to_phy_time(uint8_t uc_trx_id, uint32_t ul_ev_time_trx)
{
	uint32_t ul_sync_time_trx;
	uint32_t ul_sync_time_phy;
	uint32_t ul_ev_time_phy;
	int32_t sl_trx_time_diff;

	/* Difference between TRX synchronization time and TX/RX event time */
	ul_sync_time_trx = gpx_phy_ctl[uc_trx_id].ul_sync_time_trx;
	sl_trx_time_diff = (int32_t)ul_ev_time_trx - ul_sync_time_trx;

	/* Compute PHY event time */
	ul_sync_time_phy = gpx_phy_ctl[uc_trx_id].ul_sync_time_phy;
	ul_ev_time_phy = ul_sync_time_phy + RF215_TIME_US_Q5_TO_US(sl_trx_time_diff);

	return ul_ev_time_phy;
}

/** RF215 TRX control function declaration */
bool rf215_trx_switch_trxoff(uint8_t uc_trx_id);
bool rf215_trx_switch_txprep(uint8_t uc_trx_id);
void rf215_trx_rx_listen(uint8_t uc_trx_id, bool b_restore_channel);
void rf215_trx_wait_pll_lock(uint8_t uc_trx_id);
void rf215_trx_upd_sync(uint8_t uc_trx_id);

#ifdef __cplusplus
}
#endif

#endif  /* RF215_TRX_CTL_H_INCLUDE */
