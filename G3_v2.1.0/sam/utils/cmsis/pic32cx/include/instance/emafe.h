/* ---------------------------------------------------------------------------- */
/*                Microchip Microcontroller Software Support                    */
/*                       SAM Software Package License                           */
/* ---------------------------------------------------------------------------- */
/* Copyright (c) 2022, Microchip Technology Inc.                    */
/*                                                                              */
/* All rights reserved.                                                         */
/*                                                                              */
/* Redistribution and use in source and binary forms, with or without           */
/* modification, are permitted provided that the following condition is met:    */
/*                                                                              */
/* - Redistributions of source code must retain the above copyright notice,     */
/* this list of conditions and the disclaimer below.                            */
/*                                                                              */
/* Microchip's name may not be used to endorse or promote products derived from */
/* this software without specific prior written permission.                     */
/*                                                                              */
/* DISCLAIMER:  THIS SOFTWARE IS PROVIDED BY MICROCHIP "AS IS" AND ANY EXPRESS  */
/* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES */
/* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT    */
/* ARE DISCLAIMED. IN NO EVENT SHALL MICROCHIP BE LIABLE FOR ANY DIRECT,        */
/* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES           */
/* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; */
/* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND  */
/* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT   */
/* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF     */
/* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.            */
/* ---------------------------------------------------------------------------- */

#ifndef _PIC32CX_EMAFE_INSTANCE_
#define _PIC32CX_EMAFE_INSTANCE_

/* ========== Register definition for EMAFE peripheral ========== */
#if (defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__))
  #define EMAFE_CR                        (0x48000000U) /**< \brief (EMAFE) Control Register */
  #define EMAFE_MR                        (0x48000004U) /**< \brief (EMAFE) Mode Register */
  #define EMAFE_CDCR                      (0x48000008U) /**< \brief (EMAFE) Channels Difference Comparison Register */
  #define EMAFE_EMR                       (0x4800000CU) /**< \brief (EMAFE) Extended Mode Register */
  #define EMAFE_DMA_DATA                  (0x48000010U) /**< \brief (EMAFE) DMA Data Register */
  #define EMAFE_DATA                      (0x48000014U) /**< \brief (EMAFE) Data Channel 0 Register */
  #define EMAFE_CHA_CFG1                  (0x48000040U) /**< \brief (EMAFE) Channel A Comparison Configuration 1 */
  #define EMAFE_CHB_CFG1                  (0x48000044U) /**< \brief (EMAFE) Channel B Comparison Configuration 1 */
  #define EMAFE_CHA_CFG2                  (0x48000048U) /**< \brief (EMAFE) Channel A Comparison Configuration 2 */
  #define EMAFE_CHB_CFG2                  (0x4800004CU) /**< \brief (EMAFE) Channel B Comparison Configuration 2 */
  #define EMAFE_IER                       (0x48000050U) /**< \brief (EMAFE) Interrupt Enable Register */
  #define EMAFE_IDR                       (0x48000054U) /**< \brief (EMAFE) Interrupt Disable Register */
  #define EMAFE_IMR                       (0x48000058U) /**< \brief (EMAFE) Interrupt Mask Register */
  #define EMAFE_ISR                       (0x4800005CU) /**< \brief (EMAFE) Interrupt Status Register */
  #define EMAFE_SR                        (0x48000060U) /**< \brief (EMAFE) Status Register */
  #define EMAFE_WPMR                      (0x480000E4U) /**< \brief (EMAFE) Write Protection Mode Register */
  #define EMAFE_WPSR                      (0x480000E8U) /**< \brief (EMAFE) Write Protection Status Register */
  #define EMAFE_VERSION                   (0x480000FCU) /**< \brief (EMAFE) Version Register */
  #define EMAFE_RPR                       (0x48000100U) /**< \brief (EMAFE) Receive Pointer Register */
  #define EMAFE_RCR                       (0x48000104U) /**< \brief (EMAFE) Receive Counter Register */
  #define EMAFE_RNPR                      (0x48000110U) /**< \brief (EMAFE) Receive Next Pointer Register */
  #define EMAFE_RNCR                      (0x48000114U) /**< \brief (EMAFE) Receive Next Counter Register */
  #define EMAFE_PTCR                      (0x48000120U) /**< \brief (EMAFE) Transfer Control Register */
  #define EMAFE_PTSR                      (0x48000124U) /**< \brief (EMAFE) Transfer Status Register */
  #define EMAFE_PWPMR                     (0x48000128U) /**< \brief (EMAFE) Write Protection Mode Register */
