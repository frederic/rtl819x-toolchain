#ifndef __WEB_VOIP_H
#define __WEB_VOIP_H

#include "voip_flash.h"
#include "voip_types.h"
#include "voip_flash_mib.h"
#include "voip_flash_tool.h"
#include "voip_manager.h"

#ifdef CONFIG_APP_BOA
	#include "boa.h"
	#include "asp_page.h"
#else
	#include "webs.h"
#endif

#ifndef CONFIG_RTK_VOIP_PACKAGE_867X
#include "apform.h"
#endif

#ifdef __mips__
#if CONFIG_RTK_VOIP_PACKAGE_865X
#define VOIP_CONFIG_PATH	"/www/config_voip.dat"
#elif CONFIG_RTK_VOIP_PACKAGE_867X
#define VOIP_CONFIG_PATH	"/var/config_voip.dat"
#else
#define VOIP_CONFIG_PATH	"/web/config_voip.dat"
#endif
#else
#define VOIP_CONFIG_PATH	"../web/config_voip.dat"
#endif

#ifdef CONFIG_APP_BOA
#define gstrcmp strcmp
#define websRedirect	send_redirect_perm
#define websWrite		req_format_write
#define websGetVar		req_get_cstream_var
typedef char char_t;
typedef struct request *webs_t;
#define T(x)			(x)
#endif

/* To show the register status on Web page. */
#define _PATH_TMP_STATUS	"/tmp/status"

// SIP Config
#define _PATH_SIP_CONFIG	"/etc/solar.conf"

#if defined(CONFIG_RTK_VOIP_DRIVERS_FXO) && !defined(CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA)
//#define FXO_REDIAL
#endif

// init web functions
int web_voip_init();

#ifdef CONFIG_APP_BOA
int asp_voip_getInfo(webs_t wp, int argc, char_t **argv);
int asp_voip_GeneralGet(webs_t wp, int argc, char_t **argv);
int asp_voip_DialPlanGet( webs_t wp, int argc, char_t **argv);
int asp_voip_ToneGet(webs_t wp, int argc, char_t **argv);
int asp_voip_RingGet(webs_t wp, int argc, char_t **argv);
int asp_voip_OtherGet(webs_t wp, int argc, char_t **argv);
int asp_voip_ConfigGet(webs_t wp, int argc, char_t **argv);
int asp_voip_FwupdateGet(webs_t wp, int argc, char_t **argv);
int asp_voip_FxoGet(webs_t wp, int argc, char_t **argv);
int asp_voip_NetGet(webs_t wp, int argc, char_t **argv);
#else
int asp_voip_getInfo(int eid, webs_t wp, int argc, char_t **argv);
int asp_voip_GeneralGet(int ejid, webs_t wp, int argc, char_t **argv);
int asp_voip_DialPlanGet(int ejid, webs_t wp, int argc, char_t **argv);
int asp_voip_ToneGet(int ejid, webs_t wp, int argc, char_t **argv);
int asp_voip_RingGet(int ejid, webs_t wp, int argc, char_t **argv);
int asp_voip_OtherGet(int ejid, webs_t wp, int argc, char_t **argv);
int asp_voip_ConfigGet(int ejid, webs_t wp, int argc, char_t **argv);
int asp_voip_FwupdateGet(int ejid, webs_t wp, int argc, char_t **argv);
int asp_voip_FxoGet(int ejid, webs_t wp, int argc, char_t **argv);
int asp_voip_NetGet(int ejid, webs_t wp, int argc, char_t **argv);
#endif

void asp_voip_GeneralSet(webs_t wp, char_t *path, char_t *query);
void asp_voip_DialPlanSet(webs_t wp, char_t *path, char_t *query);
void asp_voip_ToneSet(webs_t wp, char_t *path, char_t *query);
void asp_voip_RingSet(webs_t wp, char_t *path, char_t *query);
void asp_voip_OtherSet(webs_t wp, char_t *path, char_t *query);
void asp_voip_ConfigSet(webs_t wp, char_t *path, char_t *query);
void asp_voip_FwSet(webs_t wp, char_t *path, char_t *query);
void asp_voip_IvrReqSet(webs_t wp, char_t *path, char_t *query);
void asp_voip_FxoSet(webs_t wp, char_t *path, char_t *query);
void asp_voip_NetSet(webs_t wp, char_t *path, char_t *query);
#ifdef CONFIG_RTK_VOIP_SIP_TLS
void asp_voip_TLSCertUpload(webs_t wp, char_t *path, char_t *query);
int asp_voip_TLSGetCertInfo(int eid, webs_t wp, int argc, char_t **argv);
#endif
#endif

// flash api in WEB
int web_flash_get(voipCfgParam_t **cfg);
int web_flash_set(voipCfgParam_t *cfg);

// misc function
int web_restart_solar();
int web_voip_saveConfig();

