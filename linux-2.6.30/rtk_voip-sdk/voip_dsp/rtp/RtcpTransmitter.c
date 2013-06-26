
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

//static const char* const RtcpTransmitter_cxx_Version =
//    "$Id: RtcpTransmitter.c,v 1.3 2008-01-15 06:33:23 kennylin Exp $";

/* Kao
#include <iostream>
*/
/*#include <stdlib.h>
#include <stdio.h>*/
#include "assert.h"
/*#include <time.h>
#include <sys/types.h>*/
/* Kao
#include "vtypes.h"
*/
/*#include <unistd.h>*/
#include <linux/string.h>


// networking
/*#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>*/

#ifdef DEBUG_LOG
#include "cpLog.h"
#endif
/* Kao
#include "vsock.hxx"
*/

#include "rtpTypes.h"
#include "rtpTools.h"
#include "NtpTime.h"
#include "Rtp.h"
#include "Rtcp.h"
#include "RtcpTransmitterXR.h"
/*#include <debug.h>*/

#include "rtk_voip.h"
#include "voip_control.h"
#include "voip_init.h"
#include "voip_proc.h"

#define RTCP_REPORT_IN_PROC	1

#ifdef SUPPORT_RTCP

RtcpPacket RTCP_TX_DEC[RTCP_TX_DEC_NUM];
RtcpTransmitter RtcpTxInfo[MAX_DSP_RTK_SS_NUM];

#ifdef RTCP_REPORT_IN_PROC
static RtcpReport RtcpReportPrev[MAX_DSP_RTK_SS_NUM];
#endif

static int rtcp_cur_send;
//unsigned short RtcpMeanTxInterval = 10000; //unit: msec
#define INITIAL_RTCP_INTERVAL	10000 //unit: msec

#ifdef CONFIG_RTK_VOIP_RTCP_XR
static uint32 RtcpTxEnableXR[MAX_DSP_RTK_SS_NUM];
#endif

extern RtpReceiver RtpRxInfo[MAX_DSP_RTK_SS_NUM];;
//extern CRtpConfig m_xConfig[MAX_DSP_RTK_SS_NUM];

/* ----------------------------------------------------------------- */
/* --- RtcpTransmitter Constructor --------------------------------- */
/* ----------------------------------------------------------------- */

void RtcpTx_Init (void)
{
/*    NetworkAddress netAddress; 

    if ( remoteHost )
    {
        netAddress.setPort(remoteMinPort);
//        OutputMessage("\n setHostName [11]") ;
		netAddress.setHostName(remoteHost);
    }

    if (receiver)
    {   
        myStack = receiver->getUdpStack();
        myStack->setDestination(&netAddress);
        remoteAddr = netAddress;
        //myStack->connectPorts();
        freeStack = false;
    }
    else
    {   
        myStack = new UdpStack(&netAddress, remoteMinPort, remoteMaxPort,
                               sendonly) ;
        remoteAddr = netAddress;
        //myStack->connectPorts();
        freeStack = true;
    }*/

	uint32 sid;

	rtcp_cur_send = 0;
	
	//RtcpMeanTxInterval = 10000;
	
	for(sid=0; sid<DSP_RTK_SS_NUM; sid++)
		RtcpTx_InitByID(sid, 1, INITIAL_RTCP_INTERVAL);
}

void RtcpTx_InitByID (uint32 sid, uint32 enableXR, uint32 txInterval)
{
	char dummy[2] = "";
	extern RtpReceiver* RtpRx_getInfo (uint32 sid);
	RtcpTransmitter *pInfo;
	if(sid > DSP_RTK_SS_NUM)
		return;
	pInfo = &RtcpTxInfo[sid];

/*	pInfo->tran = NULL;
	pInfo->recv = NULL;
	pInfo->rtcpRecv = NULL;
	pInfo->SDESInfo = NULL;
*/
	pInfo->tran = RtpTx_getInfo(sid);
	pInfo->recv = RtpRx_getInfo(sid);
	pInfo->rtcpRecv = RtcpRx_getInfo(sid);
	pInfo->SDESInfo = pInfo->SDESdataBuf;

    // prepare for rtcp timing intervals
/*    pInfo->nextInterval = Ntp_getTime(); */
	Ntp_getTime(&pInfo->nextInterval);
	pInfo->txInterval = txInterval;
    
	RtcpTx_updateInterval(sid);


    // SDES infromation for transmitter
	RtcpTx_setSdesCname(sid);
	RtcpTx_setSdesName(sid, dummy);
	RtcpTx_setSdesEmail(sid, dummy);
	RtcpTx_setSdesPhone(sid, dummy);
	RtcpTx_setSdesLoc(sid, dummy);
	RtcpTx_setSdesTool(sid, dummy);
	RtcpTx_setSdesNote(sid, dummy);

#ifdef RTCP_REPORT_IN_PROC
	memset( &RtcpReportPrev[ sid ], 0, sizeof( RtcpReportPrev[ 0 ] ) );
#endif

#ifdef CONFIG_RTK_VOIP_RTCP_XR
	RtcpTxEnableXR[ sid ] = enableXR;
#endif
	
	memset( &pInfo ->txLogger, 0, sizeof( pInfo ->txLogger ) );
	
	pInfo ->txMode = rtptran_droppacket;
}

#if 0
void RtcpTx_Close (void)
{
    if (freeStack)
    {
        delete myStack;
        myStack = 0;
    }

    if ((tran) && (SDESInfo))
    {
        delete SDESInfo;
        SDESInfo = NULL;
    }

    tran = NULL;
    recv = NULL;
    rtcpRecv = NULL;
}
#endif

RtcpTransmitter* RtcpTx_getInfo (uint32 sid)
{
    RtcpTransmitter* pInfo = NULL;

	if(sid >= DSP_RTK_SS_NUM)
		return NULL;

	pInfo = &RtcpTxInfo[sid];
	return pInfo;
}

/*
void RtcpTx_setRemoteAddr (uint32 sid, const NetworkAddress& theAddr)
{
	RtcpTransmitter *pInfo;
	if(sid > SESS_NUM)
		return;
	pInfo = &RtcpTxInfo[sid];

    pInfo->remoteAddr = theAddr;
}
*/

