
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

//static const char* const RtpPacket_cxx_Version =
//    "$Id: RtpPacket.c,v 1.3 2008-01-15 06:33:23 kennylin Exp $";

//#include <time.h>

//#include <sys/types.h>

//#include <unistd.h>
//#include <string.h>

// network socket
//#include <netinet/in.h>                // struct socketaddr_in

#ifdef DEBUG_LOG
#include "cpLog.h"
#endif

#include <asm/io.h>

#include "assert.h"
//#include <NetworkAddress.h>

//#include "rtpTypes.h"
#include "rtpTools.h"
#include "Rtp.h"
#include "RtpPacket.h"
#include "Rtcp.h"
/* Kao
#include "rtpCodec.h"
*/

//#include <debug.h>

///////////////////////////////////////////////////////////////////////
// global variable
RtpPacket RTP_RX_DEC[RTP_RX_DEC_NUM];
RtpPacket RTP_TX_DEC[RTP_TX_DEC_NUM];
RtpPacket RTP_TX_DTMF[RTP_TX_DTMF_NUM];


/* ----------------------------------------------------------------- */
/* --- RtpPacket Constructor --------------------------------------- */
/* ----------------------------------------------------------------- */

void RtpPacket_Init (RtpPacket* pst, int newpayloadSize, int npadSize, int csrc_count)
{
	pst->packetData = NULL;
	pst->header = NULL;

	// check given paramters
	assert (csrc_count >= 0);
	assert (csrc_count <= 15);
	assert (newpayloadSize >= 0);
	assert (npadSize >= 0);

	// create memory allocation
	pst->packetAlloc = sizeof(RtpHeader) - sizeof(RtpSrc) + csrc_count * sizeof(RtpSrc) + newpayloadSize + npadSize;
//	assert (pst->packetAlloc < RTP_MTU);
//    pst->packetData = new char[pst->packetAlloc];
	pst->packetData = pst->packetbuf;
//    assert (pst->packetData);
	memset (pst->packetData, 0, pst->packetAlloc + 1);

	// set private variables
	pst->header = (RtpHeader*) (char*)(pst->packetData);
	setPadbyteSize (pst, npadSize);
	setPayloadUsage (pst, 0);
	assert (pst->unusedSize == newpayloadSize);

	// set rtp header values
	pst->header->version = RTP_VERSION;
	pst->header->padding = (npadSize > 0) ? 1 : 0;
	pst->header->extension = 0;
	pst->header->count = csrc_count;
	pst->header->marker = 0;
	pst->header->type = (u_int32_t)rtpPayloadUndefined;
	pst->header->sequence = 0;
	pst->header->timestamp = 0;

	// set flags
	pst->sequenceSet = FALSE;
	pst->timestampSet = FALSE;
	
	// RFC2833 flags 
	pst->RFC2833 = FALSE;
	pst->EventPktMarker = FALSE;
	pst->EventPktBody = FALSE;
	pst->EventPktEdge = FALSE;
	pst->EventPktDuration = FALSE;
}


void RtpPacket_Exit (RtpPacket* pst)
{
	if (pst->packetData != NULL)
	{
		pst->packetData = NULL;
//		delete []packetData; packetData = NULL;
	}
	pst->header = NULL;
}


/* --- Header and packet data -------------------------------------- */

/// Pointer to packet data
/*
char* getPacketData (void)
*/
char* getPacketData (RtpPacket* pst)
{
	return pst->packetData;
}

/// Pointer to packet header
/*
RtpHeader* getHeader (void)
*/
RtpHeader* getHeader (RtpPacket* pst)
{
	return pst->header;
}


/* --- Size and Locations ------------------------------------------ */


/*     payload          */
/*
char* getPayloadLoc (void)
*/
char* getPayloadLoc (RtpPacket* pst)
{
	assert (pst->header);
	return (pst->packetData + sizeof(RtpHeader) - sizeof(RtpSrc) + (pst->header->count)*sizeof(RtpSrc));
}

/*
int getPayloadSize(RtpPacket* pst)
*/
int getPayloadSize (RtpPacket* pst)
{
	assert (pst->header);
	return (pst->packetAlloc - sizeof(RtpHeader) + sizeof(RtpSrc) - (pst->header->count)*sizeof(RtpSrc) - getPadbyteSize(pst));
}

/*
void setPayloadUsage (int size)
*/
void setPayloadUsage (RtpPacket* pst, int size)
{
#ifdef DEBUG_LOG
	if (!(size <= getPayloadSize(pst)))
		cerr << "ERR" << size << " " << getPayloadSize(pst);
#endif
	assert (size <= getPayloadSize(pst));
	if (size > getPayloadSize(pst))
	{
		DBG_ERROR("%s: %d > %d (%d)\n",
			__FUNCTION__,
			size,
			getPayloadSize(pst),
			pst->unusedSize
		);
	}

	pst->unusedSize = getPayloadSize(pst) - size;
}

/*
int getPayloadUsage (void)
*/
int getPayloadUsage (RtpPacket* pst)
{
	return getPayloadSize(pst) - pst->unusedSize;
}

/*     padbyte          */
/*
char* getPadbyteLoc (void)
*/
char* getPadbyteLoc (RtpPacket* pst)
{
	return getPayloadLoc(pst) + getPayloadSize(pst);
}

