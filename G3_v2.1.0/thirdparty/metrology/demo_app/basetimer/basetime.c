/**
 * \file
 *
 * \brief Meter Demo : Basetime module
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

/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include "ioport.h"
#include "main.h"
#include "metrology.h"
#include "command.h"
#include "display.h"
#include "task.h"
#include "basetime.h"

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

static volatile uint32_t sulMetUpdTimer = 0;
static volatile uint32_t sulMetUpdTimerRef = 0;

static volatile uint32_t sulAppTimer = 0;

static pf_basetimer_callback pfMetUpdCb = NULL;
static pf_basetimer_callback pfAppCb = NULL;

/**
 * \brief Systick Handler.
 */
void SysTick_Handler(void)
{
	if (sulMetUpdTimer) {
		sulMetUpdTimer--;
	}

	if (sulAppTimer) {
		sulAppTimer--;
	}
}

/**
 * \brief Base Timer Initialization.
 */
void BaseTimerInit(void)
{
	/* Configure Systick: 5ms */
	SysTick_Config(SystemCoreClock / 200);
}

/**
 * \brief Base Timer Process.
 */
void BaseTimerProcess(void)
{
	/* Check Metrology Update Timer */
	if ((sulMetUpdTimer == 0) && (pfMetUpdCb)) {
		pfMetUpdCb();
	}

	/* Check Application Timer */
	if ((sulAppTimer == 0) && (pfAppCb)) {
		pfAppCb();
	}
}

/**
 * \brief Set Timer in ms to check Metrology Update Process.
 *
 * \param value_ms Timer value in milliseconds
 */
void BaseTimerSetMetUpdTimer(uint32_t value_ms)
{
	if (value_ms) {
		sulMetUpdTimerRef = value_ms / 5;
		sulMetUpdTimer = sulMetUpdTimerRef;
	}
}

/**
 * \brief Set callback function for Metrology Update Timer.
 *
 * \param metUpdCB Pointer to the callback function to call when Metrology update timer expires
 */
void BaseTimerSetMetUpdCallback(pf_basetimer_callback metUpdCB)
{
	pfMetUpdCb = metUpdCB;
}

/**
 * \brief Restart Metrology Update Timer.
 */
void BaseTimerRestartMetUpdTimer(void)
{
	sulMetUpdTimer = sulMetUpdTimerRef;
}

/**
 * \brief Set callback function for Metrology Update Timer.
 *
 * \param appCB Pointer to the callback function to call when Metrology update timer expires
 * \param ul_time_ms  Value in milliseconds to call to the callback function.
 */
void BaseTimerSetAppCallback(pf_basetimer_callback appCB, uint32_t ul_time_ms)
{
	if (ul_time_ms) {
		sulAppTimer = (ul_time_ms/5);
		pfAppCb = appCB;
	}
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