RtcpPacket* RtcpTx_getPacket(void)
{
    RtcpPacket* p = &RTCP_TX_DEC[rtcp_cur_send];

	if(p->packetState == PKT_ALLOC)
		return NULL;

	rtcp_cur_send++;
	rtcp_cur_send &= (RTCP_TX_DEC_NUM-1);

/*	RtcpPkt_Init(p);*/
    p->packetAlloc = PACKETSIZE;
	p->unusedSize = p->packetAlloc;
	return p;
}

int RtcpTx_freePacket(RtcpPacket* p)
{
	if(p == NULL)
		return -1;

	if(p->packetState == PKT_FREE)
		return -1;

/*	RtcpPkt_Init(p);*/
/*    p->packetData = NULL;

    p->packetAlloc = PACKETSIZE;
    assert (p->packetAlloc < RTP_MTU);
    p->packetData = p->packetbuf;
    assert (p->packetData);
    memset (pst->packetData, 0, pst->packetAlloc);
    p->unusedSize = p->packetAlloc;
*/
	p->packetState = PKT_FREE;

	return 0;
}

/* transmit function*/
int RtcpTx_transmitRTCP (uint32 sid)
{
	//extern RtpSessionState sessionState[MAX_SESS_NUM];
	extern RtpSessionError sessionError[MAX_DSP_RTK_SS_NUM];
	uint32 chid;
	RtcpPacket* p;
	RtcpTransmitter *pInfo;

	if(sid >= DSP_RTK_SS_NUM)
		return -1;
	
	if( ( pInfo = RtcpTx_getInfo( sid ) ) == NULL )
		return -1;
	
//printk("--- transmit RTCP 1 ---\n");
 
	//if ( !( m_xConfig[sid].m_uTRMode == rtp_session_sendrecv
	//        || m_xConfig[sid].m_uTRMode == rtp_session_sendonly ) )
	if( pInfo ->txMode != rtptran_normal )
	{
		sessionError[sid] = session_wrongState;
#ifdef DEBUG_LOG
		cpLog (LOG_ERR, "RTCP stack can't transmit. Wrong state");
#endif
		return -1;
	}
//printk("--- transmit RTCP 2 ---\n");
	chid = chanInfo_GetChannelbySession(sid);
	if(	chid == CHANNEL_NULL)
		return -1;
//printk("--- transmit RTCP 3 ---\n");
    // generate compound packet
	p = RtcpTx_getPacket();

	if(p == NULL)
		return -1;
//printk("--- transmit RTCP 4 ---\n");
	p->chid = chid;
	p->sid = sid;

	// load with report packet
	RtcpTx_addSR(sid, p, 0);

    // load with SDES information
    // currently only sender sends SDES, recv only receiver doesn't
    RtcpTx_addSDES(sid, p, 0);

	// load with RTCP XR packet 
#ifdef CONFIG_RTK_VOIP_RTCP_XR
	if( RtcpTxEnableXR[ sid ] )
    	RtcpTx_addXR(sid, p, 0);
#endif

    // transmit packet
    pInfo ->txLogger.packet_count ++;
	int ret = RtcpTx_transmit(p);

	RtcpTx_freePacket(p);
    return ret;
}

int RtcpTx_transmitRTCPBYE (uint32 sid)
{
	uint32 chid;
	RtcpPacket* p;
	RtcpTransmitter* pInfo;
	char reason[] = "Program Ended.";

	if(sid >= DSP_RTK_SS_NUM)
		return -1;
	//printk("BYE-1\n");

	pInfo = RtcpTx_getInfo( sid );
	
	if( pInfo == NULL || pInfo ->txInterval == 0 )
		return -1;

	chid = chanInfo_GetChannelbySession(sid);
	if(	chid == CHANNEL_NULL)
		return -1;
	//printk("BYE-2\n");

	/* generate compound packet */
	p = RtcpTx_getPacket();

	if(p == NULL)
		return -1;
	//printk("BYE-3\n");
	
	p->chid = chid;
	p->sid = sid;

	/* load with report packet */
	RtcpTx_addSR(sid, p, 0);
	//printk("BYE-4\n");

	/* load with SDES CNAME */
	RtcpTx_addSDES(sid, p, rtcpSdesCname);
	//printk("BYE-5\n");

	/* load with BYE packet */
	RtcpTx_addBYE(sid, p, reason, 0);
	//printk("BYE-6\n");

	/* transmit packet */
	pInfo ->txLogger.packet_count ++;
	int ret = RtcpTx_transmit(p);
	//printk("BYE-7\n");

	RtcpTx_freePacket(p);
	PRINT_MSG("RTCP-BYE\n");
	return ret;
}


int RtcpTx_checkIntervalRTCP (uint32 sid)
{
    return RtcpTx_checkInterval(sid);
}


/* --- send packet functions --------------------------------------- */

int RtcpTx_transmit (RtcpPacket* p)
{
	if(!p)	/* attempt to transmit a null rtcp packet*/
		return -1;

	DSP_rtcpWrite(p);

    /* exit with sucess */
    return 0;
}


void RtcpTx_updateInterval (uint32 sid)
{
    // RTCP_INTERVAL random offset between (.5 to 1.5)
    //int delayMs = RTCP_INTERVAL * (500 + rand() / (RAND_MAX / 1000)) / 1000;
    // Assume the fixed period. Just for temp  jimmylin 2002/03/13
#if 0
#if 0
    int delayMs = RTCP_INTERVAL;
#else
    int delayMs = RtcpMeanTxInterval;
#endif
#endif
	RtcpTransmitter *pInfo;
	if(sid > DSP_RTK_SS_NUM)
		return;
	pInfo = &RtcpTxInfo[sid];

/*	pInfo->nextInterval = pInfo->nextInterval + delayMs;*/
	//NTP_addms(&pInfo->nextInterval, delayMs, &pInfo->nextInterval);
	NTP_addms(&pInfo->nextInterval, pInfo ->txInterval, &pInfo->nextInterval);

}

static uint32 rtcp_cnt = 0;

