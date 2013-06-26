#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "ui_config.h"
#include "ioctl_keypad.h"
#include "ioctl_kernel.h"
#include "keypad_map.h"		/* hardware key scan code */
#include "ui_vkey.h"		/* UI virtual key */

#define M_TWO_KEYS( x )		{ VKEY_##x, KEY_##x }
#define VKEY_PhoneBook		VKEY_PHONEBOOK
static const struct key_mapping_s {
	unsigned char vkey;
	wkey_t scan_code;
} key_mapping[] = {
	M_TWO_KEYS( 0 ),
	M_TWO_KEYS( 1 ),
	M_TWO_KEYS( 2 ),
	M_TWO_KEYS( 3 ),
	M_TWO_KEYS( 4 ),
	M_TWO_KEYS( 5 ),
	M_TWO_KEYS( 6 ),
	M_TWO_KEYS( 7 ),
	M_TWO_KEYS( 8 ),
	M_TWO_KEYS( 9 ),
	M_TWO_KEYS( STAR ),
	M_TWO_KEYS( POUND ),
	M_TWO_KEYS( HOOK ),
#if KEYPAD_MAP_VENDOR == VENDOR_WCO
	M_TWO_KEYS( OUTVOL_PLUS ),
	M_TWO_KEYS( OUTVOL_MINUS ),
	M_TWO_KEYS( SPEAKER ),
	
	M_TWO_KEYS( OK ),
	M_TWO_KEYS( CANCEL ),
	
	M_TWO_KEYS( UP ),
	M_TWO_KEYS( DOWN ),
	M_TWO_KEYS( LEFT ),
	M_TWO_KEYS( RIGHT ),
	
	M_TWO_KEYS( CONFERENCE ),
	M_TWO_KEYS( PICK ),
	M_TWO_KEYS( TRANSFER ),
	M_TWO_KEYS( REDIAL ),
	M_TWO_KEYS( HOLD ),
	
	M_TWO_KEYS( LINE1 ),
	M_TWO_KEYS( LINE2 ),
	M_TWO_KEYS( F1 ),
	M_TWO_KEYS( F2 ),
	M_TWO_KEYS( F3 ),
	M_TWO_KEYS( FORWARD ),
	M_TWO_KEYS( DND ),
	M_TWO_KEYS( MISSED ),
	M_TWO_KEYS( VMS ),
	M_TWO_KEYS( BLIND_XFER ),
	M_TWO_KEYS( MUTE ),
	M_TWO_KEYS( HEADSET ),
#elif KEYPAD_MAP_VENDOR == VENDOR_WCO2
	M_TWO_KEYS( OUTVOL_PLUS ),
	M_TWO_KEYS( OUTVOL_MINUS ),
	
	M_TWO_KEYS( UP ),
	M_TWO_KEYS( DOWN ),

	M_TWO_KEYS( HEADSET ),
	M_TWO_KEYS( MUTE ),
	M_TWO_KEYS( SPEAKER ),

	M_TWO_KEYS( CONFERENCE ),
	M_TWO_KEYS( PICK ),
	M_TWO_KEYS( TRANSFER ),
	M_TWO_KEYS( REDIAL ),
	M_TWO_KEYS( HOLD ),
	
	M_TWO_KEYS( LINE1 ),
	M_TWO_KEYS( LINE2 ),
	M_TWO_KEYS( F1 ),
	M_TWO_KEYS( F2 ),
	M_TWO_KEYS( F3 ),
	M_TWO_KEYS( FORWARD ),
	M_TWO_KEYS( DND ),
	M_TWO_KEYS( MISSED ),
	M_TWO_KEYS( VMS ),
	M_TWO_KEYS( BLIND_XFER ),

	M_TWO_KEYS( INS_B1 ),
	M_TWO_KEYS( INS_B2 ),
	M_TWO_KEYS( INS_B3 ),
	M_TWO_KEYS( INS_B4 ),
	M_TWO_KEYS( INS_L1 ),
	M_TWO_KEYS( INS_L2 ),
	M_TWO_KEYS( INS_L3 ),
	M_TWO_KEYS( INS_L4 ),
#elif KEYPAD_MAP_VENDOR == VENDOR_TCO
	M_TWO_KEYS( OK ),
	M_TWO_KEYS( CANCEL ),

	M_TWO_KEYS( UP ),
	M_TWO_KEYS( DOWN ),
	M_TWO_KEYS( LEFT ),
	M_TWO_KEYS( RIGHT ),

	M_TWO_KEYS( MENU ),
	M_TWO_KEYS( HEADSET ),
	
	M_TWO_KEYS( OUTVOL_PLUS ),
	M_TWO_KEYS( OUTVOL_MINUS ),
	M_TWO_KEYS( SPEAKER ),	

	M_TWO_KEYS( CONFERENCE ),
	M_TWO_KEYS( TRANSFER ),
	M_TWO_KEYS( MUTE ),
	M_TWO_KEYS( HOLD ),
	M_TWO_KEYS( REDIAL ),
  #ifndef CONFIG_RTK_VOIP_GPIO_IPP_8972_V00
	M_TWO_KEYS( CALL_LOG ),
  #endif

	M_TWO_KEYS( F1 ),
	M_TWO_KEYS( F2 ),
	M_TWO_KEYS( F3 ),
	M_TWO_KEYS( F4 ),
	M_TWO_KEYS( F5 ),
	M_TWO_KEYS( F6 ),
	M_TWO_KEYS( F7 ),
	M_TWO_KEYS( F8 ),
	M_TWO_KEYS( F9 ),
	M_TWO_KEYS( F10 ),
	
  #ifndef CONFIG_RTK_VOIP_GPIO_IPP_8972_V00
	M_TWO_KEYS( ENTER ),	/* special key, share with OK */
  #endif
#elif KEYPAD_MAP_VENDOR == VENDOR_BCO
	M_TWO_KEYS( ENTER ),
	M_TWO_KEYS( DELETE	),
	M_TWO_KEYS( UP ),
	M_TWO_KEYS( DOWN ),
	M_TWO_KEYS( MENU ),
	M_TWO_KEYS( PHONEBOOK ),
	M_TWO_KEYS( REDIAL ),
	M_TWO_KEYS( CONFERENCE ),
	M_TWO_KEYS( TRANSFER ),
	M_TWO_KEYS( HOLD ),
	M_TWO_KEYS( CALL_LOG ),
	M_TWO_KEYS( DND ),
	M_TWO_KEYS( OUTVOL_PLUS ),
	M_TWO_KEYS( OUTVOL_MINUS ),
	M_TWO_KEYS( SPEAKER ),
#else
	M_TWO_KEYS( MENU ),
	M_TWO_KEYS( UP ),
	M_TWO_KEYS( DOWN ),
	M_TWO_KEYS( CANCEL ),
	M_TWO_KEYS( TXT_NUM ),
	M_TWO_KEYS( TRANSFER ),
	M_TWO_KEYS( REDIAL ),
	M_TWO_KEYS( OUTVOL_PLUS ),
	M_TWO_KEYS( OUTVOL_MINUS ),
	M_TWO_KEYS( INVOL_PLUS ),
	M_TWO_KEYS( INVOL_MINUS ),
	M_TWO_KEYS( SPEAKER ),
	M_TWO_KEYS( HOLD ),
	M_TWO_KEYS( NET ),
	M_TWO_KEYS( MESSAGE ),
	M_TWO_KEYS( LINE1 ),
	M_TWO_KEYS( LINE2 ),
	M_TWO_KEYS( CONFERENCE ),
	M_TWO_KEYS( BLIND_XFER ),
	M_TWO_KEYS( DND ),
	M_TWO_KEYS( M1 ),
	M_TWO_KEYS( M2 ),
	M_TWO_KEYS( M3 ),
	M_TWO_KEYS( M4 ),
	M_TWO_KEYS( M5 ),
	M_TWO_KEYS( PhoneBook ),	
#endif /* KEYPAD_MAP_VENDOR */
};
#undef VKEY_PhoneBook
#undef M_TWO_KEYS

