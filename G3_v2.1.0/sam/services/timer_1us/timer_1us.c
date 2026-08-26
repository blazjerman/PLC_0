/**
 *
 * \file
 *
 * \brief Timer of 1 us service.
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

#include "timer_1us.h"
#include "timer_1us_hal.h"
#include "conf_timer_1us.h"
#include "tc.h"
#include "sysclk.h"
#include "string.h"

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* @endcond */

/* Delay between current time and programmed interrupt limited to
 * [20 us, 35 minutes (half loop of 32-bit counter of 1 us)] */
#define TIMER_1US_MIN_DELAY_US         20
#define TIMER_1US_MAX_DELAY_US         (35 * 60 * 1000000)

#if (!PIC32CX)
/* 16-bit counter */
typedef uint16_t tc_cv_t;
# define TIMER_1US_HALF_LOOP_CYCLES    (1UL << 15)
# define TIMER_1US_TC_CONFIG(tcclks)   ul_tcclks | TC_CMR_WAVE | TC_CMR_EEVT_XC0
#else
/* 32-bit counter */
typedef uint32_t tc_cv_t;
# define TIMER_1US_HALF_LOOP_CYCLES    (1UL << 31)
# define TIMER_1US_TC_CONFIG(tcclks)   ul_tcclks | TC_CMR_WAVE | TC_CMR_EEVT_XC0, 0
#endif

/* Enumeration of time interrupt states */
typedef enum time_int_state {
	/* No TC interrupt assigned */
	TIME_INT_REG_NONE,
	/* RA compare TC interrupt assigned */
	TIME_INT_REG_RA,
	/* RB compare TC interrupt assigned */
	TIME_INT_REG_RB,
} time_int_state_t;

/* Definition of callback handler */
typedef void (*pf_time_int_handler)(uint32_t);

/* Struct of time interrupt descriptor */
typedef struct time_int_desc {
	pf_time_int_handler p_handler;
	uint32_t ul_int_time_us;
	uint32_t ul_int_id;
	tc_cv_t us_tc_comp_value;
	time_int_state_t uc_int_state;
} time_int_desc_t;

/* Array of interrupt descriptors (queue, ordered by remaining time) */
static time_int_desc_t spx_time_int_queue[TIMER_1US_MAX_INTERRUPTS];

/* Accumulated time of complete TC loops in us [uQ32.32] */
static uint64_t sull_time_acc_us_q32;

/* Time in us of 1 TC loop (2^16 or 2^32 cycles) [uQ32.32] and half [uQ32.0] */
static uint64_t sull_tc_cv_loop_us_q32;
static uint32_t sul_tc_cv_half_loop_us;

/* Frequency used by TC in MHz [uQ5.27] and period in us [uQ1.31] */
static uint32_t sul_tc_freq_mhz_q27;
static uint32_t sul_tc_period_us_q31;

/* Critical section control (enable/disable IRQ) */
static uint32_t sul_tc_irq_crit_sect_count;

/* Last TC counter value */
static tc_cv_t sus_last_tc_cv;

/* Number of pending interrupts */
static uint8_t suc_time_int_pending;

/* Time interrupt identifier (from 0 to 0xFFFFFFFF) */
static uint32_t sul_time_int_id;

/**
 * \brief Convert cycles of TC to us [uQ17.32]
 *
 * \param ul_tc_cycles TC cycles [uQ32.0] (maximum is 0xFFFFFFFF)
 *
 * \return Time in us [uQ17.32]
 */
__always_inline static uint64_t _tc_cycles_to_us_q32(uint32_t ul_tc_cycles)
{
	/* T_us [uQ32.32] = Cycles [uQ32.0] * TcPeriod_us [uQ0.31] * 2 */
	return ((uint64_t)ul_tc_cycles * sul_tc_period_us_q31) << 1;
}

/**
 * \brief Convert time in us [uQ32.0] to us [uQ32.32]
 *
 * \param ul_time_us Time in us [uQ32.0]
 *
 * \return Time in us [uQ32.32]
 */
__always_inline static uint64_t _time_us_to_us_q32(uint32_t ul_time_us)
{
	return ((uint64_t)ul_time_us << 32);
}

/**
 * \brief Convert time in us [uQ32.32] to us [uQ32.0]
 *
 * \param ull_time_us_q32 Time in us [uQ32.32]
 *
 * \return Time in us [uQ32.0]
 */
