/**
 *
 * \file
 *
 * \brief ADP Type definition shared with other modules file
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

#ifndef __ADP_SHARED_TYPES_H__
#define __ADP_SHARED_TYPES_H__

#include <AdpApiTypes.h>
#include <Timer.h>

/**********************************************************************************************************************/
/** The ADP_Common_DataSend_Callback primitive reports the results of a ADP_Common_DataSend Request
 **********************************************************************************************************************/
typedef void (*ADP_Common_DataSend_Callback)(uint8_t u8Status);

/**********************************************************************************************************************/
/** The MCPS-DATA.confirm primitive reports the results of a MCPS-DATA.request
 **********************************************************************************************************************/
typedef void (*AdpMac_Callback_DataConfirm)(uint8_t u8Status, void *pUserData);

#pragma pack(push,1)

struct TDataSendParameters {
  struct TAdpAddress m_SrcDeviceAddress;
  struct TAdpAddress m_DstDeviceAddress;
  bool m_bDiscoverRoute;
  uint8_t m_u8Handle;
  uint8_t m_u8MaxHops;
  uint8_t m_u8DataType;
  uint8_t m_u8OriginalDataType;
  uint8_t m_u8QualityOfService;
  uint8_t m_u8Security;
  uint8_t m_u8BroadcastSeqNo;
  bool m_bMeshHeaderNeeded;
  bool m_bMulticast;
  uint16_t m_u16DataLength;
  uint16_t m_u16FragmentOffset;
  uint16_t m_u16LastFragmentSize;
  uint8_t m_u8BufferOffset;
  uint16_t m_u16DatagramTag;
  uint16_t m_u16DatagramSize;
  uint8_t m_u8NumRepairReSendAttemps;
  ADP_Common_DataSend_Callback m_fnctCallback;
  uint8_t m_u8MediaType;
};

struct TDataSend1280 {
  struct TDataSendParameters m_SendParameters;
  uint8_t m_au8Data[1281]; // 1280 + 1 extra byte needed for Lowpan IPv6 header (compressed or not)
  struct TTimer m_fragTimer;
};

struct TDataSend400 {
  struct TDataSendParameters m_SendParameters;
  uint8_t m_au8Data[401]; // payload size + extra data for headers + 1 extra byte needed for Lowpan IPv6 header (compressed or not)
};

struct TDataSend100 {
  struct TDataSendParameters m_SendParameters;
  uint8_t m_au8Data[101]; // payload size + extra data for headers + 1 extra byte needed for Lowpan IPv6 header (compressed or not)
};

struct TProcessQueueEntry {
  struct TDataSendParameters *m_pSendParameters;
  uint8_t *m_pData;
  uint16_t m_u16DataSize;
  bool m_bProcessing;
  bool m_bDelayed;
  struct TProcessQueueEntry *m_pNext;
  int32_t m_i32DelayTime;
  int32_t m_i32ValidTime;
  struct TTimer *m_pFragTimer;
};


// The maximum number of fragments which can be used to receive a fragmented message
#define MAX_NUMBER_OF_FRAGMENTS 6

struct TFragmentInfo {
  uint16_t m_u16Offset;
  uint16_t m_u16Size;
};

struct TLowpanFragmentedData {
  uint16_t m_u16DatagramOrigin;
  uint16_t m_u16DatagramTag;
  uint16_t m_u16DatagramSize;
  uint8_t m_au8Data[1281]; // 1280 max IPv6 packet + 1 byte IPv6 6Lowpan header
  struct TFragmentInfo m_Fragments[MAX_NUMBER_OF_FRAGMENTS];
  bool m_bWasCompressed;
  bool used;
  /// Absolute time in milliseconds when the entry expires
  int32_t m_i32ValidTime;
};

struct TUserDataRREQRREP {
  uint8_t m_u8FrameType;
  uint8_t m_u8MediaType;
  uint16_t m_u16DstAddr;
  void * m_pRREPGeneration;
  void * m_pRouteEntry;
};

struct TUserDataPREQ {
  uint16_t m_u16DstAddr;
  uint16_t m_u16OrigAddr;
  uint16_t m_u16NextHopAddr;
  uint16_t m_u16RsvBits;
  uint8_t m_u8MediaType;
  uint8_t m_u8MetricType;
  uint8_t m_u8ForwardHops;
};

struct TUserDataData {
  void *m_generic_pointer;
};

union TUserDataUnion {
  struct TUserDataRREQRREP m_sUserDataRREQRREP;
  struct TUserDataPREQ m_sUserDataPREQ;
  struct TUserDataData m_sUserDataData;
  uint8_t auc_buffer[8];
};

struct TAdpMac_NeighbourDescriptor {
  uint16_t m_u16ShortAddress;
  enum EAdpMac_Modulation m_eModulation;
  uint8_t m_u8ActiveTones;
  uint8_t m_u8SubCarriers;
  uint8_t m_u8Lqi;
};

struct TAdpMac_DataRequest {
  uint8_t m_u8SrcAddrSize;
  struct TAdpAddress m_DstDeviceAddress;
  uint16_t m_u16MsduLength;
  uint8_t m_Msdu[400];
  uint8_t m_u8TxOptions;
  uint8_t m_u8QualityOfService;
  uint8_t m_u8SecurityLevel;
  uint8_t m_u8KeyIndex;
  uint8_t m_u8MediaType;
};

#pragma pack(pop)

/**********************************************************************************************************************/
/** Functions to get pointers and sizes from AdpConf file to populate TDataSendContext
 **********************************************************************************************************************/
struct TDataSend1280* AdpConfGet1280BufPtr(void);
struct TDataSend400* AdpConfGet400BufPtr(void);
struct TDataSend100* AdpConfGet100BufPtr(void);
uint8_t AdpConfGet1280BufCount(void);
uint8_t AdpConfGet400BufCount(void);
uint8_t AdpConfGet100BufCount(void);
struct TProcessQueueEntry* AdpConfGetProcessQueuePtr(void);
uint8_t AdpConfGetProcessQueueCount(void);
struct TLowpanFragmentedData* AdpConfGetFragmentedTransferTablePtr(void);
uint8_t AdpConfGetFragmentedTransferTableCount(void);

#endif

/**********************************************************************************************************************/
/** @}
 **********************************************************************************************************************/
