#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include "lcm_rts26151.h"
#include "../gpio/gpio.h"
#include "voip_timer.h"
#include <linux/delay.h>	//udelay(),mdelay()

// GPIO PIN definition 
#define MS_PIN_CLK		GPIO_ID(GPIO_PORT_H,6)	//output
#define MS_PIN_DIN		GPIO_ID(GPIO_PORT_H,7)	//output
#define MS_PIN_LD		GPIO_ID(GPIO_PORT_G,2)	//output 
#define MS_PIN_POL		GPIO_ID(GPIO_PORT_G,3)	//output 

// LCD size configuration 
#define MS_LCD_WIDTH		128
#define MS_LCD_HEIGHT		64
#define MS_LCD_VRAM_SIZE	( ( MS_LCD_WIDTH / 8 ) * ( MS_LCD_HEIGHT / 8 ) )
#define MS_LCD_CASCADE		2	// We cascade 2 RTS26151 to implement a 128*64 controller, 
								// so input is 2*96 bits per LD. 

#define REFRESH_PERIOD		50	// 500ms 

static unsigned char lcd_buffer[ MS_LCD_RAM_SIZE ];
static unsigned char lcd_POL;

static inline void rts26151_udelay( unsigned long delay )
{
	udelay( delay );
}

static inline void rts26151_mdelay( unsigned long delay )
{
	mdelay( delay );
}

static inline void rts26151_nsdelay( unsigned long delay )
{
	delay /= 1000;
	
	if( delay < 10 )
		rts26151_udelay( 10 );
	else
		rts26151_udelay( delay );
}

static inline void rts26151_init_lcm_gpio( void )
{
	_rtl867x_initGpioPin(MS_PIN_CLK, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl867x_initGpioPin(MS_PIN_DIN, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl867x_initGpioPin(MS_PIN_LD, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl867x_initGpioPin(MS_PIN_POL, GPIO_DIR_OUT, GPIO_INT_DISABLE);
}

static inline void rts26151_write_CLK( unsigned char clk )
{
	_rtl867x_setGpioDataBit( MS_PIN_CLK, clk );
}

static inline void rts26151_write_LD( unsigned char ld )
{
	_rtl867x_setGpioDataBit( MS_PIN_LD, ld );
}

static inline void rts26151_write_POL( unsigned char pol )
{
	_rtl867x_setGpioDataBit( MS_PIN_POL, pol );
}

static void rts26151_write_DIN_with_LD( unsigned char din )
{
	_rtl867x_setGpioDataBit( MS_PIN_DIN, din );
	_rtl867x_setGpioDataBit( MS_PIN_CLK, 1 );		// pull CLK high 
	
	rts26151_nsdelay( 100 );		// TDS (100ns)
	
	rts26151_write_LD( 1 );							// pull LD high  
	_rtl867x_setGpioDataBit( MS_PIN_CLK, 0 );		// pull CLK low 
	
	rts26151_nsdelay( 100 );		// TDH (100ns)
}

static void rts26151_write_DIN( unsigned char din )
{
	_rtl867x_setGpioDataBit( MS_PIN_DIN, din );
	_rtl867x_setGpioDataBit( MS_PIN_CLK, 1 );		// pull CLK high 
	
	rts26151_nsdelay( 100 );		// TDS (100ns)
	
	_rtl867x_setGpioDataBit( MS_PIN_CLK, 0 );		// pull CLK low 
	
	rts26151_nsdelay( 100 );		// TDH (100ns)
}

static void rts26151_refresh_screen( void )
{
	int x, y, i;
	unsigned char bit;
	unsigned char data;
	unsigned char *pBuffer = lcd_buffer;
	
	// pull POL 
	rts26151_write_POL( lcd_POL );
	lcd_POL ^= 1;
	
	for( y = 0; y < MS_LCD_HEIGHT; y ++ ) {
	
		rts26151_write_LD( 0 );	// pull LD low 
	
		// output SEG# 
		for( x = 0; x < MS_LCD_WIDTH / 8; x ++ ) {
		
			data = *pBuffer ++;
			bit = 0x80;
			
			for( i = 0; i < 8; i ++ ) {
				rts26151_write_DIN( data & bit );
				bit >>= 1;
			}
		}
		
		// output COM# 
		for( x = 0; x < y; x ++ ) 				// zero in beginning 
			rts26151_write_DIN( 0 );
		
		if( x == MS_LCD_HEIGHT - 1 )			// one in middle 
			;
		else {
			rts26151_write_DIN( 1 );
			x ++;
		}
		
		for( ; x < MS_LCD_HEIGHT - 1; x ++ ) 	// zero in tail 
			rts26151_write_DIN( 0 );
			
		if( x == y ) {							// last one with LD pulse 
			rts26151_write_DIN_with_LD( 1 );
		} else {
			rts26151_write_DIN_with_LD( 0 );
		}
	} // for each row 
}

// ============================================================== 
// Refresh screen timer 
// ============================================================== 
static void rts26151_do_refresh_screen( unsigned long arg )
{
	// refresh screen 
	rts26151_refresh_screen();
}

static void rts26151_init_refresh_screen_timer( void )
{
	register_timer( ( fn_timer_t )rts26151_do_refresh_screen, NULL, REFRESH_PERIOD * 10 );
}

// ============================================================== 
// API for external usage 
// ============================================================== 
void rts26151_WriteData( int start, const unsigned char *pdata, int len )
{
	if( start >= MS_LCD_VRAM_SIZE )
		return;
		
	if( start + len > MS_LCD_VRAM_SIZE )
		len = MS_LCD_VRAM_SIZE - start;
		
	memcpy( &lcd_buffer[ start ], pdata, len );
	
	// We will update these data to screen by timer. 
}

void rts26151_LCM_init( void )
{
	// init variable 
	memset( lcd_buffer, 0, MS_LCD_VRAM_SIZE );
	lcd_POL = 0;
	
	// init GPIO 
	rts26151_init_lcm_gpio();
	
	// init PIN 
	rts26151_write_POL( 1 );	// pull POL high (related to lcd_POL)
	rts26151_write_LD( 1 );		// pull LD high 
	rts26151_write_CLK( 0 );	// pull CLK low 
	
	// init software timer to refresh screen 
	rts26151_init_refresh_screen_timer();
}