__always_inline static uint32_t _time_us_q32_to_us(uint64_t ull_time_us_q32)
{
	return (uint32_t)((ull_time_us_q32 + (1UL << 31)) >> 32);
}

/**
 * \brief Convert time in us [uQ32.0] to TC cycles [16.0]
 *
 * \param ul_time_us Absolute time in us [uQ32.0]
 *
 * \return TC cycles [uQ32.0] (0 to 0xFFFFFFFF)
 */
__always_inline static tc_cv_t _time_us_to_tc_cycles(uint32_t ul_time_us)
{
	uint64_t ull_cycles_aux;
	uint64_t ull_delay_us_q32;
	uint32_t ul_delay_us;

	/* Delay since last TC loop [uQ32.0] */
	ull_delay_us_q32 = _time_us_to_us_q32(ul_time_us) - sull_time_acc_us_q32;
	ul_delay_us = _time_us_q32_to_us(ull_delay_us_q32);

	/* Cycles [37.27] =  Delay_us[uQ32.0] * F_MHz [uQ5.27] */
	ull_cycles_aux = (uint64_t)ul_delay_us * sul_tc_freq_mhz_q27;
	ull_cycles_aux = (ull_cycles_aux + (1UL << 26)) >> 27;

	return (tc_cv_t)ull_cycles_aux;
}

/**
 * \brief Read TC Counter value, check overflow to update accumulated time in us
 * [uQ32.32] and return current time in us [uQ32.0]
 *
 * \return Current time in us [uQ32.0]
 */
__always_inline static uint32_t _current_time_us(void)
{
	uint64_t ull_time_us_q32;
	tc_cv_t us_tc_cv;

	/* Read TC timer value */
	us_tc_cv = (tc_cv_t)tc_read_cv(TIMER_1US_TC, TIMER_1US_TC_CHN);

	if (us_tc_cv < sus_last_tc_cv) {
		/* CV overflow: Accum 1 loop to total time in us [uQ32.32].
		 * Since COVFS and CPCS (half loop) interrupts are enabled, it
		 * is assumed that CV will be read at least twice every TC loop
		 * (<=65536 ms) */
		sull_time_acc_us_q32 += sull_tc_cv_loop_us_q32;
	}

	/* Update CV for next read */
	sus_last_tc_cv = us_tc_cv;

	/* Compute current time in us [uQ32.32] */
	ull_time_us_q32 = _tc_cycles_to_us_q32(us_tc_cv) + sull_time_acc_us_q32;

	/* Convert current timer to us [uQ32.0] */
	return _time_us_q32_to_us(ull_time_us_q32);
}

/**
 * \brief Try to assign a TC compare interrupt for the specified counter value,
 * if there is one available (RA/RB)
 *
 * \param us_tc_comp_value TC counter value to be compared with RA/RB/RC
 *
 * \return Time interrupt status result (time_int_state_t)
 */
static inline time_int_state_t _tc_assign_int(tc_cv_t us_tc_comp_value)
{
	uint32_t ul_tc_int_mask;
	time_int_state_t uc_state_result;

	/* Get TC interrupt mask to check available TC compare interrupts */
	ul_tc_int_mask = tc_get_interrupt_mask(TIMER_1US_TC, TIMER_1US_TC_CHN);

	/* Check if RA compare interrupt is enabled and with the same value */
	if (ul_tc_int_mask & TC_IMR_CPAS) {
		tc_cv_t us_tc_ra;
		us_tc_ra = (tc_cv_t)tc_read_ra(TIMER_1US_TC, TIMER_1US_TC_CHN);
		if (us_tc_ra == us_tc_comp_value) {
			return TIME_INT_REG_RA;
		}
	}

	/* Check if RB compare interrupt is enabled and with the same value */
	if (ul_tc_int_mask & TC_IMR_CPBS) {
		tc_cv_t us_tc_rb;
		us_tc_rb = (tc_cv_t)tc_read_rb(TIMER_1US_TC, TIMER_1US_TC_CHN);
		if (us_tc_rb == us_tc_comp_value) {
			/* RB compare IRQ already enabled with same value */
			return TIME_INT_REG_RB;
		}
	}

	/* There is not an interrupt with the same counter compare value.
	 * Program a new interrupt, if there is IRQ available (RA/RB) */
	uc_state_result = TIME_INT_REG_NONE;

	if (!(ul_tc_int_mask & TC_IMR_CPAS)) {
		/* RA compare interrupt is available */
		tc_write_ra(TIMER_1US_TC, TIMER_1US_TC_CHN, us_tc_comp_value);
		tc_enable_interrupt(TIMER_1US_TC, TIMER_1US_TC_CHN, TC_IER_CPAS);
		uc_state_result = TIME_INT_REG_RA;
	} else if (!(ul_tc_int_mask & TC_IMR_CPBS)) {
		/* RB compare interrupt is available */
		tc_write_rb(TIMER_1US_TC, TIMER_1US_TC_CHN, us_tc_comp_value);
		tc_enable_interrupt(TIMER_1US_TC, TIMER_1US_TC_CHN, TC_IER_CPBS);
		uc_state_result = TIME_INT_REG_RB;
	}

	return uc_state_result;
}

