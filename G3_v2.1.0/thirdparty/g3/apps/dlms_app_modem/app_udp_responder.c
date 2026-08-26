/**
 * \file
 *
 * \brief UDP responder application, required in G3 Conformance test
 *
 * Copyright (c) 2018 Atmel Corporation. All rights reserved.
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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "compiler.h"

#include "app_udp_responder.h"
#include "Logger.h"
#include "error.h"

#include "app_adp_mng.h"
#include "ipv6_mng.h"
#include "drivers/g3/network_adapter_g3.h"
#include "async_ping.h"

#ifdef DLMS_DEBUG_CONSOLE
#       define LOG_APP_DEBUG(a)   printf a
#else
#       define LOG_APP_DEBUG(a)   (void)0
#endif

/* Vars for conformance socket communication */
/* Socket for the UDP over PLC communication */
Socket *spx_conf_socket;
/* Local IP address */
IpAddr sx_conf_local_ip_addr;
/* Local Socket PORT */
uint16_t sus_conf_udp_port;
/* Socket opened */
bool b_is_conf_socket_open = false;

extern uint8_t g_u8NetworkJoinStatus;
extern bool b_ipv6_initialized;

/* Maximum length of IPv6 PDUs for DLMS application */
#define MAX_LENGTH_IPv6_PDU 1200

uint8_t udp_buffer[MAX_LENGTH_IPv6_PDU]; /* Serial data buffer */
size_t u16_length = 0;

/**
 * \brief Process UDP message for G3-PLC Conformance test
 */
