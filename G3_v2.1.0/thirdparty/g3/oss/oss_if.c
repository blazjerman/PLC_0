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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifdef _G3_SIM_
#include "oss_if.h"
#include "led.h"
#else
/* asf includes */
#include "asf.h"
#endif

#include "board.h"
#include "hal/hal.h"

/* OSS includes */
#include "conf_hal.h"
#include "conf_usi.h"
#include "conf_project.h"

#ifdef OSS_ENABLE_IPv6_STACK_SUPPORT
	#include "core/net.h"
#endif

#ifdef OSS_USE_FREERTOS
/* Kernel includes. */
	#include "FreeRTOS.h"
	#include "task.h"
	#include "timers.h"
#endif

#ifdef OSS_G3_ADP_MAC_SUPPORT
	#include "AdpApi.h"
#endif

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

/* Global milliseconds counter */
static uint32_t g_ms_counter = 0;

/* Global 100 ms counter */
static uint32_t g_100ms_counter = 0;

/* Global 10 s counter */
static uint32_t g_10s_counter = 0;

/* User registration tasks */
#define OSS_IF_MAX_TASKS       5
static uint8_t suc_oss_num_task_registered;

static pfv_t sx_oss_init_tasks[OSS_IF_MAX_TASKS];
static pfv_t sx_oss_proc_tasks[OSS_IF_MAX_TASKS];
static pfv_t sx_oss_updt_tasks[OSS_IF_MAX_TASKS];
static pfv_t sx_oss_proc_prio_tasks[OSS_IF_MAX_TASKS];

static void _oss_execute_tasks(enum oss_task_type ttype)
{
	uint8_t uc_idx;
	pfv_t *pfv = NULL;

	if (suc_oss_num_task_registered) {
		/* Select array of functions to execute */
		switch (ttype) {
		case OSS_TASK_INIT:
			pfv = sx_oss_init_tasks;
			break;

		case OSS_TASK_PROCESS:
			pfv = sx_oss_proc_tasks;
			break;

		case OSS_TASK_UPDATE:
			pfv = sx_oss_updt_tasks;
			break;

		case OSS_TASK_PROCESS_PRIO:
			pfv = sx_oss_proc_prio_tasks;
			break;
		}

		for (uc_idx = 0; uc_idx < suc_oss_num_task_registered; uc_idx++, pfv++) {
			if ((pfv != NULL) && (*pfv != NULL)) {
				(*pfv)();
			}
		}
	}
}

static void _oss_1ms_timer_handler(void)
{
	/* update count ms */
	g_ms_counter++;

	if ((g_ms_counter % 100) == 0) {
		/* 100 ms elapsed; update counter */
		g_100ms_counter++;

		if ((g_100ms_counter % 100) == 0) {
			/* 10 s elapsed; update counter */
			g_10s_counter++;
		}
	}

#ifdef OSS_ENABLE_IPv6_STACK_SUPPORT
	/* update count ms for IPv6 stack */
	systemTicks++;
#endif

	/* Tasks 1ms timer callback, if registered */
	_oss_execute_tasks(OSS_TASK_UPDATE);
}

uint32_t oss_get_up_time_ms(void)
{
	return g_ms_counter;
}

uint32_t oss_get_up_time_100ms(void)
{
	return g_100ms_counter;
}

uint32_t oss_get_up_time_10s(void)
{
	return g_10s_counter;
}

/**
 * Register new Task in OSS
 */
int oss_register_task(oss_task_t *app_task)
{
	if (suc_oss_num_task_registered == OSS_IF_MAX_TASKS) {
		return -1;
	}

	sx_oss_init_tasks[suc_oss_num_task_registered] = app_task->task_init;
	sx_oss_proc_tasks[suc_oss_num_task_registered] = app_task->task_process;
	sx_oss_updt_tasks[suc_oss_num_task_registered] = app_task->task_1ms_timer_cb;
	sx_oss_proc_prio_tasks[suc_oss_num_task_registered] = app_task->task_process_prio;

	suc_oss_num_task_registered++;

	return 0;
}

/**
 * Initialize the HW.
 */
void oss_init(void)
{
#ifdef OSS_ENABLE_IPv6_STACK_SUPPORT
#ifdef CONF_BOARD_SDRAMC
	/* Initialize Ethernet HW, this MUST BE DONE BEFORE initializing the SDRAM!*/
	same70EthInitGpio(NULL);
#endif
#endif
#ifndef _G3_SIM_
	/* Prepare the hardware */
	platform_init_hw();
#endif
	memset(&sx_oss_init_tasks, 0, sizeof(sx_oss_init_tasks));
	memset(&sx_oss_proc_tasks, 0, sizeof(sx_oss_proc_tasks));
	memset(&sx_oss_updt_tasks, 0, sizeof(sx_oss_updt_tasks));
	memset(&sx_oss_proc_prio_tasks, 0, sizeof(sx_oss_proc_prio_tasks));

	suc_oss_num_task_registered = 0;
}

#ifdef OSS_USE_FREERTOS