int RtcpTx_checkInterval (uint32 sid)
{
	
	NtpTime ntpTime;
	RtcpTransmitter *pInfo;
	if(sid > DSP_RTK_SS_NUM)
		return 0;
	pInfo = &RtcpTxInfo[sid];
	Ntp_getTime(&ntpTime);
	rtcp_cnt++;
	//printk("%d, %d\n",ntpTime.seconds, ntpTime.fractional);

/*    if (Ntp_getTime() > pInfo->nextInterval)*/
	
#if 1
	//if (RtcpMeanTxInterval == 0)// Don't send RTCP
	if( pInfo ->txInterval == 0 )
		return 0;
	
		
	if(NTP_isLarge(&ntpTime, &pInfo->nextInterval))
#else
	if (rtcp_cnt == 100)
#endif
    	{
    		rtcp_cnt = 0;
       	 	// prepare for next interval
        	RtcpTx_updateInterval(sid);
        	return 1;
    	}

    // time not up yet
    return 0;
}


/* --- SR/RR RTCP report packets------------------------------------ */

int RtcpTx_addSR (uint32 sid, RtcpPacket* p, int npadSize)
{
	extern void rtcp_logger_unit_new_data( RtcpStatisticsUnit *pUnit,
								unsigned long data );
	
    // header
    RtcpHeader* header = (RtcpHeader*) (RtcpPkt_freeData(p));
    int usage = RtcpPkt_allocData(p, sizeof(RtcpHeader));
	RtcpTransmitter *pInfo;
	NtpTime nowNtp;
	Ntp32Time nowNtp32, thenNtp32;
	Ntp32Diff NtpDiff32;
	int i;

	if(sid > DSP_RTK_SS_NUM)
		return -1;
	pInfo = &RtcpTxInfo[sid];

    header->version = RTP_VERSION;
    header->padding = (npadSize > 0) ? 1 : 0;
    header->count = 0;
    header->type = (pInfo->tran) ? rtcpTypeSR : rtcpTypeRR;

/*    NtpTime nowNtp = Ntp_getTime(); */
	Ntp_getTime(&nowNtp);
    // sender information
    if (pInfo->tran)
    {
        //cpLog (LOG_DEBUG_STACK, "RTCP: Making Sender Info");
        RtcpSender* senderInfo = (RtcpSender*) (RtcpPkt_freeData(p));
        usage += RtcpPkt_allocData (p, sizeof(RtcpSender));

        int diffNtp = 0;
/*		if (nowNtp > pInfo->tran->seedNtpTime)
			diffNtp = nowNtp - pInfo->tran->seedNtpTime;
        else
            if (pInfo->tran->seedNtpTime > nowNtp)
                diffNtp = pInfo->tran->seedNtpTime - nowNtp;*/

		if (NTP_isLarge(&nowNtp, &pInfo->tran->seedNtpTime))
            diffNtp = NTP_sub(&nowNtp, &pInfo->tran->seedNtpTime);
        else
            if (NTP_isLarge(&pInfo->tran->seedNtpTime, &nowNtp))
                diffNtp = NTP_sub(&pInfo->tran->seedNtpTime, &nowNtp);

        //RtpTime diffRtp = (diffNtp * tran->network_pktSampleSize) / 1000; jimmylin 2002/03/13
        RtpTime diffRtp = _idiv32(_imul32(diffNtp, pInfo->tran->bitRate/*apiFormat_bitRate*/), 1000);

#if 0
        senderInfo->ssrc = htonl(pInfo->tran->ssrc);
        senderInfo->ntpTimeSec = htonl(nowNtp.getSeconds());
        senderInfo->ntpTimeFrac = htonl(nowNtp.getFractional());
        senderInfo->rtpTime = htonl(pInfo->tran->seedRtpTime + diffRtp);
        senderInfo->packetCount = htonl(pInfo->tran->packetSent);
        senderInfo->octetCount = htonl(pInfo->tran->payloadSent);
#else
        senderInfo->ssrc = pInfo->tran->ssrc;
        senderInfo->ntpTimeSec = nowNtp.seconds;
        senderInfo->ntpTimeFrac = nowNtp.fractional;
        senderInfo->rtpTime = pInfo->tran->seedRtpTime + diffRtp;
        senderInfo->packetCount = pInfo->tran->packetSent;
        senderInfo->octetCount = pInfo->tran->payloadSent;
#endif
        //TRACE(DEBUG_UI, "RTCP: sent: %d  ", tran->packetSent);
    }
    else
    {
        RtcpChunk* chunk = (RtcpChunk*) (RtcpPkt_freeData(p));
        usage += RtcpPkt_allocData(p, (sizeof(RtpSrc)));
        chunk->ssrc = 0 ;  /* if recv only, give src 0 for receiver for now */
    }

    // report blocks
    /*if ((pInfo->rtcpRecv) && (pInfo->rtcpRecv->getTranInfoCount() > 0)) */
    if (RtcpRx_getTranInfoCount(sid) > 0)
    {
        //cpLog (LOG_DEBUG_STACK, "RTCP: Making Report Block");
        RtpTranInfo* tranInfo = NULL;
        RtpReceiver* recvInfoSpec = NULL;
        RtcpReport* reportBlock = NULL;

        for (i = 0; i < RtcpRx_getTranInfoCount(sid); i++)
        {
            tranInfo = RtcpRx_getTranInfoList(sid, i);
            recvInfoSpec = tranInfo->recv;

            // only receieved RTCP packets from transmitter
            if (recvInfoSpec == NULL)
                continue;

            // don't report on probation transmitters
            if (recvInfoSpec->probation < 0)
                continue;

            //cpLog (LOG_DEBUG_STACK, "RTCP:  Report block for src %d",
            //       recvInfoSpec->ssrc);
            reportBlock = (RtcpReport*) (RtcpPkt_freeData(p));
            usage += RtcpPkt_allocData (p, sizeof(RtcpReport));

#if 0
            reportBlock->ssrc = htonl(recvInfoSpec->ssrc);
#else
			reportBlock->ssrc = recvInfoSpec->ssrc;
#endif
            reportBlock->fracLost = RtcpTx_calcLostFrac(tranInfo);
            u_int32_t lost = (RtcpTx_calcLostCount(tranInfo)) & 0xffffff;
            reportBlock->cumLost[2] = lost & 0xff;
            reportBlock->cumLost[1] = (lost & 0xff00) >> 8;
            reportBlock->cumLost[0] = (lost & 0xff0000) >> 16;
#if 0
            reportBlock->recvCycles = htons(recvInfoSpec->recvCycles);
            reportBlock->lastSeqRecv = htons(recvInfoSpec->prevSeqRecv);
#else
            reportBlock->recvCycles = recvInfoSpec->recvCycles >> RTP_SEQ_MOD_SHIFT;
            reportBlock->lastSeqRecv = recvInfoSpec->prevSeqRecv;
#endif

            // fracational
            // reportBlock->jitter = htonl((u_int32_t)recvInfoSpec->jitter);

            // interger
            //            if (recvInfoSpec->jitter > 0)
#if 0
            reportBlock->jitter = htonl(recvInfoSpec->jitter >> 4);
            reportBlock->lastSRTimeStamp = htonl(tranInfo->lastSRTimestamp);
#else
            //reportBlock->jitter = recvInfoSpec->jitter >> 4;
            reportBlock->jitter = recvInfoSpec->interjitter.Ji_Q4 >> 4;
            reportBlock->lastSRTimeStamp = tranInfo->lastSRTimestamp;
#endif

            // reportBlock->lastSRDelay in the unit of 1/65536 of sec ??
            // currently it is in ms
            if (tranInfo->lastSRTimestamp == 0)
                reportBlock->lastSRDelay = 0;
            else
            {
#if 1
				nowNtp32 = NTP32_getNTP32( &nowNtp );
				thenNtp32 = NTP32_getNTP32( &tranInfo->recvLastSRTimestamp );
				
				NtpDiff32 = NTP32_sub( nowNtp32, thenNtp32 );
				
				if( NtpDiff32 < 0 )
					reportBlock->lastSRDelay = 0;
				else
					reportBlock->lastSRDelay = NtpDiff32;
#else
				// wrong unit: It must be 1/65536 seconds instead of ms 
				
/*                NtpTime thenNtp = tranInfo->recvLastSRTimestamp; */
				Ntp_cpy(&thenNtp, &tranInfo->recvLastSRTimestamp);
                //                NtpTime thenNtp ((tranInfo->lastSRTimestamp >> 16) |
                //                                 (nowNtp.getSeconds() & 0xffff0000),
                //                                 tranInfo->lastSRTimestamp << 16);
                reportBlock->lastSRDelay = 0;
/*                if (nowNtp > thenNtp) */
				if(NTP_isLarge(&nowNtp, &thenNtp))
				{
#if 0
                    reportBlock->lastSRDelay = htonl(nowNtp - thenNtp);
#else
					reportBlock->lastSRDelay = NTP_sub(&nowNtp, &thenNtp);
#endif
				} else
                    reportBlock->lastSRDelay = 0;
#endif
            }
            
            // tx logger 
            rtcp_logger_unit_new_data( &pInfo ->txLogger.fraction_loss,
            							reportBlock->fracLost );
            
            rtcp_logger_unit_new_data( &pInfo ->txLogger.inter_jitter,
            							reportBlock->jitter );
            
#ifdef RTCP_REPORT_IN_PROC
			RtcpReportPrev[ sid ] = *reportBlock;
#endif
            // next known transmitter
            header->count++;
            //TRACE(DEBUG_UI, "Recv: %d  CumLos: %d  Jitter: %d  ", recvInfoSpec->packetReceived, calcLostCount(tranInfo), recvInfoSpec->jitter >> 4);
        }
    }
    //TRACE(DEBUG_UI, "\n");

    // profile-specific extensions
    // future: not implemented


    // padding
    if (npadSize > 0)
    {
        // future: not implemented
        assert (0);
    }

    // overall packet must ends on 32-bit count
    assert (usage % 4 == 0);

#if 0
    header->length = htons((usage / 4) - 1);
#else
	header->length = (usage / 4) - 1;
#endif
    //cpLog (LOG_DEBUG_STACK, "RTCP:  SR/RR packet used %d bytes/ %d words",
    //       usage, usage/4);
    return usage;
}

