#ifndef __UI_VKEY_H__
#define __UI_VKEY_H__

#define VKEY_0				'0'
#define VKEY_1		        '1'
#define VKEY_2		        '2'
#define VKEY_3		        '3'
#define VKEY_4		        '4'
#define VKEY_5		        '5'
#define VKEY_6		        '6'
#define VKEY_7		        '7'
#define VKEY_8		        '8'
#define VKEY_9		        '9'
#define VKEY_STAR	        '*'
#define VKEY_POUND	        '#'

#define VKEY_OUTVOL_PLUS	'+'
#define VKEY_OUTVOL_MINUS	'-'
#define VKEY_SPEAKER		'S'

#define VKEY_HOOK			'K'

#define VKEY_OFF_HOOK		'O'		/* VKEY_HOOK is divided into OFF_ and ON_ HOOK */
#define VKEY_ON_HOOK		'o'

#define VKEY_LONG_CLEAR		'r'		/* long-pressed VKEY_BACKSPACE */
#define VKEY_IME			VKEY_POUND

#define VKEY_FXO			VKEY_MUTE		/* borrow to switch FXO */

/* ================================================ */
#if KEYPAD_MAP_VENDOR == VENDOR_WCO

#define VKEY_OK				'M'
#define VKEY_CANCEL			'C'

#define VKEY_UP				'^'
#define VKEY_DOWN			'v'
#define VKEY_LEFT			'<'
#define VKEY_RIGHT			'>'

#define VKEY_CONFERENCE		'c'
#define VKEY_PICK			'P'
#define VKEY_TRANSFER		'T'
#define VKEY_REDIAL			'R'
#define VKEY_HOLD			'H'

#define VKEY_LINE1			'L'
#define VKEY_LINE2			'l'
#define VKEY_F1				'F'
#define VKEY_F2				'g'
#define VKEY_F3				'b'
#define VKEY_FORWARD		'f'
#define VKEY_DND			'd'
#define VKEY_MISSED			'm'
#define VKEY_VMS			'V'
#define VKEY_BLIND_XFER		'x'
#define VKEY_MUTE			'u'
#define VKEY_HEADSET		'h'

#define VKEY_MENU			VKEY_OK
#define VKEY_PHONEBOOK		VKEY_F1
#define VKEY_BACKSPACE		VKEY_LEFT

#define VKEY_MICSPK_SWITCH	VKEY_HEADSET	/* borrow to switch mic/spk adjustment */

/* ================================================ */
#elif KEYPAD_MAP_VENDOR == VENDOR_WCO2

#define VKEY_OK				'M'	//VKEY_INS_B1
#define VKEY_CANCEL			'C'	//VKEY_INS_B2
#define VKEY_CLEAR			'a'

#define VKEY_UP				'^'
#define VKEY_DOWN			'v'
#define VKEY_LEFT			VKEY_UP
#define VKEY_RIGHT			VKEY_DOWN

#define VKEY_MUTE			'u'
#define VKEY_HEADSET		'h'

#define VKEY_CONFERENCE		'c'
#define VKEY_PICK			'P'
#define VKEY_TRANSFER		'T'
#define VKEY_REDIAL			'R'
#define VKEY_HOLD			'H'

#define VKEY_LINE1			'L'
#define VKEY_LINE2			'l'
#define VKEY_F1				'F'
#define VKEY_F2				'g'
#define VKEY_F3				'b'
#define VKEY_FORWARD		'f'
#define VKEY_DND			'd'
#define VKEY_MISSED			'm'
#define VKEY_VMS			'V'
#define VKEY_BLIND_XFER		'x'

#define VKEY_INS_B1			')'
#define VKEY_INS_B2			'('
#define VKEY_INS_B3			'\\'
#define VKEY_INS_B4			'/'
#define VKEY_INS_L1			'['
#define VKEY_INS_L2			']'
#define VKEY_INS_L3			'{'
#define VKEY_INS_L4			'}'

#define VKEY_MENU			VKEY_OK
#define VKEY_PHONEBOOK		'p'	//VKEY_F1
#define VKEY_BACKSPACE		VKEY_LEFT

#define VKEY_MICSPK_SWITCH	VKEY_HEADSET	/* borrow to switch mic/spk adjustment */

