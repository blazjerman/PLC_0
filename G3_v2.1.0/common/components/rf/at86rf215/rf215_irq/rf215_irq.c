/**
 *
 * \file
 *
 * \brief RF215 IRQ handler.
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

/* RF215 includes */
#include "rf215_irq.h"
#include "rf215_irq_defs.h"
#include "rf215_pll.h"
#include "rf215_fe.h"
#include "rf215_bbc.h"
#include "rf215_tx_rx.h"
#include "rf215_trx_ctl.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

/* Counter of consecutive RF interrupts without any reported flag */
extern uint8_t guc_irqs_empty_count;

/**
 * \brief RF215 Chip Reset Event. Initialize common registers after Power-on
 * Reset or Chip Reset (Wake-up Interrupt on both TRX). This function is called
 * from IRQ.
 */
static inline void _rf215_irq_chip_reset(void)
{
	uint8_t puc_pn_vn[2] = {0};

	/* Check Device Part Number and Version Number registers */
	rf215_spi_read(RF215_ADDR_RF_PN, puc_pn_vn, 2);
	if ((puc_pn_vn[0] == RF215_PART_NUMBER) && (puc_pn_vn[1] == RF215_RF_VN_V3)) {
		/* Disable clock output by default (not used) */
		rf215_spi_reg_write(RF215_ADDR_RF_CLKO, RF_CLKO_CFG);

		/* Everything OK. Common registers initialized. Set chip reset
		 * flag */
		guc_rf215_exception_mask |= AT86RF_EXCEPTION_RESET;

		/* Turn on TX and RX LEDs to indicate chip reset */
		gx_rf215_hal_wrp.rf_led(RF215_LED_TX, true);
		gx_rf215_hal_wrp.rf_led(RF215_LED_RX, true);

#if defined(AT86RF215_DISABLE_RF09_TRX)
		/* RF09 TRX disabled: Switch it to sleep state to save power */
		rf215_spi_reg_write(RF215_ADDR_RF09_CMD, RF215_RFn_CMD_RF_SLEEP);
#endif

#ifdef AT86RF215_DISABLE_RF24_TRX
		/* RF24 TRX disabled: Switch it to sleep state to save power */
		rf215_spi_reg_write(RF215_ADDR_RF24_CMD, RF215_RFn_CMD_RF_SLEEP);
#endif
	} else {
		/* Unexpected Part or Version Number. Set exception flag */
		guc_rf215_exception_mask |= AT86RF_EXCEPTION_INIT_ERR;
	}
}

/**
 * \brief RF215 TRX Reset Event. Initialize specific registers of the
 * corresponing transceiver after TRX Reset (Wake-up interrupt). This function
 * is called from IRQ.
 *
 * \param uc_trx_id TRX identifier
 */
static inline void _rf215_irq_trx_reset(uint8_t uc_trx_id)
{
	rf_phy_state_t uc_phy_state = gpx_phy_ctl[uc_trx_id].uc_phy_state;

	if (uc_phy_state == RF_PHY_STATE_SLEPT) {
		/* Unexpected reset while TRX slept. Send SLEEP command again */
		rf215_trx_cmd_sleep(uc_trx_id);
	} else if (uc_phy_state != RF_PHY_STATE_RESET) {
		/* Unexpected reset. Abort TX/RX in progress */
		rf215_tx_rx_check_aborts(uc_trx_id, true);

		/* Reset TRX to process reset under control.
		 * Processed in new IRQ. TRX will be in TRXOFF */
		rf215_trx_cmd_reset(uc_trx_id);
		gpx_phy_ctl[uc_trx_id].uc_phy_state = RF_PHY_STATE_RESET;
	} else {
		uint8_t puc_rf_irqm_auxs[2];

		/* RFn_IRQM: Enable WAKEUP, TRXRDY, EDC and TRXERR interrupts */
		puc_rf_irqm_auxs[0] = RF_IRQM_CFG;

		/* RFn_AUXS: Enable AUXS.AVEN bit by default. The state
		* transition time is reduced, at the cost of increasing power
		* consumption in TRXOFF state, but if the transceiver will
		* always be transmitting/receiving it doesn't matter */
		puc_rf_irqm_auxs[1] = RF_AUXS_CFG;

		/* Write 2 registers: RFn_IRQM, RFn_AUXS */
		rf215_spi_write(RF215_ADDR_RFn_IRQM(uc_trx_id), puc_rf_irqm_auxs, 2);

		/* Initialize Frequency Synthesizer (PLL) registers */
		rf215_pll_trx_reset_event(uc_trx_id);

		/* Initialize Baseband Core (PHY) registers */
		rf215_bbc_trx_reset_event(uc_trx_id);

		/* Initialize TX&RX frontend registers */
		rf215_fe_trx_reset_event(uc_trx_id);

		/* TRX registers initialized. Start listening */
		rf215_trx_rx_listen(uc_trx_id, true);
	}
}

