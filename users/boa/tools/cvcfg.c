/*
 *      AP MIB binary<->text convertion tool
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: cvcfg.c,v 1.17 2009/07/30 11:32:12 keith_huang Exp $
 *
 */
    
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#ifndef WIN32
	#include <unistd.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#else
	#include <Winsock2.h>
	#include <io.h>
#endif

#include "apmib.h"
#include "mibtbl.h"

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
#include "voip_flash.h"
#include "voip_flash_mib.h"
#endif

//#define DEBUG_CVCFG 1
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

#define SPACE			(' ')
#define EOL			('\n')
#define TAB			('\x9')

// MIB value, id mapping table
typedef struct _mib_table_ {
	int id;
	char *name;
} mib_table_T;

#ifdef COMPRESS_MIB_SETTING
int cvcfg_mib_compress_write(CONFIG_DATA_T type, char *data, PARAM_HEADER_T *pheader, int fh, int *pcomplen);
#ifdef MIB_TLV
void mib_display_data_content(CONFIG_DATA_T type, unsigned char * pdata, unsigned int mib_data_len);
void mib_display_tlv_content(CONFIG_DATA_T type, unsigned char * ptlv, unsigned int mib_tlv_len);
unsigned int mib_get_setting_len(CONFIG_DATA_T type);
unsigned int mib_tlv_save(CONFIG_DATA_T type, void *mib_data, unsigned char *mib_tlvfile, unsigned int *tlv_content_len);
unsigned int mib_tlv_init_from(const mib_table_entry_T *mib_root_tbl, unsigned char *pdata_array, void *pfile, unsigned int data_size, unsigned int *pmib_root_offset);
unsigned int mib_tlv_init(const mib_table_entry_T *mib_tbl, unsigned char *from_data, void *pfile, unsigned int tlv_content_len);
#endif //  #ifdef MIB_TLV
#endif

#ifdef WIN32
typedef unsigned long in_addr_t;
extern int inet_aton(const char *cp, struct in_addr *addr);
#endif

#ifdef DEBUG_CVCFG
static int flash_write_file(char *buf, int len , char * filename);
#endif

//////////////////////////////////////////////////////////////////////////////
static int _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}

#if 0
// Validate digit
static int _isdigit(char c)
{
    return ((c >= '0') && (c <= '9'));
}
#endif

//////////////////////////////////////////////////////////////////////////////
static int string_to_hex(char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii=0;
	for (idx=0; idx<len; idx+=2) {
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx+1];
		tmpBuf[2] = 0;
		if ( !_is_hex(tmpBuf[0]) || !_is_hex(tmpBuf[1]))
			return 0;

		key[ii++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}

//////////////////////////////////////////////////////////////////////////////
static char *getVal(char *value, char **p)
{
	int len=0;

	while (*value == ' ' ) value++;

	*p = value;

	while (*value && *value!=',') {
		value++;
		len++;
	}

	if ( !len ) {
		*p = NULL;
		return NULL;
	}

	if ( *value == 0)
		return NULL;

	*value = 0;
	value++;

	return value;
}

////////////////////////////////////////////////////////////////////////////
#define WRITE_LINE(format, prefix, name, value) { \
	sprintf(line, format, prefix, name, value); \
	fwrite(line, 1, strlen(line), fp); \
}

/////////////////////////////////////////////////////////////////////////////
static int acNum;

//#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_) Keith remove
static int meshAclNum;
//#endif Keith remove
static int scheduleRuleNum;

#ifdef HOME_GATEWAY
static int macFilterNum, portFilterNum, ipFilterNum, portFwNum, triggerPortNum;

#if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)
static int qosRuleNum;
#endif

#if defined(CONFIG_RTL_8198_AP_ROOT) || defined(HOME_GATEWAY) //defined(VLAN_CONFIG_SUPPORTED) Keith Modify
#if defined(VLAN_CONFIG_SUPPORTED)
static int vlanRuleNum;
#endif
#endif

#endif

#ifdef HOME_GATEWAY
#ifdef VPN_SUPPORT
static int  ipsecTunnelNum;
#endif
#endif
static int wdsNum;
static int disable_hwsetting=0;

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

static struct all_config config;

/* Local function decalaration */
static void showHelp(void);
static int checkFileType(const char *filename);
static int parseBinSectionConfig(int type,int fh,struct all_config *pMib);
static int parseBinConfig(int type, const char *filename, struct all_config *pConfig);
static int parseTxtConfig(const char *filename, struct all_config *pConfig);
static int getToken(char *line, char *value, int *def_flag, int *hw_tbl, int *wlan_inx, int *vwlan_inx);
static int generateBinFile(int type, const char *filename, int flag);
static int generateTxtFile(const char *filename);
static int set_mib(struct all_config *pConfig, int id, void *value, int def_mib, int hw_tbl, int idx, int v_idx);
static void swap_mib_word_value(APMIB_Tp pMib);
static void getVal2(char *value, char **p1, char **p2);
static void getVal5(char *value, char **p1, char **p2, char **p3, char **p4, char **p5);
#ifdef HOME_GATEWAY
static void getVal3(char *value, char **p1, char **p2, char **p3);
static void getVal4(char *value, char **p1, char **p2, char **p3, char **p4);
#endif
#if defined(HOME_GATEWAY) || defined(CONFIG_RTL_8198_AP_ROOT)
static void getVal7(char *value, char **p1, char **p2, char **p3, char **p4, char **p5, char **p6, char **p7);
#endif
#ifdef HOME_GATEWAY
#ifdef VPN_SUPPORT
static void getVal24(char *value, char **p1, char **p2, char **p3, char **p4, char **p5, char **p6, char **p7,\
        char **p8, char **p9, char **p10, char **p11, char **p12, char **p13, char **p14, char **p15, char **p16,\
	char **p17, char **p18, char **p19, char **p20, char **p21, char **p22, char **p23, char **p24);
//#define IPSECTUNNEL_FORMAT ("%d, %d, %s, %d, %s, %d, %d, %s , %d, %s, %d, %d, %d, %d, %d, %d, %s, %d, %d, %d, %lu, %lu, %d, %s, %s, %s")
#define IPSECTUNNEL_FORMAT ("%d, %d, %s, %d, %s, %d, %d, %s , %d, %s, %d, %d,  %d, %d,  %s, %d, %d, %d, %lu, %lu, %d, %s, %s, %s")
#endif

#ifdef CONFIG_IPV6
static void getVal33(char *value, char **p1, char **p2, char **p3, char **p4, char **p5, char **p6, \
 		 char **p7,  char **p8,  char **p9,  char **p10, char **p11, char **p12, char **p13,\
		 char **p14, char **p15, char **p16, char **p17, char **p18, char **p19, char **p20,\
		 char **p21, char **p22, char **p23, char **p24, char **p25, char **p26, char **p27,\
		 char **p28, char **p29, char **p30, char **p31, char **p32, char **p33);

static void getVal19(char *value, char **p1, char **p2, char **p3, char **p4, char **p5, char **p6,\
		 char  **p7, char **p8,  char **p9, char **p10, char **p11, char **p12, char **p13,\
		 char **p14, char **p15, char **p16,char **p17, char **p18, char **p19);

#define RADVD_FORMAT ("%u, %s, %u, %u, %u, %u, %u, %u, %u, %u ,%u, %u, %s, %u, %u, %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x, %u, %u, %u, %u, %u, %u, %s, %u, %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x, %u, %u, %u, %u, %u, %u, %s, %u")
#define DNSV6_FORMAT ("%d, %s")
#define DHCPV6S_FORMAT ("%d, %s, %s, %s, %s")
#define ADDR6_FORMAT ("%d, %d, %d, %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x, %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x")
#endif
#endif
/////////////////////////////////////////////////////////////////////////////
static char *copyright="Copyright (c) Realtek Semiconductor Corp., 2007. All Rights Reserved.";
static char *version="1.0";

/////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	int argNum=1, outFileType=0, inFileType, flag, raw_data=0;
#ifdef COMPACT_FILENAME_BUFFER
	const char *inFile = NULL, *outFile = NULL;
#else
	char inFile[80]={0}, outFile[80]={0};
#endif
	char imsg[100]={0},omsg[100]={0};

	while (argNum < argc) {
		if ( !strcmp(argv[argNum], "-in") ) {
			if (++argNum >= argc)
				break;
#ifdef COMPACT_FILENAME_BUFFER
			inFile = argv[argNum];
#else
			sscanf(argv[argNum], "%s", inFile);
#endif
		}
		else if ( !strcmp(argv[argNum], "-ot") ) {
			if (++argNum >= argc)
				break;
			outFileType = TXT_MODE;
#ifdef COMPACT_FILENAME_BUFFER
			outFile = argv[argNum];
#else
			sscanf(argv[argNum], "%s", outFile);
#endif
		}
		else if ( !strcmp(argv[argNum], "-ogu") ) {
			if (++argNum >= argc)
				break;
			outFileType = TARGET_MODE;
#ifdef COMPACT_FILENAME_BUFFER
			outFile = argv[argNum];
#else
			sscanf(argv[argNum], "%s", outFile);
#endif
			flag = 1;	// update
		}
		else if ( !strcmp(argv[argNum], "-ogg") ) {
			if (++argNum >= argc)
				break;
			outFileType = TARGET_MODE;
#ifdef COMPACT_FILENAME_BUFFER
			outFile = argv[argNum];
#else
			sscanf(argv[argNum], "%s", outFile);
#endif
			flag = 0;	// upgrade
		}
		else if ( !strcmp(argv[argNum], "-ogf") ) {
			if (++argNum >= argc)
				break;
			outFileType = TARGET_MODE;
#ifdef COMPACT_FILENAME_BUFFER
			outFile = argv[argNum];
#else
			sscanf(argv[argNum], "%s", outFile);
#endif
			flag = 2;	// force
		}
		else if ( !strcmp(argv[argNum], "-op") ) {
			if (++argNum >= argc)
				break;
			outFileType = PC_MODE;
#ifdef COMPACT_FILENAME_BUFFER
			outFile = argv[argNum];
#else
			sscanf(argv[argNum], "%s", outFile);
#endif
		}
		else if ( !strcmp(argv[argNum], "-h") ||
				!strcmp(argv[argNum], "/h") ) {
			showHelp();
			exit(1);
		}
		else if ( !strcmp(argv[argNum], "-r") ) {
			raw_data = 1;
		}
		else if ( !strcmp(argv[argNum], "-no_hw") ) {
			disable_hwsetting = 1;
		}		
		else {
#ifdef COMPACT_FILENAME_BUFFER
			if( inFile == NULL )
				inFile = argv[argNum];
			else if( outFile == NULL ) {
				outFileType = TARGET_MODE;
				outFile = argv[argNum];
				flag = 1;	// update
			}
#else
			if ( !inFile[0] ) {
				sscanf(argv[argNum], "%s", inFile);
			}
			else if (!outFile[0]) {
				outFileType = TARGET_MODE;
				sscanf(argv[argNum], "%s", outFile);
				flag = 1;	// update
			}
#endif
		}
		argNum++;
	}

#ifdef COMPACT_FILENAME_BUFFER
	if( inFile == NULL || outFile == NULL )
#else
	if ( !inFile[0] || !outFile[0] ) 
#endif
	{
		showHelp();
		exit(1);
	}

	if (raw_data)
		flag |= RAW_TYPE;

	inFileType=checkFileType(inFile);
	if (inFileType == 0) {
		printf("\nInvalid input file!\n");
		exit(1);
	}

	if ( (inFileType & MODE_MASK) == TXT_MODE )
		strcat(imsg, "TXT");
	else {
		if ((inFileType & MODE_MASK) == PC_MODE )
			strcat(imsg, "PC");
		else
			strcat(imsg, "TARGET");
		if ((inFileType& TYPE_MASK) & HS_TYPE)
			strcat(imsg, "-HS");
		if ((inFileType& TYPE_MASK) & DS_TYPE)
			strcat(imsg, "-DS");
		if ((inFileType& TYPE_MASK) & CS_TYPE)
			strcat(imsg, "-CS");
	}


	if ( (outFileType & MODE_MASK) == TXT_MODE )
		strcat(omsg, "TXT");
	else {
		if ((outFileType & MODE_MASK) == TARGET_MODE)
			strcat(omsg, "TARGET");
		else
			strcat(omsg, "PC");
	}

	printf("\nConverting %s to %s\n", imsg, omsg);

	memset(&config, '\0', sizeof(config) );

	switch (inFileType & MODE_MASK) {
		case PC_MODE:
		case TARGET_MODE:
			if ( parseBinConfig(inFileType, inFile, &config) < 0) {
				printf("Parse binary file error!\n");
				exit(1);
			}
			break;

		case TXT_MODE:
			if ( parseTxtConfig(inFile, &config) < 0) {
				printf("Parse text file error!\n");
				exit(1);
			}
			break;
	}
	switch (outFileType & MODE_MASK) {
		case PC_MODE:
		case TARGET_MODE:
			if ( generateBinFile(outFileType, outFile, flag) < 0) {
				printf("Generate binary output file error!\n");
				exit(1);
			}
#ifdef WIN32
			_chmod(outFile, S_IREAD);
#else
			chmod(outFile,  DEFFILEMODE);
#endif
			break;

		case TXT_MODE:
			if ( generateTxtFile(outFile) < 0) {
				printf("Generate text output file error!\n");
				exit(1);
			}
#ifdef WIN32
			_chmod(outFile, S_IREAD);
#else
			chmod(outFile,  DEFFILEMODE);
#endif
			break;
	}

	printf(" => %s ok.\n", outFile);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
static void showHelp(void)
{
	printf("\nRTL865x config file converting tool [%s].\n", 
#ifdef HOME_GATEWAY
		"gw"
#else
		"ap"
#endif
		);

	printf("%s Ver %s.\n\n", copyright, version);
	printf("Usage: arguments...\n");
	printf("  Where arguments are:\n");
	printf("    -in input_filename, specify the input filename.\n");
	printf("    -ot out_filename, generate a output filename in text mode.\n");
	printf("    -ogu out_filename, generate a output filename for target mode [update].\n");
	printf("    -ogg out_filename, generate a output filename for target mode [upgrade].\n");
	printf("    -ogf out_filename, generate a output filename for target mode [force].\n");
	printf("    -op out_filename, generate a output filename PC mode.\n");
	printf("    -no_hw, do not to generate hw setting.\n");
	printf("    -r generate raw configutation with pad.\n\n");	
}

////////////////////////////////////////////////////////////////////////////////
static int checkFileType(const char *filename)
{
	int fh=0;
	struct stat status;
	char *buf=NULL, *ptr;
	int flag=0, len;
	COMPRESS_MIB_HEADER_Tp pHeader;

	if ( stat(filename, &status) < 0 )
		goto back;

	buf = malloc(status.st_size);

	if ( buf == NULL)
		goto back;
#ifdef WIN32
	fh = open(filename, O_RDONLY|O_BINARY);
#else
	fh = open(filename, O_RDONLY);
#endif
	if ( fh == -1 )
		goto back;

	lseek(fh, 0L, SEEK_SET);
	if ( read(fh, buf, status.st_size) != status.st_size)
		goto back;

	// Assume PC config file
	flag = PC_MODE;
	//printf("###flag=%d,buf=%s,<%s>%d\n",flag,buf,__FUNCTION__,__LINE__);
//	len = HW_SETTING_OFFSET;
	len = 0;
	pHeader = (COMPRESS_MIB_HEADER_Tp)&buf[len];
	//printf("###status.st_size=%d,pHeaderllen=%x,sizeof(COMPRESS_MIB_HEADER_T)=%d,<%s>%d\n",status.st_size,pHeader->compLen,sizeof(COMPRESS_MIB_HEADER_T),__FUNCTION__,__LINE__);
	if ( (int)(pHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T)) > status.st_size )
		goto check_target;
	//printf("###flag=%d,<%s>%d\n",flag,__FUNCTION__,__LINE__);

	ptr = &buf[len+sizeof(COMPRESS_MIB_HEADER_T)];
	//printf("###flag=%d,<%s>%d\n",flag,__FUNCTION__,__LINE__);
	if (memcmp(pHeader->signature, COMP_HS_SIGNATURE, COMP_SIGNATURE_LEN))
		goto check_target;
	//printf("###flag=%d,<%s>%d\n",flag,__FUNCTION__,__LINE__);
	flag |= HS_TYPE;

	len = DEFAULT_SETTING_OFFSET;
	pHeader = (COMPRESS_MIB_HEADER_Tp)&buf[len];
	ptr = &buf[len+sizeof(COMPRESS_MIB_HEADER_T)];
//	if ( !CHECKSUM_OK(ptr, pHeader->len) )
//		goto check_target;
	//printf("###flag=%d,<%s>%d\n",flag,__FUNCTION__,__LINE__);

	if (memcmp(pHeader->signature, COMP_DS_SIGNATURE, COMP_SIGNATURE_LEN))
		goto check_target;
	//printf("###flag=%d,<%s>%d\n",flag,__FUNCTION__,__LINE__);
	flag |= DS_TYPE;

	len = CURRENT_SETTING_OFFSET;
	pHeader = (COMPRESS_MIB_HEADER_Tp)&buf[len];
	//if ( !CHECKSUM_OK(ptr, pHeader->len) )
	//	goto check_target;
	//printf("###flag=%d,<%s>%d\n",flag,__FUNCTION__,__LINE__);
	if (memcmp(pHeader->signature, COMP_CS_SIGNATURE, COMP_SIGNATURE_LEN))
		goto check_target;
	//printf("###flag=%d,<%s>%d\n",flag,__FUNCTION__,__LINE__);
	flag |= CS_TYPE;
	goto back;

