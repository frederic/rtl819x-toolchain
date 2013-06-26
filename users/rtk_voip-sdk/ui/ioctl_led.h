#ifndef __IOCTL_LED_H__
#define __IOCTL_LED_H__

/* ================================================ */
#if LED_MAP_VENDOR == VENDOR_WCO

#define LED_BIT_OK			0x0001
#define LED_BIT_unused1		0x0002
#define LED_BIT_unused2		0x0004
#define LED_BIT_CANCEL		0x0008

#define LED_BIT_LINE1		0x0010
#define LED_BIT_LINE2		0x0020
#define LED_BIT_F1			0x0040
#define LED_BIT_F2			0x0080

#define LED_BIT_F3			0x0100
#define LED_BIT_FORWARD		0x0200
#define LED_BIT_DND			0x0400
#define LED_BIT_MISSED		0x0800

#define LED_BIT_VMS			0x1000
#define LED_BIT_BLIND_TRAN	0x2000
#define LED_BIT_MUTE		0x4000
#define LED_BIT_HEADSET		0x8000

#define LED_MASK_ALL		0xFFF9

/* ================================================ */
#elif LED_MAP_VENDOR == VENDOR_WCO2

#define LED_BIT_CONF		0x00000004	/* bit 2 */
#define LED_BIT_PICK		0x00000020	/* bit 5 */
#define LED_BIT_TRANSFER	0x00000100	/* bit 8 */
#define LED_BIT_REDIAL		0x00000200	/* bit 9 */
#define LED_BIT_HOLD		0x00000400	/* bit 10 */

#define LED_BIT_LINE1		0x00000002	/* bit 1 */
#define LED_BIT_LINE2		0x00000010	/* bit 4 */
#define LED_BIT_F1			0x00000080	/* bit 7 */
#define LED_BIT_F2			0x00000800	/* bit 11 */
#define LED_BIT_F3			0x00001000	/* bit 12 */

#define LED_BIT_FORWARD		0x00000001	/* bit 0 */
#define LED_BIT_DND			0x00000008	/* bit 3 */
#define LED_BIT_MISSED		0x00000040	/* bit 6 */
#define LED_BIT_VMS			0x00002000	/* bit 13 */
#define LED_BIT_BLIND_TRAN	0x00004000	/* bit 14 */

#define LED_BIT_HEADSET		0x00008000	/* bit 15 */
#define LED_BIT_MUTE		0x00010000	/* bit 16 */
#define LED_BIT_SPEAKER		0x00020000	/* bit 17 */

#define LED_BIT_SYS1		0x00040000	/* bit 18 */
#define LED_BIT_SYS2		0x00080000	/* bit 19 */
#define LED_BIT_SYS3		0x00100000	/* bit 20 */

#define LED_MASK_ALL		0x001FFFFF

/* ================================================ */
#elif LED_MAP_VENDOR == VENDOR_TCO
/* Use 22 bits among 28 bits */
#define LED_BIT_PG1			0x00000001
#define LED_BIT_PB1			0x00000002
//#define LED_BIT_PR1			0x00000004

#define LED_BIT_LEDR6		0x00000010
#define LED_BIT_LEDG6		0x00000040

#define LED_BIT_LEDR5		0x00000100
#define LED_BIT_LEDG5		0x00000400

#define LED_BIT_LEDR4		0x00001000
#define LED_BIT_LEDR10		0x00002000
#define LED_BIT_LEDG4		0x00004000
//#define LED_BIT_LEDG10		0x00008000
#define LED_BIT_SPKR		0x00008000

#define LED_BIT_LEDR3		0x00010000
#define LED_BIT_LEDR9		0x00020000
#define LED_BIT_LEDG3		0x00040000
#define LED_BIT_LEDG9		0x00080000

#define LED_BIT_LEDR2		0x00100000
#define LED_BIT_LEDR8		0x00200000
#define LED_BIT_LEDG2		0x00400000
#define LED_BIT_LEDG8		0x00800000

#define LED_BIT_LEDR1		0x01000000
#define LED_BIT_LEDR7		0x02000000
#define LED_BIT_LEDG1		0x04000000
#define LED_BIT_LEDG7		0x08000000

#define LED_BIT_SERVICE		( LED_BIT_PG1 | LED_BIT_PB1 )
#define LED_BIT_F10			( LED_BIT_LEDR10 /*| LED_BIT_LEDG10*/ )
#define LED_BIT_F9			( LED_BIT_LEDR9 | LED_BIT_LEDG9 )
#define LED_BIT_F8			( LED_BIT_LEDR8 | LED_BIT_LEDG8 )
#define LED_BIT_F7			( LED_BIT_LEDR7 | LED_BIT_LEDG7 )
#define LED_BIT_F6			( LED_BIT_LEDR6 | LED_BIT_LEDG6 )
#define LED_BIT_F5			( LED_BIT_LEDR5 | LED_BIT_LEDG5 )
#define LED_BIT_F4			( LED_BIT_LEDR4 | LED_BIT_LEDG4 )
#define LED_BIT_F3			( LED_BIT_LEDR3 | LED_BIT_LEDG3 )
#define LED_BIT_F2			( LED_BIT_LEDR2 | LED_BIT_LEDG2 )
#define LED_BIT_F1			( LED_BIT_LEDR1 | LED_BIT_LEDG1 )

#define LED_BIT_LINE1		LED_BIT_F1		/* to be modified */
#define LED_BIT_LINE2		LED_BIT_F2		/* to be modified */

#define LED_MASK_RED		( LED_BIT_PB1 | LED_BIT_LEDR1 | LED_BIT_LEDR2 | LED_BIT_LEDR3 | LED_BIT_LEDR4 | LED_BIT_LEDR5 | LED_BIT_LEDR6 | LED_BIT_LEDR7 | LED_BIT_LEDR8 | LED_BIT_LEDR9 | LED_BIT_LEDR10 )
#define LED_MASK_GREEN		( LED_BIT_PG1 | LED_BIT_LEDG1 | LED_BIT_LEDG2 | LED_BIT_LEDG3 | LED_BIT_LEDG4 | LED_BIT_LEDG5 | LED_BIT_LEDG6 | LED_BIT_LEDG7 | LED_BIT_LEDG8 | LED_BIT_LEDG9 | LED_BIT_SPKR )

#define LED_MASK_ALL		0x0FFFFFFF

/* ================================================ */
#elif LED_MAP_VENDOR == VENDOR_BCO
#define LED_BIT_HANDFREE	0x00000001	/* handfree */
#define LED_BIT_VOIP1		0x00000002	/* SIP register indicator */

#define LED_MASK_ALL		0x00000003

/* ================================================ */
#else
  #error "LED_MAP_VENDOR??"
#endif

/* LED general purpose control function */
extern void TurnOnOffLEDThruoghGPIO( unsigned long mask, unsigned long set );

/* Turn off all LED  */
static inline void TurnOffAllLEDsThroughGPIO( void )
{
	TurnOnOffLEDThruoghGPIO( LED_MASK_ALL, 0 );
}

/* Turn on all LED */
static inline void TurnOnAllLEDsThroughGPIO( void )
{
	TurnOnOffLEDThruoghGPIO( LED_MASK_ALL, LED_MASK_ALL );
}

/* Turn off single LED */
static inline void TurnOffLEDThroughGPIO( unsigned long bit )
{
	TurnOnOffLEDThruoghGPIO( bit, 0 );
}

/* Turn on single LED */
static inline void TurnOnLEDThroughGPIO( unsigned long bit )
{
	TurnOnOffLEDThruoghGPIO( bit, bit );
}
#endif /* __IOCTL_LED_H__ */

