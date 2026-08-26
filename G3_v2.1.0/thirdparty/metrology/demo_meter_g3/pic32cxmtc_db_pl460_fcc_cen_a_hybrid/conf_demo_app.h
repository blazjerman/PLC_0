/**
 * \file
 *
 * \brief Demo Application configuration.
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

#ifndef CONF_DEMO_APP_H_INCLUDED
#define CONF_DEMO_APP_H_INCLUDED

/* Enable ICM demonstration: System checks that the metrology library binary (Flash memory) matches with IRAM1, and monitors continously the IRAM1 */
#define ICM_MONITORING              1

/* Enable checking Metrology Update Timer */
/* #define ENABLE_CHECK_METUPD_TIMER */

/* Activate Demo Application Debug */
/* #define APP_DEMO_DEBUG_CONSOLE */

/* Enable VBAT */
/* #define CONF_APP_VBAT */

/* Energy threshold to update energy register stored in external memory */
#define ENERGY_TOU_MEM_THRESHOLD        10000 // 0.01kWh (Units: 10^-4 Wh)

#define APP_DEMO_GPBR_REG               GPBR8

/* Configure TIME between History data Updates in MINUTES : each 15 minutes by default */
#define CONF_APP_TIME_HISTORY_UPD       15

/* Configure DISPLAY_BOARD_ID */
#define DISPLAY_BOARD_VERSION           (0x0001)

/* Configure MIKROBUS_PIN_AN(PA31) as a general DEBUG output pin for Core0 or Core1 use */
#define USE_MIKROBUS_PIN_AN_AS_OUTPUT
#define USE_MIKROBUS_PIN_AN_FOR_END_OF_INTEGRATION_TOGGLE

/* Configure MIKROBUS_PIN_PWM as a general DEBUG output pin for Core0 or Core1 use */
/* This functionality is not available in case G3 Hybrid Profile is used, port is already in use */
/* #define USE_MIKROBUS_PIN_PWM_AS_OUTPUT */

/* Configure MIKROBUS_PIN_RST as output for Raw Zero-Crossings or debug purposes */
/* This functionality is not available in case G3 Hybrid Profile is used, port is already in use */
/* #define USE_MIKROBUS_PIN_RST_AS_OUTPUT */
/* #define USE_MIKROBUS_PIN_RST_FOR_RZC_TOGGLE */
/* #define USE_MIKROBUS_PIN_RST_FOR_RZC_SQUARE_OUT */

#endif /* CONF_DEMO_APP_H_INCLUDED */