#else
  #define EMAFE_CR       (*(__O  uint32_t*)0x48000000U) /**< \brief (EMAFE) Control Register */
  #define EMAFE_MR       (*(__IO uint32_t*)0x48000004U) /**< \brief (EMAFE) Mode Register */
  #define EMAFE_CDCR     (*(__IO uint32_t*)0x48000008U) /**< \brief (EMAFE) Channels Difference Comparison Register */
  #define EMAFE_EMR      (*(__IO uint32_t*)0x4800000CU) /**< \brief (EMAFE) Extended Mode Register */
  #define EMAFE_DMA_DATA (*(__I  uint32_t*)0x48000010U) /**< \brief (EMAFE) DMA Data Register */
  #define EMAFE_DATA     (*(__I  uint32_t*)0x48000014U) /**< \brief (EMAFE) Data Channel 0 Register */
  #define EMAFE_CHA_CFG1 (*(__IO uint32_t*)0x48000040U) /**< \brief (EMAFE) Channel A Comparison Configuration 1 */
  #define EMAFE_CHB_CFG1 (*(__IO uint32_t*)0x48000044U) /**< \brief (EMAFE) Channel B Comparison Configuration 1 */
  #define EMAFE_CHA_CFG2 (*(__IO uint32_t*)0x48000048U) /**< \brief (EMAFE) Channel A Comparison Configuration 2 */
  #define EMAFE_CHB_CFG2 (*(__IO uint32_t*)0x4800004CU) /**< \brief (EMAFE) Channel B Comparison Configuration 2 */
  #define EMAFE_IER      (*(__O  uint32_t*)0x48000050U) /**< \brief (EMAFE) Interrupt Enable Register */
  #define EMAFE_IDR      (*(__O  uint32_t*)0x48000054U) /**< \brief (EMAFE) Interrupt Disable Register */
  #define EMAFE_IMR      (*(__I  uint32_t*)0x48000058U) /**< \brief (EMAFE) Interrupt Mask Register */
  #define EMAFE_ISR      (*(__I  uint32_t*)0x4800005CU) /**< \brief (EMAFE) Interrupt Status Register */
  #define EMAFE_SR       (*(__I  uint32_t*)0x48000060U) /**< \brief (EMAFE) Status Register */
  #define EMAFE_WPMR     (*(__IO uint32_t*)0x480000E4U) /**< \brief (EMAFE) Write Protection Mode Register */
  #define EMAFE_WPSR     (*(__I  uint32_t*)0x480000E8U) /**< \brief (EMAFE) Write Protection Status Register */
  #define EMAFE_VERSION  (*(__I  uint32_t*)0x480000FCU) /**< \brief (EMAFE) Version Register */
  #define EMAFE_RPR      (*(__IO uint32_t*)0x48000100U) /**< \brief (EMAFE) Receive Pointer Register */
  #define EMAFE_RCR      (*(__IO uint32_t*)0x48000104U) /**< \brief (EMAFE) Receive Counter Register */
  #define EMAFE_RNPR     (*(__IO uint32_t*)0x48000110U) /**< \brief (EMAFE) Receive Next Pointer Register */
  #define EMAFE_RNCR     (*(__IO uint32_t*)0x48000114U) /**< \brief (EMAFE) Receive Next Counter Register */
  #define EMAFE_PTCR     (*(__O  uint32_t*)0x48000120U) /**< \brief (EMAFE) Transfer Control Register */
  #define EMAFE_PTSR     (*(__I  uint32_t*)0x48000124U) /**< \brief (EMAFE) Transfer Status Register */
  #define EMAFE_PWPMR    (*(__IO uint32_t*)0x48000128U) /**< \brief (EMAFE) Write Protection Mode Register */
#endif /* (defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__)) */

#endif /* _PIC32CX_EMAFE_INSTANCE_ */
