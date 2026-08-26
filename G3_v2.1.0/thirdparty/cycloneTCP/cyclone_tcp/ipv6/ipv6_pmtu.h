/**
 * @file ipv6_pmtu.h
 * @brief Path MTU Discovery for IPv6
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

#ifndef _IPV6_PMTU_H
#define _IPV6_PMTU_H

//Dependencies
#include "core/net.h"

//Path MTU discovery support
#ifndef IPV6_PMTU_SUPPORT
   #define IPV6_PMTU_SUPPORT ENABLED
#elif (IPV6_PMTU_SUPPORT != ENABLED && IPV6_PMTU_SUPPORT != DISABLED)
   #error IPV6_PMTU_SUPPORT parameter is not valid
#endif

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//Path MTU discovery related functions
size_t ipv6GetPathMtu(NetInterface *interface, const Ipv6Addr *destAddr);

void ipv6UpdatePathMtu(NetInterface *interface,
   const Ipv6Addr *destAddr, size_t tentativePathMtu);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
