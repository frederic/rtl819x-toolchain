#ifndef __CVCFG__
#define __CVCFG__

#include "apmib.h"
#include "mibtbl.h"

#ifdef __cplusplus
extern "C" {
#endif

	// File type mask
#define TYPE_MASK		0x0f
#define CS_TYPE			0x1	// bit mask for cs
#define DS_TYPE			0X2	// bit mask for ds
#define HS_TYPE			0x4	// bit mask for hs

#define RAW_TYPE		0x80	// bit nask for raw data with pad

// mode select
#define MODE_MASK		0xf0
#define TXT_MODE		0x10
#define TARGET_MODE		0X20
#define PC_MODE			0X40

struct all_config {
	int hwmib_exist, dsmib_exist, csmib_exist;
	int hwmib_ver, dsmib_ver, csmib_ver;
	int hwmib_len, dsmib_len, csmib_len;
	HW_SETTING_T hwmib;
	unsigned char tmp1[100];
	APMIB_T dsmib;
	unsigned char tmp2[100];
	APMIB_T csmib;
	unsigned char tmp3[100];
};

int checkFileType_win32(char *filename);

int generateBinFile_win32(int type, char *filename, int flag, struct all_config *allConfig);

int generateTxtFile_win32(char *filename, const struct all_config *mib_config);

int parseBinConfig_win32(int type, char *filename, struct all_config *pMib);

int parseTxtConfig_win32(char *filename, struct all_config *pMib);


#ifdef __cplusplus
}
#endif
#endif