#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include "../gpio/gpio.h"
#include "base_gpio.h"
#include <linux/delay.h>	//udelay(),mdelay()
#include "lcm_splc780d.h"

/* 
 * This driver support 4 and 8 bits mode of 16x2 character LCM. 
 * 4 bits mode is identical to lcm_char16x2, but we strongly 
 * recommend this one. 
 */

#define SPLC780D_8BITS_MODE 
//#define SPLC780D_PRINT_INS_CODE
#define SPLC780D_NS_DELAY

// GPIO PIN definition 
#define SP_PIN_RS		GPIO_ID(GPIO_PORT_D,4)	//output
#define SP_PIN_E		GPIO_ID(GPIO_PORT_D,7)	//output
#define SP_PIN_RW		GPIO_ID(GPIO_PORT_E,0)	//output
#ifdef SPLC780D_8BITS_MODE
#define SP_PIN_DB0		GPIO_ID(GPIO_PORT_B,1)	//input/output (out only) 
#define SP_PIN_DB1		GPIO_ID(GPIO_PORT_B,2)	//input/output (out only) 
#define SP_PIN_DB2		GPIO_ID(GPIO_PORT_B,3)	//input/output (out only) 
#define SP_PIN_DB3		GPIO_ID(GPIO_PORT_C,3)	//input/output
#endif
#define SP_PIN_DB4		GPIO_ID(GPIO_PORT_C,4)	//input/output
#define SP_PIN_DB5		GPIO_ID(GPIO_PORT_D,0)	//input/output
#define SP_PIN_DB6		GPIO_ID(GPIO_PORT_D,2)	//input/output
#define SP_PIN_DB7		GPIO_ID(GPIO_PORT_D,3)	//input/output

// SPLC780D Command definition 
#define LCM_CMD_CLEAR_DISPLAY			0x01
#define LCM_CMD_RETURN_HOME				0x02
#define LCM_CMD_ENTRY_MODE_SET			0x04
#define LCM_CMD_ENTRY_MODE_SET_INC			0x02
#define LCM_CMD_ENTRY_MODE_SET_DEC			0x00
#define LCM_CMD_ENTRY_MODE_SET_SHIFT		0X01
#define LCM_CMD_DISPLAY_CONTROL			0x08
#define LCM_CMD_DISPLAY_CONTROL_DISPLAY		0x04
#define LCM_CMD_DISPLAY_CONTROL_CURSOR		0x02
#define LCM_CMD_DISPLAY_CONTROL_BLINK		0x01
#define LCM_CMD_CURSOR_DISPLAY_SHIFT	0x10
#define LCM_CMD_CURSOR_DISPLAY_SHIFT_SC		0x80
#define LCM_CMD_CURSOR_DISPLAY_SHIFT_RL		0x40
#define LCM_CMD_FUNCTION_SET			0x20
#define LCM_CMD_FUNCTION_SET_DATA_LEN_8BITS	0x10	// 8 bit mode
#define LCM_CMD_FUNCTION_SET_DATA_LEN_4BITS	0x00	// 4 bit mode
#define LCM_CMD_FUNCTION_SET_DISPLAY_2LINES	0x08	// 2 or 1 line
#define LCM_CMD_FUNCTION_SET_FONT_TYPE		0x04	// 5x10 or 5x8 dots 
#define LCM_CMD_SET_CGRAM_ADDRESS		0x40
#define LCM_CMD_SET_CGRAM_ADDRESS_MASK		0x3F
#define LCM_CMD_SET_DDRAM_ADDRESS		0x80
#define LCM_CMD_SET_DDRAM_ADDRESS_MASK		0x7F

// LCM size 
#define SPLC780D_LCM_HEIGHT		2
#define SPLC780D_LCM_WIDTH		16

static unsigned char CheckBusyFlag = 0;
#ifndef SPLC780D_8BITS_MODE
static unsigned char ConfigurationStage = 1;		// LCM configuration stage 
#endif


