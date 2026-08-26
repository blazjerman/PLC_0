/**
 * @file mld.h
 * @brief MLD (Multicast Listener Discovery for IPv6)
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

#ifndef _MLD_H
#define _MLD_H

//Dependencies
#include "core/net.h"

//MLD support
#ifndef MLD_SUPPORT
   #define MLD_SUPPORT DISABLED
#elif (MLD_SUPPORT != ENABLED && MLD_SUPPORT != DISABLED)
   #error MLD_SUPPORT parameter is not valid
#endif

//MLD tick interval
#ifndef MLD_TICK_INTERVAL
   #define MLD_TICK_INTERVAL 1000
#elif (MLD_TICK_INTERVAL < 10)
   #error MLD_TICK_INTERVAL parameter is not valid
#endif

//Unsolicited report interval
#ifndef MLD_UNSOLICITED_REPORT_INTERVAL
   #define MLD_UNSOLICITED_REPORT_INTERVAL 10000
#elif (MLD_UNSOLICITED_REPORT_INTERVAL < 1000)
   #error MLD_UNSOLICITED_REPORT_INTERVAL parameter is not valid
#endif

//Hop Limit used by MLD messages
#define MLD_HOP_LIMIT 1

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief MLD node states
 **/

typedef enum
{
   MLD_STATE_NON_LISTENER      = 0,
   MLD_STATE_DELAYING_LISTENER = 1,
   MLD_STATE_IDLE_LISTENER     = 2
} MldState;


//CodeWarrior or Win32 compiler?
#if defined(__CWCC__) || defined(_WIN32)
   #pragma pack(push, 1)
#endif


/**
 * @brief MLD message
 **/

typedef __start_packed struct
{
   uint8_t type;           //0
   uint8_t code;           //1
   uint16_t checksum;      //2-3
   uint16_t maxRespDelay;  //4-5
   uint16_t reserved;      //6-7
   Ipv6Addr multicastAddr; //8-23
} __end_packed MldMessage;


//CodeWarrior or Win32 compiler?
#if defined(__CWCC__) || defined(_WIN32)
   #pragma pack(pop)
#endif


//Tick counter to handle periodic operations
extern systime_t mldTickCounter;

//MLD related functions
error_t mldInit(NetInterface *interface);
error_t mldStartListening(NetInterface *interface, Ipv6FilterEntry *entry);
error_t mldStopListening(NetInterface *interface, Ipv6FilterEntry *entry);

void mldTick(NetInterface *interface);
void mldLinkChangeEvent(NetInterface *interface);

void mldProcessListenerQuery(NetInterface *interface, Ipv6PseudoHeader *pseudoHeader,
   const NetBuffer *buffer, size_t offset, uint8_t hopLimit);

void mldProcessListenerReport(NetInterface *interface, Ipv6PseudoHeader *pseudoHeader,
   const NetBuffer *buffer, size_t offset, uint8_t hopLimit);

error_t mldSendListenerReport(NetInterface *interface, Ipv6Addr *ipAddr);
error_t mldSendListenerDone(NetInterface *interface, Ipv6Addr *ipAddr);

uint32_t mldRand(uint32_t max);

void mldDumpMessage(const MldMessage *message);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
