/**
 *
 * \file
 *
 * \brief Hardware Abstraction Layer Header file
 *
 * Copyright (c) 2023 Microchip Technology Inc. and its subsidiaries.
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

#ifndef HAL_H
#define HAL_H

#ifndef _G3_SIM_

#if (!PIC32CX)
#define RESET_WATCHDOG                WDT->WDT_CR = WDT_CR_KEY_PASSWD | WDT_CR_WDRSTT;
#define RSTC_RSTC_SR                  RSTC->RSTC_SR
#else
#define RESET_WATCHDOG                DWDT->WDT0_CR = WDT0_CR_KEY_PASSWD | WDT0_CR_WDRSTT; DWDT->WDT1_CR = WDT1_CR_KEY_PASSWD | WDT1_CR_WDRSTT;
#define RSTC_RSTC_SR                  RSTC->RSTC_SR
/* Internal flash page size. */
#define IFLASH_PAGE_SIZE             IFLASH0_PAGE_SIZE
/* User Signature page size. */
#define USER_SIG_PAGE_SIZE           (64) /* 8 64-bit words = 512 bytes */
#endif

#else

/* Empty definitions for simulation. */
#define RESET_WATCHDOG

#define RSTC_RSTC_SR                 0
#define RSTC_SR_RSTTYP_Msk           0
#define RSTC_SR_RSTTYP_GeneralReset  0

#define IFLASH_PAGE_SIZE             512

#define Disable_global_interrupt()
#define Enable_global_interrupt()

#define flash_read_user_signature(p_data, ul_size)
#define flash_write_user_signature(p_buffer, ul_size)
#define flash_erase_user_signature()

#endif

#include <compiler.h>

/* \name User Signature configuration parameters */
/* @{ */
#define USER_SIGNATURE_SIZE                (IFLASH_PAGE_SIZE / (sizeof(uint64_t)))
#define MACCFG_OFFSET_USER_SIGN            0
#define PHYCFG_OFFSET_USER_SIGN            16
#define G3CFG_OFFSET_USER_SIGN             32
/* @} */

#endif /* #ifndef HAL_H */

/**********************************************************************************************************************/

/** @}
 **********************************************************************************************************************/
