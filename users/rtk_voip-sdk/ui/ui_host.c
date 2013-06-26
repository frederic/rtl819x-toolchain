#include <stdio.h>
#include <string.h>
#include "ui_config.h"
#include "ui_buildno.h"
#include "ui_host.h"
#include "ui_call.h"
#include "ui_mode.h"
#include "ui_state_check.h"
#include "ui_flags.h"
#include "ui_datetime.h"
#include "ui_softkey.h"
#include "gs_lib.h"
#include "ioctl_keypad.h"
#include "ioctl_led.h"
#include "ioctl_kernel.h"
#include "mm_ring.h"
#include "mm_sup.h"
#include "mm_led.h"
#include "flash_rw_api.h"

/* flags */
fHostFlags_t fHostFlags;
fTempFlags_t fTempFlags;
fLongJobFlags_t fLongJobFlags;
fCallFlags_t fCallFlags;
fLineFlags_t fLineFlags[ CALL_LINE_NUM ];

CT_ASSERT( sizeof( fHostFlags ) == sizeof( fHostFlags.all ) );
CT_ASSERT( sizeof( fTempFlags ) == sizeof( fTempFlags.all ) );
CT_ASSERT( sizeof( fLongJobFlags ) == sizeof( fLongJobFlags.all ) );
CT_ASSERT( sizeof( fCallFlags ) == sizeof( fCallFlags.all ) );
CT_ASSERT( sizeof( fLineFlags[ 0 ] ) == sizeof( fLineFlags[ 0 ].all ) );

#if CALL_LINE_NUM != 2
???
#endif

/* state */
extern const ui_state_context_t uiStateContext[];
ui_state_t uiState;

/* state stack */
#define UI_STACK_SIZE		20

static struct {
	ui_state_t state[ UI_STACK_SIZE ];
	int help[ UI_STACK_SIZE ];
	int index;
} uiStateStack;

static union {
	unsigned char szDatetime[ SHORT_DATETIME_STRING_LEN + 1 ];
} sh_host;

/* prompt */
static int nParamBackFromPrompt;
static ui_state_t uiStateBackFromPrompt;
static uptime_t nTimeBackFromPrompt;

/* long job timer */
static uptime_t nTimeLastKeypress;

/* menu selection */
int nSelectedItem;

extern int KeyOwnInputMethodEditor( unsigned char key );
extern int KeyOwnByTextEditor( unsigned char key );
extern int KeyOwnByMenuSelection( unsigned char key );

extern void InputMethodEditorTimerEvent( void );
extern void MenuSelectionTimerEvent( void );

static inline void DoLongJobsIfLongIdlePeriod( void );

static void RefreshStandbyFrame( int param );

void UI_BasicInitialization( void )
{
	extern void PhonebookBasicInitialization( int bClearContent );
	extern int CheckFlashVersionFeatureSize( void );
	extern void WriteFlashVersionFeatureSize( void );
	extern void LoadSettingsFromStorage( int bLoadDefault );
	extern void LoadModeSettingsFromStorage( int bLoadDefault );
	extern void LoadCallRecordsFromFlash( int bDefault );

	GS_Initialization();
	
	MM_Initialize();

	/* Initialize flags */
	fHostFlags.all = 0;
	fTempFlags.all = 0;
	fLongJobFlags.all = 0;
	fCallFlags.all = 0;
	fLineFlags[ 0 ].all = 0;
	fLineFlags[ 1 ].all = 0;
	fModeFlags.all = 0;
		
	/* check flash version, feature and size */
	if( CheckFlashVersionFeatureSize() ) {
		/* load old settings */
		PhonebookBasicInitialization( 0 );
		
		LoadSettingsFromStorage( 0 );
		
		LoadModeSettingsFromStorage( 0 );

		LoadCallRecordsFromFlash( 0 );
	} else {
		/* reset to default settings */
		debug_out( "Check version fail, so use default settings.\n" );
		
		PhonebookBasicInitialization( 1 );
		
		LoadSettingsFromStorage( 1 /* Load default */ );
		
		LoadModeSettingsFromStorage( 1 /* Load default */ );

		LoadCallRecordsFromFlash( 1 /* default */ );

		WriteFlashVersionFeatureSize();		/* keep it to be last one */

		FlashValidateWriting( 1 );
	}

	/* Initalize state */
	uiState = UI_STATE_INIT;
	uiStateStack.index = 0;

	/* Hardware status */
	HookStatusIsAcknowledgeByUI( 0 /* on-hook */ );	/* prevent re-enable PCM */

	if( ( fHostFlags.b.hookStatus = ( GetKeypadHookStatus() ? 1 : 0 ) ) )
		HookStatusIsAcknowledgeByUI( 1 /* off-hook */ );	/* prevent re-enable PCM */

#if 0		
	/* Disable FXO relay */
	API_SwitchLineToFXO( IP_SID_TEMP, 0 /* enable */, 0 /* not connected */ );
#endif

	/* retrieve kernel build information */
	rtk_RetrieveMiscInfo( &buildno_kernel, &builddate_kernel );

	/* OK. Start to standby */
	if( fHostFlags.b.hookStatus ) {
		UI_StateTransition( UI_STATE_STANDBY, PARAM_NOT_DRAW );
		DoEditDialNumberWithStateTransition( 0 /* not handfree */, 0 );
	} else
		UI_StateJump( UI_STATE_STANDBY, 0 );
}

