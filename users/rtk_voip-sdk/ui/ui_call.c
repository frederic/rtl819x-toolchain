#include <stdio.h>
#include <string.h>
#include "ui_config.h"
#include "ui_call.h"
#include "ui_host.h"
#include "ui_flags.h"
#include "ui_mode.h"
#include "ui_ansi_color.h"
#include "gs_lib.h"
#include "ui_limits.h"
#include "ui_state_check.h"
#include "mm_ring.h"
#include "mm_animate.h"
#include "mm_sup.h"
#include "mm_led.h"
#include "ioctl_led.h"
#include "ui_records.h"
#include "ui_datetime.h"
#include "ui_softkey.h"
#include "flash_layout.h"
#include "flash_rw_api.h"

static void DoOutgoingCallWithStateTransition( sid_t sid, const unsigned char *pszPhonenumber, ocf_t ocf );
#if 0
static void DoFxoSwitchWithStateTransition( sid_t sid, unsigned int bInConnection );
#endif
static void DisplayLedInConnectionRelyOnFlags( void );
static int InConnectionLineSwitching( lsr_t lsr );
static void ReadyToStartTimerInConnection( sid_t sid );

typedef struct call_line_s {
	//unsigned long nSecConnectionBase;	/* If do hold/resume, save the elapse time */
	unsigned long nSecConnectionStart;
	unsigned long nSecConnectionLast;
	unsigned char szDisplayTime[ 9 ];	/* 00:00:00 */
	unsigned char szCalledPhonenumber[ MAX_LEN_OF_PHONENUMBER ];
	datetime_t dtIncomingCall;	/* datetime of incoming call */
	uptime_t nDiscPromptTimer;
} call_line_t;

static struct {
	call_line_t line[ CALL_LINE_NUM ];
	uptime_t nVolumePromptTimer;
} sh;

static union {
	uptime_t nDisconnectTimer;		/* UI_STATE_DISCONNECTION */
	struct {
		unsigned long msAutoDialTimeThres;
		uptime_t nAutoDialTimer;
		unsigned long msOffHookAlarmThres;
		uptime_t nOffHookAlarmTimer;
		uptime_t nVolumePromptDialTimer;
		int nEditPhonenumberLength;
		unsigned char szEditPhonenumber[ MAX_LEN_OF_PHONENUMBER ];
	} dial;							/* UI_STATE_DIAL / UI_STATE_IN_CONN_DIAL */
	struct {
		unsigned long msAutoAnswerTimeThres;
		uptime_t nAutoAnswerTimer;
	} incomingCall;					/* UI_STATE_INCOMING_CALL */
} sht;

static int DoHotLineOutgoingCallWithStateTransition( void )
{
	unsigned char szHotLine[ MAX_LEN_OF_PHONE_NUMBER + 1 ];
	unsigned char bcdHotLine[ BCD_LEN_OF_PHONE_NUMBER ];

	/* enable ? */
	if( !fModeFlags.b.hotLine )
		return 0;

	/* read hot line phonenumber */
	FlashReadData( MY_FIELD_OFFSET( ui_falsh_layout_t, mode.bcdHotLine ),
				bcdHotLine, sizeof( bcdHotLine ) );

	if( GetTextStringFromBcd( szHotLine, MAX_LEN_OF_PHONE_NUMBER, bcdHotLine ) == 0 ) {
		debug_out( "Not hot line phonenumber??\n" );
		return 0;
	}

	/* allocate phone call */
	API_AllocateLineForCall( IP_SID_INIT, 0 /* connected */ );

	DoOutgoingCallWithStateTransition( IP_SID_TEMP, szHotLine, OCF_NORMAL );

	return 1;
}

void DoEditDialNumberWithStateTransition( int bHandfree, unsigned char key )
{
	if( bHandfree ) {
		EnableHandfreeFunctionalityAndFlag( 1 /* enable */ );
	} else {
		EnableHandfreeFunctionalityAndFlag( 0 /* disable */ );
	}

	if( DoHotLineOutgoingCallWithStateTransition() )
		;
	else 
		UI_StateTransition( UI_STATE_DIAL, ( int )key );
}

static int RefreshAdjustOutVolumeDialFrame( unsigned char key )
{
	int ret;
	inout_vol_type_t type;
	
	/* decide type by flags */
	if( fHostFlags.b.handfree ) {
		type = INOUTVOL_TYPE_SPEAKER;
	} else {
		type = INOUTVOL_TYPE_RECEIVER;
	}
	
	/* ok refresh frame */
	sht.dial.nVolumePromptDialTimer = GetUptimeInMillisecond();
	ret = RefreshAdjustOutVolumeXxxFrame( fCallFlags.b.volumePromptDial, 
										  key, 
										  1 /* refresh hardware */, 
										  type );
	fCallFlags.b.volumePromptDial = 1;

	return ret;
}

static inline void EndOfAdjustVolumeDialReturnDialFrame( void )
{
	extern void SetupDialFrame( int param );

	fCallFlags.b.volumePromptDial = 0;

	SetupDialFrame( PARAM_VOL_BACK );
}

static inline int VolumeKeyProcessInDialState( unsigned char key )
{
	if( key == VKEY_OUTVOL_PLUS || key == VKEY_OUTVOL_MINUS ) {

		if( !fCallFlags.b.volumePromptDial ) {
			/* save text content */
			sht.dial.nEditPhonenumberLength = 
				GetTextEditorTextContent( sht.dial.szEditPhonenumber, MAX_LEN_OF_PHONENUMBER );

			DeactivateTextEditor();
		}

		RefreshAdjustOutVolumeDialFrame( key );

		return 1;
	} else {
		/* cancel volume adjust screen */
		if( fCallFlags.b.volumePromptDial ) {
#if 0
			if( IsDialKey( key ) && 
				sht.dial.nEditPhonenumberLength < MAX_LENGTH_OF_SINGLE_LINE_EDITOR ) 
			{
				/* Append current key. */
				/* In normal case, this action is processed by editor. */
				sht.dial.szEditPhonenumber[ sht.dial.nEditPhonenumberLength ++ ] = key;
				sht.dial.szEditPhonenumber[ sht.dial.nEditPhonenumberLength ] = '\x0';
			}
#endif

			EndOfAdjustVolumeDialReturnDialFrame();

#if 1
			KeyOwnByTextEditor( key );
#endif
		}
	}

	return 0;
}

void SetupDialFrame( int param )
{
	params_for_single_line_editor_t editor_params;
	unsigned char key = '\x0';

	GS_DrawOffScreenAndClearScreen();

	GS_TextOut( 0, 0, szDialNumberPrompt, RES_strlen( szDialNumberPrompt ) );

	if( param == 0 || param == PARAM_PROMPT_BACK || param == PARAM_BORROW ) {
		/* param == PARAM_PROMPT_BACK --> input empty string */
		/* param == PARAM_BORROW -> UI_STATE_IN_CONN_DIAL */
		editor_params.nDefaultTextLength = 0;
	} else if( param == PARAM_VOL_BACK ) {
		/* param == PARAM_PROMPT_BACK --> adjust volume */
		editor_params.nDefaultTextLength = sht.dial.nEditPhonenumberLength;
		editor_params.pszDefaultText = sht.dial.szEditPhonenumber;
	} else {
		key = param;
		editor_params.nDefaultTextLength = 1;
		editor_params.pszDefaultText = ( unsigned char * )&key;
	}
	editor_params.nMaxTextLength = 0;	/* use default value */
	editor_params.tTextType = TEXT_TYPE_IP;
	editor_params.fParams.all = 0;
	editor_params.fParams.b.keyBypass = 1;	/* bypass key */

	ActivateTextEditor( EDITOR_ID_SINGLE_LINE, ( params_for_editor_t * )&editor_params );

	LeftSoftkeyConfigurationWithInstructionText( VKEY_OK, szInsDial );
	if( param == PARAM_BORROW )
		RightSoftkeyConfigurationWithInstructionText( VKEY_CANCEL, szInsBack );

	GS_DrawOnScreen( NULL );	
	GS_SetCursorStatus( CURSOR_STATUS_ON );
	
	/* If back from prompt, it is not necessary to do followings. */
	if( param == PARAM_PROMPT_BACK || param == PARAM_VOL_BACK )
		goto label_draw_dial_frame_done;
	
	/* Allocate a line for outgoing call */
	if( param != PARAM_BORROW )
		API_AllocateLineForCall( IP_SID_INIT, 0 /* connected */ );
		
	/* Play dial tone */
	if( key ) {
		if( CheckIfPlayKeypressTone() ) 
			PlayKeypressTone( key );
	} else {
		MM_StartTonePlaying( TONE_ID_DIAL, 0 /* repeat */ );
		
		fHostFlags.b.keyStopTonePluse = 1;	/* any key to stop it */
	}

	/* load auto dial time */
	sht.dial.msAutoDialTimeThres = ReadAutoDialTimeInMillisecond();
	sht.dial.nAutoDialTimer = GetUptimeInMillisecond();

	/* off-hook alarm time */
	sht.dial.msOffHookAlarmThres = ReadOffHookAlarmTimeInMillisecond();
	sht.dial.nOffHookAlarmTimer = GetUptimeInMillisecond();

	/* volume prompt */
	fCallFlags.b.volumePromptDial = 0;

label_draw_dial_frame_done:
	;
}

