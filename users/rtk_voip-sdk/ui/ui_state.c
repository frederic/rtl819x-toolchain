#include "ui_config.h"
#include "ui_state.h"
#include "ui_menu.h"
#include "ui_phonebook.h"

extern void SetupDummyFrame( int param );
extern void StateDummyOnKey( unsigned char key );
extern void SetupInitFrame( int param );
extern void StateInitOnKey( unsigned char key );	/* UI_STATE_INIT */
extern void SetupStandbyFrame( int param );
extern void StateStandbyOnKey( unsigned char key );	/* UI_STATE_STANDBY */

extern void SetupDialFrame( int param );
extern void StateDialOnKey( unsigned char key );	/* UI_STATE_DIAL */
extern void SetupDialTimeoutFrame( int param );
extern void StateDialTimeoutOnKey( unsigned char key );	/* UI_STATE_DIAL_TIMEOUT */


extern void SetupIncomingCallFrame( int param );
extern void StateIncomingCallOnKey( unsigned char key );	/* UI_STATE_INCOMING_CALL */
extern void SetupIncomingCallWaitFrame( int param );
extern void StateIncomingCallWaitOnKey( unsigned char key );	/* UI_STATE_INCOMING_CALL_WAIT */
extern void SetupOutgoingCallFrame( int param );
extern void StateOutgoingCallOnKey( unsigned char key );	/* UI_STATE_OUTGOING_CALL */
extern void SetupInConnectionFrame( int param );
extern void StateInConnectionOnKey( unsigned char key );	/* UI_STATE_IN_CONNECTION */
extern void SetupInConnectionDialFrame( int param );
extern void StateInConnectionDialOnKey( unsigned char key );	/* UI_STATE_IN_CONN_DIAL */
extern void SetupDisconnectionWaitFrame( int param );
extern void StateDisconnectionWaitOnKey( unsigned char key );	/* UI_STATE_DISCONNECTION_WAIT */
extern void SetupDisconnectionFrame( int param );
extern void StateDisconnectionOnKey( unsigned char key );	/* UI_STATE_DISCONNECTION */

extern void SetupMenuActFrame( int param );
extern void StateMenuActOnKey( unsigned char key );	/* UI_STATE_MENU_ACT */
extern void SetupMenuViewFrame( int param );
extern void StateMenuViewOnKey( unsigned char key );	/* UI_STATE_MENU_VIEW */
extern void SetupMenuViewNetFrame( int param );
extern void StateMenuViewNetOnKey( unsigned char key );	/* UI_STATE_MENU_VIEW_NET */
extern void SetupMenuViewNetIpFrame( int param );
extern void StateMenuViewNetIpOnKey( unsigned char key );	/* UI_STATE_MENU_VIEW_NET_IP */
extern void SetupMenuViewNetNetmaskFrame( int param );
extern void StateMenuViewNetNetmaskOnKey( unsigned char key );	/* UI_STATE_MENU_VIEW_NET_NETMASK */
extern void SetupMenuViewNetGatewayFrame( int param );
extern void StateMenuViewNetGatewayOnKey( unsigned char key );	/* UI_STATE_MENU_VIEW_NET_GATEWAY */
extern void SetupMenuViewNetDnsFrame( int param );
extern void StateMenuViewNetDnsOnKey( unsigned char key );	/* UI_STATE_MENU_VIEW_NET_DNS */

extern void SetupMenuViewSoftVerFrame( int param );
extern void StateMenuViewSoftVerOnKey( unsigned char key );	/* UI_STATE_MENU_VIEW_SOFT_VER */

extern void SetupMenuViewPingFrame( int param );
extern void StateMenuViewPingOnKey( unsigned char key );	/* UI_STATE_MENU_VIEW_PING */
extern void SetupMenuViewPingWaitFrame( int param );
extern void StateMenuViewPingWaitOnKey( unsigned char key );	/* UI_STATE_MENU_VIEW_PING_WAIT */

extern void SetupMenuViewCallRecordsFrame( int param );
extern void StateMenuViewCallRecordsOnKey( unsigned char key );	/* UI_STATE_MENU_VIEW_CALL_RECORDS */