/* ================================================ */
#elif KEYPAD_MAP_VENDOR == VENDOR_TCO

#define VKEY_OK				'M'
#define VKEY_CANCEL			'C'

#define VKEY_ENTER			VKEY_OK		/* It is a very special application */

#define VKEY_UP				'^'
#define VKEY_DOWN			'v'
//#define VKEY_UP				VKEY_OUTVOL_PLUS
//#define VKEY_DOWN			VKEY_OUTVOL_MINUS
#define VKEY_LEFT			'<'
#define VKEY_RIGHT			'>'

#define VKEY_HEADSET		'h'

#define VKEY_MENU			'm'

#define VKEY_F1				'!'
#define VKEY_F2				'@'
#define VKEY_F3				':'
#define VKEY_F4				'$'
#define VKEY_F5				'%'
#define VKEY_F6				'|'
#define VKEY_F7				'&'
#define VKEY_F8				';'
#define VKEY_F9				'('
#define VKEY_F10			')'

#define VKEY_PHONEBOOK		VKEY_F3
#define VKEY_MICSPK_SWITCH	VKEY_F2	/* borrow to switch mic/spk adjustment */

#define VKEY_REDIAL			'R'
#define VKEY_LINE1			VKEY_F10
#define VKEY_LINE2			VKEY_F9
#define VKEY_CONFERENCE		'c'
#define VKEY_TRANSFER		'T'
#define VKEY_MUTE			'u'
#define VKEY_HOLD			'H'
#define VKEY_CALL_LOG		'A'

// followings are undefined keys 
#define VKEY_PICK			'?'
#define VKEY_MISSED			'?'

#define VKEY_BACKSPACE		VKEY_LEFT

/* ================================================ */
#elif KEYPAD_MAP_VENDOR == VENDOR_BCO

#define VKEY_OK				VKEY_ENTER
#define VKEY_BACKSPACE		VKEY_DELETE
#define VKEY_CANCEL			VKEY_DELETE

#define VKEY_ENTER			'M'
#define VKEY_DELETE			'C'

#define VKEY_MENU			'm'
#define VKEY_PHONEBOOK		'p'

#define VKEY_UP				'^'
#define VKEY_DOWN			'v'
#define VKEY_LEFT			VKEY_OUTVOL_MINUS
#define VKEY_RIGHT			VKEY_OUTVOL_PLUS

#define VKEY_REDIAL			'R'
#define VKEY_CONFERENCE		'c'
#define VKEY_TRANSFER		'T'
#define VKEY_HOLD			'H'
#define VKEY_CALL_LOG		'A'
#define VKEY_DND			'D'

// followings are sfotkey 
#define VKEY_LINE1			'L'
#define VKEY_LINE2			'l'

// followings are undefined keys 
#define VKEY_PICK			'?'
#define VKEY_MISSED			'?'
#define VKEY_MUTE			'?'
#define VKEY_MICSPK_SWITCH	VKEY_DND

/* ================================================ */
#else
#define VKEY_MENU        	'M'
#define VKEY_UP		        '^'
#define VKEY_DOWN	        'v'
#define VKEY_CANCEL	        'C'
#define VKEY_TXT_NUM		'E'
#define VKEY_TRANSFER	    'T'
#define VKEY_REDIAL	        'R'

#define VKEY_INVOL_PLUS		'='
#define VKEY_INVOL_MINUS	'_'
#define VKEY_HOLD			'H'
#define VKEY_NET			'N'
#define VKEY_MESSAGE		'G'
#define VKEY_LINE1			'L'
#define VKEY_LINE2			'I'
#define VKEY_CONFERENCE		'c'
#define VKEY_BLIND_XFER		'B'
#define VKEY_DND			'D'
#define VKEY_M1				'e'
#define VKEY_M2				'r'
#define VKEY_M3				's'
#define VKEY_M4				'f'
#define VKEY_M5				'w'
#define VKEY_PHONEBOOK		'p'

#define VKEY_OK				VKEY_MENU
#define VKEY_BACKSPACE		VKEY_UP
#define VKEY_LEFT			VKEY_UP
#define VKEY_RIGHT			VKEY_DOWN

#endif /* KEYPAD_MAP_VENDOR */

#endif /* __UI_VKEY_H__ */