void StateDialOnKey( unsigned char key )	/* UI_STATE_DIAL */
{
	/* Cancel key is processed by UI_KeypadInput() */
	/* NOTE: We bypass the keys owned by editor. */
	unsigned char szDial[ MAX_LEN_OF_PHONENUMBER + 1 ];	

	if( VolumeKeyProcessInDialState( key ) )
		goto label_key_process_done;
	/* No 'else' statement here. */

	if( key == VKEY_OK || key == VKEY_POUND ) {
		if( GetTextEditorTextContent( szDial, MAX_LEN_OF_PHONENUMBER ) ) {
label_dial_text_ready__do_dial_out:
			DoOutgoingCallWithStateTransition( IP_SID_TEMP, szDial, OCF_NORMAL );
		} else {
			SetupPromptFrame( szEmptyIsNotAllow );
			UI_StateTransitionToPrompt( UI_STATE_DIAL, PARAM_PROMPT_BACK );
		}
	} else if( key == VKEY_ON_HOOK ) {
label_leave_dial_state:
		UI_StateTransition( UI_STATE_STANDBY, 0 );
	} else if( key == VKEY_REDIAL ) {
		if( GetTheLastOutgoingCallRecord( szDial ) ) 
			goto label_dial_text_ready__do_dial_out;
	} else if( key == VKEY_SPEAKER ) {
		if( fHostFlags.b.handfree ) {
			EnableHandfreeFunctionalityAndFlag( 0 /* disable */ );
			goto label_leave_dial_state;
		}
	} else if( key == VKEY_OFF_HOOK ) {
		if( fHostFlags.b.handfree )
			EnableHandfreeFunctionalityAndFlag( 0 /* disable */ );
	} else if( IsDialKey( key ) ) {
		/* NOTE: these keys were processed by editor */
		sht.dial.msOffHookAlarmThres = 0;	/* turn off off-hook alarm time */
	} else if( key == VKEY_FXO ) {
		//DoFxoSwitchWithStateTransition( IP_SID_INIT, 0 /* not in connection */ );
		DoOutgoingCallWithStateTransition( IP_SID_INIT, szNull, OCF_FXO );
	}

label_key_process_done:
	/* refresh auto dial timer */
	sht.dial.nAutoDialTimer = GetUptimeInMillisecond();
}

void DialStateTimerEvent( void )
{
	/* check auto dial time */
	if( fModeFlags.b.autoDial &&
		sht.dial.msAutoDialTimeThres != 0 &&
		CheckIfTimeoutInMillisecond( &sht.dial.nAutoDialTimer, 
									sht.dial.msAutoDialTimeThres ) == 0 )
	{
		if( GetTextEditorTextContent( NULL, 0 /* ignore */ ) ) {
			StateDialOnKey( VKEY_OK );
			return;
		}
	}

	/* check off-hook alarm time */
	if( fModeFlags.b.offHookAlarm &&
		sht.dial.msOffHookAlarmThres &&
		CheckIfTimeoutInMillisecond( &sht.dial.nOffHookAlarmTimer,
									 sht.dial.msOffHookAlarmThres ) == 0 )
	{
		UI_StateTransition( UI_STATE_DIAL_TIMEOUT, 0 );
		return;
	}

	/* check volume prompt time */
	if( fCallFlags.b.volumePromptDial ) {
		if( CheckIfTimeoutInMillisecond( &sht.dial.nVolumePromptDialTimer,
										 2000 /* 2 seconds */ ) == 0 )
		{
			EndOfAdjustVolumeDialReturnDialFrame();
			return;
		}
	}
}

void SetupDialTimeoutFrame( int param )
{
	rect_t rect;

	/* Play busy tone */
	MM_StartTonePlaying( TONE_ID_BUSY, 0 /* repeat */ );

	GS_DrawOffScreen();

	ClearSoftkeyInstructionTextRetangle( &rect );

	GS_DrawOnScreen( &rect );
}

void StateDialTimeoutOnKey( unsigned char key )	/* UI_STATE_DIAL_TIMEOUT */
{
	/* Cancel key is processed by UI_KeypadInput() */
	if( fHostFlags.b.handfree && key == VKEY_SPEAKER )
		UI_StateTransition( UI_STATE_STANDBY, 0 );
}

static void AnswerIncomingCall( int bHandfree )
{
	if( bHandfree )
		EnableHandfreeFunctionalityAndFlag( 1 /* enable */ );

	MM_StopTonePlaying();	/* stop incoming ringtone */
	fLineFlags[ IP_SID_TEMP ].b.interactIncoming = 1;
	API_AcceptIncomingCall( IP_SID_TEMP );
	UI_StateTransition( UI_STATE_INCOMING_CALL_WAIT, 0 );
}

void IncomingCallStateTimerEvent( void )
{
	/* check auto dial time */
	if( fModeFlags.b.autoAnswer &&
		sht.incomingCall.msAutoAnswerTimeThres != 0 &&
		CheckIfTimeoutInMillisecond( &sht.incomingCall.nAutoAnswerTimer, 
									sht.incomingCall.msAutoAnswerTimeThres ) == 0 )
	{
		AnswerIncomingCall( 1 /* handfree */ );
	}
}

void SetupIncomingCallFrame( int param )
{
	/* Read auto answer */
	sht.incomingCall.nAutoAnswerTimer = GetUptimeInMillisecond();
	sht.incomingCall.msAutoAnswerTimeThres = ReadAutoAnswerTimeInMillisecond();
}

void StateIncomingCallOnKey( unsigned char key )	/* UI_STATE_INCOMING_CALL */
{
	if( key == VKEY_OFF_HOOK ) {
		AnswerIncomingCall( 0 /* normal */ );
	} else if( key == VKEY_SPEAKER || key == VKEY_PICK ) {
		AnswerIncomingCall( 1 /* handfree */ );
	} else if( key == VKEY_CANCEL ) {
		MM_StopTonePlaying();	/* stop incoming ringtone */
		fLineFlags[ IP_SID_TEMP ].b.interactIncoming = 1;
		API_RejectIncomingCall();
		UI_StateTransition( UI_STATE_DISCONNECTION_WAIT, 0 );
	}
}

void SetupIncomingCallWaitFrame( int param )
{
}

void StateIncomingCallWaitOnKey( unsigned char key )	/* UI_STATE_INCOMING_CALL_WAIT */
{
	if( key == VKEY_ON_HOOK ) {
		API_RejectIncomingCall();
		UI_StateTransition( UI_STATE_DISCONNECTION_WAIT, 0 );
	}
}

static void RefreshOutgoingCallFrame( const unsigned char *pszOutgoingCallTitle,
										const unsigned char *pszPhonenumber )
{
	int len = strlen( ( const char * )pszOutgoingCallTitle );

	GS_DrawOffScreenAndClearScreen();

	GS_TextOut( 0, 0, pszOutgoingCallTitle, len );
	GS_TextOut( 0, 1, pszPhonenumber, strlen( ( const char * )pszPhonenumber ) );

	RightSoftkeyConfigurationWithInstructionText( VKEY_CANCEL, szInsCancel );

	GS_DrawOnScreen( NULL );

	/* Start waiting dot animation */
	StartPlayingAnimation( len, 0, ANI_ID_WAITING_DOT );
}

static void DoOutgoingCallWithStateTransition( sid_t sid, const unsigned char *pszPhonenumber, ocf_t ocf )
{
	fLineFlags_t *pfLineFlags;
	
	pfLineFlags = &fLineFlags[ sid ];

	/* call flags */
	if( pfLineFlags ->b.disconnected ) {
		debug_out( "fCallFlags.b.disconnected should be clear.\n" );
	}
	
	pfLineFlags ->all = 0;	
	pfLineFlags ->b.active = 1;
	pfLineFlags ->b.contact = 1;
	pfLineFlags ->b.incomingCall = 0;	/* outgoing call */

	strcpy( ( char * )sh.line[ sid ].szCalledPhonenumber, ( const char * )pszPhonenumber );

	/* Ask SIP to do outgoing call */
	API_OutgoingCall( sid, pszPhonenumber, ocf );
	
	if( !( ocf & OCF_CONNECTED ) ) {
		UI_StateTransition( UI_STATE_OUTGOING_CALL, 0 );
	
		RefreshOutgoingCallFrame( ( ocf & OCF_FXO ) ? szDoingFxoConnecting : szDoingOutgoingCall, 
									pszPhonenumber );
	}
	
	/* play tone */
	//MM_StartTonePlaying( TONE_ID_IN_RING, 0 /* repeat */ );

	/* call record */
	AddRecordOfOutgoingCall( pszPhonenumber, GetCurrentDateTime() );
}

#if 0
static void DoFxoSwitchWithStateTransition( sid_t sid, unsigned int bInConnection )
{
	fLineFlags_t *pfLineFlags;
	
	pfLineFlags = &fLineFlags[ sid ];

	/* call flags */
	if( pfLineFlags ->b.disconnected ) {
		debug_out( "fCallFlags.b.disconnected should be clear.\n" );
	}
	
	pfLineFlags ->all = 0;	
	pfLineFlags ->b.active = 1;
	pfLineFlags ->b.contact = 1;
	pfLineFlags ->b.incomingCall = 0;	/* outgoing call */

	//strcpy( ( char * )sh.line[ sid ].szCalledPhonenumber, ( const char * )pszPhonenumber );
	sh.line[ sid ].szCalledPhonenumber[ 0 ] = '\x0';

	/* ok, draw connection frame */
	ReadyToStartTimerInConnection( sid );
	//TurnOnLEDThroughGPIO( sid ? LED_BIT_LINE2 : LED_BIT_LINE1 );
	pfLineFlags ->b.connected = 1;
	//StopPlayingAnimation();
	UI_StateTransition( UI_STATE_IN_CONNECTION, 0 );	/* Do state transition at the last action */

	API_SwitchLineToFXO( sid, 1 /* enable */, bInConnection );
}
#endif

void SetupOutgoingCallFrame( int param )
{
}

void StateOutgoingCallOnKey( unsigned char key )	/* UI_STATE_OUTGOING_CALL */
{
	if( key == VKEY_CANCEL || key == VKEY_ON_HOOK ) {
		API_CancelOutgoingCall( IP_SID_TEMP, 0 );
		UI_StateTransition( UI_STATE_DISCONNECTION_WAIT, 0 );
	}
}

