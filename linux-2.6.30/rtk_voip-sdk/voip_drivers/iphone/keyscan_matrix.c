#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/string.h>
#include "keyscan_matrix.h"
#include "../gpio/gpio.h"
#include "base_gpio.h"
#include <linux/delay.h>	//udelay(),mdelay()
#include "keypad_map.h"
#include "keypad_interface.h"
#if defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00 )
#include <asm-mips/rtl865x/platform.h>
#endif

// GPIO mode switch (interrupt/polling)
#ifdef CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00
  #define KS_INT_MODE		// interrupt or polling mode 
  #ifdef CONFIG_DEFAULTS_KERNEL_2_6
    #define KS_GPIO_IRQ 17   // ABCD: 16, EFGH: 17
  #else
    #define KS_GPIO_IRQ	0	// ABCD: 1, EFGH: 0 
  #endif
  //#define KS_HELP_HOOK_POLLING
#else
  //#define KS_INT_MODE		// interrupt or polling mode 
  #define KS_GPIO_IRQ	5???
  //#define KS_HELP_HOOK_POLLING
#endif

// debug option 
//#define KS_DEBUG_INT_MSG
//#define KS_DEBUG_MSG
//#define KS_DEBUG_TIMER_MSG

// GPIO ploarity definition 
#ifdef CONFIG_RTK_VOIP_GPIO_IPP_8952_V00
  //#define KS_GPIO_POLARITY
  //#define KS_WEAK_OUT_GPIO
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00 )
  #define KS_GPIO_POLARITY
  #define KS_ONLY_ONE_OUT_GPIO 	// set only one GPIO to be out simultaneously. 
#endif

#ifdef KS_GPIO_POLARITY
  #define POL_ON		1
  #define POL_OFF		0
  #define POL_CHK( d )	( d )
#else
  #define POL_ON		0
  #define POL_OFF		1
  #define POL_CHK( d )	( !d )
#endif

// GPIO PIN definition 
// --------------------------------------------------------------
#ifdef CONFIG_RTK_VOIP_GPIO_IPP_8952_V00
#define KS_PIN_COL0		GPIO_ID(GPIO_PORT_A,0)	//output
#define KS_PIN_COL1		GPIO_ID(GPIO_PORT_A,1)	//output
#define KS_PIN_COL2		GPIO_ID(GPIO_PORT_A,2)	//output
#define KS_PIN_COL3		GPIO_ID(GPIO_PORT_A,3)	//output 
#define KS_PIN_COL4		GPIO_ID(GPIO_PORT_A,4)	//output 
#define KS_PIN_COL5		GPIO_ID(GPIO_PORT_A,5)	//output 
#define KS_PIN_COL6		GPIO_ID(GPIO_PORT_A,6)	//output

#define KS_PIN_ROW0		GPIO_ID(GPIO_PORT_A,7)	//input
#define KS_PIN_ROW1		GPIO_ID(GPIO_PORT_C,0)	//input
#define KS_PIN_ROW2		GPIO_ID(GPIO_PORT_C,1)	//input
#define KS_PIN_ROW3		GPIO_ID(GPIO_PORT_C,2)	//input
#define KS_PIN_ROW4		GPIO_ID(GPIO_PORT_C,3)	//input
#define KS_PIN_ROW5		GPIO_ID(GPIO_PORT_C,4)	//input

#define KS_PIN_INPUT( n )	KS_PIN_ROW ## n
#define KS_PIN_INPUT_NUM	6	
#define KS_PIN_OUTPUT( m )	KS_PIN_COL ## n
#define KS_PIN_OUTPUT_NUM	7

#define PIN_IN_ONLY( idx )	( 0 )
#define PIN_OUT_ONLY( idx )	( 0 )
// --------------------------------------------------------------
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00 )
#define KS_PIN_COL0		GPIO_ID(GPIO_PORT_E,1)	//input
#define KS_PIN_COL1		GPIO_ID(GPIO_PORT_E,2)	//input
#define KS_PIN_COL2		GPIO_ID(GPIO_PORT_E,3)	//input
#define KS_PIN_COL3		GPIO_ID(GPIO_PORT_E,4)	//input 
#define KS_PIN_COL4		GPIO_ID(GPIO_PORT_E,5)	//input 
#define KS_PIN_COL5		GPIO_ID(GPIO_PORT_F,4)	//input (IN only)
#define KS_PIN_COL6		GPIO_ID(GPIO_PORT_F,5)	//input (IN only)