u_int32_t RtcpTx_calcLostFrac (RtpTranInfo* s)
{
    /* from A.3 of RFC 1889 - RTP/RTCP Standards */

    RtpReceiver* r = s->recv;

    int expected = ((r->recvCycles + r->prevSeqRecv) - r->seedSeq + 1);
    int expected_interval, received_interval, lost_interval;

    expected_interval = expected - s->expectedPrior;
    s->expectedPrior = expected;
    received_interval = r->packetReceived - s->receivedPrior;
    s->receivedPrior = r->packetReceived;
    lost_interval = expected_interval - received_interval;

    u_int32_t fraction;
    if (expected_interval == 0 || lost_interval <= 0) fraction = 0;
    else fraction = _idiv32((lost_interval << 8), expected_interval);

    return fraction;
}


u_int32_t RtcpTx_calcLostCount (RtpTranInfo* s)
{
    /* from A.3 of RFC 1889 - RTP/RTCP Standards */

    RtpReceiver* r = s->recv;

    u_int32_t expected = ((r->recvCycles + r->prevSeqRecv) - r->seedSeq + 1);
    return expected - r->packetReceived;
}




/* --- SDES RTCP packet -------------------------------------------- */

int RtcpTx_addSDES_Item (uint32 sid, RtcpPacket* p, RtcpSDESType item, int npadSize)
{
	RtcpTransmitter *pInfo;
	if(sid > DSP_RTK_SS_NUM)
		return -1;
	pInfo = &RtcpTxInfo[sid];

    if (!pInfo->tran) return -1;

    RtcpSDESType list[2];
    list[0] = item;
    list[1] = rtcpSdesEnd;
    return RtcpTx_addSDES_List (sid, p, list, npadSize);
}


