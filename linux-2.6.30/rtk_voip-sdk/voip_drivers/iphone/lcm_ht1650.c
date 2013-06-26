#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include "lcm_ht1650.h"
#include "../gpio/gpio.h"
#include <linux/delay.h>	//udelay(),mdelay()

// GPIO PIN definition 
#define HT_PIN_CS		GPIO_ID(GPIO_PORT_A,5)	//output
#define HT_PIN_WR		GPIO_ID(GPIO_PORT_B,3)	//output
#define HT_PIN_DB0		GPIO_ID(GPIO_PORT_B,4)	//output or input
#define HT_PIN_DB1		GPIO_ID(GPIO_PORT_B,5)	//output or input
#define HT_PIN_DB2		GPIO_ID(GPIO_PORT_B,6)	//output or input
#define HT_PIN_DB3		GPIO_ID(GPIO_PORT_B,7)	//output or input

#define HT_PIN_PROTECT

// HT1650 Command definition 
#define LCM_WRITE				0x5000
#define LCM_CMD_SYS_DIS			0x4000	// system disable 
#define LCM_CMD_SYS_EN			0x4010	// turn on system oscillator 
#define LCM_CMD_LCD_OFF			0x4020	// turn off LCD display 
#define LCM_CMD_LCD_ON			0x4030	// turn on LCD display 
#define LCM_CMD_LARGE_BIAS		0x4160	// large bias 
#define LCM_CMD_MIDDLE_BIAS		0x4170	// middle bias 
#define LCM_CMD_SMALL_BIAS		0x4180	// small bias 
#define LCM_CMD_BIAS_1_6 		0x41A0	// bias 1/6 
#define LCM_CMD_BIAS_1_5 		0x4190	// bias 1/5 
#define LCM_CMD_FRAME_170HZ		0x41C0	// 170Hz frame
#define LCM_CMD_FRAME_89HZ		0x41D0	// 89Hz frame
#define LCM_CMD_FRAME_64HZ		0x41E0	// 64Hz frame
#define LCM_CMD_SELECT_80x16	0x41F0	// select 80x16  

// HT1650 RAM Mapping 
#define LCM_ADDR_BOUNDARY		320		// 80 x 16 / 4 = 320 

extern int32 _rtl867x_initGpioPin( uint32 gpioId, enum GPIO_DIRECTION direction, 
                                           enum GPIO_INTERRUPT_TYPE interruptEnable );
extern int32 _rtl867x_setGpioDataBit( uint32 gpioId, uint32 data );

static inline void ht1650_udelay( unsigned long delay )
{
	// In our experimental
	// 2  ... fail 
	// 3  ... ok 
	// 4  ... 
	// 5  ... ok 
	// 20 ... ok
	
	if( delay < 3 )
		udelay( 3 );
	else
		udelay( delay );
}

static inline void ht1650_mdelay( unsigned long delay )
{
	mdelay( delay );
}

static inline void ht1650_nsdelay( unsigned long delay )
{
	ht1650_udelay( delay / 1000 );	// convert ns to us 
}

static inline void ht1650_write_raw_nibble( int DB3, int DB2, int DB1, int DB0 )
{
	_rtl867x_setGpioDataBit( HT_PIN_WR, 0 );	// pull low WR  
	
	_rtl867x_setGpioDataBit( HT_PIN_DB3, ( DB3 ? 1 : 0 ) );
	_rtl867x_setGpioDataBit( HT_PIN_DB2, ( DB2 ? 1 : 0 ) );
	_rtl867x_setGpioDataBit( HT_PIN_DB1, ( DB1 ? 1 : 0 ) );
	_rtl867x_setGpioDataBit( HT_PIN_DB0, ( DB0 ? 1 : 0 ) );

	ht1650_nsdelay( 120 );	// delay 120 ns (tsu) 
	_rtl867x_setGpioDataBit( HT_PIN_WR, 1 );	// pull high WR  
	ht1650_nsdelay( 120 );	// delay 120 ns (th) 	
}