/**
 * \brief TC interrupt handler.
 */
void TIMER_1US_TC_Handler(void)
{
	pf_time_int_handler ppf_handlers[TIMER_1US_MAX_INTERRUPTS];
	uint32_t pul_int_times[TIMER_1US_MAX_INTERRUPTS];
	uint32_t pul_int_ids[TIMER_1US_MAX_INTERRUPTS];
	uint32_t ul_current_time_us;
	uint32_t ul_tc_status;
	uint32_t ul_basepri_prev;
	uint8_t uc_int_expired;

	/* Enter critical region. Disable all interrupts except highest priority
	 * (<2: 0, 1) to avoid conflicts from higher priority interrupts */
	ul_basepri_prev = __get_BASEPRI();
	if (ul_basepri_prev != (1 << (8 - __NVIC_PRIO_BITS))) {
		__set_BASEPRI(2 << (8 - __NVIC_PRIO_BITS));
	}

	/* Get status to check the RA/RB compare interrupts */
	ul_tc_status = tc_get_status(TIMER_1US_TC, TIMER_1US_TC_CHN);

	/* Get current time in us [uQ32.0] */
	ul_current_time_us = _current_time_us();

	/* Check RA compare flag */
	if (ul_tc_status & TC_SR_CPAS) {
		tc_cv_t us_tc_ra;
		tc_cv_t us_tc_ra_diff;

		/* Check if TC counter value has actually reached RA value */
		us_tc_ra = (tc_cv_t)tc_read_ra(TIMER_1US_TC, TIMER_1US_TC_CHN);
		us_tc_ra_diff = sus_last_tc_cv - us_tc_ra;
		if (us_tc_ra_diff < TIMER_1US_HALF_LOOP_CYCLES) {
			tc_disable_interrupt(TIMER_1US_TC, TIMER_1US_TC_CHN, TC_IDR_CPAS);
		}
	}

	/* Check RB compare flag */
	if (ul_tc_status & TC_SR_CPBS) {
		tc_cv_t us_tc_rb;
		tc_cv_t us_tc_rb_diff;

		/* Check if TC counter value has actually reached RB value */
		us_tc_rb = (tc_cv_t)tc_read_rb(TIMER_1US_TC, TIMER_1US_TC_CHN);
		us_tc_rb_diff = sus_last_tc_cv - us_tc_rb;
		if (us_tc_rb_diff < TIMER_1US_HALF_LOOP_CYCLES) {
			tc_disable_interrupt(TIMER_1US_TC, TIMER_1US_TC_CHN, TC_IDR_CPBS);
		}
	}

	/* For each pending time interrupt, check if it expired or program TC
	 * interrupt if remaining time is less than half loop */
	uc_int_expired = 0;
	for (uint8_t uc_i = 0; uc_i < suc_time_int_pending; uc_i++) {
		int32_t sl_delay_time_us;
		time_int_desc_t *px_int_desc = &spx_time_int_queue[uc_i];

		/* Enter critical region. Disable all interrupts except highest
		 * priority (<1: 0) to avoid delays in current time */
		__set_BASEPRI(1 << (8 - __NVIC_PRIO_BITS));

		/* Remaining time for programmed interrupt */
		ul_current_time_us = _current_time_us();
		sl_delay_time_us = (int32_t)px_int_desc->ul_int_time_us - ul_current_time_us;

		if (sl_delay_time_us < TIMER_1US_MIN_DELAY_US) {
			/* Time expired. If TC interrupt was not programmed yet,
			 * it's late. Add callback to array of notifications */
			ppf_handlers[uc_int_expired] = px_int_desc->p_handler;
			pul_int_ids[uc_int_expired] = px_int_desc->ul_int_id;
			pul_int_times[uc_int_expired] = px_int_desc->ul_int_time_us;
			uc_int_expired++;
		} else if (px_int_desc->uc_int_state == TIME_INT_REG_NONE) {
			/* No TC interrupt assigned yet */
			if ((uint32_t)sl_delay_time_us < sul_tc_cv_half_loop_us) {
				/* RemainingTime < HalfLoopTC: Assign TC IRQ (if
				 * available, otherwise assign later) */
				px_int_desc->uc_int_state = _tc_assign_int(px_int_desc->us_tc_comp_value);
			}
		}

		/* Current time used. Allow interrupts with prio 0/1. Other ones
		 * not allowed to avoid conflicts from higher prio interrupts */
		if (ul_basepri_prev != (1 << (8 - __NVIC_PRIO_BITS))) {
			__set_BASEPRI(2 << (8 - __NVIC_PRIO_BITS));
		}
	}

	/* Update pending interrupt queue */
	suc_time_int_pending -= uc_int_expired;
	if ((suc_time_int_pending != 0) && (uc_int_expired != 0)) {
		time_int_desc_t *px_src = &spx_time_int_queue[uc_int_expired];
		time_int_desc_t *px_dest = &spx_time_int_queue[0];
		memmove(px_dest, px_src, sizeof(time_int_desc_t) * suc_time_int_pending);
	}

	/* Leave critical region */
	__set_BASEPRI(ul_basepri_prev);

	/* Notify expired interrupts */
	for (uint8_t uc_i = 0; uc_i < uc_int_expired; uc_i++) {
		uint32_t ul_timeout;
		uint32_t ul_int_time_us;
		int32_t sl_delay_time_us;

		/* Check if current time is actually same or later than desired
		 * interrupt time */
		ul_int_time_us = pul_int_times[uc_i];
		sl_delay_time_us = (int32_t)ul_current_time_us - ul_int_time_us;
		ul_timeout = 5000;

		while ((sl_delay_time_us < 0) && (ul_timeout > 0)) {
			/* Update current time. timer_1us_get() used to disable
			 * interrupts and avoid conficts */
			ul_current_time_us = timer_1us_get();
			sl_delay_time_us = (int32_t)ul_current_time_us - ul_int_time_us;
			ul_timeout--;
		}

		/* Call hanlder */
		ppf_handlers[uc_i](pul_int_ids[uc_i]);
	}
}

