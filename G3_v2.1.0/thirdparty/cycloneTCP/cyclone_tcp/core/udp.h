/**
 * @file udp.h
 * @brief UDP (User Datagram Protocol)
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

#ifndef _UDP_H
#define _UDP_H

//Dependencies
#include "core/net.h"
#include "core/tcp.h"

//UDP support
#ifndef UDP_SUPPORT
   #define UDP_SUPPORT ENABLED
#elif (UDP_SUPPORT != ENABLED && UDP_SUPPORT != DISABLED)
   #error UDP_SUPPORT parameter is not valid
#endif

//Maximum number of callback functions that can be registered
//to process incoming UDP datagrams
#ifndef UDP_CALLBACK_TABLE_SIZE
   #define UDP_CALLBACK_TABLE_SIZE 10
#elif (UDP_CALLBACK_TABLE_SIZE < 1)
   #error UDP_CALLBACK_TABLE_SIZE parameter is not valid
#endif

//Receive queue depth for connectionless sockets
#ifndef UDP_RX_QUEUE_SIZE
   #define UDP_RX_QUEUE_SIZE 4
#elif (UDP_RX_QUEUE_SIZE < 1)
   #error UDP_RX_QUEUE_SIZE parameter is not valid
#endif

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


//CodeWarrior or Win32 compiler?
#if defined(__CWCC__) || defined(_WIN32)
   #pragma pack(push, 1)
#endif


/**
 * @brief UDP header
 **/

typedef __start_packed struct
{
   uint16_t srcPort;  //0-1
   uint16_t destPort; //2-3
   uint16_t length;   //4-5
   uint16_t checksum; //6-7
   uint8_t data[];    //8
} __end_packed UdpHeader;


//CodeWarrior or Win32 compiler?
#if defined(__CWCC__) || defined(_WIN32)
   #pragma pack(pop)
#endif


/**
 * @brief Data received callback
 **/

typedef void (*UdpRxCallback)(NetInterface *interface, const IpPseudoHeader *pseudoHeader,
   const UdpHeader *header, const NetBuffer *buffer, size_t offset, void *param);


/**
 * @brief Entry describing a user callback
 **/

typedef struct
{
   NetInterface *interface;
   uint16_t port;
   UdpRxCallback callback;
   void *param;
} UdpRxCallbackDesc;


//Global variables
extern OsMutex udpCallbackMutex;
extern UdpRxCallbackDesc udpCallbackTable[UDP_CALLBACK_TABLE_SIZE];

//UDP related functions
error_t udpInit(void);
uint16_t udpGetDynamicPort(void);

error_t udpProcessDatagram(NetInterface *interface,
   IpPseudoHeader *pseudoHeader, const NetBuffer *buffer, size_t offset);

error_t udpSendDatagram(Socket *socket, const IpAddr *destIpAddr,
   uint16_t destPort, const void *data, size_t length, size_t *written,
   uint_t flags);

error_t udpSendDatagramEx(NetInterface *interface, const IpAddr *srcIpAddr,
   uint16_t srcPort, const IpAddr *destIpAddr, uint16_t destPort,
   NetBuffer *buffer, size_t offset, uint_t flags);

error_t udpReceiveDatagram(Socket *socket, IpAddr *srcIpAddr, uint16_t *srcPort,
   IpAddr *destIpAddr, void *data, size_t size, size_t *received, uint_t flags);

NetBuffer *udpAllocBuffer(size_t length, size_t *offset);

void udpUpdateEvents(Socket *socket);

error_t udpAttachRxCallback(NetInterface *interface,
   uint16_t port, UdpRxCallback callback, void *param);

error_t udpDetachRxCallback(NetInterface *interface, uint16_t port);

error_t udpInvokeRxCallback(NetInterface *interface, const IpPseudoHeader *pseudoHeader,
   const UdpHeader *header, const NetBuffer *buffer, size_t offset);

void udpDumpHeader(const UdpHeader *datagram);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
