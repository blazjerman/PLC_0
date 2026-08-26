/**
 *
 * \file
 *
 * \brief PLC MAC Layer definitions file
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

#ifndef MAC_DEFS_H_
#define MAC_DEFS_H_

#include <MacCommon.h>

enum EModulationType {
  MODULATION_ROBUST = 0x00,
  MODULATION_DBPSK_BPSK = 0x01,
  MODULATION_DQPSK_QPSK = 0x02,
  MODULATION_D8PSK_8PSK = 0x03,
  MODULATION_16_QAM = 0x04,
};

enum EModulationScheme {
  MODULATION_SCHEME_DIFFERENTIAL = 0x00,
  MODULATION_SCHEME_COHERENT = 0x01,
};

struct TMacFc {
  uint16_t m_nFrameType : 3;
  uint16_t m_nSecurityEnabled : 1;
  uint16_t m_nFramePending : 1;
  uint16_t m_nAckRequest : 1;
  uint16_t m_nPanIdCompression : 1;
  uint16_t m_nReserved : 3;
  uint16_t m_nDestAddressingMode : 2;
  uint16_t m_nFrameVersion : 2;
  uint16_t m_nSrcAddressingMode : 2;
};

struct TMacMhr {
  struct TMacFc m_Fc;
  uint8_t m_u8SequenceNumber;
  TPanId m_nDestinationPanIdentifier;
  struct TMacAddress m_DestinationAddress;
  TPanId m_nSourcePanIdentifier;
  struct TMacAddress m_SourceAddress;
  struct TMacAuxiliarySecurityHeader m_SecurityHeader;
};

struct TMacFrame {
  struct TMacMhr m_Header;
  uint16_t m_u16PayloadLength;
  uint8_t *m_pu8Payload;
  uint8_t m_u8PadLength;
  uint16_t m_u16Fcs;
};

#endif

/**********************************************************************************************************************/
/** @}
 **********************************************************************************************************************/
