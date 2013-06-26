// Page1.cpp : implementation file
//

#include "stdafx.h"
#include <io.h>
#undef interface	// mib use 'interface' as a name 
#include "cvvoip.h"
#include "Page1.h"
#include "voip_flash.h"
#include "voip_flash_mib.h"
#include "voip_flash_tool.h"
#include "cvcfg.h"

#define __STDC__ 1
#ifdef __cplusplus
extern "C" {
#endif
#include "regex-0.12/regex.h"
#ifdef __cplusplus
}
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPage1 property page

IMPLEMENT_DYNCREATE(CPage1, CPropertyPage)

CPage1::CPage1() : CPropertyPage(CPage1::IDD)
{
	//{{AFX_DATA_INIT(CPage1)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPage1::~CPage1()
{
}

void CPage1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPage1)
	DDX_Control(pDX, IDC_EDIT3, m_edtKey);
	DDX_Control(pDX, IDC_EDIT2, m_edtCfgFile);
	DDX_Control(pDX, IDC_EDIT1, m_edtMACList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPage1, CPropertyPage)
	//{{AFX_MSG_MAP(CPage1)
	ON_BN_CLICKED(IDC_BUTTON1, OnMACList)
	ON_BN_CLICKED(IDC_BUTTON2, OnConfigFile)
	ON_BN_CLICKED(IDC_BUTTON3, OnBatchConvert)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPage1 message handlers

BOOL CPage1::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPage1::OnMACList() 
{
	char szFilters[]=
		"MAC Files (*.mac)|*.mac|All Files (*.*)|*.*||";
	CFileDialog fileDlg(TRUE, NULL, NULL,
		OFN_FILEMUSTEXIST, szFilters, this);

	fileDlg.m_ofn.lpstrInitialDir = ((CCVVoIPApp *) AfxGetApp())->m_curdir;
	if (fileDlg.DoModal() == IDOK)
	{
		m_edtMACList.SetWindowText(fileDlg.GetPathName());
	}
}

void CPage1::OnConfigFile() 
{
	char szFilters[]= "*.dat,*.txt|*.dat;*.txt||";
	CFileDialog fileDlg(TRUE, NULL, NULL,
		OFN_FILEMUSTEXIST, szFilters, this);

	fileDlg.m_ofn.lpstrInitialDir = ((CCVVoIPApp *) AfxGetApp())->m_curdir;
	if (fileDlg.DoModal() == IDOK)
	{
		m_edtCfgFile.SetWindowText(fileDlg.GetPathName());
	}
}

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

int create_txtfile(char *text, int text_len,
	char *mac, char *display, char *username, char *authname, char *authpwd,
	char *filename)
{
	FILE *fp;
	char *fix_values[FIX_END];
	int fix_flags[FIX_END];
	int i, idx;
	char line[600];

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
	int i, size;

	if (start < 0)
		return -1;
	
	// remove leading space
	for (i=start; i<end && string[i] == ' '; i++);

	size = end - i;
	if (size < max_size)
	{
		memcpy(substr, &string[i], size);
		substr[size] = 0;
		return 0;
	}

	return -1;
}

int text2config(char *in_filename, char *out_filename)
{
	char *buf, *text;
	int buf_len, text_len;

	// read file
	if (flash_voip_read_file(in_filename, &buf, &buf_len) != 0)
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

	// To config file
	// 1. encode to config format
	if (flash_voip_encode(text, text_len, &buf, &buf_len) != 0)
	{
		fprintf(stderr, "VoIP Converting Error: encode failed\n");
		free(text);
		return -1;
	}

	free(text); // free unused buffer

	// 2. output to config file
	if (flash_voip_write_file(out_filename, buf, buf_len) != 0)
	{
		fprintf(stderr, "VoIP Converting Error: write failed\n");
		free(buf);
		return -1;
	}
		
	free(buf);
	return 0;
}

