/*
 * voip_flash_mib.c: VoIP Flash Tool Header
 *
 * Authors: Rock <shaofu@realtek.com.tw>
 *
 */

#ifndef __VOIP_FLASH_TOOL_H
#define __VOIP_FLASH_TOOL_H

#ifdef __cplusplus
extern "C" {
#endif

int flash_voip_import_text(voipCfgAll_t *cfg_all, char *text, int text_len);

int flash_voip_import_check(voipCfgParam_t *pVoIPCfg);

int flash_voip_import_fix(voipCfgParam_t *pVoIPCfg, const voipCfgParam_t *pVoIPCfg_Old);

int flash_voip_import(voipCfgAll_t *cfg_all, char *buf, int buf_len);

int flash_voip_import_from_file(voipCfgAll_t *cfg_all, const char *filename);

int flash_voip_export(const voipCfgAll_t *cfg_all, char **buf, int *buf_len, int mode);

int flash_voip_export_to_file(const voipCfgAll_t *cfg_all, const char *filename, const int mode);

int flash_voip_write_file(const char *filename, const char *buf, const int buf_len);

int flash_voip_read_file(const char *filename, char **buf, int *buf_len);

int flash_voip_encode(const char *text, const int text_len, char **buf, int *buf_len);

int flash_voip_decode(char *buf, int buf_len, char **text, int *text_len);

#ifdef WIN32
#include <winsock2.h>

const char *inet_ntop(int af, const void *src, 
	char *dst, size_t cnt);

int inet_pton(int af, const char *src, void *dst);

#endif

#ifdef __cplusplus
}
#endif

#endif // __VOIP_FLASH_TOOL_H
