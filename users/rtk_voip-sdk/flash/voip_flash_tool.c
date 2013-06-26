/*
 * voip_flash.c: VoIP Flash Tool 
 *
 * Authors: Rock <shaofu@realtek.com.tw>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef WIN32
	#include <io.h>
	#include <winsock2.h>
	#include <ws2tcpip.h>
#else
	#include <unistd.h>
	#include <netinet/in.h>
#endif
#include <fcntl.h>

#include "voip_flash.h"
#include "voip_flash_mib.h"
#include "voip_flash_tool.h"

/* scramble saved configuration data */
#define ENCODE_DATA(data,len) { \
	unsigned int i; \
	for (i=0; i<len; i++) \
		data[i] = ~ ( data[i] + 0x38); \
}

#define DECODE_DATA(data,len) { \
	unsigned int i; \
	for (i=0; i<len; i++) \
		data[i] = ~data[i] - 0x38;  \
}

/* Do checksum and verification for configuration data */
#ifdef WIN32
__inline unsigned char CHECKSUM(unsigned char *data, int len)
#else
static inline unsigned char CHECKSUM(unsigned char *data, int len)
#endif
{
    int i;
    unsigned char sum=0;

    for (i=0; i<len; i++)
        sum += data[i];

    sum = ~sum + 1;
    return sum;
}

#ifdef WIN32
__inline int CHECKSUM_OK(unsigned char *data, int len)
#else
static inline int CHECKSUM_OK(unsigned char *data, int len)
#endif
{
    int i;
    unsigned char sum=0;

    for (i=0; i<len; i++)
        sum += data[i];

    if (sum == 0)
        return 1;
    else
        return 0;
}

// encode to config format
int flash_voip_encode(const char *text, const int text_len, char **buf, int *buf_len)
{
	VOIP_CFG_HEADER *pHeader;
	unsigned char checksum;
	char *ptr;

// config file struct:
// | ------------------- |
// | config file header  |
// | ------------------- |
// | voip encode config  |
// | ------------------- |
// | checksum (1 byte)   |
// | ------------------- |

	(*buf) = (char *) malloc(sizeof(VOIP_CFG_HEADER) + text_len + sizeof(checksum)); // config data
	if (*buf == NULL)
	{
		fprintf(stderr, "VoIP MIB Encode Error: malloc failed\n");
		return -1;
	}

	// create config header
	pHeader = (VOIP_CFG_HEADER *) (*buf);
	pHeader->signature = htonl(VOIP_CONFIG_SIGNATURE);
	pHeader->len = htons((unsigned short) (text_len + sizeof(checksum)));
	
	// create config data
	ptr = &(*buf)[sizeof(VOIP_CFG_HEADER)];
	memcpy(ptr, text, text_len);

	// create checksum
	checksum = CHECKSUM((unsigned char *) ptr, text_len);
	(*buf)[sizeof(VOIP_CFG_HEADER) + text_len] = checksum;

	// encode config data
	ptr = &(*buf)[sizeof(VOIP_CFG_HEADER)];
	ENCODE_DATA(ptr, text_len + sizeof(checksum));

	// export to config data ok
	*buf_len = sizeof(VOIP_CFG_HEADER) + text_len + sizeof(checksum);
	return 0;
}

int flash_voip_export(const voipCfgAll_t *cfg_all, char **buf, int *buf_len, int mode)
{
#define VOIP_TEMP_FILE	"/tmp/flash_voip.$$$"
	FILE *fp;
	struct stat status;
	char *text;
	int error = -1;

	unlink(VOIP_TEMP_FILE);
	fp = fopen(VOIP_TEMP_FILE, "w+");
	if (fp == NULL)
	{
		fprintf(stderr, "VoIP MIB Export Error: create temp failed\n");
		return -1;
	}

	// output to text file
	if ((cfg_all->mode & VOIP_CURRENT_SETTING) &&
		(voip_mibtbl_write(&cfg_all->current_setting, fileno(fp), VOIP_CURRENT_SETTING) != 0))
	{
		fprintf(stderr, "VoIP MIB Export Error: write failed\n");
		goto flash_voip_export_end;
	}

	if ((cfg_all->mode & VOIP_DEFAULT_SETTING) &&
		(voip_mibtbl_write(&cfg_all->default_setting, fileno(fp), VOIP_DEFAULT_SETTING) != 0))
	{
		fprintf(stderr, "VoIP Default MIB Export Error: write failed\n");
		goto flash_voip_export_end;
	}

	if (fstat(fileno(fp), &status) < 0)
	{
		fprintf(stderr, "VoIP MIB Export Error: stat failed\n");
		goto flash_voip_export_end;
	}

	text = (char *) malloc(status.st_size); // text data
	if (text == NULL)
	{
		fprintf(stderr, "VoIP MIB Export Error: malloc failed\n");
		goto flash_voip_export_end;
	}

	// get text data from text file
	fseek(fp, 0L, SEEK_SET);
	if (fread(text, status.st_size, 1, fp) != 1)
	{
		fprintf(stderr, "VoIP MIB Export Error: read failed\n");
		goto flash_voip_export_end2;
	}

	if (mode == 0) 
	{
		// export to text data ok
		error = 0;
		*buf = text;
		*buf_len = status.st_size;
		goto flash_voip_export_end; // no free text, its return value
	}

	// encode to config data
	if (flash_voip_encode(text, status.st_size, buf, buf_len) != 0)
	{
		fprintf(stderr, "VoIP MIB Export Error: encode failed\n");
		goto flash_voip_export_end2;
	}

	error = 0;

flash_voip_export_end2:
	free(text);
flash_voip_export_end:
	// close temp file
	fclose(fp); 
	unlink(VOIP_TEMP_FILE);
	return error;
}

