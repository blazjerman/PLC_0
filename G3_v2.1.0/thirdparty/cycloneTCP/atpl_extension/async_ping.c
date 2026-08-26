/**
 * \file
 *
 * \brief Asyncronous Ping Implementation. Based on Ping.c from Oryx CycloneTCP Open.
 *
 * Copyright (c) 2016 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

//Dependencies
#include "core/net.h"
#include "async_ping.h"
#include "core/ip.h"
#include "ipv4/ipv4.h"
#include "ipv4/icmp.h"
#include "ipv6/ipv6.h"
#include "ipv6/ipv6_misc.h"
#include "ipv6/icmpv6.h"
#include "core/socket.h"
#include "core/net_mem.h"
#include "debug.h"
//Sequence number field
static uint16_t pingSequenceNumber = 0;

struct s_ping_status_t{
      uint8_t b_free;
      NetInterface *interface;
      IcmpEchoMessage *message;
      const IpAddr *ipAddr;
      size_t size;
      uint8_t ttl;
      systime_t timeout;
      systime_t startTime;
      uint16_t identifier;
      uint16_t sequenceNumber;
      Socket *socket;
};

static struct s_ping_status_t s_ping_status;

/**
 * @brief Initialize Module
 *
 * Ping operates by sending an ICMP Echo Request message to the
 * target host and waiting for an ICMP Echo Reply message
 *
 **/
void async_ping_init(void){
	s_ping_status.b_free = 1;
}




/**
 * @brief Sends the Ping Message
 *
 * Ping operates by sending an ICMP Echo Request message to the
 * target host and waiting for an ICMP Echo Reply message
 *
 * @param[in] interface Underlying network interface (optional parameter)
 * @param[in] ipAddr IP address of the host to reach
 * @param[in] size Size of the ping packet in bytes
 * @param[in] ttl Time-To-Live value to be used
 * @param[in] timeout Maximum time to wait before giving up
 * @return Error code
 **/

error_t async_ping_send(NetInterface *interface, const IpAddr *ipAddr,
   size_t size, uint8_t ttl, systime_t timeout)
{
    return async_ping_send_with_content(interface, ipAddr, size, NULL, ttl, timeout);
}


/**
 * @brief Sends the Ping Message with the specified content
 *
 * Ping operates by sending an ICMP Echo Request message to the
 * target host and waiting for an ICMP Echo Reply message
 *
 * @param[in] interface Underlying network interface (optional parameter)
 * @param[in] ipAddr IP address of the host to reach
 * @param[in] size Size of the ping packet in bytes
 * @param[in] ttl Time-To-Live value to be used
 * @param[in] timeout Maximum time to wait before giving up
 * @return Error code
 **/

