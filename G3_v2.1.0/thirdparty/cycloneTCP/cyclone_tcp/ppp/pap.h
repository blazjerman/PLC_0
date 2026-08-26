/**
 * @file pap.h
 * @brief PAP (Password Authentication Protocol)
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

#ifndef _PAP_H
#define _PAP_H

//Dependencies
#include "core/net.h"
#include "ppp/ppp.h"

//PAP authentication support
#ifndef PAP_SUPPORT
   #define PAP_SUPPORT ENABLED
#elif (PAP_SUPPORT != ENABLED && PAP_SUPPORT != DISABLED)
   #error PAP_SUPPORT parameter is not valid
#endif

//Restart timer
#ifndef PAP_RESTART_TIMER
   #define PAP_RESTART_TIMER 3000
#elif (PAP_RESTART_TIMER < 1000)
   #error PAP_RESTART_TIMER parameter is not valid
#endif

//Maximum number of retransmissions for Authenticate-Request packets
#ifndef PAP_MAX_REQUESTS
   #define PAP_MAX_REQUESTS 5
#elif (PAP_MAX_REQUESTS < 1)
   #error PAP_MAX_REQUESTS parameter is not valid
#endif

//C++ guard
#ifdef __cplusplus
   extern "C" {
#endif


/**
 * @brief PAP states
 **/

typedef enum
{
   PAP_STATE_0_INITIAL  = 0,
   PAP_STATE_1_STARTED  = 1,
   PAP_STATE_2_REQ_SENT = 2,
   PAP_STATE_3_REQ_RCVD = 3,
   PAP_STATE_4_ACK_SENT = 4,
   PAP_STATE_5_ACK_RCVD = 5,
   PAP_STATE_6_NAK_SENT = 6,
   PAP_STATE_7_NAK_RCVD = 7
} PapState;


/**
 * @brief Code field values
 **/

typedef enum
{
   PAP_CODE_AUTH_REQ = 1, ///<Authenticate-Request
   PAP_CODE_AUTH_ACK = 2, ///<Authenticate-Ack
   PAP_CODE_AUTH_NAK = 3  ///<Authenticate-Nak
} PapCode;


//CodeWarrior or Win32 compiler?
#if defined(__CWCC__) || defined(_WIN32)
   #pragma pack(push, 1)
#endif


/**
 * @brief Authenticate-Request packet
 **/

typedef __start_packed struct
{
   uint8_t code;         //0
   uint8_t identifier;   //1
   uint16_t length;      //2-3
   uint8_t peerIdLength; //4
   uint8_t peerId[];     //5
} __end_packed PapAuthReqPacket;


/**
 * @brief Authenticate-Ack packet
 **/

typedef __start_packed struct
{
   uint8_t code;       //0
   uint8_t identifier; //1
   uint16_t length;    //2-3
   uint8_t msgLength;  //4
   uint8_t message[];  //5
} __end_packed PapAuthAckPacket;


/**
 * @brief Authenticate-Nak packet
 **/

typedef __start_packed struct
{
   uint8_t code;       //0
   uint8_t identifier; //1
   uint16_t length;    //2-3
   uint8_t msgLength;  //4
   uint8_t message[];  //5
} __end_packed PapAuthNakPacket;


//CodeWarrior or Win32 compiler?
#if defined(__CWCC__) || defined(_WIN32)
   #pragma pack(pop)
#endif


/**
 * @brief PAP finite state machine
 **/

typedef struct
{
   uint_t localState;       ///<Local state
   uint_t peerState;        ///<Peer state
   uint8_t identifier;      ///<Identifier used to match requests and replies
   uint_t restartCounter;   ///<Restart counter
   systime_t timestamp;     ///<Timestamp to manage retransmissions
   const uint8_t *password; ///<Peer's password
   size_t passwordLen;      ///<Length of the password in bytes
} PapFsm;


//PAP related functions
error_t papStartAuth(PppContext *context);
error_t papAbortAuth(PppContext *context);

void papTick(PppContext *context);

void papProcessPacket(PppContext *context,
   const PppPacket *packet, size_t length);

error_t papProcessAuthReq(PppContext *context,
   const PapAuthReqPacket *authReqPacket, size_t length);

error_t papProcessAuthAck(PppContext *context,
   const PapAuthAckPacket *authAckPacket, size_t length);

error_t papProcessAuthNak(PppContext *context,
   const PapAuthNakPacket *authNakPacket, size_t length);

error_t papSendAuthReq(PppContext *context);
error_t papSendAuthAck(PppContext *context, uint8_t identifier);
error_t papSendAuthNak(PppContext *context, uint8_t identifier);

bool_t papCheckPassword(PppContext *context, const char_t *password);

//C++ guard
#ifdef __cplusplus
   }
#endif

#endif