// internal functions 
static inline void splc780d_udelay( unsigned long delay )
{
	// In our experimental
	//        4bits  8bits
	// 1  ... ok     ok*
	// 2  ... ok     ok*
	// 3  ...        ok
	// 4  ... 
	// 5  ... ok     ok
	// 10 ... ok     ok
	// 20 ... 
	//
	// * Need additional delay in reset 
	
	if( delay < 1 )
		udelay( 1 );
	else
		udelay( delay );
}

static inline void splc780d_mdelay( unsigned long delay )
{
	mdelay( delay );
}

#ifdef SPLC780D_NS_DELAY
static __inline__ void __nsdelay(unsigned long nssecs, unsigned long lpj)
{
	/* This function is a copy of __udelay() in linux-2.4.18/include/asm-mips/delay.h */
    unsigned long lo;

    //usecs *= 0x00068db8;        /* 2**32 / (1000000 / HZ) */
    nssecs *= 0x000001AE;        /* 2**32 / (1000000 * 1000 / HZ) */
    __asm__("multu\t%2,%3"
        :"=h" (nssecs), "=l" (lo)
        :"r" (nssecs),"r" (lpj));
    __delay(nssecs);
}
#endif

static inline void splc780d_nsdelay( unsigned long delay )
{
#ifdef SPLC780D_NS_DELAY
	__nsdelay( delay, __udelay_val );	// __udelay_val defined in linux-2.4.18/include/asm-mips/delay.h
#else
	splc780d_udelay( delay / 1000 );	// convert ns to us 
#endif
}

static void splc780d_config_data_dir( const enum GPIO_DIRECTION dir )
{
#ifdef SPLC780D_8BITS_MODE
	__lcm_initGpioPin(SP_PIN_DB0, dir, GPIO_INT_DISABLE);
	__lcm_initGpioPin(SP_PIN_DB1, dir, GPIO_INT_DISABLE);
	__lcm_initGpioPin(SP_PIN_DB2, dir, GPIO_INT_DISABLE);	
	__lcm_initGpioPin(SP_PIN_DB3, dir, GPIO_INT_DISABLE);	
#endif
	__lcm_initGpioPin(SP_PIN_DB4, dir, GPIO_INT_DISABLE);	
	__lcm_initGpioPin(SP_PIN_DB5, dir, GPIO_INT_DISABLE);	
	__lcm_initGpioPin(SP_PIN_DB6, dir, GPIO_INT_DISABLE);	
	__lcm_initGpioPin(SP_PIN_DB7, dir, GPIO_INT_DISABLE);
}