#define NUM_OF_KEYPAD_KEYS	( sizeof( key_mapping ) / sizeof( key_mapping[ 0 ] ) )

#if KEYPAD_MAP_VENDOR == VENDOR_TCO
#define NUM_OF_SPECIAL_KEYS		1
#else
#define NUM_OF_SPECIAL_KEYS		0
#endif

static void KeypadSignalHandler( int sig );
static int keypad_queue_write( wkey_t wkey );
static int keypad_queue_read( wkey_t *pWKey );
static int keypad_queue_clear( void );

void InitializeKeypad( void )
{	
	/* set signal handler. */
	signal( SIGUSR1, KeypadSignalHandler );
	
	/* Tell keypad driver pid, so that it can send a signal to this one. */
	rtk_SetKeypadSetTarget();

#ifdef _DEBUG_MODE
	/* check if there are redundant VKEY (for development only) */
	#define SIZE_OF_EXT_VKEY	( sizeof( ext_vkey ) / sizeof( ext_vkey[ 0 ] ) )

	int i, j;
	const unsigned char ext_vkey[] = { VKEY_OFF_HOOK, VKEY_ON_HOOK, VKEY_LONG_CLEAR };
	
	for( i = 0; i < NUM_OF_KEYPAD_KEYS - 1 - NUM_OF_SPECIAL_KEYS; i ++ )
		for( j = i + 1; j < NUM_OF_KEYPAD_KEYS - NUM_OF_SPECIAL_KEYS; j ++ )
			if( key_mapping[ i ].vkey == key_mapping[ j ].vkey )
				printf( "Redundant VKEY-'%c' on %d and %d\n", key_mapping[ j ].vkey, i, j );
	
	for( i = 0; i < NUM_OF_KEYPAD_KEYS - NUM_OF_SPECIAL_KEYS; i ++ )
		for( j = 0; j < SIZE_OF_EXT_VKEY; j ++ )
			if( key_mapping[ i ].vkey == ext_vkey[ j ] )
				printf( "Redundant ext. VKEY-'%c' on %d and ext_vkey:%d\n", key_mapping[ j ].vkey, i, j );
				
	#undef SIZE_OF_EXT_VKEY
#endif /* _DEBUG_MODE */
}

