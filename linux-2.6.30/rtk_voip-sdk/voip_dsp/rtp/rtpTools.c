
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

//static const char* const rtpTools_cxx_Version =
//    "$Id: rtpTools.c,v 1.3 2008-01-15 06:33:23 kennylin Exp $";

/* Kao
#include <iostream>
#include <stdio.h>
*/
//#include <stdlib.h>
//#include "rtpTypes.h"
//#include <linux/rand.h>
#include <linux/net.h>
#include "rtpTools.h"
#ifdef DEBUG_LOG
#include "cpLog.h"
#endif
//#include "vsock.hxx"
//#include <debug.h>

/* ----------------------------------------------------------------- */
/* --- Number Functions -------------------------------------------- */
/* ----------------------------------------------------------------- */

/*  32-bit random number     */
u_int32_t generate32 (void)
{
	// should be seeded by main program
/* Kao
    return random();
*/
//	return rand();
//	return 1483;
	return net_random();
}

/*  random SRC number        */
RtpSrc generateSRC(void)
{
	// doesn't check for collision
	RtpSrc src = 0;
	while (src == 0)
		src = generate32();
	return src;
}

BOOL RtpSeqGreater (RtpSeqNumber a, RtpSeqNumber b)
{
	if ( (a > (RTP_SEQ_MOD - 20000) && b < 20000) || (a < 20000 && b > RTP_SEQ_MOD - 20000) )
	{
		//cpLog(LOG_DEBUG_STACK,"backwards compare %d > %d", a, b);
		return (a < b);
	}
	else
		return (a > b);
}