static void splc780d_init_lcm_gpio( void )
{
	__lcm_initGpioPin(SP_PIN_RS, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	__lcm_initGpioPin(SP_PIN_E, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	__lcm_initGpioPin(SP_PIN_RW, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	
	splc780d_config_data_dir( GPIO_DIR_OUT );
	
	__lcm_setGpioDataBit( SP_PIN_E, 0 );	// pull low E 
}

static void splc780d_write_core( unsigned char RS, unsigned char DB )
{
	int i;
#ifdef SPLC780D_8BITS_MODE
	const int c = 1;
#else
	const int c = ( ConfigurationStage ? 1 : 2 );
#endif

#ifdef SPLC780D_PRINT_INS_CODE
	printk( "Write: RS=%d, DB=%02X\n", RS, DB );
#endif
		
	splc780d_config_data_dir( GPIO_DIR_OUT );

	for( i = 0; i < c; i ++ ) {		
		__lcm_setGpioDataBit( SP_PIN_RS, RS );
		
		__lcm_setGpioDataBit( SP_PIN_RW, 0 );	// write mode 
		
		splc780d_nsdelay( 40 );		// tSP1 = 40ns 
		
		__lcm_setGpioDataBit( SP_PIN_E, 1 );

#ifdef SPLC780D_8BITS_MODE
		__lcm_setGpioDataBit( SP_PIN_DB0, ( DB & 0x01 ? 1 : 0 ) );
		__lcm_setGpioDataBit( SP_PIN_DB1, ( DB & 0x02 ? 1 : 0 ) );
		__lcm_setGpioDataBit( SP_PIN_DB2, ( DB & 0x04 ? 1 : 0 ) );
		__lcm_setGpioDataBit( SP_PIN_DB3, ( DB & 0x08 ? 1 : 0 ) );
		__lcm_setGpioDataBit( SP_PIN_DB4, ( DB & 0x10 ? 1 : 0 ) );
		__lcm_setGpioDataBit( SP_PIN_DB5, ( DB & 0x20 ? 1 : 0 ) );
		__lcm_setGpioDataBit( SP_PIN_DB6, ( DB & 0x40 ? 1 : 0 ) );
		__lcm_setGpioDataBit( SP_PIN_DB7, ( DB & 0x80 ? 1 : 0 ) );
#else
		if( i == 0 ) {
			__lcm_setGpioDataBit( SP_PIN_DB4, ( DB & 0x10 ? 1 : 0 ) );
			__lcm_setGpioDataBit( SP_PIN_DB5, ( DB & 0x20 ? 1 : 0 ) );
			__lcm_setGpioDataBit( SP_PIN_DB6, ( DB & 0x40 ? 1 : 0 ) );
			__lcm_setGpioDataBit( SP_PIN_DB7, ( DB & 0x80 ? 1 : 0 ) );
		} else {
			__lcm_setGpioDataBit( SP_PIN_DB4, ( DB & 0x01 ? 1 : 0 ) );
			__lcm_setGpioDataBit( SP_PIN_DB5, ( DB & 0x02 ? 1 : 0 ) );
			__lcm_setGpioDataBit( SP_PIN_DB6, ( DB & 0x04 ? 1 : 0 ) );
			__lcm_setGpioDataBit( SP_PIN_DB7, ( DB & 0x08 ? 1 : 0 ) );
		}
#endif
		
		splc780d_nsdelay( 230 );		// tPW = 230ns (tSP2 = 80ns within this range) 
		
		__lcm_setGpioDataBit( SP_PIN_E, 0 );
		
		splc780d_nsdelay( 10 );			// tHD1 = 10ns
		
		__lcm_setGpioDataBit( SP_PIN_RS, ( RS ? 0 : 1 ) );
		
		splc780d_nsdelay( 260 );		// tC - tPW - tHD1 = 500 - 230 - 10 = 260 
	}
}

static unsigned char splc780d_read_core( unsigned char RS )
{
	int i;
#ifdef SPLC780D_8BITS_MODE
	const int c = 1;
#else
	const int c = ( ConfigurationStage ? 1 : 2 );
#endif

	uint32 temp;
	unsigned char DB = 0;
	
	splc780d_config_data_dir( GPIO_DIR_IN );
	
	for( i = 0; i < c; i ++ ) {
		__lcm_setGpioDataBit( SP_PIN_RS, RS );
		
		__lcm_setGpioDataBit( SP_PIN_RW, 1 );	// read mode 
		
		splc780d_nsdelay( 40 );		// tSP1 = 40ns 
		
		__lcm_setGpioDataBit( SP_PIN_E, 1 );
		
		splc780d_nsdelay( 120 );		// tD = 120ns 
		
#ifdef SPLC780D_8BITS_MODE
		__lcm_getGpioDataBit( SP_PIN_DB0, &temp );	DB |= ( temp ? 0x01 : 0 );
		__lcm_getGpioDataBit( SP_PIN_DB1, &temp );	DB |= ( temp ? 0x02 : 0 );
		__lcm_getGpioDataBit( SP_PIN_DB2, &temp );	DB |= ( temp ? 0x04 : 0 );
		__lcm_getGpioDataBit( SP_PIN_DB3, &temp );	DB |= ( temp ? 0x08 : 0 );
		__lcm_getGpioDataBit( SP_PIN_DB4, &temp );	DB |= ( temp ? 0x10 : 0 );
		__lcm_getGpioDataBit( SP_PIN_DB5, &temp );	DB |= ( temp ? 0x20 : 0 );
		__lcm_getGpioDataBit( SP_PIN_DB6, &temp );	DB |= ( temp ? 0x40 : 0 );
		__lcm_getGpioDataBit( SP_PIN_DB7, &temp );	DB |= ( temp ? 0x80 : 0 );
#else
		if( i == 0 ) {
			__lcm_getGpioDataBit( SP_PIN_DB4, &temp );	DB |= ( temp ? 0x10 : 0 );
			__lcm_getGpioDataBit( SP_PIN_DB5, &temp );	DB |= ( temp ? 0x20 : 0 );
			__lcm_getGpioDataBit( SP_PIN_DB6, &temp );	DB |= ( temp ? 0x40 : 0 );
			__lcm_getGpioDataBit( SP_PIN_DB7, &temp );	DB |= ( temp ? 0x80 : 0 );
		} else {
			__lcm_getGpioDataBit( SP_PIN_DB4, &temp );	DB |= ( temp ? 0x01 : 0 );
			__lcm_getGpioDataBit( SP_PIN_DB5, &temp );	DB |= ( temp ? 0x02 : 0 );
			__lcm_getGpioDataBit( SP_PIN_DB6, &temp );	DB |= ( temp ? 0x04 : 0 );
			__lcm_getGpioDataBit( SP_PIN_DB7, &temp );	DB |= ( temp ? 0x08 : 0 );
		}
#endif
		
		splc780d_nsdelay( 110 );		// tPW - tD = 230 - 120 = 110ns 
		
		__lcm_setGpioDataBit( SP_PIN_E, 0 );
		
		splc780d_nsdelay( 10 );			// tHD1 = 10ns
		
		__lcm_setGpioDataBit( SP_PIN_RS, ( RS ? 0 : 1 ) );
		
		splc780d_nsdelay( 260 );		// tC - tPW - tHD1 = 500 - 230  10 = 260 
	}
	
	return DB;
}

static void splc780d_check_flag_if_necessary( void )
{
	unsigned char busyflag;
	
	if( !CheckBusyFlag )
		return;
	
	while( 1 ) {
		busyflag = splc780d_read_core( 0 /* instruction */ );
		
		if( busyflag & 0x80 ) {
#ifdef SPLC780D_PRINT_INS_CODE	
			printk( "LCM_Busy\n" );
#endif
		} else
			break;
	} 

#ifdef SPLC780D_PRINT_INS_CODE	
	printk( "BF:%X\n", busyflag );
#endif
}

static inline void splc780d_write_instruction( unsigned char ins )
{
	splc780d_write_core( 0 /* instruction */, ins );
	
	splc780d_check_flag_if_necessary();		
}

// This function will read busy flag 
//static inline unsigned char splc780d_read_instruction( void )
//{
//	return splc780d_read_core( 0 /* instruction */ );
//}

static inline void splc780d_write_data( unsigned char data )
{
	splc780d_write_core( 1 /* data */, data );
	
	splc780d_check_flag_if_necessary();	
}

static inline unsigned char splc780d_read_data( void )
{
	return splc780d_read_core( 1 /* data */ );
}

static void splc780d_reset( void )
{
	CheckBusyFlag = 0;			// Don't check BusyFlag 
#ifndef SPLC780D_8BITS_MODE
	ConfigurationStage = 1;		// LCM configuration stage 
#endif
	
	//splc780d_mdelay( 15 );		// Wait time > 15 ms

	splc780d_write_instruction( LCM_CMD_FUNCTION_SET | 
								LCM_CMD_FUNCTION_SET_DATA_LEN_8BITS );
	
	splc780d_udelay( 4100 );	// Wait time > 4.1 ms = 4100 us
	
	splc780d_write_instruction( LCM_CMD_FUNCTION_SET | 
								LCM_CMD_FUNCTION_SET_DATA_LEN_8BITS );
	
	splc780d_udelay( 100 );		// Wait time > 100 us
	
	splc780d_write_instruction( LCM_CMD_FUNCTION_SET | 
								LCM_CMD_FUNCTION_SET_DATA_LEN_8BITS );
	
	CheckBusyFlag = 1;		// Check BusyFlag after this instruction 

#ifdef SPLC780D_8BITS_MODE	
	splc780d_udelay( 100 );		// Additional delay, or (udelay < 2) hack will be fail 

	splc780d_write_instruction( LCM_CMD_FUNCTION_SET |
								LCM_CMD_FUNCTION_SET_DATA_LEN_8BITS |
								LCM_CMD_FUNCTION_SET_DISPLAY_2LINES );
#else
	splc780d_write_instruction( LCM_CMD_FUNCTION_SET |
								LCM_CMD_FUNCTION_SET_DATA_LEN_4BITS |
								LCM_CMD_FUNCTION_SET_DISPLAY_2LINES );
	ConfigurationStage = 0;
	splc780d_write_instruction( LCM_CMD_FUNCTION_SET |
								LCM_CMD_FUNCTION_SET_DATA_LEN_4BITS |
								LCM_CMD_FUNCTION_SET_DISPLAY_2LINES );
#endif
	
	// Display off 
	splc780d_write_instruction( LCM_CMD_DISPLAY_CONTROL );	
	
	// Display clear 
	splc780d_write_instruction( LCM_CMD_CLEAR_DISPLAY );
	
	// Entry mode set 
	splc780d_write_instruction( LCM_CMD_ENTRY_MODE_SET |
								LCM_CMD_ENTRY_MODE_SET_INC );
}

void splc780d_DrawText( unsigned char x, unsigned char y, 
						unsigned char *pszText, int len )
{
	if( y >= SPLC780D_LCM_HEIGHT || x >= SPLC780D_LCM_WIDTH )
		return;
	
	if( x + len > SPLC780D_LCM_WIDTH )	/* too long */
		len = SPLC780D_LCM_WIDTH - x;
		
	/*
	 * Now, it normalize to 
	 *  0 <= x < SPLC780D_LCM_WIDTH
	 *  0 <= y < SPLC780D_LCM_WIDTH
	 *  1 <= x + len < SPLC780D_LCM_WIDTH
	 */
	splc780d_MoveCursorPosition( x, y );
	
	while( len -- )
		splc780d_write_data( *pszText ++ );
}

void splc780d_MoveCursorPosition( unsigned char x, unsigned char y )
{
	if( y >= SPLC780D_LCM_HEIGHT || x >= SPLC780D_LCM_WIDTH )
		return;
	
	/* set AC address */
	if( y == 0 ) {
		splc780d_write_instruction( LCM_CMD_SET_DDRAM_ADDRESS + x );
	} else {
		splc780d_write_instruction( LCM_CMD_SET_DDRAM_ADDRESS + 0x40 + x );
	}
}

void splc780d_DisplayOnOff( unsigned char display, 
							unsigned char cursor,
							unsigned char blink )
{
	unsigned char ins;
	
	ins = 	LCM_CMD_DISPLAY_CONTROL |
			( display ? LCM_CMD_DISPLAY_CONTROL_DISPLAY : 0 ) |
			( cursor ? LCM_CMD_DISPLAY_CONTROL_CURSOR : 0 ) |
			( blink ? LCM_CMD_DISPLAY_CONTROL_BLINK : 0 );
	
	splc780d_write_instruction( ins );
}

void splc780d_InitLCM( void )
{
	splc780d_init_lcm_gpio();
	
	splc780d_reset();
	
	// Display on 
	splc780d_DisplayOnOff( 1, 0, 0 );
	
	// Draw a prompt string 
	splc780d_DrawText( 0, 0, "Startup...", 10 );
}

