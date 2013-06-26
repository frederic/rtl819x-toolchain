#ifndef __PRMT_LIMIT_H__
#define __PRMT_LIMIT_H__

#include "kernel_config.h"
#include "rtk_voip.h"

#define TRUE 1
#define FALSE 0

#define VERIFY_OK		1
#define VERIFY_ERROR	0

#define G711_COUNT	2

#ifdef CONFIG_RTK_VOIP_G729AB
#define G729_COUNT	1
#else
#define G729_COUNT	0
#endif /*CONFIG_RTK_VOIP_G729AB*/

#ifdef CONFIG_RTK_VOIP_G7231
#define G723_COUNT	2
#else
#define G723_COUNT	0
#endif /*CONFIG_RTK_VOIP_G7231*/

#ifdef CONFIG_RTK_VOIP_G726
#define G726_COUNT	4
#else
#define G726_COUNT 0
#endif /*CONFIG_RTK_VOIP_G726*/

#ifdef CONFIG_RTK_VOIP_GSMFR
#define GSM_COUNT	1
#else
#define GSM_COUNT	0
#endif /*CONFIG_RTK_VOIP_GSMFR*/

#ifdef CONFIG_RTK_VOIP_ILBC
#define ILBC_COUNT	2
#else
#define ILBC_COUNT	0
#endif /*CONFIG_RTK_VOIP_ILBC*/

#ifdef CONFIG_RTK_VOIP_G722
#define G722_COUNT	1
#else
#define G722_COUNT	0
#endif /*CONFIG_RTK_VOIP_G722*/

/* Definition in tr-104 */
#if 1
#define MAX_PROFILE_COUNT					CON_CH_NUM
#else
#ifdef CONFIG_RTK_VOIP_SLIC_NUM_1
#define MAX_PROFILE_COUNT					1
#endif /*CONFIG_RTK_VOIP_SLIC_NUM_1*/

#ifdef CONFIG_RTK_VOIP_SLIC_NUM_2
#define MAX_PROFILE_COUNT					2
#endif /*CONFIG_RTK_VOIP_SLIC_NUM_2*/

#ifdef CONFIG_RTK_VOIP_SLIC_NUM_3
#define MAX_PROFILE_COUNT					3
#endif /*CONFIG_RTK_VOIP_SLIC_NUM_2*/

#ifdef CONFIG_RTK_VOIP_SLIC_NUM_4
#define MAX_PROFILE_COUNT					4
#endif /*CONFIG_RTK_VOIP_SLIC_NUM_2*/

#ifdef CONFIG_RTK_VOIP_SLIC_NUM_5
#define MAX_PROFILE_COUNT					5
#endif /*CONFIG_RTK_VOIP_SLIC_NUM_2*/

#ifdef CONFIG_RTK_VOIP_SLIC_NUM_6
#define MAX_PROFILE_COUNT					6
#endif /*CONFIG_RTK_VOIP_SLIC_NUM_2*/

#ifdef CONFIG_RTK_VOIP_SLIC_NUM_7
#define MAX_PROFILE_COUNT					7
#endif /*CONFIG_RTK_VOIP_SLIC_NUM_2*/

#ifdef CONFIG_RTK_VOIP_SLIC_NUM_8
#define MAX_PROFILE_COUNT					8
#endif /*CONFIG_RTK_VOIP_SLIC_NUM_2*/

#ifdef CONFIG_RTK_VOIP_IP_PHONE
#define MAX_PROFILE_COUNT					1
#endif /*CONFIG_RTK_VOIP_IP_PHONE*/
#endif

#define PROFILE_MAX_SESSION					2		

#define MAX_SESSION_COUNT					( MAX_PROFILE_COUNT * PROFILE_MAX_SESSION )	/* 2 channel * 2 seessions */

#define SIGNALING_PROTOCOLS					"SIP"

#define SUPPORT_TONE_GENERATION_OBJECT		0	/* Support object VoiceProfile.{i}.Tone */

#define SUPPORT_RING_GENERATION_OBJECT		0	/* Support object VoiceProfile.{i}.Ringer */

#define SUPPORT_VOICE_LINE_TESTS_OBJECT		0	/* Support object VoiceProfile.{i}.Tests */

#define SIP_ROLE							"UserAgent"

#define SIP_TRANSPORTS						"UDP"

#define SIP_URI_SCHEMES					"sip"

#define SERVICE_PROVIDER					"Powered by Realtek"

#define ORGANIZATION_HEADER				"Organization header"

/* Definition for our platform  */
#define MAX_LINE_PER_PROFILE				1

#define MAX_CODEC_LIST		(G711_COUNT + G729_COUNT + G723_COUNT + G726_COUNT + GSM_COUNT + ILBC_COUNT + G722_COUNT)

/* ASSERT */
#ifndef CT_ASSERT
#define CT_ASSERT( expr )	extern int __ct_assert[ 2 * ( expr ) - 1 ]
#endif

#endif /* __PRMT_LIMIT_H__ */

