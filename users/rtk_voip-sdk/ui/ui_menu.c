#include <stdio.h>
#include <string.h>
#ifndef _TEST_MODE
#include <sys/ioctl.h>
#include <net/if.h> 
#include <linux/in.h>
#include <unistd.h>
#endif
#include <stdlib.h>
#include "ui_config.h"
#include "ui_buildno.h"
#include "ui_menu.h"
#include "ui_host.h"
#include "ui_flags.h"
#include "ui_limits.h"
#include "ui_mode.h"
#include "ui_softkey.h"
#include "gs_lib.h"
#include "mm_animate.h"
#include "flash_layout.h"
#include "flash_rw_api.h"
#include "ioctl_net.h"
#include "voip_version.h"

unsigned long idPingRequest = 0;

void LoadSettingsFromStorage( int bLoadDefault )
{
	/* FlashValidateWriting() by caller */
}

void SetupMenuActFrame( int param )
{
	static const menu_item_t mainMenuItems[] = {
		{ szItemView /* "View" */, 0, NULL },
		{ szItemConfiguration /* "Configuration" */, 0, NULL },
		{ szItemPhonebook /* "Phonebook" */, 0, NULL },
#ifdef _DEBUG_MODE 
		{ szItemTestCase /* "Test Case" */, 0, NULL },
#endif
	};
	
	static const menu_select_t mainMenuSelect = {
		0,		/* attribute */
		sizeof( mainMenuItems ) / sizeof( mainMenuItems[ 0 ] ),	/* number of items */
		mainMenuItems,	/* point to items */
		NULL,	/* point to get item text function */
	};
	
	ActivateMenuSelection( &mainMenuSelect, param );
}

void StateMenuActOnKey( unsigned char key )	/* UI_STATE_MENU_ACT */
{
	/* Cancel key is processed by UI_KeypadInput() */
	if( key == VKEY_OK ) {
		nSelectedItem = DeactivateMenuSelection();

		switch( nSelectedItem ) {
		case MENU_ID_VIEW:
			UI_StateTransition( UI_STATE_MENU_VIEW, 0 );
			break;

		case MENU_ID_CONFIG:
			UI_StateTransition( UI_STATE_MENU_CONF, 0 );
			//SetupPromptFrame( ( const unsigned char * )"Not Implement yet." );
			//UI_StateTransitionToPrompt( UI_STATE_MENU_ACT, MENU_ID_CONFIG );
			break;

		case MENU_ID_PHONEBOOK:
			UI_StateTransitionWithBackHelp( UI_STATE_PHONEBOOK_LIST, 0, MENU_ID_PHONEBOOK );
			break;

		case MENU_ID_TEST_CASE:
			UI_StateTransition( UI_STATE_MENU_TEST_CASE, 0 );
			break;
		}
	}
}

void SetupMenuViewFrame( int param )
{
	static const menu_item_t viewMenuItems[] = {
		{ szItemNetworkSettings /* "Network Settings" */, 0, NULL },
		{ szItemPing /* "Ping" */, 0, NULL },
		{ szItemSoftwareVersion /* "Software Version" */, 0, NULL },
		{ szItemCallRecords /* "Call Records" */, 0, NULL },
	};
	
	static const menu_select_t viewMenuSelect = {
		0,		/* attribute */
		sizeof( viewMenuItems ) / sizeof( viewMenuItems[ 0 ] ),	/* number of items */
		viewMenuItems,	/* point to items */
		NULL,	/* point to get item text function */
	};
	
	ActivateMenuSelection( &viewMenuSelect, param );
}

void StateMenuViewOnKey( unsigned char key )	/* UI_STATE_MENU_VIEW */
{
	/* Cancel key is processed by UI_KeypadInput() */

	if( key == VKEY_OK ) {
		nSelectedItem = DeactivateMenuSelection();

		switch( nSelectedItem ) {
		case MENU_ID_VIEW_NET:
			UI_StateTransition( UI_STATE_MENU_VIEW_NET, 0 );
			break;

		case MENU_ID_VIEW_PING:
			UI_StateTransition( UI_STATE_MENU_VIEW_PING, 0 );
			break;

		case MENU_ID_VIEW_SOFT_VER:
			UI_StateTransition( UI_STATE_MENU_VIEW_SOFT_VER, 0 );
			break;

		case MENU_ID_VIEW_CALL_RECORDS:
			UI_StateTransition( UI_STATE_MENU_VIEW_CALL_RECORDS, 0 );
			break;

		default:
			SetupPromptFrame( ( const unsigned char * )"Not Implement yet." );
			UI_StateTransitionToPrompt( UI_STATE_MENU_VIEW, nSelectedItem );
			break;
		}
	}
}

void SetupMenuViewNetFrame( int param )
{
	static const menu_item_t viewNetMenuItems[] = {
		{ szItemIPAddress /* "IP Address" */, 0, NULL },
		{ szItemMask /* "Mask" */, 0, NULL },
		{ szItemGateway /* "Gateway" */, 0, NULL },
		{ szItemDNS /* "DNS" */, 0, NULL },
	};
	
	static const menu_select_t viewNetMenuSelect = {
		0,		/* attribute */
		sizeof( viewNetMenuItems ) / sizeof( viewNetMenuItems[ 0 ] ),	/* number of items */
		viewNetMenuItems,	/* point to items */
		NULL,	/* point to get item text function */
	};
	
	ActivateMenuSelection( &viewNetMenuSelect, param );
}

void StateMenuViewNetOnKey( unsigned char key )	/* UI_STATE_MENU_VIEW_NET */
{
	/* Cancel key is processed by UI_KeypadInput() */
	if( key == VKEY_OK ) {
		nSelectedItem = DeactivateMenuSelection();

		switch( nSelectedItem ) {
		case MENU_ID_VIEW_NET_IP:
			UI_StateTransition( UI_STATE_MENU_VIEW_NET_IP, 0 );
			break;
			
		case MENU_ID_VIEW_NET_NETMASK:
			UI_StateTransition( UI_STATE_MENU_VIEW_NET_NETMASK, 0 );
			break;
			
		case MENU_ID_VIEW_NET_GATEWAY:
			UI_StateTransition( UI_STATE_MENU_VIEW_NET_GATEWAY, 0 );
			break;
			
		case MENU_ID_VIEW_NET_DNS:
			UI_StateTransition( UI_STATE_MENU_VIEW_NET_DNS, 0 );
			break;
		}
	}
}