int do_batch_converting(struct all_config *allConfig, char *filename)
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
	char outFilename[256];
	int flag=1; //for upgrade
	struct all_config *tmpConfig;

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
		tmpConfig=(struct all_config *)malloc(sizeof(struct all_config));
		memset(tmpConfig, 0, sizeof(struct all_config));
		memcpy((void *)tmpConfig, allConfig, sizeof(struct all_config));
		if(tmpConfig->csmib_exist == 1){
			// change the cs confgiuration
			memcpy((void *)tmpConfig->csmib.voipCfgParam.ports[0].proxies[0].display_name,
				display, sizeof(display));
			memcpy((void *)tmpConfig->csmib.voipCfgParam.ports[0].proxies[0].number,
				username, sizeof(username));
			memcpy((void *)tmpConfig->csmib.voipCfgParam.ports[0].proxies[0].login_id,
				authname, sizeof(authname));
			memcpy((void *)tmpConfig->csmib.voipCfgParam.ports[0].proxies[0].password,
				authpwd, sizeof(authpwd));
		}

		if(tmpConfig->dsmib_exist == 1){
			// change the default confgiuration
			memcpy((void *)tmpConfig->dsmib.voipCfgParam.ports[0].proxies[0].display_name,
				display, sizeof(display));
			memcpy((void *)tmpConfig->dsmib.voipCfgParam.ports[0].proxies[0].number,
				username, sizeof(username));
			memcpy((void *)tmpConfig->dsmib.voipCfgParam.ports[0].proxies[0].login_id,
				authname, sizeof(authname));
			memcpy((void *)tmpConfig->dsmib.voipCfgParam.ports[0].proxies[0].password,
				authpwd, sizeof(authpwd));
		}

		// create config file
		sprintf(outFilename, "%s.dat", mac);
		generateBinFile_win32(TARGET_MODE, outFilename, flag, tmpConfig);
		free(tmpConfig);
	}

	fprintf(stderr, "VoIP Converting Info: Done\n");
	regfree(&re);
	fclose(fp);
	return 0;
}

void CPage1::OnBatchConvert() 
{
	voipCfgAll_t cfg_all;
	char mac_filename[256];
	char cfg_filename[256];
	char key[32];
	int status;
	int inFileType;
	struct all_config allCfg;

	memset(mac_filename, 0, sizeof(mac_filename));
	memset(cfg_filename, 0, sizeof(cfg_filename));
	memset(key, 0, sizeof(key));
	
	m_edtMACList.GetWindowText(mac_filename, sizeof(mac_filename));
	m_edtCfgFile.GetWindowText(cfg_filename, sizeof(cfg_filename));
	m_edtKey.GetWindowText(key, sizeof(key));

	inFileType=checkFileType_win32(cfg_filename);
	if (inFileType == 0) {
		MessageBox("Invalid config file!");
		return;
	}
	memset(&allCfg, '\0', sizeof(struct all_config) );
	switch (inFileType & MODE_MASK) {
		case PC_MODE:
		case TARGET_MODE:
			if ( parseBinConfig_win32(inFileType, cfg_filename, &allCfg) < 0) {
				MessageBox("Parse binary file error!");
				return;
			}
			break;

		case TXT_MODE:
			if ( parseTxtConfig_win32(cfg_filename, &allCfg) < 0) {
				MessageBox("Parse text file error!");
				return;
			}

			break;
	}
	memcpy((void *)&cfg_all.current_setting, (const void *)&allCfg.csmib.voipCfgParam, sizeof(voipCfgParam_t));
	memcpy((void *)&cfg_all.default_setting, (const void *)&allCfg.dsmib.voipCfgParam, sizeof(voipCfgParam_t));
	// don't need check feature here
	// it will check different feature on importing to flash
	if ((inFileType& TYPE_MASK) & DS_TYPE)
	{
		cfg_all.mode=VOIP_DEFAULT_SETTING;
		if(flash_voip_import_check(&cfg_all.default_setting) < 0)
		{
			MessageBox("VoIP Converting Error: import check failed\n");
			return;
		}
	}
	if ((inFileType& TYPE_MASK) & CS_TYPE)
	{
		cfg_all.mode=VOIP_CURRENT_SETTING;
		if(flash_voip_import_check(&cfg_all.current_setting) < 0)
		{
			MessageBox("VoIP Converting Error: import check failed\n");
			return;
		}
	}
	status = do_batch_converting(&allCfg, mac_filename);

	if (status == 0)
		MessageBox("Converting done.");
	else
		MessageBox("Converting failed.");

}
