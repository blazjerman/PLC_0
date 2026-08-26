/**
 *
 * \file
 *
 * \brief PLC MAC RT Layer Definitions file
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

#ifndef MAC_RT_DEFS_H_
#define MAC_RT_DEFS_H_

#if defined (__CC_ARM)
  #pragma anon_unions
#endif

#define MAC_RT_MAX_PAYLOAD_SIZE (404u)
#define MAC_RT_FULL_HEADER_SIZE (32)
#define MAC_RT_SECURITY_HEADER_SIZE (6)
#define MAC_RT_SEGMENT_CONTROL_SIZE (3)
#define MAC_RT_FCS_SIZE (2)

#define MAX_PHY_TONES 72
#define MAX_PHY_TONE_GROUPS 24

struct TRtToneMap {
  uint8_t m_au8Tm[(MAX_PHY_TONE_GROUPS + 7) / 8];
};

struct TRtToneMask {
  uint8_t m_au8ToneMask[(MAX_PHY_TONES + 7) / 8];
};

enum ERtModulationType {
  RT_MODULATION_ROBUST = 0x00,
  RT_MODULATION_DBPSK_BPSK = 0x01,
  RT_MODULATION_DQPSK_QPSK = 0x02,
  RT_MODULATION_D8PSK_8PSK = 0x03,
  RT_MODULATION_16_QAM = 0x04,
};

enum ERtModulationScheme {
  RT_MODULATION_SCHEME_DIFFERENTIAL = 0x00,
  RT_MODULATION_SCHEME_COHERENT = 0x01,
};

struct TRtToneMapResponseData {
  enum ERtModulationType m_eModulationType;
  enum ERtModulationScheme m_eModulationScheme;
  struct TRtToneMap m_ToneMap;
};

struct TExtAddress {
  uint8_t m_au8Address[8];
};

enum EMacRtAddressMode {
  MAC_RT_ADDRESS_MODE_NO_ADDRESS = 0x00,
  MAC_RT_ADDRESS_MODE_SHORT = 0x02,
  MAC_RT_ADDRESS_MODE_EXTENDED = 0x03,
};

struct TMacRtAddress {
  enum EMacRtAddressMode m_eAddrMode;
  union {
    uint16_t m_nShortAddress;
    struct TExtAddress m_ExtendedAddress;
  };
};

enum EMacRtFrameType {
  MAC_RT_FRAME_TYPE_BEACON = 0x00,
  MAC_RT_FRAME_TYPE_DATA = 0x01,
  MAC_RT_FRAME_TYPE_ACKNOWLEDGMENT = 0x02,
  MAC_RT_FRAME_TYPE_MAC_COMMAND = 0x03,
  MAC_RT_FRAME_TYPE_RESERVED_4 = 0x04,
  MAC_RT_FRAME_TYPE_RESERVED_5 = 0x05,
  MAC_RT_FRAME_TYPE_RESERVED_6 = 0x06,
  MAC_RT_FRAME_TYPE_RESERVED_7 = 0x07,
};

enum EMacRtCommand {
  MAC_RT_COMMAND_BEACON_REQUEST = 0x07,
  MAC_RT_COMMAND_TONE_MAP_RESPONSE = 0x0A,
};

struct TMacRtDsnTableEntry {
  struct TMacRtAddress m_Address;
  uint8_t m_u8Dsn;
  uint8_t m_u8DsnValidTime;
};

#pragma pack(push,1)

struct TMacRtPOSEntry {
  uint16_t m_nShortAddress;
  uint8_t m_u8Lqi;
  uint16_t m_u16POSValidTime;
};

struct TMacRtTxCoef {
  uint8_t m_au8TxCoef[6];
};

struct TMacRtNeighbourEntry {
  uint16_t m_nShortAddress;
  struct TRtToneMap m_ToneMap;
  uint8_t m_nModulationType : 3;
  uint8_t m_nTxGain : 4;
  uint8_t m_nTxRes : 1;
  struct TMacRtTxCoef m_TxCoef;
  uint8_t m_nModulationScheme : 1;
  uint8_t m_nPhaseDifferential : 3;
  uint8_t m_u8Lqi;
  uint16_t m_u16TmrValidTime;
};

#pragma pack(pop)

struct TMacRtFc {
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

struct TMacRtSegmentControl {
  uint8_t m_nRes : 4;
  uint8_t m_nTmr : 1;
  uint8_t m_nCc : 1;
  uint8_t m_nCap : 1;
  uint8_t m_nLsf : 1;
  uint16_t m_nSc : 6;
  uint16_t m_nSl : 10;
};

struct TMacRtAuxiliarySecurityHeader {
  uint8_t m_nSecurityLevel : 3;
  uint8_t m_nKeyIdentifierMode : 2;
  uint8_t m_nReserved : 3;
  uint32_t m_u32FrameCounter;
  uint8_t m_u8KeyIdentifier;
};

struct TMacRtMhr {
  struct TMacRtFc m_Fc;
  uint8_t m_u8SequenceNumber;
  uint16_t m_nDestinationPanIdentifier;
  struct TMacRtAddress m_DestinationAddress;
  uint16_t m_nSourcePanIdentifier;
  struct TMacRtAddress m_SourceAddress;
  struct TMacRtAuxiliarySecurityHeader m_SecurityHeader;
};

struct TMacRtFrame {
  struct TMacRtSegmentControl m_SegmentControl;
  struct TMacRtMhr m_Header;
  uint16_t m_u16PayloadLength;
  uint8_t *m_pu8Payload;
  uint8_t m_u8PadLength;
  uint16_t m_u16Fcs;
};

// TonaMapResponse structure
struct TRtToneMapResponse {
  uint8_t m_nModulationScheme;
  uint8_t m_nModulationType;
  uint8_t m_nTxGain;
  uint8_t m_nTxRes;
  struct TRtToneMap m_ToneMap;
  uint8_t m_nReservedOne;
  uint8_t m_u8Lqi;
  struct TMacRtTxCoef m_TxCoef;
  uint8_t m_u8ReservedTwo;
};

enum EMacRtStatus {
  MAC_RT_STATUS_SUCCESS = 0x00,
  MAC_RT_STATUS_CHANNEL_ACCESS_FAILURE = 0xE1,
  MAC_RT_STATUS_DENIED = 0xE2,
  MAC_RT_STATUS_INVALID_INDEX = 0xF9,
  MAC_RT_STATUS_INVALID_PARAMETER = 0xE8,
  MAC_RT_STATUS_NO_ACK = 0xE9,
  MAC_RT_STATUS_READ_ONLY = 0xFB,
  MAC_RT_STATUS_TRANSACTION_OVERFLOW = 0xF1,
  MAC_RT_STATUS_UNSUPPORTED_ATTRIBUTE = 0xF4,
};

enum EMacRtTxState {
  MAC_RT_TX_START, // Start of transmission.
  MAC_RT_TX_CSMA_CA, // Start of CSMA_CA.
  MAC_RT_TX_FAIL_CSMA_CA, // CSMA_CA channel not idle.
  MAC_RT_TX_WAIT_SEND, // Wait until a data request can be issued.
  MAC_RT_TX_WAIT_CONFIRM, // Wait for data confirm.
  MAC_RT_TX_WAIT_ACK, // Wait for ACK / NAK.
  MAC_RT_TX_SEND_OK, // Segment send succeeded and positive ACK received / no ACK requested.
  MAC_RT_TX_BIG_FAIL, // No ACK was received.
  MAC_RT_TX_LITTLE_FAIL, // Negative ACK was received.
  MAC_RT_TX_ABORT, // Unexpected PHY error.
};

#define MAC_RT_SHORT_ADDRESS_BROADCAST (0xFFFFu)
#define MAC_RT_SHORT_ADDRESS_UNDEFINED (0xFFFFu)
#define MAC_RT_PAN_ID_BROADCAST (0xFFFFu)

#endif

/**********************************************************************************************************************/
/** @}
 **********************************************************************************************************************/