extern void SetupMenuConfigFrame( int param );
extern void StateMenuConfigOnKey( unsigned char key );	/* UI_STATE_MENU_CONF */
extern void SetupMenuConfigOutVolumeReceiverFrame( int param );
extern void StateMenuConfigOutVolumeReceiverOnKey( unsigned char key );	/* UI_STATE_MENU_CONF_OUTVOL_RECEIVER */
extern void SetupMenuConfigOutVolumeSpeakerFrame( int param );
extern void StateMenuConfigOutVolumeSpeakerOnKey( unsigned char key );	/* UI_STATE_MENU_CONF_OUTVOL_SPEAKER */
extern void SetupMenuConfigInVolumeMicRFrame( int param );
extern void StateMenuConfigInVolumeMicROnKey( unsigned char key );	/* UI_STATE_MENU_CONF_INVOL_MIC_R */
extern void SetupMenuConfigInVolumeMicSFrame( int param );
extern void StateMenuConfigInVolumeMicSOnKey( unsigned char key );	/* UI_STATE_MENU_CONF_INVOL_MIC_S */
extern void SetupMenuConfigAutoDialTimeFrame( int param );
extern void StateMenuConfigAutoDialTimeOnKey( unsigned char key );	/* UI_STATE_MENU_CONF_AUTO_DIAL_TIME */
extern void SetupMenuConfigAutoAnswerTimeFrame( int param );
extern void StateMenuConfigAutoAnswerTimeOnKey( unsigned char key );	/* UI_STATE_MENU_CONF_AUTO_DIAL_TIME */
extern void SetupMenuConfigOffHookAlarmTimeFrame( int param );
extern void StateMenuConfigOffHookAlarmTimeOnKey( unsigned char key );	/* UI_STATE_MENU_CONF_OFF_HOOK_ALARM_TIME */
extern void SetupMenuConfigHotLineFrame( int param );
extern void StateMenuConfigHotLineOnKey( unsigned char key );	/* UI_STATE_MENU_CONF_HOT_LINE */


extern void SetupMenuTestCaseFrame( int param );
extern void StateMenuTestCaseOnKey( unsigned char key );	/* UI_STATE_MENU_TEST_CASE */
extern void SetupMenuTestCaseTextVerticalFrame( int param );
extern void StateMenuTestCaseTextVerticalOnKey( unsigned char key );	/* UI_STATE_MENU_TEST_CASE_TEXT_VERTICAL */
extern void SetupMenuTestCaseTextHorizontalFrame( int param );
extern void StateMenuTestCaseTextHorizontalOnKey( unsigned char key );	/* UI_STATE_MENU_TEST_CASE_TEXT_HORIZONTAL */
extern void SetupMenuTestCaseTextChineseFrame( int param );
extern void StateMenuTestCaseTextChineseOnKey( unsigned char key );	/* UI_STATE_MENU_TEST_CASE_TEXT_CHINESE */


extern void SetupCallRecordMissedFrame( int param );
extern void StateCallRecordMissedOnKey( unsigned char key );	/* UI_STATE_CALL_RECORD_MISSED */
extern void SetupCallRecordIncomingFrame( int param );
extern void StateCallRecordIncomingOnKey( unsigned char key );	/* UI_STATE_CALL_RECORD_INCOMING */
extern void SetupCallRecordOutgoingFrame( int param );
extern void StateCallRecordOutgoingOnKey( unsigned char key );	/* UI_STATE_CALL_RECORD_OUTGOING */
extern void SetupCallRecordMissedDetailFrame( int param );
extern void StateCallRecordMissedDetailOnKey( unsigned char key );	/* UI_STATE_CALL_RECORD_MISSED_DETAIL */
extern void SetupCallRecordIncomingDetailFrame( int param );
extern void StateCallRecordIncomingDetailOnKey( unsigned char key );	/* UI_STATE_CALL_RECORD_INCOMING_DETAIL */
extern void SetupCallRecordOutgoingDetailFrame( int param );
extern void StateCallRecordOutgoingDetailOnKey( unsigned char key );	/* UI_STATE_CALL_RECORD_OUTGOING_DETAIL */


extern void SetupPhonebookListFrame( int param );
extern void StatePhonebookListOnKey( unsigned char key );	/* UI_STATE_PHONEBOOK_LIST */
extern void SetupPhonebookEditNameFrame( int param );
extern void StatePhonebookEditNameOnKey( unsigned char key );	/* UI_STATE_PHONEBOOK_EDIT_NAME */
extern void SetupPhonebookEditNumberFrame( int param );
extern void StatePhonebookEditNumberOnKey( unsigned char key );	/* UI_STATE_PHONEBOOK_EDIT_NUMBER */


