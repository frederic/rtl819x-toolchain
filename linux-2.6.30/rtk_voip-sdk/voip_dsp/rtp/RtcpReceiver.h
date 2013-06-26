
#ifndef RTCPRECEIVER_H
#define RTCPRECEIVER_H

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

//static const char* const RtcpReceiver_hxx_Version =
//    "$Id: RtcpReceiver.h,v 1.3 2008-01-15 06:33:23 kennylin Exp $";


/*#include "Rtcp.h"*/

/*#include <sys/types.h>
#include <map> */

/* #include <UdpStack.hxx> */

#include "rtpTypes.h"

#include "RtcpPacket.h"
/* #include <NetworkAddress.h> */

//#define MAX_TRANINFO_LIST 4
#if 0
///Struct to transmit RTCP packets
typedef struct stRtcpReceiver
{
	/* list of known sources */
/*	map < RtpSrc, RtpTranInfo* > tranInfoList;*/
	RtpTranInfoStruct tranInfoList[MAX_TRANINFO_LIST];
	int tranInfoCnt;

	/* my UDP stack */
/*	UdpStack* myStack;*/
	int localPort;
	int remotePort;

	int packetReceived;

	int accumOneWayDelay;
	int avgOneWayDelay;

	int accumRoundTripDelay;
	int avgRoundTripDelay;

}RtcpReceiver;
#endif
/** initial RTCP Rx variable
	@param port associated port
**/
void RtcpRx_Init (void);

void RtcpRx_InitByID (uint32 sid, uint32 enableXR);

/*void RtcpRx_Close (void);*/

RtcpReceiver* RtcpRx_getInfo (uint32 sid);

/** Receives an incoming RTCP packet
	@return: NULL no data recprocess
**/
RtcpPacket* RtcpRx_getPacket (uint32 sid);

/** Checks if an RTCP packet is valid
	@return 0 not valid, 1 valid
**/
int RtcpRx_isValid (RtcpPacket* p);

int RtcpRx_receiveRTCP (uint32 sid);

/* reads compound RtcpPacket and calls apporiate read function */
int RtcpRx_readRTCP (uint32 sid, RtcpPacket* p);

/** searches inside packet for given type.
	@return NULL means type not found, else ptr to first found
**/
RtcpHeader* RtcpRx_findRTCP (RtcpPacket* p, RtcpType type);

/**
 *  These functions will search inside packet for apporiate type
 *  @return -1 if type not found.  Otherwise will call related
 *  function to use packet and return 0.
**/
int RtcpRx_readSR (uint32 sid, RtcpPacket* p);

int RtcpRx_readSDES (uint32 sid, RtcpPacket* p);

int RtcpRx_readBYE (uint32 sid, RtcpPacket* p);

int RtcpRx_readAPP (uint32 sid, RtcpPacket* p);

/**
 *  These functions will search inside packet for apporiate type
 * Thses functions will read packet and store packet information.
 * Same result as calling, ie  readSR(findRTCP(p, RtcpTypeSR))
**/
void RtcpRx_readSR_H (uint32 sid, RtcpHeader* head);

void RtcpRx_readSDES_H (uint32 sid, RtcpHeader* head);

int RtcpRx_readBYE_H (uint32 sid, RtcpHeader* head);

void RtcpRx_readAPP_H (uint32 sid, RtcpHeader* head);

void RtcpRx_addSDESItem (uint32 sid, RtpSrc ssrc, RtcpSDESItem* item);

/* Adds receiver to source listing */
RtpTranInfo* RtcpRx_addTranInfo (uint32 sid, RtpSrc ssrc, RtpReceiver* recv);
RtpTranInfo* RtcpRx_addTranFinal (uint32 sid, RtpTranInfo* s);
/** Remove receiver from  source listing
 *  @return 0 sucess, 1 not found
**/
int RtcpRx_removeTranInfo (uint32 sid, RtpSrc ssrc, int flag);
/** Finds pointer to source structure
 *  Creates source struture if not found
 *  @return pointer to source structure
**/
RtpTranInfo* RtcpRx_findTranInfo (uint32 sid, RtpSrc ssrc);

/* Access specified souurce infomration */
RtpTranInfo* RtcpRx_getTranInfoList (uint32 sid, int index);
/* Number of known sources */
int RtcpRx_getTranInfoCount(uint32 sid);

/* Port this receiver is receiving it signal */
int RtcpRx_getPort (uint32 sid);

/* get the data for latency (ms) */
int RtcpRx_getAvgOneWayDelay(uint32 sid);

int RtcpRx_getAvgRoundTripDelay(uint32 sid);

extern RtcpPacket RTCP_RX_DEC[MAX_DSP_RTK_SS_NUM][RTCP_RX_DEC_NUM];

#endif // RTCPRECEIVER_H
