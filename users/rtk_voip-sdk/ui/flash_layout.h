#ifndef __FLASH_LAYOUT_H__
#define __FLASH_LAYOUT_H__

/*
 * If someone try to add structure to flash layout, please pay attention
 * to followings:
 *  - The file 'include/ui_flash_layout.h' link to 'flash_layout.h' to 
 *    assist in flash modules of webs/solar. 
 *    Thus, UI files do NOT try to include 'ui_flash_layout.h'. 
 *  - All references in file should be put in 'include' fold. 
 */

#include "ui_limits.h"		/* for MAX_LEN_OF_PHONENUMBER */

#pragma pack( 1 )

/* datetime_t copied from ui_datetime.h */
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

/* Call Records */
#define NUM_OF_CALL_RECORD_UNITS		10
#define NUM_OF_CALL_RECORD_DATETIME		3

typedef struct call_records_unit_s {
	unsigned char	szPhonenumber[ MAX_LEN_OF_PHONENUMBER + 1 ];
	datetime_t		datetime[ NUM_OF_CALL_RECORD_DATETIME ];
} call_records_unit_t;

typedef struct call_records_fold_s {
	unsigned char		used;		/* used number */
	unsigned char		sort[ NUM_OF_CALL_RECORD_UNITS ];
	call_records_unit_t	unit[ NUM_OF_CALL_RECORD_UNITS ];
} call_records_fold_t;

typedef struct call_records_s {
	call_records_fold_t missed;
	call_records_fold_t incoming;
	call_records_fold_t outgoing;
} call_records_t;

/* Phonebook */
#define NUM_OF_PHONEBOOK_RECORD			50
#define MAX_LEN_OF_PHONE_NAME			15	
#define MAX_LEN_OF_PHONE_NUMBER			15	/* store in BCD format */
#define BCD_LEN_OF_PHONE_NUMBER			( ( MAX_LEN_OF_PHONE_NUMBER + 1 ) / 2 )

typedef struct phonebook_record_s {
	unsigned char szName[ MAX_LEN_OF_PHONE_NAME + 1 ];	/* +1 for null-terminator */
	unsigned char bcdNumber[ BCD_LEN_OF_PHONE_NUMBER ];
} phonebook_record_t;

typedef struct phonebook_info_s {
	unsigned int		nNumberOfPhonebookRecord;
	unsigned int		riFromViPhonebookRecordSort[ NUM_OF_PHONEBOOK_RECORD ];
	phonebook_record_t	records[ NUM_OF_PHONEBOOK_RECORD ];
} phonebook_info_t;

/* mode settings */
typedef struct mode_info_s {
	unsigned char		bKeypressTone;
	unsigned char 		nModeOutVolumeReceiver;
	unsigned char 		nModeOutVolumeSpeaker;
	unsigned char 		nModeInVolumeMic_R;
	unsigned char 		nModeInVolumeMic_S;
	unsigned char		nAutoDialTime;
	unsigned char		nAutoAnswerTime;
	unsigned char		nOffHookAlarmTime;
	unsigned char		bHotLine;
	unsigned char		bcdHotLine[ BCD_LEN_OF_PHONE_NUMBER ];
} mode_info_t;

/* flash layout */
typedef struct ui_flash_layout_s {
	unsigned long		version;
	unsigned long		features;
	unsigned long		size;
	unsigned long		signature;
	/* phonebook */
	phonebook_info_t	phonebook;
	/* call records */
	call_records_t		callrecord;
	/* mode settings */
	mode_info_t			mode;
} ui_falsh_layout_t;

#define MAX_ADDR_OF_FLASH	sizeof( ui_falsh_layout_t )

#pragma pack()

#endif /* __FLASH_LAYOUT_H__ */