extern void SetupPhonebookActFrame( int param );
extern void StatePhonebookActOnKey( unsigned char key );	/* UI_STATE_PHONEBOOK_ACT */
extern void SetupPhonebookActAddFrame( int param );
extern void StatePhonebookActAddOnKey( unsigned char key );	/* UI_STATE_PHONEBOOK_ACT_ADD */
extern void SetupPhonebookActModifyFrame( int param );
extern void StatePhonebookActModifyOnKey( unsigned char key );	/* UI_STATE_PHONEBOOK_ACT_MODIFY */
extern void SetupPhonebookActDelFrame( int param );
extern void StatePhonebookActDelOnKey( unsigned char key );	/* UI_STATE_PHONEBOOK_ACT_DEL */
extern void SetupPhonebookActDelAllFrame( int param );
extern void StatePhonebookActDelAllOnKey( unsigned char key );	/* UI_STATE_PHONEBOOK_ACT_DEL_ALL */
extern void SetupPhonebookActStatusFrame( int param );
extern void StatePhonebookActStatusOnKey( unsigned char key );	/* UI_STATE_PHONEBOOK_ACT_STATUS */


extern void StatePromptOnKey( unsigned char key );	/* UI_STATE_PROMPT */


extern const ui_state_context_t uiStateContext[];

