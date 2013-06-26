
#ifndef RTCPPACKET_H
#define RTCPPACKET_H

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

//static const char* const RtcpPacket_hxx_Version =
//    "$Id: RtcpPacket.h,v 1.3 2008-01-15 06:33:23 kennylin Exp $";


#include "rtpTypes.h"
/*#include "Rtcp.h"*/

#define PACKETSIZE 1480

typedef enum
{
	PKT_FREE = 0,
	PKT_ALLOC
}PKTSTATE;

/// Compound RTCP control packet
typedef struct stRtcpPacket
{
	/* Default RTCP packet size */
/*	static const int PACKETSIZE = 2048; */

	/* Pointer to raw packet memory */
	char* packetData;
	char packetbuf[PACKETSIZE];

	/* Allocated raw packet memory size */
	int packetAlloc;

	/* Amount of unused packet memory */
	int unusedSize;

	/// Packet state 
	PKTSTATE packetState;
	
	uint32 chid;
	uint32 sid;

}RtcpPacket;

void RtcpPkt_Init (RtcpPacket* pst);

void RtcpPkt_Exit (void);

/* Pointer to beginning of memory of packet */
char* RtcpPkt_getPacketData (RtcpPacket* pst);

/** Pointer to beginning of free mememory of packet.
	Must call allocData() afterwards saying how much you used
 **/
char* RtcpPkt_freeData (RtcpPacket* pst);

/* Increases packet memory usage by newSize. Returns newSize */
int RtcpPkt_allocData (RtcpPacket* pst, int s);

/* Total mem allocated in packet */
int RtcpPkt_getPacketAlloc (RtcpPacket* pst);

/* Unused memory in payload area */
int RtcpPkt_getUnused (RtcpPacket* pst);

/** Sets total unused memory in packet.
	Useful when receiving packet into buffer
**/
void RtcpPkt_setTotalUsage (RtcpPacket* pst, int size);

/** Total unused memory in packet.
	Useful when transmitting packet
**/
int RtcpPkt_getTotalUsage (RtcpPacket* pst);

int RtcpPkt_getVersion (RtcpPacket* pst);

int RtcpPkt_getPadbyteFlag (RtcpPacket* pst);

int RtcpPkt_getCount (RtcpPacket* pst);

RtcpType RtcpPkt_getPayloadType (RtcpPacket* pst);

int RtcpPkt_getLength (RtcpPacket* pst);

#endif // RTCPPACKET_H
