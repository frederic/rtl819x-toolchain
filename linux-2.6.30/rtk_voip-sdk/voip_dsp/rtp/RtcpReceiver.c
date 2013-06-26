
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

//static const char* const RtcpReceiver_cxx_Version =
//    "$Id: RtcpReceiver.c,v 1.3 2008-01-15 06:33:23 kennylin Exp $";

/* Kao
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
*/

/*#include <assert.h>
#include <time.h>
#include <sys/types.h>*/
/* Kao
#include "vtypes.h"
*/
/*#include <unistd.h>
#include <string.h>
#include <map>*/

/* Kao
// networking
#include <sys/types.h>
#include <sys/socket.h>
*/
/*#include <netinet/in.h> */

#ifdef DEBUG_LOG
#include "cpLog.h"
#endif
/* Kao
#include "vsock.hxx"
*/
#include <linux/string.h>

#include "rtpTypes.h"
#include "rtpTools.h"
#include "NtpTime.h"
#include "Rtp.h"
#include "Rtcp.h"
//#include "types.h"
#include "rtk_voip.h"
#include "voip_types.h"
/*#include <debug.h>*/
#include "RtcpReceiverXR.h"

#include "voip_control.h"
#include "voip_init.h"
#include "voip_proc.h"
#include "voip_dev.h"

#ifdef SUPPORT_RTCP

RtcpPacket RTCP_RX_DEC[MAX_DSP_RTK_SS_NUM][RTCP_RX_DEC_NUM];
/* static variable*/
static int rtcp_cur_get[MAX_DSP_RTK_SS_NUM];

RtcpReceiver RtcpRxInfo[MAX_DSP_RTK_SS_NUM];



/* ---- DD debug --------------------------------------------------- */

#if 1
static inline void rtcp_dd_write( const char *format, ... )	{}
#else
#include <stdarg.h>

#define RTCP_DD_BUFF_SIZE	1024
static char rtcp_dd_buff[ RTCP_DD_BUFF_SIZE ];

void rtcp_dd_write( const char *format, ... )
{
	va_list args;
	int n;
	
	va_start( args, format );
	n = vsnprintf( rtcp_dd_buff, RTCP_DD_BUFF_SIZE, format, args );
	ddinst_write( VOIP_DEV_RTCPTRACE, rtcp_dd_buff, n );
	va_end( args );
	
}
#endif

/* ----------------------------------------------------------------- */
/* --- rtpcReceiver Constructor ------------------------------------ */
/* ----------------------------------------------------------------- */

void RtcpRx_Init (void)
{
	uint32 sid;
	for(sid=0; sid<DSP_RTK_SS_NUM; sid++)
	{
		RtcpRx_InitByID(sid, 1);
    }
}

void RtcpRx_InitByID (uint32 sid, uint32 enableXR)
{
	extern int rtcp_cur_rx[];
    RtcpReceiver* pRxInfo = NULL;
    RtpTranInfo *info;
    int i;

	if(sid >= DSP_RTK_SS_NUM)
		return;

	pRxInfo = &RtcpRxInfo[sid];

	pRxInfo->packetReceived = 0;

	pRxInfo->accumOneWayDelay = 0;
	pRxInfo->accumOneWayDelayCount = 0;
	pRxInfo->avgOneWayDelay = 0;
	pRxInfo->accumRoundTripDelay = 0;
	pRxInfo->accumRoundTripDelayCount = 0;
    pRxInfo->avgRoundTripDelay = 0;
	pRxInfo->tranInfoCnt = 0;
	
	rtcp_dd_write( "RTD: %s:%d %lu \n"
						"\tpRxInfo->avgRoundTripDelay=%lu\n"
						"\tpRxInfo->accumRoundTripDelay=%lu\n"
						"\tpRxInfo->accumRoundTripDelayCount=%lu\n", 
						__FUNCTION__, __LINE__, sid, 
						pRxInfo->avgRoundTripDelay,
						pRxInfo->accumRoundTripDelay,
						pRxInfo->accumRoundTripDelayCount );

/*	info = pRxInfo->tranInfoList;*/
	for(i=0; i<MAX_TRANINFO_LIST; i++)
	{
		info = &pRxInfo->tranInfoList[i];
	    memset( info, 0, sizeof( *info ) );	// add to clean all info. 
		info->balloc = 0;
	    info->ssrc = 0;
/*		info += sizeof(RtpTranInfo);*/
/*	    	info++;*/
	}

	rtcp_cur_get[sid] = 0;

#if  1 // tim add 
		rtcp_cur_rx[sid] = 0;
		
		for( i=0; i<RTCP_RX_DEC_NUM; i++)
		{
			RtcpPkt_Init(&RTCP_RX_DEC[sid][i]);
			RTCP_RX_DEC[sid][i].packetState = PKT_FREE;
			RTCP_RX_DEC[sid][i].chid = 0;
			RTCP_RX_DEC[sid][i].sid = 0;
		}
#endif
	
	memset( &pRxInfo ->rxLogger, 0, sizeof( pRxInfo ->rxLogger ) );
}

