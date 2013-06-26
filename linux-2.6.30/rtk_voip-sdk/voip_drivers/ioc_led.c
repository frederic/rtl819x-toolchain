#include "rtk_voip.h"
#include "voip_init.h"
#include "gpio/gpio.h"

#include "con_register.h"

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
#elif defined(CONFIG_RTK_VOIP_GPIO_8954C_SOUNDWIN_XVN1420) 
static const uint32 leds_gpio_id[] = {
	PIN_VOIP0_LED,
	PIN_VOIP1_LED,
};
#elif defined( CONFIG_RTK_VOIP_GPIO_8954C_V400 ) 
static const uint32 leds_gpio_id[] = {
	PIN_VOIP0_LED,
	PIN_VOIP1_LED,
	//PIN_VOIP2_LED,
};
#elif defined( CONFIG_RTK_VOIP_GPIO_8972D_V100 ) 
static const uint32 leds_gpio_id[] = {
	PIN_VOIP0_LED,
	PIN_VOIP1_LED,
	//PIN_VOIP2_LED,
};
#elif defined (CONFIG_RTK_VOIP_GPIO_8676P_IAD_2LAYER_DEMO_BOARD_V01A) || defined (CONFIG_RTK_VOIP_GPIO_8676PN_IAD_2LAYER_DEMO_BOARD_V01)
static const uint32 leds_gpio_id[] = {
	PIN_VOIP0_LED,
};
#endif

#define NUM_OF_LEDS_GPIO	( sizeof( leds_gpio_id ) / sizeof( leds_gpio_id[ 0 ] ) )

static void LED_init( void )
{
	int i;
	
	for ( i=0; i < NUM_OF_LEDS_GPIO; i++ ) {
		_rtl_generic_initGpioPin( leds_gpio_id[ i ], GPIO_CONT_GPIO,
								GPIO_DIR_OUT, GPIO_INT_DISABLE );
	}
}

static int voip_ioc_gpio_led_set_state( struct voip_ioc_s *this, ioc_state_t state )
{
	const uint32 gpio_id = ( uint32 )this ->priv;
	
	switch( state ) {
	case IOC_STATE_LED_OFF:	/* off */
		_rtl_generic_setGpioDataBit( gpio_id, 1 );
		break;
		
	case IOC_STATE_LED_ON:	/* on */
		_rtl_generic_setGpioDataBit( gpio_id, 0 );
		break;
		
	default:
		return -1;
	}
	
	return 0;
}

static uint32 voip_ioc_gpio_led_get_id( struct voip_ioc_s *this )
{
	// for display 
	const uint32 gpio_id = ( uint32 )this ->priv;
	
	return gpio_id;
}

// --------------------------------------------------------
// ioc register 
// --------------------------------------------------------

static const ioc_ops_t voip_ioc_gpio_led_ops = {
	.set_state 	= voip_ioc_gpio_led_set_state, 
	.get_id		= voip_ioc_gpio_led_get_id, 
};

static voip_ioc_t voip_ioc_gpio_led[ NUM_OF_LEDS_GPIO ];

static int __init voip_init_ioc_gpio_led( void )
{
	int i;
	
	for( i = 0; i < NUM_OF_LEDS_GPIO; i ++ ) {
		voip_ioc_gpio_led[ i ].ioch = i;
		voip_ioc_gpio_led[ i ].name = "LED(g)";
		voip_ioc_gpio_led[ i ].ioc_type = IOC_TYPE_LED;
		voip_ioc_gpio_led[ i ].pre_assigned_snd_ptr = NULL;	// system assign 
		voip_ioc_gpio_led[ i ].ioc_ops = &voip_ioc_gpio_led_ops;
		voip_ioc_gpio_led[ i ].priv = ( void * )leds_gpio_id[ i ];
	}
	
	register_voip_ioc( voip_ioc_gpio_led, NUM_OF_LEDS_GPIO );
	
	LED_init();
	
	return 0;
}

voip_initcall_ioc( voip_init_ioc_gpio_led );