int RtcpTx_addSDES (uint32 sid, RtcpPacket* p, int npadSize)
{
	RtcpTransmitter *pInfo;
	if(sid > DSP_RTK_SS_NUM)
		return -1;
	pInfo = &RtcpTxInfo[sid];

    if (!pInfo->tran) return -1;

    RtcpSDESType list[8];
    int i = 0;

    if (strlen(RtcpTx_getSdesCname(sid)) > 0) list[i++] = rtcpSdesCname;
    if (strlen(RtcpTx_getSdesName(sid)) > 0) list[i++] = rtcpSdesName;
    if (strlen(RtcpTx_getSdesEmail(sid)) > 0) list[i++] = rtcpSdesEmail;
    if (strlen(RtcpTx_getSdesPhone(sid)) > 0) list[i++] = rtcpSdesPhone;
    if (strlen(RtcpTx_getSdesLoc(sid)) > 0) list[i++] = rtcpSdesLoc ;
    if (strlen(RtcpTx_getSdesTool(sid)) > 0) list[i++] = rtcpSdesTool ;
    if (strlen(RtcpTx_getSdesNote(sid)) > 0) list[i++] = rtcpSdesNote ;

    list[i] = rtcpSdesEnd;
    return RtcpTx_addSDES_List (sid, p, list, npadSize);
}


int RtcpTx_addSDES_List (uint32 sid, RtcpPacket* p, RtcpSDESType* SDESlist, int npadSize)
{
	int i;
	RtcpTransmitter *pInfo;
	RtpTransmitter *tran;
	if(sid > DSP_RTK_SS_NUM)
		return -1;
	pInfo = &RtcpTxInfo[sid];

	tran = pInfo->tran;
    if (!tran) return -1;

    // header
    //cpLog (LOG_DEBUG_STACK, "RTCP: Making SDES packet");
    RtcpHeader* header = (RtcpHeader*) (RtcpPkt_freeData(p));
    int usage = RtcpPkt_allocData (p, sizeof(RtcpHeader));

    header->version = RTP_VERSION;
    header->padding = (npadSize > 0) ? 1 : 0;
    header->count = 1;
    header->type = rtcpTypeSDES;

    // only sends the sender's SDE

    // SDES chunk
    RtcpChunk* chunk = (RtcpChunk*) (RtcpPkt_freeData(p));
    usage += RtcpPkt_allocData (p, sizeof(RtpSrc));
    /*
    cout << "sizeof(RtcpChunk) =" << sizeof(RtcpChunk) << endl; // ?? should be 7
    */
    //chunk->ssrc = htonl(tran->ssrc);
    chunk->ssrc = tran->ssrc;

    // SDES items
    RtcpSDESItem* item = NULL;
    for (i = 0; SDESlist[i] != rtcpSdesEnd; i++)
    {
        //cpLog (LOG_DEBUG_STACK, "RTCP:  Adding SDES %d", SDESlist[i]);
        item = (RtcpSDESItem*) (RtcpPkt_freeData(p));
        usage += RtcpPkt_allocData (p, sizeof(RtcpSDESItem) - 1);

        int len = 0;
        int buf_len = RtcpPkt_getUnused(p);

        switch (SDESlist[i])
        {
            case rtcpSdesCname:
            	strncpy(&(item->startOfText), RtcpTx_getSdesCname(sid), buf_len);
            	len = strlen(RtcpTx_getSdesCname(sid));
            	//cpLog (LOG_DEBUG_STACK, "RTCP:  SDES Item Length: %d", len);
            	//cpLog (LOG_DEBUG_STACK, "RTCP:  SDES Item Value: %s", getSdesCname());
            	break;
            case rtcpSdesName:
            	strncpy(&(item->startOfText), RtcpTx_getSdesName(sid), buf_len);
            	len = strlen(RtcpTx_getSdesName(sid));
            	break;
            case rtcpSdesEmail:
            	strncpy(&(item->startOfText), RtcpTx_getSdesEmail(sid), buf_len);
            	len = strlen(RtcpTx_getSdesEmail(sid));
            	break;
            case rtcpSdesPhone:
            	strncpy(&(item->startOfText), RtcpTx_getSdesPhone(sid), buf_len);
            	len = strlen(RtcpTx_getSdesPhone(sid));
            	break;
            case rtcpSdesLoc:
            	strncpy(&(item->startOfText), RtcpTx_getSdesLoc(sid), buf_len);
            	len = strlen(RtcpTx_getSdesLoc(sid));
            	break;
            case rtcpSdesTool:
            	strncpy(&(item->startOfText), RtcpTx_getSdesTool(sid), buf_len);
            	len = strlen(RtcpTx_getSdesTool(sid));
            	break;
            case rtcpSdesNote:
            	strncpy(&(item->startOfText), RtcpTx_getSdesNote(sid), buf_len);
            	len = strlen(RtcpTx_getSdesNote(sid));
            	break;
            case rtcpSdesPriv:
            	// future: not implemented
            	assert (0);
            	break;
            default:
#ifdef DEBUG_LOG
            	cpLog (LOG_ERR, "RtcpTransmitter:  SDES type unknown");
#endif
            	assert (0);
            	break;
        }
        item->type = SDESlist[i];

        // strlen removes the null that was copied
        item->length = len;
        usage += RtcpPkt_allocData (p, item->length);
    }

    // ending SDES item
    item = (RtcpSDESItem*) (RtcpPkt_freeData(p));
    usage += RtcpPkt_allocData (p, sizeof(RtcpSDESItem) - 1);
    item->type = rtcpSdesEnd;
    item->length = 0;

    // padding
    if (npadSize > 0)
    {
        // future: not implemented
        assert (0);
    }

    // end packet on 32-bit count
    if (usage % 4 != 0)
    {
        //cpLog (LOG_DEBUG_STACK, "RTCP:  SDES padded by: %d", 4-usage%4);
        usage += RtcpPkt_allocData (p, 4 - usage % 4);
    }

/*	header->length = htons((usage / 4) - 1); */
	header->length = ((usage / 4) - 1);
    //cpLog (LOG_DEBUG_STACK, "RTCP:  SDES packet used %d bytes/ %d words", usage, usage/4);
    return usage;
}



/* --- BYE RTCP packet --------------------------------------------- */

int RtcpTx_addBYE (uint32 sid, RtcpPacket* p, char* reason, int npadSize)
{
    RtpSrc list[1];
    list[0] = RtpTx_getSSRC(sid);
    return RtcpTx_addBYE_List (p, list, 1, reason, npadSize);
}