void DrawTimerInUnitOfSecond( int x, int y, unsigned long nSecond )
{
	unsigned char szBuffer[ 10 ];	/* 00:00:00 */
	unsigned int hh, mm, ss;
	
	mm = nSecond / 60;
	ss = nSecond % 60;
	
	hh = mm / 60;
	mm = mm % 60;
	
	if( hh >= 100 ) 
		hh %= 100;
	
	sprintf( ( char * )szBuffer, "%02u:%02u:%02u", hh, mm, ss );
	
	GS_TextOut( x, y, szBuffer, strlen( ( char * )szBuffer ) );
}

static unsigned long GetConnectionPeriodInUnitOfSecond( sid_t sid )
{
	const call_line_t * const pCallLine = &sh.line[ sid ];

	return pCallLine ->nSecConnectionLast - pCallLine ->nSecConnectionStart;
}

static void DrawOneLineTimerInConnection( int bEntire, int bEnableDraw, int bDrawOnly,
										  call_line_t *pCallSess, int y )
{
	#define CONNECT_TIMER_X_OFFSET		0

	/* Timer store in format of "00:00:00", and its type is string. */
	static const struct table_s {
		unsigned int index;
		unsigned int overflow;
	} table[] = { 
		{ 7, 10 }, { 6, 6 },	/* ss */
		{ 4, 10 }, { 3, 6 },	/* mm */
		{ 1, 10 }, { 0, 10 },	/* hh */
	};

	unsigned long carry;
	unsigned long now;
	int i, index = 0xFF, overflow;

	if( bDrawOnly )
		goto label_draw_one_line_timer_in_connection;
	
	now = GetUptimeInSecond();
	
	carry = now - pCallSess ->nSecConnectionLast;
	
	if( carry > 10 ) {
		carry = 10;	/* prevent unexpected character */
		debug_out( "DrawOneLineTimerInConnection(): Delta is too large\n" );
	}
	
	if( carry > 0 ) {	
		for( i = 0; i < 6; i ++ ) {
			index = table[ i ].index;
			overflow = table[ i ].overflow;
			
			pCallSess ->szDisplayTime[ index ] += ( unsigned char )carry;
			
			if( ( pCallSess ->szDisplayTime[ index ] - '0' ) >= overflow ) {
				pCallSess ->szDisplayTime[ index ] -= overflow;
				carry = 1;
			} else
				break;
		}
	} 

	pCallSess ->nSecConnectionLast = now;	

label_draw_one_line_timer_in_connection:
	/* Draw timer unconditionally, or refresh modified part only. */
	if( bEnableDraw ) {
		if( bEntire )
			GS_TextOut( CONNECT_TIMER_X_OFFSET + 0, y, &pCallSess ->szDisplayTime[ 0 ], 8 );
		else if( index <= 7 )
			GS_TextOut( CONNECT_TIMER_X_OFFSET + index, y, &pCallSess ->szDisplayTime[ index ], 8 - index );
	}	
	
	#undef CONNECT_TIMER_X_OFFSET
}

static inline void DrawOneLineInfoInConenction( int bEntire, int bEnableDraw, 
											    int y, sid_t sid )
{
	#define CONNECT_STATUS_X_OFFSET		8	/* 00:00:00[xxxxxx] */

	const fLineFlags_t * const pfLineFlags = &fLineFlags[ sid ];
	unsigned short len1, len2;

	/* Update/draw connection timer */
	if( pfLineFlags ->b.connected ) 
	{
		DrawOneLineTimerInConnection( bEntire, bEnableDraw, pfLineFlags ->b.disconnected, 
										&sh.line[ sid ], y );
	}

	if( !bEnableDraw )
		return;

	if( !pfLineFlags ->b.connected ) {
		if( pfLineFlags ->b.waiting ) {
			/* call waiting --> show 'Call:12345678' */
			if( !bEntire )
				return;		/* draw only if draw entire screen, because it only need draw once. */

			GS_TextOut( 0, y, szCallPrompt, ( len1 = RES_strlen( szCallPrompt ) ) );	
			len2 = strlen( ( const char * )sh.line[ sid ].szCalledPhonenumber );

			if( len2 + len1 > VRAM_WIDTH_IN_TEXT_UNIT )
				len2 = VRAM_WIDTH_IN_TEXT_UNIT - len1;

			if( len2 )
				GS_TextOut( len1, y, sh.line[ sid ].szCalledPhonenumber, len2 );
			else
				GS_TextOut( len1, y, szNoname, RES_strlen( szNoname ) );
		} else if( pfLineFlags ->b.contact && !pfLineFlags ->b.incomingCall &&
				   pfLineFlags ->b.disconnected && pfLineFlags ->b.discDisplay ) 
		{
			/* Disconnected Prompt */
			GS_TextOut( 0, y, szDisconnection, RES_strlen( szDisconnection ) );
		} else if( pfLineFlags ->b.contact && !pfLineFlags ->b.incomingCall ) {
			/* outgoing call 2 --> show '12345678...' */
			if( !bEntire )
				return;		/* draw only if draw entire screen, because it only need draw once. */

			len1 = strlen( ( const char * )sh.line[ sid ].szCalledPhonenumber );

			if( len1 > VRAM_WIDTH_IN_TEXT_UNIT - 3 )	/* 3 for ... */
				len1 = VRAM_WIDTH_IN_TEXT_UNIT - 3;

			GS_TextOut( 0, y, sh.line[ sid ].szCalledPhonenumber, len1 );
			StartPlayingAnimation( len1, y, ANI_ID_WAITING_DOT );
		}
	} else if( pfLineFlags ->b.connected ) {
		if( pfLineFlags ->b.disconnected && pfLineFlags ->b.discDisplay ) {
			/* Disconnected Prompt */
			GS_TextOut( CONNECT_STATUS_X_OFFSET, y, szCallDisconnectedStatus, RES_strlen( szCallDisconnectedStatus ) );
		} else if( pfLineFlags ->b.hold ) {
			/* active hold */
			if( pfLineFlags ->b.held )
				GS_TextOut( CONNECT_STATUS_X_OFFSET, y, szCallHoldStatus, RES_strlen( szCallHoldStatus ) );
			else	/* holding */
				GS_TextOut( CONNECT_STATUS_X_OFFSET, y, szCallHoldingStatus, RES_strlen( szCallHoldingStatus ) );
		} else if( pfLineFlags ->b.held ) {
			/* passive hold */
			GS_TextOut( CONNECT_STATUS_X_OFFSET, y, szCallHeldStatus, RES_strlen( szCallHeldStatus ) );
		} else if( fCallFlags.b.conference ) {
			/* Conference */
			GS_TextOut( CONNECT_STATUS_X_OFFSET, y, szCallConferenceStatus, RES_strlen( szCallConferenceStatus ) );
		} 
	}

	#undef CONNECT_STATUS_X_OFFSET
}

static void DrawAllLinesInfoInConnection( int bEntire, int bEnableDraw, int bDrawTitle )
{
	sid_t activate_sid;
	int y;

	if( !fCallFlags.b.multiline ) {
		/* which one activate? */
		if( fLineFlags[ 0 ].b.contact && !fLineFlags[ 0 ].b.disconnected )
			activate_sid = 0;
		else if( fLineFlags[ 1 ].b.contact && !fLineFlags[ 1 ].b.disconnected )
			activate_sid = 1;
		else {
			if( uiState == UI_STATE_IN_CONN_DIAL )
				;	/* another session may disconnect */
			else
				debug_out( "No activate line??\n" );
			return;
		}

		if( bEnableDraw && bDrawTitle )
			GS_TextOut( 0, 0, szInConnection, RES_strlen( szInConnection ) );

		/* draw single line only */
		DrawOneLineInfoInConenction( bEntire, bEnableDraw, 1, activate_sid );
	} else {

		/* draw two lines */
		for( activate_sid = 0; activate_sid < CALL_LINE_NUM; activate_sid ++ ) {

			y = activate_sid;

			/* draw each line */
			DrawOneLineInfoInConenction( bEntire, bEnableDraw, y, activate_sid );
		}
	}
}

static inline void DrawInConnectionNormalFrame( void )
{
	DrawAllLinesInfoInConnection( 1 /* update entire digits */, 1 /* draw */, 1 /* draw title */ );
}