/**
 * \brief Timer of 1us service initialization.
 */
void timer_1us_init(void)
{
	uint64_t ull_tc_period_aux;
	uint64_t ull_tc_half_loop_aux;
	uint32_t ul_tcclks;

	/* Initialize static variables */
	sull_time_acc_us_q32 = 0;
	sus_last_tc_cv = 0;
	sul_tc_irq_crit_sect_count = 0;
	suc_time_int_pending = 0;
	sul_time_int_id = 0;

	/* Get TC clock source (TCCLKS) and exact TC freq in MHz [uQ5.27] */
	ul_tcclks = timer_1us_hal_init(&sul_tc_freq_mhz_q27);
	/* Compute TC period (1 cycle) in us [uQ1.31] */
	ull_tc_period_aux = div_round(1ULL << 58, sul_tc_freq_mhz_q27);
	sul_tc_period_us_q31 = (uint32_t)min(ull_tc_period_aux, UINT32_MAX);

	/* Compute time in us of one [uQ32.32] / half [uQ32.0] TC loop */
	ull_tc_half_loop_aux = _tc_cycles_to_us_q32(TIMER_1US_HALF_LOOP_CYCLES);
	sull_tc_cv_loop_us_q32 = ull_tc_half_loop_aux << 1;
	sul_tc_cv_half_loop_us = _time_us_q32_to_us(ull_tc_half_loop_aux);

	/* Enable TC peripheral clock */
	sysclk_enable_peripheral_clock(TIMER_1US_ID_TC);

	/* Initialize the TC peripheral. Waveform mode must be used in order to
	 * use RA/RB compare interrupts. External event must be different to
	 * TIOB in order to use RB compare interrupt  */
	tc_init(TIMER_1US_TC, TIMER_1US_TC_CHN, TIMER_1US_TC_CONFIG(ul_tcclks));

	/* Enable COVFS (counter overlow) and CPCS interrupts to ensure that two
	 * interrupts occur every TC loop and overflow is detected always */
	tc_write_rc(TIMER_1US_TC, TIMER_1US_TC_CHN, TIMER_1US_HALF_LOOP_CYCLES);
	tc_enable_interrupt(TIMER_1US_TC, TIMER_1US_TC_CHN,
			TC_IER_COVFS | TC_IER_CPCS);

	/* Enable TC interrupt and set priority */
	NVIC_ClearPendingIRQ((IRQn_Type)TIMER_1US_ID_TC);
	NVIC_SetPriority((IRQn_Type)TIMER_1US_ID_TC, TIMER_1US_PRIO);
	NVIC_EnableIRQ((IRQn_Type)TIMER_1US_ID_TC);

	/* Start the timer */
	tc_start(TIMER_1US_TC, TIMER_1US_TC_CHN);
}

