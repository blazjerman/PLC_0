/**
 *
 * \file
 *
 * \brief PLC MAC RT Layer Constants file
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

#ifndef MAC_RT_CONSTANTS_H_
#define MAC_RT_CONSTANTS_H_

#define MAC_RT_PREAMBLE_LENGTH_DEFAULT   8

extern uint32_t g_u32MacSlotTime;
extern uint32_t g_u32MacCifsTime;
extern uint32_t g_u32MacCifsRetransmitTime;
extern uint32_t g_u32MacRifsTime;
extern uint32_t g_u32MacIdleSpacingTime;
extern uint32_t g_u32MacAckTime;
extern uint32_t g_u32MacEifsTime;
extern uint32_t g_u32MacAckWaitDuration;

struct TRtToneMask;
extern struct TRtToneMask g_ToneMaskDefault;
extern struct TRtToneMask g_ToneMaskNotched;
extern struct TRtToneMap g_RtToneMapDefault;

extern struct TMacRtNeighbourEntry g_NeighbourEntryDefault;

struct TPhyBandInformation {
  uint16_t m_u16FlMax;
  uint8_t m_u8Band;
  uint8_t m_u8Tones;
  uint8_t m_u8Carriers;
  uint8_t m_u8TonesInCarrier;
  uint8_t m_u8FlBand;
  uint8_t m_u8MaxRsBlocks;
  uint8_t m_u8TxCoefBits;
  uint8_t m_u8PilotsFreqSpa;
};
extern struct TPhyBandInformation g_PhyBandInformation;

void MacRtConstantsInitialize(uint8_t u8Band, uint8_t u8PreambleLength, struct TRtToneMask *pToneMask);

#endif

/**********************************************************************************************************************/
/** @}
 **********************************************************************************************************************/
