#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/string.h>
#include "PT6961.h"
#include "../gpio/gpio.h"
#include <linux/delay.h>	//udelay(),mdelay()
#include "keypad_map.h"
#include "voip_timer.h"

// GPIO PIN definition 
#define PT6961_PIN_STB		GPIO_ID(GPIO_PORT_A,6)	//output
#define PT6961_PIN_CLK		GPIO_ID(GPIO_PORT_B,3)	//output
#define PT6961_PIN_DIN		GPIO_ID(GPIO_PORT_B,4)	//output 
#define PT6961_PIN_DOUT		GPIO_ID(GPIO_PORT_B,5)	//input

// PT6961 Command definition 
// arguments with '*' symbol are default. 
#define DMOD_CMD			0x00	// bit 7-6 = 00
#define DMOD_ARG_6x12		0x02	// bit 1-0 = 10
#define DMOD_ARG_7x11		0x03	// bit 1-0 = 11*
#define DATA_CMD			0x40	// bit 7-6 = 01
#define DATA_ARG1_INC_ADDR	0X00	// bit 2 = 0*
#define DATA_ARG1_FIX_ADDR	0X04	// bit 2 = 1
#define DATA_ARG2_WRITE		0X00	// bit 1-0 = 00*
#define DATA_ARG2_READ		0X02	// bit 1-0 = 10
#define ADDR_CMD			0xC0	// bit 7-6 = 11
#define ADDR_ARG_ADDR_MASK	0x0F	// bit 3-0: range 0 ~ 13 
#define DCTL_CMD			0x80	// bit 7-6 = 10
#define DCTL_ARG1_OFF		0x00	// bit 3 = 0*
#define DCTL_ARG1_ON		0x08	// bit 3 = 1
#define DCTL_ARG2_1_16		0x00	// bit 2-0 = 000*
#define DCTL_ARG2_2_16		0x01	// bit 2-0 = 001
#define DCTL_ARG2_4_16		0x02	// bit 2-0 = 010
#define DCTL_ARG2_10_16		0x03	// bit 2-0 = 011
#define DCTL_ARG2_11_16		0x04	// bit 2-0 = 100
#define DCTL_ARG2_12_16		0x05	// bit 2-0 = 101
#define DCTL_ARG2_13_16		0x06	// bit 2-0 = 110
#define DCTL_ARG2_14_16		0x07	// bit 2-0 = 111

// limit 
#define RAM_ADDR_BOUNDARY	14
#define KEY_DATA_NUM		5		// 5 bytes 

// configuration 
#undef _PT6961_CTL_HOOK			// use PT6961 to control HOOK 
#ifdef CONFIG_RTK_VOIP_GPIO_IPP_8972_V01
#define _SUPPORT_DOUBLE_KEY		// support double key 
#endif

static keypad_dev_t pt6961_keypad_dev_data = { 0, 0 };
#ifdef _PT6961_CTL_HOOK
static int pt6961_hook_status = 0;	// 0->on-hook, 1->off-hook
#endif

static inline void pt6961_udelay( unsigned long delay )
{
	// In our experimental
	// 3 ... fail 
	// 4 ... ok 
	if( delay < 10 )
		udelay( 10 );
	else
		udelay( delay );
}

static inline void pt6961_mdelay( unsigned long delay )
{
	mdelay( delay );
}

static inline void pt6961_nsdelay( unsigned long delay )
{
	pt6961_udelay( delay / 1000 );	// convert ns to us 
}