#ifndef _TEST_MODE
static void GetNetRelatedListItemText( unsigned int idxItem, unsigned char *pszItemText, int request )
{
    struct ifreq ifr;
    int skfd;
    struct sockaddr_in *addr;
    int index;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);

	switch( idxItem )  {
	case VIEW_NET_IP_ID_LAN:
		strcpy( ifr.ifr_name, "br0");
		strcpy( pszItemText, szColonLAN );
		index = RES_strlen( szColonLAN );
		break;
	case VIEW_NET_IP_ID_WAN:
		strcpy( ifr.ifr_name, "eth1");		
		strcpy( pszItemText, szColonWAN );
		index = RES_strlen( szColonWAN );
		break;
	default:
		strcpy( ( char * )pszItemText, ( const char * )szError );
		return;
	}	
	
	if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
		strcat( ( char * )pszItemText, ( const char * )szZerosIP );
		return;
	}

	if (ioctl(skfd, request, &ifr) == 0) {
		addr = ((struct sockaddr_in *)&ifr.ifr_addr);
		sprintf( ( char * )pszItemText + index, "%u.%u.%u.%u",
							*( ( unsigned char * )&addr->sin_addr + 0 ),
							*( ( unsigned char * )&addr->sin_addr + 1 ),
							*( ( unsigned char * )&addr->sin_addr + 2 ),
							*( ( unsigned char * )&addr->sin_addr + 3 ) );
	} else 
		strcpy( ( char * )pszItemText + index, szZerosIP );
	
	close( skfd );
}
#endif /* !_TEST_MODE */

static void GetNetIpListItemText( unsigned int idxItem, unsigned char *pszItemText )
{
#ifdef _TEST_MODE
	sprintf( ( char * )pszItemText, "LAN%d:192.168.123.123", idxItem );
#else
	GetNetRelatedListItemText( idxItem, pszItemText, SIOCGIFADDR );
#endif
}

void SetupMenuViewNetIpFrame( int param )
{
	static const menu_select_t viewNetIpSelect = {
		SELECT_ATTRIB_GET_TEXT_FUNC,		/* attribute */
		NUM_OF_VIEW_NET_IP,	/* number of items */
		NULL,	/* point to items */
		GetNetIpListItemText,	/* point to get item text function */
	};
	
	ActivateMenuSelection( &viewNetIpSelect, param );
}

void StateMenuViewNetIpOnKey( unsigned char key )	/* UI_STATE_MENU_VIEW_NET_IP */
{
	/* Cancel key is processed by UI_KeypadInput() */
}

static void GetNetMaskListItemText( unsigned int idxItem, unsigned char *pszItemText )
{
#ifdef _TEST_MODE
	sprintf( ( char * )pszItemText, "LAN%d:255.255.255.0", idxItem );
#else
	GetNetRelatedListItemText( idxItem, pszItemText, SIOCGIFNETMASK );
#endif
}

void SetupMenuViewNetNetmaskFrame( int param )
{
	static const menu_select_t viewNetMaskSelect = {
		SELECT_ATTRIB_GET_TEXT_FUNC,		/* attribute */
		NUM_OF_VIEW_NET_IP,	/* number of items */
		NULL,	/* point to items */
		GetNetMaskListItemText,	/* point to get item text function */
	};
	
	ActivateMenuSelection( &viewNetMaskSelect, param );
}

void StateMenuViewNetNetmaskOnKey( unsigned char key )	/* UI_STATE_MENU_VIEW_NET_NETMASK */
{
	/* Cancel key is processed by UI_KeypadInput() */
}

void SetupMenuViewNetGatewayFrame( int param )
{
#ifndef _TEST_MODE
	unsigned char szGateway[ MAX_LEN_OF_IP_ADDR + 1 ];
#endif

	GS_DrawOffScreenAndClearScreen();

	GS_TextOut( 0, 0, szGatewayPrompt, RES_strlen( szGatewayPrompt ) );

#ifndef _TEST_MODE
	if( GetDefaultGatewayText( szGateway ) )
		GS_TextOut( 0, 1, szGateway, strlen( szGateway ) );
	else
#endif
	{
		GS_TextOut( 0, 1, szZerosIP, RES_strlen( szZerosIP ) );
	}

	SoftkeyStandardConfiguration_Back();

	GS_DrawOnScreen( NULL );
}

void StateMenuViewNetGatewayOnKey( unsigned char key )	/* UI_STATE_MENU_VIEW_NET_GATEWAY */
{
	/* Cancel key is processed by UI_KeypadInput() */
}

void SetupMenuViewNetDnsFrame( int param )
{
#ifndef _TEST_MODE
	unsigned char szDNS[ MAX_LEN_OF_IP_ADDR + 1 ];
#endif

	GS_DrawOffScreenAndClearScreen();

	GS_TextOut( 0, 0, szDnsPrompt, RES_strlen( szDnsPrompt ) );
#ifndef _TEST_MODE
	if( GetFirstDNSText( szDNS ) )
		GS_TextOut( 0, 1, szDNS, strlen( szDNS ) );
	else
#endif
	{
		GS_TextOut( 0, 1, szZerosIP, RES_strlen( szZerosIP ) );
	}

	SoftkeyStandardConfiguration_Back();

	GS_DrawOnScreen( NULL );
}

void StateMenuViewNetDnsOnKey( unsigned char key )	/* UI_STATE_MENU_VIEW_NET_DNS */
{
	/* Cancel key is processed by UI_KeypadInput() */
}

void SetupMenuViewSoftVerFrame( int param )
{
#define SOFTVER_VER_NUM		2

#if 1
	int i;

	for( i = 0; i < SOFTVER_VER_NUM; i ++ ) {
		DynamicMenuItem[ i ].fMenuItemAttrib = 0;
		DynamicMenuItem[ i ]. pFnItemGetItemGeneral = NULL;
	}

	sprintf( ( char * )DynamicMenuItem[ 0 ].pszItemText, "UI: " UI_SOFT_VER " b%lu", buildno );
	sprintf( ( char * )DynamicMenuItem[ 1 ].pszItemText, "VoIP: " VOIP_VERSION " b%lu", buildno_kernel );
#else
	static const menu_item_t viewSoftVerMenuItems[] = {
		{ ( const unsigned char * )"UI: " UI_SOFT_VER, 0, NULL },
		{ ( const unsigned char * )"VoIP: " VOIP_VERSION, 0, NULL },
	};
#endif
	
	static const menu_select_t viewSoftVerMenuSelect = {
		0,		/* attribute */
#if 1
		SOFTVER_VER_NUM,	/* number of items */
		DynamicMenuItem,	/* point to items */
#else
		sizeof( viewSoftVerMenuItems ) / sizeof( viewSoftVerMenuItems[ 0 ] ),	/* number of items */
		viewSoftVerMenuItems,	/* point to items */
#endif
		NULL,	/* point to get item text function */
	};
	
	ActivateMenuSelection( &viewSoftVerMenuSelect, param );
#undef SOFTVER_VER_NUM
}