#if 0
void RtcpRx_Close (void)
{
    // must remove each transmitter block and each SDES info
    map < RtpSrc, RtpTranInfo* > ::iterator s = tranInfoList.begin();
    while (s != tranInfoList.end())
    {
        removeTranInfo((s->second)->ssrc);
        s = tranInfoList.begin();
    }
    //cpLog(LOG_DEBUG_STACK, "RTCP: Receiver removed");
}
#endif

RtcpReceiver* RtcpRx_getInfo (uint32 sid)
{
    RtcpReceiver* pRxInfo = NULL;

	if(sid >= DSP_RTK_SS_NUM)
		return NULL;

	pRxInfo = &RtcpRxInfo[sid];
	return pRxInfo;
}

/* --- receive packet functions ------------------------------------ */


RtcpPacket* RtcpRx_getPacket (uint32 sid)
{
	int *cur_get = NULL;

	if(sid >= DSP_RTK_SS_NUM)
		return NULL;

	cur_get = &rtcp_cur_get[sid];
	
    /* create packet */
	RtcpPacket* p = &RTCP_RX_DEC[sid][*cur_get];

	if(p->packetState == PKT_FREE)
		return NULL;
	
	//printk("cur_get=%d, sid=%d\n", *cur_get, sid);
	
	(*cur_get)++;
	*cur_get &= (RTCP_RX_DEC_NUM - 1);
	

	/*	RtcpPkt_Init(p);*/
	p->packetAlloc = PACKETSIZE;
	//printk("packetAlloc=%d, unusedSize=%d, p=%p\n", p->packetAlloc, p->unusedSize, p);
	/*    RtcpPkt_setTotalUsage(p, len);*/

    // check packet
	/*    if (!RtcpRx_isValid(p))
	{
		p->packetState = PKT_FREE;
		p = NULL;
		return NULL;
	}*/
    return p;
}

int RtcpRx_freePacket(RtcpPacket* p)
{
	if(p == NULL)
	{
		//printk("p=NULL, Can't Free Packet\n");
		return -1;
	}

	if(p->packetState == PKT_FREE)
	{
		//printk("Packet is Free, Can't Free Packet again\n");
		return -1;
	}

	p->packetState = PKT_FREE;
	
	//printk("Free Packet\n");
	//printk("\n");

	return 0;
}

int RtcpRx_isValid (RtcpPacket* p)
{
    char* begin = (char*) (RtcpPkt_getPacketData(p));
    char* end = (char*) (begin + RtcpPkt_getTotalUsage(p));
    RtcpHeader* middle = (RtcpHeader*) (begin);

    // check if known payload type for first packet
    if (middle->type != rtcpTypeSR && middle->type != rtcpTypeRR)
        return 0;

    // check padbyte
    if (middle->padding)
        return 0;

    // check header lengths
    while (begin < end && (int)middle->version == RTP_VERSION)
    {
        begin += _mul32((ntohs(middle->length) + 1), sizeof(RtpSrc));
        middle = (RtcpHeader*) (begin);
    }

    if (begin != end)
        return 0;

    // exit with success
#ifdef DEBUG_LOG
    cpLog(LOG_DEBUG_STACK, "RTCP packet is valid");
#endif
    //    cout << "RTCP packet is valid" << endl;
    return 1;
}

int RtcpRx_receiveRTCP (uint32 sid)
{
	extern RtpSessionState sessionState[MAX_DSP_RTK_SS_NUM];
	extern RtpSessionError sessionError[MAX_DSP_RTK_SS_NUM];
	RtcpPacket* p;
	RtcpPacket* p1;

	if(sid >= DSP_RTK_SS_NUM)
	{
		printk("sid >= DSP_RTK_SS_NUM, return !\n");
		return -1;
	}	

    if ( !( sessionState[sid] == rtp_session_sendrecv
            || sessionState[sid] == rtp_session_recvonly ) )
    {
        p1 = RtcpRx_getPacket(sid);
        
        if (p1) 
        	RtcpRx_freePacket(p1);
        	
        sessionError[sid] = session_wrongState;

	//printk("RTCP stack can't receive. Wrong state"); 

        return -1;
    }

    // generate compound packet
    p = RtcpRx_getPacket(sid);

	if(p == NULL)
		return -1;
		
	//printk("RtcpRx-1\n");

    int ret = 0;

    // read compound packet
    if (RtcpRx_readRTCP(sid, p) == 1)
    {
       ret = 1;
    }
    //printk("R%d\n", sid);

    RtcpRx_freePacket(p);
    
    return ret;
}