void UI_Termination( void )
{
	MM_Terminate();

	GS_Termination();
}

static void DoStateTrnasition( int param )
{
	/* Reset to 'clean' status */
	GS_SetCursorStatus( CURSOR_STATUS_OFF_NODRAW );
	DeactivateInputMethodEditor();
	DeactivateTextEditor();
	DeactivateMenuSelection();

	/* Reset softkey */
	if( fHostFlags.b.promptSoftkey ) {
		fHostFlags.b.promptSoftkey = 0;
		
		if( uiState == UI_STATE_PROMPT )
			goto label_do_not_reset_softkey;
	} 

	ResetSoftkeyConfiguration();

label_do_not_reset_softkey:

	/* Draw frame */
	( *uiStateContext[ uiState ].pFnSetupFrame )( param );
	
	/* Keep it to be last one, because tiemr should be depend on settings. */
	ChangeTimerPeriodForApplication();
}

void UI_StateJump( ui_state_t newState, int param )
{
	uiState = newState;
	uiStateStack.index = 0;
	
	DoStateTrnasition( param );	
}

void UI_StateBack( state_back_level_t level )
{
	int param;

	param = uiStateContext[ uiState ].nCancelKeyBackParam;

	if( level > uiStateStack.index ) {
		printf( "No so much states in stack.\n" );
		uiStateStack.index = 0;
	} else 
		uiStateStack.index -= level;

	uiState = uiStateStack.state[ uiStateStack.index ];

	DoStateTrnasition( param );
}

void UI_StateBackButLikeOneLevel( state_back_level_t level )
{
	if( level > uiStateStack.index ) {
		printf( "No so much states in stack.\n" );
		uiStateStack.index = 0;
	} else 
		uiStateStack.index -= level;

	uiState = uiStateStack.state[ uiStateStack.index ];

	UI_StateBack( STATE_BACK_LEVEL_ONE );
}

void UI_PeekStateStack( state_back_level_t level, ui_state_t *state, int *help )
{
	int index;

	index = uiStateStack.index - level;		/* level >= 1 */

	if( index < 0 ) {
		printf( "Too deep to peek.\n" );
		return;
	} 

	*state = uiStateStack.state[ index ];
	*help = uiStateStack.help[ index ];
}

void UI_StateTransitionWithBackHelp( ui_state_t newState, int param, int help )
{
	int i;

	for( i = uiStateStack.index - 1; i >= 0; i -- )
		if( uiStateStack.state[ i ] == newState ) {
			uiStateStack.index = i;
			goto label_stack_process_done;
		}

	if( uiStateStack.index >= UI_STACK_SIZE )
		printf( "Out of uiStateStack\n" );
	else {
		uiStateStack.state[ uiStateStack.index ] = uiState;
		uiStateStack.help[ uiStateStack.index ] = help;

		uiStateStack.index ++;
	}
	
label_stack_process_done:
	uiState = newState;
	
	DoStateTrnasition( param );
}

void UI_StateTransition( ui_state_t newState, int param )
{
	UI_StateTransitionWithBackHelp( newState, param, 0 /* ignore */ );
}

void UI_StateTransitionToPrompt( ui_state_t nextState, int paramForNextState )
{
	uiStateBackFromPrompt = nextState;
	nParamBackFromPrompt = paramForNextState;
	nTimeBackFromPrompt = GetUptimeInMillisecond();

	UI_StateTransition( UI_STATE_PROMPT, 0 );
}

static void EndOfPromptState( void )
{
	UI_StateTransition( uiStateBackFromPrompt, nParamBackFromPrompt );
}

