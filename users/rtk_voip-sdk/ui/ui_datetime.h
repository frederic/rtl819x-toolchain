#ifndef __UI_DATETIME_H__
#define __UI_DATETIME_H__

#ifndef __T_DATETIME__
#define __T_DATETIME__
typedef union datetime_s {	/* 32 bits datetime format */
	unsigned long	all;
	struct {
		unsigned long	year:6;	/* 2000 ~ 2063 */
		unsigned long	mon:4;	/* 0 ~ 11: 1 ~ 12 */
		unsigned long	day:5;	/* 1 ~ 31 */
		unsigned long	hh:5;	/* 0 ~ 24 */
		unsigned long	mm:6;	/* 0 ~ 59 */
		unsigned long	ss:6;	/* 0 ~ 59 */
	} s;
} datetime_t;
#endif /* __T_DATETIME__ */

extern datetime_t GetCurrentDateTime( void );
extern datetime_t MakeDateTime( unsigned int year, unsigned int mon, unsigned int day,
						 unsigned int hour, unsigned int min, unsigned int sec );

#define DATETIME_STRING_LEN					19	/* exclusive null terminator */
#define LOW_PRECISION_DATETIME_STRING_LEN	16	/* exclusive null terminator */
#define SHORT_DATETIME_STRING_LEN			15	/* exclusive null terminator */

extern void MakeDateTimeString( unsigned char *pszDateTime, datetime_t datetime );
extern void MakeLowPrecisionDateTimeString( unsigned char *pszDateTime, datetime_t datetime );
extern void MakeShortDateTimeString( unsigned char *pszShortDateTime, datetime_t datetime );

#endif /* __UI_DATETIME_H__ */
