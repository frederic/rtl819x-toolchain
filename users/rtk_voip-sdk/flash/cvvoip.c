/*
 * voipcv.c: VoIP Converting Tool
 *
 * Authors: Rock <shaofu@realtek.com.tw>
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include "voip_flash.h"
#include "voip_flash_mib.h"
#include "voip_flash_tool.h"

static char *copyright="Copyright (c) Realtek Semiconductor Corp., 2006-2008. All Rights Reserved.";

enum {
	FIX_DISPLAY= 0,
	FIX_USERNAME,
	FIX_AUTHNAME,
	FIX_AUTHPWD,
	FIX_END
};

int fix_mib_ids[FIX_END] = {
	MIB_VOIP_PROXY_DISPLAY_NAME,
	MIB_VOIP_PROXY_NUMBER,
	MIB_VOIP_PROXY_LOGIN_ID,
	MIB_VOIP_PROXY_PASSWORD
};

char *fix_mib_names[FIX_END] = {
	"VOIP.PORT[0].PROXIES[0].DISPLAY_NAME",
	"VOIP.PORT[0].PROXIES[0].NUMBER",
	"VOIP.PORT[0].PROXIES[0].LOGIN_ID",
	"VOIP.PORT[0].PROXIES[0].PASSWORD"
};

void usage(char *cmd)
{
	printf("\nVoIP config file converting tool.\n");
	printf("%s Ver %d.%d.\n\n", copyright, VOIP_FLASH_VER, VOIP_FLASH_MIB_VER);
	printf("Usage: %s -in input-file -ot text-file\n", cmd);
	printf("   or: %s -in input-file -ob config-file\n", cmd);
	printf("   or: %s -in input-file batch-file\n", cmd);
	printf("\n");
	printf("arguments:\n");
	printf("   text-file: voip config in text mode\n");
	printf("   config-file: voip config in binary mode\n");
	printf("   input-file: text-file or config-file\n");
	printf("   batch-file: config file for batch mode\n");
	printf("\n");
}

int fix_line(char *line, char *fix_values[], int fix_flags[])
{
	int mode;
	char name[200], full_name[200];
	char value[200];
	const voipMibEntry_T *pmib;
	int i, offset;

	// get name & value
	mode = voip_mibline_from(line, name, value);
	if (mode == VOIP_NONE_SETTING)
	{
		fprintf(stderr, "VoIP Converting Error: parse \"%s\" failed\n", line);
		return -1;
	}

	if (name[0] == 0)
	{
		return 0; // comment line
	}

	// get mib and data offset via name
	if (voip_mib_from(mibtbl_voip, name, &pmib, &offset) != 0)
	{	
		fprintf(stderr, "VoIP Converting Error: couldn't find %s in mib table\n", name);
		return -1;
	}

	// fix value if need
	for (i=0; i<FIX_END; i++)
	{
		if (pmib->id == fix_mib_ids[i])
		{
			strcpy(value, fix_values[i]);
			fix_flags[i] = 1;
			break;
		}
	}

	if (i == FIX_END)
	{
		// no need fix
		return 0;
	}

	if (mode == VOIP_CURRENT_SETTING)
		sprintf(full_name, "VOIP.%s", name);
	else if (mode == VOIP_DEFAULT_SETTING)
		sprintf(full_name, "DEF_VOIP.%s", name);
	
	// do fix value
	if (voip_mibline_to(line, full_name, value) != 0)
	{
		fprintf(stderr, "VoIP Converting Error: write %s failed in %s\n", value, full_name);
		return -1;
	}

	return 0;
}

int create_cfgfile(char *text, int text_len,
	char *mac, char *display, char *username, char *authname, char *authpwd)
{
	FILE *fp;
	char filename[100];
	char *fix_values[FIX_END];
	int fix_flags[FIX_END];
	int i, idx;
	char line[600];

	sprintf(filename, "%s.dat", mac);
	fp = fopen(filename, "w+");
	if (fp == NULL)
	{
		fprintf(stderr, "VoIP Converting Error: create %s failed\n", filename);
		return -1;
	}

	// prepare fix information
	fix_values[0] = display;
	fix_values[1] = username;
	fix_values[2] = authname;
	fix_values[3] = authpwd;
	memset(fix_flags, 0, sizeof(fix_flags));

	// import setting from text data
	for (i=0, idx=0; i<text_len; i++)
	{
		if (text[i] == '\n')
		{
			line[idx++] = '\n';
			line[idx] = 0;
			fix_line(line, fix_values, fix_flags);
			fputs(line, fp);
			idx = 0;
			continue;
		}
	
		if (idx < sizeof(line) - 2) // reserved 2 bytes for '\0' '\n'
			line[idx++] = text[i];
	}

	if (idx > 0)
	{
		line[idx] = 0;
		fix_line(line, fix_values, fix_flags);
		fputs(line, fp);
	}

	// add fix line if not found
	for (i=0; i<FIX_END; i++)
	{
		if (fix_flags[i] == 0)
		{
			sprintf(line, "%s=", fix_mib_names[i]);
			fix_line(line, fix_values, fix_flags);
			fputs(line, fp);
		}
	}

	fclose(fp);
	return 0;
}

int create_substr(char *string, int start, int end, char *substr, int max_size)
{
	int size;

	size = end - start;
	if (start >= 0 && size < max_size)
	{
		memcpy(substr, &string[start], size);
		substr[size] = 0;
		return 0;
	}

	return -1;
}

int do_batch_converting(char *text, int text_len, char *filename)
{
	FILE *fp;
	char line[200];
	regex_t re;
	regmatch_t match[6];
	int status;
	char mac[13];
	char display[DNS_LEN];
	char username[DNS_LEN];
	char authname[DNS_LEN];
	char authpwd[DNS_LEN];

	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "VoIP Converting Error: open %s failed\n", filename);
		return -1;
	}

	if (regcomp(&re, "^\\(.*\\),\\(.*\\),\\(.*\\),\\(.*\\),\\(.*\\)\n$", 0) != 0)
	{
		fprintf(stderr, "VoIP Converting Error: create RE failed\n");
		fclose(fp);
		return -1;
	}

	while (fgets(line, sizeof(line), fp))
	{
		if (line[0] == ';')
			continue;

		status = regexec(&re, line, 6, match, 0);
		if (status != 0)
		{
			fprintf(stderr, "VoIP Converting Error: RE failed\n");
			continue;
		}

		status = create_substr(line, match[1].rm_so, match[1].rm_eo, mac, sizeof(mac)) == 0 &&
			create_substr(line, match[2].rm_so, match[2].rm_eo, display, sizeof(display)) == 0 &&
			create_substr(line, match[3].rm_so, match[3].rm_eo, username, sizeof(username)) == 0 &&
			create_substr(line, match[4].rm_so, match[4].rm_eo, authname, sizeof(authname)) == 0 &&
			create_substr(line, match[5].rm_so, match[5].rm_eo, authpwd, sizeof(authpwd)) == 0;

		if (!status)
		{
			// size not match
			fprintf(stderr, "VoIP Converting Error: size not match\n");
			continue;
		}

		create_cfgfile(text, text_len,
			mac, display, username, authname, authpwd);
	}

	fprintf(stderr, "VoIP Converting Info: Done\n");
	regfree(&re);
	fclose(fp);
	return 0;
}

int main(int argc, char *argv[])
{
	voipCfgAll_t cfg_all;
	char *buf, *text;
	int buf_len, text_len;

	if (argc != 5 && argc != 4)
	{
		usage(argv[0]);
		return -1;
	}

	if (strcmp(argv[1], "-in") != 0)
		return -1;

	if (argc == 5 &&
		strcmp(argv[3], "-ot") != 0 &&
		strcmp(argv[3], "-ob") != 0)
		return -1;

	// read file
	if (flash_voip_read_file(argv[2], &buf, &buf_len) != 0)
	{
		fprintf(stderr, "VoIP Converting Error: read failed\n");
		return -1;
	}

	// decode to text data
	if (flash_voip_decode(buf, buf_len, &text, &text_len) != 0)
	{
		fprintf(stderr, "VoIP Converting Error: decode failed\n");
		free(buf);
		return -1;
	}

	free(buf); // free unused buffer

	// import text for checking
	memset(&cfg_all, 0, sizeof(cfg_all));
	if (flash_voip_import_text(&cfg_all, text, text_len) != 0)
	{
		fprintf(stderr, "VoIP Converting Error: import text failed\n");
		free(text);
		return -1;
	}

	// don't need check feature here
	// it will check different feature on importing to flash
	if ((cfg_all.mode & VOIP_CURRENT_SETTING) &&
		(flash_voip_import_check(&cfg_all.current_setting) < 0))
	{
		fprintf(stderr, "VoIP Converting Error: import check failed\n");
		free(text);
		return -1;
	}

	if ((cfg_all.mode & VOIP_DEFAULT_SETTING) &&
		(flash_voip_import_check(&cfg_all.default_setting) < 0))
	{
		fprintf(stderr, "VoIP Converting Error: import check failed\n");
		free(text);
		return -1;
	}

	if (argc == 4)
	{
		int status;

		status = do_batch_converting(text, text_len, argv[3]);
		free(text);
		return status;
	}

	if (argc == 5 && strcmp(argv[3], "-ot") == 0)
	{
		// output to text file directly
		if (flash_voip_write_file(argv[4], text, text_len) != 0)
		{
			fprintf(stderr, "VoIP MIB Convert Error: write failed\n");
			free(text);
		}
		return 0;
	}

	if (argc == 5 && strcmp(argv[3], "-ob") == 0)
	{
		// config file
		// 1. encode to config format
		if (flash_voip_encode(text, text_len, &buf, &buf_len) != 0)
		{
			fprintf(stderr, "VoIP Converting Error: encode failed\n");
			free(text);
			return -1;
		}

		free(text); // free unused buffer

		// 2. output to config file
		if (flash_voip_write_file(argv[4], buf, buf_len) != 0)
		{
			fprintf(stderr, "VoIP Converting Error: write failed\n");
			free(buf);
			return -1;
		}
		
		free(buf);
		return 0;
	}

	usage(argv[0]);
	return -1;
}