void StateMenuViewSoftVerOnKey( unsigned char key )	/* UI_STATE_MENU_VIEW_SOFT_VER */
{
	/* Cancel key is processed by UI_KeypadInput() */

}

void SetupMenuViewPingFrame( int param )
{
	params_for_single_line_editor_t editor_params;

	GS_DrawOffScreenAndClearScreen();

	GS_TextOut( 0, 0, szInputIpPrompt, RES_strlen( szInputIpPrompt ) );

	if( 1 ) {
		editor_params.nDefaultTextLength = 0;
	} else {
		editor_params.nDefaultTextLength = 1;
		editor_params.pszDefaultText = ( unsigned char * )NULL;
	}
	editor_params.nMaxTextLength = MAX_LEN_OF_IP_ADDR;
	editor_params.tTextType = TEXT_TYPE_IP;
	editor_params.fParams.all = 0;

	ActivateTextEditor( EDITOR_ID_SINGLE_LINE, ( params_for_editor_t * )&editor_params );

	SoftkeyStandardConfiguration_OkBack();

	GS_DrawOnScreen( NULL );	
	GS_SetCursorStatus( CURSOR_STATUS_ON );
}

void StateMenuViewPingOnKey( unsigned char key )	/* UI_STATE_MENU_VIEW_PING */
{
	/* Cancel key is processed by UI_KeypadInput() */
	extern unsigned long AnnouncePingRequestThread( const unsigned char *pszIp );
	unsigned char szIpAddr[ MAX_LEN_OF_IP_ADDR + 1 ];

	if( key == VKEY_OK ) {
		if( GetTextEditorTextContent( szIpAddr, MAX_LEN_OF_IP_ADDR ) ) {
			fHostFlags.b.pingRequest = 1;
			UI_StateTransition( UI_STATE_MENU_VIEW_PING_WAIT, ( int )( unsigned int )szIpAddr );
			idPingRequest = AnnouncePingRequestThread( szIpAddr );
		}
	}
}

static void DrawPingAckResultAndUnsetFlag( unsigned long result )
{
	int x = RES_strlen( szPing ) + 3;		/* +3 for waiting dot animation */

	/* unset flag */
	fHostFlags.b.pingRequest = 0;

	/* draw result */
	GS_DrawOffScreen();

	if( result )
		GS_TextOut( x, 0, szOK, RES_strlen( szOK ) );
	else
		GS_TextOut( x, 0, szFail, RES_strlen( szFail ) );

	GS_DrawOnScreen( NULL );
}

void API_PingAckToUI( unsigned long idPing, unsigned long result )
{
	if( uiState == UI_STATE_MENU_VIEW_PING_WAIT ) {
		if( idPingRequest == idPing ) {
			DrawPingAckResultAndUnsetFlag( result );
		} else {
			debug_out( "Ping request ID not match!(%lu,%lu)\n", idPingRequest, idPing );
		}
	} else {
		debug_out( "Not handle ping ack.\n" );
	}

	debug_out( "ping result: %lu\n", result );
}

void SetupMenuViewPingWaitFrame( int param )
{
	int len;
	const unsigned char * const pszIpAddr = ( const unsigned char * )( unsigned int )param;

	/* draw prompt */
	GS_DrawOffScreenAndClearScreen();

	GS_TextOut( 0, 0, szPing, ( len = RES_strlen( szPing ) ) );
	GS_TextOut( 0, 1, pszIpAddr, strlen( ( const char * )pszIpAddr ) );

	SoftkeyStandardConfiguration_Back();

	GS_DrawOnScreen( NULL );

	/* start animation */
	StartPlayingAnimation( len, 0, ANI_ID_WAITING_DOT );
}

void StateMenuViewPingWaitOnKey( unsigned char key )	/* UI_STATE_MENU_VIEW_PING_WAIT */
{
	/* Cancel key is processed by UI_KeypadInput() */
}

static isItemStatus_t ConfigKeypressToneGetItemStatus( ioItemOperate_t ioItemOperate )
{
	if( ioItemOperate == ITEM_OPER_GET_VALUE ) {
	} else {	/* ioItemOperate == ITEM_OPER_SWITCH_VALUE */
		if( fModeFlags.b.keypressTone )
			fModeFlags.b.keypressTone = 0;
		else
			fModeFlags.b.keypressTone = 1;
			
		FlashWriteOneByte( MY_FIELD_OFFSET( ui_falsh_layout_t, mode.bKeypressTone ),
						   fModeFlags.b.keypressTone );
	
		FlashValidateWriting( 0 );
	}

	return ( fModeFlags.b.keypressTone ? ITEM_STATUS_TRUE : ITEM_STATUS_FALSE );
}

static void ConfigOutVolumeXxxGetItemText( unsigned char *pszItemText, 
										   inout_vol_type_t outvol, 
										   const unsigned char *pszVolume )
{
	unsigned char szVolumeQuantity[ 6 ];	/* [xxx] */
	unsigned int lenVolQuantity, lenVolText;
	int padding, i;

	sprintf( ( char * )szVolumeQuantity, "[%u]", nModeOutVolume[ outvol ] );
	lenVolQuantity = strlen( ( const char * )szVolumeQuantity );

	strcpy( ( char * )pszItemText, ( const char * )pszVolume );
	lenVolText = strlen( ( const char * )pszVolume );

	padding = VRAM_WIDTH_IN_TEXT_UNIT - MENU_ITEM_INDICATOR - lenVolQuantity - lenVolText;	/* -1 for cursor */
	
	if( padding < 0 )
		padding = 0;

	for( i = 0; i < padding; i ++ )
		pszItemText[ lenVolText + i ] = ' ';

	strcpy( ( char * )&pszItemText[ lenVolText + i ], ( const char * )szVolumeQuantity );
}

static void ConfigOutVolumeReceiverGetItemText( unsigned char *pszItemText )
{
	ConfigOutVolumeXxxGetItemText( pszItemText, INOUTVOL_TYPE_RECEIVER, szVolumeReceiver );
}