int RtcpTx_addBYE_List (RtcpPacket* p, RtpSrc* list, int count, char* reason, int npadSize)
{
	int i;
    assert (count > 0);

    // header
    //cpLog (LOG_DEBUG_STACK, "RTCP: Making BYE packet");
    RtcpHeader* header = (RtcpHeader*) (RtcpPkt_freeData(p));

    int usage = RtcpPkt_allocData (p, sizeof(RtcpHeader));

    header->version = RTP_VERSION;
    header->padding = (npadSize > 0) ? 1 : 0;
    header->count = count;
    header->type = rtcpTypeBYE;

    // transmitter leaving
    RtpSrc* s = NULL;
    for (i = 0; i < count; i++)
    {
        s = (RtpSrc*) (RtcpPkt_freeData(p));
        usage += RtcpPkt_allocData (p, sizeof(RtpSrc));
        //*s = htonl(list[i]);
        *s = list[i];
        //cpLog (LOG_DEBUG_STACK, "RTCP:  SRC: %d", list[i]);
    }

    // reason - optional

    if (reason)
    {
        RtcpBye* byeReason = (RtcpBye*) (RtcpPkt_freeData(p));
        usage += RtcpPkt_allocData (p, sizeof(RtcpBye) - 1);

        byeReason->length = strlen(reason);
        strncpy (&(byeReason->startOfText), reason, byeReason->length);
        usage += RtcpPkt_allocData (p, byeReason->length);
        //cpLog (LOG_DEBUG_STACK, "RTCP:  Reason: %s", reason);
    }

    // padding

    if (npadSize > 0)
    {
        // future: not implemented
        assert (0);
    }

    // end packet on 32-bit count
    if (usage % 4 != 0)
    {
        //cpLog (LOG_DEBUG_STACK, "RTCP:  BYE padded by: %d", 4-usage%4);
        usage += RtcpPkt_allocData (p, 4 - usage % 4);
    }

/*	header->length = htons((usage / 4) - 1); */
	header->length = ((usage / 4) - 1);
    //cpLog (LOG_DEBUG_STACK, "RTCP:  BYE packet used %d bytes/ %d words",
    //       usage, usage/4);
    return usage;
}


/* --- APP RTCP packet --------------------------------------------- */

int RtcpTx_addAPP (RtcpPacket* packet, int npadSize)
{
    // future: not implemented
    assert (0);
    return -1;
}



/* --- SDES Information -------------------------------------------- */

// debug, added by xavier to avoid compiling error
//
#ifdef __ECOS
extern "C" int gethostname(char*, int);
#endif

void RtcpTx_setSdesCname (uint32 sid)
{
    char user[64] = "default_user";
    char hn[64] = "uknown_host";
    char cnameres [64 + 20 + 64];
	RtcpTransmitter *pInfo;
	uint32 pid;
	NtpTime time;

	if(sid > DSP_RTK_SS_NUM)
		return;
	pInfo = &RtcpTxInfo[sid];

	Ntp_getTime(&time);
/*	pid = time.seconds + time.fractional;*/
	pid = sid;

/*    gethostname (hn, sizeof(hn));
	snprintf (cnameres, (64+20+64), "%s.%d@%s.Realtek", user, getpid(), hn);*/
	snprintf (cnameres, (64+20+64), "%s.%d@%s.Realtek", user, pid, hn);

    assert (strlen(cnameres) < 255);
    strncpy (pInfo->SDESInfo->cname, cnameres, 256);
}


void RtcpTx_setSdesName (uint32 sid, char* text)
{
	RtcpTransmitter *pInfo;

	if(sid > DSP_RTK_SS_NUM)
		return;
	pInfo = &RtcpTxInfo[sid];

    assert (strlen(text) < 255);
    strncpy (pInfo->SDESInfo->name, text, 256);
}

void RtcpTx_setSdesEmail (uint32 sid, char* text)
{
	RtcpTransmitter *pInfo;

	if(sid > DSP_RTK_SS_NUM)
		return;
	pInfo = &RtcpTxInfo[sid];

    assert (strlen(text) < 255);
    strncpy (pInfo->SDESInfo->email, text, 256);
}

void RtcpTx_setSdesPhone (uint32 sid, char* text)
{
	RtcpTransmitter *pInfo;

	if(sid > DSP_RTK_SS_NUM)
		return;
	pInfo = &RtcpTxInfo[sid];

    assert (strlen(text) < 256);
    strncpy (pInfo->SDESInfo->phone, text, 256);
}

void RtcpTx_setSdesLoc (uint32 sid, char* text)
{
	RtcpTransmitter *pInfo;

	if(sid > DSP_RTK_SS_NUM)
		return;
	pInfo = &RtcpTxInfo[sid];

    assert (strlen(text) < 255);
    strncpy (pInfo->SDESInfo->loc, text, 256);
}

void RtcpTx_setSdesTool (uint32 sid, char* text)
{
	RtcpTransmitter *pInfo;

	if(sid > DSP_RTK_SS_NUM)
		return;
	pInfo = &RtcpTxInfo[sid];

    assert (strlen(text) < 255);
    strncpy (pInfo->SDESInfo->tool, text, 256);
}

void RtcpTx_setSdesNote (uint32 sid, char* text)
{
	RtcpTransmitter *pInfo;

	if(sid > DSP_RTK_SS_NUM)
		return;
	pInfo = &RtcpTxInfo[sid];

    assert (strlen(text) < 255);
    strncpy (pInfo->SDESInfo->note, text, 256);
}


char* RtcpTx_getSdesCname (uint32 sid)
{
	RtcpTransmitter *pInfo;

	if(sid > DSP_RTK_SS_NUM)
		return NULL;
	pInfo = &RtcpTxInfo[sid];

    return pInfo->SDESInfo->cname;
}

char* RtcpTx_getSdesName (uint32 sid)
{
	RtcpTransmitter *pInfo;

	if(sid > DSP_RTK_SS_NUM)
		return NULL;
	pInfo = &RtcpTxInfo[sid];

    return pInfo->SDESInfo->name;
}

char* RtcpTx_getSdesEmail (uint32 sid)
{
	RtcpTransmitter *pInfo;

	if(sid > DSP_RTK_SS_NUM)
		return NULL;
	pInfo = &RtcpTxInfo[sid];

    return pInfo->SDESInfo->email;
}