int RtcpRx_readRTCP (uint32 sid, RtcpPacket* p)
{

	char* begin = (char*) (RtcpPkt_getPacketData(p));
	char* end = (char*) (begin + RtcpPkt_getTotalUsage(p));
	RtcpHeader* middle = NULL;
	int ret = 0;
	RtcpReceiver* pRxInfo;
    
	pRxInfo = RtcpRx_getInfo( sid );
	
	if( pRxInfo == NULL )
		ret = -1;
	else {
		pRxInfo ->rxLogger.packet_count ++;
	}

	//printk("--->RtcpRx_readRTCP, sid=%d\n", sid);
	//printk("begin=%p, end=%p\n", begin, end);

    while (begin < end)
    {
        middle = (RtcpHeader*) (begin);
        switch (middle->type)
        {
            case (rtcpTypeSR):
            case (rtcpTypeRR):
            	RtcpRx_readSR_H (sid, middle);
            	break;
            case (rtcpTypeSDES):
                RtcpRx_readSDES_H (sid, middle);
            	break;
            case (rtcpTypeBYE):
            	if ( RtcpRx_readBYE_H (sid, middle) == 0)
                {
                	ret = 1;
                }
            	break;
            case (rtcpTypeAPP):
            	RtcpRx_readAPP_H (sid, middle);
            	break;
#ifdef CONFIG_RTK_VOIP_RTCP_XR
			case (rtcpTypeXR):
				RtcpRx_readXR( sid, middle );
				break;
#endif
            default:
#ifdef DEBUG_LOG
            cpLog (LOG_ERR, "RTCP: Unknown RTCP type");
#endif
            	break;
        }
/*        begin += _mul32((ntohs(middle->length) + 1), sizeof(u_int32_t));*/
		begin += _mul32((middle->length + 1), sizeof(u_int32_t));
    }
    return ret;
}



RtcpHeader* RtcpRx_findRTCP (RtcpPacket* p, RtcpType type)
{
    char* begin = (char*) (RtcpPkt_getPacketData(p));
    char* end = (char*) (begin + RtcpPkt_getTotalUsage(p));
    RtcpHeader* middle = NULL;

    while (begin < end)
    {
        middle = (RtcpHeader*) (begin);
        if (type == (RtcpType) (middle->type))
            return middle;
        begin += _mul32((ntohs(middle->length) + 1), sizeof(u_int32_t));
    }

    // packet type not found
#ifdef DEBUG_LOG
    cpLog (LOG_ERR, "RTCP: Type found here: %d", (int)type);
#endif
    return NULL;
}



/* --- Read SR RTCP packet ----------------------------------------- */

int RtcpRx_readSR (uint32 sid, RtcpPacket* p)
{
    RtcpHeader* head = RtcpRx_findRTCP (p, rtcpTypeSR);
    if (head == NULL) head = RtcpRx_findRTCP (p, rtcpTypeRR);
    if (head == NULL) return -1;

    RtcpRx_readSR_H (sid, head);

    // read next RR packet if found
    // future: - ?

    return 0;
}