static void _udp_responder_process_message(uint8_t *pUdpPayload, uint16_t u16UdpPayloadLength)
{
	LOG_APP_DEBUG(("[UDP_RESPONDER] _udp_responder_process_message()"));
	bool x_error;
	size_t x_transmitted_bytes;
	uint8_t u8ConfigResult;
	IpAddr multicastIpAddr;

	if (u16UdpPayloadLength > 0) {
		if (pUdpPayload[0] == 0x01) {
			/*
			 * In order to validate RFC6282 UDP header compression, an exchange of frames transported over UDP is required. For this purpose a very
			 * simple UDP responder needs to be implemented.
			 * - The device listen to port 0xF0BF over UDP.
			 * - The first byte of UDP payload indicate the message type, the rest of the UDP payload correspond to the message data:
			 * - 0x01(UDP request): upon reception, the device must send back an UDP frame to the original sender, using the received frame source
			 * and destination ports for the destination and source
			 * ports (respectively) of the response frame, setting the message type to 0x02 (UDP reply) and copying the message data from the
			 * request;
			 * - 0x02 (UDP reply): this message is dropped upon reception;
			 * - other value: this message is dropped upon reception;
			 */

			/* UDP responder needed for conformance testing */
			/* update UPD payload */
			pUdpPayload[0] = 0x02;

			udp_socket_send(spx_conf_socket, &sx_conf_local_ip_addr, sus_conf_udp_port, pUdpPayload, u16UdpPayloadLength, &x_transmitted_bytes);
		} else if (pUdpPayload[0] == 0x03) {
			/*
			 * The following extension is added to the UDP responder, in order to make the IUT generate ICMPv6
			 * ECHO Request frames.
			 * The new message type 0x03 (ICMPv6 ECHO request trigger) is added: upon reception, the device
			 * must send back an ICMPv6 ECHO request frame to the original sender. The ICMPv6 Identifier,
			 * Sequence Number and Data fields are filled (in that order) using the received message data.
			 * Example: If an UDP message with a payload of ?03 010203040506070809? is received, then an
			 * ICMPv6 echo request is sent back with an ICMPv6 content of ?80 00 xxxx 0102 0304 0506070809?
			 * (where xxxx correspond to the ICMP checksum).
			 */

			x_error = async_ping_send_with_content(&netInterface[0], &sx_conf_local_ip_addr, u16UdpPayloadLength - 1, &pUdpPayload[1], CONFORMANCE_PING_TTL,
					CONFORMANCE_PING_TTL * 1000);

			if (x_error == NO_ERROR) {
				LOG_APP_DEBUG(("\r\n[UDP_RESPONDER] Ping Sent ok!\r\n"));
				/* ul_pings_sent += 1; */
				/* uc_ping_sent = 1; */
			} else {
				LOG_APP_DEBUG(("\r\n[UDP_RESPONDER] Fail sending Ping!  Error: %d \r\n", x_error));
			}
		} else if (pUdpPayload[0] == 0x04) {
			/*
			 * Multicast traffic trigger
			 * Upon reception, the device must send an UDP frame to the
			 * ff02::1 multicast address, using the received frame source and destination ports for the
			 * destination and source ports (respectively) of the response frame, setting the message type to
			 * 0x02 (UDP reply) and copying the message data from the request
			 */

			/* UDP responder needed for conformance testing */
			/* update UPD payload */
			pUdpPayload[0] = 0x02;

			/* Send to multicast group */
			multicastIpAddr.length = sizeof(Ipv6Addr);
			ipv6StringToAddr(APP_IPV6_MULTICAST_ADDR_0, &multicastIpAddr.ipv6Addr);
			udp_socket_send(spx_conf_socket, &multicastIpAddr, sus_conf_udp_port, pUdpPayload, u16UdpPayloadLength, &x_transmitted_bytes);
		} else if (pUdpPayload[0] == 0x05) {
			/*
			 * The following extension is added to the UDP responder, in order to make the IUT change his RF
			 * configuration.
			 * The new message type 0x05 (RF configuration change) and 0x06 (RF configuration confirmation)
			 * are added: upon reception of a RF configuration change message, the device must change its
			 * configuration according to the configuration indicated in the received message, then send back
			 * an RF configuration confirmation indicating the status of the change.
			 * RF configuration change (type 0x05): The configuration is defined using 3 or 4
			 * bytes, directly following the message type identifier:
			 *   - The frequency band (1 byte), corresponding to the parameter macFrequencyBand_RF defined in [1]
			 *   - The mode (1 byte), corresponding to the parameter macOperatingMode_RF defined in [1]
			 *   - The frequency hopping activation (1 byte). The following 2 values are possible:
			 *     - 0x00: Indicate that the frequency hopping mechanism is deactivated, and so that the single
			 *       carrier mode is used. In this case, the channel number that has to be used will be indicated by
			 *       the next byte
			 *     - 0x01: Indicate that the frequency hopping mechanism is activated (this case is not yet possible
			 *       as the frequency hopping mechanism has not been specified)
			 *   - If (frequency hopping activation == 0x00), the channel number (1 byte) is included,
			 *     corresponding to the parameter macChannelNumber_RF defined in [1]
			 * RF configuration confirmation (type 0x06): The status is defined by one byte directly following the
			 * message type identifier. The following values are possible:
			 *   - 0x00, indicating that the RF configuration is successful
			 *   - 0x01, indicating that the RF configuration failed
			 *   - 0x02, indicating that the requested RF configuration is unsupported by the device
			 */

			LOG_APP_DEBUG(("\r\n[UDP_RESPONDER] Change RF config request\r\n"));
			u8ConfigResult = change_conformance_rf_config(&pUdpPayload[1], u16UdpPayloadLength - 1);

			/* u8ConfigResult contains the response byte to send back as defined above */
			pUdpPayload[0] = 0x06;
			pUdpPayload[1] = u8ConfigResult;
			u16UdpPayloadLength = 2;
			udp_socket_send(spx_conf_socket, &sx_conf_local_ip_addr, sus_conf_udp_port, pUdpPayload, u16UdpPayloadLength, &x_transmitted_bytes);
		} else if (pUdpPayload[0] == 0x07) {
			/*
			 * Trickle/Clusterhead/Jitter control request (type 0x07): upon reception, the device must change its
			 * configuration to set the following parameters and send back the Trickle/Clusterhead/Jitter control
			 * confirmation message (type 0x08) to confirm the activation. The configuration to apply will be
			 * defined by one byte directly following the message type identifier:
			 *   - 0x00 (deactivation):
			 *   - 0x01 (activation):
			 * Trickle/Clusterhead/Jitter control confirmation (type 0x08): this message is dropped upon reception.
			 * The status is defined by one byte directly following the message type identifier. The following
			 * values are possible:
			 *   - 0x00, indicating that the requested change is successful
			 *   - 0x01, indicating that the requested change failed
			 */

			LOG_APP_DEBUG(("\r\n[UDP_RESPONDER] Trickle/Cluster/Jitter activation/deactivation\r\n"));
			u8ConfigResult = change_conformance_trickle_config(&pUdpPayload[1]);

			/* u8ConfigResult contains the response byte to send back as defined above */
			pUdpPayload[0] = 0x08;
			pUdpPayload[1] = u8ConfigResult;
			u16UdpPayloadLength = 2;
			udp_socket_send(spx_conf_socket, &sx_conf_local_ip_addr, sus_conf_udp_port, pUdpPayload, u16UdpPayloadLength, &x_transmitted_bytes);
		} else if (pUdpPayload[0] == 0x09) {
			/*
			 * Upon reception of a “start RF continuous TX mode” message, the DUT shall activate the
			 * transmission of a D-M1 signal as defined in ETSI EN 303 204 v3.1.1 §5.2.7. This
			 * unmodulated signal is emitted continuously, at the center frequency of the current band and
			 * channel which can be selected using “Change RF configuration” message (0x05). If the
			 * DUT is in a “Frequency hopping” mode, it shall use channel 0 for the continuous TX mode.
			 * This transmission shall be stopped when rebooting the DUT: after power-up, the DUT shall
			 * recover its normal behaviour.
			 */

			LOG_APP_DEBUG(("\r\n[UDP_RESPONDER] RF continuous tx request\r\n"));
			u8ConfigResult = set_rf_continuous_tx();
		} else if (pUdpPayload[0] == 0x0A) {
			/*
			 * LastGasp mode activation
			 * Upon reception of this message, adpLastGasp is set to TRUE
			 * by the IUT, then an ICMPv6 Echo request is sent to multicast address ff02::1 by the IUT
			 * (required to have the IUT generate a broadcast message on activation)
			 * No confirmation message is defined or required. Validation of the LastGasp feature uses
			 * normal ICMPv6 Echo messages, no further application layer modifications are required
			 * Disabling LastGasp mode will be done by rebooting the DUT: after power-up, the DUT
			 * shall start with adpLastGasp set to FALSE, as defined in G3-PLC specification
			 */

			LOG_APP_DEBUG(("\r\n[UDP_RESPONDER] Last Gasp activation request\r\n"));
			set_last_gasp_mode();
			/* Send back multicast ICMPv6 */
			multicastIpAddr.length = sizeof(Ipv6Addr);
			ipv6StringToAddr(APP_IPV6_MULTICAST_ADDR_0, &multicastIpAddr.ipv6Addr);
			x_error = async_ping_send(&netInterface[0], &multicastIpAddr, 10, CONFORMANCE_PING_TTL,	CONFORMANCE_PING_TTL * 1000);
		} else {
			LOG_APP_DEBUG(("[UDP_RESPONDER] Drop UDP message: invalid protocol"));
		}
	} else {
		LOG_APP_DEBUG(("[UDP_RESPONDER] Drop UDP message: invalid header / length"));
	}
}

/**
 * \brief Timers of UDP Responder Application
 *
 */
void udp_responder_app_timers_update(void)
{
}

/**
 * \brief Process of UDP Responder Application
 *
 */
void udp_responder_app_process(void)
{
	if (ipv6_mng_ready()) {
		if (b_is_conf_socket_open && udp_socket_is_open(spx_conf_socket)) {
			/* Process UDP messages for conformance */
			if (udp_socket_receive(spx_conf_socket, &sx_conf_local_ip_addr, &sus_conf_udp_port, udp_buffer, MAX_LENGTH_IPv6_PDU, &u16_length)) {
				if (u16_length > 0) {
					_udp_responder_process_message(udp_buffer, u16_length);
				}
			}
		} else {
			/* Open sockets */
			b_is_conf_socket_open = udp_socket_open(spx_conf_socket, CONFORMANCE_SOCKET_PORT);
		}
	}
}

/**
 * \brief Initialize UDP Responder Application
 *
 */
void udp_responder_app_init(void)
{
	/* Initialize socket */
	spx_conf_socket = udp_socket_initialize();
}
