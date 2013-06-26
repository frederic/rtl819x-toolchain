#ifndef _KEYPAD_MAP_H_
#define _KEYPAD_MAP_H_

// ------------------------------------------------------------------ //
// Keypad map for IP phone v100, v101 
// ------------------------------------------------------------------ //
#if defined( CONFIG_RTK_VOIP_GPIO_IPP_100 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_101 )

#define KEY_0			0x17
#define KEY_1		    0x18
#define KEY_2		    0x49
#define KEY_3		    0x36
#define KEY_4		    0x01
#define KEY_5		    0x15
#define KEY_6		    0x37
#define KEY_7		    0x13
#define KEY_8		    0x39
#define KEY_9		    0x38
#define KEY_STAR	    0x14
#define KEY_POUND	    0x16

//#define KEY_MENU	        0x33	// ??
//#define KEY_TXT_NUM		0x23	// ??
//#define KEY_INVOL_PLUS		0x21	// ??
//#define KEY_INVOL_MINUS		0x51	// ??
//#define KEY_NET			0x56	// ??
//#define KEY_MESSAGE		0x02	// ??
//#define KEY_M1			0x41	// ??
//#define KEY_M2			0x01	// ??
//#define KEY_M3			0x11	// ??
//#define KEY_M4			0x15	// ??
//#define KEY_M5			0x05	// ??
//#define KEY_PhoneBook		0x45	// ??

#define KEY_OUTVOL_PLUS		0x45
#define KEY_OUTVOL_MINUS	0x46
#define KEY_SPEAKER		0x78

#define KEY_OK			0x59
#define KEY_CANCEL		0x25

#define KEY_UP			0x02
#define KEY_DOWN		0x12
#define KEY_LEFT		0x23
#define KEY_RIGHT		0x35

#define KEY_CONFERENCE	0x48
#define KEY_PICK		0x08
#define KEY_TRANSFER	0x89
#define KEY_REDIAL		0x06
#define KEY_HOLD		0x69

#define KEY_LINE1		0x04
#define KEY_LINE2		0x57
#define KEY_F1			0x56
#define KEY_F2			0x34
#define KEY_F3			0x05
#define KEY_FORWARD		0x27
#define KEY_DND			0x24
#define KEY_MISSED		0x47
#define KEY_VMS			0x79
#define KEY_BLIND_XFER	0x67
#define KEY_MUTE		0x19
#define KEY_HEADSET		0x58

#define KEY_HOOK		0x0F	/* TODO: modify */

// ------------------------------------------------------------------ //
// Keypad map for IP phone 8972_v00  
// ------------------------------------------------------------------ //
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V00 )

// octal notation 
#define KEY_0			001
#define KEY_1		    013
#define KEY_2		    014
#define KEY_3		    015
#define KEY_4		    010
#define KEY_5		    011
#define KEY_6		    012
#define KEY_7		    003
#define KEY_8		    004
#define KEY_9		    005
#define KEY_STAR	    000
#define KEY_POUND	    002

#define KEY_OUTVOL_PLUS		025	
#define KEY_OUTVOL_MINUS	022	
#define KEY_SPEAKER		020

#define KEY_OK			031
#define KEY_CANCEL		024

#define KEY_MENU		021
#define KEY_HEADSET		030

#define KEY_F1			044
#define KEY_F2			041
#define KEY_F3			034
#define KEY_F4			023
#define KEY_F5			032
#define KEY_F6			035	
#define KEY_F7			042
#define KEY_F8			057		// undefined
#define KEY_F9			057		// undefined
#define KEY_F10			057		// undefined

#define KEY_UP			057		// undefined
#define KEY_DOWN		057		// undefined
#define KEY_LEFT		057		// undefined
#define KEY_RIGHT		057		// undefined

#define KEY_CONFERENCE	057		// undefined
#define KEY_PICK		057		// undefined
#define KEY_TRANSFER	057		// undefined
#define KEY_REDIAL		057		// undefined
#define KEY_HOLD		057		// undefined

#define KEY_LINE1		057		// undefined
#define KEY_LINE2		057		// undefined
#define KEY_FORWARD		057		// undefined
#define KEY_DND			057		// undefined
#define KEY_MISSED		057		// undefined
#define KEY_VMS			057		// undefined
#define KEY_BLIND_XFER	057		// undefined
#define KEY_MUTE		057		// undefined

#define KEY_HOOK		045

// ------------------------------------------------------------------ //
// Keypad map for IP phone 8972_v01  
// ------------------------------------------------------------------ //
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V01 )

// octal notation 
#define KEY_0			0001
#define KEY_1		    0013
#define KEY_2		    0014
#define KEY_3		    0113	// double key 
#define KEY_4		    0010
#define KEY_5		    0011
#define KEY_6		    0110	// double key 
#define KEY_7		    0003
#define KEY_8		    0004
#define KEY_9		    0103	// double key 
#define KEY_STAR	    0000
#define KEY_POUND	    0100	// double key 

#define KEY_OUTVOL_PLUS		0030	
#define KEY_OUTVOL_MINUS	0023	
#define KEY_SPEAKER		0020

#define KEY_OK			0044
#define KEY_CANCEL		0043

#define KEY_MENU		0033
#define KEY_HEADSET		0057		// undefined

#define KEY_F1			0101	// double key 
#define KEY_F2			0104	// double key 
#define KEY_F3			0111	// double key 
#define KEY_F4			0114	// double key 
#define KEY_F5			0121	// double key 
#define KEY_F6			0124	// double key 
#define KEY_F7			0131	// double key
#define KEY_F8			0134	// double key 
#define KEY_F9			0141	// double key 
#define KEY_F10			0144	// double key 