void TerminateKeypad( void )
{
	/* default signal handler */
	signal( SIGUSR1, SIG_DFL );

	/* Tell keypad driver to unset target */
	rtk_SetKeypadUnsetTarget();
}

int GetKeypadInput( unsigned char *pKey )
{
	wkey_t wkey;
	unsigned char bLongPress;
	int i;
	
	if( keypad_queue_read( &wkey )== 0 )
		return 0;	/* No key */

	bLongPress = ( unsigned char )( ( wkey & 0x8000 ) >> 8 );
	wkey &= 0x00FF;
	
	for( i = 0; i < NUM_OF_KEYPAD_KEYS; i ++ )
		if( key_mapping[ i ].scan_code == wkey ) {
			*pKey = key_mapping[ i ].vkey | bLongPress;
			return 1;	/* ok, found! */
		}
	
	debug_out( "Key no mapping: %u\n", wkey );
	
	return 0;	/* no mapping ??!! */
}

int ClearKeypadInput( void )
{
	return keypad_queue_clear();
}

int GetKeypadHookStatus( void )		/* 0: off hook, 1: on hook */
{
	return rtk_GetKeypadHookStatus();
}

void HookStatusIsAcknowledgeByUI( int status )	/* status, 0: on-hook, 1: off-hook */
{
	int ret;
	
	ret = rtk_eanblePCM( IP_CHID, ( status ? 1 /* off-hook */ : 0 /* on-hook */ ) );
	
	if( ret )
		debug_out( "HookStatusIsAcknowledgeByUI fail: %d\n", ret );
}

static void KeypadSignalHandler( int sig )
{
	extern int AnnounceKeypadSignalIsArrived( void );
	
	wkey_t wkey;
	
	/* It receive a signal, so there is an input. */
	if( rtk_SetKeypadReadKey( &wkey ) == 0 ) {
		/* place key on buffer */
		keypad_queue_write( wkey );		

		/* announce main loop to retrieve this key */
		if( AnnounceKeypadSignalIsArrived() == 0 )
			debug_out( "Signal handler announce to event handler fail.\n" );
	}
}

/* ************************************************************** */
/*               Keypad queue to store input keys                 */
/* ************************************************************** */
#define NUM_OF_KEYPAD_QUEUE			( 5 + 1 )	/* buffer size: 5 */
static wkey_t keypad_queue[ NUM_OF_KEYPAD_QUEUE ];
static unsigned char keypad_r = 0, keypad_w = 0;

static int keypad_queue_write( wkey_t wkey )
{
	unsigned char next_w;
	
	next_w = ( keypad_w + 1 ) % NUM_OF_KEYPAD_QUEUE;
	
	if( keypad_r == next_w )	/* full */
		return 0;
	
	/* write to queue */
	keypad_queue[ keypad_w ] = wkey;
	keypad_w = next_w;
	
	return 1;
}

static int keypad_queue_read( wkey_t *pWKey )
{
	if( keypad_w == keypad_r )
		return 0;
	
	/* read from queue */
	*pWKey = keypad_queue[ keypad_r ];
	keypad_r = ( keypad_r + 1 ) % NUM_OF_KEYPAD_QUEUE;
	
	return 1;
}

static int keypad_queue_clear( void )
{
	/* 
	 * NOTE: don't assign to zero, because keypad_w owned by signal handler.
	 *       keypad_r used by application.
	 */
	keypad_r = keypad_w;
	
	return 1;
}

