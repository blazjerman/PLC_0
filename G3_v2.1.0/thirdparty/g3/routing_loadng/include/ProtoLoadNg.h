/**
 *
 * \file
 *
 * \brief LOADng Protocol API file
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

#ifndef __PROTO_LOAD_NG_H__
#define __PROTO_LOAD_NG_H__

#include <RoutingTypes.h>

/**********************************************************************************************************************/
/** Flags
 ***********************************************************************************************************************/
#define LOADNG_FLAG_ROUTE_REPAIR 0x08
#define LOADNG_FLAG_UNICAST_RREQ 0x04


/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
void LOADNG_Reset(uint8_t u8Band, struct TRoutingTables *g_RoutingTables);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
void LOADNG_DiscoverPath(uint16_t u16DstAddr, uint8_t u8MetricType, LOADNG_DiscoverPath_Callback callback);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
void LOADNG_ProcessMessage(uint16_t u16MacSrcAddr, uint8_t u8MediaType, enum EAdpMac_Modulation eModulation, uint8_t u8ActiveTones,
  uint8_t u8SubCarriers, uint8_t u8LQI, uint16_t u16MessageLength, uint8_t *pMessageBuffer);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
void LOADNG_NotifyRouteError(uint16_t u16DstAddr, uint16_t u16UnreachableAddress, uint8_t u8ErrorCode);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
void LOADNG_DiscoverRoute(uint16_t u16DstAddr, uint8_t u8MaxHops, bool bRepair, void *pUserData,
  LOADNG_DiscoverRoute_Callback fnctDiscoverCallback);


/**********************************************************************************************************************/
/** Refresh the valid time of the route
 **********************************************************************************************************************/
void LOADNG_RefreshRoute(uint16_t u16DstAddr);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
void LOADNG_AddCircularRoute(uint16_t m_u16LastCircularRouteAddress);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
void LOADNG_DeleteRoute(uint16_t u16DstAddr);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
void LOADNG_DeleteRoutePosition(uint32_t u32Position);

/**********************************************************************************************************************/
/** returns true if route is known
 **********************************************************************************************************************/
bool LOADNG_RouteExists(uint16_t u16DestinationAddress);

/**********************************************************************************************************************/
/** before calling this function, check if route exists (LOADNG_RouteExists)
 **********************************************************************************************************************/
uint16_t LOADNG_GetRouteAndMediaType(uint16_t u16DestinationAddress, uint8_t *pu8MediaType);

/**********************************************************************************************************************/
/** Inserts a route in the routing table
 **********************************************************************************************************************/
struct TAdpRoutingTableEntry *LOADNG_AddRouteEntry(struct TAdpRoutingTableEntry *pNewEntry, bool *pbTableFull);

/**********************************************************************************************************************/
/** Add new candidate route
 **********************************************************************************************************************/
struct TAdpRoutingTableEntry *LOADNG_AddRoute(uint16_t u16DstAddr, uint16_t u16NextHopAddr, uint8_t u8MediaType, bool *pbTableFull);

/**********************************************************************************************************************/
/** Gets a pointer to Route Entry. before calling this function, check if route exists (LOADNG_RouteExists)
 **********************************************************************************************************************/
struct TAdpRoutingTableEntry *LOADNG_GetRouteEntry(uint16_t u16DestinationAddress);

/**********************************************************************************************************************/
/** Gets the route count
 **********************************************************************************************************************/
uint32_t LOADNG_GetRouteCount(void);

/**********************************************************************************************************************/
/** Returns true if the address is in the Destination Address Set (CCTT#183)
 **********************************************************************************************************************/
bool LOADNG_IsInDestinationAddressSet(uint16_t u16Addr);

/**********************************************************************************************************************/
/** Returns LOADNG MIB value
 **********************************************************************************************************************/
void LOADNG_GetMib(uint32_t u32AttributeId, uint16_t u16AttributeIndex, struct TAdpGetConfirm *pGetConfirm);

/**********************************************************************************************************************/
/** Sets LOADNG MIB value
 **********************************************************************************************************************/
void LOADNG_SetMib(uint32_t u32AttributeId, uint16_t u16AttributeIndex,
  uint8_t u8AttributeLength, const uint8_t *pu8AttributeValue, struct TAdpSetConfirm *pSetConfirm);

/**********************************************************************************************************************/
/** Adds node to blacklist for a given medium
 **********************************************************************************************************************/
void LOADNG_AddBlacklistOnMedium(uint16_t u16Addr, uint8_t u8MediaType);

/**********************************************************************************************************************/
/** Removes a node from blacklist for a given medium
 **********************************************************************************************************************/
void LOADNG_RemoveBlacklistOnMedium(uint16_t u16Addr, uint8_t u8MediaType);

/**********************************************************************************************************************/
/** Returns true if node has a Route to u16Addr, to its next hop, and to any other node
 **********************************************************************************************************************/
bool LOADNG_IsRouterTo(uint16_t u16Addr);

#endif


/**********************************************************************************************************************/
/** @}
 **********************************************************************************************************************/