int flash_voip_write_file(const char *filename, const char *buf, const int buf_len)
{
	int fd;

	if (strcmp(filename, "-") == 0)
	{
		fwrite(buf, buf_len, 1, stdout);
		return 0;
	}

	fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0666); // mode = mode & ~umask
	if (fd < 0)
	{
		fprintf(stderr, "VoIP MIB Write File Error: open %s failed\n", filename);
		return -1;
	}

	if (write(fd, buf, buf_len) != buf_len)
	{
		fprintf(stderr, "VoIP MIB Write File Error: write to %s failed\n", filename);
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}

int flash_voip_export_to_file(const voipCfgAll_t *cfg_all, const char *filename, const int mode)
{
	char *buf;
	int buf_len;

	if (flash_voip_export(cfg_all, &buf, &buf_len, mode) != 0)
	{
		fprintf(stderr, "VoIP MIB Export File Error: export failed\n");
		return -1;
	}

	if (flash_voip_write_file(filename, buf, buf_len) != 0)
	{
		fprintf(stderr, "VoIP MIB Export File Error: write failed\n");
		free(buf);
		return -1;
	}

	free(buf);
	return 0;
}

// decode to text format
int flash_voip_decode(char *buf, int buf_len, char **text, int *text_len)
{
	VOIP_CFG_HEADER *pHeader;
	char *ptr;

	pHeader = (VOIP_CFG_HEADER *) buf;
	if (ntohl(pHeader->signature) == VOIP_CONFIG_SIGNATURE) 
	{
		// config data
		if (sizeof(VOIP_CFG_HEADER) + ntohs(pHeader->len) > (unsigned int) buf_len)
		{
			fprintf(stderr, "VoIP MIB Decode Error: buffer size is not enough\n");
			return -1;
		}
		else if (sizeof(VOIP_CFG_HEADER) + ntohs(pHeader->len) < (unsigned int) buf_len)
		{
			// check padding case ?
			fprintf(stderr, "VoIP MIB Decode Warning: buffer size (%d) is bigger than config (%d)\n",
				buf_len, sizeof(VOIP_CFG_HEADER) + ntohs(pHeader->len));
			// do warning only
		}

		// decode data & checksum
		ptr = &buf[sizeof(VOIP_CFG_HEADER)];
		DECODE_DATA(ptr, ntohs(pHeader->len));

		// calculate checksum is ok
		ptr = &buf[sizeof(VOIP_CFG_HEADER)];
		if (!CHECKSUM_OK((unsigned char *) ptr, ntohs(pHeader->len))) 
		{
			fprintf(stderr, "VoIP MIB Decode Error: checksum failed\n");
			return -1;
		}

		buf_len -= sizeof(VOIP_CFG_HEADER) + 1;

		// it is text data now
	}
	else
	{
		// assume text data
		ptr = buf;
	}

	*text = (char *) malloc(buf_len);
	if (*text == NULL)
	{
		fprintf(stderr, "VoIP MIB Decode Error: malloc failed\n");
		return -1;
	}

	memcpy(*text, ptr, buf_len);
	*text_len = buf_len;
	return 0;
}

int flash_voip_import_text(voipCfgAll_t *cfg_all, char *text, int text_len)
{
	int i, idx;
	char line[600];

	// import setting from text data
	for (i=0, idx=0; i<text_len && idx<600; i++)
	{
		if (text[i] == '\n')
		{
			line[idx] = 0;
			voip_mibtbl_read_line(cfg_all, line);
			idx = 0;
			continue;
		}
		
		line[idx++] = text[i];
	}

	if (idx >= 600)
	{
		return -1;
	}

	if (idx > 0)
	{
		line[idx] = 0;
		voip_mibtbl_read_line(cfg_all, line);
	}

	return 0;
}

