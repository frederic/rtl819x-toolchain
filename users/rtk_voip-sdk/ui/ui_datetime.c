#include "ui_config.h"
#include "ui_datetime.h"
#include "res.h"
#include <time.h>
#include <stdio.h>

datetime_t GetCurrentDateTime( void )
{
	time_t now;
	const struct tm *tm_now;
	datetime_t ui_now;

	time( &now );

	tm_now = localtime( &now );

	ui_now.s.year = tm_now ->tm_year + 1900 - 2000;		/* tm_year: 1900+, ui_year: 2000+ */
	ui_now.s.mon = tm_now ->tm_mon;
	ui_now.s.day = tm_now ->tm_mday;
	ui_now.s.hh = tm_now ->tm_hour;
	ui_now.s.mm = tm_now ->tm_min;
	ui_now.s.ss = tm_now ->tm_sec;

	return ui_now;
}

void MakeDateTimeString( unsigned char *pszDateTime, datetime_t datetime )
{
	/* look like '2007-07-02 13:05:03' --> textual length 19 */

	sprintf( ( char * )pszDateTime, "%u-%02u-%02u %02u:%02u:%02u", 
								datetime.s.year + 2000, datetime.s.mon + 1, datetime.s.day,
								datetime.s.hh, datetime.s.mm, datetime.s.ss );
}

void MakeLowPrecisionDateTimeString( unsigned char *pszDateTime, datetime_t datetime )
{
	/* look like '2007-07-02 13:05' --> textual length 16 */

	sprintf( ( char * )pszDateTime, "%u-%02u-%02u %02u:%02u", 
								datetime.s.year + 2000, datetime.s.mon + 1, datetime.s.day,
								datetime.s.hh, datetime.s.mm );
}

void MakeShortDateTimeString( unsigned char *pszShortDateTime, datetime_t datetime )
{
#if 0
	static const unsigned char * const pszMonth[] = {
		szMonthJan, szMonthFeb, szMonthMar, szMonthApr, 
		szMonthMay, szMonthJun, szMonthJul, szMonthAug, 
		szMonthSep, szMonthOct, szMonthNov, szMonthDec, 
	};

  #if defined( LANG_BIG5 ) || defined( LANG_GB2312 )
	sprintf( ( char * )pszShortDateTime, "%s%u %02u:%02u:%02u", 
									pszMonth[ datetime.s.mon ], datetime.s.day,
									datetime.s.hh, datetime.s.mm, datetime.s.ss );
  #else
	sprintf( ( char * )pszShortDateTime, "%u %s %02u:%02u:%02u", 
									datetime.s.day, pszMonth[ datetime.s.mon ],
									datetime.s.hh, datetime.s.mm, datetime.s.ss );
  #endif

#else
	/* look like '13 Jul 13:05:03' --> textual length 15 */
#define _M_MONTH( s )	( const unsigned char * )s

	static const unsigned char * const pszMonth[] = {
		_M_MONTH( "Jan" ), _M_MONTH( "Feb" ), _M_MONTH( "Mar" ), _M_MONTH( "Apr" ), 
		_M_MONTH( "May" ), _M_MONTH( "Jun" ), _M_MONTH( "Jul" ), _M_MONTH( "Aug" ), 
		_M_MONTH( "Sep" ), _M_MONTH( "Oct" ), _M_MONTH( "Nov" ), _M_MONTH( "Dec" ), 
	};

	sprintf( ( char * )pszShortDateTime, "%u %s %02u:%02u:%02u", 
									datetime.s.day, pszMonth[ datetime.s.mon ],
									datetime.s.hh, datetime.s.mm, datetime.s.ss );

#undef _M_MONTH
#endif
}

datetime_t MakeDateTime( unsigned int year, unsigned int mon, unsigned int day,
						 unsigned int hour, unsigned int min, unsigned int sec )
{
	datetime_t datetime;

	datetime.s.year = ( year < 2000 ? 0 : year - 2000 );
	datetime.s.mon = mon;
	datetime.s.day = day;
	datetime.s.hh = hour;
	datetime.s.mm = min;
	datetime.s.ss = sec;

	return datetime;
}
