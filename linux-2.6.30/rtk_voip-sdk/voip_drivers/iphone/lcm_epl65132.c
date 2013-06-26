#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include "lcm_epl65132.h"
#include "../gpio/gpio.h"
#include <linux/delay.h>	//udelay(),mdelay()

#define SERIAL_MODE		// serial or parallel 
//#define INTEL_MPU			// intel 80-family or motorola 68-family 

#define EPL65132_LCM_MMAP	// support mmap 

// GPIO PIN definition 
#ifdef SERIAL_MODE
	#define EPL_PIN_RES		GPIO_ID(GPIO_PORT_C,5)	//output
	#define EPL_PIN_CS		GPIO_ID(GPIO_PORT_G,6)	//output
	#define EPL_PIN_A0		GPIO_ID(GPIO_PORT_G,7)	//output
	#define EPL_PIN_WR		GPIO_ID(GPIO_PORT_H,6)	//output
	#define EPL_PIN_SDO		GPIO_ID(GPIO_PORT_F,4)	//output
	#define EPL_PIN_SCK		GPIO_ID(GPIO_PORT_F,2)	//output
	#define EPL_PIN_SDI		GPIO_ID(GPIO_PORT_E,1)	//output	
#else
	#define EPL_PIN_RES		GPIO_ID(GPIO_PORT_A,6)	//output
	#define EPL_PIN_CS		GPIO_ID(GPIO_PORT_F,7)	//output
	#define EPL_PIN_A0		GPIO_ID(GPIO_PORT_H,0)	//output
  #ifdef INTEL_MPU
	#define EPL_PIN_WR		GPIO_ID(GPIO_PORT_H,2)	//output
	#define EPL_PIN_RD		GPIO_ID(GPIO_PORT_H,1)	//output
  #else
	#define EPL_PIN_RW		GPIO_ID(GPIO_PORT_H,2)	//output
	#define EPL_PIN_E		GPIO_ID(GPIO_PORT_H,1)	//output
  #endif
	#define EPL_PIN_DB0		GPIO_ID(GPIO_PORT_A,5)	//output
	#define EPL_PIN_DB1		GPIO_ID(GPIO_PORT_G,1)	//output
	#define EPL_PIN_DB2		GPIO_ID(GPIO_PORT_G,2)	//output
	#define EPL_PIN_DB3		GPIO_ID(GPIO_PORT_G,3)	//output
	#define EPL_PIN_DB4		GPIO_ID(GPIO_PORT_G,4)	//output
	#define EPL_PIN_DB5		GPIO_ID(GPIO_PORT_G,5)	//output
	#define EPL_PIN_DB6		GPIO_ID(GPIO_PORT_G,6)	//output
	#define EPL_PIN_DB7		GPIO_ID(GPIO_PORT_G,7)	//output
#endif

// EPL65132 Command definition 
#define LCM_CMD_DISPLAY_ON			0xAF	// Display ON  
#define LCM_CMD_DISPLAY_OFF			0xAE	// Display OFF  
#define LCM_CMD_SET_PAGE_ADDR		0xB0	// Page address set 
#define LCM_CMD_SET_PAGE_ADDR_MASK	0x0F
#define LCM_CMD_SET_COL_ADDR_HI		0x10	// Column address set up (upper bit)
#define LCM_CMD_SET_COL_ADDR_HI_MASK	0x0F
#define LCM_CMD_SET_COL_ADDR_LO		0x00	// Column address set up (lower bit)
#define LCM_CMD_SET_COL_ADDR_LO_MASK	0x0F
#define LCM_CMD_INV_DISPLAY_ON		0xA7
#define LCM_CMD_INV_DISPLAY_OFF		0xA6
#define LCM_CMD_SET_STATIC_IND_ON	0xAD
#define LCM_CMD_SET_STATIC_IND_OFF	0xAC
#define LCM_CMD_POWER_CONTROL		0x28
#define LCM_CMD_POWER_CONTROL_MASK	0x07
#define LCM_CMD_REGULATOR_SEL		0x20
#define LCM_CMD_REGULATOR_SEL_MASK	0x07
#define LCM_CMD_SHL_SELECT_NORMAL	0xC0	// COM0 -> COM63 (vertical)
#define LCM_CMD_SHL_SELECT_REVERSE	0xC8	// COM63 -> COM0 (vertical)
#define LCM_CMD_ADC_SELECT_NORMAL	0xA0	// SEG0 -> SEG131 (horizontal)
#define LCM_CMD_ADC_SELECT_REVERSE	0xA1	// SEG131 -> SEG0 (horizontal)