void UI_KeypadInput( unsigned char key )
{	
/*	
	if( key == '1' ) {
		GS_SetCursorPosition( 3, 0 );
		GS_ChangeCursorStatus( CURSOR_STATUS_ON );
	} else if( key == '2' ) {
		GS_SetCursorPosition( 4, 0 );
		GS_ChangeCursorStatus( CURSOR_STATUS_SQUARE );
	} else if( key == '3' ) {
		GS_SetCursorPosition( 5, 0 );
	}
*/	

	/* Keys translation and deny */
	if( key == VKEY_HOOK ) {	/* Hook status translation */
		if( GetKeypadHookStatus() ) {
			if( fHostFlags.b.hookStatus )
				return;	/* deny */
			else {
				fHostFlags.b.hookStatus = 1;
				key = VKEY_OFF_HOOK;
				HookStatusIsAcknowledgeByUI( 1 /* off-hook */ );
			}
		} else {
			if( !fHostFlags.b.hookStatus )
				return;	/* deny */
			else {
				fHostFlags.b.hookStatus = 0;
				key = VKEY_ON_HOOK;
				HookStatusIsAcknowledgeByUI( 0 /* on-hook */ );
			}
		}
	} else if( key & 0x80 ) {	/* Long-pressed key translation */
		if( key == ( VKEY_BACKSPACE | 0x80 ) )
			key = VKEY_LONG_CLEAR;
		else
			return;		/* deny */
	}
	
	/* the last key press time for long job */
	nTimeLastKeypress = GetUptimeInMillisecond();

	if( key & 0x80 )	
		debug_out( "UI Long Press Key: %c\n", key & 0x7F );
	else
		debug_out( "UI Key: %c\n", key );
	
	/* Play key press tone */
	if( fHostFlags.b.keyStopTonePluse ) {
		if( key == VKEY_ON_HOOK || key == VKEY_OFF_HOOK || 
			key == VKEY_OUTVOL_PLUS || key == VKEY_OUTVOL_MINUS )
		{
			;
		} else {
			fHostFlags.b.keyStopTonePluse = 0;
			MM_StopTonePlaying();
		}
	}
		
	if( CheckIfPlayKeypressTone() ) {
		PlayKeypressTone( key );
	}

	/* Softkey translation */
	DoSoftkeyTranslation( &key );
	
	/* Key own by IME */
	if( KeyOwnInputMethodEditor( key ) )
		return;
	
	/* Key own by text editor */
	if( KeyOwnByTextEditor( key ) )
		return;
	
	/* Key own by menu selection */
	if( KeyOwnByMenuSelection( key ) )
		return;
	
	/* Process keys automatically */
	if( key == VKEY_CANCEL ) {			/* 'cancel' key */
		switch( uiStateContext[ uiState ].nCancelKeyAct ) {
		case CANCEL_KEY_ACT_BACK_ONE_LEVEL:
			UI_StateBack( STATE_BACK_LEVEL_ONE /* one level */ );
			return;
		case CANCEL_KEY_ACT_STANDBY:
			UI_StateTransition( UI_STATE_STANDBY, 0 );
			return;
		default:
			break;
		}
	} else if( key == VKEY_OFF_HOOK ) {	/* off-hook key */
		if( !StateOnKeyProcessOffHook() ) {
			DoEditDialNumberWithStateTransition( 0 /* not handfree */, 0 );
			return;
		}
	} else if( key == VKEY_ON_HOOK ) {	/* on-hook key */
		if( !StateOnKeyProcessOnHook() ) {
			UI_StateTransition( UI_STATE_STANDBY, 0 );
			return;
		}
	}

	( *uiStateContext[ uiState ].pFnOnKey )( key );
}

