/**
 * @file ndp_misc.h
 * @brief Helper functions for NDP (Neighbor Discovery Protocol)
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

#ifndef _NDP_MISC_H
#define _NDP_MISC_H

//Dependencies
#include "core/net.h"
#include "ipv6/ndp.h"

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//NDP related functions
void ndpParsePrefixInfoOption(NetInterface *interface, NdpPrefixInfoOption *option);

void ndpUpdateAddrList(NetInterface *interface);
void ndpUpdatePrefixList(NetInterface *interface);
void ndpUpdateDefaultRouterList(NetInterface *interface);

error_t ndpSelectDefaultRouter(NetInterface *interface,
   const Ipv6Addr *unreachableAddr, Ipv6Addr *addr);

bool_t ndpIsFirstHopRouter(NetInterface *interface,
   const Ipv6Addr *destAddr, const Ipv6Addr *nextHop);

error_t ndpSelectNextHop(NetInterface *interface, const Ipv6Addr *destAddr,
   const Ipv6Addr *unreachableNextHop, Ipv6Addr *nextHop, uint_t flags);

void ndpUpdateNextHop(NetInterface *interface, const Ipv6Addr *nextHop);

void ndpAddOption(void *message, size_t *messageLen, uint8_t type,
   const void *value, size_t length);

void *ndpGetOption(uint8_t *options, size_t length, uint8_t type);

error_t ndpCheckOptions(const uint8_t *options, size_t length);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