static inline void ConfigSoftkeyInConnectionByFlags( void )
{
	fLineFlags_t *pfLineFlags, *pfLineFlags_another = NULL;
	int bConference = 0, bWaiting = 0, bHold = 0;			/* for multiline */
	int bDial = 0, bTransfer = 0;	/* for single line */
	sid_t activate_sid, another_sid;

	/* single line */
	if( !fCallFlags.b.multiline ) {
		if( fLineFlags[ 0 ].b.active ) {
			activate_sid = 0;
			another_sid = 1;
			pfLineFlags = &fLineFlags[ 0 ];
		} else if( fLineFlags[ 1 ].b.active ) {
			activate_sid = 1;
			another_sid = 0;
			pfLineFlags = &fLineFlags[ 1 ];
		} else {
			debug_out( "No active line?\n" );
			return;
		}

		goto label_decide_line_flags_done;
	}

	/* multiline */
	if( fLineFlags[ 0 ].b.active && !fLineFlags[ 1 ].b.active ) {
		pfLineFlags = &fLineFlags[ 0 ];
		pfLineFlags_another = &fLineFlags[ 1 ];
		activate_sid = 0;
		another_sid = 1;
	} else if( fLineFlags[ 1 ].b.active && !fLineFlags[ 0 ].b.active ) {
		pfLineFlags = &fLineFlags[ 1 ];
		pfLineFlags_another = &fLineFlags[ 0 ];
		activate_sid = 1;
		another_sid = 0;
	} else {
		debug_out( "No active line?\n" );
		return;
	}

label_decide_line_flags_done:

	/* ------------------------------------------------------------------------ */
	/* Now, we are going to decide possible actions. */
	/* 1. conference */
	if( fCallFlags.b.multiline ) {
		if( fCallFlags.b.conference ) 
			goto label_decide_softkey_done;
		if( pfLineFlags ->b.switching || pfLineFlags_another ->b.switching ) 
			goto label_decide_softkey_done;
		
		if( pfLineFlags ->b.connected && !pfLineFlags ->b.hold && !pfLineFlags ->b.held && 
			!pfLineFlags ->b.disconnected &&
			pfLineFlags_another ->b.connected && !pfLineFlags_another ->b.disconnected ) 
		{
			bConference = 1;
			bHold = 1;
		}

		if( pfLineFlags_another ->b.waiting )
			bWaiting = 1;
	
		goto label_decide_softkey_done;
	}

	/* single line here */
	if( !pfLineFlags ->b.switching && !pfLineFlags ->b.held ) {
		bDial = 1;
		bTransfer = 1;
	}

label_decide_softkey_done:

	/* ------------------------------------------------------------------------ */
	/* Now, we are going to config instruction text */
	/* Reset softkey */
	ResetSoftkeyConfiguration();
	ClearSoftkeyInstructionTextRetangle( NULL );

	if( bDial )
		LeftSoftkeyConfigurationWithInstructionText( ( another_sid ? VKEY_LINE2 : VKEY_LINE1 ), szInsDial );

	if( bTransfer )
		MiddleSoftkeyConfigurationWithInstructionText( VKEY_TRANSFER, szInsTransfer );

	if( bConference ) 
		Middle2SoftkeyConfigurationWithInstructionText( VKEY_CONFERENCE, szInsConference );

	if( bWaiting )
		RightSoftkeyConfigurationWithInstructionText( ( another_sid ? VKEY_LINE2 : VKEY_LINE1 ), szInsAnswer );

	if( bHold )
		LeftSoftkeyConfigurationWithInstructionText( ( another_sid ? VKEY_LINE2 : VKEY_LINE1 ), szInsHold );
}

static void RefreshInConnectionNormalFrame( void )
{
	GS_DrawOffScreenAndClearScreen();
	DrawInConnectionNormalFrame();
	ConfigSoftkeyInConnectionByFlags();
	GS_DrawOnScreen( NULL );
}

static void ReadyToStartTimerInConnection( sid_t sid )
{
	call_line_t * const pCallLine = &sh.line[ sid ];

	/* initialze connection variables */
	pCallLine ->nSecConnectionStart = pCallLine ->nSecConnectionLast
								= GetUptimeInSecond();
	strcpy( ( char * )pCallLine ->szDisplayTime, ( const char * )"00:00:00" );
}

void RefreshAllLinesInfoInConnection( void )
{
	GS_DrawOffScreen();
	
	if( fCallFlags.b.volumePrompt ) {
		if( CheckIfTimeoutInMillisecond( &sh.nVolumePromptTimer, 2000 /* 2 seconds */ ) == 0 ) {
			fCallFlags.b.volumePrompt = 0;
			GS_ClearScreen();
			DrawInConnectionNormalFrame();
			return;
		}
		DrawAllLinesInfoInConnection( 0 /* update difference digits only */, 0 /* don't draw */, 0 /* don't draw title */ );
	} else {
		DrawAllLinesInfoInConnection( 0 /* update difference digits only */, 1 /* draw */, 0 /* don't draw title */ );
	}

	GS_DrawOnScreen( NULL );	
}

void RefreshAllLinesTimerOfInfoInConnection( void )
{
	DrawAllLinesInfoInConnection( 0 /* update difference digits only */, 0 /* don't draw */, 0 /* don't draw title */ );
}

void CheckDisconnectedPromptTimeoutInConnection( void )
{
	/* Not only active line shows disc. prompt, but also inactive one. */

	sid_t sid, another_sid;
	fLineFlags_t *pfLineFlags;
	const fLineFlags_t *pfLineFlags_another;
	call_line_t *pCallLine;
	int bRefreshConnectionFrame;

	if( fLineFlags[ 0 ].b.discDisplay || fLineFlags[ 1 ].b.discDisplay )
		;
	else
		return;

	/* If two lines are disconnected, state will be DISCONENCTION. */
	if( fLineFlags[ 0 ].b.discDisplay && fLineFlags[ 1 ].b.discDisplay )
		debug_out( AC_FORE_RED "Display two disc. in connection?\n" AC_RESET );

	bRefreshConnectionFrame = 0;

	for( sid = 0; sid < CALL_LINE_NUM; sid ++ ) {

		pfLineFlags = &fLineFlags[ sid ];
		pCallLine = &sh.line[ sid ];

		if( pfLineFlags ->b.discDisplay ) {
			if( CheckIfTimeoutInMillisecond( &pCallLine ->nDiscPromptTimer, 
												DISCONNECT_PROMPT_PERIOD /* 5000 ms */ ) == 0 )
			{
				bRefreshConnectionFrame = 1;
				break;
			}
		}
	}

	if( bRefreshConnectionFrame ) {

		/* If it is active and disconnected, switch to another line */
		if( pfLineFlags ->b.active ) {

			another_sid = ( sid ? 0 : 1 );
			pfLineFlags_another = &fLineFlags[ another_sid ];

			if( !pfLineFlags_another ->b.active &&	
				pfLineFlags_another ->b.connected && pfLineFlags_another ->b.held && 
				!pfLineFlags_another ->b.disconnected )
			{
				InConnectionLineSwitching( ( another_sid ? LSR_LINE2 : LSR_LINE1 ) );
			}
		}

		/* Turn off disc. display */
		pfLineFlags ->b.discDisplay = 0;	/* this flag is used by InConnectionLineSwitching() */

		/* Turn off multi-line display */
		fCallFlags.b.multiline = 0;

		/* Draw timer frame */
		fCallFlags.b.volumePrompt = 0;	/* turn off volume prompt immediately */

		/* after set flags, re-draw connection frame */
		RefreshInConnectionNormalFrame();

		DisplayLedInConnectionRelyOnFlags();
	}
}

#ifndef NO_LINE_LED
static void DetermineFlagsLedStyle( const fLineFlags_t *pfLineFlags, unsigned long mask,
								    unsigned long *pLedBlinkingMask, 
									unsigned long *pLedOnMask )
{
	if( pfLineFlags ->b.disconnected && pfLineFlags ->b.discDisplay )
		goto label_turn_on_led;

	if( !pfLineFlags ->b.contact || pfLineFlags ->b.disconnected )
		return;		/* not in use */

	if( !pfLineFlags ->b.connected && pfLineFlags ->b.incomingCall )
		*pLedBlinkingMask |= mask;	/* incoming not connected -> blinking */

	if( pfLineFlags ->b.held )
		*pLedBlinkingMask |= mask;	/* connected but held -> blinking */

label_turn_on_led:
	*pLedOnMask |= mask;
}
#endif

static void DisplayLedInConnectionRelyOnFlags( void )
{
#ifndef NO_LINE_LED
	/* Control LED_BIT_LINE1 and LED_BIT_LINE2 only. */
	unsigned long ledBlinkingMask = 0;
	unsigned long ledOnMask = 0;

	DetermineFlagsLedStyle( &fLineFlags[ 0 ], LED_BIT_LINE1, &ledBlinkingMask, &ledOnMask );
	DetermineFlagsLedStyle( &fLineFlags[ 1 ], LED_BIT_LINE2, &ledBlinkingMask, &ledOnMask );

	if( uiState == UI_STATE_IN_CONN_DIAL )
		TurnOnLEDThroughGPIO( ledOnMask | ledBlinkingMask );
	else
		TurnOnOffLEDThruoghGPIO( LED_BIT_LINE1 | LED_BIT_LINE2, ledOnMask | ledBlinkingMask );
	MM_StartLedBlinking( ledBlinkingMask );
#endif
}

void SetupInConnectionFrame( int param )
{
	/* draw screen */
	RefreshInConnectionNormalFrame();

	DisplayLedInConnectionRelyOnFlags();
}