static void ConfigOutVolumeSpeakerGetItemText( unsigned char *pszItemText )
{
	ConfigOutVolumeXxxGetItemText( pszItemText, INOUTVOL_TYPE_SPEAKER, szVolumeSpeaker );
}

static void ConfigInVolumeMicRGetItemText( unsigned char *pszItemText )
{
	ConfigOutVolumeXxxGetItemText( pszItemText, INOUTVOL_TYPE_MIC_R, szVolumeMicR );
}

static void ConfigInVolumeMicSGetItemText( unsigned char *pszItemText )
{
	ConfigOutVolumeXxxGetItemText( pszItemText, INOUTVOL_TYPE_MIC_S, szVolumeMicS );
}

static isItemStatus_t ConfigAutoDialTimeGetItemStatus( ioItemOperate_t ioItemOperate )
{
	unsigned char nAutoDialTime;
	
	FlashReadData( MY_FIELD_OFFSET( ui_falsh_layout_t, mode.nAutoDialTime ),
						&nAutoDialTime, sizeof( nAutoDialTime ) );

	if( ioItemOperate == ITEM_OPER_GET_VALUE ) {
	} else {	/* ioItemOperate == ITEM_OPER_SWITCH_VALUE */
		if( fModeFlags.b.autoDial ) {
			fModeFlags.b.autoDial = 0;

			nAutoDialTime = 0;
			FlashWriteData( MY_FIELD_OFFSET( ui_falsh_layout_t, mode.nAutoDialTime ),
						&nAutoDialTime, sizeof( nAutoDialTime ) );

			FlashValidateWriting( 0 );
		} else {
			/* StateMenuConfigOnKey() process this ok key */
			return ITEM_STATUS_NOT_FEED_KEY;
		}
	}

	return ( fModeFlags.b.autoDial ? ( isItemStatus_t )( nAutoDialTime /* 3 ~ 9 */ ) : ITEM_STATUS_FALSE );
}

static isItemStatus_t ConfigAutoAnswerTimeGetItemStatus( ioItemOperate_t ioItemOperate )
{
	unsigned char nAutoAnswerTime;
	
	FlashReadData( MY_FIELD_OFFSET( ui_falsh_layout_t, mode.nAutoAnswerTime ),
						&nAutoAnswerTime, sizeof( nAutoAnswerTime ) );

	if( ioItemOperate == ITEM_OPER_GET_VALUE ) {
	} else {	/* ioItemOperate == ITEM_OPER_SWITCH_VALUE */
		if( fModeFlags.b.autoAnswer ) {
			fModeFlags.b.autoAnswer = 0;

			nAutoAnswerTime = 0;
			FlashWriteData( MY_FIELD_OFFSET( ui_falsh_layout_t, mode.nAutoAnswerTime ),
						&nAutoAnswerTime, sizeof( nAutoAnswerTime ) );

			FlashValidateWriting( 0 );
		} else {
			/* StateMenuConfigOnKey() process this ok key */
			return ITEM_STATUS_NOT_FEED_KEY;
		}
	}

	return ( fModeFlags.b.autoAnswer ? ( isItemStatus_t )( nAutoAnswerTime /* 3 ~ 9 */ ) : ITEM_STATUS_FALSE );
}

static isItemStatus_t ConfigOffHookAlarmTimeGetItemStatus( ioItemOperate_t ioItemOperate )
{
	unsigned char nOffHookAlarmTime;
	
	FlashReadData( MY_FIELD_OFFSET( ui_falsh_layout_t, mode.nOffHookAlarmTime ),
						&nOffHookAlarmTime, sizeof( nOffHookAlarmTime ) );

	if( ioItemOperate == ITEM_OPER_GET_VALUE ) {
	} else {	/* ioItemOperate == ITEM_OPER_SWITCH_VALUE */
		if( fModeFlags.b.offHookAlarm ) {
			fModeFlags.b.offHookAlarm = 0;

			nOffHookAlarmTime = 0;
			FlashWriteData( MY_FIELD_OFFSET( ui_falsh_layout_t, mode.nOffHookAlarmTime ),
						&nOffHookAlarmTime, sizeof( nOffHookAlarmTime ) );

			FlashValidateWriting( 0 );
		} else {
			/* StateMenuConfigOnKey() process this ok key */
			return ITEM_STATUS_NOT_FEED_KEY;
		}
	}

	return ( fModeFlags.b.offHookAlarm ? ( isItemStatus_t )( nOffHookAlarmTime /* 10 ~ 60 */ ) : ITEM_STATUS_FALSE );
}

static isItemStatus_t ConfigHotLineGetItemStatus( ioItemOperate_t ioItemOperate )
{
	unsigned char bHotLine;
	
	if( ioItemOperate == ITEM_OPER_GET_VALUE ) {
	} else {	/* ioItemOperate == ITEM_OPER_SWITCH_VALUE */
		if( fModeFlags.b.hotLine ) {
			fModeFlags.b.hotLine = 0;

			bHotLine = 0;
			FlashWriteData( MY_FIELD_OFFSET( ui_falsh_layout_t, mode.bHotLine ),
						&bHotLine, sizeof( bHotLine ) );

			FlashValidateWriting( 0 );
		} else {
			/* StateMenuConfigOnKey() process this ok key */
			return ITEM_STATUS_NOT_FEED_KEY;
		}
	}

	return ( fModeFlags.b.hotLine ? ITEM_STATUS_TRUE : ITEM_STATUS_FALSE );
}

void SetupMenuConfigFrame( int param )
{
	static const menu_item_t configMenuItems[] = {
		{ szItemKeypressTone /* "KeypressTone" */, ITEM_ATT_OWNER_HANDLE_OK_KEY, ConfigKeypressToneGetItemStatus },
		{ NULL /* "ReceVolume" */, ITEM_ATT_GET_TEXT_FUNC, ConfigOutVolumeReceiverGetItemText },	/* old style coding */
		{ NULL /* "Spk Volume" */, ITEM_ATT_GET_TEXT_FUNC, ConfigOutVolumeSpeakerGetItemText },
		{ NULL /* "Mic(R) Vol" */, ITEM_ATT_GET_TEXT_FUNC, ConfigInVolumeMicRGetItemText },	/* old style coding */
		{ NULL /* "Mic(S) Vol" */, ITEM_ATT_GET_TEXT_FUNC, ConfigInVolumeMicSGetItemText },
		{ szAutoDial, ITEM_ATT_OWNER_HANDLE_OK_KEY, ConfigAutoDialTimeGetItemStatus },				/* new style coding */
		{ szAutoAnswer, ITEM_ATT_OWNER_HANDLE_OK_KEY, ConfigAutoAnswerTimeGetItemStatus },				/* new style coding */
		{ szOffHookAlarm, ITEM_ATT_OWNER_HANDLE_OK_KEY, ConfigOffHookAlarmTimeGetItemStatus },
		{ szHotLine, ITEM_ATT_OWNER_HANDLE_OK_KEY, ConfigHotLineGetItemStatus },
	};
	
	static const menu_select_t configMenuSelect = {
		0,		/* attribute */
		sizeof( configMenuItems ) / sizeof( configMenuItems[ 0 ] ),	/* number of items */
		configMenuItems,	/* point to items */
		NULL,	/* point to get item text function */
	};
	
	ActivateMenuSelection( &configMenuSelect, param );	
}