void UI_TimerEvent( void )
{
	extern void CheckDisconnectedPromptTimeoutInConnection( void );
	extern void RefreshAllLinesInfoInConnection( void );
	extern void RefreshAllLinesTimerOfInfoInConnection( void );
	extern void DisconnectPromptTimerEvent( void );
	extern int TimerEventPlayAnimation( void );
	extern void MM_TimerEvent( void );
	extern void DialStateTimerEvent( void );
	extern void IncomingCallStateTimerEvent( void );
	extern void TestCaseTextVerticalTimer( void );
	extern void TestCaseTextHorizontalTimer( void );

	MM_TimerEvent();
	
	/* long jobs check */
	DoLongJobsIfLongIdlePeriod();
	
	switch( uiState ) {		
	case UI_STATE_PROMPT:
		if( CheckIfTimeoutInMillisecond( &nTimeBackFromPrompt, SHOW_PROMPT_PERIOD ) == 0 )
			EndOfPromptState();
		break;

	case UI_STATE_STANDBY:
		RefreshStandbyFrame( PARAM_SPECIAL );
		break;
		
	case UI_STATE_DIAL:
		DialStateTimerEvent();
		break;

	case UI_STATE_INCOMING_CALL:
		IncomingCallStateTimerEvent();
		break;
		
	case UI_STATE_IN_CONNECTION:
		CheckDisconnectedPromptTimeoutInConnection();
		RefreshAllLinesInfoInConnection();
		MM_LedBlinkingTimerEvent( 0 /* By timer */ );
		TimerEventPlayAnimation();
		break;

	case UI_STATE_IN_CONN_DIAL:
		RefreshAllLinesTimerOfInfoInConnection();
		MM_LedBlinkingTimerEvent( 0 /* By timer */ );
		break;

	case UI_STATE_DISCONNECTION:
		DisconnectPromptTimerEvent();
		break;

	case UI_STATE_OUTGOING_CALL:
		TimerEventPlayAnimation();
		break;

	case UI_STATE_MENU_VIEW_PING_WAIT:
		if( fHostFlags.b.pingRequest )
			TimerEventPlayAnimation();
		break;

	case UI_STATE_MENU_TEST_CASE_TEXT_VERTICAL:
		TestCaseTextVerticalTimer();
		break;

	case UI_STATE_MENU_TEST_CASE_TEXT_HORIZONTAL:
		TestCaseTextHorizontalTimer();
		break;
	
	default:
		break;
	}

	/* Cursor timer event */
	GS_CursorBlinkTimerEvent();
	
	/* IME timer event */
	InputMethodEditorTimerEvent();

	/* Menu selection timer event */
	MenuSelectionTimerEvent();
}

static inline void DoLongJobsIfLongIdlePeriod( void )
{	
	if( CheckStatesInConnection() )
		;	/* do nothing in connection state */
	else if( fLongJobFlags.all && 
			 CheckIfTimeoutInMillisecond( &nTimeLastKeypress, IDLE_PERIOD_TO_DO_LONG_JOB ) == 0 ) 
	{
		/* flash write */
		if( fLongJobFlags.b.flashWrite ) {
			fLongJobFlags.b.flashWrite = 0;
			FlashValidateWriting( 1 );
		}
	}
}

int CheckIfPlayKeypressTone( void ) 
{
	if( fModeFlags.b.keypressTone ) {
		if( fHostFlags.b.hookStatus || fHostFlags.b.handfree )
			return 1;
	}

	return 0;
}

void API_SipRegisterToUI( register_status_t status )
{
	switch( status ) {
	case SIP_NOT_REGISTER:	/* no register account */
		fHostFlags.b.sipDisplay = 0;
		fHostFlags.b.sipRegister = 0;
#ifdef LED_BIT_VOIP1
		TurnOffLEDThroughGPIO( LED_BIT_VOIP1 );
#endif
		break;
		
	case SIP_REGISTERING:	/* try to register */
		fHostFlags.b.sipDisplay = 1;
		fHostFlags.b.sipRegister = 0;
#ifdef LED_BIT_VOIP1
		TurnOffLEDThroughGPIO( LED_BIT_VOIP1 );
#endif
		break;
		
	case SIP_REGISTERED:	/* registered */
		fHostFlags.b.sipDisplay = 1;
		fHostFlags.b.sipRegister = 1;
#ifdef LED_BIT_VOIP1
		TurnOnLEDThroughGPIO( LED_BIT_VOIP1 );
#endif
		break;
	}
	
	/* refresh standby frame */
	if( uiState == UI_STATE_STANDBY )
		RefreshStandbyFrame( PARAM_SPECIAL );
}

void SetupDummyFrame( int param )
{
	/* do nothing */
}

void StateDummyOnKey( unsigned char key )
{
	/* do nothing */
}

void SetupInitFrame( int param )
{
}

void StateInitOnKey( unsigned char key )	/* UI_STATE_INIT */
{
}

