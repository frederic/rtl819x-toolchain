#include "con_register.h"
#include "voip_init.h"
#include "voip_timer.h"

// ----------------------------------------------------------
// LED blinking timer 
// ----------------------------------------------------------
#define LED_BLINKING_PERIOD		5	//50ms

static int leds_blinking_timer_active = 0;

static void activate_leds_blinking_timer( void )
{
	//leds_blinking_timer.expires = jiffies + LED_BLINKING_PERIOD;
	//add_timer( &leds_blinking_timer);
	//mod_timer( &leds_blinking_timer, jiffies + LED_BLINKING_PERIOD);
	
	leds_blinking_timer_active = 1;
}

static void deactivate_leds_blinking_timer( void )
{
	leds_blinking_timer_active = 0;
}

static void leds_blinking_timer_func( void *data )
{
	voip_ioc_t *p_ioc = find_first_ioc();
	int blinking_count = 0;

	if( leds_blinking_timer_active == 0 )
		return;
	
	while( p_ioc ) {
		
		if( p_ioc ->ioc_type == IOC_TYPE_LED &&
			p_ioc ->mode_var == IOC_MODE_BLINKING )
		{
			blinking_count ++;
			
			// invert state 
			if( p_ioc ->state_var == IOC_STATE_LED_ON )
				p_ioc ->state_var = IOC_STATE_LED_OFF;
			else
				p_ioc ->state_var = IOC_STATE_LED_ON;
			
			// set state 
			p_ioc ->ioc_ops ->set_state( p_ioc, p_ioc ->state_var );
		}
		
		// next 
		p_ioc = p_ioc ->link.next;
	}
	
	/* add timer again */
	if( blinking_count )
		activate_leds_blinking_timer();
	else
		deactivate_leds_blinking_timer();
}

static int __init voip_init_led_blinking_timer( void )
{
	register_timer( leds_blinking_timer_func, NULL, LED_BLINKING_PERIOD * 10 );
	
	return 0;
}

voip_initcall( voip_init_led_blinking_timer );

// ----------------------------------------------------------
// User's request  
// ----------------------------------------------------------
int Set_LED_Display_ioc( voip_ioc_t *p_ioc, 
						ioc_mode_t ioc_mode, ioc_state_t ioc_state )
{
	int ret;
	
	p_ioc ->mode_var = ioc_mode;
	p_ioc ->state_var = ioc_state;
	
	ret = p_ioc ->ioc_ops ->set_state( p_ioc, ioc_state );
	
	if( ret == 0 && ioc_mode == IOC_MODE_BLINKING )
		activate_leds_blinking_timer();
	
	return ret;
}

int Set_SLIC_Relay_ioc( voip_ioc_t *p_ioc, ioc_state_t ioc_state )
{
	p_ioc ->mode_var = IOC_MODE_NORMAL;
	p_ioc ->state_var = ioc_state;
	
	return p_ioc ->ioc_ops ->set_state( p_ioc, ioc_state );
}

// ----------------------------------------------------------
// Assign IOC to con 
// ----------------------------------------------------------
static inline int catenate_ioc_to_con( voip_con_t *p_con, voip_ioc_t *p_ioc )
{
	switch( p_ioc ->ioc_type ) {
	case IOC_TYPE_LED:
		if( !p_con ->ioc_led0_ptr ) {
			p_con ->ioc_led0_ptr = p_ioc;
			p_ioc ->con_ptr = p_con;
			goto label_ok;
		}
		
		if( !p_con ->ioc_led1_ptr ) {
			p_con ->ioc_led1_ptr = p_ioc;
			p_ioc ->con_ptr = p_con;
			goto label_ok;
		}
		
		break;
		
	case IOC_TYPE_RELAY:
		if( !p_con ->ioc_relay_ptr ) {
			p_con ->ioc_relay_ptr = p_ioc;
			p_ioc ->con_ptr = p_con;
			goto label_ok;
		}
		
		break;
	} 
	
	return -1;

label_ok:
	
	return 1;
}

int assign_ioc_led_relay_to_con( voip_con_t voip_con[], int num )
{
	int i;
	int type_loop;
	voip_ioc_t *p_ioc = find_first_ioc();
	int ok_count = 0;
	
	// scan for pre-assigned ioc 
	while( p_ioc ) {
		
		voip_snd_t *p_snd = p_ioc ->pre_assigned_snd_ptr;
		voip_con_t *p_con;
		
		if( !p_snd )	// no pre-assigned snd ptr 
			goto label_next_preassigned_ioc;
		
		p_con = p_snd ->con_ptr;
		
		if( !p_con )	// no bind con 
			goto label_next_preassigned_ioc;
		
		ok_count += catenate_ioc_to_con( p_con, p_ioc );

label_next_preassigned_ioc:		
		p_ioc = p_ioc ->link.next;
	}
	
	// try to assign other ioc to con 
	for( type_loop = 0; type_loop < 3; type_loop ++ ) {
		// 0: led0, 1: led1, 2: relay 
		voip_ioc_t *p_ioc_tmp;
		ioc_type_t ioc_type_tmp;
		
		p_ioc = find_first_ioc();
		
		for( i = 0; i < num; i ++ ) {

			switch( type_loop ) {
			case 0:
				p_ioc_tmp = voip_con[ i ].ioc_led0_ptr;
				ioc_type_tmp = IOC_TYPE_LED;
				break;
			case 1:
				p_ioc_tmp = voip_con[ i ].ioc_led1_ptr;
				ioc_type_tmp = IOC_TYPE_LED;
				break;
			case 2:
				p_ioc_tmp = voip_con[ i ].ioc_relay_ptr;
				ioc_type_tmp = IOC_TYPE_RELAY;
				break;
			default:
				p_ioc_tmp = ( voip_ioc_t * )0x12345678;	// magic to break. avoid compiler warning only 
				ioc_type_tmp = IOC_TYPE_RELAY;
				break;
			}
			
			// this con has this ioc ptr 
			if( p_ioc_tmp )
				continue;
			
			// search for a suitable ioc 
			while( p_ioc ) {
				
				int match = 1;
				
				if( p_ioc ->ioc_type != ioc_type_tmp )	// check type 
					match = 0;
				
				if( p_ioc ->pre_assigned_snd_ptr )	// ignore pre-assign 
					match = 0;
				
				if( p_ioc ->con_ptr )	// assigned 
					match = 0;
				
				// do assignment if need 
				if( match ) 
					ok_count += catenate_ioc_to_con( &voip_con[ i ], p_ioc );
				
				p_ioc = p_ioc ->link.next;
				
				if( match ) 
					break;
			}
		}
	}
	
	return ok_count;
}

