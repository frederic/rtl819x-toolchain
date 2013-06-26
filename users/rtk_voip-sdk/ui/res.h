#ifndef __RES_H__
#define __RES_H__

#define EXTERN_RES_STRING( name )			\
	extern const unsigned char name[];		\
	extern const unsigned short name ## _____len

#define RES_strlen( name )		name ## _____len

/* Resource of String */
EXTERN_RES_STRING( szOK );
EXTERN_RES_STRING( szFail );
EXTERN_RES_STRING( szPhonebookHasNoRecord );
EXTERN_RES_STRING( szPhonebookIsFull );
EXTERN_RES_STRING( szEnterName );
EXTERN_RES_STRING( szAdd );
EXTERN_RES_STRING( szEnterNumber );
EXTERN_RES_STRING( szModify );
EXTERN_RES_STRING( szDelete );
EXTERN_RES_STRING( szDeleteAll );
EXTERN_RES_STRING( szStatus );
EXTERN_RES_STRING( szUsedFormat );
EXTERN_RES_STRING( szFreeFormat );
EXTERN_RES_STRING( szStandbyPrompt );
EXTERN_RES_STRING( szQSure );
EXTERN_RES_STRING( szEmptyIsNotAllow );
EXTERN_RES_STRING( szDialNumberPrompt );
EXTERN_RES_STRING( szDoingOutgoingCall );
EXTERN_RES_STRING( szDoingFxoConnecting );
EXTERN_RES_STRING( szInConnection );
EXTERN_RES_STRING( szDisconnection );
EXTERN_RES_STRING( szIncomingCall );
EXTERN_RES_STRING( szDisconnecting );
EXTERN_RES_STRING( szError );
EXTERN_RES_STRING( szColonLAN );
EXTERN_RES_STRING( szColonWAN );
EXTERN_RES_STRING( szZerosIP );
EXTERN_RES_STRING( szVolume );
EXTERN_RES_STRING( szVolumeReceiver );
EXTERN_RES_STRING( szVolumeSpeaker );
EXTERN_RES_STRING( szVolumeMicR );
EXTERN_RES_STRING( szVolumeMicS );
EXTERN_RES_STRING( szVolumeWithDigitsFormat );
EXTERN_RES_STRING( szVolumeReceiverWithDigitsFormat );
EXTERN_RES_STRING( szVolumeSpeakerWithDigitsFormat );
EXTERN_RES_STRING( szVolumeMicRWithDigitsFormat );
EXTERN_RES_STRING( szVolumeMicSWithDigitsFormat );
EXTERN_RES_STRING( szInputIpPrompt );
EXTERN_RES_STRING( szPing );
EXTERN_RES_STRING( szGatewayPrompt );
EXTERN_RES_STRING( szDnsPrompt );
EXTERN_RES_STRING( szCallWaiting );
EXTERN_RES_STRING( szCallPrompt );
EXTERN_RES_STRING( szCallHoldStatus );
EXTERN_RES_STRING( szCallHoldingStatus );
EXTERN_RES_STRING( szCallHeldStatus );
EXTERN_RES_STRING( szCallConferenceStatus );
EXTERN_RES_STRING( szCallDisconnectedStatus );
EXTERN_RES_STRING( szNoname );
EXTERN_RES_STRING( szNoRecord );
EXTERN_RES_STRING( szHotLine );
EXTERN_RES_STRING( szHotLinePrompt );
EXTERN_RES_STRING( szAutoDial );
EXTERN_RES_STRING( szInputAutoDialPrompt );
EXTERN_RES_STRING( szInputAutoDialRange );
EXTERN_RES_STRING( szAutoAnswer );
EXTERN_RES_STRING( szInputAutoAnswerPrompt );
EXTERN_RES_STRING( szInputAutoAnswerRange );
EXTERN_RES_STRING( szInputOffHookAlarmPrompt );
EXTERN_RES_STRING( szInputOffHookAlarmRange );
EXTERN_RES_STRING( szInvalidValue );
EXTERN_RES_STRING( szOffHookAlarm );
EXTERN_RES_STRING( szSipRegister );
EXTERN_RES_STRING( szSipNotRegister );
EXTERN_RES_STRING( szRebooting );
EXTERN_RES_STRING( szNull );
EXTERN_RES_STRING( szGraphicOnly );
/* Instruction Text */
EXTERN_RES_STRING( szInsPhonebook );
EXTERN_RES_STRING( szInsMenu );
EXTERN_RES_STRING( szInsCancel );
EXTERN_RES_STRING( szInsOK );
EXTERN_RES_STRING( szInsBack );
EXTERN_RES_STRING( szInsClear );
EXTERN_RES_STRING( szInsYes );
EXTERN_RES_STRING( szInsNo );
EXTERN_RES_STRING( szInsDetail );
EXTERN_RES_STRING( szInsDial );
EXTERN_RES_STRING( szInsReject );
EXTERN_RES_STRING( szInsTransfer );
EXTERN_RES_STRING( szInsHold );
EXTERN_RES_STRING( szInsConference );
EXTERN_RES_STRING( szInsPick );
EXTERN_RES_STRING( szInsAnswer );
/* Menu Item Text */
EXTERN_RES_STRING( szItemView );
EXTERN_RES_STRING( szItemConfiguration );
EXTERN_RES_STRING( szItemPhonebook );
EXTERN_RES_STRING( szItemTestCase );
EXTERN_RES_STRING( szItemNetworkSettings );
EXTERN_RES_STRING( szItemPing );
EXTERN_RES_STRING( szItemSoftwareVersion );
EXTERN_RES_STRING( szItemCallRecords );
EXTERN_RES_STRING( szItemIPAddress );
EXTERN_RES_STRING( szItemMask );
EXTERN_RES_STRING( szItemGateway );
EXTERN_RES_STRING( szItemDNS );
EXTERN_RES_STRING( szItemKeypressTone );
EXTERN_RES_STRING( szItemMissedCallRecords );
EXTERN_RES_STRING( szItemIncomingCallRecords );
EXTERN_RES_STRING( szItemOutgoingCallRecords );
/* Month Text */
EXTERN_RES_STRING( szMonthJan );
EXTERN_RES_STRING( szMonthFeb );
EXTERN_RES_STRING( szMonthMar );
EXTERN_RES_STRING( szMonthApr );
EXTERN_RES_STRING( szMonthMay );
EXTERN_RES_STRING( szMonthJun );
EXTERN_RES_STRING( szMonthJul );
EXTERN_RES_STRING( szMonthAug );
EXTERN_RES_STRING( szMonthSep );
EXTERN_RES_STRING( szMonthOct );
EXTERN_RES_STRING( szMonthNov );
EXTERN_RES_STRING( szMonthDec );


#undef EXTERN_RES_STRING

#endif /* __RES_H__ */