static void DrawCurrentDateTime( int param, rect_t *pInvalidateRect )
{
	datetime_t dtNow;
	unsigned char szNow[ SHORT_DATETIME_STRING_LEN + 1 ];
	int x, len;
	
	dtNow = GetCurrentDateTime();
	
	MakeShortDateTimeString( szNow, dtNow );

	len = strlen( ( const char * )szNow );

	if( param == PARAM_SPECIAL ) {	// only update different part 
		for( x = 0; x < len; x ++ )
			if( sh_host.szDatetime[ x ] != szNow[ x ] )
				break;
	} else
		x = 0;

	strcpy( ( char * )&sh_host.szDatetime[ x ], ( const char * )&szNow[ x ] );
	
	if( x < len ) {
		GS_TextOut( x, 0, szNow + x, len - x );

		pInvalidateRect ->left = x * CHAR_WIDTH;
		pInvalidateRect ->top = 0;
		pInvalidateRect ->right = len * CHAR_WIDTH;
		pInvalidateRect ->bottom = CHAR_HEIGHT;
	} else {
		pInvalidateRect ->left = 0;
		pInvalidateRect ->top = 0;
		pInvalidateRect ->right = 0;
		pInvalidateRect ->bottom = 0;
	}
}

static void RefreshStandbyFrame( int param )
{
	rect_t rect;

	if( param == PARAM_SPECIAL )
		GS_DrawOffScreen();
	else {
		GS_DrawOffScreenAndClearScreen();

		LeftSoftkeyConfigurationWithInstructionText( VKEY_MENU, szInsMenu );
		RightSoftkeyConfigurationWithInstructionText( VKEY_PHONEBOOK, szInsPhonebook );
	}

#if 1
	DrawCurrentDateTime( param, &rect );
#else
	GS_TextOut( 0, 0, szStandbyPrompt, RES_strlen( szStandbyPrompt ) );
#endif

	if( fHostFlags.b.sipDisplay ) {
		if( fHostFlags.b.sipRegister )
			GS_TextOut( 0, 1, szSipRegister, RES_strlen( szSipRegister ) );
		else
			GS_TextOut( 0, 1, szSipNotRegister, RES_strlen( szSipNotRegister ) );

		GS_DrawOnScreen( NULL );

		return;
	}
	
	if( param == PARAM_SPECIAL )
		GS_DrawOnScreen( &rect );
	else
		GS_DrawOnScreen( NULL );

#if 0
	{	/* Test LED */
		static int a = 0;

		if( a == 0 ) {
			TurnOffLEDThroughGPIO( LED_MASK_ALL );
			TurnOnLEDThroughGPIO( LED_MASK_RED );
			TurnOffLEDThroughGPIO( LED_MASK_GREEN );

			a ++;
		} else if( a == 1 ) {
			TurnOnLEDThroughGPIO( LED_MASK_GREEN );
			TurnOffLEDThroughGPIO( LED_MASK_RED );

			a ++;
		} else {
			TurnOnLEDThroughGPIO( LED_MASK_ALL );

			a = 0;
		}

	}
#endif
}

void SetupStandbyFrame( int param )
{
	if( param == PARAM_NOT_DRAW )
		goto label_draw_frame_done;

	RefreshStandbyFrame( param );

label_draw_frame_done:	
	/* Turn off tone */
	MM_StopTonePlaying();
	
	/* Set sip state to be ideal. */
	API_ReleaseLineForCall();

	/* Turn off handfree */
	EnableHandfreeFunctionalityAndFlag( 0 /* disable */ );
	
	/* clear call flags */
	fCallFlags.all = 0;
	fLineFlags[ 0 ].all = 0;
	fLineFlags[ 1 ].all = 0;
}

void StateStandbyOnKey( unsigned char key )	/* UI_STATE_STANDBY */
{
	if( key == VKEY_MENU )
		UI_StateTransition( UI_STATE_MENU_ACT, 0 );
	else if( key == VKEY_PHONEBOOK )
		UI_StateTransition( UI_STATE_PHONEBOOK_LIST, 0 );
	else if( key == VKEY_UP )
		UI_StateTransition( UI_STATE_CALL_RECORD_OUTGOING, 0 );
	else if( key == VKEY_MISSED )
		UI_StateTransition( UI_STATE_CALL_RECORD_MISSED, 0 );
	else if( key == VKEY_SPEAKER )
		DoEditDialNumberWithStateTransition( 1 /* handfree */, 0 );
#if 0
	else if( key == VKEY_OFF_HOOK ) {
		/* Allocate a line for outgoing call */
		DoEditDialNumberWithStateTransition( 0 /* not handfree */, 0 );
	} 
#endif
	else if( IsDialKey( key ) ) {
		/* Allocate a line for outgoing call */
		DoEditDialNumberWithStateTransition( 1 /* handfree */, key );
	} 

#if 0
	// for test only 
	else
	if( key == '1' ) {
		GS_SetCursorPosition( 3, 0 );
		GS_SetCursorStatus( CURSOR_STATUS_ON );
	} else if( key == '2' ) {
		GS_SetCursorPosition( 4, 0 );
		GS_SetCursorStatus( CURSOR_STATUS_SQUARE );
	} else if( key == '3' ) {
		GS_SetCursorPosition( 5, 0 );
	}
#endif
}