/**
 * \brief Enable/Disable TC interrupt
 *
 * \param b_enable Enable (true) or disable (false)
 */
void timer_1us_enable_interrupt(bool b_enable)
{
	if (b_enable) {
		/* Leave critical section */
		if (sul_tc_irq_crit_sect_count > 0) {
			/* Decrement critical section counter */
			sul_tc_irq_crit_sect_count--;
		}

		if (sul_tc_irq_crit_sect_count == 0) {
			/* Enable IRQ in NVIC */
			NVIC_EnableIRQ((IRQn_Type)TIMER_1US_ID_TC);
		}
	} else {
		/* Enter critical section: Disable IRQ in NVIC */
		NVIC_DisableIRQ((IRQn_Type)TIMER_1US_ID_TC);

		/* Increment critical section counter */
		sul_tc_irq_crit_sect_count++;
	}
}

/**
 * \brief Get current time in us.
 *
 * \return Current time in us, referred to internal 32-bit counter of 1 us
 */
uint32_t timer_1us_get(void)
{
	uint32_t ul_timer_us;
	uint32_t ul_basepri_prev;

	/* Enter critical region. Disable all interrupts except highest priority
	 * (<2: 0, 1) to avoid conflicts from higher priority interrupts */
	ul_basepri_prev = __get_BASEPRI();
	if (ul_basepri_prev != (1 << (8 - __NVIC_PRIO_BITS))) {
		__set_BASEPRI(2 << (8 - __NVIC_PRIO_BITS));
	}

	/* Get current time in us */
	ul_timer_us = _current_time_us();

	/* Leave critical region */
	__set_BASEPRI(ul_basepri_prev);

	return ul_timer_us;
}

/**
 * \brief Set an interrupt for the specified time in us. Delay from current time
 * must be between 20 us and 35.8 minutes (half loop of 32-bit counter of 1 us).
 * The handler will be called from TC interrupt (TIMER_1US_PRIO). It could be
 * delayed if interrupt is disabled or because of another interrupt with higher
 * priority
 *
 * \param[in] ul_time_us Absolute/Relative time in us
 * \param[in] b_relative Speficied time is absolute time, referred to to
 * internal 32-bit counter of 1 us (false) or relative to the current time
 * (true)
 * \param[in] p_handler Callback handler to call at specified time
 * \param[out] pul_int_id Time interrupt identifier. Same value will be used as
 * parameter of handler function. It can also be used to cancel a programmed
 * interrupt
 *
 * \return Result: Interrupt programmed (true) or invalid parameters / queue
 * full (false)
 */