/* read SR by Header*/
void RtcpRx_readSR_H (uint32 sid, RtcpHeader* head)
{
	extern void rtcp_logger_unit_new_data( RtcpStatisticsUnit *pUnit,
								unsigned long data );

    char* middle = NULL;
    RtcpReceiver* pRxInfo = NULL;

	NtpTime nowNtp;//, nowNtp1;
	NtpTime thenNtp;
	Ntp32Time nowNtp32, thenNtp32;
	Ntp32Diff delta_ntp32;
	
	int i;
	unsigned long t;

	if(sid >= DSP_RTK_SS_NUM)
		return;

	pRxInfo = &RtcpRxInfo[sid];

    Ntp_getTime(&nowNtp);
    nowNtp32 = NTP32_getNTP32( &nowNtp );

    // read SR block
    if (head->type == rtcpTypeSR)
    {
		RtcpSender* senderBlock = (RtcpSender*)((char*)head + sizeof(RtcpHeader));
/*        RtpTranInfo* s = RtcpRx_findTranInfo(sid, ntohl(senderBlock->ssrc)); */

		RtpTranInfo* s = RtcpRx_findTranInfo(sid, senderBlock->ssrc);

		if(s == NULL) 
		{
			printk("RTCP: sid(%d) RtpTranInfo NULL \n",sid);
			return NULL;
			
		}
/*		s->lastSRTimestamp = (ntohl(senderBlock->ntpTimeSec) << 16 | ntohl(senderBlock->ntpTimeFrac) >> 16); */
		s->lastSRTimestamp = ((senderBlock->ntpTimeSec) << 16 | (senderBlock->ntpTimeFrac) >> 16);
/*        s->recvLastSRTimestamp = nowNtp; */

		Ntp_cpy(&s->recvLastSRTimestamp, &nowNtp);

        //printSR (senderBlock);  // - debug

        pRxInfo->packetReceived++;

/*		NtpTime thenNtp ( ntohl(senderBlock->ntpTimeSec), ntohl(senderBlock->ntpTimeFrac) ); */
		Ntp_TimeInit(&thenNtp, senderBlock->ntpTimeSec, senderBlock->ntpTimeFrac);
		thenNtp32 = NTP32_getNTP32( &thenNtp );

		t = pRxInfo->accumOneWayDelay;
/*      pRxInfo->accumOneWayDelay += (nowNtp - thenNtp); */
        //pRxInfo->accumOneWayDelay += NTP_sub(&nowNtp, &thenNtp);
		if( ( delta_ntp32 = NTP32_sub( nowNtp32, thenNtp32 ) ) > 0 )
			pRxInfo->accumOneWayDelay += NTP32_diff2ms( delta_ntp32 );
		else
			pRxInfo->accumOneWayDelay += 0;
		pRxInfo->accumOneWayDelayCount ++;
		
		if( t > pRxInfo->accumOneWayDelay ) {	// Overflow!! 
			pRxInfo->accumOneWayDelay = ( t >> 4 );
			pRxInfo->accumOneWayDelayCount >>= 4;
			
			if( pRxInfo->accumOneWayDelayCount == 0 )
				pRxInfo->accumOneWayDelayCount = 1;
		}

        pRxInfo->avgOneWayDelay = _idiv32(pRxInfo->accumOneWayDelay, pRxInfo->packetReceived);

        middle = (char*)head + sizeof(RtcpHeader) + sizeof(RtcpSender);
    }
    else
    {
        // move over blank RR header
        middle = (char*)head + sizeof(RtcpHeader);

        // move over the ssrc of packet sender
        RtpSrc* sender = (RtpSrc*) (middle);
        RtpSrc ssrc;
#ifdef CONFIG_RTK_VOIP_RTCP_XR
		RtpTranInfo* s;
#endif

/*        ssrc = ntohl(*sender); */
		ssrc = *sender;
        middle += sizeof(RtpSrc);

#ifdef CONFIG_RTK_VOIP_RTCP_XR
		s = RtcpRx_findTranInfo(sid, ssrc);
		
		Ntp_cpy(&s->recvLastRRTimestamp, &nowNtp);
#endif

        pRxInfo->packetReceived++;
    }
	
	if( pRxInfo->packetReceived == 1 )	// ignore first one! 
		return;
	
    // read RR blocks
    RtcpReport* block = (RtcpReport*) (middle);
    for (i = head->count; i > 0; i--)
    {
    	//u_int32_t delta_ms;
    	
		if( block->lastSRTimeStamp == 0 && block->lastSRDelay == 0 )
		{
			break;	// ignore SRTimestamp == 0 && lastSRDelay == 0
		}
		
		// logger 
		rtcp_logger_unit_new_data( &pRxInfo ->rxLogger.fraction_loss,
									block ->fracLost );
		
		rtcp_logger_unit_new_data( &pRxInfo ->rxLogger.inter_jitter,
									block ->jitter );
		
        //printRR (block);  // - debug

        // - ? what these are if the count is more than 1??
/*        NtpTime thenNtp (ntohl(block->lastSRTimeStamp) >> 16, ntohl(block->lastSRTimeStamp) << 16 ); */
		//Ntp_TimeInit(&thenNtp, (block->lastSRTimeStamp) >> 16, (block->lastSRTimeStamp) << 16);
		thenNtp32 = block->lastSRTimeStamp;
		rtcp_dd_write( "RTD: %s:%d %lu \n"
						"\tthenNtp32=%X\n"
						"\tblock->lastSRTimeStamp=%lX\n", 
						__FUNCTION__, __LINE__, sid, 
						thenNtp32,
						block->lastSRTimeStamp );
		
/*        NtpTime nowNtp1 (nowNtp.getSeconds() & 0x0000FFFF, nowNtp.getFractional() & 0xFFFF0000); */
		//Ntp_TimeInit(&nowNtp1, nowNtp.seconds & 0x0000FFFF, nowNtp.fractional & 0xFFFF0000);
		//nowNtp32 = NTP32_getNTP32( &nowNtp );
/*        pRxInfo->accumRoundTripDelay += ((nowNtp1 - thenNtp) - ntohl(block->lastSRDelay)); */
		t = pRxInfo->accumRoundTripDelay;
		
		// consider : 
		//   t0  -> delta_ms(9840) = nowNtp1(FFFF.6E0E0000) - thenNtp(FFF5.97010000)
		//   t10 -> delta_ms(4229441137) = nowNtp1(9.6E0E0000) - thenNtp(FFFF.97010000)
#if 0
		if( ( delta_ms = NTP_sub(&nowNtp1, &thenNtp) ) > block->lastSRDelay )	// check under flow 
			pRxInfo->accumRoundTripDelay += (delta_ms - (block->lastSRDelay));
		else
			pRxInfo->accumRoundTripDelay += 0;
#else
		if( ( delta_ntp32 = NTP32_sub( nowNtp32, thenNtp32 ) ) > block->lastSRDelay )	// check under flow 
			pRxInfo->accumRoundTripDelay += NTP32_diff2ms( delta_ntp32 - block->lastSRDelay );
		else
			pRxInfo->accumRoundTripDelay += 0;
#endif
		pRxInfo->accumRoundTripDelayCount ++;
		
		rtcp_dd_write( "RTD: %s:%d %lu \n"
						"\tt=%lu\n"
						"\tpRxInfo->accumRoundTripDelay=%lu\n"
						"\tdelta_ntp32(%ld) = nowNtp32(%X) - thenNtp32(%X)\n" 
						"\tblock->lastSRDelay=%lu\n"
						"\tpRxInfo->accumRoundTripDelay=%lu\n"
						"\tpRxInfo->accumRoundTripDelayCount=%lu\n", 
						__FUNCTION__, __LINE__, sid, 
						t, 
						pRxInfo->accumRoundTripDelay,
						delta_ntp32, nowNtp32, thenNtp32, 
						block->lastSRDelay,
						pRxInfo->accumRoundTripDelay,
						pRxInfo->accumRoundTripDelayCount );
		
		if( t > pRxInfo->accumRoundTripDelay ) {	// Overflow 
			pRxInfo->accumRoundTripDelay = ( t >> 4 );
			pRxInfo->accumRoundTripDelayCount >>= 4;
			
			if( pRxInfo->accumRoundTripDelayCount == 0 )
				pRxInfo->accumRoundTripDelayCount = 1;
				
			rtcp_dd_write( "RTD: %s:%d %lu \n"
						"\tpRxInfo->avgRoundTripDelay=%lu\n"
						"\tpRxInfo->accumRoundTripDelay=%lu\n"
						"\tpRxInfo->accumRoundTripDelayCount=%lu\n", 
						__FUNCTION__, __LINE__, sid, 
						pRxInfo->avgRoundTripDelay,
						pRxInfo->accumRoundTripDelay,
						pRxInfo->accumRoundTripDelayCount );
		}
		
#if 0
		printk( "now: %lu %lu\n", nowNtp1.seconds, nowNtp1.fractional );
		printk( "then: %lu %lu\n", thenNtp.seconds, thenNtp.fractional );
		printk( "diff: %lu\n", NTP_sub(&nowNtp1, &thenNtp) );
		printk( "LSRD: %lu\n", block->lastSRDelay );
		printk( "RTD: %lu\n", NTP_sub(&nowNtp1, &thenNtp) - (block->lastSRDelay) );
		printk( "acc: %lu (%lu)\n", pRxInfo->accumRoundTripDelay, pRxInfo->accumRoundTripDelayCount );
#endif
		
        pRxInfo->avgRoundTripDelay = _idiv32(pRxInfo->accumRoundTripDelay, pRxInfo->accumRoundTripDelayCount);
		
		rtcp_dd_write( "RTD: %s:%d %lu \n"
						"\tpRxInfo->avgRoundTripDelay=%lu\n"
						"\tpRxInfo->accumRoundTripDelay=%lu\n"
						"\tpRxInfo->accumRoundTripDelayCount=%lu\n", 
						__FUNCTION__, __LINE__, sid, 
						pRxInfo->avgRoundTripDelay,
						pRxInfo->accumRoundTripDelay,
						pRxInfo->accumRoundTripDelayCount );
		
        ++block;
    }

    // handle profile specific extensions
    // - ?
}

