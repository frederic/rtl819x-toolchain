#include "voip_types.h"
#include "con_register.h"
#include "zarlink_api.h"
#include "snd_zarlink_ioc_op.h"

// ------------------------------------------------------------
// Zarlink IOC ops 
// ------------------------------------------------------------
void InitializeZarlinkIO( voip_ioc_t *this )
{
	zarlink_ioc_priv_t * const priv = this ->priv;
	RTKLineObj * const pLine = (RTKLineObj * )priv ->snd_priv;
	
	/* set output */
	ZarlinkSetIODir( pLine, priv ->IO, 1 );
	
	/* set low */
	//ZarlinkSetIOState( pLine, priv ->IO, 0 );
	
	/* set by state_var */
	this ->ioc_ops ->set_state( this, this ->state_var );
}

static int ioc_op_set_led_state_zarlink( voip_ioc_t *this, ioc_state_t state )
{
	zarlink_ioc_priv_t * const priv = this ->priv;
	RTKLineObj * const pLine = (RTKLineObj * )priv ->snd_priv;
	int bHigh;
	
	switch( state ) {
	case IOC_STATE_LED_ON:
		bHigh = 1;	// high --> LED ON 
		break;
		
	case IOC_STATE_LED_OFF:
		bHigh = 0;	// low --> LED off 
		break;
		
	default:
		return -1;
	}
	
	ZarlinkSetIOState( pLine, priv ->IO, bHigh );
	
	return 0;
}

static int ioc_op_set_relay_state_zarlink( voip_ioc_t *this, ioc_state_t state )
{
	zarlink_ioc_priv_t * const priv = this ->priv;
	RTKLineObj * const pLine = (RTKLineObj * )priv ->snd_priv;
	int bHigh;
	
	switch( state ) {
	case IOC_STATE_RELAY_CLOSE:
		bHigh = 1;	// high --> close 
		break;
		
	case IOC_STATE_RELAY_OPEN:
		bHigh = 0;	// low --> open 
		break;
		
	default:
		return -1;
	}
	
	ZarlinkSetIOState( pLine, priv ->IO, bHigh );
	
	return 0;
}

static uint32 ioc_op_get_id_zarlink( voip_ioc_t *this )	// for display 
{
	zarlink_ioc_priv_t * const priv = this ->priv;
	
	return ( uint32 )priv ->IO;
}

ioc_ops_t ioc_led_ops_zarlink = {
	.set_state = ioc_op_set_led_state_zarlink,
	.get_id = ioc_op_get_id_zarlink,
};

ioc_ops_t ioc_relay_ops_zarlink = {
	.set_state = ioc_op_set_relay_state_zarlink,
	.get_id = ioc_op_get_id_zarlink,
};

// ------------------------------------------------------------
// Zarlink VPIO list definition 
// ------------------------------------------------------------
VPIO zarlink_VPIO_list[] = {
    VPIO_IO1,  
    VPIO_IO2,
    VPIO_IO3,
    VPIO_IO4,	
};

int zarlink_VPIO_list_num = ( sizeof( zarlink_VPIO_list ) / sizeof( zarlink_VPIO_list[ 0 ] ) );

