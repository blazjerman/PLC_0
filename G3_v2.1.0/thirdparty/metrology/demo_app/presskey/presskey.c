  /**
 * \file
 *
 * \brief PressKey Module file.
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

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

#include "asf.h"

#include "presskey.h"
#include "display.h"
#include "task.h"
#include "rtcproc.h"
#include "main.h"

PRESSKEY_Str VPressKey;

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

/**
 * \brief SCROLL DOWN Interrupt Handler.
 */
static void _scroll_down_handler(uint32_t temp0, uint32_t temp1)
{
	(void)temp0;
	(void)temp1;

	VPressKey.dnkey = PRESSKEY_KEY;

	/* Simulate FAILURE detection */
	if (pio_get(PIN_PUSHBUTTON_2_PIO, PIO_INPUT, PIN_PUSHBUTTON_2_MASK) == 0) {
		Vsys.mode = BACKUP_MODE;
		/* set system stop time */
		Vsys.stop_time = VRTC;
	} else {
		/* Check Circular Display info */
		VDisplay.direction = BACKWARD;
		TaskPutIntoQueue(DisplayChangeInfo);
	}
}

/**
 * \brief SCROLL UP Interrupt Handler.
 */
static void _scroll_up_handler(uint32_t temp0, uint32_t temp1)
{
	(void)temp0;
	(void)temp1;

	VPressKey.upkey = PRESSKEY_KEY;

	/* Simulate FAILURE detection */
	if (pio_get(PIN_PUSHBUTTON_1_PIO, PIO_INPUT, PIN_PUSHBUTTON_1_MASK) == 0) {
		/* Configure PB25 as output, level high */
		ioport_set_pin_dir(MIKROBUS_PIN_INT, IOPORT_DIR_OUTPUT);
		ioport_set_pin_level(MIKROBUS_PIN_INT, IOPORT_PIN_LEVEL_HIGH);
		/* Forces WDOG reset */
		printf("\t-I- Forcing WDOG0 reset...\r\n");
		while(1);
	} else {
		/* Check Circular Display info */
		VDisplay.direction = FORWARD;
		TaskPutIntoQueue(DisplayChangeInfo);
	}
}

/**
 * \brief Configure SCROLL DOWN/UP keys.
 */
void PressKeyInit(void)
{
	NVIC_DisableIRQ(PIN_PUSHBUTTON_1_IRQn);
	NVIC_ClearPendingIRQ(PIN_PUSHBUTTON_1_IRQn);
	NVIC_SetPriority(PIN_PUSHBUTTON_1_IRQn, 1);

	NVIC_DisableIRQ(PIN_PUSHBUTTON_2_IRQn);
	NVIC_ClearPendingIRQ(PIN_PUSHBUTTON_2_IRQn);
	NVIC_SetPriority(PIN_PUSHBUTTON_2_IRQn, 1);

	/* Init SCROLL DOWN key */
	/* Adjust pio debounce filter parameters, uses 2 Hz filter. */
	pio_set_debounce_filter(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_MASK, 2);

	/* Initialize pios interrupt handlers, see PIO definition in board.h. */
	pio_handler_set(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_ID, PIN_PUSHBUTTON_1_MASK,
			PIN_PUSHBUTTON_1_ATTR, _scroll_down_handler);

	/* Enable PIO line interrupts. */
	pio_enable_interrupt(PIN_PUSHBUTTON_1_PIO, PIN_PUSHBUTTON_1_MASK);

	/* Init SCROLL UP key */
	/* Adjust pio debounce filter parameters, uses 2 Hz filter. */
	pio_set_debounce_filter(PIN_PUSHBUTTON_2_PIO, PIN_PUSHBUTTON_2_MASK, 2);

	/* Initialize pios interrupt handlers, see PIO definition in board.h. */
	pio_handler_set(PIN_PUSHBUTTON_2_PIO, PIN_PUSHBUTTON_2_ID, PIN_PUSHBUTTON_2_MASK,
			PIN_PUSHBUTTON_2_ATTR, _scroll_up_handler);

	/* Enable PIO line interrupts. */
	pio_enable_interrupt(PIN_PUSHBUTTON_2_PIO, PIN_PUSHBUTTON_2_MASK);


	NVIC_EnableIRQ(PIN_PUSHBUTTON_1_IRQn);
	NVIC_EnableIRQ(PIN_PUSHBUTTON_2_IRQn);
}

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