void DrawCentralizedString( const unsigned char *pszPromptText )
{
	int len, y = 0, x, pos_nl;
	unsigned char ch;

	/* search for '\n' */
	for( pos_nl = 0; ; pos_nl ++ ) {
		ch = pszPromptText[ pos_nl ];
		if( ch == '\n' )
			break;
		else if( ch == '\x0' ) {
			pos_nl = 0;		/* 0 to indicate no '\n' */
			break;
		}
	}

	if( pos_nl > 0 && pos_nl <= VRAM_WIDTH_IN_TEXT_UNIT ) {
		x = ( VRAM_WIDTH_IN_TEXT_UNIT - pos_nl ) / 2;
		GS_TextOut( x, y, pszPromptText, pos_nl );

		pszPromptText += pos_nl + 1;
		y ++;
	} 

	while( y < VRAM_HEIGHT_IN_TEXT_UNIT ) {

		len = strlen( ( const char * )pszPromptText );
		
		if( len > VRAM_WIDTH_IN_TEXT_UNIT ) {
			GS_TextOut( 0, y, pszPromptText, VRAM_WIDTH_IN_TEXT_UNIT );
			pszPromptText += VRAM_WIDTH_IN_TEXT_UNIT;
		} else {
			x = ( VRAM_WIDTH_IN_TEXT_UNIT - len ) / 2;
			GS_TextOut( x, y, pszPromptText, len );
			break;
		}

		y ++;
	}
}

void SetupFrameWithCentralizedString( const unsigned char *pszPromptText )
{
	GS_DrawOffScreenAndClearScreen();
	GS_SetCursorStatus( CURSOR_STATUS_OFF_NODRAW );
	
	DrawCentralizedString( pszPromptText );

	GS_DrawOnScreen( NULL );
}

void SetupFrameWithCentralizedStringAndSoftkey( const unsigned char *pszPromptText, std_skm_t std_skm )
{
	GS_DrawOffScreenAndClearScreen();
	GS_SetCursorStatus( CURSOR_STATUS_OFF_NODRAW );
	
	DrawCentralizedString( pszPromptText );

	if( std_skm & 0x80000000 ) {
		switch( std_skm ) {
		case SOFTKEY_LR_OK_BACK:
			SoftkeyStandardConfiguration_OkBack();
			break;
		case SOFTKEY_LR_OK_CLEAR:
			SoftkeyStandardConfiguration_OkClear();
			break;
		case SOFTKEY_LR_NONE_BACK:
			SoftkeyStandardConfiguration_Back();
			break;
		case SOFTKEY_LR_YES_NO:
			SoftkeyStandardConfiguration_YesNo();
			break;
		}
	} else {
		if( std_skm & SOFTKEY_L_OK )
			LeftSoftkeyConfigurationWithInstructionText( VKEY_OK, szInsOK );

		if( std_skm & SOFTKEY_R_BACK )
			RightSoftkeyConfigurationWithInstructionText( VKEY_CANCEL, szInsBack );
		else if( std_skm & SOFTKEY_R_CLEAR )
			RightSoftkeyConfigurationWithInstructionText( VKEY_CLEAR, szInsClear );
	}

	GS_DrawOnScreen( NULL );
}

void SetupPromptFrame( const unsigned char *pszPromptText )
{
	/* Reset softkey */
	ResetSoftkeyConfiguration();

	/* Draw prompt frame */
	GS_DrawOffScreenAndClearScreen();
	GS_SetCursorStatus( CURSOR_STATUS_OFF_NODRAW );
	
	DrawCentralizedString( pszPromptText );

	RightSoftkeyConfigurationWithInstructionText( VKEY_CANCEL, szInsBack );

	GS_DrawOnScreen( NULL );

	/* set flag for DoStateTransition() */
	fHostFlags.b.promptSoftkey = 1;
}

void StatePromptOnKey( unsigned char key )	/* UI_STATE_PROMPT */
{
	if( key == VKEY_CANCEL )
		EndOfPromptState();
}