/* --- Read SDES packet -------------------------------------------- */

int RtcpRx_readSDES (uint32 sid, RtcpPacket* p)
{
    RtcpHeader* head = RtcpRx_findRTCP (p, rtcpTypeSDES);
    if (head == NULL) return -1;

    RtcpRx_readSDES_H (sid, head);

    // read next SDES packet if found
    // future: - ?

    return 0;
}


void RtcpRx_readSDES_H (uint32 sid, RtcpHeader* head)
{
	int i;
    char* begin = (char*) ((char*)head + sizeof(RtcpHeader));
    RtcpChunk* middle = (RtcpChunk*) (begin);

    RtcpSDESItem* item = NULL;
    RtcpSDESItem* nextitem = NULL;
    RtpSrc ssrc;

    for (i = head->count; i > 0; i--)
    {
/*		ssrc = ntohl(middle->ssrc); */
		ssrc = middle->ssrc;

        for (item = &(middle->startOfItem); item->type; item = nextitem)
        {
            RtcpRx_addSDESItem(sid, ssrc, item);
            nextitem = (RtcpSDESItem*)((char*)item + sizeof(RtcpSDESItem) - 1 + item->length);
        }

        middle = (RtcpChunk*) (item);
    }
}



void RtcpRx_addSDESItem (uint32 sid, RtpSrc ssrc, RtcpSDESItem* item)
{
    RtpTranInfo* s = RtcpRx_findTranInfo(sid, ssrc);

    if (s == NULL)
    {
    	PRINT_Y("%s, tran info is NULL, sid=%d\n", __FUNCTION__, sid); 
	return;
    }

    switch (item->type)
    {
        case rtcpSdesCname:
			strncpy ((s->SDESInfo).cname, &(item->startOfText), item->length + 1);
			break;
        case rtcpSdesName:
			strncpy ((s->SDESInfo).name, &(item->startOfText), item->length + 1);
			break;
        case rtcpSdesEmail:
			strncpy ((s->SDESInfo).email, &(item->startOfText), item->length + 1);
			break;
        case rtcpSdesPhone:
			strncpy ((s->SDESInfo).phone, &(item->startOfText), item->length + 1);
			break;
        case rtcpSdesLoc:
			strncpy ((s->SDESInfo).loc, &(item->startOfText), item->length + 1);
			break;
        case rtcpSdesTool:
			strncpy ((s->SDESInfo).tool, &(item->startOfText), item->length + 1);
			break;
        case rtcpSdesNote:
			strncpy ((s->SDESInfo).note, &(item->startOfText), item->length + 1);
			break;
        case rtcpSdesPriv:
			// future: not implemented
        default:
#ifdef DEBUG_LOG
			cpLog (LOG_ERR, "RtcpReceiver: SDES type unknown");
#endif
			break;
    }

    /*
    // - debug
    cerr <<"Update "<<src<<" with "<< (int) item->type <<" "<< (int) item->length;
    char output [255];
    memset (output, 0, 255);
    strncpy (output, &(item->startOfText), item->length+1);
    cerr << endl <<output<<endl;
    cerr <<"_SDES_";
    */
}



