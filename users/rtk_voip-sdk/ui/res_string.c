#include "ui_config.h"
#include "res.h"

#ifdef LANG_BIG5	/* Traditional Chinese (BIG5) */
#define DECLARE_RES_STRING( name, szEng, szBig5, szGB2312 )		\
	const unsigned char name[] = { szBig5 };	\
	const unsigned short name ## _____len = sizeof( name ) - 1
#elif defined( LANG_GB2312 )	/* Simplified Chinese (GB2312) */
#define DECLARE_RES_STRING( name, szEng, szBig5, szGB2312 )		\
	const unsigned char name[] = { szGB2312 };	\
	const unsigned short name ## _____len = sizeof( name ) - 1
#else				/* English */
#define DECLARE_RES_STRING( name, szEng, szBig5, szGB2312 )		\
	const unsigned char name[] = { szEng };	\
	const unsigned short name ## _____len = sizeof( name ) - 1
#endif

DECLARE_RES_STRING( szOK,		"OK",		"ЧΘ",		"完成" );
DECLARE_RES_STRING( szFail,		"Fail",		"ア毖",		"失败" );
DECLARE_RES_STRING( szPhonebookHasNoRecord,	"No record!\nAdd one?",		"礚癘魁\n琌穝糤",		"无记录！\n是否新增？" );
DECLARE_RES_STRING( szPhonebookIsFull,		"Phonebook is full!",		"筿杠茂骸",				"电话本已满！" );
DECLARE_RES_STRING( szEnterName,		"Name:",		"羛蹈:",		"联络人:" );
DECLARE_RES_STRING( szEnterNumber,		"Number:",		"筿杠腹絏:",	"电话号码:" );
DECLARE_RES_STRING( szAdd,				"Add",			"穝糤",			"新增" );
DECLARE_RES_STRING( szModify,			"Modify",		"э",			"修改" );
DECLARE_RES_STRING( szDelete,			"Delete",		"埃",			"删除" );
DECLARE_RES_STRING( szDeleteAll,		"Delete All",	"埃场",		"删除全部" );
DECLARE_RES_STRING( szStatus,			"Status",	"篈",			"状态" );
DECLARE_RES_STRING( szUsedFormat,		"Used: %d",	"ㄏノ丁:%d",	"使用空间:%d" );
DECLARE_RES_STRING( szFreeFormat,		"Free: %d",	"逞緇丁:%d",	"剩余空间:%d" );
DECLARE_RES_STRING( szStandbyPrompt,	"IP Phone",	"IP筿杠",		"IP电话" );
DECLARE_RES_STRING( szQSure,			"Sure?",	"叫絋粄",		"请确认？" );
DECLARE_RES_STRING( szEmptyIsNotAllow,			"Error!\nIt is empty!",		"岿粇ぃフ",		"错误！不可空白！" );
DECLARE_RES_STRING( szDialNumberPrompt,			"Dial Number:",		"挤腹:",		"拨号:" );
DECLARE_RES_STRING( szDoingOutgoingCall,		"Outgoing call",	"挤杠",			"发话" );
DECLARE_RES_STRING( szDoingFxoConnecting,		"Connect FXO",		"FXO硈絬",		"FXO联机" );
DECLARE_RES_STRING( szInConnection,				"In Connection",	"硄杠い",		"通话中" );
DECLARE_RES_STRING( szDisconnection,			"Disconnection",	"硄杠挡",		"通话结束" );
DECLARE_RES_STRING( szIncomingCall,				"Incoming call",	"ㄓ筿",			"来电" );
DECLARE_RES_STRING( szDisconnecting,			"Disconnect...",	"挡硄杠い...",	"结束通话中..." );
DECLARE_RES_STRING( szError,					"Error!",		"岿粇",		"错误！" );
DECLARE_RES_STRING( szColonLAN,					"LAN:",			"LAN:",			"LAN:" );
DECLARE_RES_STRING( szColonWAN,					"WAN:",			"WAN:",			"WAN:" );
DECLARE_RES_STRING( szZerosIP,					"0.0.0.0",		"0.0.0.0",		"0.0.0.0" );
DECLARE_RES_STRING( szVolume,					"Volume",		"秖",			"音量" );
DECLARE_RES_STRING( szVolumeReceiver,			"MonoVolume",	"虫秖",		"单音音量" );
DECLARE_RES_STRING( szVolumeSpeaker,			"Spk Volume",	"斥秖",		"喇叭音量" );
DECLARE_RES_STRING( szVolumeMicR,				"Mic(R) Vol.",	"Mic(R)秖",	"Mic(R)音量" );
DECLARE_RES_STRING( szVolumeMicS,				"Mic(S) Vol.",	"Mic(S)秖",	"Mic(S)音量" );
DECLARE_RES_STRING( szVolumeWithDigitsFormat,	"Volume:%u",	"秖:%u",		"音量:%u" );
DECLARE_RES_STRING( szVolumeReceiverWithDigitsFormat,	"Mono Volume:%u",	"虫秖:%u",		"单音音量:%u" );
DECLARE_RES_STRING( szVolumeSpeakerWithDigitsFormat,	"Spk Volume:%u",	"斥秖:%u",		"喇叭音量:%u" );
DECLARE_RES_STRING( szVolumeMicRWithDigitsFormat,		"Mic(R) Volume:%u",	"Mic(R)秖:%u",	"Mic(R)音量:%u" );
DECLARE_RES_STRING( szVolumeMicSWithDigitsFormat,		"Mic(S) Volume:%u",	"Mic(S)秖:%u",	"Mic(S)音量:%u" );
DECLARE_RES_STRING( szInputIpPrompt,			"Input IP:",		"块IP:",		"输入IP:" );
DECLARE_RES_STRING( szPing,						"Ping",				"Ping",			"Ping" );
DECLARE_RES_STRING( szGatewayPrompt,			"Gateway:",			"箇砞筯笵:",	"预设网关:" );
DECLARE_RES_STRING( szDnsPrompt,				"DNS:",				"DNS:",			"DNS:" );
DECLARE_RES_STRING( szCallWaiting,				"Call waiting...",	"ㄓ筿单...",	"来电等待..." );
DECLARE_RES_STRING( szCallPrompt,				"Call:",			"ㄓ筿:",		"来电:" );
DECLARE_RES_STRING( szCallHoldStatus,			"[Hold]",			"[玂痙]",		"[已保留]" );
DECLARE_RES_STRING( szCallHoldingStatus,		"[Hold..]",			"[玂痙い]",		"[保留中]" );
DECLARE_RES_STRING( szCallHeldStatus,			"[Held]",			"[砆玂痙]",		"[被保留]" );
DECLARE_RES_STRING( szCallConferenceStatus,		"[Conf.]",			"[穦某]",		"[会议]" );
DECLARE_RES_STRING( szCallDisconnectedStatus,	"[Disc.]",			"[挡]",		"[结束]" );
DECLARE_RES_STRING( szNoname,					"(No name)",		"ゼ㏑",		"未命名" );
DECLARE_RES_STRING( szNoRecord,					"No record!",		"礚戈",		"无资料" );
DECLARE_RES_STRING( szHotLine,					"Hot Line",			"荐絬",			"热线" );
DECLARE_RES_STRING( szHotLinePrompt,			"Hot Line:",		"荐絬:",		"热线:" );
DECLARE_RES_STRING( szAutoDial,					"Auto Dial",		"笆挤腹砞﹚",		"自动拨号设置" );
DECLARE_RES_STRING( szInputAutoDialPrompt,		"Auto Dial Time:",	"笆挤腹丁:",	"自动拨号时间:" );
DECLARE_RES_STRING( szInputAutoDialRange,		"(3-9)",			"(3-9)",			"(3-9)" );
DECLARE_RES_STRING( szAutoAnswer,				"Auto Answer",		"笆钡钮砞﹚",		"自动接听设定" );
DECLARE_RES_STRING( szInputAutoAnswerPrompt,	"Auto Ans Time:",	"笆钡钮丁:",	"自动接听时间:" );
DECLARE_RES_STRING( szInputAutoAnswerRange,		"(3-9)",			"(3-9)",			"(3-9)" );
DECLARE_RES_STRING( szInvalidValue,				"Invalid Value!!",	"块岿粇",		"输入值错误！" );
DECLARE_RES_STRING( szOffHookAlarm,				"Off-hook Alarm",	"矗诀牡砞﹚",		"提机警告设置" );
DECLARE_RES_STRING( szInputOffHookAlarmPrompt,	"Off-hook Alarm:",	"矗诀牡丁:",	"提机警告时间:" );
DECLARE_RES_STRING( szInputOffHookAlarmRange,	"(10-60)",			"(10-60)",			"(10-60)" );
DECLARE_RES_STRING( szSipRegister,				"SIP registered  ",	"SIP爹",		"SIP已注册" );
DECLARE_RES_STRING( szSipNotRegister,			"SIP not register",	"SIPゼ爹",		"SIP未注册" );
DECLARE_RES_STRING( szRebooting,				"Reboot...",		"穝秨诀い...",	"重新开机中..." );
DECLARE_RES_STRING( szNull,				"",				"",					"" );
DECLARE_RES_STRING( szGraphicOnly,		"Graphic Only",	"Graphic Only",		"Graphic Only" );
/* Instruction Text */
DECLARE_RES_STRING( szInsPhonebook,		"Phonebook",	"筿杠茂",	"电话本" );
DECLARE_RES_STRING( szInsMenu,			"Menu",			"匡虫",	"菜单" );
DECLARE_RES_STRING( szInsCancel,		"Cancel",		"",	"取消" );
DECLARE_RES_STRING( szInsOK,			"OK",			"匡拒",	"选择" );
DECLARE_RES_STRING( szInsBack,			"Back",			"癶",	"退出" );
DECLARE_RES_STRING( szInsClear,			"Clear",		"睲埃",	"清除" );
DECLARE_RES_STRING( szInsYes,			"Yes",			"琌",	"是" );
DECLARE_RES_STRING( szInsNo,			"No",			"",	"否" );
DECLARE_RES_STRING( szInsDetail,		"Detail",		"冈薄",	"详情" );
DECLARE_RES_STRING( szInsDial,			"Dial",			"挤腹",	"拨号" );
DECLARE_RES_STRING( szInsReject,		"Reject",		"┶荡",	"拒绝" );
DECLARE_RES_STRING( szInsTransfer,		"Xfer",			"锣钡",	"转接" );
DECLARE_RES_STRING( szInsHold,			"Hold",			"玂痙",	"保留" );
DECLARE_RES_STRING( szInsConference,	"Conf",			"穦某",	"会议" );
DECLARE_RES_STRING( szInsPick,			"Pick",			"钡钮",	"接听" );
DECLARE_RES_STRING( szInsAnswer,		"Ans",			"钡钮",	"接听" );
/* Menu Item Text */
DECLARE_RES_STRING( szItemView,				"View",				"浪跌",		"查看" );
DECLARE_RES_STRING( szItemConfiguration,	"Configuration",	"砞﹚",		"设置" );
DECLARE_RES_STRING( szItemPhonebook,		"Phonebook",		"筿杠茂",	"电话本" );
DECLARE_RES_STRING( szItemTestCase,			"Test Case",		"秨祇代刚",	"开发测试" );
DECLARE_RES_STRING( szItemNetworkSettings,	"Network Settings",	"呼隔砞﹚",	"网络设置" );
DECLARE_RES_STRING( szItemPing,				"Ping",				"Ping",		"Ping" );
DECLARE_RES_STRING( szItemSoftwareVersion,	"Software Version",	"硁砰セ",	"软件版本" );
DECLARE_RES_STRING( szItemCallRecords,		"Call Records",		"硄杠癘魁",	"通话记录" );
DECLARE_RES_STRING( szItemIPAddress,		"IP Address",		"IP",	"IP地址" );
DECLARE_RES_STRING( szItemMask,				"Mask",				"綛竛",	"子屏蔽" );
DECLARE_RES_STRING( szItemGateway,			"Gateway",			"箇砞筯笵",	"预设网关" );
DECLARE_RES_STRING( szItemDNS,				"DNS",				"DNS",		"DNS" );
DECLARE_RES_STRING( szItemKeypressTone,		"KeypressTone",		"龄",	"按键音" );
DECLARE_RES_STRING( szItemMissedCallRecords,		"Missed Call Records",		"ゼ钡ㄓ筿",	"未接来电" );
DECLARE_RES_STRING( szItemIncomingCallRecords,		"Incoming Call Records",	"钡ㄓ筿",	"已接来电" );
DECLARE_RES_STRING( szItemOutgoingCallRecords,		"Outgoing Call Records",	"挤筿杠",	"已拨电话" );
/* Month Text */
DECLARE_RES_STRING( szMonthJan,				"Jan",				"る",		"一月" );
DECLARE_RES_STRING( szMonthFeb,				"Feb",				"る",		"二月" );
DECLARE_RES_STRING( szMonthMar,				"Mar",				"る",		"三月" );
DECLARE_RES_STRING( szMonthApr,				"Apr",				"る",		"四月" );
DECLARE_RES_STRING( szMonthMay,				"May",				"きる",		"五月" );
DECLARE_RES_STRING( szMonthJun,				"Jun",				"せる",		"六月" );
DECLARE_RES_STRING( szMonthJul,				"Jul",				"る",		"七月" );
DECLARE_RES_STRING( szMonthAug,				"Aug",				"る",		"八月" );
DECLARE_RES_STRING( szMonthSep,				"Sep",				"る",		"九月" );
DECLARE_RES_STRING( szMonthOct,				"Oct",				"る",		"十月" );
DECLARE_RES_STRING( szMonthNov,				"Nov",				"11る",		"11月" );
DECLARE_RES_STRING( szMonthDec,				"Dec",				"12る",		"12月" );
