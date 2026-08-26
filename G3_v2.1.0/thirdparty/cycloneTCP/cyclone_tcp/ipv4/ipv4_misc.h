/**
 * @file ipv4_misc.h
 * @brief Helper functions for IPv4
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

#ifndef _IPV4_MISC_H
#define _IPV4_MISC_H

//Dependencies
#include <string.h>
#include "core/net.h"
#include "ipv4/ipv4.h"

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//IPv4 related functions
error_t ipv4CheckSourceAddr(NetInterface *interface, Ipv4Addr ipAddr);
error_t ipv4CheckDestAddr(NetInterface *interface, Ipv4Addr ipAddr);

error_t ipv4SelectSourceAddr(NetInterface **interface,
   Ipv4Addr destAddr, Ipv4Addr *srcAddr);

bool_t ipv4IsOnLink(NetInterface *interface, Ipv4Addr ipAddr);
bool_t ipv4IsBroadcastAddr(NetInterface *interface, Ipv4Addr ipAddr);
bool_t ipv4IsTentativeAddr(NetInterface *interface, Ipv4Addr ipAddr);
bool_t ipv4IsLocalHostAddr(Ipv4Addr ipAddr);

uint_t ipv4GetAddrScope(Ipv4Addr ipAddr);
uint_t ipv4GetPrefixLength(Ipv4Addr mask);

error_t ipv4GetBroadcastAddr(NetInterface *interface, Ipv4Addr *addr);

error_t ipv4MapMulticastAddrToMac(Ipv4Addr ipAddr, MacAddr *macAddr);

void ipv4UpdateInStats(NetInterface *interface, Ipv4Addr destIpAddr,
   size_t length);

void ipv4UpdateOutStats(NetInterface *interface, Ipv4Addr destIpAddr,
   size_t length);

void ipv4UpdateErrorStats(NetInterface *interface, error_t error);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
