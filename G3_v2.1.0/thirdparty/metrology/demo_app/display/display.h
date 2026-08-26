/**
 * \file
 *
 * \brief Display Module Header file.
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

#ifndef DISPLAY_H_INCLUDED
#define DISPLAY_H_INCLUDED

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

#include "compiler.h"
#include "status_codes.h"

#define SOFTWARE_VERSION        (0x100201)      /* U201 */

typedef enum {
	DISPLAY_TOTAL_ENERGY = 0,
	DISPLAY_TOU1_ENERGY,
	DISPLAY_TOU2_ENERGY,
	DISPLAY_TOU3_ENERGY,
	DISPLAY_TOU4_ENERGY,
	DISPLAY_RTC_TIME,
	DISPLAY_RTC_DATE,
	DISPLAY_VA_RMS,
	DISPLAY_VB_RMS,
	DISPLAY_VC_RMS,
	DISPLAY_IA_RMS,
	DISPLAY_IB_RMS,
	DISPLAY_IC_RMS,
	DISPLAY_TOTAL_MAX_DEMAND,
	DISPLAY_TOU1_MAX_DEMAND,
	DISPLAY_TOU2_MAX_DEMAND,
	DISPLAY_TOU3_MAX_DEMAND,
	DISPLAY_TOU4_MAX_DEMAND,
	DISPLAY_APP_INFO,
	DISPLAY_BOARD_ID,
	DISPLAY_VERSION,
	DISPLAY_MAX_TYPE,
} display_info_t;

typedef struct {
	uint32_t timer_ref;        // Timer reference value for changing the info in loop mode. Each unit is equal to 1 second
	uint32_t timer;            // Internal Time counter
	display_info_t info;
	uint32_t direction;        // Circular display direction. Forward or backward
} display_t;

typedef enum
{
    FORWARD                     = 0x05,
    BACKWARD	                = 0x50,
} DISPLAY_CONST;

extern display_t VDisplay;

status_code_t DisplayInit(void);
void DisplayProcess(void);
void DisplayChangeInfo(void);
void DisplaySetAppInfo(char *msg, uint8_t len);
void DisplaySetTimerLoop(uint32_t time_sec);
void DisplayAddLoopInfo(display_info_t info);

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */

#endif /* DISPLAY_H_INCLUDED */
