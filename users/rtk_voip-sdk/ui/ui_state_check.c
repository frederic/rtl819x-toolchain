#include <stdio.h>
#include "ui_config.h"
#include "ui_state.h"
#include "ui_state_check.h"
#include "mm_ring.h"

extern ui_state_t uiState;

int StateOnKeyProcessOffHook( void )
{
	// FIXME: prompt state 

	switch( uiState ) {
	case UI_STATE_INCOMING_CALL:
		MM_StopTonePlaying();
		return 1;
	case UI_STATE_DIAL:
		debug_out( "UI_STATE_DIAL should be NOT off-hook.\n" );
		return 1;
	case UI_STATE_OUTGOING_CALL:
	case UI_STATE_IN_CONNECTION:
	case UI_STATE_IN_CONN_DIAL:
	case UI_STATE_INCOMING_CALL_WAIT:
	case UI_STATE_DISCONNECTION_WAIT:
	case UI_STATE_DISCONNECTION:
		return 1;	/* process off-hook by their OnKey() functions. */
	default:
		break;
	}
	
	return 0;
}

int StateOnKeyProcessOnHook( void )
{
	// FIXME: prompt state 

	switch( uiState ) {
	case UI_STATE_INCOMING_CALL:
		MM_StopTonePlaying();
		return 1;
	case UI_STATE_STANDBY:
		debug_out( "UI_STATE_STANDBY should be NOT on-hook.\n" );
		return 1;
	case UI_STATE_INCOMING_CALL_WAIT:
	case UI_STATE_DIAL:
	case UI_STATE_OUTGOING_CALL:
	case UI_STATE_IN_CONNECTION:
	case UI_STATE_IN_CONN_DIAL:
	case UI_STATE_DISCONNECTION_WAIT:
	case UI_STATE_DISCONNECTION:
		return 1;	/* process on-hook by their OnKey() functions. */
	default:
		break;
	}
	
	return 0;
}

int CheckStatesInConnection( void )
{
	// FIXME: prompt state 

	switch( uiState ) {
	case UI_STATE_IN_CONNECTION:
	case UI_STATE_IN_CONN_DIAL:
		return 1;
	default:
		break;
	}
	
	return 0;
}