#define KS_PIN_ROW0		GPIO_ID(GPIO_PORT_D,5)	//output
#define KS_PIN_ROW1		GPIO_ID(GPIO_PORT_D,6)	//output
#define KS_PIN_ROW2		GPIO_ID(GPIO_PORT_F,2)	//output
#define KS_PIN_ROW3		GPIO_ID(GPIO_PORT_A,7)	//output (OUT only)
#define KS_PIN_ROW4		GPIO_ID(GPIO_PORT_C,1)	//output
#define KS_PIN_ROW5		GPIO_ID(GPIO_PORT_C,2)	//output

#define KS_PIN_INPUT( n )	KS_PIN_COL ## n
#define KS_PIN_INPUT_NUM	7	
#define KS_PIN_OUTPUT( m )	KS_PIN_ROW ## m
#define KS_PIN_OUTPUT_NUM	6

#define PIN_IN_ONLY( idx )	( idx >= 5 )
#define PIN_OUT_ONLY( idx )	( idx == 3 )
#endif


// GPIO PIN matrix 
static const uint32 keyscan_output[] = {
	KS_PIN_OUTPUT( 0 ),
	KS_PIN_OUTPUT( 1 ),
	KS_PIN_OUTPUT( 2 ),
	KS_PIN_OUTPUT( 3 ),
	KS_PIN_OUTPUT( 4 ),
	KS_PIN_OUTPUT( 5 ),
#if KS_PIN_OUTPUT_NUM == 7
	KS_PIN_OUTPUT( 6 ),
#endif
};

static const uint32 keyscan_input[] = {
	KS_PIN_INPUT( 0 ),
	KS_PIN_INPUT( 1 ),
	KS_PIN_INPUT( 2 ),
	KS_PIN_INPUT( 3 ),
	KS_PIN_INPUT( 4 ),
	KS_PIN_INPUT( 5 ),
#if KS_PIN_INPUT_NUM == 7
	KS_PIN_INPUT( 6 ),
#endif
};

#define KS_PIN_NUM_OF_OUTPUT	( sizeof( keyscan_output ) / sizeof( keyscan_output[ 0 ] ) )
#define KS_PIN_NUM_OF_INPUT		( sizeof( keyscan_input ) / sizeof( keyscan_input[ 0 ] ) )

// NOTE: If number of column and row is larger than 16, 
//       pay attention to keyscan code. 

// internal variables 
//static keypad_dev_t keyscan_dev_data = { 0, 0 };
#ifdef KS_INT_MODE
//static int keyscan_timer_scheduled = 0;	// prevent to add timer duplicately. 
static int keyscan_debounce_working = 0;
static int keyscan_interrupt_cookie = 0;
#endif

// ====================================================================== 
// internal functions 
static void keyscan_init_gpio( void )
{
	int i;
		
	// init output 
	for( i = 0; i < KS_PIN_NUM_OF_OUTPUT; i ++ ) {
		__key_initGpioPin( keyscan_output[ i ], GPIO_DIR_OUT, GPIO_INT_DISABLE );
#ifdef KS_INT_MODE
		__key_setGpioDataBit( keyscan_output[ i ], POL_ON );
#endif
	}

	// init input 
	for( i = 0; i < KS_PIN_NUM_OF_INPUT; i ++ ) {
#ifdef KS_INT_MODE
		//if( i == 0 )
		//	__key_initGpioPin( keyscan_input[ i ], GPIO_DIR_IN, GPIO_INT_BOTH_EDGE );
		//else
		//	__key_initGpioPin( keyscan_input[ i ], GPIO_DIR_IN, GPIO_INT_DISABLE );
		
		__key_initGpioPin( keyscan_input[ i ], GPIO_DIR_IN, GPIO_INT_BOTH_EDGE );	
#else
		__key_initGpioPin( keyscan_input[ i ], GPIO_DIR_IN, GPIO_INT_DISABLE );	
#endif
	}
}