bool timer_1us_set_int(uint32_t ul_time_us, bool b_relative, void (*p_handler)(uint32_t), uint32_t *pul_int_id)
{
	uint32_t ul_delay_time_us;
	int32_t sl_delay_time_us;
	uint32_t ul_current_time_us;
	uint32_t ul_prog_time_us;
	uint32_t ul_basepri_prev;
	tc_cv_t us_tc_comp_value;
	time_int_state_t uc_int_state;
	uint8_t uc_int_idx;
	uint8_t uc_num_next_ints;

	if (suc_time_int_pending >= TIMER_1US_MAX_INTERRUPTS) {
		/* Time interrupt queue full: no more interrupts allowed */
		return false;
	} else if (p_handler == NULL) {
		return false;
	}

	/* Enter critical region. Disable all interrupts except highest priority
	 * (<1: 0) to avoid delays in computations with current time */
	ul_basepri_prev = __get_BASEPRI();
	__set_BASEPRI(1 << (8 - __NVIC_PRIO_BITS));

	/* Get current time in us */
	ul_current_time_us = _current_time_us();

	if (b_relative) {
		/* Relative Time mode */
		ul_prog_time_us = ul_current_time_us + ul_time_us;
	} else {
		/* Absolute Time mode */
		ul_prog_time_us = ul_time_us;
	}

	/* Delay between current time and programmed time */
	sl_delay_time_us = (int32_t)ul_prog_time_us - ul_current_time_us;

	/* Check if delay is valid. If time is in the past (or more than 35.8
	 * minutes, half loop of 32-bit counter of 1 us), return error */
	if (sl_delay_time_us < 0) {
		/* Leave critical region */
		__set_BASEPRI(ul_basepri_prev);
		return false;
	}

	/* If delay is less than minimum (20 us), adjust programmed time */
	ul_delay_time_us = (uint32_t)sl_delay_time_us;
	if (ul_delay_time_us < TIMER_1US_MIN_DELAY_US) {
		ul_prog_time_us = ul_prog_time_us - ul_delay_time_us + TIMER_1US_MIN_DELAY_US;
		ul_delay_time_us = TIMER_1US_MIN_DELAY_US;
	}

	/* Find the index to insert the interrupt descriptor. Array queue sorted
	 * in ascending order (remaining time) */
	uc_int_idx = suc_time_int_pending;
	for (uint8_t uc_i = 0; uc_i < suc_time_int_pending; uc_i++) {
		uint32_t ul_int_time_us = spx_time_int_queue[uc_i].ul_int_time_us;
		int32_t sl_time_diff = (int32_t)(ul_prog_time_us - ul_int_time_us);
		if (sl_time_diff < 0) {
			uc_int_idx = uc_i;
			break;
		}
	}

	/* Compute the TC counter value that will be used for TC interrupt
	 * (it can be for a loop in the future loop, but we compute it now) */
	us_tc_comp_value = _time_us_to_tc_cycles(ul_prog_time_us);

	/* Number of pending interrupts with time later than this one */
	uc_num_next_ints = suc_time_int_pending - uc_int_idx;

	if (ul_delay_time_us < sul_tc_cv_half_loop_us) {
		/* Delay < HalfLoopTC: Try to assign TC interrupt */
		uc_int_state = _tc_assign_int(us_tc_comp_value);

		/* If there are not available TC interrupts, check if there is a
		 * programmed interrupt for a time later than this one */
		if (uc_int_state == TIME_INT_REG_NONE) {
			uint8_t uc_int_idx_next = suc_time_int_pending - 1;
			for (uint8_t uc_i = 0; uc_i < uc_num_next_ints; uc_i++) {
				time_int_state_t uc_int_state_next;
				uc_int_state_next = spx_time_int_queue[uc_int_idx_next--].uc_int_state;
				if (uc_int_state_next == TIME_INT_REG_RA) {
					/* Reassign RA compare interrupt */
					tc_write_ra(TIMER_1US_TC, TIMER_1US_TC_CHN, us_tc_comp_value);
					uc_int_state = TIME_INT_REG_RA;
					break;
				} else if (uc_int_state_next == TIME_INT_REG_RB) {
					/* Reassign RB compare interrupt */
					tc_write_rb(TIMER_1US_TC, TIMER_1US_TC_CHN, us_tc_comp_value);
					uc_int_state = TIME_INT_REG_RB;
					break;
				}
			}

			/* If a TC compare interrupt was reassigned to this one,
			 * change the status of affected pending interrupts */
			if (uc_int_state != TIME_INT_REG_NONE) {
				for (uint8_t uc_i = uc_int_idx; uc_i < suc_time_int_pending; uc_i++) {
					if (spx_time_int_queue[uc_i].uc_int_state == uc_int_state) {
						spx_time_int_queue[uc_i].uc_int_state = TIME_INT_REG_NONE;
					}
				}
			}
		}
	} else {
		/* Delay > one TC loop: TC interrupt not programmed yet */
		uc_int_state = TIME_INT_REG_NONE;
	}

	/* TC IRQ programmed if needed. Allow interrupts with prio 0/1. Other
	 * ones not allowed to avoid conflicts from higher prio interrupts */
	if (ul_basepri_prev != (1 << (8 - __NVIC_PRIO_BITS))) {
		__set_BASEPRI(2 << (8 - __NVIC_PRIO_BITS));
	}

	/* Move pending interrupts in the queue to insert this one */
	if (uc_num_next_ints != 0) {
		time_int_desc_t *px_src = &spx_time_int_queue[uc_int_idx];
		time_int_desc_t *px_dest = &spx_time_int_queue[uc_int_idx + 1];
		memmove(px_dest, px_src, sizeof(time_int_desc_t) * uc_num_next_ints);
	}

	/* Insert new time interrupt descriptor in the queue */
	spx_time_int_queue[uc_int_idx].p_handler = p_handler;
	spx_time_int_queue[uc_int_idx].ul_int_time_us = ul_prog_time_us;
	spx_time_int_queue[uc_int_idx].ul_int_id = sul_time_int_id;
	spx_time_int_queue[uc_int_idx].us_tc_comp_value = us_tc_comp_value;
	spx_time_int_queue[uc_int_idx].uc_int_state = uc_int_state;
	*pul_int_id = sul_time_int_id;
	suc_time_int_pending++;
	sul_time_int_id++;

	/* Leave critical region */
	__set_BASEPRI(ul_basepri_prev);

	return true;
}

