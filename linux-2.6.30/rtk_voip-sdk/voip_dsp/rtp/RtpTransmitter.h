#ifndef RTPTRANSMITTER_H
#define RTPTRANSMITTER_H

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

//static const char* const RtpTransmitter_hxx_Version =
//    "$Id: RtpTransmitter.h,v 1.3 2008-01-15 06:33:23 kennylin Exp $";

//#include <sys/types.h>
//#include <map>

#include "rtpTypes.h"
#include "NtpTime.h"



/// Data structure for RTP Transmitter

void RtpTx_Init (void);
void RtpTx_InitbyID (uint32 sid);

void RtpTx_renewSession (uint32 sid, int randomly, RtpSrc SSRC, RtpSeqNumber seqno, RtpTime timestamp,
							int max_red_audio, int max_red_rfc2833);
/// Creates a packet with transmitter's payload type and SRC number
//RtpPacket* createPacket (int npadSize = 0, int csrc_count = 0);

RtpTransmitter* RtpTx_getInfo (uint32 sid);

int RtpTx_transmitRaw (uint32 chid, uint32 sid, char* buffer, int data_len);

///
RtpSrc RtpTx_getSSRC (uint32 sid);

///
int RtpTx_getPacketSent (uint32 sid);

///
int RtpTx_getPayloadSent (uint32 sid);

///
RtpTime RtpTx_getPrevRtpTime (uint32 sid);
///
void RtpTx_setMarkerOnce(uint32 sid);
// jimmylin -- add an interface to set payload type
/// set payload format
void RtpTx_setFormat (uint32 sid, RtpPayloadType newtype, int frameRate);
// jmmylin -- add or subtract timestamp an unit
void RtpTx_addTimestamp (uint32 sid);
void RtpTx_addTimestampOfOneFrame (uint32 sid);
void RtpTx_subTimestamp (uint32 sid);
void RtpTx_subTimestampIfNecessary( uint32 sid, char *pBuf, int32 size );

int RtpTx_transmitEvent(uint32 chid, uint32 sid, int event, int delay_ms);

void RtpTx_setMode(uint32 sid, RtpTransmitMode opmode);
int32 isTranMode(uint32 sid);

#ifdef SUPPORT_RFC_2833		//tyhuang: dtmf open
int RtpTx_transmitEvent(uint32 chid, uint32 sid, int event, int delay_ms);
int RtpTx_transmitEvent_ISR( uint32 chid, uint32 sid, int event);
#endif

#endif // RTPTRANSMITTER_H