void StateMenuConfigOnKey( unsigned char key )	/* UI_STATE_MENU_CONF */
{
	/* Cancel key is processed by UI_KeypadInput() */
	if( key == VKEY_OK ) {
		nSelectedItem = DeactivateMenuSelection();

		switch( nSelectedItem ) {
		case CONFIG_ID_KEYPRESS_TONE:
			debug_out( "Keypress tone should not have OnKey().\n" );
			break;

		case CONFIG_ID_OUTVOL_RECEIVER:
			UI_StateTransition( UI_STATE_MENU_CONF_OUTVOL_RECEIVER, 0 );
			break;
			
		case CONFIG_ID_OUTVOL_SPEAKER:
			UI_StateTransition( UI_STATE_MENU_CONF_OUTVOL_SPEAKER, 0 );
			break;

		case CONFIG_ID_INVOL_MIC_R:
			UI_StateTransition( UI_STATE_MENU_CONF_INVOL_MIC_R, 0 );
			break;

		case CONFIG_ID_INVOL_MIC_S:
			UI_StateTransition( UI_STATE_MENU_CONF_INVOL_MIC_S, 0 );
			break;

		case CONFIG_ID_AUTO_DIAL_TIME:
			UI_StateTransition( UI_STATE_MENU_CONF_AUTO_DIAL_TIME, 0 );
			break;

		case CONFIG_ID_AUTO_ANSWER_TIME:
			UI_StateTransition( UI_STATE_MENU_CONF_AUTO_ANSWER_TIME, 0 );
			break;

		case CONFIG_ID_OFF_HOOK_ALARM_TIME:
			UI_StateTransition( UI_STATE_MENU_CONF_OFF_HOOK_ALARM_TIME, 0 );
			break;

		case CONFIG_ID_HOT_LINE:
			UI_StateTransition( UI_STATE_MENU_CONF_HOT_LINE, 0 );
			break;
		}
	}
}

void SetupMenuConfigOutVolumeReceiverFrame( int param )
{
	extern void HelpSetupMenuConfigOutVolumeXxxFrame( inout_vol_type_t type );
	
	HelpSetupMenuConfigOutVolumeXxxFrame( INOUTVOL_TYPE_RECEIVER );
}

void StateMenuConfigOutVolumeReceiverOnKey( unsigned char key )	/* UI_STATE_MENU_CONF_OUTVOL_RECEIVER */
{
	/* Cancel key is processed by UI_KeypadInput() */
	extern int HelpStateMenuConfigOutVolumeXxxOnKey( unsigned char key, inout_vol_type_t type );

	if( HelpStateMenuConfigOutVolumeXxxOnKey( key, INOUTVOL_TYPE_RECEIVER ) ) {
	
		SetupPromptFrame( szOK );
		UI_StateTransitionToPrompt( UI_STATE_MENU_CONF, CONFIG_ID_OUTVOL_RECEIVER );
	
		FlashWriteOneByte( MY_FIELD_OFFSET( ui_falsh_layout_t, mode.nModeOutVolumeReceiver ),
							nModeOutVolume[ INOUTVOL_TYPE_RECEIVER ] );
	
		FlashValidateWriting( 0 );
	}
}

void SetupMenuConfigOutVolumeSpeakerFrame( int param )
{
	extern void HelpSetupMenuConfigOutVolumeXxxFrame( inout_vol_type_t type );
	
	HelpSetupMenuConfigOutVolumeXxxFrame( INOUTVOL_TYPE_SPEAKER );
}

void StateMenuConfigOutVolumeSpeakerOnKey( unsigned char key )	/* UI_STATE_MENU_CONF_OUTVOL_SPEAKER */
{
	/* Cancel key is processed by UI_KeypadInput() */
	extern int HelpStateMenuConfigOutVolumeXxxOnKey( unsigned char key, inout_vol_type_t type );

	if( HelpStateMenuConfigOutVolumeXxxOnKey( key, INOUTVOL_TYPE_SPEAKER ) ) {
	
		SetupPromptFrame( szOK );
		UI_StateTransitionToPrompt( UI_STATE_MENU_CONF, CONFIG_ID_OUTVOL_SPEAKER );

		FlashWriteOneByte( MY_FIELD_OFFSET( ui_falsh_layout_t, mode.nModeOutVolumeSpeaker ),
							nModeOutVolume[ INOUTVOL_TYPE_SPEAKER ] );
	
		FlashValidateWriting( 0 );
	}
}

void SetupMenuConfigInVolumeMicRFrame( int param )
{
	extern void HelpSetupMenuConfigOutVolumeXxxFrame( inout_vol_type_t type );
	
	HelpSetupMenuConfigOutVolumeXxxFrame( INOUTVOL_TYPE_MIC_R );
}

void StateMenuConfigInVolumeMicROnKey( unsigned char key )	/* UI_STATE_MENU_CONF_INVOL_MIC_R */
{
	/* Cancel key is processed by UI_KeypadInput() */
	extern int HelpStateMenuConfigOutVolumeXxxOnKey( unsigned char key, inout_vol_type_t type );

	if( HelpStateMenuConfigOutVolumeXxxOnKey( key, INOUTVOL_TYPE_MIC_R ) ) {
	
		SetupPromptFrame( szOK );
		UI_StateTransitionToPrompt( UI_STATE_MENU_CONF, CONFIG_ID_INVOL_MIC_R );

		FlashWriteOneByte( MY_FIELD_OFFSET( ui_falsh_layout_t, mode.nModeInVolumeMic_R ),
							nModeOutVolume[ INOUTVOL_TYPE_MIC_R ] );
	
		FlashValidateWriting( 0 );
	}
}