static int DoLineSwitchingCase_SingleLineActive( lsr_t lsr )
{
	/* 
	 * Pre-condition: 
	 *   1. Not conference
	 *   2. Both of two lines are not switching. 
	 */
	fLineFlags_t *pfLineFlags;
	fLineFlags_t *pfLineFlags_another;

	/* Line select */
	if( lsr == LSR_LINE1 ) {
		pfLineFlags = &fLineFlags[ 0 ];	/* Line 1 */
		pfLineFlags_another = &fLineFlags[ 1 ];	/* Line 2 */
	} else if( lsr == LSR_LINE2 ) {
		pfLineFlags = &fLineFlags[ 1 ];	/* Line 2 */
		pfLineFlags_another = &fLineFlags[ 0 ];	/* Line 1 */		
	} else
		return 0;

	/* it is active line */
	if( pfLineFlags ->b.active ) {		/* active */
		debug_out( "It is an active line.\n" );
		goto label_line_switching__single_line_active_deny;
	}

	/* another line was held (it should be also active line) */
	if( pfLineFlags_another ->b.held && !pfLineFlags_another ->b.hold ) {	/* held */
		debug_out( "Another line was held.\n" );
		goto label_line_switching__single_line_active_deny;
	}
	
	if( !pfLineFlags ->b.contact || pfLineFlags ->b.disconnected ) {
		/* idle line -> going to dial and do outgoing call */
		if( pfLineFlags_another ->b.held && !pfLineFlags_another ->b.hold ) {
			debug_out( "Current active line is held, so it not allow to switch!\n" );
			goto label_line_switching__single_line_active_deny;
		}

		fTempFlags.b.connDial = 1;
		fTempFlags.b.connDialLine = ( lsr == LSR_LINE1 ? 0 : 1 );		
		fTempFlags.b.connDialTransfer = 0;

		if( !pfLineFlags_another ->b.held ) {	/* If it is NOT held. */
			pfLineFlags_another ->b.hold = 1;
			pfLineFlags_another ->b.switching = 1;	/* unset after HELD IND. */
		}

		/* If this line is still display 'disc', I think this line is just disconnect in conference. */
		if( pfLineFlags ->b.discDisplay )
			fCallFlags.b.multiline = 0;

		pfLineFlags ->all = 0;
		pfLineFlags ->b.active = 1;
		pfLineFlags_another ->b.active = 0;

		API_LineSwitching( lsr, 0 /* not call transfer */, 0 /* not back */ );		
	} else if( pfLineFlags ->b.connected ) {
		/* In connection */
		if( !fCallFlags.b.multiline ) {
			debug_out( "No multiline to switch?\n" );
			goto label_line_switching__single_line_active_deny;
		}

		if( !pfLineFlags_another ->b.active ) {
			debug_out( "Another line has to be active!\n" );
			goto label_line_switching__single_line_active_deny;
		}

#if 0
		/* another line connected and NOT held */
		if( pfLineFlags_another ->b.connected && 
			!pfLineFlags_another ->b.disconnected &&
			!pfLineFlags_another ->b.held )
#else
		if( pfLineFlags_another ->b.connected && 
			!pfLineFlags_another ->b.disconnected )
#endif
		{
			/* resume this line, and hold another */
			pfLineFlags ->b.switching = 1;	/* unset after RESUME IND. if it hold other */

			if( !pfLineFlags_another ->b.held ) {
				pfLineFlags_another ->b.hold = 1;
				pfLineFlags_another ->b.switching = 1;	/* unset after HELD IND. */
			}

			pfLineFlags ->b.active = 1;
			pfLineFlags_another ->b.active = 0;

			API_LineSwitching( lsr, 0 /* not call transfer */, 0 /* not back */ );
		} else if( pfLineFlags_another ->b.contact &&
				   !pfLineFlags_another ->b.incomingCall &&
				   !pfLineFlags_another ->b.connected &&
				   pfLineFlags_another ->b.disconnected &&
				   pfLineFlags_another ->b.discDisplay )
		{
			/* outgoing call 2 fail -> switch to another one */
			pfLineFlags ->b.switching = 1;	/* unset after RESUME IND. */

			fTempFlags.b.unsetDiscDisplay = 1;

			pfLineFlags ->b.active = 1;
			pfLineFlags_another ->b.active = 0;

			API_LineSwitching( lsr, 0 /* not call transfer */, 0 /* not back */ );
		} else {
			debug_out( "Another line is not active?\n" );
			goto label_line_switching__single_line_active_deny;
		}
	} else if( pfLineFlags ->b.waiting ) {
		/* Call waiting */
		
#if 0
		/* another line connected and NOT held */
		if( pfLineFlags_another ->b.connected && 
			!pfLineFlags_another ->b.disconnected &&
			!pfLineFlags_another ->b.held )
#else
		/* another line is active */
		if( pfLineFlags_another ->b.active )
#endif
		{
			/* accept this line, and hold another */
			pfLineFlags ->b.interactIncoming = 1;
			pfLineFlags ->b.switching = 1;	/* unset after CONNECT IND. */

			if( !pfLineFlags_another ->b.held ) {
				pfLineFlags_another ->b.hold = 1;
				pfLineFlags_another ->b.switching = 1;	/* unset after HELD IND. */
			}

			pfLineFlags ->b.active = 1;
			pfLineFlags_another ->b.active = 0;

			API_LineSwitching( lsr, 0 /* not call transfer */, 0 /* not back */ );
		} else {
			debug_out( "Another line is not active (waiting)?\n" );
		}
	} else if( pfLineFlags ->b.contact && !pfLineFlags ->b.incomingCall ) {
		debug_out( "This line is do outgoing call 2.\n" );
		goto label_line_switching__single_line_active_deny;
	} else {
		debug_out( "What is this case??\n" );
		goto label_line_switching__single_line_active_deny;
	}
	
	return 1;
	
label_line_switching__single_line_active_deny:
	return 0;
}

static int DoLineSwitchingCase_Conference( lsr_t lsr )
{
	/* 
	 * Pre-condition: 
	 *   1. Not conference
	 *   2. Both of two lines are not switching. 
	 */
	fLineFlags_t * pfLineFlags;
	int i, count;
	
	if( !fCallFlags.b.multiline ) {
		debug_out( "Conference only if multi-lines.\n" );
		goto label_line_switching__conference_deny;
	}
	
	if( !fLineFlags[ 0 ].b.connected || fLineFlags[ 0 ].b.disconnected || 
		!fLineFlags[ 1 ].b.connected || fLineFlags[ 1 ].b.disconnected ) 
	{
		/* call waiting can't do conference. */
		debug_out( "All lines should be connected.\n" );
		goto label_line_switching__conference_deny;
	}
	
	if( ( !fLineFlags[ 0 ].b.hold && fLineFlags[ 0 ].b.held ) ||
		( !fLineFlags[ 1 ].b.hold && fLineFlags[ 1 ].b.held ) )
	{
		debug_out( "Hold by other.\n" );
		goto label_line_switching__conference_deny;
	}
	
	/* set flags for each line */
	for( i = 0, count = 0; i < CALL_LINE_NUM; i ++ ) {
		
		pfLineFlags = &fLineFlags[ i ];
		
		if( pfLineFlags ->b.held ) {
			pfLineFlags ->b.active = 1;
			pfLineFlags ->b.switching = 1;	/* unset after RESUME IND. */
			count ++;
		}
	}
	
	if( count != 1 ) {
		debug_out( AC_FORE_RED "Conference error call:%08lu line1:%08lu line2:%08lu.\n" AC_RESET, 
					fCallFlags.all, fLineFlags[ 0 ].all, fLineFlags[ 1 ].all );
	}
	
	API_LineSwitching( lsr, 0 /* not call transfer */, 0 /* not back */ );
	
	return 1;

label_line_switching__conference_deny:
	return 0;	
}

static int InConnectionLineSwitching( lsr_t lsr )
{
	/* check if possible to run switching */
	if( fCallFlags.b.conference ) {
		debug_out( "In conference call.\n" );
		goto label_line_switching_deny;
	}
	
	if( fLineFlags[ 0 ].b.switching || fLineFlags[ 1 ].b.switching ) {
		debug_out( "It is switching...\n" );
		goto label_line_switching_deny;
	}
	
	/* depend on LSR to check lines states */
	switch( lsr ) {
	case LSR_LINE1:
	case LSR_LINE2:
		if( !DoLineSwitchingCase_SingleLineActive( lsr ) )
			goto label_line_switching_deny;
		break;
	
	case LSR_CONFERENCE:
		if( !DoLineSwitchingCase_Conference( lsr ) )
			goto label_line_switching_deny;
		break;
		
	default:
		debug_out( "We has this case??\n" );
		goto label_line_switching_deny;
		break;
	}
	
	return 1;

label_line_switching_deny:
	return 0;
}

static int InConnectionCallTransfer( void )
{
	fLineFlags_t *pfLineFlags;
	fLineFlags_t *pfLineFlags_another;
	lsr_t lsr_target;

	/* check if possible to run call transfer */
	if( fCallFlags.b.multiline ) {
		debug_out( "Multiline can't do call transfer!\n" );
		goto label_call_transfer_deny;
	}

	if( fCallFlags.b.transfer ) {
		debug_out( "It is transfering.\n" );
		goto label_call_transfer_deny;
	}

	if( fLineFlags[ 0 ].b.switching || fLineFlags[ 1 ].b.switching ) {
		debug_out( "It is switching...\n" );
		goto label_call_transfer_deny;
	}

	/* get active line */
	if( fLineFlags[ 0 ].b.active ) {
		pfLineFlags = &fLineFlags[ 0 ];
		pfLineFlags_another = &fLineFlags[ 1 ];
		lsr_target = LSR_LINE2;
	} else if( fLineFlags[ 1 ].b.active ) {
		pfLineFlags = &fLineFlags[ 1 ];
		pfLineFlags_another = &fLineFlags[ 0 ];
		lsr_target = LSR_LINE1;
	} else {
		debug_out( "No active line?\n" );
		goto label_call_transfer_deny;
	}

	/* check active line */
	if( pfLineFlags ->b.held ) {
		debug_out( "Active line is held.\n" );
		goto label_call_transfer_deny;
	}

	if( !pfLineFlags ->b.contact || !pfLineFlags ->b.connected || pfLineFlags ->b.disconnected ) {
		debug_out( "Active line is not connected.\n" );
		goto label_call_transfer_deny;
	}

	/* Now, hold active line and going to do call transfer */
	fCallFlags.b.transfer = 1;

	pfLineFlags ->b.hold = 1;
	pfLineFlags ->b.switching = 1;

	pfLineFlags_another ->all = 0;

	pfLineFlags ->b.active = 0;
	pfLineFlags_another ->b.active = 1;

	API_LineSwitching( lsr_target, 1 /* call transfer */, 0 /* not back */ );

	fTempFlags.b.connDialLine = ( lsr_target == LSR_LINE1 ? 0 : 1 );		
	fTempFlags.b.connDialTransfer = 1;

	return 1;

label_call_transfer_deny:
	return 0;
}

