
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

//static const char* const RtcpPacket_cxx_Version =
//    "$Id: RtcpPacket.c,v 1.3 2008-01-15 06:33:23 kennylin Exp $";

/* Kao
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
*/
#include "assert.h"
/*#include <time.h>
#include <sys/types.h>*/
/* Kao
#include "vtypes.h"
*/
/*#include <unistd.h>*/
#include <linux/string.h>

/* Kao
// networking
#include <sys/types.h>
#include <sys/socket.h>
*/
/*#include <netinet/in.h>*/

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
//#include "types.h"
/*#include <debug.h>*/

#ifdef SUPPORT_RTCP

/*///////////////////////////////////////////////////////////////////*/
/* global variable */
/*RtcpPacket RTCP_RX_DEC[MAX_SESS_NUM][RTP_DEC_NUM];*/
/*RtcpPacket RTCP_TX_DEC[RTP_DEC_NUM];*/

/* ----------------------------------------------------------------- */
/* --- rtcp Packet Constructor ------------------------------------- */
/* ----------------------------------------------------------------- */

void RtcpPkt_Init (RtcpPacket* pst)
{
	pst->packetData = NULL;

    // create memory allocation
    pst->packetAlloc = PACKETSIZE;
    assert (pst->packetAlloc < RTP_MTU);
    pst->packetData = pst->packetbuf;
    assert (pst->packetData);
    memset (pst->packetData, 0, pst->packetAlloc);
    pst->unusedSize = pst->packetAlloc;
}

/*
void RtcpPkt_Exit (void)
{
    delete []packetData; packetData = NULL;
}
*/


/* --- Size and Locations ------------------------------------------ */

char* RtcpPkt_getPacketData (RtcpPacket* pst)
{
    return pst->packetData;
}

char* RtcpPkt_freeData (RtcpPacket* pst)
{
    return pst->packetData + (pst->packetAlloc - pst->unusedSize);
}

int RtcpPkt_allocData (RtcpPacket* pst, int s)
{
    if (pst->unusedSize < s)
    {
        //cpLog (LOG_ERR, "RTCP: Out of allocated memory for RTCP packet");
        return -1;
    }
    pst->unusedSize -= s;
    memset (RtcpPkt_freeData(pst), 0, s);
    return s;
}


/*     unused memory    */

int RtcpPkt_getPacketAlloc (RtcpPacket* pst)
{
    return pst->packetAlloc;
}

int RtcpPkt_getUnused (RtcpPacket* pst)
{
    return pst->unusedSize;
}

void RtcpPkt_setTotalUsage (RtcpPacket* pst, int size)
{
    assert (size <= pst->packetAlloc);
    pst->unusedSize = pst->packetAlloc - size;
    //printk("RtcpPkt_setTotalUsage, size = %d\n", size);
}

int RtcpPkt_getTotalUsage (RtcpPacket* pst)
{
    return pst->packetAlloc - pst->unusedSize;
}



/* --- Packet Header functions ------------------------------------- */

int RtcpPkt_getVersion (RtcpPacket* pst)
{
    RtcpHeader* header = (RtcpHeader*) (pst->packetData);
    return header->version;
}

int RtcpPkt_getPadbyteFlag (RtcpPacket* pst)
{
    RtcpHeader* header = (RtcpHeader*) (pst->packetData);
    return header->padding;
}

int RtcpPkt_getCount (RtcpPacket* pst)
{
    RtcpHeader* header = (RtcpHeader*) (pst->packetData);
    return header->count;
}

RtcpType RtcpPkt_getPayloadType (RtcpPacket* pst)
{
    RtcpHeader* header = (RtcpHeader*) (pst->packetData);
    return (RtcpType) (header->type);
}


int RtcpPkt_getLength (RtcpPacket* pst)
{
    RtcpHeader* header = (RtcpHeader*) (pst->packetData);
    return ntohs(header->length);
}

#endif /* SUPPORT_RTCP */