void SetupMenuConfigInVolumeMicSFrame( int param )
{
	extern void HelpSetupMenuConfigOutVolumeXxxFrame( inout_vol_type_t type );
	
	HelpSetupMenuConfigOutVolumeXxxFrame( INOUTVOL_TYPE_MIC_S );
}

void StateMenuConfigInVolumeMicSOnKey( unsigned char key )	/* UI_STATE_MENU_CONF_INVOL_MIC_S */
{
	/* Cancel key is processed by UI_KeypadInput() */
	extern int HelpStateMenuConfigOutVolumeXxxOnKey( unsigned char key, inout_vol_type_t type );

	if( HelpStateMenuConfigOutVolumeXxxOnKey( key, INOUTVOL_TYPE_MIC_S ) ) {
	
		SetupPromptFrame( szOK );
		UI_StateTransitionToPrompt( UI_STATE_MENU_CONF, CONFIG_ID_INVOL_MIC_S );

		FlashWriteOneByte( MY_FIELD_OFFSET( ui_falsh_layout_t, mode.nModeInVolumeMic_S ),
							nModeOutVolume[ INOUTVOL_TYPE_MIC_S ] );
	
		FlashValidateWriting( 0 );
	}
}

static void HelpSetupMenuConfigForSingleLineEditorFrame( const unsigned char *pszPromptText,
														 int nPromptTextLen,
														 const unsigned char *pszRangeText,
														 int nRangeTextLen,
														 int nEditorBufferLen )
{
	params_for_single_line_editor_t edit_params;

	GS_DrawOffScreenAndClearScreen();

	/* Draw title */
	GS_TextOut( 0, 0, pszPromptText, nPromptTextLen );
	GS_TextOut( VRAM_WIDTH_IN_TEXT_UNIT - nRangeTextLen, 1, pszRangeText, nRangeTextLen );

	/* Activate editor */
	edit_params.nDefaultTextLength = 0;
	edit_params.pszDefaultText = NULL;
	edit_params.nMaxTextLength = nEditorBufferLen;
	edit_params.tTextType = TEXT_TYPE_NORMAL;
	edit_params.fParams.all = 0;

	ActivateTextEditor( EDITOR_ID_SINGLE_LINE, ( const params_for_editor_t * )&edit_params );

	SoftkeyStandardConfiguration_OkBack();

	GS_DrawOnScreen( NULL );
	GS_SetCursorStatus( CURSOR_STATUS_ON );
}

#if MAX_AUTO_DIAL_TIME > 9
  #define TXT_LEN_AUTO_DIAL_TIME	2
#else
  #define TXT_LEN_AUTO_DIAL_TIME	1
#endif

void SetupMenuConfigAutoDialTimeFrame( int param )
{
	HelpSetupMenuConfigForSingleLineEditorFrame( szInputAutoDialPrompt, RES_strlen( szInputAutoDialPrompt ),
												 szInputAutoDialRange, RES_strlen( szInputAutoDialRange ),
												 TXT_LEN_AUTO_DIAL_TIME );
}

void StateMenuConfigAutoDialTimeOnKey( unsigned char key )	/* UI_STATE_MENU_CONF_AUTO_DIAL_TIME */
{
	/* Cancel key is processed by UI_KeypadInput() */
	unsigned char szAutoDialTime[ TXT_LEN_AUTO_DIAL_TIME + 1 ];
	unsigned char nAutoDialTime;

	if( key == VKEY_OK ) {
		if( GetTextEditorTextContent( szAutoDialTime, TXT_LEN_AUTO_DIAL_TIME ) ) {
			/* check range */
			nAutoDialTime = ( unsigned char )atoi( ( const char * )szAutoDialTime );
			if( nAutoDialTime < MIN_AUTO_DIAL_TIME || nAutoDialTime > MAX_AUTO_DIAL_TIME )
				goto label_invalid_auto_dial_time;

			/* write to flash */
			FlashWriteData( MY_FIELD_OFFSET( ui_falsh_layout_t, mode.nAutoDialTime ),
						&nAutoDialTime, sizeof( nAutoDialTime ) );

			FlashValidateWriting( 0 );

			/* flag */
			fModeFlags.b.autoDial = 1;

			/* State Transition */
			SetupPromptFrame( szOK );
			UI_StateTransitionToPrompt( UI_STATE_MENU_CONF, CONFIG_ID_AUTO_DIAL_TIME );

		} else {
label_invalid_auto_dial_time:
			SetupPromptFrame( szInvalidValue );
			UI_StateTransitionToPrompt( UI_STATE_MENU_CONF_AUTO_DIAL_TIME, 0 );
		}
	}
}

#undef TXT_LEN_AUTO_DIAL_TIME

#if MAX_AUTO_ANSWER_TIME > 9
  #define TXT_LEN_AUTO_ANSWER_TIME	2
#else
  #define TXT_LEN_AUTO_ANSWER_TIME	1
#endif

void SetupMenuConfigAutoAnswerTimeFrame( int param )
{
	HelpSetupMenuConfigForSingleLineEditorFrame( szInputAutoAnswerPrompt, RES_strlen( szInputAutoAnswerPrompt ),
												 szInputAutoAnswerRange, RES_strlen( szInputAutoAnswerRange ),
												 TXT_LEN_AUTO_ANSWER_TIME );
}

void StateMenuConfigAutoAnswerTimeOnKey( unsigned char key )	/* UI_STATE_MENU_CONF_AUTO_ANSWER_TIME */
{
	/* Cancel key is processed by UI_KeypadInput() */
	unsigned char szAutoAnswerTime[ TXT_LEN_AUTO_ANSWER_TIME + 1 ];
	unsigned char nAutoAnswerTime;

	if( key == VKEY_OK ) {
		if( GetTextEditorTextContent( szAutoAnswerTime, TXT_LEN_AUTO_ANSWER_TIME ) ) {
			/* check range */
			nAutoAnswerTime = ( unsigned char )atoi( ( const char * )szAutoAnswerTime );
			if( nAutoAnswerTime < MIN_AUTO_ANSWER_TIME || nAutoAnswerTime > MAX_AUTO_ANSWER_TIME )
				goto label_invalid_auto_answer_time;

			/* write to flash */
			FlashWriteData( MY_FIELD_OFFSET( ui_falsh_layout_t, mode.nAutoAnswerTime ),
						&nAutoAnswerTime, sizeof( nAutoAnswerTime ) );

			FlashValidateWriting( 0 );

			/* flag */
			fModeFlags.b.autoAnswer = 1;

			/* State Transition */
			SetupPromptFrame( szOK );
			UI_StateTransitionToPrompt( UI_STATE_MENU_CONF, CONFIG_ID_AUTO_ANSWER_TIME );

		} else {
label_invalid_auto_answer_time:
			SetupPromptFrame( szInvalidValue );
			UI_StateTransitionToPrompt( UI_STATE_MENU_CONF_AUTO_ANSWER_TIME, 0 );
		}
	}
}

