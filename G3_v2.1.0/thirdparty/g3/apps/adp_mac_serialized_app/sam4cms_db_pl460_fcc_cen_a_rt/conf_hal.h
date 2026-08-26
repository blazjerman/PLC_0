/**
 * \file
 *
 * \brief Platform specific configuration.
 *
 * Copyright (C) 2016 Atmel Corporation. All rights reserved.
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

#ifndef CONF_HAL_H_INCLUDED
#define CONF_HAL_H_INCLUDED

/**********************************************************************************************************************/
/************************************************ -------------------- ************************************************/
/************************************************ TIMERS CONFIGURATION ************************************************/
/************************************************ -------------------- ************************************************/
/**********************************************************************************************************************/

#define ID_TC_1MS                   ID_TC3
#define TC_1MS                      TC1
#define TC_1MS_CHN                  0
#define TC_1MS_IRQn                 TC3_IRQn
#define _platform_tc_1ms_handler    TC3_Handler

#define ID_TC_10MS                  ID_TC4
#define TC_10MS                     TC1
#define TC_10MS_CHN                 1
#define TC_10MS_IRQn                TC4_IRQn
#define TC_10MS_Handler             TC4_Handler

#define PIN_TC0_TIOA0               (PIO_PA13_IDX)
#define PIN_TC0_TIOA0_MUX           (IOPORT_MODE_MUX_B)
#define PIN_TC0_TIOA0_FLAGS         (IOPORT_MODE_MUX_B)

/**********************************************************************************************************************/
/************************************************ -------------------- ************************************************/
/************************************************    LCD SIGNALLING    ************************************************/
/************************************************ -------------------- ************************************************/
/**********************************************************************************************************************/

/* #define PLATFORM_LCD_SIGNALLING_ENABLE */
/* Define LCD text */
#define PLATFORM_LCD_TEXT     "G3 SERIAL"

/**********************************************************************************************************************/
/************************************************ -------------------- ************************************************/
/************************************************ POWER DOWN DETECTION ************************************************/
/************************************************ -------------------- ************************************************/
/**********************************************************************************************************************/

/*
 * Detect Power Down using the micro-controller internal Power Supply Monitor
 */

#define PLATFORM_PDD_INTERNAL_SUPPLY_MONITOR

/*
 * Detect Power Down using an external voltage divider connected to a pin with ADC capabilities
 */

/* #define PLATFORM_PDD_EXTERNAL_VOLTAGE_DIVIDER */

#ifdef PLATFORM_PDD_INTERNAL_SUPPLY_MONITOR

/*
 * Monitoring Rate for Supply Monitor
 */
#define CONTINUOUS_MONITORING                0x00000100
#define MONITOR_ONE_OUT_OF_32SLCK_CYCLES     0x00000200
#define MONITOR_ONE_OUT_OF_256SLCK_CYCLES    0x00000200
#define MONITOR_ONE_OUT_OF_2048SLCK_CYCLES   0x00000200

#define PLATFORM_PDD_MONITORING_RATE         CONTINUOUS_MONITORING

/*
 * Threshold for Supply Monitor
 */
#define THRESHOLD_3V40                   0x0000000F
#define THRESHOLD_3V28                   0x0000000E
#define THRESHOLD_3V16                   0x0000000D
#define THRESHOLD_3V04                   0x0000000C
#define THRESHOLD_2V92                   0x0000000B
#define THRESHOLD_2V80                   0x0000000A
#define THRESHOLD_2V68                   0x00000009
#define THRESHOLD_2V56                   0x00000008
#define THRESHOLD_2V44                   0x00000007
#define THRESHOLD_2V32                   0x00000006
#define THRESHOLD_2V20                   0x00000005
#define THRESHOLD_2V08                   0x00000004

#define PLATFORM_PDD_MONITOR_THRESHOLD   THRESHOLD_3V04

#endif

#ifdef PLATFORM_PDD_EXTERNAL_VOLTAGE_DIVIDER

/**
 * Critical threshold from which a power down is assumed by the PDD
 * Value corresponding to Power Supply = 2 v.
 */
#define PLATFORM_PDD_VDD_CRITICAL_THRESHOLD           625

/**
 * PDD's power supply monitoring frequence (Hz)
 */
#define PLATFORM_PDD_POWER_SUPPLY_MONITORING_FREQ     100

/**
 * PDD ADC channel for power supply sensing
 */
#define PLATFORM_PDD_ADC_CHANNEL_POWER_SUPPLY         ADC_CHANNEL_3

#endif

/**********************************************************************************************************************/
/************************************************ -------------------- ************************************************/
/************************************************   EUI64 AND STORAGE  ************************************************/
/************************************************ -------------------- ************************************************/
/**********************************************************************************************************************/

/**
 * Storage of G3 data configuration. Select one of the available options
 */
/* #define CONF_STORAGE_INTERNAL_FLASH */
#define CONF_STORAGE_USER_SIGNATURE

/**
 * EUI64 source. Select one of the available options
 */
#define CONF_EUI64_FROM_CHIP_ID
/* #define CONF_EUI64_FROM_USER_SIGNATURE */

#endif /* CONF_HAL_H_INCLUDED */