error_t async_ping_send_with_content(NetInterface *interface, const IpAddr *ipAddr,
   size_t size, uint8_t *content, uint8_t ttl, systime_t timeout)
{
   error_t error;
   uint_t i;
   size_t length;



   if( s_ping_status.b_free){

	   //Save input information
	   s_ping_status.b_free = 0;
	   s_ping_status.interface = interface;
	   s_ping_status.ipAddr = ipAddr;
	   s_ping_status.size = size;
	   s_ping_status.ttl = ttl;
	   s_ping_status.size = size;
	   s_ping_status.timeout = timeout;

	   //Limit the size of the ping request
	   size = MIN (size, PING_MAX_DATA_SIZE);

	   //Debug message
	   TRACE_INFO("Pinging %s with %u bytes of data...\r\n",
		  ipAddrToString(ipAddr, NULL), size);

	   //Length of the complete ICMP message including header and data
	   length = sizeof(IcmpEchoMessage) + size;

	   //Allocate memory buffer to hold an ICMP message
	   s_ping_status.message = memPoolAlloc(length);
	   //Failed to allocate memory?
	   if(!s_ping_status.message) return ERROR_OUT_OF_MEMORY;

	   //Identifier field is used to help matching requests and replies
	   s_ping_status.identifier = netGetRand();
	   //Sequence Number field is increment each time an Echo Request is sent
	   s_ping_status.sequenceNumber = pingSequenceNumber++;

	   //Format ICMP Echo Request message
	   s_ping_status.message->type = ICMP_TYPE_ECHO_REQUEST;
	   s_ping_status.message->code = 0;
	   s_ping_status.message->checksum = 0;
	   s_ping_status.message->identifier = s_ping_status.identifier;
	   s_ping_status.message->sequenceNumber = s_ping_status.sequenceNumber;

	   //Copy or generate data
           if(content != NULL){
             memcpy(s_ping_status.message->data, content, size);
           }
           else{
             for(i = 0; i < size; i++)
                     s_ping_status.message->data[i] = i & 0xFF;
           }

	#if (IPV4_SUPPORT == ENABLED)
	   //Target address is an IPv4 address?
	   if(ipAddr->length == sizeof(Ipv4Addr))
	   {
		  Ipv4Addr srcIpAddr;

		  //Select the source IPv4 address and the relevant network
		  //interface to use when pinging the specified host
		  error = ipv4SelectSourceAddr(&interface, ipAddr->ipv4Addr, &srcIpAddr);

		  //Any error to report?
		  if(error)
		  {
			 //Free previously allocated memory
			 memPoolFree(s_ping_status.message);
			//Free status flag
			s_ping_status.b_free = 1;
			 //Return the corresponding error code
			 return error;
		  }

		  //ICMP Echo Request message
		  s_ping_status.message->type = ICMP_TYPE_ECHO_REQUEST;
		  //Message checksum calculation
		  s_ping_status.message->checksum = ipCalcChecksum(s_ping_status.message, length);

		  //Open a raw socket
		  s_ping_status.socket = socketOpen(SOCKET_TYPE_RAW_IP, SOCKET_IP_PROTO_ICMP);
	   }
	   else
	#endif
	#if (IPV6_SUPPORT == ENABLED)
	   //Target address is an IPv6 address?
	   if(ipAddr->length == sizeof(Ipv6Addr))
	   {
		  Ipv6PseudoHeader pseudoHeader;

		  //Select the source IPv6 address and the relevant network
		  //interface to use when pinging the specified host
		  error = ipv6SelectSourceAddr(&interface, &ipAddr->ipv6Addr, &pseudoHeader.srcAddr);

		  //Any error to report?
		  if(error)
		  {
			 //Free previously allocated memory
			 memPoolFree(s_ping_status.message);
			//Free status flag
			s_ping_status.b_free = 1;
			 //Return the corresponding error code
			 return error;
		  }

		  //ICMPv6 Echo Request message
		  s_ping_status.message->type = ICMPV6_TYPE_ECHO_REQUEST;
		  //Format IPv6 pseudo header
		  pseudoHeader.destAddr = ipAddr->ipv6Addr;
		  pseudoHeader.length = htonl(length);
		  pseudoHeader.reserved = 0;
		  pseudoHeader.nextHeader = IPV6_ICMPV6_HEADER;

		  //Message checksum calculation
		  s_ping_status.message->checksum = ipCalcUpperLayerChecksum(
			 &pseudoHeader, sizeof(Ipv6PseudoHeader), s_ping_status.message, length);

		  //Open a raw socket
		  s_ping_status.socket = socketOpen(SOCKET_TYPE_RAW_IP, SOCKET_IP_PROTO_ICMPV6);
	   }
	   else
	#endif
	   //Target address is not valid?
	   {
		  //Free previously allocated memory
		  memPoolFree(s_ping_status.message);
		  //Free status flag
		  s_ping_status.b_free = 1;
		  //Report an error
		  return ERROR_INVALID_ADDRESS;
	   }

	   //Failed to open socket?
	   if(!s_ping_status.socket)
	   {
		  //Free previously allocated memory
		  memPoolFree(s_ping_status.message);
		  //Free status flag
		  s_ping_status.b_free = 1;
		  //Report an error
		  return ERROR_OPEN_FAILED;
	   }

	   //Set the TTL value to be used
	   s_ping_status.socket->ttl = ttl;

	   //Associate the newly created socket with the relevant interface
	   error = socketBindToInterface(s_ping_status.socket, interface);

	   //Unable to bind the socket to the desired interface?
	   if(error)
	   {
		  //Free previously allocated memory
		  memPoolFree(s_ping_status.message);
		  //Close socket
		  socketClose(s_ping_status.socket);
		  //Free status flag
		  s_ping_status.b_free = 1;
		  //Return status code
		  return error;
	   }

	   //Connect the socket to the target host
	   error = socketConnect(s_ping_status.socket, ipAddr, 0);

	   //Any error to report?
	   if(error)
	   {
		  //Free previously allocated memory
		  memPoolFree(s_ping_status.message);
		  //Close socket
		  socketClose(s_ping_status.socket);
		  //Free status flag
		  s_ping_status.b_free = 1;
		  //Return status code
		  return error;
	   }

	   //Send Echo Request message
	   error = socketSend(s_ping_status.socket, s_ping_status.message, length, NULL, 0);

	   //Failed to send message ?
	   if(error)
	   {
		  //Free previously allocated memory
		  memPoolFree(s_ping_status.message);
		  //Close socket
		  socketClose(s_ping_status.socket);
		  //Free status flag
		  s_ping_status.b_free = 1;
		  //Return status code
		  return error;
	   }

	   //Save the time at which the request was sent
	   s_ping_status.startTime = osGetSystemTime();
   }else{
	   TRACE_DEBUG("async_ping_send called but previous send in progress\r\n");
	   //Free previously allocated memory
	  memPoolFree(s_ping_status.message);
	  //Close socket
	  socketClose(s_ping_status.socket);

	   //Free status flag
	   s_ping_status.b_free = 1;
	   return ERROR_IN_PROGRESS;
   }
   return NO_ERROR;
}