#undef TXT_LEN_AUTO_ANSWER_TIME

#if MIN_OFF_HOOK_ALARM_TIME >= 10 && MAX_OFF_HOOK_ALARM_TIME <= 99
  #define TXT_LEN_OFF_HOOK_ALARM_TIME	2
#else
  #error "give a suitable TXT_LEN_OFF_HOOK_ALARM_TIME"
#endif

void SetupMenuConfigOffHookAlarmTimeFrame( int param )
{
	params_for_single_line_editor_t edit_params;

	GS_DrawOffScreenAndClearScreen();

	/* Draw title */
	GS_TextOut( 0, 0, szInputOffHookAlarmPrompt, RES_strlen( szInputOffHookAlarmPrompt ) );
	GS_TextOut( VRAM_WIDTH_IN_TEXT_UNIT - RES_strlen( szInputOffHookAlarmRange ), 1, szInputOffHookAlarmRange, RES_strlen( szInputOffHookAlarmRange ) );

	/* Activate editor */
	edit_params.nDefaultTextLength = 0;
	edit_params.pszDefaultText = NULL;
	edit_params.nMaxTextLength = TXT_LEN_OFF_HOOK_ALARM_TIME;
	edit_params.tTextType = TEXT_TYPE_NORMAL;
	edit_params.fParams.all = 0;

	ActivateTextEditor( EDITOR_ID_SINGLE_LINE, ( const params_for_editor_t * )&edit_params );

	SoftkeyStandardConfiguration_OkBack();

	GS_DrawOnScreen( NULL );
	GS_SetCursorStatus( CURSOR_STATUS_ON );
}

void StateMenuConfigOffHookAlarmTimeOnKey( unsigned char key )	/* UI_STATE_MENU_CONF_OFF_HOOK_ALARM_TIME */
{
	/* Cancel key is processed by UI_KeypadInput() */
	unsigned char szOffHookAlarmTime[ TXT_LEN_OFF_HOOK_ALARM_TIME + 1 ];
	unsigned char nOffHookAlarmTime;

	if( key == VKEY_OK ) {
		if( GetTextEditorTextContent( szOffHookAlarmTime, TXT_LEN_OFF_HOOK_ALARM_TIME ) ) {
			/* check range */
			nOffHookAlarmTime = ( unsigned char )atoi( ( const char * )szOffHookAlarmTime );
			if( nOffHookAlarmTime < MIN_OFF_HOOK_ALARM_TIME || nOffHookAlarmTime > MAX_OFF_HOOK_ALARM_TIME )
				goto label_invalid_off_hook_alarm_time;

			/* write to flash */
			FlashWriteData( MY_FIELD_OFFSET( ui_falsh_layout_t, mode.nOffHookAlarmTime ),
						&nOffHookAlarmTime, sizeof( nOffHookAlarmTime ) );

			FlashValidateWriting( 0 );

			/* flag */
			fModeFlags.b.offHookAlarm = 1;

			/* State Transition */
			SetupPromptFrame( szOK );
			UI_StateTransitionToPrompt( UI_STATE_MENU_CONF, CONFIG_ID_OFF_HOOK_ALARM_TIME );

		} else {
label_invalid_off_hook_alarm_time:
			SetupPromptFrame( szInvalidValue );
			UI_StateTransitionToPrompt( UI_STATE_MENU_CONF_OFF_HOOK_ALARM_TIME, 0 );
		}
	}
}

#undef TXT_LEN_OFF_HOOK_ALARM_TIME

void SetupMenuConfigHotLineFrame( int param )
{
	params_for_single_line_editor_t edit_params;
	unsigned char bcdHotLine[ BCD_LEN_OF_PHONE_NUMBER ];

	FlashReadData( MY_FIELD_OFFSET( ui_falsh_layout_t, mode.bcdHotLine ),
				bcdHotLine, sizeof( bcdHotLine ) );

	/* Setup frame */
	GS_DrawOffScreenAndClearScreen();

	/* Draw Title */
	GS_TextOut( 0, 0, szHotLinePrompt, RES_strlen( szHotLinePrompt ) );

	/* Activate editor */
	edit_params.nDefaultTextLength = 0;		/* ignore if BCD */
	edit_params.pszDefaultText = ( param == PARAM_SPECIAL ) ? NULL : bcdHotLine;
	edit_params.nMaxTextLength = MAX_LEN_OF_PHONE_NUMBER;
	edit_params.tTextType = TEXT_TYPE_BCD;
	edit_params.fParams.all = 0;

	ActivateTextEditor( EDITOR_ID_SINGLE_LINE, ( const params_for_editor_t * )&edit_params );

	SoftkeyStandardConfiguration_OkBack();

	GS_DrawOnScreen( NULL );
	GS_SetCursorStatus( CURSOR_STATUS_ON );
}

void StateMenuConfigHotLineOnKey( unsigned char key )	/* UI_STATE_MENU_CONF_HOT_LINE */
{
	/* Cancel key is processed by UI_KeypadInput() */
	unsigned char bcdHotLine[ BCD_LEN_OF_PHONE_NUMBER ];
	unsigned char bHotLine;

	if( key == VKEY_OK ) {
		if( GetTextEditorTextContent( bcdHotLine, BCD_LEN_OF_PHONE_NUMBER ) == 0 ) {
			SetupPromptFrame( szEmptyIsNotAllow );
			UI_StateTransitionToPrompt( UI_STATE_MENU_CONF_HOT_LINE, PARAM_SPECIAL );
		} else {
			/* write to flash */
			bHotLine = 1;

			FlashWriteData( MY_FIELD_OFFSET( ui_falsh_layout_t, mode.bHotLine ),
						&bHotLine, sizeof( bHotLine ) );

			FlashWriteData( MY_FIELD_OFFSET( ui_falsh_layout_t, mode.bcdHotLine ),
						bcdHotLine, sizeof( bcdHotLine ) );

			FlashValidateWriting( 0 );

			/* flag */
			fModeFlags.b.hotLine = 1;

			SetupPromptFrame( szOK );
			UI_StateTransitionToPrompt( UI_STATE_MENU_CONF, CONFIG_ID_HOT_LINE );
		}
	}
}