/**
 * \brief Cancel a programmed time interrupt
 *
 * \param ul_int_id Time interrupt identifier
 *
 * \return Result: Interrupt cancelled (true) or not pending interrupt with
 * specified identifier (false)
 */
bool timer_1us_cancel_int(uint32_t ul_int_id)
{
	uint32_t ul_basepri_prev;
	uint8_t uc_int_idx = 0;
	uint8_t uc_num_next_ints;
	time_int_state_t uc_int_state;
	bool b_int_id_found;

	/* Enter critical region. Disable all interrupts except highest priority
	 * (<2: 0, 1) to avoid conflicts from higher priority interrupts */
	ul_basepri_prev = __get_BASEPRI();
	if (ul_basepri_prev != (1 << (8 - __NVIC_PRIO_BITS))) {
		__set_BASEPRI(2 << (8 - __NVIC_PRIO_BITS));
	}

	/* Find pending interrupt with specified identifier in the queue */
	b_int_id_found = false;
	for (uint8_t uc_i = 0; uc_i < suc_time_int_pending; uc_i++) {
		if (spx_time_int_queue[uc_i].ul_int_id == ul_int_id) {
			uc_int_idx = uc_i;
			b_int_id_found = true;
			break;
		}
	}

	/* If interrupt identifier not found, return error */
	if (!b_int_id_found) {
		/* Leave critical region */
		__set_BASEPRI(ul_basepri_prev);
		return false;
	}

	/* State of time interrupt to cancel */
	uc_int_state = spx_time_int_queue[uc_int_idx].uc_int_state;

	/* Move pending interrupts in the queue to remove this one */
	uc_num_next_ints = suc_time_int_pending - uc_int_idx - 1;
	if (uc_num_next_ints != 0) {
		time_int_desc_t *px_src = &spx_time_int_queue[uc_int_idx + 1];
		time_int_desc_t *px_dest = &spx_time_int_queue[uc_int_idx];
		memmove(px_dest, px_src, sizeof(time_int_desc_t) * uc_num_next_ints);
	}

	/* Update pending interrupts */
	suc_time_int_pending--;

	/* If A TC compare interrupt was already assigned to this interupt,
	 * check if there is another interrupt using the same TC register to
	 * disable TC interrupt */
	if (uc_int_state != TIME_INT_REG_NONE) {
		bool b_disable_int = true;
		for (uint8_t uc_i = 0; uc_i < suc_time_int_pending; uc_i++) {
			if (spx_time_int_queue[uc_i].uc_int_state == uc_int_state) {
				b_disable_int = false;
				break;
			}
		}

		if (b_disable_int) {
			if (uc_int_state == TIME_INT_REG_RA) {
				/* Disable TC RA compare interrupt */
				tc_disable_interrupt(TIMER_1US_TC, TIMER_1US_TC_CHN, TC_IDR_CPAS);
			}

			if (uc_int_state == TIME_INT_REG_RB) {
				/* Disable TC RB compare interrupt */
				tc_disable_interrupt(TIMER_1US_TC, TIMER_1US_TC_CHN, TC_IDR_CPBS);
			}
		}
	}

	/* Leave critical region */
	__set_BASEPRI(ul_basepri_prev);

	return true;
}

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* @endcond */