static int InConnectionDialSwitchBack( lsr_t lsr )
{
	/* lsr is old active session, so another indicates the dialing session. */
	fLineFlags_t *pfLineFlags;
	fLineFlags_t *pfLineFlags_another;
	
	if( fLineFlags[ 0 ].b.switching || fLineFlags[ 1 ].b.switching ) {
		debug_out( "It is switching...\n" );
		goto label_connection_dial_switch_back_deny;
	}

	/* Line select */
	if( lsr == LSR_LINE1 ) {
		pfLineFlags = &fLineFlags[ 0 ];	/* Line 1 */
		pfLineFlags_another = &fLineFlags[ 1 ];	/* Line 2 */
	} else if( lsr == LSR_LINE2 ) {
		pfLineFlags = &fLineFlags[ 1 ];	/* Line 2 */
		pfLineFlags_another = &fLineFlags[ 0 ];	/* Line 1 */		
	} else
		return 0;

	/* active rules */
	if( !pfLineFlags ->b.active && pfLineFlags_another ->b.active )
		;
	else {
		debug_out( "Bad active condition to back.\n" );
		goto label_connection_dial_switch_back_deny;
	}

	/* target should hole */
	if( !pfLineFlags ->b.contact || pfLineFlags ->b.disconnected ) {
		debug_out( "Back to a disconnected line?\n" );
		goto label_connection_dial_switch_back_deny;
	}

	if( pfLineFlags ->b.connected && pfLineFlags ->b.hold )
		;
	else {
		debug_out( "Back line should be connected and hold.\n" );
		goto label_connection_dial_switch_back_deny;
	}

	/* current should be going to dial */
	if( !pfLineFlags_another ->b.contact && !pfLineFlags_another ->b.incomingCall &&
		!pfLineFlags_another ->b.connected ) 
	{
	} else {
		debug_out( "Current should be going to dial.\n" );
		goto label_connection_dial_switch_back_deny;
	}

	pfLineFlags_another ->b.active = 0;	/* easy to search this flag */
	pfLineFlags_another ->all = 0;

	pfLineFlags ->b.active = 1;
	pfLineFlags ->b.switching = 1;	/* unset after RESUME IND. */

	fCallFlags.b.transfer = 0;	/* switch back call transfer */

	API_LineSwitching( lsr, 0 /* Not call transfer */, 1 /* back */ );

	return 1;

label_connection_dial_switch_back_deny:
	return 0;
}

static void InConnectionLineSwitchingPreProcedure( void )
{
#ifdef _DEBUG_MODE
	if( fHostFlags.b.lsrProcPair ) {
		/* Don't worry with this message, which is only a debug message. */
		debug_out( "Enter LSR pre-procedure twice.\n" );
	}

	fHostFlags.b.lsrProcPair = 1;
#endif

	fTempFlags.b.connDial = 0;
	fTempFlags.b.unsetDiscDisplay = 0;
}

static void InConnectionLineSwitchingPostProcedure( lsr_t lsr )
{
	fLineFlags_t *pfLineFlags;

#ifdef _DEBUG_MODE
	if( !fHostFlags.b.lsrProcPair ) 
		debug_out( "Execute LSR pre-procedure first.\n" );

	fHostFlags.b.lsrProcPair = 0;
#endif

	if( lsr == LSR_LINE1 )
		pfLineFlags = &fLineFlags[ 0 ];
	else if( lsr == LSR_LINE2 )
		pfLineFlags = &fLineFlags[ 1 ];
	else {
		debug_out( "InConnectionLineSwitchingPostProcedure() unexpected lsr.\n" );
		return;
	}

	if( fTempFlags.b.connDial ) {
		UI_StateTransition( UI_STATE_IN_CONN_DIAL, 0 );

		fTempFlags.b.connDial = 0;
	} else if( fTempFlags.b.unsetDiscDisplay ) {

		pfLineFlags ->b.discDisplay = 0;

		fCallFlags.b.multiline = 0;
		fCallFlags.b.volumePrompt = 0;

		/* after set flags, re-draw connection frame */
		RefreshInConnectionNormalFrame();

		DisplayLedInConnectionRelyOnFlags();

		fTempFlags.b.unsetDiscDisplay = 0;
	}
}

static int RefreshAdjustOutVolumeInConnectionFrame( unsigned char key )
{
	int ret;
	inout_vol_type_t type;
	
	/* decide type by flags */
	if( fHostFlags.b.handfree ) {
		if( fHostFlags.b.adjustMic )
			type = INOUTVOL_TYPE_MIC_S;
		else
			type = INOUTVOL_TYPE_SPEAKER;
	} else {
		if( fHostFlags.b.adjustMic )
			type = INOUTVOL_TYPE_MIC_R;
		else
			type = INOUTVOL_TYPE_RECEIVER;
	}
	
	/* ok refresh frame */
	sh.nVolumePromptTimer = GetUptimeInMillisecond();
	ret = RefreshAdjustOutVolumeXxxFrame( fCallFlags.b.volumePrompt, 
										  key, 
										  1 /* refresh hardware */, 
										  type );
	fCallFlags.b.volumePrompt = 1;

	return ret;
}

void StateInConnectionOnKey( unsigned char key )	/* UI_STATE_IN_CONNECTION */
{
	sid_t sid;

	if( IsDialKey( key ) ) {
		for( sid = 0; sid < CALL_LINE_NUM; sid ++ )
			if( fLineFlags[ sid ].b.active )
				API_SendDTMF( sid, key );
	} else if( key == VKEY_CANCEL ) {
label_disconnect_call:
		API_DisconnectCall();
		UI_StateTransition( UI_STATE_DISCONNECTION_WAIT, 0 );
	} else if( key == VKEY_ON_HOOK ) {
		if( !fHostFlags.b.handfree )
			goto label_disconnect_call;
	} else if( key == VKEY_OFF_HOOK ) {
		if( fHostFlags.b.handfree )
			EnableHandfreeFunctionalityAndFlag( 0 /* disable */ );
	} else if( key == VKEY_OUTVOL_PLUS || key == VKEY_OUTVOL_MINUS ) {
		/* show volume status */
		RefreshAdjustOutVolumeInConnectionFrame( key );
	} else if( key == VKEY_SPEAKER ) {
		if( fHostFlags.b.handfree ) {
			if( !fHostFlags.b.hookStatus ) {
				/* handfree and on-hook, then press 'speaker' */
#if 1
				goto label_disconnect_call;
#else
				goto label_ignore_this_key;
#endif
			}

			EnableHandfreeFunctionalityAndFlag( 0 /* disable */ );
		} else {
			EnableHandfreeFunctionalityAndFlag( 1 /* enable */ );
		}
		
		/* show volume status */
label_show_volume_status:
		RefreshAdjustOutVolumeInConnectionFrame( '\x0' /* key */ );
	} else if( key == VKEY_MICSPK_SWITCH ) {
		fHostFlags.b.adjustMic ^= 1;
		goto label_show_volume_status;
	} else if( key == VKEY_LINE1 ) {
label_do_action_of_vkey_line1:
		InConnectionLineSwitchingPreProcedure();

		if( !InConnectionLineSwitching( LSR_LINE1 ) ) {
			// TODO: play a tone ?? 
		} else {
			InConnectionLineSwitchingPostProcedure( LSR_LINE1 );
		}
	} else if( key == VKEY_LINE2 ) {
label_do_action_of_vkey_line2:
		InConnectionLineSwitchingPreProcedure();

		if( !InConnectionLineSwitching( LSR_LINE2 ) ) {
			// TODO: play a tone ?? 
		} else {
			InConnectionLineSwitchingPostProcedure( LSR_LINE2 );
		}
	} else if( key == VKEY_CONFERENCE ) {
		if( !InConnectionLineSwitching( LSR_CONFERENCE ) ) {
			// TODO: play a tone ?? 
		}
	} else if( key == VKEY_TRANSFER ) {
		if( InConnectionCallTransfer() )
			UI_StateTransition( UI_STATE_IN_CONN_DIAL, 0 );
	} else if( key == VKEY_PICK ) {
		if( fLineFlags[ 0 ].b.waiting )
			goto label_do_action_of_vkey_line1;
		else if( fLineFlags[ 1 ].b.waiting )
			goto label_do_action_of_vkey_line2;
	} else if( key == VKEY_HOLD ) {
		if( fCallFlags.b.multiline ) {
			if( fLineFlags[ 0 ].b.waiting )
				goto label_do_action_of_vkey_line1;
			else if( fLineFlags[ 1 ].b.waiting )
				goto label_do_action_of_vkey_line2;
		} else {
			if( fLineFlags[ 0 ].b.active && fLineFlags[ 0 ].b.connected && !fLineFlags[ 0 ].b.held )
				goto label_do_action_of_vkey_line2;
			else if( fLineFlags[ 1 ].b.active && fLineFlags[ 1 ].b.connected && !fLineFlags[ 1 ].b.held )
				goto label_do_action_of_vkey_line1;
		}
	}

#if 0
	else if( key == VKEY_CONFERENCE )
		MM_StartTonePlaying( TONE_ID_DIAL, 0 );
#endif

//label_ignore_this_key:
	;
}

void SetupInConnectionDialFrame( int param )
{
	SetupDialFrame( PARAM_BORROW );

#ifndef NO_LINE_LED
	TurnOnLEDThroughGPIO( ( fTempFlags.b.connDialLine ? LED_BIT_LINE2 : LED_BIT_LINE1 ) );
#endif
}

