/**
 * @file llmnr_common.h
 * @brief Definitions common to LLMNR client and responder
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

#ifndef _LLMNR_COMMON_H
#define _LLMNR_COMMON_H

//Dependencies
#include "core/net.h"
#include "dns/dns_common.h"

//Default resource record TTL (cache lifetime)
#ifndef LLMNR_DEFAULT_RESOURCE_RECORD_TTL
   #define LLMNR_DEFAULT_RESOURCE_RECORD_TTL 30
#elif (LLMNR_DEFAULT_RESOURCE_RECORD_TTL < 1)
   #error LLMNR_DEFAULT_RESOURCE_RECORD_TTL parameter is not valid
#endif

//LLMNR port number
#define LLMNR_PORT 5355
//Default IP TTL value
#define LLMNR_DEFAULT_IP_TTL 255

//LLMNR IPv4 multicast group
#define LLMNR_IPV4_MULTICAST_ADDR IPV4_ADDR(224, 0, 0, 252)

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


//CodeWarrior or Win32 compiler?
#if defined(__CWCC__) || defined(_WIN32)
   #pragma pack(push, 1)
#endif


/**
 * @brief LLMNR message header
 **/

typedef __start_packed struct
{
   uint16_t id;         //0-1
#ifdef _CPU_BIG_ENDIAN
   uint8_t qr : 1;      //2
   uint8_t opcode : 4;
   uint8_t c : 1;
   uint8_t tc : 1;
   uint8_t t : 1;
   uint8_t z : 4;       //3
   uint8_t rcode : 4;
#else
   uint8_t t : 1;       //2
   uint8_t tc : 1;
   uint8_t c : 1;
   uint8_t opcode : 4;
   uint8_t qr : 1;
   uint8_t rcode : 4;   //3
   uint8_t z : 4;
#endif
   uint16_t qdcount;    //4-5
   uint16_t ancount;    //6-7
   uint16_t nscount;    //8-9
   uint16_t arcount;    //10-11
   uint8_t questions[]; //12
} __end_packed LlmnrHeader;


//CodeWarrior or Win32 compiler?
#if defined(__CWCC__) || defined(_WIN32)
   #pragma pack(pop)
#endif


//LLMNR IPv6 multicast group
extern const Ipv6Addr LLMNR_IPV6_MULTICAST_ADDR;

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