static void keyscan_in_gpio_interrupt( int bEnable )
{
	int i;
	const enum GPIO_INTERRUPT_TYPE type = ( bEnable ? GPIO_INT_BOTH_EDGE : GPIO_INT_DISABLE );
	
	// turn on interrupt 
	for( i = 0; i < KS_PIN_NUM_OF_INPUT; i ++ ) {
		__key_initGpioPin( keyscan_input[ i ], GPIO_DIR_IN, type );	
	}
}

static int keyscan_do_scan_core( unsigned char *pkey )
{
	int i, j;
	uint32 data;
	int count = 0;

#ifdef KS_DEBUG_MSG 	
	printk( "S " );
#endif
	
	// set all output to 0 
	for( i = 0; i < KS_PIN_NUM_OF_OUTPUT; i ++ ) {
		__key_setGpioDataBit( keyscan_output[ i ], POL_OFF );
	}
	
	for( i = 0; i < KS_PIN_NUM_OF_OUTPUT; i ++ ) {
		
#ifdef KS_ONLY_ONE_OUT_GPIO
		for( j = 0; j < KS_PIN_NUM_OF_OUTPUT; j ++ ) {
			if( !PIN_OUT_ONLY( j ) )
				__key_initGpioPin( keyscan_output[ j ], GPIO_DIR_IN, GPIO_INT_DISABLE );
		}
			
		__key_initGpioPin( keyscan_output[ i ], GPIO_DIR_OUT, GPIO_INT_DISABLE );		
#endif
	
		// set current col to be 1 
		__key_setGpioDataBit( keyscan_output[ i ], POL_ON );
		
		for( j = 0; j < KS_PIN_NUM_OF_INPUT; j ++ ) {
		
			// clear row data, or residual input signal was detected. 
			// note: don't output to pin which is input only on 8972B
			if( !PIN_IN_ONLY( j ) ) {
				__key_initGpioPin( keyscan_input[ j ], GPIO_DIR_OUT, GPIO_INT_DISABLE );	
				__key_setGpioDataBit( keyscan_input[ j ], POL_OFF );
			}
		
			// get row data 
			__key_initGpioPin( keyscan_input[ j ], GPIO_DIR_IN, GPIO_INT_DISABLE );	
			__key_getGpioDataBit( keyscan_input[ j ], &data );
			
			if( POL_CHK( data ) ) {	// this key is pressed! 
				count ++;
				*pkey = ( ( unsigned char )i << 4 ) | ( unsigned char )( j + KS_PIN_NUM_OF_OUTPUT );
#ifndef KS_DEBUG_MSG
				if( count >= 2 ) {
					goto label_break_keyscan;
				}
#else
				printk( "%X ", *pkey );
#endif
			}
		} // each row 
		
		// unset current col to be 1 
		__key_setGpioDataBit( keyscan_output[ i ], POL_OFF );		
	} // each col 

label_break_keyscan:

#ifdef KS_DEBUG_MSG
	printk( "\n" );
#endif
	
	return count;
}

static void keyscan_do_scan( unsigned long arg )
{
	unsigned char key;
	int count;
	dbstate_t dbstate;

#ifdef KS_DEBUG_TIMER_MSG
	printk( "KT\n" );
#endif

#ifdef KS_HELP_HOOK_POLLING
	if( !keyscan_debounce_working ) {
		keypad_scan_result( KEY_NUM_HOOK_ONLY, 0 );
		goto label_hook_polling_done;
	}
#endif
	
	// scan core 
	count = keyscan_do_scan_core( &key );	// NOTE: This function will change INT status 
	
	dbstate = keypad_scan_result( count, key );
#ifdef KS_DEBUG_INT_MSG
	printk( "dbstate=%d\n", dbstate );
#endif
	
	// timer 
#ifdef KS_HELP_HOOK_POLLING
label_hook_polling_done:
#endif

#ifdef KS_INT_MODE	
	if( check_keypad_debounce_complete() ) {
		if( keyscan_debounce_working ) {
  #ifdef KS_HELP_HOOK_POLLING
			// turn on interrupt 
			//keyscan_in_gpio_interrupt( 1 );
			keyscan_init_gpio();
  #endif
			
			keyscan_debounce_working = 0;
  #ifdef KS_DEBUG_INT_MSG
			printk( "0EF_IMR:%lx\n", REG32( 0xB8003530 ) );
			printk( "0ABCD:%lx,EFGH:%lx\n", REG32( 0xB8003510 ), REG32( 0xB800352C ) );
			printk( "keyscan_debounce_working=0\n" );
  #endif
		}
  #ifndef KS_HELP_HOOK_POLLING
		if( check_hook_debounce_complete() ) {
			// turn on interrupt 
			//keyscan_in_gpio_interrupt( 1 );
			keyscan_init_gpio();
			
			ipphone_hook_interrupt( 1 );
			return;
		}
  #endif
	} 
#endif
}

