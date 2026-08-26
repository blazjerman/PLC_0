/**
 *
 * \file
 *
 * \brief PLC MAC RT Layer API file
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

#ifndef MAC_RT_H_
#define MAC_RT_H_

#include <MacRtDefs.h>

struct TMacRtRxParameters {
  bool m_bHighPriority;
  uint8_t m_u8PpduLinkQuality;
  uint8_t m_u8PhaseDifferential;
  enum ERtModulationType m_eModulationType;
  enum ERtModulationScheme m_eModulationScheme;
  struct TRtToneMap m_ToneMap;
  struct TRtToneMapResponseData m_ToneMapResponseData;
};

typedef void (*MacRtDataIndication)(uint8_t *pMrtsdu, uint16_t u16MrtsduLen);
typedef void (*MacRtSnifferIndication)(uint8_t *pMrtsdu, uint16_t u16MrtsduLen);
typedef void (*MacRtCommStatusIndication)(uint8_t *pMrtsdu);
typedef void (*MacRtTxConfirm)(enum EMacRtStatus eStatus, bool bUpdateTimestamp);
typedef void (*MacRtRxParamsIndication)(struct TMacRtRxParameters *pParameters);

struct TMacRtNotifications {
  MacRtDataIndication m_pDataIndication;
  MacRtSnifferIndication m_pSnifferIndication;
  MacRtCommStatusIndication m_pCommStatusIndication;
  MacRtTxConfirm m_pMacRtTxConfirm;
  MacRtRxParamsIndication m_pMacRtRxParamsIndication;
};

void MacRtInitialize(uint8_t u8Band, struct TMacRtNotifications *pNotifications);
void MacRtEventHandler(void);

void MacRtTxRequest(uint8_t *pTxSdu, uint16_t u16TxSduLen);
void MacRtResetRequest(bool bResetMib);
uint32_t MacRtGetPhyTime(void);

#endif

/**********************************************************************************************************************/
/** @}
 **********************************************************************************************************************/