/* --- Read BYE packet --------------------------------------------- */

int RtcpRx_readBYE (uint32 sid, RtcpPacket* p)
{
    RtcpHeader* head = RtcpRx_findRTCP (p, rtcpTypeBYE);
    if (head == NULL) return -1;

    RtcpRx_readBYE_H (sid, head);

    // read next BYE packet if found
    // future: - ?

    return 0;
}


int RtcpRx_readBYE_H (uint32 sid, RtcpHeader* head)
{

	int i;
    //    char* end = reinterpret_cast<char*>
    //                ((char*)head + sizeof(RtpSrc) * (ntohs(head->length) + 1));
    RtpSrc* src = (RtpSrc*)((char*)head + sizeof(RtcpHeader));


    for (i = head->count; i > 0; i--)
    {
#ifdef DEBUG_LOG
        cpLog( LOG_DEBUG_STACK, "readRtcpBye for %d", ntohl(*src) );
#endif
        //       cerr << "readRtcpBye for " << ntohl(*src) << endl;
/*		RtcpRx_removeTranInfo (sid, ntohl(*src++)); */
		/*
		 * pkshih: Some soft-switch send 'RTCP Goodbye' before RTP. 
		 * In this case, we will deny all RTP packets. 
		 */
		//RtcpRx_removeTranInfo (sid, *src++, 0);
	}

    return 0;
}



/* --- Read APP packet --------------------------------------------- */

int RtcpRx_readAPP (uint32 sid, RtcpPacket* p)
{
    RtcpHeader* head = RtcpRx_findRTCP (p, rtcpTypeAPP);
    if (head == NULL) return -1;

    RtcpRx_readAPP_H (sid, head);

    // read next APP packet if found
    // future: - ?

    return 0;
}


void RtcpRx_readAPP_H (uint32 sid, RtcpHeader* head)
{
    // future: not implemented
    assert (0);
}


/* --- known transmitter list functions ---------------------------- */

RtpTranInfo* RtcpRx_addTranInfo (uint32 sid, RtpSrc ssrc, RtpReceiver* recv)
{
	int nResult = 0;

	RtpTranInfo s;

	if (recv) 
		assert (ssrc == recv->ssrc);
	s.recv = recv;
	s.ssrc = ssrc;
	s.expectedPrior = 0;
	s.receivedPrior = 0;

	return RtcpRx_addTranFinal (sid, &s);
}

RtpTranInfo* RtcpRx_addTranFinal (uint32 sid, RtpTranInfo* s)
{
	RtcpReceiver* pRxInfo;
	RtpTranInfo* info;
	int i;
	if(sid >= DSP_RTK_SS_NUM)
		return NULL;
	
	pRxInfo = &RtcpRxInfo[sid];

	for(i=0; i<MAX_TRANINFO_LIST; i++)
	{
		info = &pRxInfo->tranInfoList[i];
		if(info->ssrc == s->ssrc)	/* transmitter already in listing */
		{
			assert (info->recv == NULL);  // - ?
        	info->recv = s->recv;
        	return info;
		}
	}

	for(i=0; i<MAX_TRANINFO_LIST; i++)
	{
		info = &pRxInfo->tranInfoList[i];
		if(info->balloc == 0)
			break;
	}
	if(i == MAX_TRANINFO_LIST)
		return NULL;

	info->balloc = 1;
	info->recv =  s->recv;
	info->ssrc = s->ssrc;
	info->expectedPrior = s->expectedPrior;
	info->receivedPrior = s->receivedPrior;
	pRxInfo->tranInfoCnt++;
	
	PRINT_MSG("RTCP(%d): Transmitter add: %u, cnt: %u\n", sid, s->ssrc, pRxInfo->tranInfoCnt);
	
	return info;
}



