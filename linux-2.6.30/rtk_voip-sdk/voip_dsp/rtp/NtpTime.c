//#define __NET_TRACE		1

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

//static const char* const NtpTime_cxx_Version =
//    "$Id: NtpTime.c,v 1.3 2008-01-15 06:33:23 kennylin Exp $";

#include "NtpTime.h"
/*#include <sys/types.h>*/

/*#ifndef __vxworks
#include <sys/time.h>
#endif*/

/*#include <unistd.h>*/
#include "assert.h"
/*#include <debug.h>*/

#ifdef SUPPORT_RTCP

//#define USE_LINUX_KERNEL_TIMEKEEPING	// use linux's do_gettimeofday() or not 

// The offset between Jan/1/1970 and Jan/1/1900
// The time value got on Linux is base on Jan/1/1970
// The Ntp time on the internet is base on Jan/1/1900
//static u_int32_t Second_Diff = 0x83AA7E80;
static unsigned long Second_Diff = 0x83AA7E80;
//static unsigned long Second_Diff = 0x83AAFFFF - 15 * 60 - 2;	// 15 minutes cause low 16 bits overflow 

#ifdef USE_LINUX_KERNEL_TIMEKEEPING
#include <linux/time.h>
#else
static unsigned long Ntp_sec;
static unsigned long Ntp_msec;
#endif

/* Kao
void NtpTime::print()
{
    fprintf (stderr, "%u.%010u", seconds, fractional);
}
*/

void Ntp_Init(void)
{
#ifndef USE_LINUX_KERNEL_TIMEKEEPING
	Ntp_sec = 0;
	Ntp_msec = 0;
#endif
}

void Ntp_TimeInit(NtpTime* time, uint32 sec, uint32 frac)
{
	time->seconds = sec;
	time->fractional = frac;
}

void Ntp_TimeInitNull(NtpTime* time)
{
/*	NtpTime* pInfo = NULL;
	if(sid >= sess_num)
		return;
	pInfo = &NtpTimeInfo[sid];
	*pInfo = Ntp_getTime();*/
	time->seconds = 0;
	time->fractional = 0;
}

void Ntp_cpy(NtpTime* time1, const NtpTime* time2)
{
	time1->seconds = time2->seconds;
	time1->fractional = time2->fractional;
}

// 2^32 = 4294,967,296
/* NtpTime NTP_addms( const NtpTime* lhs , const unsigned int msec) */
void NTP_addms( const NtpTime* lhs , const unsigned int msec, NtpTime* result_time)
{
    NtpTime result;

    uint32 delayFrac = _mul32(_mod32(msec, 1000), 4294967);

    Ntp_TimeInitNull(&result);

    result.seconds = lhs->seconds + _div32(msec, 1000);
    result.fractional = lhs->fractional + delayFrac;
    if ( (lhs->fractional > result.fractional) && (delayFrac > result.fractional) )
        result.seconds++;

	result_time->seconds = result.seconds;
	result_time->fractional = result.fractional;
/*	return result; */
}


/* NtpTime NTP_subms( const NtpTime* lhs , const unsigned int msec) */
void NTP_subms( const NtpTime* lhs , const unsigned int msec, NtpTime* result_time)
{
    NtpTime result;

    uint32 delayFrac = _mul32(_mod32(msec, 1000), 4294967);

	Ntp_TimeInitNull(&result);

    if (lhs->seconds > _div32(msec, 1000))
    {
        result.seconds = lhs->seconds - _div32(msec, 1000);
        result.fractional = lhs->fractional - delayFrac;
        if ( delayFrac > lhs->fractional )
        {
            result.seconds--;
        }
    }
    else
    {
        result.seconds = 0;
        if ( delayFrac >= lhs->fractional )
        {
            result.fractional = 0;
        }
        else
        {
            result.fractional = lhs->fractional - delayFrac;
        }
    }

	result_time->seconds = result.seconds;
	result_time->fractional = result.fractional;

/*    return result; */
}

