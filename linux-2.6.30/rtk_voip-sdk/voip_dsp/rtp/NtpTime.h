#ifndef NTPTIME_H
#define NTPTIME_H

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

#include "rtk_voip.h"
#include "voip_types.h"
//#include <sys/types.h>
/* Kao
#include "vtypes.h"
*/

/* unsigned operations */
#define _div32(x, y) ((x) / (y))
#define _mul32(x, y) ((x) * (y))
#define _mod32(x, y) ((x) % (y))

/* signed operations */
#define _idiv32(x, y) ((x) / (y))
#define _imul32(x, y) ((x) * (y))
#define _imod32(x, y) ((x) % (y))

typedef struct stNtpTimeval
{
	unsigned long	tv_sec;		/* seconds */
	unsigned long	tv_usec;	/* and microseconds */
}NtpTimeval;

///
typedef struct stNtpTime
{
	uint32 seconds;
	uint32 fractional;
}NtpTime;

// express middle 32 bits of 64 in NTP 
typedef uint32 Ntp32Time;	// 32 bits NTP 
typedef int32  Ntp32Diff;	// difference between two 32 bits NTP 

void Ntp_Init(void);
void Ntp_TimeInit(NtpTime* time, uint32 sec, uint32 frac);
void Ntp_TimeInitNull(NtpTime* time);
void Ntp_cpy(NtpTime* time1, const NtpTime* time2);

void NTP_addms(const NtpTime* rhs , const unsigned int delayMs, NtpTime* time_result);
void NTP_subms(const NtpTime* rhs , const unsigned int ms, NtpTime* time_result);
int NTP_sub(const NtpTime* rhs , const NtpTime* lhs);
bool NTP_isEqual(const NtpTime* rhs , const NtpTime* lhs);
bool NTP_isLarge(const NtpTime* rhs , const NtpTime* lhs);
bool NTP_isLess(const NtpTime* rhs , const NtpTime* lhs);
void Ntp_getTime(NtpTime* time_result);

void NTP_timetick (void);

//
// NTP32 - middle 32 bits of 64 bits NTP 
//

static inline Ntp32Time NTP32_getNTP32( const NtpTime* time )
{
	return ( ( time ->seconds ) << 16 ) | ( ( time ->fractional ) >> 16 );
}

static inline Ntp32Diff NTP32_sub( Ntp32Time lhs, Ntp32Time rhs)
{
	return ( lhs - rhs );	// may be negative 
}

static inline uint32 NTP32_diff2ms( Ntp32Diff diff )
{
	if( diff < 0 )
		diff *= -1;		// abs 
	
	//diff = diff / 65536 * 1000;
	//     = ( diff >> 16 ) * 1000
	//     = ( ( diff >> 10 ) * 1000 ) >> 6		(large number)
	//     = ( diff * 1000 ) >> 16				(small number)
	
	// 1000 occupies 10 bits 
	
	if( diff & 0xFFC00000 )	// handle dynamic range (0xFFC00000: MSB 10 bits)
		return ( ( diff >> 10 ) * 1000 ) >> 6;
	else
		return ( diff * 1000 ) >> 16;
}

#endif // NTPTIME_H

