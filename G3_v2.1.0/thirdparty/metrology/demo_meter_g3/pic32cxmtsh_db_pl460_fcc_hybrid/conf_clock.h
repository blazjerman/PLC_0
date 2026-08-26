/**
 * \file
 *
 * \brief PIC32CX clock configuration.
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

#ifndef CONF_CLOCK_H_INCLUDED
#define CONF_CLOCK_H_INCLUDED

/* ===== System Clock (MCK) Source Options */
/* #define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_SLCK_RC */
/* #define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_SLCK_XTAL */
/* #define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_SLCK_BYPASS */
/* #define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_MAINCK_12M_RC */
/* #define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_MAINCK_XTAL */
/* #define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_MAINCK_BYPASS */
/* #define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_PLLACK1 */
/* #define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_PLLBCK0 */
#define CONFIG_SYSCLK_SOURCE        SYSCLK_SRC_PLLBCK0

/* ===== System Clock (MCK) Prescaler Options   (Fmck = Fsys / (SYSCLK_PRES)) */
#define CONFIG_SYSCLK_PRES          SYSCLK_PRES_1
/* #define CONFIG_SYSCLK_PRES          SYSCLK_PRES_2 */
/* #define CONFIG_SYSCLK_PRES          SYSCLK_PRES_4 */
/* #define CONFIG_SYSCLK_PRES          SYSCLK_PRES_8 */
/* #define CONFIG_SYSCLK_PRES          SYSCLK_PRES_16 */
/* #define CONFIG_SYSCLK_PRES          SYSCLK_PRES_32 */
/* #define CONFIG_SYSCLK_PRES          SYSCLK_PRES_64 */
/* #define CONFIG_SYSCLK_PRES          SYSCLK_PRES_3 */

/* ===== PLLA Configuration */
/* - PLLA source: SLCK_32K_XTAL */
/* - PLLACK1 Configuration   (Fpll = (Fclk * (MUL + 1 + (FRACR / 2^22))) / (DIV1 + 1) */
/* - PLLACK0 Configuration   (Fpll = (Fclk * (MUL + 1 + (FRACR / 2^22))) / (DIV0 + 1) */
/* - PLLACK1 output: (32768 * (14499 + 1 + (0 / 2^22))) / (1 + 1) = 237.568.000 Hz */
/* - PLLACK0 output: (32768 * (14499 + 1 + (0 / 2^22))) / (57 + 1) = 8.192.000 Hz */
#define CONFIG_PLL0_SOURCE          PLLA_SRC_SLCK_32K_XTAL
#define CONFIG_PLL0_MUL             14499
#define CONFIG_PLL0_FRACR           0
#define CONFIG_PLL0_DIV1            1
#define CONFIG_PLL0_DIV0            57

/* ===== PLLB Configuration */
/* - PLLB source: PLLACK0 */
/* - PLLBCK0 Configuration Fpll = (Fclk * (MUL + 1 + (FRACR / 2^22))) / (DIV0 + 1) */
/* - PLLBCK0 output: (8192000 * (47 + 1 + (3473408 / 2^22))) / (1 + 1) = 200 MHz */
#define CONFIG_PLL1_SOURCE          PLLB_SRC_PLLACK0
#define CONFIG_PLL1_MUL             47
#define CONFIG_PLL1_FRACR           3473408
#define CONFIG_PLL1_DIV0            1

/* ===== Main processor frequency (MCK0) */
/* - System clock source: PLLBCK0 */
/* - Main pocessor system clock (MCK0): (PLLBCK0) / (CONFIG_CPCLK_PRES + 1) = 200 MHz */

/* ===== Coprocessor System Clock (CPMCK) Options */
/* Note: */
/* CONFIG_CPCLK_ENABLE  MUST be defined if using peripherals on bus matrix 1. */
#define CONFIG_CPCLK_ENABLE

/* Coprocessor System Clock Source Options */
#define CONFIG_CPCLK_SOURCE         CPCLK_SRC_PLLACK1

/* Fcpmck = Fcpclk_source / (CPCLK_PRES + 1) */
/* Coprocessor System Clock Prescaler Options (CPCLK_PRES may be 0 to 15). */
#define CONFIG_CPCLK_PRES           0

/* ===== Coprocessor frequency (CPMCK) */
/* - System clock source: PLLACK1 */
/* - Coprocessor system clock (MCK1): (PLLACK1) / (CONFIG_CPCLK_PRES + 1) = 237.568.000 Hz */

/* NOTE for METROLOGY APP: PCK Source should be selected by application.  */
/* ===== EMAFE MCLK ATSENSE (PCK2) */
/* Requirement : F_mclk_atsense = (16000 x 64 x 4) = 4096 KHz */
/* PCK2 source = PLLCSRC */
/* PCK2 Prescaler Options(PRES) = 1 (Selected clock is divided by PRES + 1) */
/* PLLCSRC = PLLACK0 by default */
/* PCK2 Output : (PLLCSRC) / (PRES + 1) = (PLLACK0) / (PRES + 1) = 4.096.000 Hz */
/* PCK2 Output : (8192000) / (1 + 1) = 4.096.000 Hz */

#endif /* CONF_CLOCK_H_INCLUDED */
