/**
 * @file slaac.h
 * @brief IPv6 Stateless Address Autoconfiguration
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

#ifndef _SLAAC_H
#define _SLAAC_H

//Dependencies
#include "core/net.h"
#include "ipv6/ndp.h"

//SLAAC support
#ifndef SLAAC_SUPPORT
   #define SLAAC_SUPPORT ENABLED
#elif (SLAAC_SUPPORT != ENABLED && SLAAC_SUPPORT != DISABLED)
   #error SLAAC_SUPPORT parameter is not valid
#endif

//Time constant
#define SLAAC_LIFETIME_2_HOURS (2 * 3600 * 1000)

//Forward declaration of SlaacContext structure
struct _SlaacContext;
#define SlaacContext struct _SlaacContext

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief Link state change callback
 **/

typedef void (*SlaacLinkChangeCallback)(SlaacContext *context,
   NetInterface *interface, bool_t linkState);


/**
 * @brief Router Advertisement parsing callback
 **/

typedef void (*SlaacParseRouterAdvCallback)(SlaacContext *context,
   NdpRouterAdvMessage *message, size_t length);


/**
 * @brief SLAAC settings
 **/

typedef struct
{
   NetInterface *interface;                            ///<Network interface to configure
   bool_t manualDnsConfig;                             ///<Force manual DNS configuration
   SlaacLinkChangeCallback linkChangeEvent;            ///<Link state change event
   SlaacParseRouterAdvCallback parseRouterAdvCallback; ///<Router Advertisement parsing callback
} SlaacSettings;


/**
 * @brief SLAAC context
 **/

struct _SlaacContext
{
   SlaacSettings settings; ///<SLAAC settings
   bool_t running;         ///<SLAAC is currently running
   bool_t configUpdated;   ///<This flag is set when IPv6 configuration has been updated
};


//SLAAC related functions
void slaacGetDefaultSettings(SlaacSettings *settings);
error_t slaacInit(SlaacContext *context, const SlaacSettings *settings);
error_t slaacStart(SlaacContext *context);
error_t slaacStop(SlaacContext *context);

void slaacLinkChangeEvent(SlaacContext *context);

void slaacParseRouterAdv(SlaacContext *context,
   NdpRouterAdvMessage *message, size_t length);

void slaacParsePrefixInfoOption(SlaacContext *context,
   NdpPrefixInfoOption *option);

error_t slaacGenerateLinkLocalAddr(SlaacContext *context);

void slaacDumpConfig(SlaacContext *context);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