int flash_voip_import_check(voipCfgParam_t *pVoIPCfg)
{
//  import rule:
//  0. signature have to match
// 	1. mib_version have to match (mib name ok)
//	2. feature and extend feature match, or
//     current(flash) version == config(file) version
//	   (It has to skip the not match part when importing!).
	if (pVoIPCfg->signature != VOIP_FLASH_SIGNATURE)
	{
		fprintf(stderr, "VoIP MIB Import Check Error: signature not match, %x vs %x\n",
			(int) pVoIPCfg->signature, VOIP_FLASH_SIGNATURE);
		return -1;
	}

	if (pVoIPCfg->mib_version != VOIP_FLASH_MIB_VER)
	{
		fprintf(stderr, "VoIP MIB Import Check Error: mib version not match\n");
		return -1;
	}

	return 0;
}

#if defined( WIN32 )//def CVVOIP

// not support flash_voip_import_fix & flash_voip_import

#else

int flash_voip_import_fix(voipCfgParam_t *pVoIPCfg, const voipCfgParam_t *pVoIPCfg_Old)
{
	int i, status;
	VoipFeature_t feature_new, feature_old;
	
	feature_new = VOIP_FLASH_2_SYSTEM_FEATURE(
					pVoIPCfg->feature, pVoIPCfg->extend_feature );
	feature_old = VOIP_FLASH_2_SYSTEM_FEATURE(
					pVoIPCfg_Old->feature, pVoIPCfg_Old->extend_feature );
	
	status = flash_voip_import_check(pVoIPCfg);
	if (status < 0)
	{
		// use old setting replace
		memcpy(pVoIPCfg, pVoIPCfg_Old, sizeof(voipCfgParam_t));
		return -1;
	}
	
	if ((pVoIPCfg->feature != pVoIPCfg_Old->feature) ||
		(pVoIPCfg->extend_feature != pVoIPCfg_Old->extend_feature))
	{
		if (pVoIPCfg->version != VOIP_FLASH_VER)
		{
			// feature not match, and version not match is not allowed
			// use old setting replace
			memcpy(pVoIPCfg, pVoIPCfg_Old, sizeof(voipCfgParam_t));
			return -1;
		}

		// The same version but different feature,
		// try to fix
		if (!RTK_VOIP_CODEC_CMP(feature_new, feature_old))
		{
			// restore old voip codec setting
			for (i=0; i<VOIP_PORTS; i++)
			{
				memcpy(&pVoIPCfg->ports[i].frame_size,
					&pVoIPCfg_Old->ports[i].frame_size,
					sizeof(pVoIPCfg->ports[i].frame_size) +
					sizeof(pVoIPCfg->ports[i].precedence)
					);
			}
		}

		// check other feature

		// check different feature done!
	}

	return 0;
}

int flash_voip_import(voipCfgAll_t *cfg_all, char *buf, int buf_len)
{
	voipCfgAll_t cfg_all_tmp;
	char *text;
	int text_len;
	voipCfgParam_t *pVoIPCfgTmp, *pVoIPCfg;
	int mode;

	// decode to text data
	if (flash_voip_decode(buf, buf_len, &text, &text_len) != 0)
	{
		fprintf(stderr, "VoIP MIB Import Error: decode failed\n");
		return -1;
	}

	// copy old setting before import
	memcpy(&cfg_all_tmp, cfg_all, sizeof(*cfg_all));
	cfg_all_tmp.mode = VOIP_NONE_SETTING;
	if (flash_voip_import_text(&cfg_all_tmp, text, text_len) != 0)
	{
		fprintf(stderr, "VoIP MIB Import Error: import text failed\n");
		free(text); // free unused data
		return -1;
	}

	free(text); // free unused data

	// check and fix
	mode = cfg_all_tmp.mode;

flash_voip_import_start:
	if ((mode & VOIP_CURRENT_SETTING) && 
		(cfg_all->mode & VOIP_CURRENT_SETTING))
	{
		pVoIPCfgTmp = &cfg_all_tmp.current_setting;
		pVoIPCfg = &cfg_all->current_setting;
		mode &= ~VOIP_CURRENT_SETTING;
	}
	else if ((mode & VOIP_DEFAULT_SETTING) && 
		(cfg_all->mode & VOIP_DEFAULT_SETTING))
	{
		pVoIPCfgTmp = &cfg_all_tmp.default_setting;
		pVoIPCfg = &cfg_all->default_setting;
		mode &= ~VOIP_DEFAULT_SETTING;
	}
	else
	{
		pVoIPCfgTmp = NULL;
		pVoIPCfg = NULL;
	}

	if (pVoIPCfgTmp && pVoIPCfg)
	{
		if (flash_voip_import_fix(pVoIPCfgTmp, pVoIPCfg) != 0)
		{
			fprintf(stderr, "VoIP MIB Import Error: fix failed\n");
			return -1;
		}	

		// check auto config version
		if (pVoIPCfgTmp->auto_cfg_ver && 
			pVoIPCfgTmp->auto_cfg_ver == pVoIPCfg->auto_cfg_ver)
		{
			// the same config, don't need update!
			return 1;
		}

		// reserve current version, and current feature
		pVoIPCfgTmp->version = pVoIPCfg->version;
		pVoIPCfgTmp->feature = pVoIPCfg->feature;
		pVoIPCfgTmp->extend_feature = pVoIPCfg->extend_feature;

		// checking done, update current setting
		memcpy(pVoIPCfg, pVoIPCfgTmp, sizeof(voipCfgParam_t));

		goto flash_voip_import_start;
	}

	return 0;
}
	