/*
void setPadbyteSize (int size)
*/
void setPadbyteSize (RtpPacket* pst, int size)
{
	// future: not implemented
	// ? write size to last octlet of packetData
}

/*
int getPadbyteSize (void)
*/
int getPadbyteSize (RtpPacket* pst)
{
	// future: not implemented
	// ? read last octlet of packetData
	return 0;
}

/* --- Packet Information ------------------------------------------ */

/*     payload type     */
/*
void setPayloadType (RtpPayloadType type)
*/
void setPayloadType (RtpPacket* pst, RtpPayloadType type)
{
	assert (type >= 0);
	assert (type <= 127);
	pst->header->type = type;
}

/*
RtpPayloadType getPayloadType (void)
*/
RtpPayloadType getPayloadType (RtpPacket* pst)
{
//	return static_cast < RtpPayloadType > (header->type);
	return (RtpPayloadType) (pst->header->type);
}

/*     sequence number  */
/*
void setSequence (RtpSeqNumber nseq)
*/
void setSequence (RtpPacket* pst, RtpSeqNumber nseq)
{
	assert (pst->header);
	pst->sequenceSet = TRUE;
	pst->header->sequence = htons(nseq);
}

/*
RtpSeqNumber getSequence (void)
*/
RtpSeqNumber getSequence (RtpPacket* pst)
{
	assert (pst->header);
	return ntohs(pst->header->sequence);
}

/*     timestamp        */
/*
void setRtpTime (RtpTime time)
*/
void setRtpTime (RtpPacket* pst, RtpTime time)
{
	assert (pst);
	pst->timestampSet = TRUE;
	assert (pst->header);
	pst->header->timestamp = htonl(time);
}

/*
RtpTime getRtpTime (void)
*/
RtpTime getRtpTime (RtpPacket* pst)
{
	assert (pst->header);
	return ntohl(pst->header->timestamp);
}

/*     ssrc             */
/*
void setSSRC (RtpSrc src)
*/
void setSSRC (RtpPacket* pst, RtpSrc src)
{
	assert (pst->header);
	pst->header->ssrc = htonl(src);
}

/*
RtpSrc getSSRC (void)
*/
RtpSrc getSSRC (RtpPacket* pst)
{
	assert (pst->header);
	return ntohl(pst->header->ssrc);
}

/*     csrc             */
/*
int getCSRCcount (void)
*/
int getCSRCcount (RtpPacket* pst)
{
	assert (pst->header);
	return pst->header->count;
}

// use with cuation
/*
void setCSRCcount (int i)
*/
void setCSRCcount (RtpPacket* pst, int i)
{
	assert (pst->header);
	pst->header->count = i;
}

/*
void setCSRC (RtpSrc src, unsigned int i)
*/
void setCSRC (RtpPacket* pst, RtpSrc src, unsigned int i)
{
	assert (pst->header);
	assert (i >= 1);
	assert (i <= pst->header->count);

	RtpSrc* srcPtr = &(pst->header->startOfCsrc);
	*(srcPtr + i - 1) = htonl(src);
}

/*
RtpSrc getCSRC (unsigned int i)
*/
RtpSrc getCSRC (RtpPacket* pst, unsigned int i)
{
	assert (pst->header);
	assert (i >= 1);
	assert (i <= pst->header->count);

	RtpSrc* srcPtr = &(pst->header->startOfCsrc);
	return ntohl(*(srcPtr + i - 1));
}

/// Entire size of RTP packet including header, unused, and padbyte
/*
int getPacketAlloc (void)
*/
int getPacketAlloc (RtpPacket* pst)
{
	return pst->packetAlloc;
}

/// Size of unused memory
/*
int getUnused (void)
*/
int getUnused (RtpPacket* pst)
{
	return pst->unusedSize;
}

/** Sets size of RTP packet including header and padbyte
	Extra memory will be set as unused memory
**/
/*
void setTotalUsage (int size)
*/
void setTotalUsage (RtpPacket* pst, int size)
{
	assert (size <= pst->packetAlloc);
	pst->unusedSize = pst->packetAlloc - size;
}

/// Size of RTP packet not including unused memory
/*
int getTotalUsage (void)
*/
int getTotalUsage (RtpPacket* pst)
{
	return pst->packetAlloc - pst->unusedSize;
}


// version
/*
void setVersion (int i)
*/
void setVersion (RtpPacket* pst, int i)
{
	pst->header->version = i;
}

/*
int getVersion (void)
*/
int getVersion (RtpPacket* pst)
{
	return pst->header->version;
}
///
/*
void setPaddingFlag (int i)
*/
void setPaddingFlag (RtpPacket* pst, int i)
{
	pst->header->padding = i;
}

/*
int getPaddingFlag (void)
*/
int getPaddingFlag (RtpPacket* pst)
{
	return pst->header->padding;
}

///
/*
void setExtFlag (int i)
*/
void setExtFlag (RtpPacket* pst, int i)
{
	pst->header->extension = i;
}

/*
int getExtFlag (void)
*/
int getExtFlag (RtpPacket* pst)
{
	return pst->header->extension;
}

///
/*
void setMarkerFlag (int i)
*/
void setMarkerFlag (RtpPacket* pst, int i)
{
	pst->header->marker = i;
}

/*
int getMarkerFlag (void)
*/
int getMarkerFlag (RtpPacket* pst)
{
	return pst->header->marker;
}

