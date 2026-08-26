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

#ifndef TIMER_1US_H_INCLUDED
#define TIMER_1US_H_INCLUDED

#include "compiler.h"

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* @endcond */

/**
 * \defgroup timer_1us_group Timer of 1 us service
 *
 * This service provides a 32-bit counter of 1 us, based on a HW timer. It is
 * also possible to program an interrupt at specified time.
 *
 * @{
 */

/* \name TIMER_1US interrupt priority
 * \note Highest priority allowed is 2 (the lowest value is 2) */
/* @{ */
#define TIMER_1US_PRIO                 2
/* @} */

/* \name TIMER_1US maximum number of programmed interrupts from upper layers */
/* @{ */
#define TIMER_1US_MAX_INTERRUPTS       10
/* @} */

/* \name Timer of 1 us service interface
 * \note These API functions must not be called from interrupt priority higher
 * than 2, i.e. they must be called from interrupt with priority value >= 2 (or
 * not from interrupt) */
/* @{ */
void timer_1us_init(void);
uint32_t timer_1us_get(void);
void timer_1us_enable_interrupt(bool b_enable);
bool timer_1us_set_int(uint32_t ul_time_us, bool b_relative, void (*p_handler)(uint32_t), uint32_t *pul_int_id);
bool timer_1us_cancel_int(uint32_t ul_int_id);

/* @} */

/* @} */

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* @endcond */

#endif /* TIMER_1US_H_INCLUDED */
