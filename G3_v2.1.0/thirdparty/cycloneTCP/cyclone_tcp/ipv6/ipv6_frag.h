/**
 * @file ipv6_frag.h
 * @brief IPv6 fragmentation and reassembly
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

#ifndef _IPV6_FRAG_H
#define _IPV6_FRAG_H

//Dependencies
#include "core/net.h"
#include "ipv6/ipv6.h"

//IPv6 fragmentation support
#ifndef IPV6_FRAG_SUPPORT
   #define IPV6_FRAG_SUPPORT ENABLED
#elif (IPV6_FRAG_SUPPORT != ENABLED && IPV6_FRAG_SUPPORT != DISABLED)
   #error IPV6_FRAG_SUPPORT parameter is not valid
#endif

//Support for overlapping fragments
#ifndef IPV6_OVERLAPPING_FRAG_SUPPORT
   #define IPV6_OVERLAPPING_FRAG_SUPPORT ENABLED
#elif (IPV6_OVERLAPPING_FRAG_SUPPORT != ENABLED && IPV6_OVERLAPPING_FRAG_SUPPORT != DISABLED)
   #error IPV6_OVERLAPPING_FRAG_SUPPORT parameter is not valid
#endif

//Reassembly algorithm tick interval
#ifndef IPV6_FRAG_TICK_INTERVAL
   #define IPV6_FRAG_TICK_INTERVAL 1000
#elif (IPV6_FRAG_TICK_INTERVAL < 10)
   #error IPV6_FRAG_TICK_INTERVAL parameter is not valid
#endif

//Maximum number of fragmented packets the host will accept
//and hold in the reassembly queue simultaneously
#ifndef IPV6_MAX_FRAG_DATAGRAMS
   #define IPV6_MAX_FRAG_DATAGRAMS 4
#elif (IPV6_MAX_FRAG_DATAGRAMS < 1)
   #error IPV6_MAX_FRAG_DATAGRAMS parameter is not valid
#endif

//Maximum datagram size the host will accept when reassembling fragments
#ifndef IPV6_MAX_FRAG_DATAGRAM_SIZE
   #define IPV6_MAX_FRAG_DATAGRAM_SIZE 8192
#elif (IPV6_MAX_FRAG_DATAGRAM_SIZE < 1280)
   #error IPV6_MAX_FRAG_DATAGRAM_SIZE parameter is not valid
#endif

//Maximum time an IPv6 fragment can spend waiting to be reassembled
#ifndef IPV6_FRAG_TIME_TO_LIVE
   #define IPV6_FRAG_TIME_TO_LIVE 15000
#elif (IPV6_FRAG_TIME_TO_LIVE < 1000)
   #error IPV6_FRAG_TIME_TO_LIVE parameter is not valid
#endif

//Infinity is implemented by a very large integer
#define IPV6_INFINITY 0xFFFF

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


//CodeWarrior or Win32 compiler?
#if defined(__CWCC__) || defined(_WIN32)
   #pragma pack(push, 1)
#endif


/**
 * @brief Hole descriptor
 **/

typedef __start_packed struct
{
   uint16_t first;
   uint16_t last;
   uint16_t next;
} __end_packed Ipv6HoleDesc;


//CodeWarrior or Win32 compiler?
#if defined(__CWCC__) || defined(_WIN32)
   #pragma pack(pop)
#endif


/**
 * @brief Reassembly buffer
 **/

typedef struct
{
   uint_t chunkCount;
   uint_t maxChunkCount;
   ChunkDesc chunk[N(IPV6_MAX_FRAG_DATAGRAM_SIZE) + 1];
} Ipv6ReassemblyBuffer;


/**
 * @brief Fragmented packet descriptor
 **/

typedef struct
{
   systime_t timestamp;         ///<Time at which the first fragment was received
   uint32_t identification;     ///<Fragment identification field
   size_t unfragPartLength;     ///<Length of the unfragmentable part
   size_t fragPartLength;       ///<Length of the fragmentable part
   uint16_t firstHole;          ///<Index of the first hole
   Ipv6ReassemblyBuffer buffer; ///<Buffer containing the reassembled datagram
} Ipv6FragDesc;


//Tick counter to handle periodic operations
extern systime_t ipv6FragTickCounter;

//IPv6 datagram fragmentation and reassembly
error_t ipv6FragmentDatagram(NetInterface *interface,
   Ipv6PseudoHeader *pseudoHeader, const NetBuffer *payload,
   size_t payloadOffset, size_t pathMtu, uint_t flags);

void ipv6ParseFragmentHeader(NetInterface *interface, const NetBuffer *ipPacket,
   size_t ipPacketOffset, size_t fragHeaderOffset, size_t nextHeaderOffset);

void ipv6FragTick(NetInterface *interface);

Ipv6FragDesc *ipv6SearchFragQueue(NetInterface *interface,
   Ipv6Header *packet, Ipv6FragmentHeader *header);

void ipv6FlushFragQueue(NetInterface *interface);

Ipv6HoleDesc *ipv6FindHole(Ipv6FragDesc *frag, uint16_t offset);
void ipv6DumpHoleList(Ipv6FragDesc *frag);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
