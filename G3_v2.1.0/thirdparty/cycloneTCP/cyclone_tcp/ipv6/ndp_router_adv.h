/**
 * @file ndp_router_adv.h
 * @brief Router advertisement service
 *
 * @section License
 *
 * Copyright (C) 2010-2019 Oryx Embedded SARL. All rights reserved.
 *
 * This file is part of CycloneTCP Eval.
 *
 * This software is provided in source form for a short-term evaluation only. The
 * evaluation license expires 90 days after the date you first download the software.
 *
 * If you plan to use this software in a commercial product, you are required to
 * purchase a commercial license from Oryx Embedded SARL.
 *
 * After the 90-day evaluation period, you agree to either purchase a commercial
 * license or delete all copies of this software. If you wish to extend the
 * evaluation period, you must contact sales@oryx-embedded.com.
 *
 * This evaluation software is provided "as is" without warranty of any kind.
 * Technical support is available as an option during the evaluation period.
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 1.9.4
 **/

#ifndef _NDP_ROUTER_ADV_H
#define _NDP_ROUTER_ADV_H

//Dependencies
#include "core/net.h"
#include "ipv6/ipv6.h"

//RA service support
#ifndef NDP_ROUTER_ADV_SUPPORT
   #define NDP_ROUTER_ADV_SUPPORT DISABLED
#elif (NDP_ROUTER_ADV_SUPPORT != ENABLED && NDP_ROUTER_ADV_SUPPORT != DISABLED)
   #error NDP_ROUTER_ADV_SUPPORT parameter is not valid
#endif

//RA service tick interval
#ifndef NDP_ROUTER_ADV_TICK_INTERVAL
   #define NDP_ROUTER_ADV_TICK_INTERVAL 100
#elif (NDP_ROUTER_ADV_TICK_INTERVAL < 10)
   #error NDP_ROUTER_ADV_TICK_INTERVAL parameter is not valid
#endif

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief IPv6 prefix information
 **/

typedef struct
{
   Ipv6Addr prefix;
   uint8_t length;
   bool_t onLinkFlag;
   bool_t autonomousFlag;
   uint32_t validLifetime;
   uint32_t preferredLifetime;
} NdpRouterAdvPrefixInfo;


/**
 * @brief Route information
 **/

typedef struct
{
   Ipv6Addr prefix;
   uint8_t length;
   uint8_t preference;
   uint32_t routeLifetime;
} NdpRouterAdvRouteInfo;


/**
 * @brief Context information for 6LoWPAN header compression
 **/

typedef struct
{
   uint8_t cid;
   Ipv6Addr prefix;
   uint8_t length;
   bool_t compression;
   uint16_t validLifetime;
} NdpRouterAdvContextInfo;


/**
 * @brief RA service settings
 **/

typedef struct
{
   NetInterface *interface;
   systime_t maxRtrAdvInterval;
   systime_t minRtrAdvInterval;
   uint8_t curHopLimit;
   bool_t managedFlag;
   bool_t otherConfigFlag;
   bool_t homeAgentFlag;
   uint8_t preference;
   bool_t proxyFlag;
   uint16_t defaultLifetime;
   uint32_t reachableTime;
   uint32_t retransTimer;
   uint32_t linkMtu;
   NdpRouterAdvPrefixInfo *prefixList;
   uint_t prefixListLength;
   NdpRouterAdvRouteInfo *routeList;
   uint_t routeListLength;
   NdpRouterAdvContextInfo *contextList;
   uint_t contextListLength;
} NdpRouterAdvSettings;


/**
 * @brief RA service context
 **/

typedef struct
{
   NdpRouterAdvSettings settings; ///<RA service settings
   bool_t running;                ///<This flag tells whether the RA service is running
   systime_t timestamp;           ///<Timestamp to manage retransmissions
   systime_t timeout;             ///<Timeout value
   uint_t routerAdvCount;         ///<Router Advertisement message counter
} NdpRouterAdvContext;


//Tick counter to handle periodic operations
extern systime_t ndpRouterAdvTickCounter;

//RA service related functions
void ndpRouterAdvGetDefaultSettings(NdpRouterAdvSettings *settings);

error_t ndpRouterAdvInit(NdpRouterAdvContext *context,
   const NdpRouterAdvSettings *settings);

error_t ndpRouterAdvStart(NdpRouterAdvContext *context);
error_t ndpRouterAdvStop(NdpRouterAdvContext *context);

void ndpRouterAdvTick(NdpRouterAdvContext *context);
void ndpRouterAdvLinkChangeEvent(NdpRouterAdvContext *context);

void ndpProcessRouterSol(NetInterface *interface, Ipv6PseudoHeader *pseudoHeader,
   const NetBuffer *buffer, size_t offset, uint8_t hopLimit);

error_t ndpSendRouterAdv(NdpRouterAdvContext *context, uint16_t routerLifetime);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
