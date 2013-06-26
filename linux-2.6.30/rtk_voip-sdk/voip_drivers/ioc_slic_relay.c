#include "rtk_voip.h"
#include "voip_init.h"
#include "gpio/gpio.h"

#include "con_register.h"

static const uint32 relay_gpio_id[] = {
#if defined(CONFIG_RTK_VOIP_GPIO_8954C_V100) || defined(CONFIG_RTK_VOIP_GPIO_8954C_V200)
	PIN_RELAY0, 
	PIN_RELAY1, 
#elif defined (CONFIG_RTK_VOIP_GPIO_8672_VQD01)
	PIN_RELAY,
#endif
};

#define NUM_OF_RELAY_GPIO	( sizeof( relay_gpio_id ) / sizeof( relay_gpio_id[ 0 ] ) )

static void SLIC_Relay_init( void )
{
	int i;
	
	for ( i=0; i < NUM_OF_RELAY_GPIO; i++ ) {
		_rtl_generic_initGpioPin( relay_gpio_id[ i ], GPIO_CONT_GPIO,
								GPIO_DIR_OUT, GPIO_INT_DISABLE );
		
		_rtl_generic_setGpioDataBit( relay_gpio_id[ i ], 1 );
	}
}

static int voip_ioc_gpio_relay_set_state( struct voip_ioc_s *this, ioc_state_t state )
{
	const uint32 gpio_id = ( uint32 )this ->priv;
	
	switch( state ) {
	case IOC_STATE_RELAY_CLOSE:	/* close */
		_rtl_generic_setGpioDataBit( gpio_id, 1 );
		break;
		
	case IOC_STATE_RELAY_OPEN:	/* open */
		_rtl_generic_setGpioDataBit( gpio_id, 0 );
		break;
		
	default:
		return -1;
	}
	
	return 0;
}

static uint32 voip_ioc_gpio_relay_get_id( struct voip_ioc_s *this )
{
	// for display 
	const uint32 gpio_id = ( uint32 )this ->priv;
	
	return gpio_id;
}

// --------------------------------------------------------
// ioc register 
// --------------------------------------------------------

static const ioc_ops_t voip_ioc_gpio_relay_ops = {
	.set_state 	= voip_ioc_gpio_relay_set_state, 
	.get_id		= voip_ioc_gpio_relay_get_id, 
};

static voip_ioc_t voip_ioc_gpio_relay[ NUM_OF_RELAY_GPIO ];

static int __init voip_init_ioc_gpio_relay( void )
{
	int i;
	
	for( i = 0; i < NUM_OF_RELAY_GPIO; i ++ ) {
		voip_ioc_gpio_relay[ i ].ioch = i;
		voip_ioc_gpio_relay[ i ].name = "Relay-g";
		voip_ioc_gpio_relay[ i ].ioc_type = IOC_TYPE_RELAY;
		voip_ioc_gpio_relay[ i ].pre_assigned_snd_ptr = NULL;	// system assign 
		voip_ioc_gpio_relay[ i ].ioc_ops = &voip_ioc_gpio_relay_ops;
		voip_ioc_gpio_relay[ i ].priv = ( void * )relay_gpio_id[ i ];
	}
	
	register_voip_ioc( voip_ioc_gpio_relay, NUM_OF_RELAY_GPIO );
	
	// init GPIO 
	SLIC_Relay_init();
	
	return 0;
}

voip_initcall_ioc_relay( voip_init_ioc_gpio_relay );

