/**
 * @file raw_socket.h
 * @brief TCP/IP raw sockets
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

#ifndef _RAW_SOCKET_H
#define _RAW_SOCKET_H

//Dependencies
#include "core/net.h"
#include "core/ip.h"
#include "core/socket.h"

//Raw socket support
#ifndef RAW_SOCKET_SUPPORT
   #define RAW_SOCKET_SUPPORT DISABLED
#elif (RAW_SOCKET_SUPPORT != ENABLED && RAW_SOCKET_SUPPORT != DISABLED)
   #error RAW_SOCKET_SUPPORT parameter is not valid
#endif

//Receive queue depth for raw sockets
#ifndef RAW_SOCKET_RX_QUEUE_SIZE
   #define RAW_SOCKET_RX_QUEUE_SIZE 4
#elif (RAW_SOCKET_RX_QUEUE_SIZE < 1)
   #error RAW_SOCKET_RX_QUEUE_SIZE parameter is not valid
#endif

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif

//Raw socket related functions
error_t rawSocketProcessIpPacket(NetInterface *interface,
   IpPseudoHeader *pseudoHeader, const NetBuffer *buffer, size_t offset);

void rawSocketProcessEthPacket(NetInterface *interface, EthHeader *header,
   const uint8_t *data, size_t length);

error_t rawSocketSendIpPacket(Socket *socket, const IpAddr *destIpAddr,
   const void *data, size_t length, size_t *written, uint_t flags);

error_t rawSocketSendEthPacket(Socket *socket, const void *data,
   size_t length, size_t *written);

error_t rawSocketReceiveIpPacket(Socket *socket, IpAddr *srcIpAddr,
   IpAddr *destIpAddr, void *data, size_t size, size_t *received, uint_t flags);

error_t rawSocketReceiveEthPacket(Socket *socket, void *data, size_t size,
   size_t *received, uint_t flags);

void rawSocketUpdateEvents(Socket *socket);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
