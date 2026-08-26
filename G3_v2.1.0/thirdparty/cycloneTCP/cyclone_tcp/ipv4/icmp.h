/**
 * @file icmp.h
 * @brief ICMP (Internet Control Message Protocol)
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

#ifndef _ICMP_H
#define _ICMP_H

//Dependencies
#include "core/net.h"

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief ICMP message type
 *
 * The type field indicates the type of the message. Its
 * value determines the format of the remaining data
 *
 **/

typedef enum
{
   ICMP_TYPE_ECHO_REPLY        = 0,
   ICMP_TYPE_DEST_UNREACHABLE  = 3,
   ICMP_TYPE_SOURCE_QUENCH     = 4,
   ICMP_TYPE_REDIRECT          = 5,
   ICMP_TYPE_ECHO_REQUEST      = 8,
   ICMP_TYPE_TIME_EXCEEDED     = 11,
   ICMP_TYPE_PARAM_PROBLEM     = 12,
   ICMP_TYPE_TIMESTAMP_REQUEST = 13,
   ICMP_TYPE_TIMESTAMP_REPLY   = 14,
   ICMP_TYPE_INFO_REQUEST      = 15,
   ICMP_TYPE_INFO_REPLY        = 16,
   ICMP_TYPE_ADDR_MASK_REQUEST = 17,
   ICMP_TYPE_ADDR_MASK_REPLY   = 18
} IcmpType;


/**
 * @brief Destination Unreachable message codes
 **/

typedef enum
{
   ICMP_CODE_NET_UNREACHABLE        = 0,
   ICMP_CODE_HOST_UNREACHABLE       = 1,
   ICMP_CODE_PROTOCOL_UNREACHABLE   = 2,
   ICMP_CODE_PORT_UNREACHABLE       = 3,
   ICMP_CODE_FRAG_NEEDED_AND_DF_SET = 4,
   ICMP_CODE_SOURCE_ROUTE_FAILED    = 5
} IcmpDestUnreachableCode;


/**
 * @brief Time Exceeded message codes
 **/

typedef enum
{
   ICMP_CODE_TTL_EXCEEDED             = 0,
   ICMP_CODE_REASSEMBLY_TIME_EXCEEDED = 1
} IcmpTimeExceededCode;


//CodeWarrior or Win32 compiler?
#if defined(__CWCC__) || defined(_WIN32)
   #pragma pack(push, 1)
#endif


/**
 * @brief ICMP header
 **/

typedef __start_packed struct
{
   uint8_t type;      //0
   uint8_t code;      //1
   uint16_t checksum; //2-3
   uint8_t data[];    //4
} __end_packed IcmpHeader;


/**
 * @brief ICMP Error message
 **/

typedef __start_packed struct
{
   uint8_t type;           //0
   uint8_t code;           //1
   uint16_t checksum;      //2-3
   uint32_t parameter : 8; //4
   uint32_t unused : 24;   //5-7
   uint8_t data[];         //8
} __end_packed IcmpErrorMessage;


/**
 * @brief ICMP Destination Unreachable message
 **/

typedef __start_packed struct
{
   uint8_t type;      //0
   uint8_t code;      //1
   uint16_t checksum; //2-3
   uint32_t unused;   //4-7
   uint8_t data[];    //8
} __end_packed IcmpDestUnreachableMessage;


/**
 * @brief ICMP Time Exceeded message
 **/

typedef __start_packed struct
{
   uint8_t type;      //0
   uint8_t code;      //1
   uint16_t checksum; //2-3
   uint32_t unused;   //4-7
   uint8_t data[];    //8
} __end_packed IcmpTimeExceededMessage;


/**
 * @brief ICMP Parameter Problem message
 **/

typedef __start_packed struct
{
   uint8_t type;         //0
   uint8_t code;         //1
   uint16_t checksum;    //2-3
   uint32_t pointer : 8; //4
   uint32_t unused : 24; //5-7
   uint8_t data[];       //8
} __end_packed IcmpParamProblemMessage;


/**
 * @brief ICMP Echo Request and Echo Reply messages
 **/

typedef __start_packed struct
{
   uint8_t type;            //0
   uint8_t code;            //1
   uint16_t checksum;       //2-3
   uint16_t identifier;     //4-5
   uint16_t sequenceNumber; //6-7
   uint8_t data[];          //8
} __end_packed IcmpEchoMessage;


//CodeWarrior or Win32 compiler?
#if defined(__CWCC__) || defined(_WIN32)
   #pragma pack(pop)
#endif


//ICMP related functions
void icmpProcessMessage(NetInterface *interface,
   Ipv4PseudoHeader *requestPseudoHeader, const NetBuffer *buffer,
   size_t offset);

void icmpProcessEchoRequest(NetInterface *interface,
   Ipv4PseudoHeader *requestPseudoHeader, const NetBuffer *request,
   size_t requestOffset);

error_t icmpSendErrorMessage(NetInterface *interface, uint8_t type, uint8_t code,
   uint8_t parameter, const NetBuffer *ipPacket, size_t ipPacketOffset);

void icmpUpdateInStats(uint8_t type);
void icmpUpdateOutStats(uint8_t type);

void icmpDumpMessage(const IcmpHeader *message);
void icmpDumpEchoMessage(const IcmpEchoMessage *message);
void icmpDumpErrorMessage(const IcmpErrorMessage *message);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
