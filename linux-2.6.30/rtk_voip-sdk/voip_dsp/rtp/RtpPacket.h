#ifndef RTPPACKET_H
#define RTPPACKET_H

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

//static const char* const RtpPacket_hxx_Version =
//    "$Id: RtpPacket.h,v 1.3 2008-01-15 06:33:23 kennylin Exp $";

#include "rtpTypes.h"
#include  "assert.h"

#define PKTDATA_SIZES	1480	//1514 - 14 - 20
#define RECV_BUF    1012
//#define RTP_DEC_NUM 32//16	/* use RTP_xxx_DEC_NUM and RTCP_xxx_DEC_NUM to replace this one. */

/* In our observation, numbers of follows can possibly be one. */
#define RTP_RX_DEC_NUM		2	/* It has to be power of 2 (cur_rx, cur_get) */
#define RTP_TX_DEC_NUM		2	/* It has to be power of 2 (cur_send) */
#define RTP_TX_DTMF_NUM		2	/* It has to be power of 2 (cur_send_dtmf) */

#define RTCP_RX_DEC_NUM		2	/* It has to be power of 2 (rtcp_cur_rx[], rtcp_cur_get[]) */
#define RTCP_TX_DEC_NUM		2	/* It has to be power of 2 (rtcp_cur_send) */

typedef enum
{
	OWNBYDSP = 0,
	OWNBYRTP
}RTPOWN;

/// Data struture for RTP data packet
typedef struct stRtpPacket	// RtpPacket
{
	/// flags
	BOOL sequenceSet;
	BOOL timestampSet;

	/// Pointer to raw memory allocation
	char* packetData;
	char packetbuf[PKTDATA_SIZES];

	/// Allocated packet size
	int packetAlloc;

	/// Size of unused memory
	int unusedSize;
	
	/// Channel ID of RTP Packet
	int chid;
	
	/// Session ID of RTP Packet
	int sid; 

	/// Own by 
	RTPOWN own;

	/// Easy access to header information
	RtpHeader* header;
	
	int RFC2833;	// RFC2833 contains DTMF digits, tones and signals. 
	
	int EventPktMarker;

	int EventPktBody;

	int EventPktEdge;
	
	int EventPktDuration;
	
	// RX only 
	int ip_ttl;		// TTL in IP header  

}RtpPacket;

void RtpPacket_Init (RtpPacket* pst, int newpayloadSize, int npadSize, int csrc_count);
void RtpPacket_Exit (RtpPacket* pst);

extern char* getPacketData (RtpPacket* pst);
extern RtpHeader* getHeader (RtpPacket* pst);
extern char* getPayloadLoc (RtpPacket* pst);
extern int getPayloadSize (RtpPacket* pst);
extern void setPayloadUsage (RtpPacket* pst, int size);
extern int getPayloadUsage (RtpPacket* pst);
extern char* getPadbyteLoc (RtpPacket* pst);
extern void setPadbyteSize (RtpPacket* pst, int size);
extern int getPadbyteSize (RtpPacket* pst);
extern void setPayloadType (RtpPacket* pst, RtpPayloadType type);
extern RtpPayloadType getPayloadType (RtpPacket* pst);
extern void setSequence (RtpPacket* pst, RtpSeqNumber nseq);
extern RtpSeqNumber getSequence (RtpPacket* pst);
extern void setRtpTime (RtpPacket* pst, RtpTime time);
extern RtpTime getRtpTime (RtpPacket* pst);
extern void setSSRC (RtpPacket* pst, RtpSrc src);
extern RtpSrc getSSRC (RtpPacket* pst);
extern int getCSRCcount (RtpPacket* pst);
extern void setCSRCcount (RtpPacket* pst, int i);
extern void setCSRC (RtpPacket* pst, RtpSrc src, unsigned int i);
extern RtpSrc getCSRC (RtpPacket* pst, unsigned int i);
extern int getPacketAlloc (RtpPacket* pst);
extern int getUnused (RtpPacket* pst);
extern void setTotalUsage (RtpPacket* pst, int size);
extern int getTotalUsage (RtpPacket* pst);
extern void setVersion (RtpPacket* pst, int i);
extern int getVersion (RtpPacket* pst);
extern void setPaddingFlag (RtpPacket* pst, int i);
extern int getPaddingFlag (RtpPacket* pst);
extern void setExtFlag (RtpPacket* pst, int i);
extern int getExtFlag (RtpPacket* pst);
extern void setMarkerFlag (RtpPacket* pst, int i);
extern int getMarkerFlag (RtpPacket* pst);

extern RtpPacket RTP_RX_DEC[RTP_RX_DEC_NUM];
extern RtpPacket RTP_TX_DEC[RTP_TX_DEC_NUM];
extern RtpPacket RTP_TX_DTMF[RTP_TX_DTMF_NUM];

#endif // RTPPACKET_H