/* G3 Task handler */
xTaskHandle xG3Hnd;
xTaskHandle xG3AppHnd;
/* Timers handlers */
xTimerHandle xUpdateTimer;
xTimerHandle xLEDTimer;

/* FreeRTOS utils */
void vApplicationIdleHook( void );
void vApplicationMallocFailedHook( void );
void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName );
void vApplicationTickHook( void );

/*-----------------------------------------------------------*/
void vApplicationMallocFailedHook( void )
{
	/* vApplicationMallocFailedHook() will only be called if
	 * configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	 * function that will get called if a call to pvPortMalloc() fails.
	 * pvPortMalloc() is called internally by the kernel whenever a task, queue,
	 * timer or semaphore is created.  It is also called by various parts of the
	 * demo application.  If heap_1.c or heap_2.c are used, then the size of the
	 * heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	 * FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	 * to query the size of free heap space that remains (although it does not
	 * provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	for (;;) {
		while (1) {
		}
	}
}

/*-----------------------------------------------------------*/
#ifndef _G3_SIM_
void vApplicationIdleHook( void )
{
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	 * to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	 * task.  It is essential that code added to this hook function never attempts
	 * to block in any way (for example, call xQueueReceive() with a block time
	 * specified, or call vTaskDelay()).  If the application makes use of the
	 * vTaskDelete() API function (as this demo application does) then it is also
	 * important that vApplicationIdleHook() is permitted to return to its calling
	 * function, because it is the responsibility of the idle task to clean up
	 * memory allocated by the kernel to any task that has since been deleted. */
}

#endif

/*-----------------------------------------------------------*/
void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName )
{
	(void)pcTaskName;
	(void)pxTask;

	/* Run time stack overflow checking is performed if
	 * configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	 * function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for (;;) {
		while (1) {
		}
	}
}

/*-----------------------------------------------------------*/
void vApplicationTickHook( void )
{
	/* This function will be called by each tick interrupt if
	* configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
	* added here, but the tick hook is called from an interrupt context, so
	* code must not attempt to block, and only the interrupt safe FreeRTOS API
	* functions can be used (those that end in FromISR()). */
}

#ifndef _G3_SIM_

/**
 * \internal
 * \brief Task to update internal G3 systick.
 *
 * This function must be called every 1 ms.
 *
 */
static void _update_1ms_proc(xTimerHandle pxTimer)
{
	UNUSED(pxTimer);
	taskENTER_CRITICAL();

	_oss_1ms_timer_handler();

	taskEXIT_CRITICAL();
}

/**
 * \internal
 * \brief Task to blink board LED.
 *
 */
static void _blink_led(xTimerHandle pxTimer)
{
	UNUSED(pxTimer);
	taskENTER_CRITICAL();
#if (BOARD == SAM4CMP_DB || BOARD == SAM4CMS_DB)
	LED_Toggle(LED4);
#else
	LED_Toggle(LED0);
#endif
	taskEXIT_CRITICAL();
}

#endif /* _G3_SIM_ */

/**
 * \internal
 * \brief Periodic task to process G3 App.
 *
 */
static void _g3_app_process(void *pvParameters)
{
	static portTickType xLastWakeTime;
	static portTickType xPeriod;

	/* App initialization (internally initializes G3 stack layers) */
#ifdef NUM_PORTS
	usi_init();
#endif

	/* Task-registered initialization */
	_oss_execute_tasks(OSS_TASK_INIT);

	xPeriod = G3_APP_PROCESS_EXEC_RATE;
	xLastWakeTime = xTaskGetTickCount();
	for (;;) {
		/* Task-registered PRIO processes */
		_oss_execute_tasks(OSS_TASK_PROCESS_PRIO);
		/* Task-registered processes */
		_oss_execute_tasks(OSS_TASK_PROCESS);

#ifdef NUM_PORTS
		usi_process();
#endif

#ifdef OSS_ENABLE_IPv6_STACK_SUPPORT
		netTask();
#endif

		xLastWakeTime = xTaskGetTickCount();
		vTaskDelayUntil(&xLastWakeTime, xPeriod);
	}
}

/**
 * \internal
 * \brief Periodic task to process G3. Initialize and start every layers.
 * \note Please see conf_oss file in order to configure the layers to execute.
 *
 */
static void _g3_stack_process(void *pvParameters)
{
	static portTickType xLastWakeTime;
	static portTickType xPeriod;

	UNUSED(pvParameters);

#ifndef _G3_SIM_
	/* Start timer to update counters */
	xTimerStart(xUpdateTimer, G3_UPDATE_TIMERS_RATE);
	/* Start timer to blink led */
	xTimerStart(xLEDTimer, G3_LED_PROCESS_TIMER_RATE);
#endif

	/* Initial delay on G3 process to allow for correct initialization */
	xPeriod = G3_PROCESS_INITIAL_DELAY;
	xLastWakeTime = xTaskGetTickCount();
	vTaskDelayUntil(&xLastWakeTime, xPeriod);

	xPeriod = G3_PROCESS_EXEC_RATE;
	xLastWakeTime = xTaskGetTickCount();
	for (;;) {
		/* Reset watchdog */
		RESET_WATCHDOG;

		vTaskDelayUntil(&xLastWakeTime, xPeriod);

#ifdef _G3_SIM_
		_oss_1ms_timer_handler();
#endif

#ifdef OSS_G3_ADP_MAC_SUPPORT
		/* Internally calls MAC and PHY Event handlers */
		AdpEventHandler();
#endif
	}
}

/**
 * \internal
 * \brief Create main G3 task and create timer to update internal counters.
 * \note Please see conf_oss file in order to configure layers in use.
 *
 */
void oss_start(void)
{
	/* Create new task to call processes */
	xTaskCreate(_g3_stack_process, (const signed char *const)"G3Proc",
			TASK_G3_STACK, NULL, TASK_G3_PRIO, &xG3Hnd);

	/* Create new task to call app */
	xTaskCreate(_g3_app_process, (const signed char *const)"G3AppProc",
			TASK_G3_APP, NULL, TASK_G3_APP_PRIO, &xG3AppHnd);

#ifndef _G3_SIM_
	/* Create timer to update counters */
	xUpdateTimer = xTimerCreate((const signed char *const)"UPD timer",  /* A text name, purely to help debugging. */
			G3_UPDATE_TIMERS_RATE,                                      /* The timer period. */
			pdTRUE,                                                     /* This is an auto-reload timer, so xAutoReload is set to pdTRUE. */
			NULL,                                                       /* The timer does not use its ID, so the ID is just set to NULL. */
			_update_1ms_proc                                            /* The function that is called each time the timer expires. */
			);
	/* Create timer to blink LED */
	xLEDTimer = xTimerCreate((const signed char *const)"LED blink",     /* A text name, purely to help debugging. */
			G3_LED_PROCESS_TIMER_RATE,                                  /* The timer period. */
			pdTRUE,                                                     /* This is an auto-reload timer, so xAutoReload is set to pdTRUE. */
			NULL,                                                       /* The timer does not use its ID, so the ID is just set to NULL. */
			_blink_led                                                  /* The function that is called each time the timer expires. */
			);

	configASSERT(xUpdateTimer);
	configASSERT(xLEDTimer);
#endif

#if defined(PLATFORM_PDD_INTERNAL_SUPPLY_MONITOR) || defined(PLATFORM_PDD_EXTERNAL_VOLTAGE_DIVIDER)
	/* Power down detection initialization */
	platform_init_power_down_det();
#endif

#ifdef OSS_ENABLE_IPv6_STACK_SUPPORT
	/* Initialize IPv6 stack */
	netInit();
#endif

#ifndef _G3_SIM_
	platform_led_cfg_blink_rate(OSS_LED_BLINK_RATE);
#endif

	/* Start the tasks and timers running. */
	vTaskStartScheduler();

	/* Scheduler is now running, this point is never reached */
	for (;;) {
	}
}

#else /* OSS_USE_FREERTOS */

/**
 * \internal
 * \brief Run G3 stack and App in microcontroller mode (no OS)
 * \note Please see conf_oss file in order to configure layers in use.
 *
 */
void oss_start(void)
{
	/* Set up timer interrupt and user defined callback */
	platform_set_ms_callback(&_oss_1ms_timer_handler);
	platform_led_cfg_blink_rate(OSS_LED_BLINK_RATE);
	platform_cfg_call_process_rate(OSS_G3_STACK_EXEC_RATE);
	platform_cfg_call_app_process_rate(OSS_APP_EXEC_RATE);

#if defined(PLATFORM_PDD_INTERNAL_SUPPLY_MONITOR) || defined(PLATFORM_PDD_EXTERNAL_VOLTAGE_DIVIDER)
	/* Power down detection initialization */
	platform_init_power_down_det();
#endif

#ifdef OSS_ENABLE_IPv6_STACK_SUPPORT
	/* Initialize IPv6 stack */
	netInit();
#endif

#ifdef NUM_PORTS
	/* Initialize USI */
	usi_init();
#endif

	/* Task-registered initialization */
	_oss_execute_tasks(OSS_TASK_INIT);

	/* Timers may interfere with task initialization, init after */
	platform_init_1ms_timer();

	/* main loop */
	while (1) {
		/* Reset watchdog */
		RESET_WATCHDOG;

		if (platform_flag_call_process()) {
			/* G3 stack process */
			#ifdef OSS_G3_ADP_MAC_SUPPORT
			/* Internally calls MAC and PHY Event handlers */
			AdpEventHandler();
			#endif
		}

		if (platform_flag_call_app_process()) {
			/* Task-registered processes */
			_oss_execute_tasks(OSS_TASK_PROCESS);

			#ifdef NUM_PORTS
			usi_process();
			#endif

			/* Handle TCP/IP events */
			#ifdef OSS_ENABLE_IPv6_STACK_SUPPORT
			netTask();
			#endif
		}

		/* LED blink */
		platform_led_update();
		
		/* Task-registered PRIO processes */
		_oss_execute_tasks(OSS_TASK_PROCESS_PRIO);
	}
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */

#endif
