
#ifndef RTPRECEIVER_H
#define RTPRECEIVER_H

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

//static const char* const RtpReceiver_hxx_Version =
//    "$Id: RtpReceiver.h,v 1.3 2008-01-15 06:33:23 kennylin Exp $";


//#include <sys/types.h>
//#include <map>

//#include <Sptr.hxx>
//#include <Fifo.h>

#include "rtpTypes.h"
//#include <UdpStack.hxx>
/* Kao
#include "RtpEvent.hxx"
*/
#include "NtpTime.h"
//#include "RtpEventReceiver.hxx"
#include "RtcpReceiver.h"

//#include <NetworkAddress.h>

typedef enum {
	KPT_UNKNOWN,
	KPT_RTP_NORMAL,		// Normal voice, or V.152 Audio 
	KPT_RTP_VBD,		// V.152 VBD 
	KPT_SID,			// Silence 
	KPT_RFC2833,		// RFC 2833 
	KPT_REDUNDANCY,		// RTP redundancy 
	KPT_OTHERS,			// Exist in predefined table? (static payload type) 
} KnownPayloadType;

extern KnownPayloadType checkIfKnownPayloadType( uint32 sid, RtpPayloadType pt, uint32 bPrimary, int bIncSIDcount );

void RtpRx_Init(void);
void RtpRx_InitbyID(uint32 sid);
void RtpRx_renewSession(uint32 sid);
RtpPacket* Rtp_receive (void);
int isValid (RtpPacket* packet, KnownPayloadType *pKnownPayloadType);
int updateSource (RtpPacket* p);
void RtpRx_setFormat (uint32 sid, RtpPayloadType newtype, int frameRate);
void RtpRx_setMode(uint32 sid, RtpReceiverMode opmode);
int isRecvMode(uint32 sid);

#endif // RTPRECEIVER_HXX
