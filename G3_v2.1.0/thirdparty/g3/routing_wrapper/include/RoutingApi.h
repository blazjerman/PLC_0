/**
 *
 * \file
 *
 * \brief Routing Wrapper API file
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

#ifndef __ROUTING_API_H__
#define __ROUTING_API_H__

#include <AdpApi.h>

void Routing_Reset(uint8_t u8Band);

bool Routing_IsDisabled(void);

bool Routing_IsAutoRREQDisabled(void);

bool Routing_AdpDefaultCoordRouteEnabled(void);

uint8_t Routing_AdpRREPWait(void);

uint16_t Routing_GetDiscoverRouteGlobalSeqNo(void);
void Routing_SetDiscoverRouteGlobalSeqNo(uint16_t seqNo);

void RoutingGetMib(uint32_t u32AttributeId, uint16_t u16AttributeIndex, struct TAdpGetConfirm *pGetConfirm);

void RoutingSetMib(uint32_t u32AttributeId, uint16_t u16AttributeIndex,
  uint8_t u8AttributeLength, const uint8_t *pu8AttributeValue, struct TAdpSetConfirm *pSetConfirm);

struct TAdpRoutingTableEntry *Routing_AddRouteEntry(struct TAdpRoutingTableEntry *pNewEntry, bool *pbTableFull);
struct TAdpRoutingTableEntry *Routing_GetRouteEntry(uint16_t u16DestinationAddress);
uint32_t Routing_GetRouteCount(void);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
typedef void (*LOADNG_DiscoverPath_Callback)(uint8_t u8Status, struct TPathDescriptor *pPathDescriptor);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
void Routing_DiscoverPath(uint16_t u16DstAddr, uint8_t u8MetricType, LOADNG_DiscoverPath_Callback callback);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
void Routing_NotifyRouteError(uint16_t u16DstAddr, uint16_t u16UnreachableAddress, uint8_t u8ErrorCode);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
typedef void (*LOADNG_DiscoverRoute_Callback)(uint8_t u8Status, uint16_t u16DstAddr, uint16_t u16NextHop, void *pUserData);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
void Routing_DiscoverRoute(uint16_t u16DstAddr, uint8_t u8MaxHops, bool bRepair, void *pUserData,
  LOADNG_DiscoverRoute_Callback fnctDiscoverCallback);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
void Routing_ProcessMessage(uint16_t u16MacSrcAddr, uint8_t u8MediaType, enum EAdpMac_Modulation eModulation, uint8_t u8ActiveTones,
  uint8_t u8SubCarriers, uint8_t u8LQI, uint16_t u16MessageLength, uint8_t *pMessageBuffer);

/**********************************************************************************************************************/
/** Refresh the valid time of the route
 **********************************************************************************************************************/
void Routing_RefreshRoute(uint16_t u16DstAddr);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
void Routing_AddCircularRoute(uint16_t m_u16LastCircularRouteAddress);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
void Routing_DeleteRoute(uint16_t u16DstAddr);

/**********************************************************************************************************************/
/** returns true if route is known
 **********************************************************************************************************************/
bool Routing_RouteExists(uint16_t u16DestinationAddress);

/**********************************************************************************************************************/
/** Before calling this function, check if route exists
 **********************************************************************************************************************/
uint16_t Routing_GetRouteAndMediaType(uint16_t u16DestinationAddress, uint8_t *pu8MediaType);

/**********************************************************************************************************************/
/** Add new candidate route
 **********************************************************************************************************************/
struct TAdpRoutingTableEntry *Routing_AddRoute(uint16_t u16DstAddr, uint16_t u16NextHopAddr, uint8_t u8MediaType, bool *pbTableFull);

/**********************************************************************************************************************/
/** Returns true if the address is in the Destination Address Set (CCTT#183)
 **********************************************************************************************************************/
bool Routing_IsInDestinationAddressSet(uint16_t u16Addr);

/**********************************************************************************************************************/
/** Adds node to blacklist for a given medium
 **********************************************************************************************************************/
void Routing_AddBlacklistOnMedium(uint16_t u16Addr, uint8_t u8MediaType);

/**********************************************************************************************************************/
/** Removes a node from blacklist for a given medium
 **********************************************************************************************************************/
void Routing_RemoveBlacklistOnMedium(uint16_t u16Addr, uint8_t u8MediaType);

/**********************************************************************************************************************/
/** Returns true if node has a Route to u16Addr, to its next hop, and to any other node
 **********************************************************************************************************************/
bool Routing_IsRouterTo(uint16_t u16Addr);

#endif
