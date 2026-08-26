/**
 *
 * \file
 *
 * \brief RF215 IRQ handler definitions.
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

#ifndef RF215_IRQ_DEFS_H_INCLUDE
#define RF215_IRQ_DEFS_H_INCLUDE

/* RF215 includes */
#include "rf215_reg.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Starting address to read IRQ Status. Always start from RF09_IRQS. Both
 * RFn_IRQS must be read to clear IRQ pin when Wake-up interrupt is asserted
 * after Chip Reset / Power-On-Reset */
#define RF215_IRQS_READ_ADDR     RF215_ADDR_RF09_IRQS

/** Length in bytes to read IRQ Status */
#ifdef AT86RF215_DISABLE_RF24_TRX
/* Single TRX Mode (only RF09). Read 3 bytes: RF09_IRQS, RF24_IRQS, BBC0_IRQS */
# define RF215_IRQS_READ_LEN     3
#else
/* RF24 enabled (Dual TRX Mode or single TRX Mode with RF24) */
/* Read 2 bytes per TRX: RF09_IRQS, RF24_IRQS, BBC0_IRQS, BBC1_IRQS */
# define RF215_IRQS_READ_LEN     4
#endif

/** Offset in bytes to read IRQ Status */
#ifdef AT86RF215_DISABLE_RF09_TRX
/* Single TRX Mode (only RF24). Offset of 1 byte to skip RF09_IRQS */
# define RF215_IRQS_READ_OFFSET  (RF215_ADDR_RF24_IRQS - RF215_ADDR_RF09_IRQS)
#else
/* RF09 enabled (Dual TRX Mode or single TRX Mode with RF09). No offset */
# define RF215_IRQS_READ_OFFSET  (RF215_ADDR_RF09_IRQS - RF215_ADDR_RF09_IRQS)
#endif

/** RF215 internal global variables declared as extern */
extern volatile uint8_t guc_rf215_exception_mask;

#ifdef __cplusplus
}
#endif

#endif  /* RF215_IRQ_DEFS_H_INCLUDE */
