#ifndef __UI_RECORDS_H__
#define __UI_RECORDS_H__

#include "ui_datetime.h"

extern void AddRecordOfMissedCall( const unsigned char *pszPhonenumber, datetime_t datetime );
extern void AddRecordOfIncomingCall( const unsigned char *pszPhonenumber, datetime_t datetime );
extern void AddRecordOfOutgoingCall( const unsigned char *pszPhonenumber, datetime_t datetime );

extern unsigned int GetTheLastOutgoingCallRecord( unsigned char *pszPhonenumber );

#endif /* __UI_RECORDS_H__ */