/**
 * \brief RF215 Transceiver Ready Event. Update internal states after
 * RFn_IRQS.TRXRDY interrupt. Programmed TX can be started here. This function
 * is called from IRQ.
 *
 * \param uc_trx_id TRX identifier
 */
static inline void _rf215_irq_trxrdy(uint8_t uc_trx_id)
{
	/* Set TRXDY flag */
	gpx_phy_ctl[uc_trx_id].b_trxrdy = true;

	/* Process programmed TX after TRXRDY interrupt */
	rf215_tx_trxrdy_event(uc_trx_id);
}

/**
 * \brief RF215 Energy Detection Completion Event. Handle RFn_IRQS.EDC. This
 * function is called from IRQ
 *
 * \param uc_trx_id TRX identifier
 */
static inline void _rf215_irq_edc(uint8_t uc_trx_id)
{
	/* Process TX CCA ED after EDC interrupt */
	rf215_tx_edc_event(uc_trx_id);
}

/**
 * \brief RF215 Transceiver Error Event. Update internal states and send the
 * appropriate commands after RFn_IRQS.TRXERR interrupt. This function is called
 * from IRQ.
 *
 * \param uc_trx_id TRX identifier
 */
static inline void _rf215_irq_trxerr(uint8_t uc_trx_id)
{
	/* TO DO */
	UNUSED(uc_trx_id);
}

/**
 * \brief RF215 External interrupt handler
 */