int RtcpRx_removeTranInfo (uint32 sid, RtpSrc ssrc, int flag)
{
	RtcpReceiver* pRxInfo;
	RtpTranInfo* info, *preInfo;
	int i;
	extern void removeSource (uint32 sid, RtpSrc s, int flag);

	if(sid >= DSP_RTK_SS_NUM)
		return -1;
	pRxInfo = &RtcpRxInfo[sid];

/*	info = pRxInfo->tranInfoList; */
	for(i=0; i<MAX_TRANINFO_LIST; i++)
	{
		info = &pRxInfo->tranInfoList[i];
		if(info->ssrc == ssrc)
			break;
/*		info += sizeof(RtpTranInfo); */
	}

    if (i == MAX_TRANINFO_LIST)
    {
        /* ssrc not found */
        assert (0);
    }

    /* remove from RTP stack */
    if (!flag)
        removeSource(sid, info->ssrc, 1);
    info->balloc = 0;
	info->recv = NULL;

    /* printk("RTCP: done removing\n"); */
	for(i=i+1; i<MAX_TRANINFO_LIST; i++)
	{
		preInfo = info;
		info = &pRxInfo->tranInfoList[i];
/*		info += sizeof(RtpTranInfo);*/
/*		info++;*/
		if(info->balloc != 0)
		{
			preInfo->balloc = info->balloc;
			preInfo->recv = info->recv;
			info->balloc = 0;
			info->recv = NULL;
		}
	}
	pRxInfo->tranInfoCnt--;

    /* printk ("RTCP: Transmitter removed: %d", ssrc); */
    return 0;
}



RtpTranInfo* RtcpRx_findTranInfo (uint32 sid, RtpSrc ssrc)
{
	RtcpReceiver* pRxInfo = NULL;
    RtpTranInfo* info = NULL;
    RtpTranInfo* p = NULL;
    int i;

   	if(sid >= DSP_RTK_SS_NUM)
		return NULL;
	pRxInfo = &RtcpRxInfo[sid];
	
/*	info = pRxInfo->tranInfoList; */
	for(i=0; i<MAX_TRANINFO_LIST; i++)
	{
		info = &pRxInfo->tranInfoList[i];
		if(info->ssrc == ssrc)
			break;
	}

    if (i == MAX_TRANINFO_LIST)        // receiver not found, so add it
    {
        info = RtcpRx_addTranInfo(sid, ssrc, NULL);
    }
	else if(info->balloc == 0)
	{
		p = pRxInfo->tranInfoList;
		for(i=0; i<MAX_TRANINFO_LIST; i++)
		{
			if(p->balloc == 0)
				break;
			p++;
		}

		if(i == MAX_TRANINFO_LIST)
			printk("Find tran wrong!\n");

		if(p != info)
		{
			p->ssrc = info->ssrc;
			p->expectedPrior = info->expectedPrior;
			p->receivedPrior = info->receivedPrior;
			pRxInfo->tranInfoCnt++;
			info->ssrc = 0;
			info = p;
		}
		info->balloc = 1;
	}

    return info;
}


RtpTranInfo* RtcpRx_getTranInfoList (uint32 sid, int index)
{
	RtcpReceiver* pRxInfo = NULL;
    RtpTranInfo* info = NULL;
/*    int i; */

   	if(sid >= DSP_RTK_SS_NUM)
		return NULL;
		
	if(index >= MAX_TRANINFO_LIST)
		return NULL;
	pRxInfo = &RtcpRxInfo[sid];

/*	info = pRxInfo->tranInfoList; */

/*    for (i = 0; i < index; i++)
        ++info; */
	info = &pRxInfo->tranInfoList[index];

    return info;
}

int RtcpRx_getTranInfoCount(uint32 sid)
{
	RtcpReceiver* pRxInfo = &RtcpRxInfo[sid];
    return pRxInfo->tranInfoCnt;
}

int RtcpRx_getPort (uint32 sid)
{
	RtcpReceiver* pRxInfo = &RtcpRxInfo[sid];
    return pRxInfo->localPort;
}