int flash_voip_import_from_file(voipCfgAll_t *cfg_all, const char *filename)
{
	char *buf;
	int buf_len;
	int status;

	if (flash_voip_read_file(filename, &buf, &buf_len) != 0)
	{
		fprintf(stderr, "VoIP MIB Import File Error: read failed\n");
		return -1;
	}

	status = flash_voip_import(cfg_all, buf, buf_len);
	if (status < 0)
	{
		fprintf(stderr, "VoIP MIB Import File Error: import failed\n");
		free(buf);
		return -1;
	}

	free(buf);
	return status;
}

#endif

int flash_voip_read_file(const char *filename, char **buf, int *buf_len)
{
	int fd;
	struct stat status;
	int error = -1;
	int size;

	if (stat(filename, &status) < 0)
	{
		fprintf(stderr, "VoIP MIB Read File Error: stat %s failed\n", filename);
		return -1;
	}

	*buf = (char *) malloc(status.st_size);
	if (*buf == NULL)
	{
		fprintf(stderr, "VoIP MIB Read File Error: malloc failed\n");
		return -1;
	}

#ifdef WIN32
	fd = open(filename, O_RDONLY | O_BINARY);
#else
	fd = open(filename, O_RDONLY);
#endif
	if (fd < 0)
	{
		fprintf(stderr, "VoIP MIB Read File Error: open %s failed\n", filename);
		free(*buf);
		goto flash_voip_read_file_end;
	}

	//lseek(fd, 0L, SEEK_SET);
	size = read(fd, *buf, status.st_size);
	if (size != status.st_size)
	{
		fprintf(stderr, "VoIP MIB Read File Error: read failed\n");
		free(*buf);
		goto flash_voip_read_file_end;
	}
	
	// read file ok
	error = 0;
	*buf_len = status.st_size;

flash_voip_read_file_end:
	close(fd);
	return error;
}

#ifdef WIN32

const char *inet_ntop(int af, const void *src, 
	char *dst, size_t cnt)
{
	if (af == AF_INET)
	{
		struct sockaddr_in in;

		memset(&in, 0, sizeof(in));
		in.sin_family = AF_INET;
		memcpy(&in.sin_addr, src, sizeof(struct in_addr));
		getnameinfo((struct sockaddr *)&in, sizeof(struct sockaddr_in), 
			dst, cnt, NULL, 0, NI_NUMERICHOST);
		return dst;
	}
	else if (af == AF_INET6)
	{
		struct sockaddr_in6 in;

		memset(&in, 0, sizeof(in));
		in.sin6_family = AF_INET6;
		memcpy(&in.sin6_addr, src, sizeof(struct in_addr6));
		getnameinfo((struct sockaddr *)&in, sizeof(struct sockaddr_in6), 
			dst, cnt, NULL, 0, NI_NUMERICHOST);
		return dst;
	}
	
	return NULL;
}

int inet_pton(int af, const char *src, void *dst)
{
	struct addrinfo hints, *res, *ressave;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = af;

	if (getaddrinfo(src, NULL, &hints, &res) != 0)
	{
		return -1;
	}

	ressave = res;

	while (res)
	{
		memcpy(dst, res->ai_addr, res->ai_addrlen);
		res = res->ai_next;
	}

	freeaddrinfo(ressave);
	return 1;
}

#endif