static void keyscan_init_timer( void )
{	
	register_timer_10ms( ( fn_timer_t )keyscan_do_scan, NULL );
}

#ifdef KS_INT_MODE
#ifdef CONFIG_DEFAULTS_KERNEL_2_6
static irq_handler_t keyscan_irq_handler( int irq, void *dev_id, struct pt_regs *regs )
#else
static void keyscan_irq_handler( int irq, void *dev_id, struct pt_regs *regs )
#endif
{
  #ifdef KS_DEBUG_INT_MSG
	printk( "ABCD:%lx,EFGH:%lx\n", REG32( 0xB8003510 ), REG32( 0xB800352C ) );
  #endif

	// write '1' to clear status 
  #ifdef CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00
    #if KS_GPIO_IRQ == 0 || KS_GPIO_IRQ == 17
	REG32( 0xB800352C ) = REG32( 0xB800352C );		// EFGH 
	#elif KS_GPIO_IRQ == 1 || KS_GPIO_IRQ == 16
	REG32( 0xB8003510 ) = REG32( 0xB8003510 );		// ABCD 
	#endif
  #else
	???
  #endif
	
	// check whether timer is working or not 
	if( keyscan_debounce_working ) 
		goto keyscan_irq_handler_done;
	
	keyscan_debounce_working = 1;	
#ifdef KS_DEBUG_INT_MSG
	printk( "keyscan_debounce_working=1\n" );
#endif

	// turn off interrupt 
	keyscan_in_gpio_interrupt( 0 );
  #ifndef KS_HELP_HOOK_POLLING
	ipphone_hook_interrupt( 0 );
  #endif
		
	// init timer to do keyscan 
  #ifndef KS_HELP_HOOK_POLLING
	keyscan_init_timer();
  #endif	 	

keyscan_irq_handler_done:
	;
#ifdef CONFIG_DEFAULTS_KERNEL_2_6
    return IRQ_HANDLED;
#endif
}
#endif

// ====================================================================== 
// API 
void keyscan_matrix_init( void )
{	
	keyscan_init_gpio();

#if 0	
	{
		unsigned char key;
		
		while( 1 ) {
			keyscan_do_scan_core( &key );
		}
	}
#endif

	init_timer( &keyscan_timer );

#ifdef KS_INT_MODE	
  #ifdef CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00
    #if KS_GPIO_IRQ == 0 || KS_GPIO_IRQ == 17		// EFGH
	REG32(GIMR) = (REG32(GIMR)) | (GPIO_EFGH_IE);
	REG32(IRR2) = (REG32(IRR2)) | (GPIO_EFGH_RS << 4);
    #elif KS_GPIO_IRQ == 1 || KS_GPIO_IRQ == 16		// ABCD
	REG32(GIMR) = (REG32(GIMR)) | (GPIO_ABCD_IE);
	REG32(IRR2) = (REG32(IRR2)) | (GPIO_ABCD_RS << 0);
    #endif
  #else
	???
  #endif

	request_irq( KS_GPIO_IRQ, 
				 keyscan_irq_handler, 
				 SA_SHIRQ, 
				 "matrix_keyscan",
				 &keyscan_interrupt_cookie );
  #ifdef KS_HELP_HOOK_POLLING
	keyscan_init_timer();
  #endif
#else
	keyscan_init_timer();
#endif // KS_INT_MODE

}