static void pt6961_init_gpio( void )
{
	_rtl867x_initGpioPin(PT6961_PIN_STB, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl867x_initGpioPin(PT6961_PIN_CLK, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl867x_initGpioPin(PT6961_PIN_DIN, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl867x_initGpioPin(PT6961_PIN_DOUT, GPIO_DIR_IN, GPIO_INT_DISABLE);
}

static void pt6961_enable_STB( void )
{
	_rtl867x_setGpioDataBit( PT6961_PIN_STB, 1 );	// pull high STB 
	
	pt6961_udelay( 1 );	// delay 1us (PWstb)

	_rtl867x_setGpioDataBit( PT6961_PIN_STB, 0 );	// pull low STB 	
}

static void pt6961_disable_STB( void )
{
	_rtl867x_setGpioDataBit( PT6961_PIN_STB, 1 );	// pull high STB 
}

static void pt6961_write_a_bit( int bit )
{
	_rtl867x_setGpioDataBit( PT6961_PIN_CLK, 0 );	// pull low clk 
	
	_rtl867x_setGpioDataBit( PT6961_PIN_DIN, ( bit ? 1 : 0 ) );
	
	pt6961_nsdelay( 100 );	// delay 100ns (tsetup)

	_rtl867x_setGpioDataBit( PT6961_PIN_CLK, 1 );	// pull high clk 
	
	pt6961_nsdelay( 100 );	// delay 100ns (thold)
}

static inline void pt6961_write_a_byte( unsigned byte )
{
	pt6961_write_a_bit( byte & 0x01 );
	pt6961_write_a_bit( byte & 0x02 );
	pt6961_write_a_bit( byte & 0x04 );
	pt6961_write_a_bit( byte & 0x08 );
	pt6961_write_a_bit( byte & 0x10 );
	pt6961_write_a_bit( byte & 0x20 );
	pt6961_write_a_bit( byte & 0x40 );
	pt6961_write_a_bit( byte & 0x80 );	
}

static void pt6961_display_mode_command( void )
{
	pt6961_enable_STB();
	
	pt6961_write_a_byte( DMOD_CMD | DMOD_ARG_7x11 );
	
	pt6961_disable_STB();
}

static void pt6961_write_data_command( const unsigned char *pData )
{
	int i;
	
	/* ************************************************************** */
	// Data setting command: set writing mode 
	/* ************************************************************** */
	pt6961_enable_STB();
	
	// write command 
	pt6961_write_a_byte( DATA_CMD | DATA_ARG1_INC_ADDR | DATA_ARG2_WRITE );
		
	pt6961_udelay( 1 );	// delay 1us (tclk-stb)

	/* ************************************************************** */
	// Address setting command: write data 
	/* ************************************************************** */
	pt6961_enable_STB();
	
	// write command 
	pt6961_write_a_byte( ADDR_CMD | 0 );	// addr start at 0 
	
	// write data
	for( i = 0; i < RAM_ADDR_BOUNDARY; i ++ ) 
		pt6961_write_a_byte( *pData ++ );	
		
	pt6961_disable_STB();
}

static int pt6961_read_data_command( unsigned char *pData1, unsigned char *pData2 )
{
	int i;
	unsigned int data;
	int count = 0;

	pt6961_enable_STB();
	
	// read command 
	pt6961_write_a_byte( DATA_CMD | DATA_ARG2_READ );

	pt6961_udelay( 1 );	// delay 1us (twait)
	
	// read data 	
	for( i = 0; i < KEY_DATA_NUM * 8; i ++ ) {
		_rtl867x_setGpioDataBit( PT6961_PIN_CLK, 0 );	// pull low clk 
		
		pt6961_nsdelay( 100 );	// delay 100ns (tpzl)
		
		_rtl867x_getGpioDataBit( PT6961_PIN_DOUT, &data );
		
		_rtl867x_setGpioDataBit( PT6961_PIN_CLK, 1 );	// pull high clk 

#ifdef _PT6961_CTL_HOOK
		if( i == KEY_HOOK ) {
			// on-hook, data = 1  
			// off-hook, data = 0
			pt6961_hook_status = ( data ? 0 : 1 );
		}
#endif
		
		if( data == 0 ) {
			pt6961_nsdelay( 400 );	// delay 400ns (PWclk)
			continue;
		}

#if 0
		// ignore some key 
		if( ( i & 007 ) == 002 || ( i & 007 ) == 005 ) {
		} else
#endif
		{
			// normal keys 
			count ++;
			
			switch( count ) {
			case 1:
				*pData1 = ( unsigned char )i;
				break;
				
#ifdef _SUPPORT_DOUBLE_KEY
			case 2:
				*pData2 = ( unsigned char )i;
				break;
#endif
				
			default:	// count >= 2 
				goto label_press_duplicate_keys;
				break;
			}
		}

#if 0		
		if( data )
			printk( "%02o ", i );
#endif
	}

#if 0
	printk( "\n" );
#endif
	
label_press_duplicate_keys:

	pt6961_disable_STB();
	
	return count;
}

static void pt6961_control_command( int bOnOff )
{
	short ctrl_cmd;
	
	if( bOnOff )
		ctrl_cmd = DCTL_CMD | DCTL_ARG1_ON | DCTL_ARG2_14_16;
	else
		ctrl_cmd = DCTL_CMD | DCTL_ARG1_OFF | DCTL_ARG2_1_16;
	
	pt6961_enable_STB();
	
	// write command 
	pt6961_write_a_byte( ctrl_cmd );
	
	pt6961_disable_STB();
}

///////////////////////////////////////////////////////////////////////// 
// keyscan  
static void pt6961_do_keyscan( unsigned long arg )
{	
	extern void keypad_polling_signal_target( keypad_dev_t *keypad_data_pool );
	extern void keypad_polling_scan_done( void );

	unsigned char key1, key2;
	unsigned char key;
	
	switch( pt6961_read_data_command( &key1, &key2 ) ) {
	case 1:
		key = key1;
		break;

#ifdef _SUPPORT_DOUBLE_KEY		
	case 2:
		// check if double key, possible pairs are 
		// (00, 02) (01, 02) (03, 05) (04, 05)
		// (10, 12) (11, 12) (13, 15) (14, 15)
		// (20, 22) (21, 22) (23, 25) (24, 25)
		// (30, 32) (31, 32) (33, 35) (34, 35)
		// (40, 42) (41, 42) (43, 45) (44, 45)
		// NOTE: octal notation 
		if( ( key2 & 07 ) == 02 || ( key2 & 07 ) == 05 )	// check key2 
			;
		else
			goto label_donot_process_duplicate_press;
		
		if( key2 - 2 <= key1 && key1 < key2 )		// check key1 range 
			;
		else
			goto label_donot_process_duplicate_press;
		
		//key = ( key1 << 6 ) | ( key2 );
		key = key1 | 0100;
		
		break;
#endif // _SUPPORT_DOUBLE_KEY
		
	default:
		goto label_donot_process_duplicate_press;
		break;
	}
	
	// process if only press one key 
	if( pt6961_keypad_dev_data.flags == 0 ) {
		pt6961_keypad_dev_data.flags = 1;
		pt6961_keypad_dev_data.data_string = key;
		keypad_polling_signal_target( &pt6961_keypad_dev_data );
	}

label_donot_process_duplicate_press:
	
	// call this function in period of 10ms 
	keypad_polling_scan_done();	

}

static void pt6961_init_keyscan_timer( void )
{
	register_timer_10ms( ( fn_timer_t )pt6961_do_keyscan, NULL );
}

///////////////////////////////////////////////////////////////////////// 
// extern API   
void pt6961_ClearDisplay( int bClear )
{
	unsigned char data[ RAM_ADDR_BOUNDARY ];
	
	if( bClear == 0 )
		memset( data, 0, RAM_ADDR_BOUNDARY );
	else
		memset( data, 0xFF, RAM_ADDR_BOUNDARY );
	
	pt6961_write_data_command( data );
}

void pt6961_SetDisplay( unsigned long led )
{
	unsigned char data[ RAM_ADDR_BOUNDARY ];
	unsigned long flags;
	unsigned long bitLed;
	unsigned char bitData;
	int i, j;
	
	// clear buffer 
	memset( data, 0, RAM_ADDR_BOUNDARY );
	
	// convert 32bits data to 14 bytes array 
	for( i = 0, bitLed = 0x0001; i < 7; i ++ ) {	// GR1 ~ GR7 
		for( j = 0, bitData = 0x01; j < 4; j ++ ) {		// SG1 ~ SG4
			if( led & bitLed ) {
				data[ i * 2 ] |= bitData;
			}
			
			bitData <<= 1;
			bitLed <<= 1;
		}
	}	
	
	// ok. now turn on/off LED 
	save_flags(flags); cli();

	pt6961_write_data_command( data );

	restore_flags(flags);
}

// 0->on-hook, 1->off-hook
#ifdef _PT6961_CTL_HOOK
int pt6961_GetHookStatus( void )
{
	return pt6961_hook_status;	
}
#endif

void pt6961_Initialize( void )
{
	pt6961_init_gpio();

	// delay 200 ms 
	//pt6961_mdelay( 200 );

	pt6961_ClearDisplay( 0 );		// set writing mode and clear display 
	
	pt6961_udelay( 1 );	// delay 1us (tstb)
	
	pt6961_control_command( 0 );	// display off 
	
	pt6961_udelay( 1 );	// delay 1us (tstb)
	
	pt6961_display_mode_command();	// display mode = 7 x 11 
	
	pt6961_udelay( 1 );	// delay 1us (tstb)
	
	pt6961_control_command( 1 );	// display on  	

#if 1	// testing purpose 
	pt6961_udelay( 1 );	// delay 1us (tstb)
	
	pt6961_ClearDisplay( 1 );		// set writing mode and clear display 
#endif
	
	pt6961_init_keyscan_timer();	// start keyscan timer 
	
#if 0
	while( 1 ) {
		pt6961_mdelay( 100 );
		pt6961_read_data_command( &key );
		printk( "\n" );
	}
#endif

#if 0
	while( 1 ) {
		unsigned long flags;
		
		save_flags(flags); cli();
		pt6961_ClearDisplay( 0 );
		restore_flags(flags);
		
		pt6961_mdelay( 2000 );
		
		save_flags(flags); cli();
		pt6961_ClearDisplay( 1 );
		restore_flags(flags);
		
		pt6961_mdelay( 2000 );		
	}
#endif
		
	
	printk( "pt6961 init done\n" );
}

