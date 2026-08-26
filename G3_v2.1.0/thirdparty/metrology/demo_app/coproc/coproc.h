/**
 * \file
 *
 * \brief Coprocessor Module Header file.
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

#ifndef COPROC_H_INCLUDED
#define COPROC_H_INCLUDED

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* / @endcond */

#include "metrology.h"

#define SHARED_MEMORY_ADDR    IRAM2_ADDR        //(IRAM2_ADDR + IRAM2_SIZE - sizeof(metrology_t))

#define MEM_REG_IN_ADDRESS    SHARED_MEMORY_ADDR

/* IPC channel for shared memory // */
#define IPC_INTEGRATION_IRQ     (IPC_INTERRUPT_SRC_IRQ0)
#define IPC_FULL_CYCLE_IRQ      (IPC_INTERRUPT_SRC_IRQ4)
#define IPC_HALF_CYCLE_IRQ      (IPC_INTERRUPT_SRC_IRQ5)
#define IPC_RZC_IRQ             (IPC_INTERRUPT_SRC_IRQ12)
#define IPC_INIT_IRQ            (IPC_INTERRUPT_SRC_IRQ20)
#define IPC_PULSE0_IRQ          (IPC_INTERRUPT_SRC_IRQ24)
#define IPC_PULSE1_IRQ          (IPC_INTERRUPT_SRC_IRQ25)
#define IPC_PULSE2_IRQ          (IPC_INTERRUPT_SRC_IRQ26)
/* #define IPC_STATUS_IRQ        (IPC_INTERRUPT_SRC_IRQ16) // not implemented */
/* #define IPC_CREEP_IRQ         (IPC_INTERRUPT_SRC_IRQ7) // not implemented */

extern DSP_CTRL_TYPE *ptr_mem_reg_in;
extern DSP_ST_TYPE *ptr_mem_reg_out;
extern DSP_ACC_TYPE *ptr_mem_acc_out;
extern DSP_HAR_TYPE *ptr_mem_har_out;

typedef enum reset_type {
	RESET_TYPE_HARD,
	RESET_TYPE_SOFT,
} reset_type_t;

void CoprocInit(bool monitor_en, reset_type_t reset_type);

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */

#endif /* COPROC_H_INCLUDED */
