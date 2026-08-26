/**
 * \file
 *
 * \brief Operative System Support Interface
 *
 * Copyright (c) 2013 Atmel Corporation. All rights reserved.
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

#ifndef OSS_IF_H_INCLUDED
#define OSS_IF_H_INCLUDED

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

/**
 * \weakgroup plc_group
 * @{
 */

#include "conf_oss.h"

/* ! \name G3 Stack priority */
/* @{ */
#ifndef TASK_G3_PRIO
#define TASK_G3_PRIO                    (tskIDLE_PRIORITY + 1)
#endif
/* @} */

/* ! \name G3 Stack priority */
/* @{ */
#ifndef TASK_G3_APP_PRIO
#define TASK_G3_APP_PRIO                (tskIDLE_PRIORITY + 1)
#endif
/* @} */

/* ! \name G3 Stack definition */
/* @{ */
#ifndef TASK_G3_STACK
#define TASK_G3_STACK                   (configMINIMAL_STACK_SIZE * 5)
#endif
/* @} */

/* ! \name G3 App definition */
/* @{ */
#ifndef TASK_G3_APP
#define TASK_G3_APP                     (configMINIMAL_STACK_SIZE * 5)
#endif
/* @} */

/* ! \name Main G3 Stack Update Period */
/* ! \note It must be 1 milliseconds in order to update the G3 systick */
/* @{ */
#ifndef G3_UPDATE_TIMERS_RATE
#define G3_UPDATE_TIMERS_RATE           (1 / portTICK_RATE_MS)
#endif
/* @} */

/* ! \name G3 Stack Process Initial Delay */
/* @{ */
#ifndef G3_PROCESS_INITIAL_DELAY
#define G3_PROCESS_INITIAL_DELAY        (50 / portTICK_RATE_MS)
#endif
/* @} */

/* ! \name G3 stack Process Period */
/* ! \note Defines the frequency at which the stack process will be called */
/* @{ */
#ifndef OSS_G3_STACK_EXEC_RATE
#define OSS_G3_STACK_EXEC_RATE 1
#endif

/* ! \name G3 Stack Process Period */
/* @{ */
#ifndef G3_PROCESS_EXEC_RATE
#define G3_PROCESS_EXEC_RATE           (1 / portTICK_RATE_MS)
#endif
/* @} */

/* ! \name Led Indication Period */
/* ! \note Defines the LED toggled to provide visual feedback of the system status */
/* @{ */
#define G3_LED_PROCESS_TIMER_RATE               (OSS_LED_BLINK_RATE / portTICK_RATE_MS)
/* @} */

/* ! \name App Process Period */
/* ! \note Defines the frequency at which the app process will be called */
/* @{ */
#ifndef OSS_APP_EXEC_RATE
#define OSS_APP_EXEC_RATE OSS_G3_STACK_EXEC_RATE
#endif

#define G3_APP_PROCESS_EXEC_RATE                (OSS_APP_EXEC_RATE / portTICK_RATE_MS)
/* @} */

typedef void (*pfv_t)(void);

enum oss_task_type {
	OSS_TASK_INIT,
	OSS_TASK_PROCESS,
	OSS_TASK_UPDATE,
	OSS_TASK_PROCESS_PRIO,
};

typedef struct oss_task {
	pfv_t task_init;
	pfv_t task_process;
	pfv_t task_1ms_timer_cb;
	pfv_t task_process_prio;
} oss_task_t;

/* ! \name G3 OSS interface API */
/* @{ */
uint32_t oss_get_up_time_ms(void);
uint32_t oss_get_up_time_100ms(void);
uint32_t oss_get_up_time_10s(void);

void oss_init(void);
void oss_start(void);
int oss_register_task(oss_task_t *app_task);

/* @} */
/* ! @} */

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */

#endif /* OSS_IF_H_INCLUDED */