void StateInConnectionDialOnKey( unsigned char key )	/* UI_STATE_IN_CONN_DIAL */
{
	unsigned char szDial[ MAX_LEN_OF_PHONENUMBER + 1 ];	
	sid_t sid, another_sid;
	int bFxo = 0;
	ocf_t ocf;

	if( key == VKEY_OK || key == VKEY_POUND ) {
		if( GetTextEditorTextContent( szDial, MAX_LEN_OF_PHONENUMBER ) ) {
label_in_connection_do_2nd_outgoing_call:
			sid = ( fTempFlags.b.connDialLine ? ( sid_t )1 : ( sid_t )0 );
			another_sid = ( sid ? ( sid_t )0 : ( sid_t )1 );
			
			/* Don't worry about another line is disconnect, its discDisplay is a new indicator. */
			fCallFlags.b.multiline = 1;		/* outgoing call in connection --> multi-lines */

			/* update disc. prompt timer of another line */
			if( fLineFlags[ another_sid ].b.discDisplay )
				sh.line[ another_sid ].nDiscPromptTimer = GetUptimeInMillisecond();
				
			/* decide outgoing call flags */
			ocf = OCF_CONNECTED | 
				  ( fTempFlags.b.connDialTransfer ? OCF_TRANSFER : OCF_NONE ) |
				  ( bFxo ? OCF_FXO : OCF_NONE );

			/* ok. Do 2nd outgoing call or transfer call */
			DoOutgoingCallWithStateTransition( sid, szDial, ocf );
			UI_StateTransition( UI_STATE_IN_CONNECTION, 0 );
		} else {
			// TODO: how to deal with empty string ?
			//SetupPromptFrame( szEmptyIsNotAllow );
			//UI_StateTransitionToPrompt( UI_STATE_DIAL, PARAM_PROMPT_BACK );
		}
	} else if( key == VKEY_ON_HOOK ) {
		API_DisconnectCall();
		UI_StateTransition( UI_STATE_DISCONNECTION_WAIT, 0 );
	} else if( key == VKEY_CANCEL ) {
label_cancel_in_connection_dial:
		if( InConnectionDialSwitchBack( ( fTempFlags.b.connDialLine ? LSR_LINE1 : LSR_LINE2 ) ) )
			UI_StateTransition( UI_STATE_IN_CONNECTION, 0 );
		//fCallFlgas.b.multiline = 0;	// not necessary 
	} else if( key == VKEY_LINE1 && fTempFlags.b.connDialLine ) {
		goto label_cancel_in_connection_dial;
	} else if( key == VKEY_LINE2 && fTempFlags.b.connDialLine == 0 ) {
		goto label_cancel_in_connection_dial;
	} else if( key == VKEY_FXO ) {
		bFxo = 1;
		szDial[ 0 ] = '\x0';	/* TODO: give FXO a special phonenumber. */
		goto label_in_connection_do_2nd_outgoing_call;
#if 0
		sid = ( fTempFlags.b.connDialLine ? ( sid_t )1 : ( sid_t )0 );
		another_sid = ( sid ? ( sid_t )0 : ( sid_t )1 );
		
		pfLineFlags_another = &fLineFlags[ another_sid ];

		if( pfLineFlags_another ->b.fxo )
			debug_out( "Another is already FXO.\n" );
		else {
			fCallFlags.b.multiline = 1;		/* outgoing call in connection --> multi-lines */

			/* update disc. prompt timer of another line */
			if( fLineFlags[ another_sid ].b.discDisplay )
				sh.line[ another_sid ].nDiscPromptTimer = GetUptimeInMillisecond();

			DoFxoSwitchWithStateTransition( sid, 1 /* in connection */ );
		}
#endif
	}
}

void SetupDisconnectionWaitFrame( int param )
{
	SetupFrameWithCentralizedString( szDisconnecting );
}

void StateDisconnectionWaitOnKey( unsigned char key )	/* UI_STATE_DISCONNECTION_WAIT */
{
}

void DisconnectPromptTimerEvent( void )
{
	/* Automatically leave disonnection state, only if on-hook. */
	if( fHostFlags.b.hookStatus )
		return;

	if( CheckIfTimeoutInMillisecond( &sht.nDisconnectTimer, 
						DISCONNECT_PROMPT_PERIOD /* 5 seconds */ ) == 0 )
	{
		UI_StateTransition( UI_STATE_STANDBY, 0 );
	}
}

void SetupDisconnectionFrame( int param )
{
	unsigned long nSecConnectionPeriod;
	sid_t activate_sid;
	const fLineFlags_t *pfLineFlags;
	int y;

	GS_DrawOffScreenAndClearScreen();

	if( !fCallFlags.b.multiline ) {
		/* which one activate? */
		if( fLineFlags[ 0 ].b.active ) {
			activate_sid = 0;
			pfLineFlags = &fLineFlags[ 0 ];
		} else if( fLineFlags[ 1 ].b.active ) {
			activate_sid = 1;
			pfLineFlags = &fLineFlags[ 1 ];
		} else {
			debug_out( "SetupDisconnectionFrame can't find active line!?\n" );
			return;
		}

		GS_TextOut( 0, 0, szDisconnection, RES_strlen( szDisconnection ) );
		
		/* In this case, state cross connetion. */
		if( pfLineFlags ->b.connected )
			nSecConnectionPeriod = GetConnectionPeriodInUnitOfSecond( activate_sid );
		else
			nSecConnectionPeriod = 0;
			
		DrawTimerInUnitOfSecond( 0, 1, nSecConnectionPeriod );
	} else {
		/* multi-line */
		// FIXME: call transfer ?? cause a bad display?? 
		
		for( activate_sid = 0; activate_sid < CALL_LINE_NUM; activate_sid ++ ) {

			y = activate_sid;

			nSecConnectionPeriod = GetConnectionPeriodInUnitOfSecond( activate_sid );
				
			DrawTimerInUnitOfSecond( 0, y, nSecConnectionPeriod );
		}
	}

	GS_DrawOnScreen( NULL );

	/* Turn off LED */
	MM_StopLedBlinking();
#ifndef NO_LINE_LED
	TurnOffLEDThroughGPIO( LED_BIT_LINE1 | LED_BIT_LINE2 );
#endif

	/* line flags */
	fLineFlags[ 0 ].b.disconnected = 1;
	fLineFlags[ 1 ].b.disconnected = 1;

	fLineFlags[ 0 ].b.active = 0;
	fLineFlags[ 1 ].b.active = 0;
	
	sht.nDisconnectTimer = GetUptimeInMillisecond();
}

void StateDisconnectionOnKey( unsigned char key )	/* UI_STATE_DISCONNECTION */
{
	switch( key ) {
#if 0
	case VKEY_OK:
	case VKEY_CANCEL:
#endif
	case VKEY_SPEAKER:
		if( !fHostFlags.b.handfree )
			break;
	case VKEY_ON_HOOK:
		UI_StateTransition( UI_STATE_STANDBY, 0 );
		break;
	
	case VKEY_OFF_HOOK:
		UI_StateTransition( UI_STATE_STANDBY, PARAM_NOT_DRAW );
		DoEditDialNumberWithStateTransition( 0 /* not handfree */, 0 );
		break;
	}
}

static void RefreshIncomingCallFrame( const unsigned char *pszPhonenumber )
{
	unsigned short len = strlen( ( const char * )pszPhonenumber );

	GS_DrawOffScreenAndClearScreen();

	GS_TextOut( 0, 0, szIncomingCall, RES_strlen( szIncomingCall ) );
	if( len )
		GS_TextOut( 0, 1, pszPhonenumber, len );
	else
		GS_TextOut( 0, 1, szNoname, RES_strlen( szNoname ) );

	RightSoftkeyConfigurationWithInstructionText( VKEY_CANCEL, szInsReject );

	GS_DrawOnScreen( NULL );
}

static void AddIncomingOrMissedCallRecord( sid_t sid )
{
	const fLineFlags_t * const pfLineFlags = &fLineFlags[ sid ];
	const call_line_t * const pCallLine = &sh.line[ sid ];

	if( !pfLineFlags ->b.contact || !pfLineFlags ->b.incomingCall )
		return;

	if( pfLineFlags ->b.interactIncoming )	/* incoming call */
		AddRecordOfIncomingCall( pCallLine ->szCalledPhonenumber, pCallLine ->dtIncomingCall );
	else									/* missed call */
		AddRecordOfMissedCall( pCallLine ->szCalledPhonenumber, pCallLine ->dtIncomingCall );
}

/* Incoming call */
void API_IncomingCallToUI( sid_t sid, const unsigned char *pszPhonenumber )
{
	/* UI_EVENT_TYPE_INCOMING_CALL_IND */
	//sid_t sid;
	fLineFlags_t *pfLineFlags;
	int bInConnection;
	
	/* in connection, so choose an idle line */
	if( ( bInConnection = CheckStatesInConnection() ) ) {
		if( fLineFlags[ sid ].b.contact == 0 || 
			fLineFlags[ sid ].b.disconnected == 1 )
		{
			/* Not in use */
		} else {
			/* In use */
			debug_out( "Line (%lu) is in use.\n", sid );
			return;
		}
	} else {
		if( sid != IP_SID_INIT ) {
			debug_out( "Initial line should be 0.\n" );
			sid = IP_SID_INIT;
		}
	}
		
	pfLineFlags = &fLineFlags[ sid ];
	
	/* call flags */
	if( pfLineFlags ->b.disconnected ) {
		/* TODO: do somthing, if disconnected flag exist. */
	}
	
	pfLineFlags ->all = 0;	
	pfLineFlags ->b.contact = 1;
	pfLineFlags ->b.incomingCall = 1;	/* incoming call */

	strcpy( ( char * )sh.line[ sid ].szCalledPhonenumber, ( const char * )pszPhonenumber );
	sh.line[ sid ].dtIncomingCall = GetCurrentDateTime();

	/* Not normal UI interaction in connection state. */
	if( bInConnection ) {
		pfLineFlags ->b.waiting = 1;
		fCallFlags.b.multiline = 1;		/* incoming call in connection --> multi-lines */

		/* Draw timer frame */
		fCallFlags.b.volumePrompt = 0;	/* turn off volume prompt immediately */

		/* after set flags, re-draw connection frame */
		RefreshInConnectionNormalFrame();

		DisplayLedInConnectionRelyOnFlags();
		return;
	} else
		pfLineFlags ->b.active = 1;
	
	/* play tone */
	MM_StartTonePlayingEx( TONE_ID_IN_RING, 0 /* repeat */, 1 /* force */, 1 /* speaker out */ );

	// TODO: save current editing and so on, before setup incoming call frame.  

	UI_StateTransition( UI_STATE_INCOMING_CALL, 0 );

	RefreshIncomingCallFrame( pszPhonenumber );
}

