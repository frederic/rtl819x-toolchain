
#ifndef RTCPTRANSMITTER_H
#define RTCPTRANSMITTER_H

/* ====================================================================
 * The Vovida Software License, Version 1.0 
 * 
 * Copyright (c) 2000 Vovida Networks, Inc.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 
 * 3. The names "VOCAL", "Vovida Open Communication Application Library",
 *    and "Vovida Open Communication Application Library (VOCAL)" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact vocal@vovida.org.
 *
 * 4. Products derived from this software may not be called "VOCAL", nor
 *    may "VOCAL" appear in their name, without prior written
 *    permission of Vovida Networks, Inc.
 * 
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL VOVIDA
 * NETWORKS, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT DAMAGES
 * IN EXCESS OF $1,000, NOR FOR ANY INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * 
 * ====================================================================
 * 
 * This software consists of voluntary contributions made by Vovida
 * Networks, Inc. and many individuals on behalf of Vovida Networks,
 * Inc.  For more information on Vovida Networks, Inc., please see
 * <http://www.vovida.org/>.
 *
 */

//static const char* const RtcpTransmitter_hxx_Version =
//    "$Id: RtcpTransmitter.h,v 1.3 2008-01-15 06:33:23 kennylin Exp $";


/*#include "Rtcp.h"*/

/*#include <sys/types.h>*/

#include "rtpTypes.h"

/*#include "rtcpTypes.h"*/
/*#include <UdpStack.h>
#include <NetworkAddress.h>*/

/*class RtcpReceiver;
class RtcpPacket;*/

/* Kao added -- move from class */
/// Transmission interval in ms
/*const int RTCP_INTERVAL = 5000;*/
/*#define RTCP_INTERVAL 5000 */
#if 0
#define RTCP_INTERVAL 10000
#endif

#if 0
///Struct to transmit RTCP packets
typedef struct stRtcpTransmitter
{

	/* Next time to submit RTCP packet */
	NtpTime nextInterval;

	/** Transmitter SDES information
	 *  data stored as null-term strings
	**/
	SDESdata* SDESInfo;
	SDESdata SDESdataBuf[1];

	RtpTransmitter* tran;

	RtpReceiver* recv;

	RtcpReceiver* rtcpRecv;

	/* my UDP stack */
/*	UdpStack* myStack; */
	int localPort;
	int remotePort;

/*	NetworkAddress remoteAddr; */
}RtcpTransmitter;
#endif

void RtcpTx_Init (void);
void RtcpTx_InitByID (uint32 sid, uint32 enableXR, uint32 txInterval);

/* Deconstructor */
//void RtcpTx_Close (void);

RtcpTransmitter* RtcpTx_getInfo (uint32 sid);

/** Transmits RTCP packet to remoteHost/port.
	Doesn't remove packet from memory.
	Returns -1 failure or number of bytes sent on success
**/

int RtcpTx_transmitRTCP (uint32 sid);
int RtcpTx_transmitRTCPBYE (uint32 sid);
int RtcpTx_transmit (RtcpPacket* packet);

/* set timer to next interval */
void RtcpTx_updateInterval (uint32 sid);

/* Check if time to send RTCP packet.  Returns 1 then time up */
int RtcpTx_checkInterval (uint32 sid);

int RtcpTx_checkIntervalRTCP (uint32 sid);

/* Adds SR packet into compound packet */
int RtcpTx_addSR (uint32 sid, RtcpPacket* packet, int npadSize);

/* Adds specificed SDES item to compound packet */
int RtcpTx_addSDES_Item (uint32 sid, RtcpPacket* packet, RtcpSDESType item, int npadSize);

/* Adds all known SDES items to compound packet */
int RtcpTx_addSDES (uint32 sid, RtcpPacket* packet, int npadSize);

/* Adds specificed SDES items in SDESlist, which ends with RTCP_SDES_END */
int RtcpTx_addSDES_List (uint32 sid, RtcpPacket* packet, RtcpSDESType* SDESlist, int npadSize);

/** Adds BYE packet using transmitter's SRC numbers
	@param reason optional text, null-term
**/
int RtcpTx_addBYE (uint32 sid, RtcpPacket* packet, char* reason, int npadSize);

/** Adds BYE packet using specified list of SRC numbers
	@param reason optional text, null-term
	@param count number of items in list
**/
int RtcpTx_addBYE_List (RtcpPacket* packet, RtpSrc* list, int count, char* reason, int npadSize);

/* future: not implemented */
int RtcpTx_addAPP (RtcpPacket* packet, int newpadbyeSize);


/* Used for calculating RR information */
u_int32_t RtcpTx_calcLostFrac (RtpTranInfo* source);

/* Used for calculating RR information */
u_int32_t RtcpTx_calcLostCount (RtpTranInfo* source);

void RtcpTx_setSdesCname (uint32 sid);

void RtcpTx_setSdesName (uint32 sid, char* text);

void RtcpTx_setSdesEmail (uint32 sid, char* text);

void RtcpTx_setSdesPhone (uint32 sid, char* text);

void RtcpTx_setSdesLoc (uint32 sid, char* text);

void RtcpTx_setSdesTool (uint32 sid, char* text);

void RtcpTx_setSdesNote (uint32 sid, char* text);

char* RtcpTx_getSdesCname (uint32 sid);

char* RtcpTx_getSdesName (uint32 sid);

char* RtcpTx_getSdesEmail (uint32 sid);

char* RtcpTx_getSdesPhone (uint32 sid);

char* RtcpTx_getSdesLoc (uint32 sid);

char* RtcpTx_getSdesTool (uint32 sid);

char* RtcpTx_getSdesNote (uint32 sid);

void RtcpTx_setRTPtran (uint32 sid, RtpTransmitter* s);
void RtcpTx_setRTPrecv (uint32 sid, RtpReceiver* s);
void RtcpTx_setRTCPrecv (uint32 sid, RtcpReceiver* s);

/* Port this tramsitter is sending it singal */
int RtcpTx_getPort (uint32 sid);

void RtcpTx_setMode(uint32 sid, RtpTransmitMode opmode);

/*NetworkAddress* RtcpTx_getRemoteAddr (uint32 sid);

void RtcpTx_setRemoteAddr (uint32 sid, const NetworkAddress& theAddr) ;
*/

extern RtcpPacket RTCP_TX_DEC[RTCP_TX_DEC_NUM];

#endif // RTCPTRANSMITTER_H
