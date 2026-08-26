/**
 * \file
 *
 * \brief SAM4CMS-DB LEDs support package.
 *
 * Copyright (c) 2014 Atmel Corporation. All rights reserved.
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

#ifndef SHARED_MEMORY_H_INCLUDED
#define SHARED_MEMORY_H_INCLUDED

/** ====================SHARED MEMORY SETTINGS=========================
 */
#define mem_reg_in (0x20100000)
#define mem_reg_out (mem_reg_in + 55 * 4) /* DSP_CONTROL_SIZE*4) */
#define mem_acc_out (mem_reg_out + 35 * 4) /* DSP_ST_SIZE*4) */
#define mem_har_out (mem_acc_out + 55 * 8) /* DSP_ACC_SIZE*8) */

#define METROLOGY_REG_IN_SIZE   55
#define METROLOGY_REG_OUT_SIZE  35
#define METROLOGY_ACC_OUT_SIZE  55
#define METROLOGY_HAR_OUT_SIZE  12

/** Configure IPC channels
 */
/* #define IPC_CORE0 IPC0 */
/* #define ID_IPC_CORE0 (IRQn_Type)ID_IPC0 */
/* #define IPC_CORE1 IPC1 */
/* #define ID_IPC_CORE1 (IRQn_Type)ID_IPC1 */

/* IPC channel for shared memory
 */
#define IPC_INIT_IRQ                    (0x1u << 20)
#define IPC_STATUS_IRQ                  (0x1U << 16)
#define IPC_HALF_CYCLE_IRQ              (0x1U << 5)
#define IPC_FULL_CYCLE_IRQ              (0x1U << 4)
#define IPC_INTEGRATION_IRQ             (0x1U << 0)

#define WRITE_CONTROL_DATA      1

/* **************************** COPROCESSOR ********************************
 */
/* Variable definitions */
#define RSTC_CP_KEY_VAL 0x5Au  /* Reset Controller Coprocessor */
#define CP_KEY_VAL     0xAu   /* CPKEY :  coprocessor */
#define PMC_SCER_CPKEY(value) ((PMC_SCER_CPKEY_Msk & ((value) << PMC_SCER_CPKEY_Pos)))
/* ----------------------------------------------------------------- */

extern __IO uint32_t *Metrology_Reg_In;
extern __IO uint32_t *Metrology_Reg_Out;
extern __IO uint32_t *Metrology_Acc_Out;
extern __IO uint32_t *Metrology_Har_Out;

void    configure_ipc( void );

void ipc_init_irq_handler(Ipc *p, enum ipc_interrupt_source mask);
void ipc_status_handler(Ipc *p, enum ipc_interrupt_source mask);
void ipc_half_cycle_handler(Ipc *p, enum ipc_interrupt_source mask);
void ipc_full_cycle_irq_handler(Ipc *p, enum ipc_interrupt_source mask);
void ipc_integration_irq_handler(Ipc *p, enum ipc_interrupt_source mask);
void ipc_integration_event_set_callback(void (*pf_ipc_event_cb)(void));

#endif /* SHARED_MEMORY_H_INCLUDED */
