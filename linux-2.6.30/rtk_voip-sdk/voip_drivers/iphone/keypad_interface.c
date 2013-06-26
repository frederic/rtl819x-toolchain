#include <linux/types.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_params.h"
#include "voip_control.h"
#include "keypad_map.h"
#if defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V00 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V01 )
#include "PT6961.h"
#endif
#include "base_gpio.h"
#include "keypad_interface.h"

#if defined( CONFIG_RTK_VOIP_GPIO_IPP_8952_V00 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00 )
	/* Now, keyscan_matrix.c use V2 only */
	#define ENABLE_KEYPAD_DEBOUNCING_V2
#else
	#define ENABLE_KEYPAD_DEBOUNCING
#endif

#define TIMER_INTERVAL_PERIOD		10	/* 10ms */
#define NOARML_PRESS_KEY_PERIOD		30	/* 30ms */
#define LONG_PRESS_KEY_PERIOD		500	/* 500ms */
#define HOOK_POLLING_INTERVAL		30	/* 30ms */
#define HOOK_DEBOUNCING_COUNT		0	/* HOOK_POLLING_INTERVAL * HOOK_DEBOUNCING_COUNT */

static void debug_keypad_signal_target( wkey_t wkey );
extern unsigned char iphone_hook_detect( void );

//#define NUM_OF_KEYPAD_QUEUE		( 5 + 1 )	/* queue size: 5 */

static pid_t sig_handler_pid = 0;
static wkey_t key_buffering;
static unsigned char key_w = 0, key_r = 0;
static unsigned char key_hook_status = 2;	/* 0: on-hook, 1: off-hook */

#if 0	// test only
static unsigned char hook_status = 0;	// TODO: it should read from hardware
#endif

static int GetHookStatus( unsigned char *pStatus )
{
	unsigned char status;

#if defined( CONFIG_RTK_VOIP_GPIO_IPP_100 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_101 )
	status = ( iphone_hook_detect() ? 1 : 0 );
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V00 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V01 )
	status = iphone_hook_detect();
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8952_V00 )
	status = iphone_hook_detect();
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00 )
	status = iphone_hook_detect();
#else
  #error "???"
#endif
	
	if( pStatus )
		*pStatus = status;
	
	if( status == key_hook_status )
		return 0;	/* unchange */
	
	key_hook_status = status;
	
	return 1;	/* changed!! */
}

void do_keypad_opt_ctl( void *pUser, unsigned int len )
{
	TstKeypadCtl keypadCtl;
	
	if( len > sizeof( TstKeypadCtl ) ) {
		len = sizeof( TstKeypadCtl );
		printk( "Keypad: Truncate user specified length\n" );
	}
	
	copy_from_user( &keypadCtl, pUser, len );
	
	switch( keypadCtl.General.cmd ) {
	case KEYPAD_CMD_SET_TARGET:
		sig_handler_pid = keypadCtl.SetTarget.pid;
		break;
		
	case KEYPAD_CMD_SIG_DEBUG:
		debug_keypad_signal_target( keypadCtl.SignalDebug.wkey );
		break;
	
	case KEYPAD_CMD_READ_KEY:
		if( key_w != key_r ) {
			keypadCtl.ReadKey.wkey = key_buffering;
			keypadCtl.ReadKey.validKey = 1;
			
			key_r = ( key_r + 1 ) & 0x01;
		} else
			keypadCtl.ReadKey.validKey = 0;
			
		copy_to_user( pUser, &keypadCtl.ReadKey, sizeof( keypadCtl.ReadKey ) );
		break;
		
	case KEYPAD_CMD_HOOK_STATUS:
		//keypadCtl.HookStatus.status = hook_status;
		GetHookStatus( &keypadCtl.HookStatus.status );
		copy_to_user( pUser, &keypadCtl.HookStatus, sizeof( keypadCtl.HookStatus ) );
		break;
	
	default:
		printk( "Keypad: this cmd not support\n" );
	}
}