char* RtcpTx_getSdesPhone (uint32 sid)
{
	RtcpTransmitter *pInfo;

	if(sid > DSP_RTK_SS_NUM)
		return NULL;
	pInfo = &RtcpTxInfo[sid];

    return pInfo->SDESInfo->phone;
}

char* RtcpTx_getSdesLoc (uint32 sid)
{
	RtcpTransmitter *pInfo;

	if(sid > DSP_RTK_SS_NUM)
		return NULL;
	pInfo = &RtcpTxInfo[sid];

    return pInfo->SDESInfo->loc;
}

char* RtcpTx_getSdesTool (uint32 sid)
{
	RtcpTransmitter *pInfo;

	if(sid > DSP_RTK_SS_NUM)
		return NULL;
	pInfo = &RtcpTxInfo[sid];

    return pInfo->SDESInfo->tool;
}

char* RtcpTx_getSdesNote (uint32 sid)
{
	RtcpTransmitter *pInfo;

	if(sid > DSP_RTK_SS_NUM)
		return NULL;
	pInfo = &RtcpTxInfo[sid];

    return pInfo->SDESInfo->note;
}




/* --- misc functions ---------------------------------------------- */
void RtcpTx_setRTPtran (uint32 sid, RtpTransmitter* s)
{
	RtcpTransmitter *pInfo;

	if(sid > DSP_RTK_SS_NUM)
		return;
	pInfo = &RtcpTxInfo[sid];

    if (pInfo->SDESInfo == NULL)
        pInfo->SDESInfo = pInfo->SDESdataBuf;

    pInfo->tran = s;
}

void RtcpTx_setRTPrecv (uint32 sid, RtpReceiver* s)
{
	RtcpTransmitter *pInfo;

	if(sid > DSP_RTK_SS_NUM)
		return;
	pInfo = &RtcpTxInfo[sid];
    pInfo->recv = s;
}

void RtcpTx_setRTCPrecv (uint32 sid, RtcpReceiver* s)
{
	RtcpTransmitter *pInfo;

	if(sid > DSP_RTK_SS_NUM)
		return;
	pInfo = &RtcpTxInfo[sid];
    pInfo->rtcpRecv = s;
}

int RtcpTx_getPort (uint32 sid)
{
	RtcpTransmitter *pInfo;

	if(sid > DSP_RTK_SS_NUM)
		return 0;
	pInfo = &RtcpTxInfo[sid];

    return pInfo->localPort;
};

void RtcpTx_setMode(uint32 sid, RtcpTransmitMode opmode)
{
	RtcpTransmitter *pInfo;

	if(sid > DSP_RTK_SS_NUM)
		return 0;
	pInfo = &RtcpTxInfo[sid];
	
	pInfo ->txMode = opmode;
}

int RtcpTx_getLogger( uint32 sid, TstVoipRtcpLogger *pLogger )
{
	RtcpTransmitter *pInfo;

	if(sid > DSP_RTK_SS_NUM)
		return 0;
	pInfo = &RtcpTxInfo[sid];
	
	// caller set pLogger data as zeros 
	
	pLogger ->TX_packet_count = pInfo ->txLogger.packet_count;
	if( pInfo ->txLogger.fraction_loss.count ) {
		pLogger ->TX_loss_rate_max = pInfo ->txLogger.fraction_loss.max;
		pLogger ->TX_loss_rate_min = pInfo ->txLogger.fraction_loss.min;
		pLogger ->TX_loss_rate_avg = pInfo ->txLogger.fraction_loss.sum / 
										pInfo ->txLogger.fraction_loss.count;
		pLogger ->TX_loss_rate_cur = pInfo ->txLogger.fraction_loss.last;
	}
	if( pInfo ->txLogger.inter_jitter.count ) {
		pLogger ->TX_jitter_max = pInfo ->txLogger.inter_jitter.max;
		pLogger ->TX_jitter_min = pInfo ->txLogger.inter_jitter.min;
		pLogger ->TX_jitter_avg = pInfo ->txLogger.inter_jitter.sum / 
									pInfo ->txLogger.inter_jitter.count;
		pLogger ->TX_jitter_cur = pInfo ->txLogger.inter_jitter.last;
	}
	if( pInfo ->txLogger.round_trip_delay.count ) {
		pLogger ->TX_round_trip_max = pInfo ->txLogger.round_trip_delay.max;
		pLogger ->TX_round_trip_min = pInfo ->txLogger.round_trip_delay.min;
		pLogger ->TX_round_trip_avg = pInfo ->txLogger.round_trip_delay.sum / 
										pInfo ->txLogger.round_trip_delay.count;
		pLogger ->TX_round_trip_cur = pInfo ->txLogger.round_trip_delay.last;
	}
	if( pInfo ->txLogger.MOS_LQ.count ) {
		pLogger ->TX_MOS_LQ_max_x10 = pInfo ->txLogger.MOS_LQ.max;
		pLogger ->TX_MOS_LQ_min_x10 = pInfo ->txLogger.MOS_LQ.min;
		pLogger ->TX_MOS_LQ_avg_x10 = pInfo ->txLogger.MOS_LQ.sum / 
										pInfo ->txLogger.MOS_LQ.count;
		pLogger ->TX_MOS_LQ_cur_x10 = pInfo ->txLogger.MOS_LQ.last;	
	}
	
	pLogger ->TX_MOS_LQ_max = pLogger ->TX_MOS_LQ_max_x10 / 10;
	pLogger ->TX_MOS_LQ_min = pLogger ->TX_MOS_LQ_min_x10 / 10;
	pLogger ->TX_MOS_LQ_avg = pLogger ->TX_MOS_LQ_avg_x10 / 10;
	pLogger ->TX_MOS_LQ_cur = pLogger ->TX_MOS_LQ_cur_x10 / 10;
	
	return 1;	
}

/*
NetworkAddress* RtcpTx_getRemoteAddr (uint32 sid)
{
	RtcpTransmitter *pInfo;

	if(sid > SESS_NUM)
		return;
	pInfo = &RtcpTxInfo[sid];

	return &(pInfo->remoteAddr);
}
*/

