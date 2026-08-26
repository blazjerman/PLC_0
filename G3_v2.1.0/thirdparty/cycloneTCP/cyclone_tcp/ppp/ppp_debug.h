/**
 * @file ppp_debug.h
 * @brief Data logging functions for debugging purpose (PPP)
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

#ifndef _DHCP_DEBUG_H
#define _DHCP_DEBUG_H

//Dependencies
#include "core/net.h"
#include "ppp/ppp.h"
#include "ppp/lcp.h"
#include "ppp/ipcp.h"
#include "debug.h"

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//Check current trace level
#if (PPP_TRACE_LEVEL >= TRACE_LEVEL_DEBUG)
   error_t pppDumpPacket(const PppPacket *packet, size_t length, PppProtocol protocol);
   error_t lcpDumpPacket(const PppPacket *packet, size_t length);
   error_t ncpDumpPacket(const PppPacket *packet, size_t length, PppProtocol protocol);
   error_t papDumpPacket(const PppPacket *packet, size_t length);
   error_t chapDumpPacket(const PppPacket *packet, size_t length);
   error_t lcpDumpOptions(const PppOption *option, size_t length);
   error_t ipcpDumpOptions(const PppOption *option, size_t length);
   error_t ipv6cpDumpOptions(const PppOption *option, size_t length);
#else
   #define pppDumpPacket(packet, length, protocol)
#endif

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