int RtcpRx_getLogger( uint32 sid, TstVoipRtcpLogger *pLogger )
{
	RtcpReceiver *pInfo;

	if(sid > DSP_RTK_SS_NUM)
		return 0;
	pInfo = &RtcpRxInfo[sid];
	
	// caller set pLogger data as zeros 
	
	pLogger ->RX_packet_count = pInfo ->rxLogger.packet_count;
	if( pInfo ->rxLogger.fraction_loss.count ) {
		pLogger ->RX_loss_rate_max = pInfo ->rxLogger.fraction_loss.max;
		pLogger ->RX_loss_rate_min = pInfo ->rxLogger.fraction_loss.min;
		pLogger ->RX_loss_rate_avg = pInfo ->rxLogger.fraction_loss.sum / 
										pInfo ->rxLogger.fraction_loss.count;
		pLogger ->RX_loss_rate_cur = pInfo ->rxLogger.fraction_loss.last;
	}
	if( pInfo ->rxLogger.inter_jitter.count ) {
		pLogger ->RX_jitter_max = pInfo ->rxLogger.inter_jitter.max;
		pLogger ->RX_jitter_min = pInfo ->rxLogger.inter_jitter.min;
		pLogger ->RX_jitter_avg = pInfo ->rxLogger.inter_jitter.sum / 
									pInfo ->rxLogger.inter_jitter.count;
		pLogger ->RX_jitter_cur = pInfo ->rxLogger.inter_jitter.last;
	}
	if( pInfo ->rxLogger.round_trip_delay.count ) {
		pLogger ->RX_round_trip_max = pInfo ->rxLogger.round_trip_delay.max;
		pLogger ->RX_round_trip_min = pInfo ->rxLogger.round_trip_delay.min;
		pLogger ->RX_round_trip_avg = pInfo ->rxLogger.round_trip_delay.sum / 
										pInfo ->rxLogger.round_trip_delay.count;
		pLogger ->RX_round_trip_cur = pInfo ->rxLogger.round_trip_delay.last;
	}
	if( pInfo ->rxLogger.MOS_LQ.count ) {
		pLogger ->RX_MOS_LQ_max = pInfo ->rxLogger.MOS_LQ.max;
		pLogger ->RX_MOS_LQ_min = pInfo ->rxLogger.MOS_LQ.min;
		pLogger ->RX_MOS_LQ_avg = pInfo ->rxLogger.MOS_LQ.sum / pInfo ->rxLogger.MOS_LQ.count;
		pLogger ->RX_MOS_LQ_avg_x10 = ( ( pInfo ->rxLogger.MOS_LQ.sum & 0xF0000000UL ) ?
					( pInfo ->rxLogger.MOS_LQ.sum ) / ( pInfo ->rxLogger.MOS_LQ.count / 10 ) :	// large sum case 
					( pInfo ->rxLogger.MOS_LQ.sum * 10 ) / ( pInfo ->rxLogger.MOS_LQ.count ) );
		pLogger ->RX_MOS_LQ_cur = pInfo ->rxLogger.MOS_LQ.last;	
	}
	
	return 1;	
}

/* get the data for latency (ms) */
int RtcpRx_getAvgOneWayDelay(uint32 sid)
{
	RtcpReceiver* pRxInfo = &RtcpRxInfo[sid];
	return pRxInfo->avgOneWayDelay;
}
int RtcpRx_getAvgRoundTripDelay(uint32 sid)
{
	RtcpReceiver* pRxInfo = &RtcpRxInfo[sid];
	
	rtcp_dd_write( "RTD: %s:%d %lu \n"
						"\tpRxInfo->avgRoundTripDelay=%lu\n"
						"\tpRxInfo->accumRoundTripDelay=%lu\n"
						"\tpRxInfo->accumRoundTripDelayCount=%lu\n", 
						__FUNCTION__, __LINE__, sid, 
						pRxInfo->avgRoundTripDelay,
						pRxInfo->accumRoundTripDelay,
						pRxInfo->accumRoundTripDelayCount );
				
	return pRxInfo->avgRoundTripDelay;
}

static int voip_rtcp_logrx_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	extern int rtcp_logger_string( char *buff, 
								const RtcpStatisticsLogger *pLogger );

	int /*ch,*/ ss;
	int n = 0;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	if( IS_CH_PROC_DATA( data ) ) {
		//ch = CH_FROM_PROC_DATA( data );
		//n = sprintf( buf, "channel=%d\n", ch );
	} else {
		const RtcpStatisticsLogger *pLogger;
		const RtcpStatisticsUnit *pUnit;
		
		ss = SS_FROM_PROC_DATA( data );
		n = sprintf( buf, "session=%d\n", ss );
		
		pLogger = &RtcpRxInfo[ ss ].rxLogger;
		
		n += rtcp_logger_string( buf + n, pLogger );
	}
	
	*eof = 1;
	return n;
}

int __init voip_rtcp_logrx_proc_init( void )
{
	//create_voip_channel_proc_read_entry( "rtcp", voip_rtcp_read_proc );
	create_voip_session_proc_read_entry( "rtcp_logrx", voip_rtcp_logrx_read_proc );

	return 0;
}

void __exit voip_rtcp_logrx_proc_exit( void )
{
	//remove_voip_channel_proc_entry( "rtcp" );
	remove_voip_session_proc_entry( "rtcp_logrx" );
}

voip_initcall_proc( voip_rtcp_logrx_proc_init );
voip_exitcall( voip_rtcp_logrx_proc_exit );

#endif /* SUPPORT_RTCP */