// It returns the difference in milisec between lhs and rhs
int NTP_sub( const NtpTime* lhs , const NtpTime* rhs )
{
    NtpTime result;
    unsigned int msResult;

/*	Illustration :
	The next statement will go to : 
	NtpTime.cxx		bool operator==( const NtpTime& rhs , const NtpTime& lhs ) . eric wu in 2004.11.02 .
*/
/*    if (lhs == rhs) return 0; */

/*	Illustration :
	The next statement will go to : 
	NtpTime.cxx		bool operator>( const NtpTime& rhs , const NtpTime& lhs ) . eric wu in 2004.11.02 .
*/
    /*if (lhs > rhs) */
	if(lhs->seconds >= rhs->seconds)
    {
/*	Illustration :
	The power on procedure case , other does not check . eric wu in 2004.11.02 .
*/

        result.seconds = lhs->seconds - rhs->seconds;

        /*if (lhs.fractional > rhs.fractional)*/
        if (lhs->fractional >= rhs->fractional)
        {
/*	Illustration :
	The power on procedure case , other does not check . eric wu in 2004.11.02 .
*/
            result.fractional = lhs->fractional - rhs->fractional;
        }
        else if (lhs->fractional < rhs->fractional)
        {
            result.seconds--;
            result.fractional = lhs->fractional - rhs->fractional;
        }
       /* else
        {
            result.fractional = 0;
        }
        msResult = _mul32(result.getSeconds(), 1000)
                   + _div32(result.getFractional(), 4294967);*/
		msResult = _mul32(result.seconds, 1000)
                   + _div32(result.fractional, 4294967);
    }
    else
    {
        result.seconds = rhs->seconds - lhs->seconds;

        /*if (rhs.fractional > lhs.fractional)*/
		if (rhs->fractional >= lhs->fractional)
        {
            result.fractional = rhs->fractional - lhs->fractional;
        }
        else if (rhs->fractional < lhs->fractional)
        {
            result.seconds--;
            result.fractional = rhs->fractional - lhs->fractional;
        }
/*        else
        {
            result.fractional = 0;
        }
        msResult = -( _mul32(result.getSeconds(), 1000)
                      + _div32(result.getFractional(), 4294967));*/

        msResult = -( _mul32(result.seconds, 1000)
                      + _div32(result.fractional, 4294967));
    }

    return (int)msResult;
}

bool NTP_isEqual( const NtpTime* rhs , const NtpTime* lhs )
{
    return (rhs->seconds == lhs->seconds) ?
           (rhs->fractional == lhs->fractional) : (rhs->seconds == lhs->seconds);
}

bool NTP_isLess( const NtpTime* rhs , const NtpTime* lhs )
{
    return (rhs->seconds == lhs->seconds) ?
           (rhs->fractional < lhs->fractional) : (rhs->seconds < lhs->seconds);
}


bool NTP_isLarge( const NtpTime* rhs , const NtpTime* lhs )
{
    return (rhs->seconds == lhs->seconds) ?
           (rhs->fractional > lhs->fractional) : (rhs->seconds > lhs->seconds);
}

int NTP_gettimeofday (NtpTimeval *tv)
{
#ifndef USE_LINUX_KERNEL_TIMEKEEPING
/*    unsigned long int msec; // msec since 00:00 GMT
    
    // iptime() returns msec since 00:00 GMT in reverse byte order
    // call ntohl to swap it
    msec = ntohl ( iptime() );     
    tv->tv_sec = ( msec / 1000 );
    tv->tv_usec = ( msec % 1000 ) * 1000;*/

/*	tv->tv_sec = ( Ntp_sec / 1000 );*/
	tv->tv_sec = Ntp_sec + Second_Diff;
    tv->tv_usec = ( Ntp_msec %1000 )*1000;
#else
	extern void do_gettimeofday(struct timeval *tv);
	
	struct timeval tv_linux;
	
	do_gettimeofday( &tv_linux );
	
	tv->tv_sec	= tv_linux.tv_sec + Second_Diff;
	tv->tv_usec	= tv_linux.tv_usec;
#endif

    return 0;
}

/* NtpTime Ntp_getTime(void) */
void Ntp_getTime(NtpTime* result_time)
{
    NtpTimeval now;
    NtpTime result;
    int err;
	err = NTP_gettimeofday(&now);
    assert( !err );

	Ntp_TimeInit(&result, now.tv_sec, _mul32(now.tv_usec, 4294));
/*	Ntp_TimeInit(&result, now.tv_sec, now.tv_msec);*/

	result_time->seconds = result.seconds;
	result_time->fractional = result.fractional;

/*    return result;*/
}

/*uint32 ticka, tickb;
#define REG32(offset) 			(*((volatile uint32 *)(0xBD010000+offset)))*/
void NTP_timetick (void)
{
#ifndef USE_LINUX_KERNEL_TIMEKEEPING
/*	uint32 time;*/

 	Ntp_msec += 10;

/*	ticka = (REG32(0x202c)>>8);	//timer1
	time = (tickb-ticka)*40/1000;
	tickb = ticka;
	printk("%dms\n", time);*/
	if(Ntp_msec >= 1000)
	{
		Ntp_sec++;
		Ntp_msec = Ntp_msec % 1000;
	}
#endif
}

#endif /* SUPPORT_RTCP */

