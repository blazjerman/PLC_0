/**
 * @file network_adapter_g3.c
 * @brief Wrapper layer between cycloneTCP and G3-PLC
 *
 * @section License
 *
 * Copyright (c) 2019 Atmel Corporation. All rights reserved.
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
 
#include "net_config.h"
#define TRACE_LEVEL PPP_TRACE_LEVEL

#include <hal/hal.h>
#include "debug.h"
#include "Logger.h"
#ifdef __G3_GATEWAY__
#include "conf_bs.h"
#endif
#include "network_adapter_g3.h"

/* Activate packet dump */
/* #define DUMP_CONSOLE */

#ifdef __G3_GATEWAY__
extern bool coord_get_short_addr_by_ext (uint8_t *puc_extended_address, uint16_t *pus_short_address);
#endif


#define G3_ADP_MAX_DATA_LENGTH 1280

/* Global variables */
const NetBuffer*  mp_buffer = NULL;
size_t                mp_offset = 0;
uint8_t adp_sdu[G3_ADP_MAX_DATA_LENGTH];

/* Structure to define the G3 network adapter */
NicDriver g3_adapter = { NIC_TYPE_6LOWPAN,
						 1500,
						 Init,
						 Tick,
						 EnableIrq,
						 DisableIrq,
						 RxEventHandler,
						 ipv6_send_packet,
						 SetMacFilter,
						 NULL,
						 WritePhyReg,
						 ReadPhyReg,
						 TRUE,
						 TRUE,
						 TRUE,
						 TRUE };

/* G3 driver initialization */
error_t Init (NetInterface* interface)
{
	if (interface != NULL)
	{
		/* Force the TCP/IP stack to check the link state */
		interface->nicEvent = TRUE;
		osSetEvent(&netEvent);
		/* G3 is now ready to send */
		osSetEvent(&interface->nicTxEvent);
	}
	else
	{
		/* Only for MISRA C++ compliance */
	}

	return NO_ERROR;
}

/* RX routine */
void ipv6_receive_packet (struct TAdpDataIndication *pDataIndication)
{
#ifdef DUMP_CONSOLE
  LogDump(pDataIndication->m_pNsdu, pDataIndication->m_u16NsduLength);
#endif /* DUMP_CONSOLE */          
	//osAcquireMutex(&netInterface[0U].nicDriverMutex);
#ifdef __G3_GATEWAY__
	nicProcessPacket(&netInterface[1U/*G3_IF1*/], (uint8_t *)pDataIndication->m_pNsdu, pDataIndication->m_u16NsduLength);
#else
    nicProcessPacket(&netInterface[0U/*G3_IF0*/], (uint8_t *)pDataIndication->m_pNsdu, pDataIndication->m_u16NsduLength);
#endif        
	//osReleaseMutex(&netInterface[0U].nicDriverMutex);
}

/* TX routine */
error_t ipv6_send_packet (NetInterface*        interface,
		            		const NetBuffer* buffer,
		            		size_t               offset)
{
	static uint8_t uc_nsdu_handle = 0;
	error_t  sendPacketError;
	uint16_t us_data_length = netBufferGetLength(buffer) - offset;

	if(us_data_length <= G3_ADP_MAX_DATA_LENGTH) {
		(void)netBufferRead((uint8_t *)adp_sdu, buffer, offset, us_data_length);
#ifdef __G3_GATEWAY__
                /* Check if IPv6 Dest is ULA*/
                Ipv6Header *ipHeader;
                uint16_t us_short_addr;
                struct TAdpSetConfirm pSetConfirm;
                //Retrieve the length of the IPv6 packet
                ipHeader = netBufferAt(buffer, offset);
                if (coord_get_short_addr_by_ext(ipHeader->destAddr.b+8, &us_short_addr) == true) {
                  AdpSetRequestSync(ADP_IB_MANUF_IPV6_ULA_DEST_SHORT_ADDRESS, 0, sizeof(us_short_addr),(uint8_t*)&us_short_addr, &pSetConfirm);
                  AdpDataRequest(us_data_length, adp_sdu, uc_nsdu_handle++, true, 0x00);
		  sendPacketError = NO_ERROR;
                } else {
                  if ((ipHeader->destAddr.b[8] == ((uint8_t) (G3_COORDINATOR_PAN_ID >> 8))) 
                      &&(ipHeader->destAddr.b[9] == ((uint8_t) (G3_COORDINATOR_PAN_ID )))){
                    //ULA 2nd address
                    us_short_addr = ipHeader->destAddr.b[14] << 8;    
                    us_short_addr += ipHeader->destAddr.b[15];    
                    AdpSetRequestSync(ADP_IB_MANUF_IPV6_ULA_DEST_SHORT_ADDRESS, 0, sizeof(us_short_addr),(uint8_t*)&us_short_addr, &pSetConfirm);
                    AdpDataRequest(us_data_length, adp_sdu, uc_nsdu_handle++, true, 0x00);
                    sendPacketError = NO_ERROR;
                  } else {
                    TRACE_ERROR("DEST IP ADDRESS not in BS database!\r\n");
                    sendPacketError = ERROR_ADDRESS_NOT_FOUND;
                  }
                }
#else
#ifdef DUMP_CONSOLE
                LogDump(adp_sdu, us_data_length);
#endif /* DUMP_CONSOLE */                   
                AdpDataRequest(us_data_length, adp_sdu, uc_nsdu_handle++, true, 0x00);               
		sendPacketError = NO_ERROR;
#endif  		
	} else {
		sendPacketError = ERROR_WRONG_LENGTH;
	}

	osSetEvent(&interface->nicTxEvent);
	return sendPacketError;
}

/* Empty callbacks (functionality not needed within G3 network adapter) */
void Tick (NetInterface* interface)
{
	(void)interface;
}

void RxEventHandler (NetInterface* interface)
{
	(void)interface;
}

void EnableIrq (NetInterface* interface)
{
	(void)interface;
}

void DisableIrq (NetInterface* interface)
{
	(void)interface;
}

error_t SetMacFilter (NetInterface* interface)
{
	(void)interface;
	return NO_ERROR;
}

void WritePhyReg (uint8_t opcode, uint8_t  phyAddr, uint8_t  regAddr, uint16_t data)
{
	(void)opcode;
	(void)phyAddr;
	(void)regAddr;
	(void)data;
}

uint16_t ReadPhyReg (uint8_t opcode, uint8_t phyAddr, uint8_t regAddr)
{
	(void)opcode;
	(void)phyAddr;
	(void)regAddr;
	return 0U;
}