static void do_keypad_signal_target( wkey_t wkey )
{
#if 0	// test only
	if( wkey == KEY_HOOK )
		hook_status = ( hook_status ? 0 : 1 );	// xor 
#endif

	if( !sig_handler_pid ) {
		printk( "Keypad(%o): no target pid?\n", wkey );
		return;
	}

	if( key_w == key_r ) {
	
		key_buffering = wkey;
		
		key_w = ( key_w + 1 ) & 0x01;
	}

	/* signal to user space */
	/*
	 * Value of third parameter can be 0 or 1:
	 *  0: SI_USER		(sent by kill, sigsend, raise)
	 *  1: SI_KERNEL	(sent by the kernel from somewhere)
	 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	kill_proc_info( SIGUSR1, NULL, sig_handler_pid );
#else
	kill_proc( sig_handler_pid, SIGUSR1, 0 );		
#endif
}

static void debug_keypad_signal_target( wkey_t wkey )
{
	do_keypad_signal_target( wkey );
}

#if 0
void keypad_signal_target( const keypad_dev_t *keypad_data_pool )
{
	if( keypad_data_pool ->flags == 0 )
		return;
	
	do_keypad_signal_target( keypad_data_pool ->data_string );
	
	keypad_data_pool ->flags = 0;
}
#endif /* !ENABLE_KEYPAD_DEBOUNCING */

// -----------------------------------------------------------------
// Debouncing function 
// -----------------------------------------------------------------
#ifdef ENABLE_KEYPAD_DEBOUNCING

#define NORMAL_PRESS_KEY_COUNT		( NOARML_PRESS_KEY_PERIOD / TIMER_INTERVAL_PERIOD )
#define LONG_PRESS_KEY_COUNT		( LONG_PRESS_KEY_PERIOD / TIMER_INTERVAL_PERIOD )

enum {
	DB_STATE_EMPTY,
	DB_STATE_PRESS,
	DB_STATE_CHECK_PRESS,
};

static unsigned char db_key = 0;
static unsigned int db_count = 0;
static unsigned int db_state = DB_STATE_EMPTY;
#if HOOK_POLLING_INTERVAL > 10
static unsigned int hook_polling_count = 0;
#endif

void keypad_polling_signal_target( keypad_dev_t *keypad_data_pool )
{
	/* During scan whole keys, it calls this function if a key is pressed. */
	if( keypad_data_pool ->flags == 0 )
		return;
		
	switch( db_state ) {
	case DB_STATE_EMPTY:	/* a fresh key */
	case DB_STATE_PRESS:	/* press more than one key? */
label_press_a_new_key:
		db_key = keypad_data_pool ->data_string;
		db_count = 1;
		db_state = DB_STATE_PRESS;
		break;
	
	case DB_STATE_CHECK_PRESS:	/* still press this key? */
		if( db_key != keypad_data_pool ->data_string )
			goto label_press_a_new_key;
		
		/* ok. still press */	
		db_count ++;
		db_state = DB_STATE_PRESS;
		break;
	}
	
	keypad_data_pool ->flags = 0;
}

void keypad_polling_scan_done( void )
{
	/* Call this function, once it scans whole keys. */
	/* i.e. It is executed in period of TIMER_INTERVAL_PERIOD. */
	switch( db_state ) {
	case DB_STATE_PRESS:
		if( db_count == NORMAL_PRESS_KEY_COUNT )	/* normal press key */
			do_keypad_signal_target( db_key );
		else if( db_count == LONG_PRESS_KEY_COUNT )	/* long press key */
			do_keypad_signal_target( 0x8000 | ( wkey_t )db_key );
			
		db_state = DB_STATE_CHECK_PRESS;
		break;
		
	case DB_STATE_CHECK_PRESS:
		db_key = 0;
		db_count = 0;
		db_state = DB_STATE_EMPTY;
		break;
	}
	
	/* hook status */
  #if HOOK_POLLING_INTERVAL == 10
  	{
  #else
  	if( ++ hook_polling_count >= HOOK_POLLING_INTERVAL / 10 )
	{
		hook_polling_count = 0;
		
  #endif
		if( GetHookStatus( NULL ) ) {
			do_keypad_signal_target( KEY_HOOK );
		}
	}
}
#endif /* ENABLE_KEYPAD_DEBOUNCING */

// -----------------------------------------------------------------
// Debouncing function V2
// -----------------------------------------------------------------
#ifdef ENABLE_KEYPAD_DEBOUNCING_V2

#define NORMAL_PRESS_KEY_COUNT		( NOARML_PRESS_KEY_PERIOD / TIMER_INTERVAL_PERIOD )
#define LONG_PRESS_KEY_COUNT		( LONG_PRESS_KEY_PERIOD / TIMER_INTERVAL_PERIOD )

static dbstate_t dbv2_state = DBST_INIT;
static unsigned int key_press_count = 0;
static unsigned char last_key1 = 0;
static unsigned int last_key_num = 0;

#if HOOK_DEBOUNCING_COUNT > 0
static unsigned int hook_debounce_count = 0;
#endif
#if HOOK_POLLING_INTERVAL > 10
static unsigned int hook_polling_count = 0;
#endif

static void keypad_hook_polling( void );

dbstate_t keypad_scan_result( unsigned int key_num, unsigned char key1 )
{
	if( key_num == KEY_NUM_HOOK_ONLY )
		goto label_do_hook_polling_only;

	switch( dbv2_state ) {
	case DBST_INIT:			// 0 -> 
		if( key_num > 1 ) {
			dbv2_state = DBST_MULTI_PRESS;
			break;			
		}
		
		if( key_num == 1 ) {
			key_press_count = 0;
			dbv2_state = DBST_NORMAL_PRESS;
		}
		break;
		
	case DBST_NORMAL_PRESS:	// 1 -> 
	case DBST_LONG_PRESS:	// 1 -> 
		if( key_num == 0 ) {			/* release pressed key */
			dbv2_state = DBST_INIT;
			break;
		} else if( key_num > 1 ) {		/* press more than one key simultaneously */
			dbv2_state = DBST_MULTI_PRESS;
			break;
		}
		
		if( last_key1 != key1 ) {		/* press another key */
			dbv2_state = DBST_NORMAL_PRESS;
			key_press_count = 0;
			break;
		}
		
		/* debouncing press */
		key_press_count ++;
		
		if( dbv2_state == DBST_LONG_PRESS &&
			key_press_count >= LONG_PRESS_KEY_COUNT )
		{	/* long pressed key case */
			do_keypad_signal_target( 0x8000 | ( wkey_t )key1 );
			dbv2_state = DBST_DONE;
			break;
		}
		
		if( dbv2_state == DBST_NORMAL_PRESS && 
			key_press_count >= NORMAL_PRESS_KEY_COUNT ) 
		{	/* normal pressed key case */
			do_keypad_signal_target( ( wkey_t )key1 );
			dbv2_state = DBST_LONG_PRESS;
			break;
		}
		
		break;
		
	case DBST_MULTI_PRESS:		// n -> 0
		/* Some keys are pressed, wait for user release them */
		if( key_num == 0 ) {
			dbv2_state = DBST_INIT;
		}
		break;
		
	case DBST_DONE:				// 1 -> 0
		if( key_num == 0 ) {
			dbv2_state = DBST_INIT;
		}
		break;
	}
	
	last_key1 = key1;
	last_key_num = key_num;
	
	/* hook part */
label_do_hook_polling_only:
	keypad_hook_polling();
	
	return dbv2_state;
}

int check_keypad_debounce_complete( void )
{
	/* 
	 * This function is used by interrupt mode only. 
	 * It help to check whether we can stop timer and enable interrupt. 
	 */	
	switch( dbv2_state ) {
	case DBST_INIT:
	case DBST_MULTI_PRESS:
	case DBST_DONE:
		break;
	
	default:
		return 0;
		break;
	}
	
	return 1;
}

int check_hook_debounce_complete( void )
{
	/* 
	 * This function is used by interrupt mode only. 
	 * It help to check whether we can stop timer and enable interrupt. 
	 */
#if HOOK_POLLING_INTERVAL > 10
	if( hook_polling_count )	/* polling interval */
		return 0;
#endif
#if HOOK_DEBOUNCING_INTERVAL > 0
	if( hook_debounce_count )	/* hook debouncing */
		return 0;
#endif
	
	return 1;
}

static void keypad_hook_polling( void )
{
#if HOOK_POLLING_INTERVAL > 10
  	if( ++ hook_polling_count >= HOOK_POLLING_INTERVAL / 10 )
		hook_polling_count = 0;
	else
		return;
#endif

#if HOOK_DEBOUNCING_COUNT == 0
	if( GetHookStatus( NULL ) ) {
		do_keypad_signal_target( KEY_HOOK );
	}
#else
	if( hook_debounce_count ) {
		if( GetHookStatus( NULL ) )
			hook_debounce_count = 0;	/* it is a bouncing hook */
		else {
			if( -- hook_debounce_count == 0 )
				do_keypad_signal_target( KEY_HOOK );
		}
	} else {
		if( GetHookStatus( NULL ) )
			hook_debounce_count = HOOK_DEBOUNCING_COUNT;
	}
#endif
}

#endif /* ENABLE_KEYPAD_DEBOUNCING_V2 */