void rf215_irq_handler(void)
{
	bool b_ongoing_tx;
	bool b_ongoing_rx;
	bool b_irqs_all_0;
	bool b_irqs_all_1;
	bool b_critical_error;
	uint8_t puc_irqs[RF215_IRQS_READ_LEN] = {0};

	/* Check if component is ready to handle interrupt */
	if (guc_rf215_comp_state < RF215_COMPONENT_ENABLING) {
		return;
	}

	/* Check if SPI is free. If it is busy, do nothing to avoid blocking in
	 * the IRQS SPI read and allow interrupts with same or lower priority to
	 * be handled. RF IRQ pin is configured with level interrupt, so IRQ
	 * will be triggered again until IRQS is read */
	if (gx_rf215_hal_wrp.rf_is_spi_busy()) {
		return;
	}

	/* Critical region to avoid conflicts from timer IRQ */
	gx_rf215_hal_wrp.timer_enable_int(false);

	/* Read IRQ Status registers (RFn_IRQS, BBCn,_IRQS) */
	rf215_spi_read(RF215_IRQS_READ_ADDR, puc_irqs, RF215_IRQS_READ_LEN);

	/* Check if flags are all 0's or all 1's */
	b_irqs_all_0 = true;
	b_irqs_all_1 = true;
	for (uint8_t uc_i = 0; uc_i < RF215_IRQS_READ_LEN; uc_i++) {
		uint8_t uc_irqs = puc_irqs[uc_i];
		if (uc_irqs != 0) {
			b_irqs_all_0 = false;
			guc_irqs_empty_count = 0;
		}

		if (uc_irqs != 0xFF) {
			b_irqs_all_1 = false;
		}
	}

	/* Critical error if flags are all 1's (once) or all 0's (5 times) */
	b_critical_error = b_irqs_all_1;
	if (b_irqs_all_0) {
		if (++guc_irqs_empty_count == 5) {
			b_critical_error = true;
		}
	}

	/* Check Chip Reset / Power-On-Reset (Wake-up Interrupt on both TRX) */
	if ((puc_irqs[0] & puc_irqs[1] & RF215_RFn_IRQ_WAKEUP) && (!b_critical_error)) {
		_rf215_irq_chip_reset();
		if (guc_rf215_exception_mask & AT86RF_EXCEPTION_INIT_ERR) {
			b_critical_error = true;
		}
	}

	/* If critical error at initialization, initialize again TRX interface
	 * (SPI, and RST/IRQ pins). RST pin is kept pushed. IRQ pin interrupt is
	 * disabled (in PIO, not in NVIC) */
	if (b_critical_error) {
		gx_rf215_hal_wrp.rf_init();
		guc_rf215_comp_state = RF215_COMPONENT_ERROR;
		guc_rf215_exception_mask |= AT86RF_EXCEPTION_INIT_ERR;
		gx_rf215_hal_wrp.timer_enable_int(true);
		return;
	}

	b_ongoing_tx = false;
	b_ongoing_rx = false;

	for (uint8_t uc_trx_id = 0; uc_trx_id < AT86RF_NUM_TRX; uc_trx_id++) {
		rf_phy_state_t uc_phy_state;
		uint8_t uc_rf_irqs;
		uint8_t uc_bbc_irqs;
		uint8_t uc_rx_flags;
		uint8_t uc_trx_offset = uc_trx_id + RF215_IRQS_READ_OFFSET;

		/* Check RFn_IRQS flags */
		uc_rf_irqs = puc_irqs[uc_trx_offset];

		/* Check TRX Wake-up interrupt */
		if (uc_rf_irqs & RF215_RFn_IRQ_WAKEUP) {
			_rf215_irq_trx_reset(uc_trx_id);
		}

		/* Check TRXRDY interrupt */
		if (uc_rf_irqs & RF215_RFn_IRQ_TRXRDY) {
			_rf215_irq_trxrdy(uc_trx_id);
		}

		/* Check EDC interrupt */
		if (uc_rf_irqs & RF215_RFn_IRQ_EDC) {
			_rf215_irq_edc(uc_trx_id);
		}

		/* Check TRXERR interrupt */
		if (uc_rf_irqs & RF215_RFn_IRQ_TRXERR) {
			_rf215_irq_trxerr(uc_trx_id);
		}

		/* Check BBCnn_IRQS flags */
		uc_bbc_irqs = puc_irqs[uc_trx_offset + 2];

		/* Check TXFE interrupt */
		if (uc_bbc_irqs & RF215_BBCn_IRQ_TXFE) {
			rf215_tx_frame_end_event(uc_trx_id);
		}

		/* Check Receiver interrupts */
		uc_rx_flags = uc_bbc_irqs & BBC_IRQS_RX_FLAGS;
		if (uc_rx_flags != 0) {
			rf215_rx_event(uc_trx_id, uc_rx_flags);
		}

		/* Check ongoing TX and RX */
		uc_phy_state = gpx_phy_ctl[uc_trx_id].uc_phy_state;
		if ((uc_phy_state == RF_PHY_STATE_TX) || (uc_phy_state == RF_PHY_STATE_TX_CONTINUOUS)) {
			b_ongoing_tx = true;
		} else if ((uc_phy_state == RF_PHY_STATE_RX_HEADER) || (uc_phy_state == RF_PHY_STATE_RX_PAYLOAD)) {
			b_ongoing_rx = true;
		}
	}

	/* Turn off TX LED (if there is not TX in progress) */
	if (!b_ongoing_tx) {
		gx_rf215_hal_wrp.rf_led(RF215_LED_TX, false);
	}

	/* Turn off RX LED (if there is not RX in progress) */
	if (!b_ongoing_rx) {
		gx_rf215_hal_wrp.rf_led(RF215_LED_RX, false);
	}

	/* Leave critical region */
	gx_rf215_hal_wrp.timer_enable_int(true);
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