void rtcp_logger_unit_new_data( RtcpStatisticsUnit *pUnit,
								unsigned long data )
{
	unsigned long t;
	
	if( pUnit ->count == 0 ) {	// first one 
		pUnit ->max =
			pUnit ->min =
			pUnit ->sum =
			pUnit ->last = data;
		pUnit ->count = 1;
		return;
	}
		
	// max? 
	if( pUnit ->max < data )
		pUnit ->max = data;
	
	// min?
	if( pUnit ->min > data )
		pUnit ->min = data;
	
	// sum
	t = pUnit ->sum;
	
	pUnit ->sum += data;
	pUnit ->count ++;
	
	if( t > pUnit ->sum ) {	// overflow!!
		if( pUnit ->count <= 16 ) {
			pUnit ->sum = ( t >> 1 );
			pUnit ->count >>= 1;
		} else {
			pUnit ->sum = ( t >> 4 );
			pUnit ->count >>= 4;
		}
		
		if( pUnit ->count == 0 )
			pUnit ->count = 1;
	}
	
	// last 
	pUnit ->last = data;
}

static int rtcp_logger_unit_string( char *buff, 
									const RtcpStatisticsUnit *pUnit )
{
	int n = 0;
	
	n += sprintf( buff + n, "\tmax   = %lu\n", pUnit ->max );
	n += sprintf( buff + n, "\tmin   = %lu\n", pUnit ->min );
	n += sprintf( buff + n, "\tsum   = %lu\n", pUnit ->sum );
	n += sprintf( buff + n, "\tcount = %lu\n", pUnit ->count );
	n += sprintf( buff + n, "\tlast  = %lu\n", pUnit ->last );
	
	return n;
}

int rtcp_logger_string( char *buff, 
								const RtcpStatisticsLogger *pLogger )
{
	int n = 0;
	
	n += sprintf( buff + n, "packet count: %lu\n", pLogger ->packet_count );
	
	n += sprintf( buff + n, "fraction_loss (x/256):\n" );
	n += rtcp_logger_unit_string( buff + n, &pLogger ->fraction_loss );
	n += sprintf( buff + n, "inter_jitter (timestamp unit):\n" );
	n += rtcp_logger_unit_string( buff + n, &pLogger ->inter_jitter );
	n += sprintf( buff + n, "round_trip_delay (ms):\n" );
	n += rtcp_logger_unit_string( buff + n, &pLogger ->round_trip_delay );
	n += sprintf( buff + n, "MOS_LQ:\n" );
	n += rtcp_logger_unit_string( buff + n, &pLogger ->MOS_LQ );
	
	return n;
}

static int voip_rtcp_logtx_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
#ifdef CONFIG_RTK_VOIP_RTCP_XR
	extern int rtcp_tx_logger_MOSLQ_string( char *buff, uint32 sid );
#endif
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
		
		ss = SS_FROM_PROC_DATA( data );
		n = sprintf( buf, "session=%d\n", ss );
		
		pLogger = &RtcpTxInfo[ ss ].txLogger;
		
		n += rtcp_logger_string( buf + n, pLogger );
#ifdef CONFIG_RTK_VOIP_RTCP_XR
		n += rtcp_tx_logger_MOSLQ_string( buf + n, ss );
#endif
	}
	
	*eof = 1;
	return n;
}

static int voip_rtcp_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int /*ch,*/ ss;
	int n = 0;
	
	extern unsigned char RtcpOpen[];

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	if( IS_CH_PROC_DATA( data ) ) {
		//ch = CH_FROM_PROC_DATA( data );
		//n = sprintf( buf, "channel=%d\n", ch );
	} else {
		const RtcpReport *pRtcpReport;
		
		ss = SS_FROM_PROC_DATA( data );
		n = sprintf( buf, "session=%d\n", ss );
		n += sprintf( buf + n, "RTCP status:\t%s\n", RtcpOpen[ ss ] ? "On" : "Off" );
		n += sprintf( buf + n, "Tx Interval:\t%d ms\n", RtcpTxInfo[ ss ].txInterval );
		
#ifdef CONFIG_RTK_VOIP_RTCP_XR
		n += sprintf( buf + n, "enable XR:\t%u\n", RtcpTxEnableXR[ ss ] );
#endif
		
#ifdef RTCP_REPORT_IN_PROC
		pRtcpReport = &RtcpReportPrev[ ss ];
		n += sprintf( buf + n, "SSRC:\t%u\n", pRtcpReport ->ssrc );
		n += sprintf( buf + n, "fracLost:\t%u (1/256)\n", pRtcpReport ->fracLost );
		n += sprintf( buf + n, "cumLost:\t%u\n", 
							( ( uint32 )pRtcpReport ->cumLost[ 2 ] ) |
							( ( uint32 )pRtcpReport ->cumLost[ 1 ] << 8 ) |
							( ( uint32 )pRtcpReport ->cumLost[ 0 ] << 16 ) );
		n += sprintf( buf + n, "interJitter:\t%u (timestamp)\n", 
									pRtcpReport ->jitter );
		n += sprintf( buf + n, "lastSRTimeStamp:\t%u (NTP32)\n", 
									pRtcpReport ->lastSRTimeStamp );
		n += sprintf( buf + n, "lastSRDelay:\t%u (1/65536 seconds)\n", 
									pRtcpReport ->lastSRDelay );
#endif
	}
	
	*eof = 1;
	return n;
}

int __init voip_rtcp_proc_init( void )
{
	//create_voip_channel_proc_read_entry( "rtcp", voip_rtcp_read_proc );
	create_voip_session_proc_read_entry( "rtcp", voip_rtcp_read_proc );
	create_voip_session_proc_read_entry( "rtcp_logtx", voip_rtcp_logtx_read_proc );

	return 0;
}

void __exit voip_rtcp_proc_exit( void )
{
	//remove_voip_channel_proc_entry( "rtcp" );
	remove_voip_session_proc_entry( "rtcp" );
	remove_voip_session_proc_entry( "rtcp_logtx" );
}

voip_initcall_proc( voip_rtcp_proc_init );
voip_exitcall( voip_rtcp_proc_exit );

#endif /* SUPPORT_RTCP */