// EPL65132 application definition 
//#define LCM_PAGE_NUM	4		// 32 / 8 = 4 (But EPL65132 can handle 65 bits)
#define LCM_PAGE_NUM	9		// upper( 65 / 8 ) = 9 (the last page occupies D0 only)  
#define LCM_COLS_NUM	132

// internal functions 
static inline void epl65132_udelay( unsigned long delay )
{
	// In our experimental
	// 1  ...ok
	// 2  ...ok 
	// 3  ...ok  
	// 4  ... 
	// 5  ...ok  
	// 10 ...ok
	// 20 ... 

#ifdef SERIAL_MODE
	if( delay < 2 )
		udelay( 2 );
#else	
	if( delay < 10 )
		udelay( 10 );
#endif
	else
		udelay( delay );
}

static inline void epl65132_mdelay( unsigned long delay )
{
	mdelay( delay );
}

static inline void epl65132_nsdelay( unsigned long delay )
{
	epl65132_udelay( delay / 1000 );	// convert ns to us 
}

static void epl65132_init_lcm_gpio( void )
{
#ifdef SERIAL_MODE
	_rtl865xC_initGpioPin(EPL_PIN_RES, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl865xC_initGpioPin(EPL_PIN_CS, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl865xC_initGpioPin(EPL_PIN_A0, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl865xC_initGpioPin(EPL_PIN_WR, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl865xC_initGpioPin(EPL_PIN_SDO, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);
	_rtl865xC_initGpioPin(EPL_PIN_SCK, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl865xC_initGpioPin(EPL_PIN_SDI, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
#else
	_rtl865xC_initGpioPin(EPL_PIN_RES, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl865xC_initGpioPin(EPL_PIN_CS, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl865xC_initGpioPin(EPL_PIN_A0, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
  #ifdef INTEL_MPU
	_rtl865xC_initGpioPin(EPL_PIN_WR, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl865xC_initGpioPin(EPL_PIN_RD, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
  #else
	_rtl865xC_initGpioPin(EPL_PIN_RW, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl865xC_initGpioPin(EPL_PIN_E, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
  #endif
	_rtl865xC_initGpioPin(EPL_PIN_DB0, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl865xC_initGpioPin(EPL_PIN_DB1, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl865xC_initGpioPin(EPL_PIN_DB2, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);	
	_rtl865xC_initGpioPin(EPL_PIN_DB3, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);	
	_rtl865xC_initGpioPin(EPL_PIN_DB4, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);	
	_rtl865xC_initGpioPin(EPL_PIN_DB5, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);	
	_rtl865xC_initGpioPin(EPL_PIN_DB6, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);	
	_rtl865xC_initGpioPin(EPL_PIN_DB7, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);	
#endif
}

static inline void epl65132_reset( void )
{
	_rtl865xC_setGpioDataBit( EPL_PIN_RES, 0 );
	epl65132_mdelay( 20 );	// p43 says "wait for more than 20ms." 
	_rtl865xC_setGpioDataBit( EPL_PIN_RES, 1 );
	epl65132_mdelay( 20 );
}

static inline void epl65132_init_pin_state( void )
{
#ifdef SERIAL_MODE
	_rtl865xC_setGpioDataBit( EPL_PIN_CS, 1 );	// not CS 
	_rtl865xC_setGpioDataBit( EPL_PIN_SCK, 1 );
#else
  #ifdef INTEL_MPU
	_rtl865xC_setGpioDataBit( EPL_PIN_WR, 1 );	// pull WR high  
	_rtl865xC_setGpioDataBit( EPL_PIN_RD, 1 );	// pull RD high  
  #else
	_rtl865xC_setGpioDataBit( EPL_PIN_E, 1 );	// pull enable high  
  #endif
	_rtl865xC_setGpioDataBit( EPL_PIN_CS, 1 );	// not CS 
#endif
}

static inline void epl65132_write_DB0_to_DB7( unsigned char DB )
{
#ifdef SERIAL_MODE
	int i;
	unsigned char mask = 0x80;
  #if 0
	uint32 data;
  #endif
	
	for( i = 0; i < 8; i ++ ) { 
		
		_rtl865xC_setGpioDataBit( EPL_PIN_SCK, 0 );	// pull SCK low 
		
		_rtl865xC_setGpioDataBit( EPL_PIN_SDI, ( ( DB & mask ) ? 1 : 0 ) );
		
		epl65132_nsdelay( 100 );	// tCLLS = 100ns 

  #if 0		
		_rtl865xC_getGpioDataBit( EPL_PIN_SDO, &data );
		printk( "%d", data );
  #endif
		
		_rtl865xC_setGpioDataBit( EPL_PIN_SCK, 1 );	// pull SCK high 		
		
		epl65132_nsdelay( 100 );	// tCLHS = 100ns 
		
		mask >>= 1;
	}

  #if 0	
	printk( "\n" );
  #endif
#else
	_rtl865xC_setGpioDataBit( EPL_PIN_DB0, ( ( DB & 0x01 ) ? 1 : 0 ) );
	_rtl865xC_setGpioDataBit( EPL_PIN_DB1, ( ( DB & 0x02 ) ? 1 : 0 ) );
	_rtl865xC_setGpioDataBit( EPL_PIN_DB2, ( ( DB & 0x04 ) ? 1 : 0 ) );
	_rtl865xC_setGpioDataBit( EPL_PIN_DB3, ( ( DB & 0x08 ) ? 1 : 0 ) );
	_rtl865xC_setGpioDataBit( EPL_PIN_DB4, ( ( DB & 0x10 ) ? 1 : 0 ) );
	_rtl865xC_setGpioDataBit( EPL_PIN_DB5, ( ( DB & 0x20 ) ? 1 : 0 ) );
	_rtl865xC_setGpioDataBit( EPL_PIN_DB6, ( ( DB & 0x40 ) ? 1 : 0 ) );
	_rtl865xC_setGpioDataBit( EPL_PIN_DB7, ( ( DB & 0x80 ) ? 1 : 0 ) );
#endif
}

static void epl65132_write_data_core( int DB_type, unsigned char DB )
{
#ifdef SERIAL_MODE
	_rtl865xC_setGpioDataBit( EPL_PIN_CS, 0 );	// active CS 
	
	epl65132_nsdelay( 100 );	// tCSS = 100ns 
	
	if( DB_type )
		_rtl865xC_setGpioDataBit( EPL_PIN_A0, 1 );	// display data 
	else
		_rtl865xC_setGpioDataBit( EPL_PIN_A0, 0 );	// control data 
	
	_rtl865xC_setGpioDataBit( EPL_PIN_WR, 0 );	// write mode ?? 
	
	epl65132_nsdelay( 100 );	// tASS = 100ns 
	
	epl65132_write_DB0_to_DB7( DB );	// reading within this function 
	
	epl65132_nsdelay( 100 );	// tAHS = 100ns 
	
	_rtl865xC_setGpioDataBit( EPL_PIN_CS, 1 );	// inactive CS 
#else
  #ifdef INTEL_MPU
	if( DB_type )
		_rtl865xC_setGpioDataBit( EPL_PIN_A0, 1 );	// display data 
	else
		_rtl865xC_setGpioDataBit( EPL_PIN_A0, 0 );	// control data 
	
	// delay tAW8 = 0ns 
	
	_rtl865xC_setGpioDataBit( EPL_PIN_CS, 0 );	// active CS 
	
	_rtl865xC_setGpioDataBit( EPL_PIN_WR, 0 );	// pull WR low 
	
	epl65132_write_DB0_to_DB7( DB );					// write DB0 ~ DB7 
	
	epl65132_nsdelay( 20 );	// tDS8 = 20ns 
	
	_rtl865xC_setGpioDataBit( EPL_PIN_WR, 1 );	// pull WR high 
	
	epl65132_nsdelay( 10 );	// tDH8 = 20ns 
	
	_rtl865xC_setGpioDataBit( EPL_PIN_CS, 1 );	// inactive CS 
  #else
	_rtl865xC_setGpioDataBit( EPL_PIN_E, 0 );	// pull enable low 

	if( DB_type )
		_rtl865xC_setGpioDataBit( EPL_PIN_A0, 1 );	// display data 
	else
		_rtl865xC_setGpioDataBit( EPL_PIN_A0, 0 );	// control data 

	_rtl865xC_setGpioDataBit( EPL_PIN_RW, 0 );		// write mode  

	_rtl865xC_setGpioDataBit( EPL_PIN_CS, 0 );	// active CS 
	
	// delay tAW6 = 0ns 
	epl65132_nsdelay( 0 );
	
	_rtl865xC_setGpioDataBit( EPL_PIN_E, 1 );	// pull enable high 
	
	epl65132_write_DB0_to_DB7( DB );					// write DB0 ~ DB7 

	epl65132_nsdelay( 20 );	// tDS6 = 20ns 
	
	_rtl865xC_setGpioDataBit( EPL_PIN_E, 0 );	// pull enable low 
	
	//epl65132_nsdelay( 10 );	// tDH6 = 10ns 
	epl65132_nsdelay( 1000 );	// tDH6 = 10ns 

	_rtl865xC_setGpioDataBit( EPL_PIN_E, 1 );	// pull enable high 

	_rtl865xC_setGpioDataBit( EPL_PIN_CS, 1 );	// inactive CS 
	
	epl65132_nsdelay( 10 );
  #endif // INTEL_MPU
#endif // SERIAL_MODE
}

static void epl65132_read_data_core( void )
{
#ifdef SERIAL_MODE
	_rtl865xC_setGpioDataBit( EPL_PIN_CS, 0 );	// active CS 
	
	epl65132_nsdelay( 100 );	// tCSS = 100ns 

	_rtl865xC_setGpioDataBit( EPL_PIN_A0, 1 );	// control data 
	
	_rtl865xC_setGpioDataBit( EPL_PIN_WR, 1 );	// write mode ?? 
	
	epl65132_nsdelay( 100 );	// tASS = 100ns 
	
	epl65132_write_DB0_to_DB7( 0x00 );
	
	epl65132_nsdelay( 100 );	// tAHS = 100ns 
	
	_rtl865xC_setGpioDataBit( EPL_PIN_CS, 1 );	// inactive CS 
#else
	uint32 data;
	
	epl65132_write_DB0_to_DB7( 0x9F );	// make all bits to be one 
	
	_rtl865xC_initGpioPin(EPL_PIN_DB0, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);
	_rtl865xC_initGpioPin(EPL_PIN_DB1, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);
	_rtl865xC_initGpioPin(EPL_PIN_DB2, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);	
	_rtl865xC_initGpioPin(EPL_PIN_DB3, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);	
	_rtl865xC_initGpioPin(EPL_PIN_DB4, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);	
	_rtl865xC_initGpioPin(EPL_PIN_DB5, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);	
	_rtl865xC_initGpioPin(EPL_PIN_DB6, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);	
	_rtl865xC_initGpioPin(EPL_PIN_DB7, GPIO_PERI_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE);	

  #ifdef INTEL_MPU
	_rtl865xC_setGpioDataBit( EPL_PIN_A0, 0 );	// control data 
	
	// delay tAW8 = 0ns 
	
	_rtl865xC_setGpioDataBit( EPL_PIN_CS, 0 );	// active CS 
	
	_rtl865xC_setGpioDataBit( EPL_PIN_RD, 0 );	// pull WR low 
	
	// read DB0 ~ DB7 
	_rtl865xC_getGpioDataBit( EPL_PIN_DB7, &data );	printk( "%d", data );
	_rtl865xC_getGpioDataBit( EPL_PIN_DB6, &data );	printk( "%d", data );
	_rtl865xC_getGpioDataBit( EPL_PIN_DB5, &data );	printk( "%d", data );
	_rtl865xC_getGpioDataBit( EPL_PIN_DB4, &data );	printk( "%d", data );
	_rtl865xC_getGpioDataBit( EPL_PIN_DB3, &data );	printk( "%d", data );
	_rtl865xC_getGpioDataBit( EPL_PIN_DB2, &data );	printk( "%d", data );
	_rtl865xC_getGpioDataBit( EPL_PIN_DB1, &data );	printk( "%d", data );
	_rtl865xC_getGpioDataBit( EPL_PIN_DB0, &data );	printk( "%d", data );
	printk( "\n" );
	
	epl65132_nsdelay( 20 );	// tDS8 = 20ns 
	
	_rtl865xC_setGpioDataBit( EPL_PIN_RD, 1 );	// pull WR high 
	
	epl65132_nsdelay( 10 );	// tDH8 = 20ns 
	
	_rtl865xC_setGpioDataBit( EPL_PIN_CS, 1 );	// inactive CS 
  #else
	_rtl865xC_setGpioDataBit( EPL_PIN_E, 0 );	// pull enable low 

	_rtl865xC_setGpioDataBit( EPL_PIN_A0, 0 );	// control data 

	_rtl865xC_setGpioDataBit( EPL_PIN_RW, 1 );		// read mode  

	_rtl865xC_setGpioDataBit( EPL_PIN_CS, 0 );	// active CS 
	
	// delay tAW6 = 0ns 
	epl65132_nsdelay( 0 );
	
	_rtl865xC_setGpioDataBit( EPL_PIN_E, 1 );	// pull enable high 
	
	// read DB0 ~ DB7 
	_rtl865xC_getGpioDataBit( EPL_PIN_DB7, &data );	printk( "%d", data );
	_rtl865xC_getGpioDataBit( EPL_PIN_DB6, &data );	printk( "%d", data );
	_rtl865xC_getGpioDataBit( EPL_PIN_DB5, &data );	printk( "%d", data );
	_rtl865xC_getGpioDataBit( EPL_PIN_DB4, &data );	printk( "%d", data );
	_rtl865xC_getGpioDataBit( EPL_PIN_DB3, &data );	printk( "%d", data );
	_rtl865xC_getGpioDataBit( EPL_PIN_DB2, &data );	printk( "%d", data );
	_rtl865xC_getGpioDataBit( EPL_PIN_DB1, &data );	printk( "%d", data );
	_rtl865xC_getGpioDataBit( EPL_PIN_DB0, &data );	printk( "%d", data );
	printk( "\n" );

	epl65132_nsdelay( 20 );	// tDS6 = 20ns 
	
	_rtl865xC_setGpioDataBit( EPL_PIN_E, 0 );	// pull enable low 
	
	epl65132_nsdelay( 10 );	// tDH6 = 10ns 

	_rtl865xC_setGpioDataBit( EPL_PIN_CS, 1 );	// inactive CS 

	_rtl865xC_setGpioDataBit( EPL_PIN_E, 1 );	// pull enable high 
	
	epl65132_nsdelay( 10 );
  #endif // INTEL_MPU

	_rtl865xC_initGpioPin(EPL_PIN_DB0, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl865xC_initGpioPin(EPL_PIN_DB1, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl865xC_initGpioPin(EPL_PIN_DB2, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);	
	_rtl865xC_initGpioPin(EPL_PIN_DB3, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);	
	_rtl865xC_initGpioPin(EPL_PIN_DB4, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);	
	_rtl865xC_initGpioPin(EPL_PIN_DB5, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);	
	_rtl865xC_initGpioPin(EPL_PIN_DB6, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);	
	_rtl865xC_initGpioPin(EPL_PIN_DB7, GPIO_PERI_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE);	
#endif // SERIAL_MODE
}

static inline void epl65132_write_control_data( unsigned char cmd )
{
	epl65132_write_data_core( 0 /* control data */, cmd );
}

static inline void epl65132_write_display_data( unsigned char data )
{
	// When writing a display data, col_addr is increasing automatically 
	// and stops with 83h. 
	
	epl65132_write_data_core( 1 /* display data */, data );
}

static void epl65132_set_address( unsigned char page_addr, unsigned char col_addr )
{
	// page_addr = 0 ~ 8 
	// col_addr = 0 ~ 131 (83h)
	unsigned char cmd;
	
	// page address 
	cmd = LCM_CMD_SET_PAGE_ADDR | ( page_addr & LCM_CMD_SET_PAGE_ADDR_MASK );
	
	epl65132_write_control_data( cmd );	// set page address 
	
	// column address (hi)
	cmd = LCM_CMD_SET_COL_ADDR_HI | ( ( col_addr >> 4 ) & LCM_CMD_SET_COL_ADDR_HI_MASK );
	
	epl65132_write_control_data( cmd );

	// column address (lo)
	cmd = LCM_CMD_SET_COL_ADDR_LO | ( col_addr & LCM_CMD_SET_COL_ADDR_LO_MASK );
	
	epl65132_write_control_data( cmd );
}

// API 
void epl65132_ClearScreen( int color )
{
	int i, j;
	const unsigned char data = ( color ? 0xFF : 0x00 );
	//const unsigned char data = ( color ? 0x5D : 0x00 );
	
	for( i = 0; i < LCM_PAGE_NUM; i ++ ) {
		// set address 
		epl65132_set_address( i, 0 );
		
		for( j = 0; j < LCM_COLS_NUM; j ++ )
			epl65132_write_display_data( data );
	}
}

void epl65132_WriteData( unsigned char page, unsigned char col, const unsigned char *pdata, int len )
{
	epl65132_set_address( page, col );
	
	while( len -- ) {
		epl65132_write_display_data( *pdata ++ );
		
		// wrap-around writing 
		if( len > 0 && ++ col >= LCM_COLS_NUM ) {
			col = 0;
			if( ++page >= LCM_PAGE_NUM )
				page = 0;
				
			epl65132_set_address( page, col );
		}
	}
}

#ifdef EPL65132_LCM_MMAP
#include "lcm_mmap.h"

void epl65132_DirtyMmap( unsigned char page, unsigned char col, unsigned char len, unsigned char rows )
{
	unsigned char i;
	unsigned char *plcm_mmap;
	
	if( col >= LCM_COLS_NUM )
		return;
	
	if( col + len >= LCM_COLS_NUM )
		len = LCM_COLS_NUM - col;
	
	plcm_mmap = &lcm_mmap[ page * LCM_COLS_NUM + col ];
	
	for( i = 0; i < rows; i ++ ) {
		
		if( page >= LCM_PAGE_NUM )
			break;
	
		epl65132_WriteData( page, col, plcm_mmap, len );
		
		page ++;
		plcm_mmap += LCM_COLS_NUM;
	}
}
#endif /* EPL65132_LCM_MMAP */

void epl65132_DisplayOnOff( int onOff )
{
	if( onOff )
		epl65132_write_control_data( LCM_CMD_DISPLAY_ON );
	else
		epl65132_write_control_data( LCM_CMD_DISPLAY_OFF );
}

void epl65132_LCM_init( void )
{
	// init GPIO 
	epl65132_init_lcm_gpio();
	
	// reset 
	epl65132_reset();
	
	// init PIN state 
	epl65132_init_pin_state();

#if 1
	// clean screen 
	epl65132_ClearScreen( 0 );
#endif

	// regulator resistor select  
	epl65132_write_control_data( LCM_CMD_REGULATOR_SEL |
								 ( 6 & LCM_CMD_REGULATOR_SEL_MASK ) );

	// power control 
	epl65132_write_control_data( LCM_CMD_POWER_CONTROL | 
								 ( 7 & LCM_CMD_POWER_CONTROL_MASK ) );
	// 2b, 2e, 2f 
	
	// SHL select reverse: COM63 -> COM0 (vertical)
	//epl65132_write_control_data( LCM_CMD_SHL_SELECT_REVERSE );
	
	// ADC select reverse: SEG131 -> SEG0 (horizontal)
	epl65132_write_control_data( LCM_CMD_ADC_SELECT_REVERSE );
	
	// display on 
	epl65132_DisplayOnOff( 1 );

#if 0
	while( 1 ) {
		epl65132_DisplayOnOff( 1 );
		epl65132_read_data_core();
		printk( "on\n" );
		epl65132_mdelay( 1000 );
		
		epl65132_DisplayOnOff( 0 );
		epl65132_read_data_core();
		printk( "off\n" );
		epl65132_mdelay( 1000 );
	}
#endif

#if 0
	while( 1 ) {
		epl65132_ClearScreen( 1 );
		epl65132_read_data_core();
		printk( "black\n" );
		epl65132_mdelay( 1000 );
		
		epl65132_ClearScreen( 0 );
		epl65132_read_data_core();
		printk( "white\n" );
		epl65132_mdelay( 1000 );
	}
#endif
	
#if 0	
	epl65132_ClearScreen( 1 );
#endif

#if 0
	{
		int col;
		const unsigned char data[] = {
		0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
		0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
		0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
		0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
		0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
		0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
		0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
		0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
		0x01, 0x02, 0x04, 0x08, };
		
		epl65132_ClearScreen( 0 );
		
		epl65132_WriteData( 0, 0, &data, 132 );
		epl65132_WriteData( 1, 0, &data, 132 );
		epl65132_WriteData( 2, 0, &data, 132 );
		epl65132_WriteData( 3, 0, &data, 132 );
		epl65132_WriteData( 4, 0, &data, 132 );
		epl65132_WriteData( 5, 0, &data, 132 );
		epl65132_WriteData( 6, 0, &data, 132 );
		epl65132_WriteData( 7, 0, &data, 132 );
		epl65132_WriteData( 8, 0, &data, 132 );
		
		while( 1 );
	}
#endif

	printk( "epl65132_LCM_init done!\n" );
}