const ui_state_context_t uiStateContext[] = {
	{	/* UI_STATE_INIT */						SetupInitFrame,
		/* UI_STATE_INIT */						StateInitOnKey,
		/* UI_STATE_INIT */						STATE_TIMER_IDX_SLEEP,
		/* UI_STATE_INIT */						CANCEL_KEY_ACT_DEPENDS,
		/* UI_STATE_INIT */						0,
	},
	{	/* UI_STATE_STANDBY */					SetupStandbyFrame,
		/* UI_STATE_STANDBY */					StateStandbyOnKey,
		/* UI_STATE_STANDBY */					STATE_TIMER_IDX_STANDBY_T,
		/* UI_STATE_STANDBY */					CANCEL_KEY_ACT_DEPENDS,
		/* UI_STATE_STANDBY */					0,
	},


	{	/* UI_STATE_DIAL */						SetupDialFrame,
		/* UI_STATE_DIAL */						StateDialOnKey,
		/* UI_STATE_DIAL */						STATE_TIMER_IDX_CURSOR,
		/* UI_STATE_DIAL */						CANCEL_KEY_ACT_DEPENDS,
		/* UI_STATE_DIAL */						0,
	},
	{	/* UI_STATE_DIAL_TIMEOUT */				SetupDialTimeoutFrame,
		/* UI_STATE_DIAL_TIMEOUT */				StateDialTimeoutOnKey,
		/* UI_STATE_DIAL_TIMEOUT */				STATE_TIMER_IDX_PROMPT,
		/* UI_STATE_DIAL_TIMEOUT */				CANCEL_KEY_ACT_DEPENDS,
		/* UI_STATE_DIAL_TIMEOUT */				0,
	},



	{	/* UI_STATE_INCOMING_CALL */			SetupIncomingCallFrame,
		/* UI_STATE_INCOMING_CALL */			StateIncomingCallOnKey,
		/* UI_STATE_INCOMING_CALL */			STATE_TIMER_IDX_CALL,
		/* UI_STATE_INCOMING_CALL */			CANCEL_KEY_ACT_DEPENDS,
		/* UI_STATE_INCOMING_CALL */			0,
	},
	{	/* UI_STATE_INCOMING_CALL_WAIT */		SetupIncomingCallWaitFrame,
		/* UI_STATE_INCOMING_CALL_WAIT */		StateIncomingCallWaitOnKey,
		/* UI_STATE_INCOMING_CALL_WAIT */		STATE_TIMER_IDX_CALL,
		/* UI_STATE_INCOMING_CALL_WAIT */		CANCEL_KEY_ACT_DEPENDS,
		/* UI_STATE_INCOMING_CALL_WAIT */		0,
	},
	{	/* UI_STATE_OUTGOING_CALL */			SetupOutgoingCallFrame,
		/* UI_STATE_OUTGOING_CALL */			StateOutgoingCallOnKey,
		/* UI_STATE_OUTGOING_CALL */			STATE_TIMER_IDX_CALL,
		/* UI_STATE_OUTGOING_CALL */			CANCEL_KEY_ACT_DEPENDS,
		/* UI_STATE_OUTGOING_CALL */			0,
	},
	{	/* UI_STATE_IN_CONNECTION */			SetupInConnectionFrame,
		/* UI_STATE_IN_CONNECTION */			StateInConnectionOnKey,
		/* UI_STATE_IN_CONNECTION */			STATE_TIMER_IDX_CALL,
		/* UI_STATE_IN_CONNECTION */			CANCEL_KEY_ACT_DEPENDS,
		/* UI_STATE_IN_CONNECTION */			0,
	},
	{	/* UI_STATE_IN_CONN_DIAL */				SetupInConnectionDialFrame,
		/* UI_STATE_IN_CONN_DIAL */				StateInConnectionDialOnKey,
		/* UI_STATE_IN_CONN_DIAL */				STATE_TIMER_IDX_CALL,
		/* UI_STATE_IN_CONN_DIAL */				CANCEL_KEY_ACT_DEPENDS,
		/* UI_STATE_IN_CONN_DIAL */				0,
	},
	{	/* UI_STATE_DISCONNECTION_WAIT */		SetupDisconnectionWaitFrame,
		/* UI_STATE_DISCONNECTION_WAIT */		StateDisconnectionWaitOnKey,
		/* UI_STATE_DISCONNECTION_WAIT */		STATE_TIMER_IDX_CALL,
		/* UI_STATE_DISCONNECTION_WAIT */		CANCEL_KEY_ACT_DEPENDS,
		/* UI_STATE_DISCONNECTION_WAIT */		0,
	},
	{	/* UI_STATE_DISCONNECTION */			SetupDisconnectionFrame,
		/* UI_STATE_DISCONNECTION */			StateDisconnectionOnKey,
		/* UI_STATE_DISCONNECTION */			STATE_TIMER_IDX_CALL,
		/* UI_STATE_DISCONNECTION */			CANCEL_KEY_ACT_DEPENDS,
		/* UI_STATE_DISCONNECTION */			0,
	},
	
	

	{	/* UI_STATE_MENU_ACT */					SetupMenuActFrame,
		/* UI_STATE_MENU_ACT */					StateMenuActOnKey,
		/* UI_STATE_MENU_ACT */					STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_ACT */					CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_ACT */					0,
	},
	{	/* UI_STATE_MENU_VIEW */				SetupMenuViewFrame,
		/* UI_STATE_MENU_VIEW */				StateMenuViewOnKey,
		/* UI_STATE_MENU_VIEW */				STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_VIEW */				CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_VIEW */				MENU_ID_VIEW,
	},
	
	{	/* UI_STATE_MENU_VIEW_NET */			SetupMenuViewNetFrame,
		/* UI_STATE_MENU_VIEW_NET */			StateMenuViewNetOnKey,
		/* UI_STATE_MENU_VIEW_NET */			STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_VIEW_NET */			CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_VIEW_NET */			MENU_ID_VIEW_NET,
	},
	{	/* UI_STATE_MENU_VIEW_NET_IP */			SetupMenuViewNetIpFrame,
		/* UI_STATE_MENU_VIEW_NET_IP */			StateMenuViewNetIpOnKey,
		/* UI_STATE_MENU_VIEW_NET_IP */			STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_VIEW_NET_IP */			CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_VIEW_NET_IP */			MENU_ID_VIEW_NET_IP,
	},
	{	/* UI_STATE_MENU_VIEW_NET_NETMASK */	SetupMenuViewNetNetmaskFrame,
		/* UI_STATE_MENU_VIEW_NET_NETMASK */	StateMenuViewNetNetmaskOnKey,
		/* UI_STATE_MENU_VIEW_NET_NETMASK */	STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_VIEW_NET_NETMASK */	CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_VIEW_NET_NETMASK */	MENU_ID_VIEW_NET_NETMASK,
	},
	{	/* UI_STATE_MENU_VIEW_NET_GATEWAY */	SetupMenuViewNetGatewayFrame,
		/* UI_STATE_MENU_VIEW_NET_GATEWAY */	StateMenuViewNetGatewayOnKey,
		/* UI_STATE_MENU_VIEW_NET_GATEWAY */	STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_VIEW_NET_GATEWAY */	CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_VIEW_NET_GATEWAY */	MENU_ID_VIEW_NET_GATEWAY,
	},
	{	/* UI_STATE_MENU_VIEW_NET_DNS */		SetupMenuViewNetDnsFrame,
		/* UI_STATE_MENU_VIEW_NET_DNS */		StateMenuViewNetDnsOnKey,
		/* UI_STATE_MENU_VIEW_NET_DNS */		STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_VIEW_NET_DNS */		CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_VIEW_NET_DNS */		MENU_ID_VIEW_NET_DNS,
	},
	{	/* UI_STATE_MENU_VIEW_SOFT_VER */		SetupMenuViewSoftVerFrame,
		/* UI_STATE_MENU_VIEW_SOFT_VER */		StateMenuViewSoftVerOnKey,
		/* UI_STATE_MENU_VIEW_SOFT_VER */		STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_VIEW_SOFT_VER */		CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_VIEW_SOFT_VER */		MENU_ID_VIEW_SOFT_VER,
	},
	{	/* UI_STATE_MENU_VIEW_PING */			SetupMenuViewPingFrame,
		/* UI_STATE_MENU_VIEW_PING */			StateMenuViewPingOnKey,
		/* UI_STATE_MENU_VIEW_PING */			STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_VIEW_PING */			CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_VIEW_PING */			MENU_ID_VIEW_PING,
	},
	{	/* UI_STATE_MENU_VIEW_PING_WAIT */		SetupMenuViewPingWaitFrame,
		/* UI_STATE_MENU_VIEW_PING_WAIT */		StateMenuViewPingWaitOnKey,
		/* UI_STATE_MENU_VIEW_PING_WAIT */		STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_VIEW_PING_WAIT */		CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_VIEW_PING_WAIT */		0,
	},
	{	/* UI_STATE_MENU_VIEW_CALL_RECORDS */	SetupMenuViewCallRecordsFrame,
		/* UI_STATE_MENU_VIEW_CALL_RECORDS */	StateMenuViewCallRecordsOnKey,
		/* UI_STATE_MENU_VIEW_CALL_RECORDS */	STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_VIEW_CALL_RECORDS */	CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_VIEW_CALL_RECORDS */	MENU_ID_VIEW_CALL_RECORDS,
	},

	


	{	/* UI_STATE_MENU_CONF */				SetupMenuConfigFrame,
		/* UI_STATE_MENU_CONF */				StateMenuConfigOnKey,
		/* UI_STATE_MENU_CONF */				STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_CONF */				CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_CONF */				MENU_ID_CONFIG,
	},
	{	/* UI_STATE_MENU_CONF_OUTVOL_RECEIVER */SetupMenuConfigOutVolumeReceiverFrame,
		/* UI_STATE_MENU_CONF_OUTVOL_RECEIVER */StateMenuConfigOutVolumeReceiverOnKey,
		/* UI_STATE_MENU_CONF_OUTVOL_RECEIVER */STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_CONF_OUTVOL_RECEIVER */CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_CONF_OUTVOL_RECEIVER */CONFIG_ID_OUTVOL_RECEIVER,
	},
	{	/* UI_STATE_MENU_CONF_OUTVOL_SPEAKER */	SetupMenuConfigOutVolumeSpeakerFrame,
		/* UI_STATE_MENU_CONF_OUTVOL_SPEAKER */	StateMenuConfigOutVolumeSpeakerOnKey,
		/* UI_STATE_MENU_CONF_OUTVOL_SPEAKER */	STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_CONF_OUTVOL_SPEAKER */	CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_CONF_OUTVOL_SPEAKER */	CONFIG_ID_OUTVOL_SPEAKER,
	},
	{	/* UI_STATE_MENU_CONF_INVOL_MIC_R */	SetupMenuConfigInVolumeMicRFrame,
		/* UI_STATE_MENU_CONF_INVOL_MIC_R */	StateMenuConfigInVolumeMicROnKey,
		/* UI_STATE_MENU_CONF_INVOL_MIC_R */	STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_CONF_INVOL_MIC_R */	CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_CONF_INVOL_MIC_R */	CONFIG_ID_INVOL_MIC_R,
	},
	{	/* UI_STATE_MENU_CONF_INVOL_MIC_S */	SetupMenuConfigInVolumeMicSFrame,
		/* UI_STATE_MENU_CONF_INVOL_MIC_S */	StateMenuConfigInVolumeMicSOnKey,
		/* UI_STATE_MENU_CONF_INVOL_MIC_S */	STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_CONF_INVOL_MIC_S */	CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_CONF_INVOL_MIC_S */	CONFIG_ID_INVOL_MIC_S,
	},
	{	/* UI_STATE_MENU_CONF_AUTO_DIAL_TIME */	SetupMenuConfigAutoDialTimeFrame,
		/* UI_STATE_MENU_CONF_AUTO_DIAL_TIME */	StateMenuConfigAutoDialTimeOnKey,
		/* UI_STATE_MENU_CONF_AUTO_DIAL_TIME */	STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_CONF_AUTO_DIAL_TIME */	CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_CONF_AUTO_DIAL_TIME */	CONFIG_ID_AUTO_DIAL_TIME,
	},
	{	/* UI_STATE_MENU_CONF_AUTO_ANSWER_TIME */	SetupMenuConfigAutoAnswerTimeFrame,
		/* UI_STATE_MENU_CONF_AUTO_ANSWER_TIME */	StateMenuConfigAutoAnswerTimeOnKey,
		/* UI_STATE_MENU_CONF_AUTO_ANSWER_TIME */	STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_CONF_AUTO_ANSWER_TIME */	CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_CONF_AUTO_ANSWER_TIME */	CONFIG_ID_AUTO_ANSWER_TIME,
	},
	{	/* UI_STATE_MENU_CONF_OFF_HOOK_ALARM_TIME */	SetupMenuConfigOffHookAlarmTimeFrame,
		/* UI_STATE_MENU_CONF_OFF_HOOK_ALARM_TIME */	StateMenuConfigOffHookAlarmTimeOnKey,
		/* UI_STATE_MENU_CONF_OFF_HOOK_ALARM_TIME */	STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_CONF_OFF_HOOK_ALARM_TIME */	CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_CONF_OFF_HOOK_ALARM_TIME */	CONFIG_ID_OFF_HOOK_ALARM_TIME,
	},
	{	/* UI_STATE_MENU_CONF_HOT_LINE */		SetupMenuConfigHotLineFrame,
		/* UI_STATE_MENU_CONF_HOT_LINE */		StateMenuConfigHotLineOnKey,
		/* UI_STATE_MENU_CONF_HOT_LINE */		STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_CONF_HOT_LINE */		CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_CONF_HOT_LINE */		CONFIG_ID_HOT_LINE,
	},




	{	/* UI_STATE_MENU_TEST_CASE */			SetupMenuTestCaseFrame,
		/* UI_STATE_MENU_TEST_CASE */			StateMenuTestCaseOnKey,
		/* UI_STATE_MENU_TEST_CASE */			STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_TEST_CASE */			CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_TEST_CASE */			MENU_ID_TEST_CASE,
	},
	{	/* UI_STATE_MENU_TEST_CASE_TEXT_VERTICAL */		SetupMenuTestCaseTextVerticalFrame,
		/* UI_STATE_MENU_TEST_CASE_TEXT_VERTICAL */		StateMenuTestCaseTextVerticalOnKey,
		/* UI_STATE_MENU_TEST_CASE_TEXT_VERTICAL */		STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_TEST_CASE_TEXT_VERTICAL */		CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_TEST_CASE_TEXT_VERTICAL */		TEST_CASE_ID_TEXT_ANI_VERTICAL,
	},
	{	/* UI_STATE_MENU_TEST_CASE_TEXT_HORIZONTAL */	SetupMenuTestCaseTextHorizontalFrame,
		/* UI_STATE_MENU_TEST_CASE_TEXT_HORIZONTAL */	StateMenuTestCaseTextHorizontalOnKey,
		/* UI_STATE_MENU_TEST_CASE_TEXT_HORIZONTAL */	STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_TEST_CASE_TEXT_HORIZONTAL */	CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_TEST_CASE_TEXT_HORIZONTAL */	TEST_CASE_ID_TEXT_ANI_HORIZONTAL,
	},
	{	/* UI_STATE_MENU_TEST_CASE_TEXT_CHINESE */		SetupMenuTestCaseTextChineseFrame,
		/* UI_STATE_MENU_TEST_CASE_TEXT_CHINESE */		StateMenuTestCaseTextChineseOnKey,
		/* UI_STATE_MENU_TEST_CASE_TEXT_CHINESE */		STATE_TIMER_IDX_MENU,
		/* UI_STATE_MENU_TEST_CASE_TEXT_CHINESE */		CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_MENU_TEST_CASE_TEXT_CHINESE */		TEST_CASE_ID_TEXT_CHINESE,
	},

	

	{	/* UI_STATE_CALL_RECORD_MISSED */		SetupCallRecordMissedFrame,
		/* UI_STATE_CALL_RECORD_MISSED */		StateCallRecordMissedOnKey,
		/* UI_STATE_CALL_RECORD_MISSED */		STATE_TIMER_IDX_MENU,
		/* UI_STATE_CALL_RECORD_MISSED */		CANCEL_KEY_ACT_DEPENDS,
		/* UI_STATE_CALL_RECORD_MISSED */		VIEW_CALL_RECORD_ID_MISSED,	/* only used by UI_STATE_MENU_VIEW_CALL_RECORDS */
	},
	{	/* UI_STATE_CALL_RECORD_INCOMING */		SetupCallRecordIncomingFrame,
		/* UI_STATE_CALL_RECORD_INCOMING */		StateCallRecordIncomingOnKey,
		/* UI_STATE_CALL_RECORD_INCOMING */		STATE_TIMER_IDX_MENU,
		/* UI_STATE_CALL_RECORD_INCOMING */		CANCEL_KEY_ACT_DEPENDS,
		/* UI_STATE_CALL_RECORD_INCOMING */		VIEW_CALL_RECORD_ID_INCOMING,	/* only used by UI_STATE_MENU_VIEW_CALL_RECORDS */
	},
	{	/* UI_STATE_CALL_RECORD_OUTGOING */		SetupCallRecordOutgoingFrame,
		/* UI_STATE_CALL_RECORD_OUTGOING */		StateCallRecordOutgoingOnKey,
		/* UI_STATE_CALL_RECORD_OUTGOING */		STATE_TIMER_IDX_MENU,
		/* UI_STATE_CALL_RECORD_OUTGOING */		CANCEL_KEY_ACT_DEPENDS,
		/* UI_STATE_CALL_RECORD_OUTGOING */		VIEW_CALL_RECORD_ID_OUTGOING,	/* only used by UI_STATE_MENU_VIEW_CALL_RECORDS */
	},
	{	/* UI_STATE_CALL_RECORD_MISSED_DETAIL */	SetupCallRecordMissedDetailFrame,
		/* UI_STATE_CALL_RECORD_MISSED_DETAIL */	StateCallRecordMissedDetailOnKey,
		/* UI_STATE_CALL_RECORD_MISSED_DETAIL */	STATE_TIMER_IDX_MENU,
		/* UI_STATE_CALL_RECORD_MISSED_DETAIL */	CANCEL_KEY_ACT_DEPENDS,
		/* UI_STATE_CALL_RECORD_MISSED_DETAIL */	0,
	},
	{	/* UI_STATE_CALL_RECORD_INCOMING_DETAIL */	SetupCallRecordIncomingDetailFrame,
		/* UI_STATE_CALL_RECORD_INCOMING_DETAIL */	StateCallRecordIncomingDetailOnKey,
		/* UI_STATE_CALL_RECORD_INCOMING_DETAIL */	STATE_TIMER_IDX_MENU,
		/* UI_STATE_CALL_RECORD_INCOMING_DETAIL */	CANCEL_KEY_ACT_DEPENDS,
		/* UI_STATE_CALL_RECORD_INCOMING_DETAIL */	0,
	},
	{	/* UI_STATE_CALL_RECORD_OUTGOING_DETAIL */	SetupCallRecordOutgoingDetailFrame,
		/* UI_STATE_CALL_RECORD_OUTGOING_DETAIL */	StateCallRecordOutgoingDetailOnKey,
		/* UI_STATE_CALL_RECORD_OUTGOING_DETAIL */	STATE_TIMER_IDX_MENU,
		/* UI_STATE_CALL_RECORD_OUTGOING_DETAIL */	CANCEL_KEY_ACT_DEPENDS,
		/* UI_STATE_CALL_RECORD_OUTGOING_DETAIL */	0,
	},
	

	{	/* UI_STATE_PHONEBOOK_LIST */			SetupPhonebookListFrame,
		/* UI_STATE_PHONEBOOK_LIST */			StatePhonebookListOnKey,
		/* UI_STATE_PHONEBOOK_LIST */			STATE_TIMER_IDX_MENU,
		/* UI_STATE_PHONEBOOK_LIST */			CANCEL_KEY_ACT_DEPENDS,
		/* UI_STATE_PHONEBOOK_LIST */			0,
	},
	{	/* UI_STATE_PHONEBOOK_EDIT_NAME */		SetupPhonebookEditNameFrame,
		/* UI_STATE_PHONEBOOK_EDIT_NAME */		StatePhonebookEditNameOnKey,
		/* UI_STATE_PHONEBOOK_EDIT_NAME */		STATE_TIMER_IDX_CURSOR,
		/* UI_STATE_PHONEBOOK_EDIT_NAME */		CANCEL_KEY_ACT_DEPENDS,
		/* UI_STATE_PHONEBOOK_EDIT_NAME */		0,
	},
	{	/* UI_STATE_PHONEBOOK_EDIT_NUMBER */	SetupPhonebookEditNumberFrame,
		/* UI_STATE_PHONEBOOK_EDIT_NUMBER */	StatePhonebookEditNumberOnKey,
		/* UI_STATE_PHONEBOOK_EDIT_NUMBER */	STATE_TIMER_IDX_CURSOR,
		/* UI_STATE_PHONEBOOK_EDIT_NUMBER */	CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_PHONEBOOK_EDIT_NUMBER */	0,
	},
	{	/* UI_STATE_PHONEBOOK_ACT */			SetupPhonebookActFrame,
		/* UI_STATE_PHONEBOOK_ACT */			StatePhonebookActOnKey,
		/* UI_STATE_PHONEBOOK_ACT */			STATE_TIMER_IDX_MENU,
		/* UI_STATE_PHONEBOOK_ACT */			CANCEL_KEY_ACT_DEPENDS,
		/* UI_STATE_PHONEBOOK_ACT */			0,
	},
	{	/* UI_STATE_PHONEBOOK_ACT_ADD */		SetupPhonebookActAddFrame,
		/* UI_STATE_PHONEBOOK_ACT_ADD */		StatePhonebookActAddOnKey,
		/* UI_STATE_PHONEBOOK_ACT_ADD */		STATE_TIMER_IDX_CURSOR,
		/* UI_STATE_PHONEBOOK_ACT_ADD */		CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_PHONEBOOK_ACT_ADD */		PHONEBOOK_ACT_ID_ADD,
	},
	{	/* UI_STATE_PHONEBOOK_ACT_MODIFY */		SetupPhonebookActModifyFrame,
		/* UI_STATE_PHONEBOOK_ACT_MODIFY */		StatePhonebookActModifyOnKey,
		/* UI_STATE_PHONEBOOK_ACT_MODIFY */		STATE_TIMER_IDX_MENU,
		/* UI_STATE_PHONEBOOK_ACT_MODIFY */		CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_PHONEBOOK_ACT_MODIFY */		PHONEBOOK_ACT_ID_MODIFY,
	},
	{	/* UI_STATE_PHONEBOOK_ACT_DEL */		SetupPhonebookActDelFrame,
		/* UI_STATE_PHONEBOOK_ACT_DEL */		StatePhonebookActDelOnKey,
		/* UI_STATE_PHONEBOOK_ACT_DEL */		STATE_TIMER_IDX_CONFIRM,
		/* UI_STATE_PHONEBOOK_ACT_DEL */		CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_PHONEBOOK_ACT_DEL */		PHONEBOOK_ACT_ID_DEL,
	},
	{	/* UI_STATE_PHONEBOOK_ACT_DEL_ALL */	SetupPhonebookActDelAllFrame,
		/* UI_STATE_PHONEBOOK_ACT_DEL_ALL */	StatePhonebookActDelAllOnKey,
		/* UI_STATE_PHONEBOOK_ACT_DEL_ALL */	STATE_TIMER_IDX_CONFIRM,
		/* UI_STATE_PHONEBOOK_ACT_DEL_ALL */	CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_PHONEBOOK_ACT_DEL_ALL */	PHONEBOOK_ACT_ID_DEL_ALL,
	},
	{	/* UI_STATE_PHONEBOOK_ACT_STATUS */		SetupPhonebookActStatusFrame,
		/* UI_STATE_PHONEBOOK_ACT_STATUS */		StatePhonebookActStatusOnKey,
		/* UI_STATE_PHONEBOOK_ACT_STATUS */		STATE_TIMER_IDX_CONFIRM,
		/* UI_STATE_PHONEBOOK_ACT_STATUS */		CANCEL_KEY_ACT_BACK_ONE_LEVEL,
		/* UI_STATE_PHONEBOOK_ACT_STATUS */		PHONEBOOK_ACT_ID_STATUS,
	},
	

	{	/* UI_STATE_PROMPT */		SetupDummyFrame,
		/* UI_STATE_PROMPT */		StatePromptOnKey,
		/* UI_STATE_PROMPT */		STATE_TIMER_IDX_PROMPT,
		/* UI_STATE_PROMPT */		CANCEL_KEY_ACT_DEPENDS,
		/* UI_STATE_PROMPT */		0,
	},
};

CT_ASSERT( ( sizeof( uiStateContext ) / sizeof( uiStateContext[ 1 ] ) ) == NUM_OF_UI_STATE );