void SetupMenuViewCallRecordsFrame( int param )
{
	static const menu_item_t viewCallRecordsItems[] = {
		{ szItemMissedCallRecords /* "Missed Call Records" */, 0, NULL },
		{ szItemIncomingCallRecords /* "Incoming Call Records" */, 0, NULL },
		{ szItemOutgoingCallRecords /* "Outgoing Call Records" */, 0, NULL },
	};
	
	static const menu_select_t viewCallRecordsSelect = {
		0,		/* attribute */
		sizeof( viewCallRecordsItems ) / sizeof( viewCallRecordsItems[ 0 ] ),	/* number of items */
		viewCallRecordsItems,	/* point to items */
		NULL,	/* point to get item text function */
	};
	
	ActivateMenuSelection( &viewCallRecordsSelect, param );
}

void StateMenuViewCallRecordsOnKey( unsigned char key )	/* UI_STATE_MENU_VIEW_CALL_RECORDS */
{
	/* Cancel key is processed by UI_KeypadInput() */
	if( key == VKEY_OK ) {
		nSelectedItem = DeactivateMenuSelection();

		switch( nSelectedItem ) {
		case VIEW_CALL_RECORD_ID_MISSED:
			UI_StateTransition( UI_STATE_CALL_RECORD_MISSED, 0 );
			break;

		case VIEW_CALL_RECORD_ID_INCOMING:
			UI_StateTransition( UI_STATE_CALL_RECORD_INCOMING, 0 );
			break;

		case VIEW_CALL_RECORD_ID_OUTGOING:
			UI_StateTransition( UI_STATE_CALL_RECORD_OUTGOING, 0 );
			break;
		}
	}
}

void SetupMenuTestCaseFrame( int param )
{
	static const menu_item_t testCaseItems[] = {
		{ ( const unsigned char * )"Text Ani.(vert)", 0, NULL },
		{ ( const unsigned char * )"Text Ani.(hor)", 0, NULL },
		{ ( const unsigned char * )"Chinese", 0, NULL },
	};
	
	static const menu_select_t testCaseSelect = {
		0,		/* attribute */
		sizeof( testCaseItems ) / sizeof( testCaseItems[ 0 ] ),	/* number of items */
		testCaseItems,	/* point to items */
		NULL,	/* point to get item text function */
	};
	
	ActivateMenuSelection( &testCaseSelect, param );
}

void StateMenuTestCaseOnKey( unsigned char key )	/* UI_STATE_MENU_TEST_CASE */
{
	/* Cancel key is processed by UI_KeypadInput() */
	if( key == VKEY_OK ) {
		nSelectedItem = DeactivateMenuSelection();

		switch( nSelectedItem ) {
		case TEST_CASE_ID_TEXT_ANI_VERTICAL:
			UI_StateTransition( UI_STATE_MENU_TEST_CASE_TEXT_VERTICAL, 0 );
			break;
		case TEST_CASE_ID_TEXT_ANI_HORIZONTAL:
			UI_StateTransition( UI_STATE_MENU_TEST_CASE_TEXT_HORIZONTAL, 0 );
			break;
		case TEST_CASE_ID_TEXT_CHINESE:
			UI_StateTransition( UI_STATE_MENU_TEST_CASE_TEXT_CHINESE, 0 );
			break;
		}
	}
}

void SetupMenuTestCaseTextVerticalFrame( int param )
{
	GS_DrawOffScreenAndClearScreen();

#ifdef _TEXT_MODE
	GS_TextOut( 0, 0, szGraphicOnly, RES_strlen( szGraphicOnly ) );
#else
	GS_DrawHorizontalLine( 0, 0, 100, 1 );
	GS_DrawHorizontalLine( 0, 17, 100, 1 );
	GS_DrawText( 1, 1, ( const unsigned char * )"(1,1)abc", 8 );
#endif

	SoftkeyStandardConfiguration_Back();

	GS_DrawOnScreen( NULL );
}

void StateMenuTestCaseTextVerticalOnKey( unsigned char key )	/* UI_STATE_MENU_TEST_CASE_TEXT_VERTICAL */
{
	/* Cancel key is processed by UI_KeypadInput() */
}

void TestCaseTextVerticalTimer( void )
{
#ifdef _TEXT_MODE
	return;
#else
	static int y = -21;

	GS_DrawText( 103, y, ( const unsigned char * )"(103,-21)abc", 10 );
	GS_DrawText( -21, y, ( const unsigned char * )"(-21,-21)abc", 10 );

	if( ++ y > 80 )
		y = -21;
#endif
}

void SetupMenuTestCaseTextHorizontalFrame( int param )
{
	GS_DrawOffScreenAndClearScreen();

#ifdef _TEXT_MODE
	GS_TextOut( 0, 0, szGraphicOnly, RES_strlen( szGraphicOnly ) );
#else
	GS_DrawHorizontalLine( 0, 0, 100, 1 );
	GS_DrawHorizontalLine( 0, 17, 100, 1 );
	GS_DrawText( 0, 1, ( const unsigned char * )"(0,1)abc", 8 );
#endif

	//GS_DrawText( -14, 18, ( const unsigned char * )"(-1,18)abc", 10 );
	//GS_DrawText( 120, 18, ( const unsigned char * )"(-1,18)abc", 10 );

	SoftkeyStandardConfiguration_Back();

	GS_DrawOnScreen( NULL );
}

void StateMenuTestCaseTextHorizontalOnKey( unsigned char key )	/* UI_STATE_MENU_TEST_CASE_TEXT_HORIZONTAL */
{
	/* Cancel key is processed by UI_KeypadInput() */
}

void TestCaseTextHorizontalTimer( void )
{
#ifdef _TEXT_MODE
	return;
#else
	static int x = -50;

	GS_DrawText( x, 18, ( const unsigned char * )"(-1,18)abc", 10 );

	if( ++ x > 150 )
		x = -50;
#endif
}

void SetupMenuTestCaseTextChineseFrame( int param )
{
#ifdef _TEXT_MODE
	return;
#else
	GS_DrawOffScreenAndClearScreen();

	GS_DrawText( 0, 0, ( const unsigned char * )"óÌùÕ­à", 6 );

	SoftkeyStandardConfiguration_Back();

	GS_DrawOnScreen( NULL );
#endif
}

void StateMenuTestCaseTextChineseOnKey( unsigned char key )	/* UI_STATE_MENU_TEST_CASE_TEXT_CHINESE */
{
	/* Cancel key is processed by UI_KeypadInput() */
}