/* In connection */
void API_ConnectionEstToUI( sid_t sid )
{
	/* UI_EVENT_TYPE_CONNECTION_EST_IND */
	fLineFlags_t * const pfLineFlags = &fLineFlags[ sid ];

	switch( uiState ) {
	case UI_STATE_OUTGOING_CALL:
	case UI_STATE_INCOMING_CALL_WAIT:
		ReadyToStartTimerInConnection( sid );
		//TurnOnLEDThroughGPIO( sid ? LED_BIT_LINE2 : LED_BIT_LINE1 );
		pfLineFlags ->b.connected = 1;
		StopPlayingAnimation();
		UI_StateTransition( UI_STATE_IN_CONNECTION, 0 );	/* Do state transition at the last action */
		break;

	case UI_STATE_IN_CONNECTION:
		if( pfLineFlags ->b.waiting && !pfLineFlags ->b.disconnected )
			;		/* call waiting */
		else if( pfLineFlags ->b.contact && !pfLineFlags ->b.incomingCall && !pfLineFlags ->b.disconnected ) {
			/* outgoing call in connection */
			StopPlayingAnimation();
		} else
			debug_out( "It shold not receive CONNECT EST.\n" );

		ReadyToStartTimerInConnection( sid );
		//TurnOnLEDThroughGPIO( sid ? LED_BIT_LINE2 : LED_BIT_LINE1 );
		pfLineFlags ->b.waiting = 0;
		pfLineFlags ->b.switching = 0;
		pfLineFlags ->b.connected = 1;

		/* after set flags, re-draw connection frame */
		RefreshInConnectionNormalFrame();

		DisplayLedInConnectionRelyOnFlags();
		break;

	case UI_STATE_IN_CONN_DIAL:
		debug_out( "In connection dial, recv. connection ind. is impossible.\n" );
		break;

	default:
		debug_out( "Not handle Connection Est.\n" );
		break;
	}
}

/* Disconnection */
void API_DisconnectToUI( sid_t sid )
{
	/* UI_EVENT_TYPE_DISCONNECT_IND */
	fLineFlags_t * const pfLineFlags = &fLineFlags[ sid ];
	fLineFlags_t * pfLineFlags_another;
	sid_t another_sid;

	switch( uiState ) {
	case UI_STATE_INCOMING_CALL:
		MM_StopTonePlaying();
		AddIncomingOrMissedCallRecord( sid );
	case UI_STATE_OUTGOING_CALL:
		/* only single line */
		UI_StateTransition( UI_STATE_DISCONNECTION, 0 );
		break;

	case UI_STATE_DISCONNECTION_WAIT:
	
		pfLineFlags ->b.disconnected = 1;
		pfLineFlags ->b.discDisplay = 1;
		
		//RefreshInConnectionNormalFrame();

		//DisplayLedInConnectionRelyOnFlags();

		/* wait for all lines disconnected */
		if( ( !fLineFlags[ 0 ].b.contact || fLineFlags[ 0 ].b.disconnected ) && 
			( !fLineFlags[ 1 ].b.contact || fLineFlags[ 1 ].b.disconnected ) )
		{
			UI_StateTransition( UI_STATE_DISCONNECTION, 0 );
		}
		break;

	case UI_STATE_IN_CONN_DIAL:
	case UI_STATE_IN_CONNECTION:
		//fCallFlags.b.multiline = 0;	/* We need to display disconnected status, and maybe outgoing call 2 is the disconnected line. */
		
		if( fCallFlags.b.conference ) {
			fCallFlags.b.conference = 0;
			pfLineFlags ->b.active = 0;
		}

		pfLineFlags ->b.disconnected = 1;
		pfLineFlags ->b.discDisplay = 1;
		sh.line[ sid ].nDiscPromptTimer = GetUptimeInMillisecond();

		StopPlayingAnimation();		/* outgoing call 2 animation */

		if( ( !fLineFlags[ 0 ].b.contact || fLineFlags[ 0 ].b.disconnected ) && 
			( !fLineFlags[ 1 ].b.contact || fLineFlags[ 1 ].b.disconnected ) )
		{
			/* No any connection */
			if( uiState == UI_STATE_IN_CONN_DIAL )
				;	/* While connection dial, original line is disconnected. */
			else
				UI_StateTransition( UI_STATE_DISCONNECTION, 0 );
		} else {
			another_sid = ( sid ? 0 : 1 );
			pfLineFlags_another = &fLineFlags[ another_sid ];

			/* after clear flags, re-draw connection frame */
			if( uiState == UI_STATE_IN_CONNECTION ) {

				if( pfLineFlags ->b.active ) {

					/* 
					 * If disconnect is due to fail outgoing call such as 'bad url', let it play  
					 * busy tone and user switch line to resume the hold one. 
					 */
					if( pfLineFlags ->b.contact && !pfLineFlags ->b.incomingCall &&
						!pfLineFlags ->b.connected )
					{
						/* do call transfer fail, and then resume this session. */
						if( fCallFlags.b.transfer ) {
							/* 
							 * In successful transfer, 1st line disconnection falls into this case too. 
							 * But don't worry, it will not affect 2nd line disconnection.  
							 */
							goto label_active_another_and_wait_for_resume;
						}
						
						/* 2nd outgoing call and not connected --> keep its flags */
					} else {
						/* active line disconnect --> it resumes another in period of disconnection frame display. */
label_active_another_and_wait_for_resume:

						pfLineFlags ->b.active = 0;
						pfLineFlags_another ->b.active = 1;
						pfLineFlags_another ->b.switching = 1;	/* unset after RESUME IND. */

						fCallFlags.b.transfer = 0;	/* usnet transfer flag */

						if( pfLineFlags_another ->b.hold && pfLineFlags_another ->b.held )
							;
						else
							debug_out( "Another line should be active hold!\n" );
					}
				}

				RefreshInConnectionNormalFrame();

				DisplayLedInConnectionRelyOnFlags();
			} else {	/* == UI_STATE_IN_CONN_DIAL */
				debug_out( "Unexpectation when UI_STATE_IN_CONN_DIAL recv. disconnect.\n" );
			}
		}
		AddIncomingOrMissedCallRecord( sid );
		break;

	default:
		debug_out( "Not handle Disconnect Ind.\n" );
		break;
	}
}

void API_LineHeldToUI( sid_t sid )
{
	/* UI_EVENT_TYPE_HELD_IND */
	fLineFlags_t * const pfLineFlags = &fLineFlags[ sid ];
	
	switch( uiState ) {
	case UI_STATE_IN_CONN_DIAL:
	case UI_STATE_IN_CONNECTION:
		/* set line flags */
		pfLineFlags ->b.held = 1;
		pfLineFlags ->b.switching = 0;
		
		/* if I was held, it will not change my status */
		if( !pfLineFlags ->b.hold )
			goto label_line_held_redraw_connection_frame;
		
		/* set call flags */
		fCallFlags.b.conference = 0;

label_line_held_redraw_connection_frame:
		/* after set flags, re-draw connection frame */
		if( uiState == UI_STATE_IN_CONNECTION )
			RefreshInConnectionNormalFrame();

		DisplayLedInConnectionRelyOnFlags();
		break;
		
	default:
		debug_out( "Not handle Line Held Ind.\n" );
		break;
	}
}

void API_LineResumeToUI( sid_t sid )
{
	/* UI_EVENT_TYPE_RESUME_IND */
	fLineFlags_t * const pfLineFlags = &fLineFlags[ sid ];
	fLineFlags_t * pfLineFlags_another = &fLineFlags[ ( sid ? 0 : 1 ) ];

	switch( uiState ) {
	case UI_STATE_IN_CONN_DIAL:
	case UI_STATE_IN_CONNECTION:
		if( !pfLineFlags ->b.held ) {
			debug_out( "Line(%lu) was NOT held!!\n", sid );
			break;
		}
		
		/* if I was held, it will not change my status */
		if( !pfLineFlags ->b.hold ) {
			pfLineFlags ->b.held = 0;
			goto label_line_resume_redraw_connection_frame;
		}
		
		/* set line flags */
		pfLineFlags ->b.hold = 0;
		pfLineFlags ->b.held = 0;
		pfLineFlags ->b.switching = 0;
		
		/* set call flags */
		if( fCallFlags.b.multiline ) {
			/* conference */
			if( pfLineFlags_another ->b.connected == 1 &&
				pfLineFlags_another ->b.disconnected == 0 &&
				pfLineFlags_another ->b.hold == 0 &&
				pfLineFlags_another ->b.held == 0 &&
				pfLineFlags_another ->b.switching == 0 )
			{
				fCallFlags.b.conference = 1;
			} else
				fCallFlags.b.conference = 0;
		}

label_line_resume_redraw_connection_frame:		
		/* after set flags, re-draw connection frame */
		if( uiState == UI_STATE_IN_CONNECTION )
			RefreshInConnectionNormalFrame();

		DisplayLedInConnectionRelyOnFlags();
		break;
		
	default:
		debug_out( "Not handle Line Resume Ind.\n" );
		break;
	}	
}