static void ht1650_init_lcm_gpio( void )
{
	_rtl867x_initGpioPin(HT_PIN_CS, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl867x_initGpioPin(HT_PIN_WR, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl867x_initGpioPin(HT_PIN_DB0, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl867x_initGpioPin(HT_PIN_DB1, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl867x_initGpioPin(HT_PIN_DB2, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl867x_initGpioPin(HT_PIN_DB3, GPIO_DIR_OUT, GPIO_INT_DISABLE);	
}

static void ht1650_command_mode( unsigned short cmd )
{
	_rtl867x_setGpioDataBit( HT_PIN_CS, 0 );	// pull low CS 
	
	ht1650_nsdelay( 100 );	// delay 100 ns (tsu1)
	
	// write 1st byte 
	ht1650_write_raw_nibble( cmd & 0x8000, cmd & 0x4000, 
							 cmd & 0x2000, cmd & 0x1000 );
	
	// write 2nd byte 
	ht1650_write_raw_nibble( cmd & 0x0800, cmd & 0x0400,
							 cmd & 0x0200, cmd & 0x0100 );

	// write 3st byte 
	ht1650_write_raw_nibble( cmd & 0x0080, cmd & 0x0040, 
							 cmd & 0x0020, cmd & 0x0010 );

	// write 4st byte 
	ht1650_write_raw_nibble( cmd & 0x0008, cmd & 0x0004, 
							 cmd & 0x0002, cmd & 0x0001 );
							 
	// 
	_rtl867x_setGpioDataBit( HT_PIN_CS, 1 );	// pull high CS 	
}

static void ht1650_write_mode( unsigned short addr, 	// 0 ~ 319(13Fh), bit 0 ~ 8 
							   const unsigned char *nibble,
							   unsigned short len )		// length of nibble 
{
	int np = 0;
	
	// check addr and length 
	if( addr >= LCM_ADDR_BOUNDARY )
		return;		// out of boundary 
		
	if( addr + len > LCM_ADDR_BOUNDARY )
		len = LCM_ADDR_BOUNDARY - addr;
	
	// reset pin 
	_rtl867x_setGpioDataBit( HT_PIN_CS, 1 );	// pull high CS 
	
	ht1650_nsdelay( 250 );	// delay 250 ns (tcs)
	
	// start to write 
	_rtl867x_setGpioDataBit( HT_PIN_CS, 0 );	// pull low CS 
	
	ht1650_nsdelay( 100 );	// delay 100 ns (tsu1)
	
	// write command ID code, and A8 
	ht1650_write_raw_nibble( addr & 0x0100, LCM_WRITE & 0x4000,
							 LCM_WRITE & 0x2000, LCM_WRITE & 0x1000 );	
	
	// write A7~A4 
	ht1650_write_raw_nibble( addr & 0x0080, addr & 0x0040,
							 addr & 0x0020, addr & 0x0010 );

	// write A3~A0 
	ht1650_write_raw_nibble( addr & 0x0008, addr & 0x0004, 
							 addr & 0x0002, addr & 0x0001 );
	
	// write data 
	while( len ) {
		if( np == 0 ) {
			ht1650_write_raw_nibble( *nibble & 0x0008, *nibble & 0x0004, 
									 *nibble & 0x0002, *nibble & 0x0001 );
		} else {
			ht1650_write_raw_nibble( *nibble & 0x0080, *nibble & 0x0040, 
									 *nibble & 0x0020, *nibble & 0x0010 ); 
		}
	
		len --;
		
		if( np ) 
			nibble ++;
		
		np ^= 1;
	}
	
	// 
	_rtl867x_setGpioDataBit( HT_PIN_CS, 1 );	// pull high CS 	
}

void ht1650_ClearScreen( int color )
{
	int i;
	
	_rtl867x_setGpioDataBit( HT_PIN_CS, 0 );	// pull low CS 
	
	ht1650_nsdelay( 100 );	// delay 100 ns (tsu1)
	
	// write command ID code, and A8 
	ht1650_write_raw_nibble( 0, LCM_WRITE & 0x4000,
							 LCM_WRITE & 0x2000, LCM_WRITE & 0x1000 );	
	
	// write A7~A4 
	ht1650_write_raw_nibble( 0, 0, 0, 0 );

	// write A3~A0 
	ht1650_write_raw_nibble( 0, 0, 0, 0 );
	
	// write data 
	for( i = 0; i < LCM_ADDR_BOUNDARY; i ++ )
		ht1650_write_raw_nibble( color, color, color, color );
	
	// 
	_rtl867x_setGpioDataBit( HT_PIN_CS, 1 );	// pull high CS 		
}

void ht1650_WriteData( int start, const unsigned char *pdata, int len )
{
	unsigned long flags;
		
	save_flags(flags); cli();

#ifdef HT_PIN_PROTECT
	// share with ALC 
	_rtl867x_initGpioPin( HT_PIN_CS, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	// share with PT6961 
	_rtl867x_initGpioPin( HT_PIN_DB1, GPIO_DIR_OUT, GPIO_INT_DISABLE );
#endif

	ht1650_write_mode( start * 2, pdata, len * 2 );

#ifdef HT_PIN_PROTECT	
	_rtl867x_initGpioPin( HT_PIN_DB1, GPIO_DIR_IN, GPIO_INT_DISABLE );
#endif
	
	restore_flags(flags);
}

void ht1650_DisplayOnOff( int onOff )
{
	unsigned long flags;
		
	save_flags(flags); cli();

	if( onOff )
		ht1650_command_mode( LCM_CMD_LCD_ON );		// Turn on LCD 
	else
		ht1650_command_mode( LCM_CMD_LCD_OFF );		// Turn off LCD 
	
	restore_flags(flags);
}

void ht1650_LCM_init( void )
{
	static const unsigned char logo[ 160 ] = { 
		0x26, 0x00, 0x49, 0x00, 0x49, 0x00, 0x49, 0x00, 0x32, 0x00, // S
		0x04, 0x00, 0x04, 0x00, 0x3F, 0x00, 0x44, 0x00, 0x44, 0x00, // t 
		0x20, 0x00, 0x54, 0x00, 0x54, 0x00, 0x54, 0x00, 0x78, 0x00, // a
		0x00, 0x00, 0x7C, 0x00, 0x08, 0x00, 0x04, 0x00, 0x04, 0x00, // r
		0x04, 0x00, 0x04, 0x00, 0x3F, 0x00, 0x44, 0x00, 0x44, 0x00, // t 
		0x3C, 0x00, 0x40, 0x00, 0x40, 0x00, 0x20, 0x00, 0x7C, 0x00, // u
		0x7C, 0x00, 0x14, 0x00, 0x14, 0x00, 0x14, 0x00, 0x08, 0x00, // p
		0x00, 0x00, 0x60, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, // .
		0x00, 0x00, 0x60, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, // .
		0x00, 0x00, 0x60, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, // .
#if 1
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
#else
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
#endif
	};

	ht1650_init_lcm_gpio();
	
	// delay 15 ms 
	//ht1650_mdelay(15);
	
	ht1650_command_mode( LCM_CMD_SYS_EN );		// Turn on system oscillator 
	
	ht1650_nsdelay( 250 );	// delay 250ns (tcs)

	ht1650_command_mode( LCM_CMD_SELECT_80x16 );	// Select 80x16 mode 

	ht1650_nsdelay( 250 );	// delay 250ns (tcs)

#if 0
	//ht1650_command_mode( LCM_CMD_BIAS_1_5 );

	//ht1650_udelay( 250 );	// delay 250ns (tcs)

	ht1650_command_mode( LCM_CMD_MIDDLE_BIAS );

	ht1650_udelay( 250 );	// delay 250ns (tcs)

	ht1650_command_mode( LCM_CMD_FRAME_89HZ );

	ht1650_udelay( 250 );	// delay 250ns (tcs)
#endif
	
	ht1650_ClearScreen( 0 );					// Clear screen 

	ht1650_nsdelay( 250 );	// delay 250ns (tcs)
	
	ht1650_command_mode( LCM_CMD_LCD_ON );		// Turn on LCD 

	ht1650_nsdelay( 250 );	// delay 250ns (tcs)

#if 1
	ht1650_write_mode( 0, logo, 320 /* in unit of nibble */ );
#else	
	//ht1650_write_mode( 0, vram, 20 /* in unit of nibble */ );
	ht1650_write_mode( 20 * 5, vram, 20 /* in unit of nibble */ );
#endif

#if 0	
	while( 1 ) {
	ht1650_mdelay( 2000 );
	
	ht1650_ClearScreen( 1 );

	ht1650_mdelay( 2000 );
	
	ht1650_ClearScreen( 0 );
	}

#endif
	
	printk( "ht1650 init done!\n" );
}