//printf("###flag=%d,<%s>%d\n",flag,__FUNCTION__,__LINE__);
check_target:
	// Assume target mode
	flag = TARGET_MODE;
	pHeader = (COMPRESS_MIB_HEADER_Tp)buf;
	//printf("###tgtHeader->compLen=%x,<%s>%d\n",pHeader->compLen,__FUNCTION__,__LINE__);
	len = 0;


	while (len < status.st_size) 
	{	
		pHeader->compLen = DWORD_SWAP(pHeader->compLen);
		if ( (int)(pHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T)) <= status.st_size ) 
		{
			ptr = &buf[len+sizeof(COMPRESS_MIB_HEADER_T)];
			//DECODE_DATA(ptr, pHeader->compLen);
			if (1)
			{//CHECKSUM_OK(ptr, ptgtHeader->compLen) ) {

				if (!memcmp(pHeader->signature, HW_SETTING_HEADER_TAG, TAG_LEN) ||
					!memcmp(pHeader->signature, HW_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ||
					!memcmp(pHeader->signature, HW_SETTING_HEADER_FORCE_TAG, TAG_LEN)||
					!memcmp(pHeader->signature, COMP_HS_SIGNATURE,COMP_SIGNATURE_LEN))
					flag |= HS_TYPE;
				else if (!memcmp(pHeader->signature, CURRENT_SETTING_HEADER_TAG, TAG_LEN) ||
						!memcmp(pHeader->signature, CURRENT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ||
						!memcmp(pHeader->signature, CURRENT_SETTING_HEADER_FORCE_TAG, TAG_LEN) ||
						!memcmp(pHeader->signature, COMP_CS_SIGNATURE,COMP_SIGNATURE_LEN))
					flag |= CS_TYPE;
				else if (!memcmp(pHeader->signature, DEFAULT_SETTING_HEADER_TAG, TAG_LEN) ||
						!memcmp(pHeader->signature, DEFAULT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ||
						!memcmp(pHeader->signature, DEFAULT_SETTING_HEADER_FORCE_TAG, TAG_LEN)|| 
						!memcmp(pHeader->signature, COMP_DS_SIGNATURE,COMP_SIGNATURE_LEN))
					flag |= DS_TYPE;

				len = len + sizeof(COMPRESS_MIB_HEADER_T) + pHeader->compLen;						
				pHeader = (COMPRESS_MIB_HEADER_Tp)&buf[len];
			}
			else
				break;
		}
		else
			break;
	}
	if ( !(flag & TYPE_MASK) )
		flag = TXT_MODE;	// if no config found, set as txt mode
back:
	if (fh >0)
		close(fh);
	if (buf)
		free(buf);

	return (flag);
}
static int parseBinSectionConfig(int type,int fh,struct all_config *pMib)//output set pMib
{
	int ver = -1;
	PARAM_HEADER_T Header;
	int i=0;

	COMPRESS_MIB_HEADER_T compHeader;
	char *pdata = NULL;
	char *pOutData = NULL;
	int outLen = 0;
	if ( read(fh, &compHeader, sizeof(COMPRESS_MIB_HEADER_T)) != sizeof(COMPRESS_MIB_HEADER_T)) {
		printf("Read hs header failed!\n");
		return -1;
	}
		compHeader.compLen= DWORD_SWAP(compHeader.compLen);		
		compHeader.compRate = WORD_SWAP(compHeader.compRate);
		outLen = compHeader.compLen * compHeader.compRate;
		pdata = malloc(compHeader.compLen);
		pOutData = malloc(outLen);
		if(!pdata||!pOutData)
		{
			printf("malloc failed!");
			return -1;
		}else
		{
			memset(pdata,0,compHeader.compLen);
			memset(pOutData,0,outLen);
		}		

		
		if (read(fh, pdata, compHeader.compLen) != compHeader.compLen) {
			printf("Read hw mib failed!\n");
			return -1;
		}
		
//do decode
	outLen = Decode(pdata,compHeader.compLen,pOutData);	
	//printf("indata=%s,outdata=%s,%s,%d\n",pdata,pOutData,__FUNCTION__,__LINE__);
	//printf("inlen = %d,outlen=%d,sizeof(PARAM_HEADER_T):%d\n",compHeader.compLen,outLen,sizeof(PARAM_HEADER_T));

	memcpy(&Header,pOutData,sizeof(PARAM_HEADER_T));
	Header.len = WORD_SWAP(Header.len);//important!
#ifdef MIB_TLV
	mib_table_entry_T *pmib_tl = NULL;
	unsigned char *MibData = NULL;
	unsigned int tlv_content_len = 0;
	

	if (type& HS_TYPE)
	{
		tlv_content_len = sizeof(HW_SETTING_T);
		MibData = malloc(tlv_content_len); 
		if(MibData != NULL)
			memset(MibData, 0x00, tlv_content_len);
		pmib_tl = mib_get_table(HW_SETTING);
	}else if (type& DS_TYPE)
	{
		tlv_content_len = sizeof(APMIB_T);
		MibData = malloc(tlv_content_len); 
		if(MibData != NULL)
			memset(MibData, 0x00, tlv_content_len);
		pmib_tl = mib_get_table(DEFAULT_SETTING);
	}else if (type&CS_TYPE)
	{
		tlv_content_len = sizeof(APMIB_T);
		MibData = malloc(tlv_content_len); 
		if(MibData != NULL)
			memset(MibData, 0x00, tlv_content_len);
		pmib_tl = mib_get_table(CURRENT_SETTING);
	}else
	{
		printf("match no section!\n");
		return -1;
	}	
	unsigned int tlv_checksum = 0;	
	if(pOutData != NULL)
		tlv_checksum = CHECKSUM_OK(pOutData+sizeof(PARAM_HEADER_T), Header.len);
	if(0==tlv_checksum)
	{
		printf("Checksum fail!!\nInput file invalid!\n");
		return -1;
	}

	#ifdef DEBUG_CVCFG
	flash_write_file(pOutData+sizeof(PARAM_HEADER_T),outLen-sizeof(PARAM_HEADER_T)-1,"in_tlv.raw");
	#endif

	if(tlv_checksum == 1 && mib_tlv_init(pmib_tl, pOutData+sizeof(PARAM_HEADER_T),
	(void*)MibData, outLen-sizeof(PARAM_HEADER_T)-1) == 1) //-1 for checksum
	{

		if(type& HS_TYPE )
		{
			sprintf(&Header.signature[TAG_LEN], "%02d", HW_SETTING_VER);
			Header.len = sizeof(HW_SETTING_T);
			//MibData[Header.len-1]  = CHECKSUM(MibData, Header.len-1);
		}else if(type& DS_TYPE)
		{
		//fprintf(stderr,"MibData=0x%x  len=%d,%d\n",MibData,tlv_content_len,__LINE__);
		//fprintf(stderr,"wlan0ssid=%s  %d\n",MibData+7338,__LINE__);
		//return 0;
			sprintf(&Header.signature[TAG_LEN], "%02d", DEFAULT_SETTING_VER);
			Header.len = sizeof(APMIB_T);
			//MibData[Header.len-1]  = CHECKSUM(MibData, Header.len-1);

		}else if(type&CS_TYPE )
		{
			sprintf(&Header.signature[TAG_LEN], "%02d", CURRENT_SETTING_VER);
			Header.len = sizeof(APMIB_T);
			//MibData[Header.len-1]  = CHECKSUM(MibData, Header.len-1);
		}
				
	
		if(pOutData!= NULL)
			free(pOutData);
	
		pOutData = malloc(sizeof(PARAM_HEADER_T)+Header.len);
		memcpy(pOutData, &Header, sizeof(PARAM_HEADER_T));
		memcpy(pOutData+sizeof(PARAM_HEADER_T), MibData, Header.len);
	}	else{printf("mib_tlv_init fail!\n");}
	
	if(MibData != NULL)
		free(MibData);
	
#endif
		

	if(type& HS_TYPE)
	{
		if(Header.len >  sizeof(HW_SETTING_T) + 100)
		{
			printf("hs length too large!sizeof(HW_SETTING_T)=%d\n",sizeof(HW_SETTING_T));
			return -1;
		}
		memcpy(&pMib->hwmib,pOutData+sizeof(PARAM_HEADER_T),Header.len);
		pMib->hwmib_exist = 1;
		sscanf(&Header.signature[TAG_LEN], "%02d", &ver);
		pMib->hwmib_ver = ver;
		pMib->hwmib_len = Header.len;
	}else if(type& DS_TYPE)
	{
		if(Header.len >  sizeof(APMIB_T) + 100)
		{
			printf("ds length too large!%d\n",sizeof(APMIB_T));
			return -1;
		}
		memcpy(&pMib->dsmib,pOutData+sizeof(PARAM_HEADER_T),Header.len);
//fprintf(stderr,"wlan[0][0]=%s\n",config.dsmib.wlan[0][0].ssid);
		pMib->dsmib_exist = 1;
		sscanf(&Header.signature[TAG_LEN], "%02d", &ver);
		pMib->dsmib_ver = ver;
		pMib->dsmib_len = Header.len;
	}else if(type&CS_TYPE)
	{
		if(Header.len >  sizeof(APMIB_T) + 100)
		{
			printf("cs length too large!%d\n",sizeof(APMIB_T));
			return -1;
		}
		memcpy(&pMib->csmib,pOutData+sizeof(PARAM_HEADER_T),Header.len);
		pMib->csmib_exist = 1;
		sscanf(&Header.signature[TAG_LEN], "%02d", &ver);
		pMib->csmib_ver = ver;
		pMib->csmib_len = Header.len;
	}
	free(pOutData);
	return 0;
}
////////////////////////////////////////////////////////////////////////////////
static int parseBinConfig(int type, const char *filename, struct all_config *pMib)
{
	int fh, ver;
	char *data=NULL;	
	//PARAM_HEADER_T header;
#ifdef WIN32
	fh = open(filename, O_RDONLY|O_BINARY);
#else
	fh = open(filename, O_RDONLY);
#endif
	if ( fh == -1 )
		return -1;

	lseek(fh, 0, SEEK_SET);

	// Read hw setting
	if ( (type & TYPE_MASK) & HS_TYPE ) {
		if ((type & MODE_MASK) == PC_MODE)
			lseek(fh, HW_SETTING_OFFSET, SEEK_SET);
			
		if(parseBinSectionConfig(HS_TYPE,fh,pMib)!=0)
		{
			close(fh);
			return -1;
		}
	}
//printf("parse infile OK!%d\n",__LINE__);

	// Read default setting
	if ( (type & TYPE_MASK) & DS_TYPE ) {
		if ((type & MODE_MASK) == PC_MODE)
			lseek(fh, DEFAULT_SETTING_OFFSET, SEEK_SET);
			
		if(parseBinSectionConfig(DS_TYPE,fh,pMib)!=0)
		{
			close(fh);
			return -1;
		}
	}

	// Read current setting
	if ( (type & TYPE_MASK) & CS_TYPE ) {
		if ((type & MODE_MASK) == PC_MODE)
			lseek(fh, CURRENT_SETTING_OFFSET, SEEK_SET);
			
		if(parseBinSectionConfig(CS_TYPE,fh,pMib)!=0)
		{
			close(fh);
			return -1;
		}	
	}

	close(fh);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
static int parseTxtConfig(const char *filename, struct all_config *pMib)
{
	char line[700], value[700];
	FILE *fp;
	int id, def_flag, hw_tbl, wlan_inx, vwlan_inx;

	fp = fopen(filename, "r");
	if ( fp == NULL )
		return -1;

	acNum = 0;
	scheduleRuleNum = 0;
//#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_) Keith remove
	meshAclNum = 0;
//#endif Keith remove

#ifdef HOME_GATEWAY
	portFilterNum = ipFilterNum = macFilterNum = portFwNum = triggerPortNum = 0;

#if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)
	qosRuleNum = 0;
#endif

#ifdef HOME_GATEWAY // defined(VLAN_CONFIG_SUPPORTED) Keith Modify
#if defined(VLAN_CONFIG_SUPPORTED)
	vlanRuleNum = 0;
#endif
#endif
#endif

#ifdef HOME_GATEWAY
#ifdef VPN_SUPPORT
	ipsecTunnelNum = 0 ;
#endif
#endif
	wdsNum = 0;

	while ( fgets(line, 700, fp) ) {

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
{
		int mode;
		char name[200];

		mode = voip_mibline_from(line, name, value);
		if (mode == VOIP_CURRENT_SETTING && name[0])
		{
			voip_mib_read(&pMib->csmib.voipCfgParam, name, value);
			continue;
		}
		else if (mode == VOIP_DEFAULT_SETTING)
		{
			voip_mib_read(&pMib->dsmib.voipCfgParam, name, value);
			continue;
		}
}
#endif

		id = getToken(line, value, &def_flag, &hw_tbl, &wlan_inx, &vwlan_inx);
		if ( id == 0 )
			continue;

		if ( set_mib(&config, id, value, def_flag, hw_tbl, wlan_inx, vwlan_inx) < 0) {
			printf("Parse MIB [%d] error!\n", id );
			fclose(fp);
			return -1;
		}
	}

	fclose(fp);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
static int getToken(char *line, char *value, int *def_flag, int *hw_tbl, int *wlan_inx, int *vwlan_inx)
{
	char *ptr=line, *p1;
	char token[300]={0};
	int len=0, idx;
	mib_table_entry_T *pTbl=mib_table;

	*def_flag = 0;
	*hw_tbl = 0;
	*wlan_inx = -1;
	*vwlan_inx = 0;

	if ( *ptr == ';' )	// comments
		return 0;

	// get token
	while (*ptr && *ptr!=EOL && *ptr!=';') {
		if ( *ptr == '=' ) {
			memcpy(token, line, len);

			// delete ending space
			for (idx=len-1; idx>=0; idx--) {
				if (token[idx]!= SPACE && token[idx]!= TAB)
					break;
			}
			token[idx+1] = '\0';
			ptr++;
			break;
		}
		ptr++;
		len++;
	}
	if ( !token[0] )
	{
		return 0;
	}

	// get value
	len=0;
	while (*ptr == SPACE ) ptr++; // delete space

	p1 = ptr;
	while ( *ptr && *ptr!=EOL && *ptr!=';' && *ptr!='\r') {
		ptr++;
		len++;
	}
	memcpy(value, p1, len );
	value[len] = '\0';

	// delete ending space
	for (idx=len-1; idx>=0; idx--) {
		if (value[idx]!= SPACE && value[idx]!= TAB)
			break;
	}
	value[idx+1] = '\0';

	// delete brace " "
	if (strlen(value)>0 && value[0]=='"' && value[strlen(value)-1]=='"') {
		char tmp1[100];		
		memcpy(tmp1, &value[1], strlen(value)-2);		
		tmp1[strlen(value)-2] = '\0';
		strcpy(value, tmp1);	
	}
	
	p1 = token;
	if ( !memcmp(token, "DEF_", 4)) {
		p1 = &token[4];
		if (!memcmp(p1, "WLAN", 4) && p1[5]=='_') {
			*wlan_inx = p1[4] - '0';
			if (*wlan_inx >=  NUM_WLAN_INTERFACE) {
				printf("Invalid wlan interface number!\n");
				return 0;
			}
#ifdef MBSSID
			if (!memcmp(&p1[6], "VAP", 3) && p1[10]=='_') {
				*vwlan_inx = p1[9] - '0';
				if (*vwlan_inx >=  NUM_VWLAN_INTERFACE) {
					printf("Invalid virtual wlan interface number!\n");
					return 0;
				}
				p1 += 11;
				*vwlan_inx += 1;
			}
#ifdef UNIVERSAL_REPEATER
			else if (!memcmp(&p1[6], "VXD", 3) && p1[9]=='_') {
				*vwlan_inx = NUM_VWLAN_INTERFACE;				
				p1 += 10;
			}
#endif		
			else
#endif
			p1 += 6;
			pTbl = mib_wlan_table;
		}
		else
			pTbl = mib_table;
		*def_flag = 1;
	}
	else if ( !memcmp(token, "HW_", 3)) {
		p1 = &token[3];
		if (!memcmp(p1, "WLAN", 4) && p1[5]=='_') {
			*wlan_inx = p1[4] - '0';
			if (*wlan_inx >=  NUM_WLAN_INTERFACE) {
				printf("Invalid wlan interface number!\n");
				return 0;
			}
			p1 += 6;
			pTbl = hwmib_wlan_table;
		}
		else
			pTbl = hwmib_table;
		*hw_tbl = 1;
	}
	else { // current setting
		if (!memcmp(p1, "WLAN", 4) && p1[5]=='_') {
			*wlan_inx = p1[4] - '0';
			if (*wlan_inx >=  NUM_WLAN_INTERFACE) {
				printf("Invalid wlan interface number!\n");
				return 0;
			}
#ifdef MBSSID
			if (!memcmp(&p1[6], "VAP", 3) && p1[10]=='_') {
				*vwlan_inx = p1[9] - '0';
				if (*vwlan_inx >=  NUM_VWLAN_INTERFACE) {
					printf("Invalid virtual wlan interface number!\n");
					return 0;
				}
				p1 += 11;
				*vwlan_inx += 1;
			}

#ifdef UNIVERSAL_REPEATER
			else if (!memcmp(&p1[6], "VXD", 3) && p1[9] == '_') {
				*vwlan_inx = NUM_VWLAN_INTERFACE;
				p1 += 10;
			}
#endif				
			else
#endif
			p1 += 6;
			pTbl = mib_wlan_table;
		}
	}

	idx = 0;
	while (pTbl[idx].id) {
		if (!strcmp(pTbl[idx].name, p1))
			return pTbl[idx].id;
		idx++;
	}
//printf("not found[%s]\n", p1); // for debug

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
static int set_mib(struct all_config *pConfig, int id, void *value, int def_mib, int hw_tbl, int idx, int v_idx)
{
	unsigned char key[180];
	char *p1, *p2, *p3, *p4,*p5;
#ifdef HOME_GATEWAY
	char   *p6, *p7,*p8;
#if defined(GW_QOS_ENGINE) || defined(VPN_SUPPORT) || defined(CONFIG_IPV6)
	char  *p9, *p10, *p11, *p12;
#endif

#if defined(VPN_SUPPORT) || defined(CONFIG_IPV6)
	char *p13, *p14, *p15, *p16, *p17, *p18, *p19, *p20, *p21, *p22, *p23, *p24;
#endif
#ifdef CONFIG_IPV6
	char *p25,*p26,*p27,*p28,*p29,*p30,*p31,*p32,*p33,*p34;
#endif
#endif
	struct in_addr inAddr;
	int i, j, k, int_val, max_chan;
	MACFILTER_Tp pWlAc;
	SCHEDULE_Tp pscheduleRule;
	WDS_Tp pWds;

#ifdef HOME_GATEWAY
	PORTFW_Tp pPortFw;
	PORTFILTER_Tp pPortFilter;
	IPFILTER_Tp pIpFilter;
	MACFILTER_Tp pMacFilter;
	TRIGGERPORT_Tp pTriggerPort;

#ifdef HOME_GATEWAY //VLAN_CONFIG_SUPPORTED Keith Modify
#if defined(VLAN_CONFIG_SUPPORTED)
	VLAN_CONFIG_Tp pVlan;
#endif
#endif

#ifdef GW_QOS_ENGINE
	QOS_Tp pQos;
#endif

#ifdef QOS_BY_BANDWIDTH
	IPQOS_Tp pQos;
#endif

#ifdef VPN_SUPPORT
	IPSECTUNNEL_Tp pIpsecTunnel;
#endif
#ifdef CONFIG_IPV6
	char tmp[5];
	int size;
	char *pstart,*pend;
	radvdCfgParam_Tp pradvdCfgParam;
	dnsv6CfgParam_Tp pdnsv6CfgParam;
	dhcp6sCfgParam_Tp pdhcp6sCfgParam;
	daddrIPv6CfgParam_Tp daddrIPv6CfgParam;
	tunnelCfgParam_Tp tunnelCfgParam;
#endif
#endif
	mib_table_entry_T *pTbl;
	char *pMib;

	if (def_mib) {
		if (idx >= 0) {
			pMib = (char *)&pConfig->dsmib.wlan[idx][v_idx]; //for debug sean
			pTbl = mib_wlan_table;
		}
		else {
			pMib = (char *)&pConfig->dsmib;
			pTbl = mib_table;
		}
		pConfig->dsmib_exist = 1;
	}
	else if (hw_tbl) {
		if (idx >= 0) {
			pMib = (char *)&pConfig->hwmib.wlan[idx];
			pTbl = hwmib_wlan_table;
		}
		else {
			pMib = (char *)&pConfig->hwmib;
			pTbl = hwmib_table;
		}
		pConfig->hwmib_exist = 1;
	}
	else {
		if (idx >= 0) {
			pMib = (char *)&pConfig->csmib.wlan[idx][v_idx];
			pTbl = mib_wlan_table;
		}
		else {
			pMib = (char *)&pConfig->csmib;
			pTbl = mib_table;
		}
		pConfig->csmib_exist = 1;
	}

	for (i=0; pTbl[i].id; i++) {
		if ( pTbl[i].id == id )
			break;
	}
	if ( pTbl[i].id == 0 )
		return -1;

	switch (pTbl[i].type) {
	case BYTE_T:
		*((unsigned char *)(((long)pMib) + pTbl[i].offset)) = (unsigned char)atoi(value);
		break;

	case WORD_T:
		*((unsigned short *)(((long)pMib) + pTbl[i].offset)) = (unsigned short)atoi(value);
		break;

	case STRING_T:
		if ( (int)(strlen(value)+1) > pTbl[i].size )
			return 0;
		strcpy((char *)(((long)pMib) + pTbl[i].offset), (char *)value);
		break;

	case BYTE5_T:
		if (strlen(value)!=10 || !string_to_hex(value, key, 10))
			return -1;
		memcpy((unsigned char *)(((long)pMib) + pTbl[i].offset), key, 5);
		break;

	case BYTE6_T:
		if (strlen(value)!=12 || !string_to_hex(value, key, 12))
			return -1;
		memcpy((unsigned char *)(((long)pMib) + pTbl[i].offset), key, 6);
		break;

	case BYTE13_T:
		if (strlen(value)!=26 || !string_to_hex(value, key, 26))
			return -1;
		memcpy((unsigned char *)(((long)pMib) + pTbl[i].offset), key, 13);
		break;

	case DWORD_T:
	
		*((unsigned long *)(((long)pMib) + pTbl[i].offset)) = (unsigned long)atoi(value);

		break;

	case IA_T:
		if ( !inet_aton(value, &inAddr) )
			return -1;
		memcpy((unsigned char *)(((long)pMib) + pTbl[i].offset), (unsigned char *)&inAddr,  4);
		break;

	case BYTE_ARRAY_T:
#if defined(CONFIG_RTL_8196B)
		max_chan = (pTbl[i].id == MIB_HW_TX_POWER_CCK)? MAX_CCK_CHAN_NUM:MAX_OFDM_CHAN_NUM;
#else
                //rtl8196c platform
		if((id >= MIB_HW_TX_POWER_CCK_A &&  id <=MIB_HW_TX_POWER_DIFF_OFDM))
			max_chan = MAX_2G_CHANNEL_NUM_MIB;
		else if((id >= MIB_HW_TX_POWER_5G_HT40_1S_A &&  id <=MIB_HW_TX_POWER_DIFF_5G_OFDM))
			max_chan = MAX_5G_CHANNEL_NUM_MIB;
#endif

#if defined(CONFIG_RTL_8196B)
		if ( pTbl[i].id == MIB_HW_TX_POWER_CCK || pTbl[i].id == MIB_HW_TX_POWER_OFDM ) 
#else
                if((id >= MIB_HW_TX_POWER_CCK_A &&  id <=MIB_HW_TX_POWER_DIFF_OFDM) ||
		   (id >= MIB_HW_TX_POWER_5G_HT40_1S_A &&  id <=MIB_HW_TX_POWER_DIFF_5G_OFDM))
#endif
                {
			p1 = (char *)value;
			for (k=0, j=0; k<max_chan ; k++) {
				if ( !sscanf(&p1[j], "%d", &int_val) ) {
					printf("invalid value!\n");
					return -1;
				}
				key[k] = (unsigned char)int_val;
				while ( p1[++j] ) {
					if ( p1[j]==' ') {
						j++;
						break;
					}
				}
			}
			memcpy((unsigned char *)(((long)pMib) + pTbl[i].offset), (unsigned char *)key, pTbl[i].size );
		}
		break;

	case WLAC_ARRAY_T:
		getVal2((char *)value, &p1, &p2);
		if (p1 == NULL) {
			printf("Invalid WLAC in argument!\n");
			break;
		}
		if (strlen(p1)!=12 || !string_to_hex(p1, key, 12))
			return -1;

		pWlAc = (MACFILTER_Tp)(((long)pMib)+pTbl[i].offset+acNum*sizeof(MACFILTER_T));
		memcpy(pWlAc->macAddr, key, 6);
		if (p2 != NULL )
			strcpy(pWlAc->comment, p2);
		acNum++;
		break;
	case SCHEDULE_ARRAY_T:
		getVal5((char *) value,&p1,&p2,&p3,&p4,&p5);
		if (p1 == NULL) {
			printf("Invalid SCHEDULE in argument!\n");
			break;
		}

		scheduleRuleNum = (unsigned short)atoi(p5);
		scheduleRuleNum--;
		if(scheduleRuleNum>=MAX_SCHEDULE_NUM)
			break;
		
		pscheduleRule = (SCHEDULE_Tp)(((long)pMib)+pTbl[i].offset+scheduleRuleNum*sizeof(SCHEDULE_T));

		//memcpy(&pscheduleRule->eco,p1,sizeof(unsigned short));
		pscheduleRule->eco = (unsigned short)atoi(p1);
		pscheduleRule->fTime = (unsigned short)atoi(p2);
		pscheduleRule->tTime = (unsigned short)atoi(p3);
		pscheduleRule->day = (unsigned short)atoi(p4);

		break;
//#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_) // below code copy above ACL code Keith remove
	case MESH_ACL_ARRAY_T:
		getVal2((char *)value, &p1, &p2);
		if (p1 == NULL) {
			printf("Invalid Mesh ACL in argument!\n");
			break;
		}
		if (strlen(p1)!=12 || !string_to_hex(p1, key, 12))
			return -1;

		pWlAc = (MACFILTER_Tp)(((long)pMib)+pTbl[i].offset+meshAclNum*sizeof(MACFILTER_T));
		memcpy(pWlAc->macAddr, key, 6);
		if (p2 != NULL )
			strcpy(pWlAc->comment, p2);
		meshAclNum++;
		break;
//#endif Keith remove

	case WDS_ARRAY_T:
		getVal2((char *)value, &p1, &p2);
		if (p1 == NULL) {
			printf("Invalid WDS in argument!\n");
			break;
		}
		if (strlen(p1)!=12 || !string_to_hex(p1, key, 12))
			return -1;

		pWds = (WDS_Tp)(((long)pMib)+pTbl[i].offset+wdsNum*sizeof(WDS_T));
		memcpy(pWds->macAddr, key, 6);
		if (p2 != NULL )
			strcpy(pWds->comment, p2);
		wdsNum++;
		break;


#ifdef HOME_GATEWAY
	case MACFILTER_ARRAY_T:
		getVal2((char *)value, &p1, &p2);
		if (p1 == NULL) {
			printf("Invalid MACFILTER in argument!\n");
			break;
		}
		if (strlen(p1)!=12 || !string_to_hex(p1, key, 12))
			return -1;

		pMacFilter = (MACFILTER_Tp)(((long)pMib)+pTbl[i].offset+macFilterNum*sizeof(MACFILTER_T));
		memcpy(pMacFilter->macAddr, key, 6);
		if (p2 != NULL )
			strcpy(pMacFilter->comment, p2);
		macFilterNum++;
		break;

	case PORTFW_ARRAY_T:
		getVal5((char *)value, &p1, &p2, &p3, &p4, &p5);
		if (p1 == NULL || p2 == NULL || p3 == NULL || p4 == NULL ) {
			printf("Invalid PORTFW arguments!\n");
			break;
		}
		if ( !inet_aton(p1, &inAddr) )
			return -1;

		pPortFw = (PORTFW_Tp)(((long)pMib)+pTbl[i].offset+portFwNum*sizeof(PORTFW_T));
		memcpy(pPortFw->ipAddr, (unsigned char *)&inAddr, 4);
		pPortFw->fromPort = (unsigned short)atoi(p2);
		pPortFw->toPort = (unsigned short)atoi(p3);
		pPortFw->protoType = (unsigned char)atoi(p4);
		if ( p5 )
			strcpy( pPortFw->comment, p5 );
		portFwNum++;
		break;

	case IPFILTER_ARRAY_T:
		getVal3((char *)value, &p1, &p2, &p3);
		if (p1 == NULL || p2 == NULL) {
			printf("Invalid IPFILTER arguments!\n");
			break;
		}
		if ( !inet_aton(p1, &inAddr) )
			return -1;
		pIpFilter = (IPFILTER_Tp)(((long)pMib)+pTbl[i].offset+ipFilterNum*sizeof(IPFILTER_T));
		memcpy(pIpFilter->ipAddr, (unsigned char *)&inAddr, 4);
		pIpFilter->protoType = (unsigned char)atoi(p2);
		if ( p3 )
			strcpy( pIpFilter->comment, p3 );
		ipFilterNum++;
		break;

	case PORTFILTER_ARRAY_T:
		getVal4((char *)value, &p1, &p2, &p3, &p4);
		if (p1 == NULL || p2 == NULL || p3 == NULL) {
			printf("Invalid PORTFILTER arguments!\n");
			break;
		}
		pPortFilter = (PORTFILTER_Tp)(((long)pMib)+pTbl[i].offset+portFilterNum*sizeof(PORTFILTER_T));
		pPortFilter->fromPort = (unsigned short)atoi(p1);
		pPortFilter->toPort = (unsigned short)atoi(p2);
		pPortFilter->protoType = (unsigned char)atoi(p3);
		if ( p4 )
			strcpy( pPortFilter->comment, p4 );
		portFilterNum++;
		break;

	case TRIGGERPORT_ARRAY_T:
		getVal7((char *)value, &p1, &p2, &p3, &p4, &p5, &p6, &p7);
		if (p1 == NULL || p2 == NULL || p3 == NULL || p4 == NULL || p5 == NULL || p6 == NULL) {
			printf("Invalid TRIGGERPORT arguments!\n");
			break;
		}
		pTriggerPort = (TRIGGERPORT_Tp)(((long)pMib)+pTbl[i].offset+triggerPortNum*sizeof(TRIGGERPORT_T));
		pTriggerPort->tri_fromPort = (unsigned short)atoi(p1);
		pTriggerPort->tri_toPort = (unsigned short)atoi(p2);
		pTriggerPort->tri_protoType = (unsigned char)atoi(p3);
		pTriggerPort->inc_fromPort = (unsigned short)atoi(p4);
		pTriggerPort->inc_toPort = (unsigned short)atoi(p5);
		pTriggerPort->inc_protoType = (unsigned char)atoi(p6);
		if ( p7 )
			strcpy( pTriggerPort->comment, p7);
		triggerPortNum++;
		break;
#if defined(CONFIG_RTL_8198_AP_ROOT) || defined(HOME_GATEWAY) //VLAN_CONFIG_SUPPORTED Keith Modify
#if defined(VLAN_CONFIG_SUPPORTED)
	case VLANCONFIG_ARRAY_T:

#if defined(CONFIG_RTK_VLAN_NEW_FEATURE) ||defined(CONFIG_RTL_HW_VLAN_SUPPORT)
		getVal8((char *)value, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8);
		if (p1 == NULL || p2 == NULL || p3 == NULL || p4 == NULL || p5 == NULL || p6 == NULL || p7 == NULL || p8 == NULL) {
#else
		getVal7((char *)value, &p1, &p2, &p3, &p4, &p5, &p6, &p7);
		if (p1 == NULL || p2 == NULL || p3 == NULL || p4 == NULL || p5 == NULL || p6 == NULL || p7 == NULL) {
#endif
			printf("Invalid VLAN arguments!\n",vlanRuleNum);

			break;
		}

		vlanRuleNum = (unsigned short)atoi(p7);
		vlanRuleNum -- ;
		
		if(vlanRuleNum >= MAX_IFACE_VLAN_CONFIG)
			break;
						
		pVlan = (VLAN_CONFIG_Tp)(((long)pMib)+pTbl[i].offset+vlanRuleNum*sizeof(VLAN_CONFIG_T));
		if ( p1 )
			strcpy(pVlan->netIface, p1);
		pVlan->enabled = (unsigned short)atoi(p2);
		pVlan->tagged = (unsigned short)atoi(p3);
		pVlan->priority = (unsigned short)atoi(p4);
		pVlan->cfi = (unsigned short)atoi(p5);
		pVlan->vlanId = (unsigned short)atoi(p6);
#if defined(CONFIG_RTK_VLAN_NEW_FEATURE) || defined(CONFIG_RTL_HW_VLAN_SUPPORT)
		pVlan->forwarding_rule = (unsigned short)atoi(p8);
		//printf("%s:%d pVlan->netIface=%s pVlan->forwarding_rule=%d vlanRuleNum=%d\n",
		//__FUNCTION__,__LINE__,pVlan->netIface,pVlan->forwarding_rule,vlanRuleNum);
#endif
		break;
#endif
#endif

#ifdef GW_QOS_ENGINE
	case QOS_ARRAY_T:
		getVal12((char *)value, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10, &p11, &p12);
		if (p1 == NULL || p2 == NULL || p3 == NULL || p4 == NULL || p5 == NULL || p6 == NULL || p7 == NULL ||
		    p8 == NULL || p9 == NULL || p10 == NULL || p11 == NULL || p12 == NULL ) {
			printf("Invalid QoS arguments!\n");
			break;
		}
		pQos = (QOS_Tp)(((long)pMib)+pTbl[i].offset+qosRuleNum*sizeof(QOS_T));
		pQos->enabled = (unsigned char)atoi(p1);
		pQos->priority = (unsigned char)atoi(p2);
		pQos->protocol = (unsigned short)atoi(p3);
		if ( !inet_aton(p4, &inAddr) )
			return -1;
		memcpy(pQos->local_ip_start, (unsigned char *)&inAddr, 4);
		if ( !inet_aton(p5, &inAddr) )
			return -1;
		memcpy(pQos->local_ip_end, (unsigned char *)&inAddr, 4);
        
		pQos->local_port_start = (unsigned short)atoi(p6);
		pQos->local_port_end = (unsigned short)atoi(p7);
		if ( !inet_aton(p8, &inAddr) )
			return -1;
		memcpy(pQos->remote_ip_start, (unsigned char *)&inAddr, 4);
		if ( !inet_aton(p9, &inAddr) )
			return -1;
		memcpy(pQos->remote_ip_end, (unsigned char *)&inAddr, 4);

		pQos->remote_port_start = (unsigned short)atoi(p10);
		pQos->remote_port_end = (unsigned short)atoi(p11);
        	strcpy( pQos->entry_name, p12 );
		qosRuleNum++;
		break;
#endif

#ifdef QOS_BY_BANDWIDTH
	case QOS_ARRAY_T:
		getVal7((char *)value, &p1, &p2, &p3, &p4, &p5, &p6, &p7);
		if (p1 == NULL || p2 == NULL || p3 == NULL || p4 == NULL || p5 == NULL || p6 == NULL || p7 == NULL) {
			printf("Invalid QoS arguments!\n");
			break;
		}
		pQos = (IPQOS_Tp)(((long)pMib)+pTbl[i].offset+qosRuleNum*sizeof(IPQOS_T));
		pQos->enabled = (unsigned char)atoi(p1);
		if (strlen(p2)!=12 || !string_to_hex(p2, pQos->mac, 12)) 
			return -1;
		//memset(pQos->mac, 0, MAC_ADDR_LEN+1);
		//strncpy( pQos->mac, p2, MAC_ADDR_LEN);
		pQos->mode = (unsigned char)atoi(p3);
		if ( !inet_aton(p4, &inAddr) )
			return -1;
		memcpy(pQos->local_ip_start, (unsigned char *)&inAddr, 4);
		if ( !inet_aton(p5, &inAddr) )
			return -1;
		memcpy(pQos->local_ip_end, (unsigned char *)&inAddr, 4);
        
		pQos->bandwidth = (unsigned long)atoi(p6);

        	strcpy( pQos->entry_name, p7 );
		qosRuleNum++;
		break;
#endif

#ifdef VPN_SUPPORT
	case IPSECTUNNEL_ARRAY_T:
		getVal24((char *)value, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10, &p11, &p12, &p13
		, &p14, &p15, &p16, &p17, &p18, &p19, &p20, &p21 , &p22, &p23, &p24);
		if (p1 == NULL || p2 == NULL || p3 == NULL || p4 == NULL || p5 == NULL || p6 == NULL || p7 == NULL ||\
		p9 == NULL || p10 == NULL || p11 == NULL || p12 == NULL || p13 == NULL || p14 == NULL || p15 == NULL\
		|| p16 == NULL || p17 == NULL || p18 == NULL || p19 == NULL || p20 == NULL || p21 == NULL|| p22 == NULL
		|| p23 == NULL|| p24 == NULL)
		{
			printf("Invalid Ipsec Tunnel arguments!\n");
			break;
		}
		pIpsecTunnel= (IPSECTUNNEL_Tp)(((long)pMib)+pTbl[i].offset+ipsecTunnelNum*sizeof(IPSECTUNNEL_T));
		pIpsecTunnel->tunnelId= (unsigned char)atoi(p1);
		pIpsecTunnel->enable= (unsigned char)atoi(p2);
		strcpy(pIpsecTunnel->connName,p3);
		pIpsecTunnel->lcType= (unsigned char)atoi(p4);

                if ( !inet_aton(p5, &inAddr) )
		                        return -1;
		memcpy(pIpsecTunnel->lc_ipAddr , (unsigned char *)&inAddr, 4);

		pIpsecTunnel->lc_maskLen= atoi(p6);

		pIpsecTunnel->rtType= (unsigned char)atoi(p7);
		if ( !inet_aton(p8, &inAddr) )
		                        return -1;
		memcpy(pIpsecTunnel->rt_ipAddr , (unsigned char *)&inAddr, 4);

		pIpsecTunnel->rt_maskLen= atoi(p9);

		if ( !inet_aton(p10, &inAddr) )
		                        return -1;
		memcpy(pIpsecTunnel->rt_gwAddr , (unsigned char *)&inAddr, 4);
		pIpsecTunnel->keyMode= (unsigned char)atoi(p11);
		pIpsecTunnel->conType= (unsigned char)atoi(p12);
		pIpsecTunnel->espEncr= (unsigned char)atoi(p14);
		pIpsecTunnel->espAuth= (unsigned char)atoi(p15);

		if(p15)
			strcpy(pIpsecTunnel->psKey, p15);

		pIpsecTunnel->ikeEncr= (unsigned char)atoi(p16);
		pIpsecTunnel->ikeAuth= (unsigned char)atoi(p17);
		pIpsecTunnel->ikeKeyGroup= (unsigned char)atoi(p18);
		pIpsecTunnel->ikeLifeTime = strtol(p19,(char **)NULL, 10);

		pIpsecTunnel->ipsecLifeTime = strtol(p20,(char **)NULL, 10);
		pIpsecTunnel->ipsecPfs= (unsigned char)atoi(p21);

		if(p22)
			strcpy(pIpsecTunnel->spi,p22);
		if(p23)
			strcpy(pIpsecTunnel->encrKey, p23);
		if(p24)
			strcpy(pIpsecTunnel->authKey, p24);
		ipsecTunnelNum++;
		break;

#endif

#ifdef CONFIG_IPV6
	case RADVDPREFIX_T:
		getVal33((char *)value, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10, &p11, &p12, &p13\
		, &p14, &p15, &p16, &p17, &p18, &p19, &p20, &p21 , &p22, &p23, &p24,&p25, &p26, &p27, &p28, &p29,\
		&p30, &p31, &p32, &p33);
		if (p1 == NULL || p2 == NULL || p3 == NULL || p4 == NULL || p5 == NULL || p6 == NULL || p7 == NULL ||\
		p9 == NULL || p10 == NULL || p11 == NULL || p12 == NULL || p13 == NULL || p14 == NULL || p15 == NULL\
		|| p16 == NULL || p17 == NULL || p18 == NULL || p19 == NULL || p20 == NULL || p21 == NULL|| p22 == NULL\
		|| p23 == NULL|| p24 == NULL || p25 ==NULL || p26 ==NULL || p27 == NULL || p28 == NULL || p29 ==NULL\
		|| p30 ==NULL || p32 ==NULL || p33==NULL)
		{
			printf("Invalid Radvd arguments!\n");
			break;
		}
		pradvdCfgParam=(radvdCfgParam_Tp)((long)pMib+pTbl[i].offset);
		pradvdCfgParam->enabled = atoi(p1);

		strcpy(pradvdCfgParam->interface.Name, p2);
		pradvdCfgParam->interface.MaxRtrAdvInterval = atoi(p3);
		pradvdCfgParam->interface.MinRtrAdvInterval = atoi(p4);
		pradvdCfgParam->interface.MinDelayBetweenRAs = atoi(p5);
		pradvdCfgParam->interface.AdvManagedFlag =atoi(p6);
		pradvdCfgParam->interface.AdvOtherConfigFlag = atoi(p7);
		pradvdCfgParam->interface.AdvLinkMTU = atoi(p8);
		/*replace atoi by strtoul to support max value test of ipv6 phase 2 test*/
		pradvdCfgParam->interface.AdvReachableTime = strtoul(p9,NULL,10);
		pradvdCfgParam->interface.AdvRetransTimer = strtoul(p10,NULL,10);
		pradvdCfgParam->interface.AdvCurHopLimit = atoi(p11);
		pradvdCfgParam->interface.AdvDefaultLifetime = atoi(p12);
		strcpy(pradvdCfgParam->interface.AdvDefaultPreference, p13);
		pradvdCfgParam->interface.AdvSourceLLAddress = atoi(p14);
		pradvdCfgParam->interface.UnicastOnly = atoi(p15);
		/*prefix1*/
		memset(tmp,0,5);
		pstart=p16;
		pend=pstart;
		for(i=0;i<8;i++)
		{
			size=0;
			while((*pend) && (*pend !=':'))
			{
				pend++;
				size++;
			}
			if((*pend) && (*pend == ':'))
				pend++;
			memcpy(tmp,pstart,size);
			pstart=pend;
			pradvdCfgParam->interface.prefix[0].Prefix[i]=strtol(tmp,NULL,16);
		}
		pradvdCfgParam->interface.prefix[0].PrefixLen=atoi(p17);
		pradvdCfgParam->interface.prefix[0].AdvOnLinkFlag=atoi(p18);
		pradvdCfgParam->interface.prefix[0].AdvAutonomousFlag=atoi(p19);
		/*replace atoi by strtoul to support max value test of ipv6 phase 2 test*/
		pradvdCfgParam->interface.prefix[0].AdvValidLifetime=strtoul(p20,NULL,10);
		pradvdCfgParam->interface.prefix[0].AdvPreferredLifetime=strtoul(p21,NULL,10);
		pradvdCfgParam->interface.prefix[0].AdvRouterAddr=atoi(p22);
		strcpy(pradvdCfgParam->interface.prefix[0].if6to4,p23);
		pradvdCfgParam->interface.prefix[0].enabled=atoi(p24);

		/*prefix2*/
		memset(tmp,0,5);
		pstart=p25;
		pend=pstart;
		for(i=0;i<8;i++)
		{
			size=0;
			while((*pend) && (*pend !=':'))
			{
				pend++;
				size++;
			}
			if((*pend) && (*pend == ':'))
				pend++;
			memcpy(tmp,pstart,size);
			pstart=pend;
			pradvdCfgParam->interface.prefix[1].Prefix[i]=strtol(tmp,NULL,16);
		}
		pradvdCfgParam->interface.prefix[1].PrefixLen=atoi(p26);
		pradvdCfgParam->interface.prefix[1].AdvOnLinkFlag=atoi(p27);
		pradvdCfgParam->interface.prefix[1].AdvAutonomousFlag=atoi(p28);
		/*replace atoi by strtoul to support max value test of ipv6 phase 2 test*/
		pradvdCfgParam->interface.prefix[1].AdvValidLifetime=strtoul(p29,NULL,10);
		pradvdCfgParam->interface.prefix[1].AdvPreferredLifetime=strtoul(p30,NULL,10);
		pradvdCfgParam->interface.prefix[1].AdvRouterAddr=atoi(p31);
		strcpy(pradvdCfgParam->interface.prefix[1].if6to4,p32);
		pradvdCfgParam->interface.prefix[1].enabled=atoi(p33);
		break;
	case DNSV6_T:
		getVal2(value, &p1,&p2);
		if(NULL == p1 || NULL ==p2)
		{
			printf("Invalid Ipsec Tunnel arguments!\n");
			break;
		}
		pdnsv6CfgParam=(dnsv6CfgParam_Tp)(((long)pMib)+pTbl[i].offset);
		pdnsv6CfgParam->enabled=atoi(p1);
		strcpy(pdnsv6CfgParam->routerName,p2);	
		break;
	case DHCPV6S_T:
		getVal5(value, &p1,&p2,&p3,&p4,&p5);
		if(NULL == p1 || NULL ==p2 || NULL == p3 || NULL ==p4 ||NULL == p5)
		{
			printf("Invalid dhcp6s arguments!\n");
			break;
		}
		pdhcp6sCfgParam=(dhcp6sCfgParam_Tp)(((long)pMib)+pTbl[i].offset);
		
		pdhcp6sCfgParam->enabled=atoi(p1);		
		strcpy(pdhcp6sCfgParam->DNSaddr6,p2);
		strcpy(pdhcp6sCfgParam->addr6PoolS,p3);	
		strcpy(pdhcp6sCfgParam->addr6PoolE,p4);
		strcpy(pdhcp6sCfgParam->interfaceNameds,p5);
		
		break;
	case ADDR6_T:
		getVal19((char *)value,&p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10, &p11, &p12, &p13\
		, &p14, &p15, &p16, &p17, &p18, &p19);
		
		if (p1 == NULL || p2 == NULL || p3 == NULL || p4 == NULL || p5 == NULL || p6 == NULL || p7 == NULL ||\
		p9 == NULL || p10 == NULL || p11 == NULL || p12 == NULL || p13 == NULL || p14 == NULL || p15 == NULL\
		|| p16 == NULL || p17 == NULL || p18 == NULL || p19 == NULL)
		{
			printf("Invalid dhcp6s arguments!\n");
			break;
		}		
		daddrIPv6CfgParam=(daddrIPv6CfgParam_Tp)(((long)pMib)+pTbl[i].offset);
		daddrIPv6CfgParam->enabled=atoi(p1);
		daddrIPv6CfgParam->prefix_len[0]=atoi(p2);
		daddrIPv6CfgParam->prefix_len[1]=atoi(p3);

		daddrIPv6CfgParam->addrIPv6[0][0]=atoi(p4);
		daddrIPv6CfgParam->addrIPv6[0][1]=atoi(p5);
		daddrIPv6CfgParam->addrIPv6[0][2]=atoi(p6);
		daddrIPv6CfgParam->addrIPv6[0][3]=atoi(p7);
		daddrIPv6CfgParam->addrIPv6[0][4]=atoi(p8);
		daddrIPv6CfgParam->addrIPv6[0][5]=atoi(p9);
		daddrIPv6CfgParam->addrIPv6[0][6]=atoi(p10);
		daddrIPv6CfgParam->addrIPv6[0][7]=atoi(p11);

		daddrIPv6CfgParam->addrIPv6[1][0]=atoi(p12);
		daddrIPv6CfgParam->addrIPv6[1][1]=atoi(p13);
		daddrIPv6CfgParam->addrIPv6[1][2]=atoi(p14);
		daddrIPv6CfgParam->addrIPv6[1][3]=atoi(p15);
		daddrIPv6CfgParam->addrIPv6[1][4]=atoi(p16);
		daddrIPv6CfgParam->addrIPv6[1][5]=atoi(p17);
		daddrIPv6CfgParam->addrIPv6[1][6]=atoi(p18);
		daddrIPv6CfgParam->addrIPv6[1][7]=atoi(p19);
		
		break;
	case TUNNEL6_T:
		getVal(value, &p1);
		tunnelCfgParam->enabled=atoi(p1);
		break;
#endif
#endif

	default:
		return -1;
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////
static int generateBinFile(int type, const char *filename, int flag)
{	int fh;
	char *ptr, *pHsTag, *pDsTag, *pCsTag;
	PARAM_HEADER_T header;
	unsigned char checksum;
	int mib_len, section_len;

	if (type == PC_MODE) {
		char *buf;
		buf=calloc(1, 0x6000);
		if ( buf == NULL ) {
			printf("Allocate buffer failed!\n");
			return -1;
		}
		fh = open(filename, O_RDWR|O_CREAT|O_TRUNC);
		write(fh, buf, 0x6000);
		free(buf);

		pHsTag = HW_SETTING_HEADER_TAG;

		pDsTag = DEFAULT_SETTING_HEADER_TAG;
		pCsTag = CURRENT_SETTING_HEADER_TAG;
	}
	else {
#ifdef WIN32
		_chmod(filename, S_IREAD|S_IWRITE);
		fh = open(filename, O_RDWR|O_CREAT|O_TRUNC|O_BINARY);
#else
		chmod(filename, S_IREAD|S_IWRITE);
		fh = open(filename, O_RDWR|O_CREAT|O_TRUNC);
#endif
		if ((flag & 0x7f) == 0) {  // upgrade
			pHsTag = HW_SETTING_HEADER_UPGRADE_TAG;
			pDsTag = DEFAULT_SETTING_HEADER_UPGRADE_TAG;
			pCsTag = CURRENT_SETTING_HEADER_UPGRADE_TAG;
		}
		else if ((flag & 0x7f) == 1) { // update
			pHsTag = HW_SETTING_HEADER_TAG;
			pDsTag = DEFAULT_SETTING_HEADER_TAG;
			pCsTag = CURRENT_SETTING_HEADER_TAG;
		}
		else {
			pHsTag = HW_SETTING_HEADER_FORCE_TAG;
			pDsTag = DEFAULT_SETTING_HEADER_FORCE_TAG;
			pCsTag = CURRENT_SETTING_HEADER_FORCE_TAG;
		}
	}

	if ( fh == -1 )
		return -1;

	lseek(fh, 0, SEEK_SET);

	if (config.hwmib_exist && !disable_hwsetting) {
		if (config.hwmib_ver == 0) {
			sprintf(header.signature, "%s%02d", pHsTag, HW_SETTING_VER);
			header.len = sizeof(HW_SETTING_T) + sizeof(checksum);
		}
		else {
			sprintf(header.signature, "%s%02d", pHsTag, config.hwmib_ver);
			header.len = config.hwmib_len;
		}
		mib_len = header.len;
		ptr = (char *)&config.hwmib;
		checksum = CHECKSUM(ptr, header.len-1);
		ptr[header.len-1] = checksum;
		if ( type == PC_MODE )
			lseek(fh, HW_SETTING_OFFSET, SEEK_SET);
		else {
#ifdef COMPRESS_MIB_SETTING
#else
			if ( !(flag & RAW_TYPE) ) {
   				ENCODE_DATA(ptr, header.len);
			}
#endif
			header.len = WORD_SWAP(header.len);
		}

#ifdef COMPRESS_MIB_SETTING
		int comp_len=0;
#ifdef MIB_TLV
		unsigned char *pfile = NULL;
		unsigned char *mib_tlv_data = NULL;
		unsigned int tlv_content_len = 0;
		unsigned int mib_tlv_max_len = 0;

//mib_display_data_content(HW_SETTING, ptr, WORD_SWAP(header.len)-1);	

		mib_tlv_max_len = mib_get_setting_len(HW_SETTING)*4;

		pfile = malloc(mib_tlv_max_len);
		if(pfile != NULL && mib_tlv_save(HW_SETTING, (void*)ptr, pfile, &tlv_content_len) == 1)
		{
			mib_tlv_data = malloc(tlv_content_len+1); // 1:checksum
			if(mib_tlv_data != NULL)
			{
				memcpy(mib_tlv_data, pfile, tlv_content_len);
			}
				
			free(pfile);

		}
		
		if(mib_tlv_data != NULL)
		{
			header.len = tlv_content_len+1;
			ptr = mib_tlv_data;
			checksum = CHECKSUM(ptr, header.len-1);
			ptr[tlv_content_len] = CHECKSUM(ptr, tlv_content_len);
//mib_display_tlv_content(HW_SETTING, ptr, header.len);	
			header.len = WORD_SWAP(header.len);
		}

#endif //#ifdef MIB_TLV
		if(cvcfg_mib_compress_write(HW_SETTING, ptr, &header, fh, &comp_len) == 1)
		{
//			COMP_TRACE(stderr,"\r\n cvcfg_mib_compress_write HW_SETTING DONE, __[%s-%u]", __FILE__,__LINE__);			
			if ( flag & RAW_TYPE ) { // raw type, pad 0
				section_len = HW_SETTING_SECTOR_LEN - sizeof(COMPRESS_MIB_HEADER_T) - comp_len;
				while (section_len-- >0) {
					write(fh, "\x0", 1);
				}
			}
		}
		else
		{
#endif		
		write(fh, &header, sizeof(header));
		write(fh, ptr, mib_len);
		if ( flag & RAW_TYPE ) { // raw type, pad 0
			section_len = HW_SETTING_SECTOR_LEN - sizeof(header) - mib_len;
			while (section_len-- >0) {
				write(fh, "\x0", 1);
			}
		}

#ifdef COMPRESS_MIB_SETTING
		}
#endif
	}

	if (config.dsmib_exist) {
		if (config.dsmib_ver == 0) {
			sprintf(header.signature, "%s%02d", pDsTag, DEFAULT_SETTING_VER);
			header.len = sizeof(APMIB_T) + sizeof(checksum);
		}
		else {
			sprintf(header.signature, "%s%02d", pDsTag, config.dsmib_ver);
			header.len = config.dsmib_len;
		}
		mib_len = header.len;
		ptr = (char *)&config.dsmib;
		checksum = CHECKSUM(ptr, header.len-1);
		ptr[header.len-1] = checksum;
		if ( type == PC_MODE )
			lseek(fh, DEFAULT_SETTING_OFFSET, SEEK_SET);
		else {
			swap_mib_word_value(&config.dsmib);
#ifdef COMPRESS_MIB_SETTING
#else
			if ( !(flag&RAW_TYPE) ) {
				ENCODE_DATA(ptr, header.len);
			}
#endif			
			header.len = WORD_SWAP(header.len);
		}
#ifdef COMPRESS_MIB_SETTING
		int comp_len=0;

#ifdef MIB_TLV
		unsigned char *pfile = NULL;
		unsigned char *mib_tlv_data = NULL;
		unsigned int tlv_content_len = 0;
		unsigned int mib_tlv_max_len = 0;

//mib_display_data_content(DEFAULT_SETTING, ptr, WORD_SWAP(header.len)-1);

		mib_tlv_max_len = mib_get_setting_len(DEFAULT_SETTING)*4;

		tlv_content_len = 0;

		pfile = malloc(mib_tlv_max_len);
		memset(pfile, 0x00, mib_tlv_max_len);
		
		if( pfile != NULL && mib_tlv_save(DEFAULT_SETTING, (void*)ptr, pfile, &tlv_content_len) == 1)
		{

			mib_tlv_data = malloc(tlv_content_len+1); // 1:checksum
			if(mib_tlv_data != NULL)
			{
				memcpy(mib_tlv_data, pfile, tlv_content_len);
			}
				
			free(pfile);

		}
		
		if(mib_tlv_data != NULL)
		{
			
			sprintf(header.signature, "%s%02d", DEFAULT_SETTING_HEADER_TAG, DEFAULT_SETTING_VER);
			header.len = tlv_content_len+1;
			ptr = mib_tlv_data;
			checksum = CHECKSUM(ptr, header.len-1);
			ptr[tlv_content_len] = CHECKSUM(ptr, tlv_content_len);
//mib_display_tlv_content(DEFAULT_SETTING, ptr, header.len);
			header.len = WORD_SWAP(header.len); //for x86
		}

#endif //#ifdef MIB_TLV

		if(cvcfg_mib_compress_write(DEFAULT_SETTING, ptr, &header, fh, &comp_len) == 1)
		{
//			COMP_TRACE(stderr,"\r\n cvcfg_mib_compress_write DEFAULT_SETTING DONE, __[%s-%u]", __FILE__,__LINE__);			
			if ( flag & RAW_TYPE ) { // raw type, pad 0
				section_len = DEFAULT_SETTING_SECTOR_LEN - sizeof(COMPRESS_MIB_HEADER_T) - comp_len;
				while (section_len-- >0) {
					write(fh, "\x0", 1);
				}
		   }
		}		
		else
		{
#endif			
		write(fh, &header, sizeof(header));
		write(fh, ptr, mib_len);

		if ( flag & RAW_TYPE ) { // raw type, pad 0
			section_len = DEFAULT_SETTING_SECTOR_LEN - sizeof(header) - mib_len;
			while (section_len-- >0) {
				write(fh, "\x0", 1);
			}
		}
#ifdef COMPRESS_MIB_SETTING
		}
#endif		
	}
	if (config.csmib_exist) {
		if (config.csmib_ver == 0) {
			sprintf(header.signature, "%s%02d", pCsTag, CURRENT_SETTING_VER);
			header.len = sizeof(APMIB_T) + sizeof(checksum);
		}
		else {
			sprintf(header.signature, "%s%02d", pCsTag, config.csmib_ver);
			header.len = config.csmib_len;
		}
		mib_len = header.len;
		ptr = (char *)&config.csmib;
		checksum = CHECKSUM(ptr, header.len-1);
		ptr[header.len-1] = checksum;
		if ( type == PC_MODE )
			lseek(fh, CURRENT_SETTING_OFFSET, SEEK_SET);
		else {
			swap_mib_word_value(&config.csmib);
#ifdef COMPRESS_MIB_SETTING
#else
			if (!(flag&RAW_TYPE)) {
				ENCODE_DATA(ptr, header.len);
			}
#endif
			header.len = WORD_SWAP(header.len);
		}
#ifdef COMPRESS_MIB_SETTING
		int comp_len=0;
#ifdef MIB_TLV
		unsigned char *pfile = NULL;
		unsigned char *mib_tlv_data = NULL;
		unsigned int tlv_content_len = 0;
		unsigned int mib_tlv_max_len = 0;

//mib_display_data_content(CURRENT_SETTING, ptr, WORD_SWAP(header.len)-1);

		mib_tlv_max_len = mib_get_setting_len(CURRENT_SETTING)*4;

		tlv_content_len = 0;

		pfile = malloc(mib_tlv_max_len);
		memset(pfile, 0x00, mib_tlv_max_len);
		
		if( pfile != NULL && mib_tlv_save(CURRENT_SETTING, (void*)ptr, pfile, &tlv_content_len) == 1)
		{

			mib_tlv_data = malloc(tlv_content_len+1); // 1:checksum
			if(mib_tlv_data != NULL)
			{
				memcpy(mib_tlv_data, pfile, tlv_content_len);
			}
				
			free(pfile);

		}
		
		if(mib_tlv_data != NULL)
		{
			
			sprintf(header.signature, "%s%02d", CURRENT_SETTING_HEADER_TAG, CURRENT_SETTING_VER);
			header.len = tlv_content_len+1;
			ptr = mib_tlv_data;
			checksum = CHECKSUM(ptr, header.len-1);
			ptr[tlv_content_len] = CHECKSUM(ptr, tlv_content_len);
//mib_display_tlv_content(CURRENT_SETTING, ptr, header.len);
			header.len = WORD_SWAP(header.len); //for x86

			#ifdef DEBUG_CVCFG
			flash_write_file(mib_tlv_data,tlv_content_len,"out_tlv.raw");
			#endif
		}

#endif //#ifdef MIB_TLV
		if(cvcfg_mib_compress_write(CURRENT_SETTING, ptr, &header, fh, &comp_len) == 1)
		{
		//	COMP_TRACE(stderr,"\r\n cvcfg_mib_compress_write CURRENT_SETTING DONE, __[%s-%u]", __FILE__,__LINE__);			
			if( flag & RAW_TYPE ) { // raw type, pad 0
				section_len = CURRENT_SETTING_SECTOR_LEN - sizeof(COMPRESS_MIB_HEADER_T) - comp_len;
				while (section_len-- >0) {
					write(fh, "\x0", 1);
				}
		  	}
		}		
		else
		{
#endif			
		write(fh, &header, sizeof(header));
		write(fh, ptr, mib_len);

		if ( flag & RAW_TYPE ) { // raw type, pad 0
			section_len = CURRENT_SETTING_SECTOR_LEN - sizeof(header) - mib_len;
			while (section_len-- >0) {
				write(fh, "\x0", 1);
			}
		}
#ifdef COMPRESS_MIB_SETTING
		}
#endif			
	}

	close(fh);

#ifdef WIN32
	_chmod(filename, S_IREAD);
#else
	chmod(filename,  DEFFILEMODE);
#endif

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
static int generateTxtFile(const char *filename)
{
	unsigned char *p;
	FILE *fp;
	char buf[500], line[300], prefix[20];
	int i, j;
	MACFILTER_Tp pWlAc;
	WDS_Tp pWds;
	SCHEDULE_Tp pscheduleRule;
	int max_chan_num;
#if defined(HOME_GATEWAY) || defined(CONFIG_RTL_8198_AP_ROOT)
#if defined(VLAN_CONFIG_SUPPORTED)
	VLAN_CONFIG_Tp pVlanConfig;
#endif
#endif

#ifdef HOME_GATEWAY
	PORTFW_Tp pPortFw;
	PORTFILTER_Tp pPortFilter;
	IPFILTER_Tp pIpFilter;
	MACFILTER_Tp pMacFilter;
	TRIGGERPORT_Tp pTriggerPort;

#ifdef GW_QOS_ENGINE
	QOS_Tp pQos;
	char LipS[20], LipE[20], RipS[20], RipE[20];
#endif

#ifdef QOS_BY_BANDWIDTH
	IPQOS_Tp pQos;
	char LipS[20], LipE[20];
#endif

#ifdef VPN_SUPPORT
	IPSECTUNNEL_Tp pIpsecTunnel;
	char strLcIp[20], strRtIp[20], strRtGw[20];
#endif
#endif
	int idx=0, wlan_inx=0, vwlan_inx=0;
	mib_table_entry_T *pTbl;
	char *pMib;
	APMIB_Tp pApMib;

	fp = fopen(filename, "w");
	if ( fp == NULL)
	{
		printf("can't open file %s###<%s>,%d\n",filename,__FUNCTION__,__LINE__);
		return -1;
	}
	//printf("csmib_exist = %d,csmib_len = %d,%s\n",config.csmib_exist,config.csmib_len,config.tmp1);
next_config:
	wlan_inx = 0;
	if ( ++idx>6) {

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
		fflush(fp);
		if (config.csmib_exist) {
			#ifndef VOIP_SUPPORT_TLV_CFG
			voip_mibtbl_swap_value(&config.csmib.voipCfgParam);
			#endif
			voip_mibtbl_write(&config.csmib.voipCfgParam, fileno(fp), VOIP_CURRENT_SETTING);
		}
		if (config.dsmib_exist) {
			#ifndef VOIP_SUPPORT_TLV_CFG
			voip_mibtbl_swap_value(&config.dsmib.voipCfgParam);
			#endif
			voip_mibtbl_write(&config.dsmib.voipCfgParam, fileno(fp), VOIP_DEFAULT_SETTING);
		}
#endif

		fclose(fp);
		return 0;
	}
next_wlan:
	if (idx == 1) {
		if ( !config.hwmib_exist )
			goto next_config;
		pTbl = hwmib_table;
		pMib = (char *)&config.hwmib;
		strcpy(prefix, "HW_");
	}
	else if (idx == 2) {
		if ( !config.hwmib_exist )
			goto next_config;
		pTbl = hwmib_wlan_table;
		pMib = (char *)&config.hwmib.wlan[wlan_inx];
		sprintf(prefix, "HW_WLAN%d_", wlan_inx);
	}
	else if (idx == 3) {
		if ( !config.dsmib_exist )
			goto next_config;
		pTbl = mib_table;
		pMib = (char *)&config.dsmib;
		pApMib = &config.dsmib;
		strcpy(prefix, "DEF_");
	}
	else if (idx == 4) {
		if ( !config.dsmib_exist )
			goto next_config;
		pTbl = mib_wlan_table;
		//printf("###&config.dsmib=0x%x,%d\n",&config.dsmib,__LINE__);
		//printf("###&config.dsmib.wlan=0x%x,%d\n",&config.dsmib.wlan,__LINE__);
		//printf("###wlan_inx=%d,vwlan_inx=%d,%d\n",wlan_inx,vwlan_inx,__LINE__);
		//printf("###&config.dsmib.wlan[][]=0x%x,%d\n",&config.dsmib.wlan[wlan_inx][vwlan_inx],__LINE__);
		
#ifdef MBSSID
		if (vwlan_inx > 0)
			sprintf(prefix, "DEF_WLAN%d_VAP%d_", wlan_inx, vwlan_inx-1);
		else
#endif
		{
			vwlan_inx=0;
			sprintf(prefix, "DEF_WLAN%d_", wlan_inx);
		}
		pMib = (char *)&config.dsmib.wlan[wlan_inx][vwlan_inx];
		pApMib = &config.dsmib;
	}
	else if (idx == 5) {
		if ( !config.csmib_exist )
			goto next_config;
		pTbl = mib_table;
		pMib = (char *)&config.csmib;
		pApMib = &config.csmib;
		prefix[0] = '\0';
	}
	else if (idx == 6) {
		if ( !config.csmib_exist )
			goto next_config;
		pTbl = mib_wlan_table;
		pMib = (char *)&config.csmib.wlan[wlan_inx][vwlan_inx];
		pApMib = &config.csmib;
#ifdef MBSSID
		if (vwlan_inx > 0)
			sprintf(prefix, "WLAN%d_VAP%d_", wlan_inx, vwlan_inx-1);
		else
#endif
		sprintf(prefix, "WLAN%d_", wlan_inx);
	}

	for (i=0; pTbl[i].id; i++) {
		switch (pTbl[i].type) {
		case BYTE_T:
#ifdef MBSSID
			if (pTbl[i].id == MIB_WLAN_WSC_DISABLE && vwlan_inx > 0) {
				WRITE_LINE("%s%s=%d\n", prefix, pTbl[i].name, 1);
			}
			else
#endif
			WRITE_LINE("%s%s=%d\n", prefix, pTbl[i].name,
				*((unsigned char *)(((long)pMib) + pTbl[i].offset)));
			break;

		case WORD_T:
			WRITE_LINE("%s%s=%d\n", prefix, pTbl[i].name,
				*((unsigned short *)(((long)pMib) + pTbl[i].offset)));
			break;

		case STRING_T:
			//printf("###idx=%d,type=%d,%d\n",idx,pTbl[i].type,__LINE__);
			//printf("###pTbl[i].name=%s%s\n",prefix,pTbl[i].name);
			//printf("###pMib=0x%x config.ds=0x%x\n",pMib,&config.dsmib);
			//printf("###%d %d i=%d\n",pTbl[i].offset,pTbl[i].size,i);
			WRITE_LINE("%s%s=\"%s\"\n", prefix, pTbl[i].name,
				(char *)(((long)pMib) + pTbl[i].offset));
			break;

		case BYTE5_T:
			p = (unsigned char *)(((long)pMib) + pTbl[i].offset);
			sprintf(buf, "%02x%02x%02x%02x%02x", p[0], p[1], p[2], p[3], p[4]);
			WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			break;

		case BYTE6_T:
			p = (unsigned char *)(((long)pMib) + pTbl[i].offset);
			sprintf(buf, "%02x%02x%02x%02x%02x%02x", p[0], p[1], p[2], p[3], p[4], p[5]);
			WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			break;

		case BYTE13_T:
			p = (unsigned char *)(((long)pMib) + pTbl[i].offset);
			sprintf(buf, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
				p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12]);
			WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			break;

		case DWORD_T:
		
			WRITE_LINE("%s%s=%d\n", prefix, pTbl[i].name,
				*((unsigned int *)(((long)pMib) + pTbl[i].offset)));
			break;

		case IA_T:
			WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name,
				inet_ntoa(*((struct in_addr *)(((long)pMib) + pTbl[i].offset))));
			break;

		case BYTE_ARRAY_T:
#if defined(CONFIG_RTL_8196B)
			if ( pTbl[i].id == MIB_HW_TX_POWER_CCK || pTbl[i].id == MIB_HW_TX_POWER_OFDM ) {
				max_chan_num = (pTbl[i].id == MIB_HW_TX_POWER_CCK)? MAX_CCK_CHAN_NUM: MAX_OFDM_CHAN_NUM;
				buf[0] = '\0';
				p = (unsigned char *)(((long)pMib) + pTbl[i].offset);
				for (j=0; j<max_chan_num; j++) {
					sprintf(line, "%d", *(p+ j) );
					if ( j+1 != max_chan_num)
						strcat(line, " ");
					strcat(buf, line);
				}
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}else
			{
				printf("%s %d\n",pTbl[i].name,__LINE__);
			}
			break;
#else	

			if(pTbl[i].id>=MIB_HW_TX_POWER_CCK_A&&pTbl[i].id<=MIB_HW_TX_POWER_DIFF_OFDM)
			{
				max_chan_num = MAX_CCK_CHAN_NUM;
				buf[0] = '\0';
				p = (unsigned char *)(((long)pMib) + pTbl[i].offset);
				for (j=0; j<max_chan_num; j++) {
					sprintf(line, "%d", *(p+ j) );
					if ( j+1 != max_chan_num)
						strcat(line, " ");
					strcat(buf, line);
				}
				
				//printf("%s%s=%s ;%d\n",prefix, pTbl[i].name, buf,__LINE__);
				
				WRITE_LINE("%s%s=%s ;\n", prefix, pTbl[i].name, buf);
			}
			break;

#endif

			


		case WLAC_ARRAY_T:
			for (j=0; j<pApMib->wlan[wlan_inx][vwlan_inx].acNum; j++) {
				pWlAc = (MACFILTER_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(MACFILTER_T));
				sprintf(buf, "%02x%02x%02x%02x%02x%02x", pWlAc->macAddr[0], pWlAc->macAddr[1],
					pWlAc->macAddr[2], pWlAc->macAddr[3], pWlAc->macAddr[4], pWlAc->macAddr[5]);
				if (strlen(pWlAc->comment)) {
					strcat(buf, ", ");
					strcat(buf, pWlAc->comment);
				}
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;
		case SCHEDULE_ARRAY_T:
			for (j=0; j<pApMib->wlan[wlan_inx][vwlan_inx].scheduleRuleNum; j++) {
				pscheduleRule = (SCHEDULE_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(SCHEDULE_T));
				sprintf(buf, "%d,%d,%d,%d,%d",pscheduleRule->eco,pscheduleRule->fTime,
											pscheduleRule->tTime,pscheduleRule->day,j+1);
				
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;
//#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_) // below code copy above ACL code Keith remove
		case MESH_ACL_ARRAY_T:
			for (j=0; j<pApMib->meshAclNum; j++) {
				pWlAc = (MACFILTER_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(MACFILTER_T));
				sprintf(buf, "%02x%02x%02x%02x%02x%02x", pWlAc->macAddr[0], pWlAc->macAddr[1],
					pWlAc->macAddr[2], pWlAc->macAddr[3], pWlAc->macAddr[4], pWlAc->macAddr[5]);
				if (strlen(pWlAc->comment)) {
					strcat(buf, ", ");
					strcat(buf, pWlAc->comment);
				}
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;
//#endif Keith remove

		case WDS_ARRAY_T:
			for (j=0; j<pApMib->wlan[wlan_inx][vwlan_inx].wdsNum; j++) {
				pWds = (WDS_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(WDS_T));
				sprintf(buf, "%02x%02x%02x%02x%02x%02x", pWds->macAddr[0], pWds->macAddr[1],
					pWds->macAddr[2], pWds->macAddr[3], pWds->macAddr[4], pWds->macAddr[5]);
				if (strlen(pWds->comment)) {
					strcat(buf, ", ");
					strcat(buf, pWds->comment);
				}
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;

#ifdef HOME_GATEWAY
		case PORTFW_ARRAY_T:
			for (j=0; j<pApMib->portFwNum; j++) {
				pPortFw = (PORTFW_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(PORTFW_T));
				sprintf(buf, "%s, %d, %d, %d", inet_ntoa(*((struct in_addr *)pPortFw->ipAddr)),
					pPortFw->fromPort, pPortFw->toPort, pPortFw->protoType);
				if (strlen(pPortFw->comment)) {
					strcat(buf, ", ");
					strcat(buf, pPortFw->comment);
				}
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;

		case PORTFILTER_ARRAY_T:
			for (j=0; j<pApMib->portFilterNum; j++) {
				pPortFilter = (PORTFILTER_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(PORTFILTER_T));
				sprintf(buf, "%d, %d, %d", pPortFilter->fromPort,
					pPortFilter->toPort, pPortFilter->protoType);
				if (strlen(pPortFilter->comment)) {
					strcat(buf, ", ");
					strcat(buf, pPortFilter->comment);
				}
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;

		case IPFILTER_ARRAY_T:
			for (j=0; j<pApMib->ipFilterNum; j++) {
				pIpFilter = (IPFILTER_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(IPFILTER_T));
				sprintf(buf, "%s, %d", inet_ntoa(*((struct in_addr *)pIpFilter->ipAddr)), pIpFilter->protoType);
				if (strlen(pIpFilter->comment)) {
					strcat(buf, ", ");
					strcat(buf, pIpFilter->comment);
				}
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;

		case MACFILTER_ARRAY_T:
			for (j=0; j<pApMib->macFilterNum; j++) {
				pMacFilter = (MACFILTER_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(MACFILTER_T));
				sprintf(buf, "%02x%02x%02x%02x%02x%02x", pMacFilter->macAddr[0], pMacFilter->macAddr[1],
					pMacFilter->macAddr[2], pMacFilter->macAddr[3], pMacFilter->macAddr[4], pMacFilter->macAddr[5]);
				if (strlen(pMacFilter->comment)) {
					strcat(buf, ", ");
					strcat(buf, pMacFilter->comment);
				}
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;

		case TRIGGERPORT_ARRAY_T:
			for (j=0; j<pApMib->triggerPortNum; j++) {
				pTriggerPort = (TRIGGERPORT_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(TRIGGERPORT_T));
				sprintf(buf, "%d, %d, %d, %d, %d, %d",
					pTriggerPort->tri_fromPort,pTriggerPort->tri_toPort, pTriggerPort->tri_protoType,
					pTriggerPort->inc_fromPort,pTriggerPort->inc_toPort, pTriggerPort->inc_protoType);
				if (strlen(pTriggerPort->comment)) {
					strcat(buf, ", ");
					strcat(buf, pTriggerPort->comment);
				}
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;
#if defined(CONFIG_RTL_8198_AP_ROOT) || defined(HOME_GATEWAY) //VLAN_CONFIG_SUPPORTED Keith Modify
#if defined(VLAN_CONFIG_SUPPORTED)
		case VLANCONFIG_ARRAY_T:
//			printf("%s:%d pApMib->VlanConfigNum=%d\n",__FUNCTION__,__LINE__,pApMib->VlanConfigNum);
			for(j=0;j<pApMib->VlanConfigNum;j++)
			{
				pVlanConfig = (VLAN_CONFIG_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(VLAN_CONFIG_T));
				sprintf(buf, "%s,%d,%d,%d,%d,%d,%d",pVlanConfig->netIface,pVlanConfig->enabled,
						pVlanConfig->tagged,pVlanConfig->priority,pVlanConfig->cfi,
						pVlanConfig->vlanId,j+1);
#if defined(CONFIG_RTK_VLAN_NEW_FEATURE) || defined(CONFIG_RTL_HW_VLAN_SUPPORT)
				{
					char tmpBuffer[32]={0};
					sprintf(tmpBuffer,",%d ;",pVlanConfig->forwarding_rule);
					//fprintf(stderr,"###netInterface=%s tmpBuffer=%s %d\n",pVlanConfig->netIface,tmpBuffer,__LINE__);
					strcat(buf,tmpBuffer);
				}
#endif
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			
			break;
#endif
#endif

#ifdef GW_QOS_ENGINE
		case QOS_ARRAY_T:
			for (j=0; j<pApMib->qosRuleNum; j++) {
				pQos = (QOS_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(QOS_T));
                        strcpy(LipS, inet_ntoa(*((struct in_addr*)pQos->local_ip_start)));
                        strcpy(LipE, inet_ntoa(*((struct in_addr*)pQos->local_ip_end)));
                        strcpy(RipS, inet_ntoa(*((struct in_addr*)pQos->remote_ip_start)));
                        strcpy(RipE, inet_ntoa(*((struct in_addr*)pQos->remote_ip_end)));
				sprintf(buf, QOS_FORMAT, pQos->enabled,
                                pQos->priority, pQos->protocol, 
                                LipS, LipE, 
                                pQos->local_port_start, pQos->local_port_end, 
                                RipS, RipE,
					pQos->remote_port_start, pQos->remote_port_end, pQos->entry_name );
				
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;
#endif

#ifdef QOS_BY_BANDWIDTH
		case QOS_ARRAY_T:
			for (j=0; j<pApMib->qosRuleNum; j++) {
				pQos = (IPQOS_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(IPQOS_T));
                        strcpy(LipS, inet_ntoa(*((struct in_addr*)pQos->local_ip_start)));
                        strcpy(LipE, inet_ntoa(*((struct in_addr*)pQos->local_ip_end)));
				sprintf(buf, QOS_FORMAT, pQos->enabled,
                                pQos->mac[0],pQos->mac[1],pQos->mac[2],pQos->mac[3],pQos->mac[4],pQos->mac[5],
                                pQos->mode, LipS, LipE, 
                                pQos->bandwidth, 
					pQos->entry_name );
				
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;
#endif

#ifdef VPN_SUPPORT
		case IPSECTUNNEL_ARRAY_T:

			for (j=0; j<pApMib->ipsecTunnelNum; j++) {
				pIpsecTunnel= (IPSECTUNNEL_Tp)(((long)pMib) + pTbl[i].offset + j*sizeof(IPSECTUNNEL_T));
				strcpy(strLcIp, inet_ntoa(*((struct in_addr*) pIpsecTunnel->lc_ipAddr)));
				strcpy(strRtIp, inet_ntoa(*((struct in_addr*) pIpsecTunnel->rt_ipAddr)));
				strcpy(strRtGw, inet_ntoa(*((struct in_addr*) pIpsecTunnel->rt_gwAddr)));


				sprintf(buf, IPSECTUNNEL_FORMAT, pIpsecTunnel->tunnelId, pIpsecTunnel->enable,
				pIpsecTunnel->connName, pIpsecTunnel->lcType, strLcIp, pIpsecTunnel->lc_maskLen,
				pIpsecTunnel->rtType, strRtIp, pIpsecTunnel->rt_maskLen,
				strRtGw, pIpsecTunnel->keyMode,
				pIpsecTunnel->conType, pIpsecTunnel->espEncr, pIpsecTunnel->espAuth,
				pIpsecTunnel->psKey, pIpsecTunnel->ikeEncr, pIpsecTunnel->ikeAuth, pIpsecTunnel->ikeKeyGroup,
				pIpsecTunnel->ikeLifeTime, pIpsecTunnel->ipsecLifeTime, pIpsecTunnel->ipsecPfs,
				pIpsecTunnel->spi, pIpsecTunnel->encrKey, pIpsecTunnel->authKey
				);

				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
			}
			break;
#endif

#ifdef CONFIG_IPV6
	case RADVDPREFIX_T:
		{
			radvdCfgParam_Tp pEntry=(radvdCfgParam_Tp)(((long)pMib) + pTbl[i].offset);
			sprintf(buf,RADVD_FORMAT,
				/*enabled*/
				pEntry->enabled,
				/*interface*/
				pEntry->interface.Name,pEntry->interface.MaxRtrAdvInterval,pEntry->interface.MinRtrAdvInterval,
				pEntry->interface.MinDelayBetweenRAs,pEntry->interface.AdvManagedFlag,pEntry->interface.AdvOtherConfigFlag,pEntry->interface.AdvLinkMTU,
				pEntry->interface.AdvReachableTime,pEntry->interface.AdvRetransTimer,pEntry->interface.AdvCurHopLimit,pEntry->interface.AdvDefaultLifetime,
				pEntry->interface.AdvDefaultPreference,pEntry->interface.AdvSourceLLAddress,pEntry->interface.UnicastOnly,
				/*prefix 1*/
				pEntry->interface.prefix[0].Prefix[0],pEntry->interface.prefix[0].Prefix[1],pEntry->interface.prefix[0].Prefix[2],pEntry->interface.prefix[0].Prefix[3],
				pEntry->interface.prefix[0].Prefix[4],pEntry->interface.prefix[0].Prefix[5],pEntry->interface.prefix[0].Prefix[6],pEntry->interface.prefix[0].Prefix[7],
				pEntry->interface.prefix[0].PrefixLen,pEntry->interface.prefix[0].AdvOnLinkFlag,pEntry->interface.prefix[0].AdvAutonomousFlag,pEntry->interface.prefix[0].AdvValidLifetime,
				pEntry->interface.prefix[0].AdvPreferredLifetime,pEntry->interface.prefix[0].AdvRouterAddr,pEntry->interface.prefix[0].if6to4,pEntry->interface.prefix[0].enabled,
				/*prefix 2*/
				pEntry->interface.prefix[1].Prefix[0],pEntry->interface.prefix[1].Prefix[1],pEntry->interface.prefix[1].Prefix[2],pEntry->interface.prefix[1].Prefix[3],
				pEntry->interface.prefix[1].Prefix[4],pEntry->interface.prefix[1].Prefix[5],pEntry->interface.prefix[1].Prefix[6],pEntry->interface.prefix[1].Prefix[7],
				pEntry->interface.prefix[1].PrefixLen,pEntry->interface.prefix[1].AdvOnLinkFlag,pEntry->interface.prefix[1].AdvAutonomousFlag,pEntry->interface.prefix[1].AdvValidLifetime,
				pEntry->interface.prefix[1].AdvPreferredLifetime,pEntry->interface.prefix[1].AdvRouterAddr,pEntry->interface.prefix[1].if6to4,pEntry->interface.prefix[1].enabled);
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
		}
		break;
	case DNSV6_T:
		{
				dnsv6CfgParam_Tp pEntry=(dnsv6CfgParam_Tp)(((long)pMib) + pTbl[i].offset);
				sprintf(buf,DNSV6_FORMAT,pEntry->enabled,pEntry->routerName);
				WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
		}
	case DHCPV6S_T:
		{
			dhcp6sCfgParam_Tp pEntry=(dhcp6sCfgParam_Tp)(((long)pMib) + pTbl[i].offset);
			sprintf(buf,DHCPV6S_FORMAT,pEntry->enabled,pEntry->DNSaddr6,pEntry->addr6PoolS,pEntry->addr6PoolE,pEntry->interfaceNameds);
			WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
		}
	case ADDR6_T:
		{
			daddrIPv6CfgParam_Tp pEntry=(daddrIPv6CfgParam_Tp)(((long)pMib) + pTbl[i].offset);
			sprintf(buf,ADDR6_FORMAT,pEntry->enabled,pEntry->prefix_len[0],pEntry->prefix_len[1],
				pEntry->addrIPv6[0][0],pEntry->addrIPv6[0][1],pEntry->addrIPv6[0][2],pEntry->addrIPv6[0][3],
				pEntry->addrIPv6[0][4],pEntry->addrIPv6[0][5],pEntry->addrIPv6[0][6],pEntry->addrIPv6[0][7],
				pEntry->addrIPv6[1][0],pEntry->addrIPv6[1][1],pEntry->addrIPv6[1][2],pEntry->addrIPv6[1][3],
				pEntry->addrIPv6[1][4],pEntry->addrIPv6[1][5],pEntry->addrIPv6[1][6],pEntry->addrIPv6[1][7]
				);
			WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
		}
	case TUNNEL6_T:
		{
			tunnelCfgParam_Tp pEntry=(tunnelCfgParam_Tp)(((long)pMib) + pTbl[i].offset);
			sprintf(buf,"%d",pEntry->enabled);
			WRITE_LINE("%s%s=%s\n", prefix, pTbl[i].name, buf);
		}
		break;
#endif
		case TABLE_LIST_T:
		case DHCPRSVDIP_ARRY_T:
		case URLFILTER_ARRAY_T:
		case STATICROUTE_ARRAY_T:
			break;
#endif

#ifdef VOIP_SUPPORT 
		case VOIP_T:
			break;
#endif
		default:
			printf("set mib with invalid type[%d, %s]!\n", pTbl[i].type, pTbl[i].name);
			break;
		}
	}

	if (idx == 2 || idx == 4 || idx == 6) {
#ifdef MBSSID
		if (idx == 4 || idx == 6) {
			if (++vwlan_inx <= NUM_VWLAN_INTERFACE)
				goto next_wlan;
			else
				vwlan_inx = 0;
		}
#endif
		if (++wlan_inx < NUM_WLAN_INTERFACE)
			goto next_wlan;
	}

	goto next_config;
}

#if 0
////////////////////////////////////////////////////////////////////////////////
static void swap_mib_word_value(APMIB_Tp pMib)
{
	int i, j,k;
//for debug sean
	for (i=0; i<NUM_WLAN_INTERFACE; i++) 
		for (j=0; j<(NUM_VWLAN_INTERFACE+1); j++) 
	{
		pMib->wlan[i][j].fragThreshold = WORD_SWAP(pMib->wlan[i][j].fragThreshold);
		pMib->wlan[i][j].rtsThreshold = WORD_SWAP(pMib->wlan[i][j].rtsThreshold);
		pMib->wlan[i][j].supportedRates = WORD_SWAP(pMib->wlan[i][j].supportedRates);
		pMib->wlan[i][j].basicRates = WORD_SWAP(pMib->wlan[i][j].basicRates);
		pMib->wlan[i][j].beaconInterval = WORD_SWAP(pMib->wlan[i][j].beaconInterval);
		pMib->wlan[i][j].inactivityTime = DWORD_SWAP(pMib->wlan[i][j].inactivityTime);
		pMib->wlan[i][j].wpaGroupRekeyTime = DWORD_SWAP(pMib->wlan[i][j].wpaGroupRekeyTime);
		pMib->wlan[i][j].rsPort = WORD_SWAP(pMib->wlan[i][j].rsPort);
		pMib->wlan[i][j].rsIntervalTime = WORD_SWAP(pMib->wlan[i][j].rsIntervalTime);
		pMib->wlan[i][j].fixedTxRate = WORD_SWAP(pMib->wlan[i][j].fixedTxRate);
//add by sen_liu 2011.6.17
		pMib->wlan[i][j].accountRsUpdateDelay = WORD_SWAP(pMib->wlan[i][j].accountRsUpdateDelay);
		pMib->wlan[i][j].accountRsIntervalTime = WORD_SWAP(pMib->wlan[i][j].accountRsIntervalTime);
	
		pMib->wlan[i][j].p2p_type = DWORD_SWAP(pMib->wlan[i][j].p2p_type);
		for(k=0;k<MAX_SCHEDULE_NUM;k++)
		{
			pMib->wlan[i][j].scheduleRuleArray[k].eco = WORD_SWAP(pMib->wlan[i][j].scheduleRuleArray[k].eco);
			pMib->wlan[i][j].scheduleRuleArray[k].fTime = WORD_SWAP(pMib->wlan[i][j].scheduleRuleArray[k].fTime);
			pMib->wlan[i][j].scheduleRuleArray[k].tTime = WORD_SWAP(pMib->wlan[i][j].scheduleRuleArray[k].tTime);
			pMib->wlan[i][j].scheduleRuleArray[k].day = WORD_SWAP(pMib->wlan[i][j].scheduleRuleArray[k].day);
		}
		for(k=0;k<MAX_WDS_NUM;k++)
		{
			pMib->wlan[i][j].wdsArray[k].fixedTxRate = DWORD_SWAP(pMib->wlan[i][j].wdsArray[k].fixedTxRate);
		}
#if CONFIG_RTL_WAPI_SUPPORT		
		pMib->wlan[i][j].wapiMcastRekeyTime = DWORD_SWAP(pMib->wlan[i][j].wapiMcastRekeyTime);
		pMib->wlan[i][j].wapiMcastRekeyPackets = DWORD_SWAP(pMib->wlan[i][j].wapiMcastRekeyPackets);
		pMib->wlan[i][j].wapiUcastRekeyTime = DWORD_SWAP(pMib->wlan[i][j].wapiUcastRekeyTime);
		pMib->wlan[i][j].wapiUcastRekeyPackets = DWORD_SWAP(pMib->wlan[i][j].wapiUcastRekeyPackets);
#endif		
	}

//#ifdef	CONFIG_RTK_MESH Keith remove
	pMib->meshMaxNumOfNeighbors =WORD_SWAP(pMib->meshMaxNumOfNeighbors) ;
//	pMib->meshDefaultTTL    = WORD_SWAP(pMib->meshDefaultTTL) ;
//	pMib->meshHelloInterval = WORD_SWAP(pMib->meshHelloInterval) ;
//#endif Keith remove
#ifdef HOME_GATEWAY// defined(VLAN_CONFIG_SUPPORTED) Keith Modify
#if defined(VLAN_CONFIG_SUPPORTED)
	for (i=0; i<pMib->VlanConfigNum; i++) {
		pMib->VlanConfigArray[i].vlanId =WORD_SWAP(pMib->VlanConfigArray[i].vlanId); 
	}
#endif	
#endif
#ifdef HOME_GATEWAY
	pMib->pppIdleTime = WORD_SWAP(pMib->pppIdleTime);
	pMib->pptpMtuSize = WORD_SWAP(pMib->pptpMtuSize);
	for (i=0; i<pMib->portFwNum; i++) {
		pMib->portFwArray[i].fromPort = WORD_SWAP(pMib->portFwArray[i].fromPort);
		pMib->portFwArray[i].toPort = WORD_SWAP(pMib->portFwArray[i].toPort);
	}
	for (i=0; i<pMib->portFilterNum; i++) {
		pMib->portFilterArray[i].fromPort = WORD_SWAP(pMib->portFilterArray[i].fromPort);
		pMib->portFilterArray[i].toPort = WORD_SWAP(pMib->portFilterArray[i].toPort);
	}
	for (i=0; i<pMib->triggerPortNum; i++) {
		pMib->triggerPortArray[i].tri_fromPort = WORD_SWAP(pMib->triggerPortArray[i].tri_fromPort);
		pMib->triggerPortArray[i].tri_toPort = WORD_SWAP(pMib->triggerPortArray[i].tri_toPort);
		pMib->triggerPortArray[i].inc_fromPort = WORD_SWAP(pMib->triggerPortArray[i].inc_fromPort);
		pMib->triggerPortArray[i].inc_toPort = WORD_SWAP(pMib->triggerPortArray[i].inc_toPort);
	}
	pMib->pppMtuSize = WORD_SWAP(pMib->pppMtuSize);
	pMib->fixedIpMtuSize = WORD_SWAP(pMib->fixedIpMtuSize);
	pMib->dhcpMtuSize = WORD_SWAP(pMib->dhcpMtuSize);
	pMib->l2tpMtuSize = WORD_SWAP(pMib->l2tpMtuSize); /* keith: add l2tp support. 20080519 */
	pMib->l2tpIdleTime = WORD_SWAP(pMib->l2tpIdleTime); /* keith: add l2tp support. 20080519 */
	
	pMib->pptpIdleTime = WORD_SWAP(pMib->pptpIdleTime); /* brad: add pptp support. 20081125 */
	pMib->dhcpLeaseTime = DWORD_SWAP(pMib->dhcpLeaseTime); /*hf: refine for dhcp lease time*/
#ifdef GW_QOS_ENGINE
	pMib->qosManualUplinkSpeed = DWORD_SWAP(pMib->qosManualUplinkSpeed);	
	pMib->qosManualDownLinkSpeed = DWORD_SWAP(pMib->qosManualDownLinkSpeed);	

	for (i=0; i<pMib->qosRuleNum; i++) {
		pMib->qosRuleArray[i].protocol = WORD_SWAP(pMib->qosRuleArray[i].protocol);
		pMib->qosRuleArray[i].local_port_start = WORD_SWAP(pMib->qosRuleArray[i].local_port_start);
		pMib->qosRuleArray[i].local_port_end = WORD_SWAP(pMib->qosRuleArray[i].local_port_end);
		pMib->qosRuleArray[i].remote_port_start = WORD_SWAP(pMib->qosRuleArray[i].remote_port_start);
		pMib->qosRuleArray[i].remote_port_end = WORD_SWAP(pMib->qosRuleArray[i].remote_port_end);
	}
#endif

#ifdef QOS_BY_BANDWIDTH
	pMib->qosManualUplinkSpeed = DWORD_SWAP(pMib->qosManualUplinkSpeed);	
	pMib->qosManualDownLinkSpeed = DWORD_SWAP(pMib->qosManualDownLinkSpeed);	

	for (i=0; i<pMib->qosRuleNum; i++) 
	{
		pMib->qosRuleArray[i].bandwidth = DWORD_SWAP(pMib->qosRuleArray[i].bandwidth);
		pMib->qosRuleArray[i].bandwidth_downlink = DWORD_SWAP(pMib->qosRuleArray[i].bandwidth_downlink);
	}
#endif
	
#ifdef  VPN_SUPPORT
        for (i=0; i<pMib->ipsecTunnelNum; i++) {
                pMib->ipsecTunnelArray[i].ipsecLifeTime = WORD_SWAP(pMib->ipsecTunnelArray[i].ipsecLifeTime);
                pMib->ipsecTunnelArray[i].ikeLifeTime = WORD_SWAP(pMib->ipsecTunnelArray[i].ikeLifeTime);
        }
#endif
#ifdef DOS_SUPPORT
	pMib->dosEnabled = DWORD_SWAP(pMib->dosEnabled);	
	pMib->syssynFlood = WORD_SWAP(pMib->syssynFlood);	
	pMib->sysfinFlood = WORD_SWAP(pMib->sysfinFlood);	
	pMib->sysudpFlood = WORD_SWAP(pMib->sysudpFlood);	
	pMib->sysicmpFlood = WORD_SWAP(pMib->sysicmpFlood);	
	pMib->pipsynFlood = WORD_SWAP(pMib->pipsynFlood);	
	pMib->pipfinFlood = WORD_SWAP(pMib->pipfinFlood);	
	pMib->pipudpFlood = WORD_SWAP(pMib->pipudpFlood);	
	pMib->pipicmpFlood = WORD_SWAP(pMib->pipicmpFlood);	
	pMib->blockTime = WORD_SWAP(pMib->blockTime);
#endif
#ifdef CONFIG_IPV6
	pMib->radvdCfgParam.interface.MaxRtrAdvInterval=DWORD_SWAP(pMib->radvdCfgParam.interface.MaxRtrAdvInterval);
	pMib->radvdCfgParam.interface.MinRtrAdvInterval=DWORD_SWAP(pMib->radvdCfgParam.interface.MinRtrAdvInterval);
 	pMib->radvdCfgParam.interface.MinDelayBetweenRAs=DWORD_SWAP(pMib->radvdCfgParam.interface.MinDelayBetweenRAs);
 	pMib->radvdCfgParam.interface.AdvLinkMTU=DWORD_SWAP(pMib->radvdCfgParam.interface.AdvLinkMTU);
 	pMib->radvdCfgParam.interface.AdvReachableTime=DWORD_SWAP(pMib->radvdCfgParam.interface.AdvReachableTime);
 	pMib->radvdCfgParam.interface.AdvRetransTimer=DWORD_SWAP(pMib->radvdCfgParam.interface.AdvRetransTimer);
 	pMib->radvdCfgParam.interface.AdvDefaultLifetime=WORD_SWAP(pMib->radvdCfgParam.interface.AdvDefaultLifetime);
	for(j=0;j<2;j++)
	{
		for(i=0;i<8;i++)
		{
 			pMib->radvdCfgParam.interface.prefix[j].Prefix[i]=WORD_SWAP(pMib->radvdCfgParam.interface.prefix[j].Prefix[i]);
		}
		pMib->radvdCfgParam.interface.prefix[j].AdvValidLifetime=DWORD_SWAP(pMib->radvdCfgParam.interface.prefix[j].AdvValidLifetime);
		pMib->radvdCfgParam.interface.prefix[j].AdvPreferredLifetime=DWORD_SWAP(pMib->radvdCfgParam.interface.prefix[j].AdvPreferredLifetime);
	}
#endif

//added by sen_liu 2011.6.17

	pMib->pppSessionNum = WORD_SWAP(pMib->pppSessionNum);	
	pMib->l2tpPayloadLength = WORD_SWAP(pMib->l2tpPayloadLength);	
	pMib->l2tpNs = WORD_SWAP(pMib->l2tpNs);	

#endif

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
#ifndef VOIP_SUPPORT_TLV_CFG
	voip_mibtbl_swap_value(&pMib->voipCfgParam);
#endif
#endif

//added by sen_liu 2011.6.17
#ifdef CONFIG_RTL_BT_CLIENT
	pMib->uLimit = DWORD_SWAP(pMib->uLimit);
	pMib->dLimit = DWORD_SWAP(pMib->dLimit);
#endif

#ifdef CONFIG_CWMP_TR069	
	pMib->cwmp_InformInterval = DWORD_SWAP(pMib->cwmp_InformInterval);
	pMib->cwmp_InformTime = DWORD_SWAP(pMib->cwmp_InformTime);
	pMib->cwmp_DL_StartTime = WORD_SWAP(pMib->cwmp_DL_StartTime);	
	pMib->cwmp_DL_CompleteTime = WORD_SWAP(pMib->cwmp_DL_CompleteTime);	
	pMib->cwmp_DL_FaultCode = WORD_SWAP(pMib->cwmp_DL_FaultCode);	
	pMib->cwmp_Inform_EventCode = WORD_SWAP(pMib->cwmp_Inform_EventCode);	
	pMib->cwmp_ConnReqPort = DWORD_SWAP(pMib->cwmp_ConnReqPort);
#ifdef	_PRMT_TR143_
	pMib->tr143_udpecho_port = WORD_SWAP(pMib->tr143_udpecho_port);
#endif
#endif
	pMib->system_time_year = WORD_SWAP(pMib->system_time_year);

#ifdef 	_11s_TEST_MODE_	
	pMib->meshTestParam1 = WORD_SWAP(pMib->meshTestParam1);
	pMib->meshTestParam2 = WORD_SWAP(pMib->meshTestParam2);	
	pMib->meshTestParam3 = WORD_SWAP(pMib->meshTestParam3);
	pMib->meshTestParam4 = WORD_SWAP(pMib->meshTestParam4);
	pMib->meshTestParam5 = WORD_SWAP(pMib->meshTestParam5);
	pMib->meshTestParam6 = WORD_SWAP(pMib->meshTestParam6);
	pMib->meshTestParam7 = WORD_SWAP(pMib->meshTestParam7);
	pMib->meshTestParam8 = WORD_SWAP(pMib->meshTestParam8);
	pMib->meshTestParam9 = WORD_SWAP(pMib->meshTestParam9);
	pMib->meshTestParama = WORD_SWAP(pMib->meshTestParama);
	pMib->meshTestParamb = WORD_SWAP(pMib->meshTestParamb);
	pMib->meshTestParamc = WORD_SWAP(pMib->meshTestParamc);
	pMib->meshTestParamd = WORD_SWAP(pMib->meshTestParamd);
	pMib->meshTestParame = WORD_SWAP(pMib->meshTestParame);
	pMib->meshTestParamf = WORD_SWAP(pMib->meshTestParamf);
#endif
}
#else
static int _mib_swap_value(const mib_table_entry_T *mib, void *data)
{
	short *pShort;
	int *pInt;
	switch (mib->type)
	{
	case WORD_T:
		pShort = (short *) data;
		*pShort = WORD_SWAP(*pShort);
		
		// = htons(*pShort);
		break;
	case DWORD_T:
		pInt = (int *) data;
		*pInt = DWORD_SWAP(*pInt);		
		break;
	default:
		break;
	}

	return 0;
}

static int _mibtbl_swap_value(const mib_table_entry_T *mib_tbl, void *data, int offset)
{
	int i, j;
	const mib_table_entry_T *mib;
	int new_offset;

	for (i=0; mib_tbl[i].id; i++)
	{
		mib = &mib_tbl[i];
		new_offset = offset + mib->offset;
		for (j=0; j<(mib->total_size / mib->unit_size); j++)
		{
			if (mib->type >= TABLE_LIST_T)
			{
				if (_mibtbl_swap_value(mib->next_mib_table, data, new_offset) != 0)
				{
					fprintf(stderr, "MIB (%s, %d, %d) Error: swap failed\n",
						mib_tbl[i].name, mib_tbl[i].total_size, mib_tbl[i].unit_size);
					return -1;
				}
			}
			else
			{
				_mib_swap_value(mib, (void *)((int) data + new_offset));
			}
			new_offset += mib->unit_size;
		}
	}

	return 0;
}

static void swap_mib_word_value(APMIB_Tp pMib)
{
	mib_table_entry_T *pmib_tl;

	pmib_tl = mib_get_table(CURRENT_SETTING);
	_mibtbl_swap_value(pmib_tl, pMib, 0);
#ifdef VOIP_SUPPORT
	voip_mibtbl_swap_value(&pMib->voipCfgParam);
#endif
}

#endif
////////////////////////////////////////////////////////////////////////////////
static void getVal2(char *value, char **p1, char **p2)
{
	value = getVal(value, p1);
	if ( value )
		getVal(value, p2);
	else
		*p2 = NULL;
}
////////////////////////////////////////////////////////////////////////////////
static void getVal5(char *value, char **p1, char **p2, char **p3, char **p4, char **p5)
{
	*p1 = *p2 = *p3 = *p4 = *p5 = NULL;

	value = getVal(value, p1);
	if ( !value )
		return;
	value = getVal(value, p2);
	if ( !value )
		return;
	value = getVal(value, p3);
	if ( !value )
		return;
	value = getVal(value, p4);
	if ( !value )
		return;
	getVal(value, p5);
}


#ifdef HOME_GATEWAY
////////////////////////////////////////////////////////////////////////////////
static void getVal3(char *value, char **p1, char **p2, char **p3)
{
	*p1 = *p2 = *p3 = NULL;

	value = getVal(value, p1);
	if ( !value )
		return;
	value = getVal(value, p2);
	if ( !value )
		return;
	getVal(value, p3);
}

////////////////////////////////////////////////////////////////////////////////
static void getVal4(char *value, char **p1, char **p2, char **p3, char **p4)
{
	*p1 = *p2 = *p3 = *p4 = NULL;

	value = getVal(value, p1);
	if ( !value )
		return;
	value = getVal(value, p2);
	if ( !value )
		return;
	value = getVal(value, p3);
	if ( !value )
		return;
	getVal(value, p4);
}



////////////////////////////////////////////////////////////////////////////////
#ifdef HOME_GATEWAY //defined(VLAN_CONFIG_SUPPORTED) Keith Modify
#if defined(VLAN_CONFIG_SUPPORTED)
void getVal6(char *value, char **p1, char **p2, char **p3, char **p4, char **p5, char **p6)
{
	*p1 = *p2 = *p3 = *p4 = *p5 = *p6 =NULL;

	value = getVal(value, p1);
	if ( !value )
		return;
	value = getVal(value, p2);
	if ( !value )
		return;
	value = getVal(value, p3);
	if ( !value )
		return;
	value = getVal(value, p4);
	if ( !value )
		return;
	value = getVal(value, p5);
	if ( !value )
		return;
	value = getVal(value, p6);
	
}
#endif
#endif
////////////////////////////////////////////////////////////////////////////////
static void getVal7(char *value, char **p1, char **p2, char **p3, char **p4, \
	char **p5, char **p6, char **p7)
{
	*p1 = *p2 = *p3 = *p4 = *p5 = *p6 = *p7 = NULL;

	value = getVal(value, p1);
	if ( !value )
		return;
	value = getVal(value, p2);
	if ( !value )
		return;
	value = getVal(value, p3);
	if ( !value )
		return;
	value = getVal(value, p4);
	if ( !value )
		return;
	value = getVal(value, p5);
	if ( !value )
		return;
	value = getVal(value, p6);
	if ( !value )
		return;
	value = getVal(value, p7);
}
#if defined(CONFIG_RTK_VLAN_NEW_FEATURE) || defined(CONFIG_RTL_HW_VLAN_SUPPORT)
void getVal8(char *value, char **p1, char **p2, char **p3, char **p4, \
	char **p5, char **p6, char **p7, char **p8)
{
	*p1 = *p2 = *p3 = *p4 = *p5 = *p6 = *p7 = *p8 = NULL;
	value = getVal(value, p1);
	if ( !value )
		return;
	value = getVal(value, p2);
	if ( !value )
		return;
	value = getVal(value, p3);
	if ( !value )
		return;
	value = getVal(value, p4);
	if ( !value )
		return;
	value = getVal(value, p5);
	if ( !value )
		return;
	value = getVal(value, p6);
	if ( !value )
		return;
	value = getVal(value, p7);
	if ( !value )
		return;
	value = getVal(value, p8);
}
#endif
#ifdef GW_QOS_ENGINE
////////////////////////////////////////////////////////////////////////////////
void getVal12(char *value, char **p1, char **p2, char **p3, char **p4, char **p5, 
	char **p6, char **p7, char **p8, char **p9, char **p10, char **p11, char **p12)
{
	*p1 = *p2 = *p3 = *p4 = *p5 = *p6 = *p7 = NULL;
    *p8 = *p9 = *p10 = *p11 = *p12 = NULL;
	value = getVal(value, p1);
	if ( !value )
		return;
	value = getVal(value, p2);
	if ( !value )
		return;
	value = getVal(value, p3);
	if ( !value )
		return;
	value = getVal(value, p4);
	if ( !value )
		return;
	value = getVal(value, p5);
	if ( !value )
		return;
	value = getVal(value, p6);
	if ( !value )
		return;
	value = getVal(value, p7);
	if ( !value )
		return;
	value = getVal(value, p8);
	if ( !value )
		return;
	value = getVal(value, p9);
	if ( !value )
		return;
	value = getVal(value, p10);
	if ( !value )
		return;
	value = getVal(value, p11);
	if ( !value )
		return;
	value = getVal(value, p12);
}
#endif

#ifdef VPN_SUPPORT
////////////////////////////////////////////////////////////////////////////////
static void getVal24(char *value, char **p1, char **p2, char **p3, char **p4, char **p5, char **p6, char **p7,\
	char **p8, char **p9, char **p10, char **p11, char **p12, char **p13, char **p14, char **p15, char **p16,\
	char **p17, char **p18, char **p19, char **p20, char **p21, char **p22, char **p23, char **p24)
{
	*p1 = *p2 = *p3 = *p4 = *p5 = *p6 = *p7 = *p8 = *p9 = *p10 = *p11 = *p12\
	= *p13 = *p14 = *p15 = *p16 = *p17 = *p18 = *p19 = *p20 = *p21 = *p22 = *p23 = *p24  = NULL;

	value = getVal(value, p1);
	if ( !value )
		return;
	value = getVal(value, p2);
	if ( !value )
		return;
	value = getVal(value, p3);
	if ( !value )
		return;
	value = getVal(value, p4);
	if ( !value )
		return;
	value = getVal(value, p5);
	if ( !value )
		return;
	value = getVal(value, p6);
	if ( !value )
		return;
	value = getVal(value, p7);
	if ( !value )
		return;
	value = getVal(value, p8);
	if ( !value )
		return;
	value = getVal(value, p9);
	if ( !value )
		return;
	value = getVal(value, p10);
	if ( !value )
		return;
	value = getVal(value, p11);
	if ( !value )
		return;
	value = getVal(value, p12);
	if ( !value )
		return;
	value = getVal(value, p13);
	if ( !value )
		return;
	value = getVal(value, p14);
	if ( !value )
		return;
	value = getVal(value, p15);
	if ( !value )
		return;
	value = getVal(value, p16);
	if ( !value )
		return;
	value = getVal(value, p17);
	if ( !value )
		return;
	value = getVal(value, p18);
	if ( !value )
		return;
	value = getVal(value, p19);
	if ( !value )
		return;
	value = getVal(value, p20);
	if ( !value )
		return;
	value = getVal(value, p21);
	if ( !value )
		return;
	value = getVal(value, p22);
	if ( !value )
		return;
	value = getVal(value, p23);
	if ( !value )
		return;
	value = getVal(value, p24);
	if ( !value )
		return;
}
#endif
#ifdef CONFIG_IPV6
static void getVal33(char *value, char **p1, char **p2, char **p3, char **p4, char **p5, char **p6, char **p7,\
	char **p8, char **p9, char **p10, char **p11, char **p12, char **p13, char **p14, char **p15, char **p16,\
	char **p17, char **p18, char **p19, char **p20, char **p21, char **p22, char **p23, char **p24, char **p25,\
	char **p26, char **p27, char **p28, char **p29, char **p30, char **p31, char **p32, char **p33)
{
	*p1 = *p2 = *p3 = *p4 = *p5 = *p6 = *p7 = *p8 = *p9 = *p10 = *p11 = *p12\
	= *p13 = *p14 = *p15 = *p16 = *p17 = *p18 = *p19 = *p20 = *p21 = *p22 = *p23 = *p24  = *p25\
	=*p26=*p27=*p28=*p29=*p30=*p31=*p32=*p33=NULL;

	value = getVal(value, p1);
	if ( !value )
		return;
	value = getVal(value, p2);
	if ( !value )
		return;
	value = getVal(value, p3);
	if ( !value )
		return;
	value = getVal(value, p4);
	if ( !value )
		return;
	value = getVal(value, p5);
	if ( !value )
		return;
	value = getVal(value, p6);
	if ( !value )
		return;
	value = getVal(value, p7);
	if ( !value )
		return;
	value = getVal(value, p8);
	if ( !value )
		return;
	value = getVal(value, p9);
	if ( !value )
		return;
	value = getVal(value, p10);
	if ( !value )
		return;
	value = getVal(value, p11);
	if ( !value )
		return;
	value = getVal(value, p12);
	if ( !value )
		return;
	value = getVal(value, p13);
	if ( !value )
		return;
	value = getVal(value, p14);
	if ( !value )
		return;
	value = getVal(value, p15);
	if ( !value )
		return;
	value = getVal(value, p16);
	if ( !value )
		return;
	value = getVal(value, p17);
	if ( !value )
		return;
	value = getVal(value, p18);
	if ( !value )
		return;
	value = getVal(value, p19);
	if ( !value )
		return;
	value = getVal(value, p20);
	if ( !value )
		return;
	value = getVal(value, p21);
	if ( !value )
		return;
	value = getVal(value, p22);
	if ( !value )
		return;
	value = getVal(value, p23);
	if ( !value )
		return;
	value = getVal(value, p24);
	if ( !value )
		return;
	value = getVal(value, p25);
	if ( !value )
		return;
	value = getVal(value, p26);
	if ( !value )
		return;
	value = getVal(value, p27);
	if ( !value )
		return;
	value = getVal(value, p28);
	if ( !value )
		return;
	value = getVal(value, p29);
	if ( !value )
		return;
	value = getVal(value, p30);
	if ( !value )
		return;
	value = getVal(value, p31);
	if ( !value )
		return;
	value = getVal(value, p32);
	if ( !value )
		return;
	value = getVal(value, p33);
	if ( !value )
		return;
}

static void getVal19(char *value, char **p1, char **p2, char **p3, char **p4, char **p5, char **p6, char **p7,\
	char **p8, char **p9, char **p10, char **p11, char **p12, char **p13, char **p14, char **p15, char **p16,\
	char **p17, char **p18, char **p19)
{
	*p1 = *p2 = *p3 = *p4 = *p5 = *p6 = *p7 = *p8 = *p9 = *p10 = *p11 = *p12\
	= *p13 = *p14 = *p15 = *p16 = *p17 = *p18 = *p19 =NULL;

	value = getVal(value, p1);
	if ( !value )
		return;
	value = getVal(value, p2);
	if ( !value )
		return;
	value = getVal(value, p3);
	if ( !value )
		return;
	value = getVal(value, p4);
	if ( !value )
		return;
	value = getVal(value, p5);
	if ( !value )
		return;
	value = getVal(value, p6);
	if ( !value )
		return;
	value = getVal(value, p7);
	if ( !value )
		return;
	value = getVal(value, p8);
	if ( !value )
		return;
	value = getVal(value, p9);
	if ( !value )
		return;
	value = getVal(value, p10);
	if ( !value )
		return;
	value = getVal(value, p11);
	if ( !value )
		return;
	value = getVal(value, p12);
	if ( !value )
		return;
	value = getVal(value, p13);
	if ( !value )
		return;
	value = getVal(value, p14);
	if ( !value )
		return;
	value = getVal(value, p15);
	if ( !value )
		return;
	value = getVal(value, p16);
	if ( !value )
		return;
	value = getVal(value, p17);
	if ( !value )
		return;
	value = getVal(value, p18);
	if ( !value )
		return;
	value = getVal(value, p19);
}
#endif
#endif

#ifdef WIN32
/* 
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 */
int inet_aton(cp, addr)
	register const char *cp;
	struct in_addr *addr;
{
	register in_addr_t val;
	register int base, n;
	register char c;
	unsigned int parts[4];
	register unsigned int *pp = parts;

	c = *cp;
	for (;;) {
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, isdigit=decimal.
		 */
		if (!isdigit(c))
			return (0);
		val = 0; base = 10;
		if (c == '0') {
			c = *++cp;
			if (c == 'x' || c == 'X')
				base = 16, c = *++cp;
			else
				base = 8;
		}
		for (;;) {
			if (isascii(c) && isdigit(c)) {
				val = (val * base) + (c - '0');
				c = *++cp;
			} else if (base == 16 && isascii(c) && isxdigit(c)) {
				val = (val << 4) |
					(c + 10 - (islower(c) ? 'a' : 'A'));
				c = *++cp;
			} else
				break;
		}
		if (c == '.') {
			/*
			 * Internet format:
			 *	a.b.c.d
			 *	a.b.c	(with c treated as 16 bits)
			 *	a.b	(with b treated as 24 bits)
			 */
			if (pp >= parts + 3)
				return (0);
			*pp++ = val;
			c = *++cp;
		} else
			break;
	}
	/*
	 * Check for trailing characters.
	 */
	if (c != '\0' && (!isascii(c) || !isspace(c)))
		return (0);
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = pp - parts + 1;
	switch (n) {

	case 0:
		return (0);		/* initial nondigit */

	case 1:				/* a -- 32 bits */

// david ----------
		val <<=24;
//-----------------
		
		break;

	case 2:				/* a.b -- 8.24 bits */

// david ----------
		val <<=16;
//-----------------
		if ((val > 0xffffff) || (parts[0] > 0xff))
			return (0);
		val |= parts[0] << 24;
		break;

	case 3:				/* a.b.c -- 8.8.16 bits */
// david ----------
		val <<=8;
//-----------------

		if ((val > 0xffff) || (parts[0] > 0xff) || (parts[1] > 0xff))
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16);
		break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
		if ((val > 0xff) || (parts[0] > 0xff) || (parts[1] > 0xff) || (parts[2] > 0xff))
			return (0);
		val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
		break;
	}
	if (addr)
		addr->s_addr = htonl(val);
	return (1);
}
#endif

#ifdef COMPRESS_MIB_SETTING

#define N		 4096	/* size of ring buffer */
#define F		   18	/* upper limit for match_length */
#define THRESHOLD	2   /* encode string into position and length if match_length is greater than this */
static unsigned char *text_buf;	/* ring buffer of size N, with extra F-1 bytes to facilitate string comparison */
#define LZSS_TYPE	unsigned short
#define NIL			N	/* index for root of binary search trees */
struct lzss_buffer {
	unsigned char	text_buf[N + F - 1];
	LZSS_TYPE	lson[N + 1];
	LZSS_TYPE	rson[N + 257];
	LZSS_TYPE	dad[N + 1];
};
static LZSS_TYPE		match_position, match_length;  /* of longest match.  These are set by the InsertNode() procedure. */
static LZSS_TYPE		*lson, *rson, *dad;  /* left & right children & parents -- These constitute binary search trees. */

void InsertNode(LZSS_TYPE r)
	/* Inserts string of length F, text_buf[r..r+F-1], into one of the
	   trees (text_buf[r]'th tree) and returns the longest-match position
	   and length via the global variables match_position and match_length.
	   If match_length = F, then removes the old node in favor of the new
	   one, because the old one will be deleted sooner.
	   Note r plays double role, as tree node and position in buffer. */
{
	LZSS_TYPE  i, p, cmp;
	unsigned char  *key;

	cmp = 1;
	key = &text_buf[r];
	p = N + 1 + key[0];
	rson[r] = lson[r] = NIL;
	match_length = 0;
	while(1) {
		if (cmp >= 0) {
			if (rson[p] != NIL)
				p = rson[p];
			else {
				rson[p] = r;
				dad[r] = p;
				return;
			}
		} else {
			if (lson[p] != NIL)
				p = lson[p];
			else {
				lson[p] = r;
				dad[r] = p;
				return;
			}
		}
		for (i = 1; i < F; i++)
			if ((cmp = key[i] - text_buf[p + i]) != 0)
				break;
		if (i > match_length) {
			match_position = p;
			if ((match_length = i) >= F)
				break;
		}
	}
	dad[r] = dad[p];
	lson[r] = lson[p];
	rson[r] = rson[p];
	dad[lson[p]] = r;
	dad[rson[p]] = r;
	if (rson[dad[p]] == p)
		rson[dad[p]] = r;
	else
		lson[dad[p]] = r;
	dad[p] = NIL;  /* remove p */
}

void InitTree(void)  /* initialize trees */
{
	int  i;

	/* For i = 0 to N - 1, rson[i] and lson[i] will be the right and
	   left children of node i.  These nodes need not be initialized.
	   Also, dad[i] is the parent of node i.  These are initialized to
	   NIL (= N), which stands for 'not used.'
	   For i = 0 to 255, rson[N + i + 1] is the root of the tree
	   for strings that begin with character i.  These are initialized
	   to NIL.  Note there are 256 trees. */

	for (i = N + 1; i <= N + 256; i++)
		rson[i] = NIL;
	for (i = 0; i < N; i++)
		dad[i] = NIL;
}

void DeleteNode(LZSS_TYPE p)  /* deletes node p from tree */
{
	LZSS_TYPE  q;
	
	if (dad[p] == NIL)
		return;  /* not in tree */
	if (rson[p] == NIL)
		q = lson[p];
	else if (lson[p] == NIL)
		q = rson[p];
	else {
		q = lson[p];
		if (rson[q] != NIL) {
			do {
				q = rson[q];
			} while (rson[q] != NIL);
			rson[dad[q]] = lson[q];
			dad[lson[q]] = dad[q];
			lson[q] = lson[p];
			dad[lson[p]] = q;
		}
		rson[q] = rson[p];
		dad[rson[p]] = q;
	}
	dad[q] = dad[p];
	if (rson[dad[p]] == p)
		rson[dad[p]] = q;
	else
		lson[dad[p]] = q;
	dad[p] = NIL;
}
int Encode(unsigned char *ucInput, unsigned int inLen, unsigned char *ucOutput)
{
#if defined(CHEAT_COMPRESS_MIB_SETTING)
	memcpy(ucOutput, ucInput, inLen);
	return inLen;
#else	
	LZSS_TYPE  i, len, r, s, last_match_length, code_buf_ptr;
	unsigned char c;
	unsigned char  code_buf[17], mask;
	unsigned int ulPos=0;
	int enIdx=0;

	struct lzss_buffer *lzssbuf;

	if (0 != (lzssbuf = malloc(sizeof(struct lzss_buffer)))) {
		memset(lzssbuf, 0, sizeof(struct lzss_buffer));
		text_buf = lzssbuf->text_buf;
		rson = lzssbuf->rson;
		lson = lzssbuf->lson;
		dad = lzssbuf->dad;
	} else {
		return 0;
	}
	
	InitTree();  /* initialize trees */
	code_buf[0] = 0;  /* code_buf[1..16] saves eight units of code, and
		code_buf[0] works as eight flags, "1" representing that the unit
		is an unencoded letter (1 byte), "0" a position-and-length pair
		(2 bytes).  Thus, eight units require at most 16 bytes of code. */
	code_buf_ptr = mask = 1;
	s = 0;
	r = N - F;
	for (i = s; i < r; i++)
		text_buf[i] = ' ';  /* Clear the buffer with
		any character that will appear often. */

	for (len = 0; (len < F) && ulPos < inLen; len++)
		text_buf[r + len] = ucInput[ulPos++];  /* Read F bytes into the last F bytes of the buffer */
	
	//if ((textsize = len) == 0) return;  /* text of size zero */
	if (len == 0) {
		enIdx = 0;
		goto finished;
	}
	
	for (i = 1; i <= F; i++)
		InsertNode(r - i);  /* Insert the F strings,
		each of which begins with one or more 'space' characters.  Note
		the order in which these strings are inserted.  This way,
		degenerate trees will be less likely to occur. */
	InsertNode(r);  /* Finally, insert the whole string just read.  The
		global variables match_length and match_position are set. */
	do {
		if (match_length > len) match_length = len;  /* match_length
			may be spuriously long near the end of text. */
		if (match_length <= THRESHOLD) {
			match_length = 1;  /* Not long enough match.  Send one byte. */
			code_buf[0] |= mask;  /* 'send one byte' flag */
			code_buf[code_buf_ptr++] = text_buf[r];  /* Send uncoded. */
		} else {
			code_buf[code_buf_ptr++] = (unsigned char) match_position;
			code_buf[code_buf_ptr++] = (unsigned char)
				(((match_position >> 4) & 0xf0)
			  | (match_length - (THRESHOLD + 1)));  /* Send position and
					length pair. Note match_length > THRESHOLD. */
		}
		if ((mask <<= 1) == 0) {  /* Shift mask left one bit. */
			for (i = 0; i < code_buf_ptr; i++)  /* Send at most 8 units of */
				ucOutput[enIdx++]=code_buf[i];
			//codesize += code_buf_ptr;
			code_buf[0] = 0;  code_buf_ptr = mask = 1;
		}
		last_match_length = match_length;

		for (i = 0; i< last_match_length && 
			ulPos < inLen; i++){
			c = ucInput[ulPos++];
			DeleteNode(s);		/* Delete old strings and */
			text_buf[s] = c;	/* read new bytes */
			if (s < F - 1)
				text_buf[s + N] = c;  /* If the position is near the end of buffer, extend the buffer to make string comparison easier. */
			s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);
				/* Since this is a ring buffer, increment the position
				   modulo N. */
			InsertNode(r);	/* Register the string in text_buf[r..r+F-1] */
		}
		
		while (i++ < last_match_length) {	/* After the end of text, */
			DeleteNode(s);					/* no need to read, but */
			s = (s + 1) & (N - 1);  r = (r + 1) & (N - 1);
			if (--len) InsertNode(r);		/* buffer may not be empty. */
		}
	} while (len > 0);	/* until length of string to be processed is zero */
	if (code_buf_ptr > 1) {		/* Send remaining code. */
		for (i = 0; i < code_buf_ptr; i++) 
			ucOutput[enIdx++]=code_buf[i];
		//codesize += code_buf_ptr;
	}
finished:
	free(lzssbuf);
	return enIdx;
#endif //#if defined(CHEAT_COMPRESS_MIB_SETTING)	
}

int Decode(unsigned char *ucInput, unsigned int inLen, unsigned char *ucOutput)	/* Just the reverse of Encode(). */
{
#if defined(CHEAT_COMPRESS_MIB_SETTING)
	memcpy(ucOutput, ucInput, inLen);
	return inLen;
#else	
	int  i, j, k, r, c;
	unsigned int  flags;
	unsigned int ulPos=0;
	unsigned int ulExpLen=0;

	if ((text_buf = malloc( N + F - 1 )) == 0) {
		//printf( "fail to get mem %s:%d\n", __FUNCTION__, __LINE__);
		return 0;
	}
	
	for (i = 0; i < N - F; i++)
		text_buf[i] = ' ';
	r = N - F;
	flags = 0;
	while(1) {
		if (((flags >>= 1) & 256) == 0) {
			c = ucInput[ulPos++];
			if (ulPos>inLen)
				break;
			flags = c | 0xff00;		/* uses higher byte cleverly */
		}							/* to count eight */
		if (flags & 1) {
			c = ucInput[ulPos++];
			if ( ulPos > inLen )
				break;
			ucOutput[ulExpLen++] = c;
			text_buf[r++] = c;
			r &= (N - 1);
		} else {
			i = ucInput[ulPos++];
			if ( ulPos > inLen ) break;
			j = ucInput[ulPos++];
			if ( ulPos > inLen ) break;
			
			i |= ((j & 0xf0) << 4);
			j = (j & 0x0f) + THRESHOLD;
			for (k = 0; k <= j; k++) {
				c = text_buf[(i + k) & (N - 1)];
				ucOutput[ulExpLen++] = c;
				text_buf[r++] = c;
				r &= (N - 1);
			}
		}
	}

	free(text_buf);
	return ulExpLen;
#endif //#if defined(CHEAT_COMPRESS_MIB_SETTING)	
}

unsigned int mib_get_real_len(CONFIG_DATA_T type)
{
	switch(type)
	{
		case HW_SETTING:
			return HW_SETTING_SECTOR_LEN;
		case DEFAULT_SETTING:
			return DEFAULT_SETTING_SECTOR_LEN;			
		case CURRENT_SETTING:
			return CURRENT_SETTING_SECTOR_LEN;
		default:			
			return 0;
	}
	
}

int mib_get_flash_offset(CONFIG_DATA_T type)
{
	switch(type)
	{
		case HW_SETTING:
			return HW_SETTING_OFFSET;
		case DEFAULT_SETTING:
			return DEFAULT_SETTING_OFFSET;			
		case CURRENT_SETTING:
			return CURRENT_SETTING_OFFSET;
		default:
			return -1;
	}
	
}

#ifdef MIB_TLV
unsigned int mib_get_setting_len(CONFIG_DATA_T type)
{
	switch(type)
	{
		case HW_SETTING:
			return sizeof(HW_SETTING_T);
		case DEFAULT_SETTING:
		case CURRENT_SETTING:
			return sizeof(APMIB_T);
		default:			
			return 0;
	}
	
}

void mib_display_data_content(CONFIG_DATA_T type, unsigned char * pdata, unsigned int mib_data_len)
{
	int kk;
	printf("\r\n type=%u, mibdata_len = %u",type, mib_data_len);
	printf("\r\n pdata=");
	for(kk=0; kk< mib_data_len; kk++)
	{
		printf("0x%02x_", *(pdata+kk));
		if( (kk+1)%10 == 0) printf("\r\n");
	}
	printf("%p_",CHECKSUM(pdata, mib_data_len));
}

void mib_display_tlv_content(CONFIG_DATA_T type, unsigned char * ptlv, unsigned int mib_tlv_len)
{
	int kk;
	printf("\r\n type=%u, tlv_content_len = %u",type, mib_tlv_len);			
	printf("\r\n tlv_content=");
	for(kk=0; kk< mib_tlv_len; kk++)
	{
		printf("0x%02x_", *(ptlv+kk));
		if( (kk+1)%10 == 0) printf("\r\n");
	}
}

int mib_write_to_raw(const mib_table_entry_T *mib_tbl, void *data, unsigned char *pfile, unsigned int *idx)
{	
	unsigned short tlv_tag=0;	
	unsigned short tlv_len=0;	
	unsigned char tlv_num=0;		
	int i, j, k;		

	if(mib_tbl->type >= TABLE_LIST_T)	
	{

		const mib_table_entry_T *mib = mib_tbl->next_mib_table;		
		unsigned int offset=0;				

		for(i=0 ; mib[i].id ; i++)		
		{			
			const mib_table_entry_T *pmib = &mib[i];			

			if(mib[i].type < TABLE_LIST_T)			
			{		
				mib_write_to_raw(pmib, (void *)((int) data + offset), pfile, idx);				
				offset += pmib->total_size;
			}
			else
			{				
				unsigned int ori_idx = 0;				
				unsigned short *ptlv_len = NULL;								
//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);				
				if((pmib->total_size%pmib->unit_size) == 0)					
					tlv_num = pmib->total_size/pmib->unit_size;
				
				tlv_tag = (pmib->id);
				tlv_tag = WORD_SWAP(tlv_tag); // for x86
				memcpy(pfile+*idx, &tlv_tag, 2);
				*idx+=2;	
				
				tlv_len = WORD_SWAP(tlv_len); // for x86			
				memcpy(pfile+*idx, &tlv_len, 2);				
				ptlv_len = (unsigned short *)(pfile+*idx);				
				*idx+=2;								
				tlv_num = pmib->total_size/pmib->unit_size;				
				ori_idx = *idx;
//printf("\r\n -- ptlv_len[0x%x] *idx[%u] ori_idx[%u] tlv_num[%u]<< ",ptlv_len, *idx,ori_idx,tlv_num);

				for(j=0 ; j<tlv_num ; j++)				
				{
				
					mib_write_to_raw(pmib, (void *)((int) data + offset), pfile, idx);
					offset += pmib->unit_size;					
				}											
				tlv_len = (*idx-ori_idx);
				tlv_len = WORD_SWAP(tlv_len); // for x86
				memcpy(ptlv_len, &tlv_len, 2);							
			}								
		}		
	}
	else
	{	
		unsigned char *pChar = (unsigned char *) data;		
		unsigned short mib_value;

#ifdef VOIP_SUPPORT_TLV_CFG
		if(mib_tbl->type == VOIP_T) 
		{			
			unsigned char *ptlv;
			unsigned short tlv_tag;
			unsigned short tlv_len;
			unsigned int tlv_offset_org;

			//printf("%s(%d)VoIP save TLV\n",__FUNCTION__,__LINE__);
			//printf("%s(%d)id=0x%04x, name=%s, u_sz=%d, t_sz=%d\n",__FUNCTION__,__LINE__,
			//			mib_tbl->id,mib_tbl->name,mib_tbl->unit_size,mib_tbl->total_size);

			/* Tag */
			tlv_tag = mib_tbl->id;
			tlv_tag = WORD_SWAP(tlv_tag);
			memcpy((unsigned char*)((int)pfile + *idx),&tlv_tag,sizeof(tlv_tag));
			*idx+=2;

			/* addr for tlv_len */
			ptlv = (unsigned char*)((int)pfile + *idx);

			/* we don't know exact len now */
			tlv_len = 0;
			tlv_len = WORD_SWAP(tlv_len);
			memcpy(ptlv,&tlv_len,sizeof(tlv_len));
			*idx+=2;
			tlv_offset_org = *idx;

			/* call voip TLV process */
			voip_mib_cfg_write_to_tlv(&mibtbl_voip_root, (void *)data, pfile, idx);				

			/* fill correct tlv_len */
			tlv_len = *idx - tlv_offset_org;
			tlv_len = WORD_SWAP(tlv_len);
			memcpy(ptlv,&tlv_len,sizeof(tlv_len));

			return 1;
		}
#endif /* VOIP_SUPPORT_TLV_CFG */

		tlv_tag = (mib_tbl->id);
		tlv_tag = WORD_SWAP(tlv_tag); // for x86
		memcpy(pfile+*idx, &tlv_tag, 2);		
		*idx+=2;				

		//tlv_len = (mib_tbl->size);
        tlv_len = (mib_tbl->total_size);
		tlv_len = WORD_SWAP(tlv_len); // for x86
		memcpy(pfile+*idx, &tlv_len, 2);

		*idx+=2;	

		memcpy(pfile+*idx, pChar, WORD_SWAP(tlv_len)); // for x86

		*idx+=WORD_SWAP(tlv_len); // for x86
		
	}	
}

unsigned int mib_tlv_save(CONFIG_DATA_T type, void *mib_data, unsigned char *mib_tlvfile, unsigned int *tlv_content_len)
{
	mib_table_entry_T *pmib_tl = NULL;

//fprintf(stderr,"\r\n tlv_content_len = %p, __[%s-%u]",tlv_content_len,__FILE__,__LINE__);				


	if(mib_tlvfile == NULL)
	{
		return 0;
	}
//fprintf(stderr,"\r\n mib_tlvfile = 0x%x, __[%s-%u]",mib_tlvfile,__FILE__,__LINE__);		
	
	pmib_tl = mib_get_table(type);

	if(pmib_tl==0)
	{
		return 0;
	}
	mib_write_to_raw(pmib_tl, (void *)((int) mib_data), mib_tlvfile, tlv_content_len);

	return 1;

}
int mib_search_by_id(const mib_table_entry_T *mib_tbl, unsigned short mib_id, unsigned char *pmib_num, const mib_table_entry_T **ppmib, unsigned int *offset)
{
	int i=0;
	const mib_table_entry_T *mib;
	unsigned short mib_num=0;
	
	memcpy(&mib_num, pmib_num, 1);
	
//printf("\r\n search mib_id=%u, offset=%u, mib_num=%u",mib_id,*offset, mib_num);
	
	
	for (i=0; mib_tbl[i].id; i++)
	{
//printf("\r\n mib_tbl[%u].mib_name=%s",i, mib_tbl[i].name);
		mib = &mib_tbl[i];
		
		if(mib_id == mib_tbl[i].id)
		{
			*offset += mib->offset + mib->unit_size*mib_num;
			*ppmib = mib;
//printf("\r\n !! FIND at %s TBL !!",mib_tbl[i].name);
			return 1;
		}
		else
		{
			if(mib_tbl[i].type >= TABLE_LIST_T)
			{
				
				if((mib->total_size%mib->unit_size) == 0 && mib_num < (mib->total_size /mib->unit_size))
				{					
					*offset += mib->offset + mib->unit_size*mib_num;
//printf("\r\n >> Entry %s TBL >>",mib->name);
					if(mib_search_by_id(mib->next_mib_table, mib_id, pmib_num+1, ppmib, offset) == 1)
					{
						return 1;
					}
					else
					{
//printf("\r\n << Leave %s TBL <<",mib->name);
						*offset -= mib->offset + mib->unit_size*mib_num;
					}
				}
				
			}
		}
	}
	return 0;
}

static int mib_init_value(unsigned char *ptlv_data_value, unsigned short tlv_len, const mib_table_entry_T *mib_tbl, void *data)
{
	unsigned int vInt;
	unsigned short vShort;
	unsigned char *pChar;
	
	
	
#if 0
int j=0;
printf("\r\n mib_tbl->type = %u",mib_tbl->type);
printf("\r\n %s = ",mib_tbl->name);
for(j=0; j<tlv_len; j++)
	printf("0x%x_", *(ptlv_data_value+j));
printf("\r\n");
#endif
//	printf("###%d\n",__LINE__);
			
	switch (mib_tbl->type)
	{
		case BYTE_T:
		case BYTE_ARRAY_T:
			pChar = (unsigned char *) data;
			memcpy(data, ptlv_data_value, tlv_len);	
//printf("add tlv_len=%d\n",tlv_len,__LINE__);
			break;
			
		case WORD_T:
			pChar = (unsigned char *) data;
			memcpy(&vShort, ptlv_data_value, sizeof(vShort));
			vShort = WORD_SWAP(vShort);
			memcpy(data, &vShort, sizeof(vShort));
			break;
			
		case DWORD_T:
		
			pChar = (unsigned char *) data;
			memcpy(&vInt, ptlv_data_value, sizeof(vInt));
			vInt = DWORD_SWAP(vInt);
			memcpy(data, &vInt, sizeof(vInt));
			break;
			
		case STRING_T:
			pChar = (unsigned char *) data;
			strncpy(pChar, ptlv_data_value, mib_tbl->total_size);
			
			//fprintf(stderr,"len=%d,%s data=0x%x,pChar=0x%x\n",mib_tbl->total_size,pChar,data,pChar);
			
			break;
			
		case IA_T:
			pChar = (unsigned char *) data;
			memcpy(data, ptlv_data_value, mib_tbl->total_size);			
			break;
			
		case BYTE5_T:
		case BYTE6_T:
		case BYTE13_T:
			pChar = (unsigned char *) data;
			memcpy(data, ptlv_data_value, mib_tbl->total_size); // avoid alignment issue
			
			break;
			
#ifdef HOME_GATEWAY
#ifdef CONFIG_IPV6
		case RADVDPREFIX_T:
		case DNSV6_T:
		case DHCPV6S_T:
		case ADDR6_T:
		case TUNNEL6_T:
			memcpy(data, ptlv_data_value, mib_tbl->total_size);
			break;		
#endif
#endif

#ifdef VOIP_SUPPORT
#ifdef VOIP_SUPPORT_TLV_CFG
		case VOIP_T:
			// should not run to here
			break;
#else
		case VOIP_T:
			memcpy(data, ptlv_data_value, mib_tbl->total_size);
			break;
#endif
#endif
		default :
			printf("\r\n ERR!no mib_name[%s] type[%u]. __[%s-%u]",mib_tbl->name, mib_tbl->type,__FILE__,__LINE__);			
			return 0;
		
		
	}
#if 0
									//if(mib_tbl->id==MIB_IP_ADDR)
{
int k=0;
fprintf(stderr,"%s: ",mib_tbl->name,tlv_len);
for(k=0;k<tlv_len;k++)
	fprintf(stderr,"%d",*((unsigned char*)data+k));
fprintf(stderr,"\n");
fprintf(stderr,"data=0x%x\n",data);
}
#endif		
	return 1;
}

unsigned short find_same_tag_times(unsigned char *pdata_array, unsigned short data_size)
{
	unsigned short first_tlv_tag;
	unsigned short tlv_tag;	
	unsigned short tlv_len;
	unsigned short times=0;
	unsigned char *idx = pdata_array;
	int i=0;
	
	memcpy(&first_tlv_tag, idx, sizeof(first_tlv_tag));
	//fprintf(stderr,"datasize=%d %d\n",data_size,__LINE__);
	//fprintf(stderr,"idx=0x%x size=%d idx[0]=%x %x  %d\n",idx,sizeof(first_tlv_tag),idx[0],idx[1],__LINE__);
	//fprintf(stderr,"first_tlv_tag=%x %d\n",first_tlv_tag,__LINE__);
	while(i<data_size)
	{
		memcpy(&tlv_tag, idx+i, sizeof(tlv_tag));
		i+=sizeof(tlv_tag);			
		
		//fprintf(stderr,"tlv_tag=%x %d\n",tlv_tag,__LINE__);
		if(tlv_tag == first_tlv_tag)
			times++;
		
		memcpy(&tlv_len, idx+i, sizeof(tlv_len));
		i+=sizeof(tlv_len);
		tlv_len = WORD_SWAP(tlv_len);
		//fprintf(stderr,"tlv_len=%d %d\n",tlv_len,__LINE__);
		
		i+=tlv_len;
		
	}
	#if 0
	for(i=0;i<data_size;i++)
	{
		fprintf(stderr,"%x ",pdata_array[i]);
	}
	#endif
	return times;
}


unsigned int mib_tlv_init_from(const mib_table_entry_T *mib_root_tbl, unsigned char *pdata_array, void *pfile, unsigned int data_size, unsigned int *pmib_root_offset)
{	
	unsigned char *idx;		
	int i, j,k;		
	unsigned short tlv_tag;		
	unsigned short tlv_len;	
	unsigned char tlv_num;	
	unsigned char tlv_data_value[1000];	
	unsigned char *ptlv_data_value=tlv_data_value;		
	unsigned int offset=0;		
	unsigned char mib_num[10];	
	unsigned char *pmib_num = mib_num;	
	memset(mib_num, 0x00, sizeof(mib_num));		

	i=0;	
	idx=pdata_array;	
	while(i<data_size)	
	{		
		memcpy(&tlv_tag, idx+i, sizeof(tlv_tag));
		tlv_tag = WORD_SWAP(tlv_tag);
		i+=sizeof(tlv_tag);					

		memcpy(&tlv_len, idx+i, sizeof(tlv_len));
		tlv_len = WORD_SWAP(tlv_len);
		i+=sizeof(tlv_len);
//		printf("tlv_tag=%d,tlv_len=%d %d\n",tlv_tag,tlv_len,__LINE__);

#ifdef VOIP_SUPPORT_TLV_CFG
		if( tlv_tag == MIB_VOIP_CFG ) 
		{
			unsigned int mib_offset=0;
			unsigned int mib_offset2=0;
			const mib_table_entry_T *pmib_tbl;

			if( mib_search_by_id(mib_root_tbl, tlv_tag, pmib_num, &pmib_tbl, &mib_offset) != 1) {
				printf("\r\n%s(%d)Skip tag:0x%04x, len:0x%04, mib_offset=0x%08x\n",
				__FUNCTION__,__LINE__,tlv_tag,tlv_len,mib_offset);
				
                i+=tlv_len;
                continue;
			}
			mib_offset2=mib_offset;
			voip_mib_cfg_init_from_tlv(	mibtbl_voip, pdata_array+i, 
											pfile, tlv_len, &mib_offset );
			i+=tlv_len;				

			/* voip mib has done, move forward */
			continue;
		}
#endif

		if((tlv_tag & MIB_TABLE_LIST) == 0) // NO member
		{
			const mib_table_entry_T *mib_tbl;
			unsigned int mib_offset=0;

			if((ptlv_data_value=malloc(tlv_len)) == NULL)
			{
				COMP_TRACE(stderr,"\r\n ERR! malloc fail. tlv_tag=%p, tlv_len=%u __[%s-%u]",tlv_tag, tlv_len, __FILE__,__LINE__);
				return 0;
			}
			memcpy(ptlv_data_value, idx+i, tlv_len);
			i+=tlv_len;
			
			//printf("tlv_tag=%d,tlv_len=%d %d\n",tlv_tag,tlv_len,__LINE__);			
			//printf("tlv_tag=%d,tlv_len=%d %d\n",tlv_tag,tlv_len,__LINE__);

			if( mib_search_by_id(mib_root_tbl, tlv_tag, pmib_num, &mib_tbl, &mib_offset) != 1)			
			{
				printf("\r\n Can't find mib_id=%u,%d\n",tlv_tag,__LINE__);
			}
			else
			{
				//printf("\r\n find mib_id=%u ,%d",tlv_tag,__LINE__);		

			//fprintf(stderr,"pfile=0x%x,mib_offset=0x%x! pmib_root_offset=0x%x,%d\n",pfile,mib_offset,*pmib_root_offset,__LINE__);

				mib_offset += *pmib_root_offset;
	//			printf("###%d\n",__LINE__);
				if(mib_tbl != NULL)
				{
					if(mib_init_value(ptlv_data_value, tlv_len, mib_tbl, (void *)((int) pfile + mib_offset)) != 1)
					{
						printf("\r\n Assign mib_name[%s] fail!", mib_tbl->name);
						printf("\r\n mibtbl->id (%08x) unitsize (%d) totoal size (%d) mibtbl->nextbl %p",mib_tbl->id,mib_tbl->unit_size,mib_tbl->total_size,mib_tbl->next_mib_table);
					}
					else
					{
					//fprintf(stderr,"pfile=0x%x,mib_offset=0x%x! pmib_root_offset=0x%x,%d\n",pfile,mib_offset,*pmib_root_offset,__LINE__);
#if 0
									fprintf(stderr,"pfile=0x%x,ssi=%s,%d\n",pfile,pfile+7338,__LINE__);
									//if(mib_tbl->id==MIB_IP_ADDR)
									{
						
									fprintf(stderr,"%s: ",mib_tbl->name,tlv_len);
						
										for(k=0;k<tlv_len;k++)
										fprintf(stderr,"%d",*(ptlv_data_value+k));
										fprintf(stderr,"\n");
									}
#endif

						if(ptlv_data_value != NULL)
							free(ptlv_data_value);
						//mib_display_value(mib_tbl, (void *)((int) pmib_data + mib_offset));
					}

				}
			}
		}
		else // have member		
		{			
			int j=0;
			const mib_table_entry_T *pmib_tbl;
			unsigned int mib_offset=0;
//fprintf(stderr,"have member!\n");
			if( mib_search_by_id(mib_root_tbl, tlv_tag, pmib_num, &pmib_tbl, &mib_offset) != 1)			
			{
				printf("\r\n Can't find mib_id=%u ,%d",tlv_tag,__LINE__);		
				i+=tlv_len;	
			}
			else
			{
				//printf("\r\n find mib_id=%u ,%d",tlv_tag,__LINE__);	
				if((ptlv_data_value=malloc(tlv_len)) == NULL)
				{
					COMP_TRACE(stderr,"\r\n ERR! malloc fail. tlv_tag=%p, tlv_len=%u __[%s-%u]",tlv_tag, tlv_len, __FILE__,__LINE__);
					return 0;
				}
				
				memcpy(ptlv_data_value, idx+i, tlv_len);
			//	fprintf(stderr,"\r\n ptlv_data_value=0x%x tlv_len=%u __[%s-%u]\n",ptlv_data_value, tlv_len,	__FILE__,__LINE__);
				tlv_num = find_same_tag_times(ptlv_data_value, tlv_len);
//fprintf(stderr,"\r\n tlv_num=%u __[%s-%u]\n", tlv_num,  __FILE__,__LINE__);
				if(tlv_num != 0)
					tlv_len = tlv_len/tlv_num;

				while(j<tlv_num)
				{
		//	fprintf(stderr,"\r\n TREE_NODE %s[%u] ENTRY %d\n",pmib_tbl->name,j,__LINE__);
//			printf("\r\n tlv_num=%d j=%d %d",tlv_num,j,__LINE__);
					memcpy(ptlv_data_value, idx+i+(tlv_len*j), tlv_len);
					//printf("\r\n %u<%u/%u",j,pmib_tbl->total_size, pmib_tbl->unit_size );
					if( j < (pmib_tbl->total_size / pmib_tbl->unit_size))
					{
						unsigned int mib_tlb_offset=0;
//printf("\r\n  __[%s-%u]",  __FILE__,__LINE__);
				//		printf("\r\n %u+%u+%u*%u,%d",mib_offset,*pmib_root_offset, j,pmib_tbl->unit_size );
						mib_tlb_offset = mib_offset + *pmib_root_offset+j*(pmib_tbl->unit_size);
				//		printf("\r\n TREE_NODE name =%s[%u] mib_tbl_offset is %u\n",pmib_tbl->name,  j, mib_tlb_offset);
					//	printf("\r\n tlv_len=%u __[%s-%u]", tlv_len,  __FILE__,__LINE__);
						mib_tlv_init_from(pmib_tbl->next_mib_table, ptlv_data_value, pfile, tlv_len, &mib_tlb_offset);
					}
//fprintf(stderr,"\r\n TREE_NODE %s[%u] LEAVE\n",pmib_tbl->name,j);					
					j++;
				}
				if(ptlv_data_value != NULL)
					free(ptlv_data_value);

				i+=tlv_len*tlv_num;				
			}
		}
	}

	return 1;
}

unsigned int mib_tlv_init(const mib_table_entry_T *mib_tbl, unsigned char *from_data, void *pfile, unsigned int tlv_content_len)
{
	unsigned int mib_offset = 0;

	if(mib_tbl == NULL || from_data == NULL || pfile == NULL || tlv_content_len == 0)
		return 0;

	if(mib_tlv_init_from(mib_tbl, from_data, pfile, tlv_content_len, &mib_offset) == 1)
		return 1;
	else
		return 0;

}

#endif // #ifdef MIB_TLV
int cvcfg_mib_compress_write(CONFIG_DATA_T type, char *data, PARAM_HEADER_T *pheader, int fh, int *pcomplen)
{

	unsigned char* pContent = NULL;

	COMPRESS_MIB_HEADER_T compHeader;
	unsigned char *expPtr, *compPtr;
	unsigned int expLen = 0;
	unsigned int compLen;
	unsigned int real_size = 0;
	int zipRate=0;
	char *pcomp_sig;
	int dst = mib_get_flash_offset(type);

	if(dst == -1)
	{
		printf("\r\n ERR!! no flash offset! __[%s-%u]\n",__FILE__,__LINE__);
		return 0;
	}
	
	switch(type)
	{
		case HW_SETTING:
			pcomp_sig = COMP_HS_SIGNATURE;
			break;
		case DEFAULT_SETTING:
			pcomp_sig = COMP_DS_SIGNATURE;
			break;
		case CURRENT_SETTING:
			pcomp_sig = COMP_CS_SIGNATURE;
			break;
		default:
			printf("\r\n ERR!! no type match __[%s-%u]\n",__FILE__,__LINE__);
			return 0;

	}
	expLen = WORD_SWAP(pheader->len)+sizeof(PARAM_HEADER_T); // for x86
	if(expLen == 0)
	{
		printf("\r\n ERR!! no expLen! __[%s-%u]\n",__FILE__,__LINE__);
		return 0;
	}
	real_size = mib_get_real_len(type);
	if(real_size == 0)
	{
		printf("\r\n ERR!! no expLen! __[%s-%u]\n",__FILE__,__LINE__);
		return 0;
	}
	
	if( (compPtr = malloc(real_size)) == NULL)
	{
		printf("\r\n ERR!! malloc  size %u failed! __[%s-%u]\n",real_size,__FILE__,__LINE__);
	}

	if( (expPtr = malloc(expLen)) == NULL)
	{
		printf("\r\n ERR!! malloc  size %u failed! __[%s-%u]\n",expLen,__FILE__,__LINE__);
		if(compPtr != NULL)
			free(compPtr);
	}
	
	if(compPtr != NULL && expPtr!= NULL)
	{

		pContent = &expPtr[sizeof(PARAM_HEADER_T)];	// point to start of MIB data 

		memcpy(pContent, data, WORD_SWAP(pheader->len)); // for x86
		
		memcpy(expPtr, pheader, sizeof(PARAM_HEADER_T));


		compLen = Encode(expPtr, expLen, compPtr+sizeof(COMPRESS_MIB_HEADER_T));
		sprintf(compHeader.signature,"%s",pcomp_sig);
		compHeader.compRate = WORD_SWAP((expLen/compLen)+1); // for x86
		compHeader.compLen = DWORD_SWAP(compLen); // for x86
		memcpy(compPtr, &compHeader, sizeof(COMPRESS_MIB_HEADER_T));

	

		write(fh, compPtr, compLen+sizeof(COMPRESS_MIB_HEADER_T));

		*pcomplen = compLen;
		
		if(compPtr != NULL)
			free(compPtr);
		if(expPtr != NULL)
			free(expPtr);

		return 1;
			
	}
	else
	{
		return 0;
	}

	return 0;

}
#endif

#ifdef DEBUG_CVCFG
/* This API saves uncompressed raw file for debug */
static int flash_write_file(char *buf, int len , char * filename)
{
	int fh;
	int ok=1;

	printf("\n%s(%d)write file %s len=%d\n",__FUNCTION__,__LINE__,filename,len);

	fh = open(filename, O_RDWR|O_CREAT|O_TRUNC,0644);

	if ( fh == -1 )
		return 0;

	if ( write(fh, buf, len) != len)
		ok = 0;

	close(fh);
	sync();

	return ok;
}
#endif