/**
 * @brief Checks if Reply has been received,
 *
 * Ping operates by sending an ICMP Echo Request message to the
 * target host and waiting for an ICMP Echo Reply message
 *
 * @param[out] rtt Round-trip time (optional parameter)
 * @return Error code; ERROR_TIMEOUT is returned when queue is empty
 **/

error_t async_ping_rcv( systime_t *rtt)
{

   uint_t i;
   systime_t roundTripTime;
   IcmpEchoMessage *message;
   error_t error;
   size_t length;

   message = s_ping_status.message;

   //Timeout value exceeded?
   if((osGetSystemTime() - s_ping_status.startTime) < s_ping_status.timeout)
   {
      //Adjust receive timeout
      error = socketSetTimeout(s_ping_status.socket, s_ping_status.timeout);
      //Any error to report?
      if(error) {
		//Free previously allocated memory
		memPoolFree(message);
		//Free status flag
		s_ping_status.b_free = 1;
		return error;
      }

      //Wait for an incoming ICMP message
      error = socketReceive(s_ping_status.socket, message, sizeof(IcmpEchoMessage) + s_ping_status.size, &length, SOCKET_FLAG_DONT_WAIT);
      //Any error to report?
      if( error ){

    	if (error != ERROR_TIMEOUT){// ERROR_TIMEOUT means empty reception --> is not an error
    		//Free previously allocated memory
			memPoolFree(message);
			//Free status flag
			s_ping_status.b_free = 1;
		}
		return error;
	  }

      //Check message length
      if(length != (sizeof(IcmpEchoMessage) + s_ping_status.size)){
      	//Treat this as if no response is received yet and keep waiting
		return ERROR_TIMEOUT;
	  }
      //Verify message type
      if(s_ping_status.ipAddr->length == sizeof(Ipv4Addr) && message->type != ICMP_TYPE_ECHO_REPLY){
        //Treat this as if no response is received yet and keep waiting
  		return ERROR_TIMEOUT;
	  }
      if(s_ping_status.ipAddr->length == sizeof(Ipv6Addr) && message->type != ICMPV6_TYPE_ECHO_REPLY){
        //Treat this as if no response is received yet and keep waiting
  		return ERROR_TIMEOUT;
	  }
      //Response identifier matches request identifier?
      if(message->identifier != s_ping_status.identifier){
        //Treat this as if no response is received yet and keep waiting
  		return ERROR_TIMEOUT;
	  }
      //Make sure the sequence number is correct
      if(message->sequenceNumber != s_ping_status.sequenceNumber){
        //Treat this as if no response is received yet and keep waiting
  		return ERROR_TIMEOUT;
	  }

      //Loop through data field
      for(i = 0; i < s_ping_status.size; i++)
      {
         //Compare received data against expected data
         if(message->data[i] != (i & 0xFF)) break;
      }

      //Valid Echo Reply message received?
      if(i == s_ping_status.size)
      {
         //Calculate round-trip time
         roundTripTime = osGetSystemTime() - s_ping_status.startTime;
         //Debug message
         TRACE_INFO("Echo received (round-trip time = %" PRIu32 " ms)...\r\n", roundTripTime);

         //Free previously allocated memory
        memPoolFree(message);
         //Close socket
         socketClose(s_ping_status.socket);

         //Return round-trip time
         if(rtt != NULL)
            *rtt = roundTripTime;

		 //Free status flag
		 s_ping_status.b_free = 1;
         //No error to report
         return NO_ERROR;
      }
   }else{

	   //Debug message
	   TRACE_INFO("No echo received!\r\n");
	   //Free previously allocated memory
	  memPoolFree(message);
	   //Close socket
	   socketClose(s_ping_status.socket);

	   //Free status flag
	   s_ping_status.b_free = 1;
	   //No Echo Reply received from host...
	   return ERROR_NO_RESPONSE;
   }

	return NO_ERROR;
}
