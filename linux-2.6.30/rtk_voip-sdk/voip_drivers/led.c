#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <rtk_voip.h>
#include "led.h"

/* =================== For 8972B V100 EV Board LED Control ======================== */
/* =================== For 8954C V100 EV Board LED Control ======================== */
/* =================== For 8954C V200 EV Board LED Control ======================== */
#if defined( CONFIG_RTK_VOIP_GPIO_8972B ) || defined( CONFIG_RTK_VOIP_DRIVERS_PCM89xxC ) 

#include "gpio/gpio.h"
#include "voip_init.h"

static struct timer_list leds_timer;

static unsigned int leds_state;				/* each LED occupies a bit */
static unsigned int leds_state_blinking;
static unsigned int leds_state_blinking_onoff;

#ifdef CONFIG_RTK_VOIP_GPIO_8972B
static const uint32 leds_gpio_id[] = {
	PIN_VOIP1_LED,  // CH0 - FXS1: output
	PIN_VOIP2_LED,	// CH1 - FXS2: output
	PIN_VOIP3_LED,  // CH2 - FXS3: output
	PIN_VOIP4_LED,	// CH3 - FXS4: output
	//RFU			// DAA CH0
	//RFU			// DAA CH1
	//RFU			// SIP LED ( = VOIP_CH_NUM = SLIC_CH_NUM + DAA_CH_NUM )
};
#elif defined( CONFIG_RTK_VOIP_GPIO_8954C_V100 ) || defined( CONFIG_RTK_VOIP_GPIO_8954C_V200 )
static const uint32 leds_gpio_id[] = {
	PIN_VOIP1_LED,  // CH0 - FXS1: output
	PIN_VOIP2_LED,	// CH1 - FXS2: output
	PIN_PSTN_LED,	// DAA CH0
	//RFU			// DAA CH1
	//RFU			// SIP LED ( = VOIP_CH_NUM = SLIC_CH_NUM + DAA_CH_NUM )
};
#elif defined( CONFIG_RTK_VOIP_GPIO_8954C_V400 ) 
static const uint32 leds_gpio_id[] = {
	PIN_VOIP0_LED,
	PIN_VOIP1_LED,
	PIN_VOIP2_LED
};
#elif defined( CONFIG_RTK_VOIP_GPIO_8954C_PMC )
static const uint32 leds_gpio_id[] = {
        PIN_VOIP0_LED,
        PIN_VOIP1_LED
};
#elif defined (CONFIG_RTK_VOIP_GPIO_8676P_IAD_2LAYER_DEMO_BOARD_V01A) || defined (CONFIG_RTK_VOIP_GPIO_8676PN_IAD_2LAYER_DEMO_BOARD_V01)
static const uint32 leds_gpio_id[] = {
	PIN_VOIP0_LED,
};
#endif

#define NUM_OF_LEDS_GPIO	( sizeof( leds_gpio_id ) / sizeof( leds_gpio_id[ 0 ] ) )

/* We can use virtual CHID to represent SIP LED */
//#define SIP_LED_VIRTUAL_CHID	VOIP_CH_NUM

static void display_led_state( unsigned int chid, unsigned int onoff )
{
	if( chid >= NUM_OF_LEDS_GPIO )
		return;

	if( onoff ) {	/* on */
		_rtl_generic_setGpioDataBit( leds_gpio_id[ chid ], 0 );
	} else {		/* off */
		_rtl_generic_setGpioDataBit( leds_gpio_id[ chid ], 1 );
	}
}

static void leds_blinking_timer( unsigned long data )
{
	unsigned int chid;
	unsigned int bit;

	if( leds_state_blinking == 0 )
		return;
	
	for( chid = 0; chid < NUM_OF_LEDS_GPIO; chid ++ ) {
		
		bit = 1 << chid;
		
		if( !( leds_state_blinking & bit ) )
			continue;
		
		display_led_state( chid, leds_state_blinking_onoff & bit );
		
		leds_state_blinking_onoff ^= bit;
	}
	
	/* add timer again */
	leds_timer.expires = jiffies + LED_BLINKING_FREQ;
	add_timer( &leds_timer );
}

static void set_led_state( unsigned int chid, unsigned int state )
{
#if defined (CONFIG_RTK_VOIP_DRIVERS_ATA_DECT) && defined (CONFIG_RTK_VOIP_GPIO_8954C_V200)
	unsigned int bit;
	chid = chid - 1;
	bit = 1 << chid;
#else
	unsigned int bit = 1 << chid;
#endif
	unsigned int old_state;
	
	/* TODO: Do not go if state is the same */

	switch( state ) {
	case 0:	//off 
		leds_state &= ~bit;
		leds_state_blinking &= ~bit;
		
		display_led_state( chid, 0 );
		break;
	
	case 1:	//on
		leds_state |= bit;
		leds_state_blinking &= ~bit;
		
		display_led_state( chid, 1 );
		break;
	
	case 2:	//blinking
		old_state = leds_state_blinking;
	
		leds_state |= bit;
		leds_state_blinking |= bit;
		leds_state_blinking_onoff |= bit;
		
		display_led_state( chid, 1 );
		if( old_state == 0 ) {
			/* start timer */
			leds_timer.expires = jiffies + LED_BLINKING_FREQ;
			leds_timer.function = leds_blinking_timer;
			add_timer( &leds_timer );
		}
		break;
	
	default:
		printk( "Wrong ch(%u) LED state(%u)\n", chid, state );
		break;
	}
}

// FXS
void fxs_led_state( unsigned int chid, unsigned int state )
{
	//if( chid >= SLIC_CH_NUM )
	//	return;
	if( Is_DAA_Channel( chid ) )
		return;
	
	set_led_state( chid, state );
}

// FXO
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
void fxo_led_state( unsigned int chid, unsigned int state )
{
	//if( chid < SLIC_CH_NUM || chid >= VOIP_CH_NUM )
	//	return;
	if( !Is_DAA_Channel( chid ) )
		return;
	
	set_led_state( chid, state );
}
#endif

// SIP
void sip_led_state( unsigned int state )
{
	//set_led_state( SIP_LED_VIRTUAL_CHID, state );
}

static int __init LED_Init( void )
{
	int i = 0;
	
	init_timer( &leds_timer ); /* init once */

	/* global variables */
	leds_state = 0;
	leds_state_blinking = 0;
	leds_state_blinking_onoff = 0;

	for ( i=0; i < NUM_OF_LEDS_GPIO; i++ ) {
		_rtl_generic_initGpioPin( leds_gpio_id[ i ], GPIO_CONT_GPIO,
								GPIO_DIR_OUT, GPIO_INT_DISABLE );
	}
	
	/* set FXS state */
	for ( i=0; i < CON_CH_NUM; i++ )
	{	
		// fxs_led_state() will check if FXS 
		
		fxs_led_state( i, 0 );
	}
	
	return 0;
}

voip_initcall_led( LED_Init );
#endif 