#define KEY_UP			0133	// double key 
#define KEY_DOWN		0130	// double key 
#define KEY_LEFT		0040
#define KEY_RIGHT		0041
#define KEY_ENTER		0140	// double key 

#define KEY_CONFERENCE	0031
#define KEY_PICK		0057		// undefined
#define KEY_TRANSFER	0034
#define KEY_REDIAL		0021
#define KEY_HOLD		0120	// double key 

#define KEY_LINE1		0057		// undefined
#define KEY_LINE2		0057		// undefined
#define KEY_FORWARD		0057		// undefined
#define KEY_DND			0057		// undefined
#define KEY_MISSED		0057		// undefined
#define KEY_VMS			0057		// undefined
#define KEY_BLIND_XFER	0057		// undefined
#define KEY_MUTE		0123	// double key 

#define KEY_CALL_LOG	0024
#define KEY_SHIFT		0143	// double key ">>" 

#define KEY_HOOK		0046

// ------------------------------------------------------------------ //
// Keypad map for IP phone 8952_v00  
// ------------------------------------------------------------------ //
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8952_V00 )

#define KEY_0			0x28
#define KEY_1		    0x07
#define KEY_2		    0x17
#define KEY_3		    0x2A
#define KEY_4		    0x38
#define KEY_5		    0x47
#define KEY_6		    0x57
#define KEY_7		    0x3A
#define KEY_8		    0x39
#define KEY_9		    0x37
#define KEY_STAR	    0x29
#define KEY_POUND	    0x27

#define KEY_OUTVOL_PLUS		0x67
#define KEY_OUTVOL_MINUS	0x4C

#define KEY_HEADSET		0x69
#define KEY_MUTE		0x19
#define KEY_SPEAKER		0x59

//#define KEY_OK			0x00	// undefined
//#define KEY_CANCEL		0x00	// undefined

//#define KEY_MENU		0x00	// undefined

#define KEY_UP			0x49
#define KEY_DOWN		0x09

#define KEY_CONFERENCE	0x48
#define KEY_PICK		0x08
#define KEY_TRANSFER	0x68
#define KEY_REDIAL		0x18
#define KEY_HOLD		0x58

#define KEY_LINE1		0x4A
#define KEY_LINE2		0x0A
#define KEY_F1			0x6A
#define KEY_F2			0x1A
#define KEY_F3			0x5A
#define KEY_FORWARD		0x4B
#define KEY_DND			0x5C
#define KEY_MISSED		0x6B
#define KEY_VMS			0x6C
#define KEY_BLIND_XFER	0x5B

#define KEY_INS_B1		0x0B
#define KEY_INS_B2		0x0C
#define KEY_INS_B3		0x1B
#define KEY_INS_B4		0x1C
#define KEY_INS_L1		0x3C
#define KEY_INS_L2		0x3B
#define KEY_INS_L3		0x2C
#define KEY_INS_L4		0x2B

#define KEY_HOOK		0x7E

// ------------------------------------------------------------------ //
// Keypad map for IP phone 8972B_v00  
// ------------------------------------------------------------------ //
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00 )

#define KEY_0			0x19
#define KEY_1		    0x06
#define KEY_2		    0x16
#define KEY_3		    0x26
#define KEY_4		    0x07
#define KEY_5		    0x17
#define KEY_6		    0x27
#define KEY_7		    0x08
#define KEY_8		    0x18
#define KEY_9		    0x28
#define KEY_STAR	    0x09
#define KEY_POUND	    0x29

#define KEY_OUTVOL_PLUS		0x36
#define KEY_OUTVOL_MINUS	0x37
#define KEY_SPEAKER		0x0A

//#define KEY_OK			KEY_ENTER
#define KEY_DELETE		0x1C
//#define KEY_CANCEL		KEY_DELETE

#define KEY_MENU		0x0C
#define KEY_PHONEBOOK	0x38
//#define KEY_HEADSET		0x

#define KEY_UP			0x46
#define KEY_DOWN		0x47
//#define KEY_LEFT		0x37	// KEY_OUTVOL_MINUS
//#define KEY_RIGHT		0x36	// KEY_OUTVOL_PLUS
#define KEY_ENTER		0x3A

#define KEY_CONFERENCE	0x0B
//#define KEY_PICK		0x
#define KEY_TRANSFER	0x2A
#define KEY_REDIAL		0x1A
#define KEY_HOLD		0x2B

#define KEY_LINE1		0x		// undefined
#define KEY_LINE2		0x		// undefined
#define KEY_FORWARD		0x		// undefined
#define KEY_DND			0x3C	// undefined
#define KEY_MISSED		0x		// undefined
#define KEY_VMS			0x		// undefined
#define KEY_BLIND_XFER	0x		// undefined
#define KEY_MUTE		0x	

#define KEY_CALL_LOG	0x2C

#define KEY_HOOK		0x7E

#else
  #error "Bad version definition!!"
#endif


#if 0
struct keypad_dev_s
{
	volatile unsigned char flags;	//0: own by driver, 1: own by AP
	unsigned char input_count;	//AP should clear to 0
	unsigned char data_string[50];
	
};
#endif

struct keypad_dev_s
{
	volatile unsigned char flags;	//0: own by driver, 1: own by AP
	unsigned char data_string;	
};
	

typedef struct keypad_dev_s keypad_dev_t;

extern keypad_dev_t keypad_data_pool;






#endif	//_KEYPAD_MAP_H_
