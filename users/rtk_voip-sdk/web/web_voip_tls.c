#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
/*-- Local inlcude files --*/
#include "web_voip.h"
#if defined(CONFIG_APP_BOA)
// TODO
#else
#ifdef HOME_GATEWAY
#include "../wsIntrn.h"
#endif
#endif
#include "apmib.h"
#include "utility.h"
#include "web_voip.h"

/*-- OpenSSL --*/
#include <openssl/ssl.h>
#include <openssl/err.h>

#define UPLOAD_MSG(url) { \
	websHeader(wp); \
	websWrite(wp, T("<body><blockquote><h4>Upload a file successfully!" \
                "<form><input type=button value=\"  OK  \" OnClick=window.location.replace(\"%s\")></form></blockquote></body>"), url);\
	websFooter(wp); \
	websDone(wp, 200); \
}

#define CERT1_PATH	"/var/web/cacert_01.pem"
#define CERT2_PATH	"/var/web/cacert_02.pem"
#define CERT3_PATH	"/var/web/cacert_03.pem"
#define CERT4_PATH	"/var/web/cacert_04.pem"
#define	CERT_DEV_PATH	"/dev/mtdblock2"

#define CERT_SIZE 16384

#define DATA_LENGTH	256

enum{ISSUE_DATE, VALID_DATE};

static int flash_cert_read(int certNum)
{
	int fh;
	FILE *fp;
	char buf[CERT_SIZE];
	char *certFile=NULL;
	
	fh = open(CERT_DEV_PATH, O_RDWR);
	if ( fh == -1 )
		return 0;
	switch(certNum){
		case 1:
			lseek(fh, 0L, SEEK_SET);
			certFile=CERT1_PATH;
		break;
		case 2:
			lseek(fh, CERT_SIZE, SEEK_SET);
			certFile=CERT2_PATH;
		break;
		case 3:
			lseek(fh, (CERT_SIZE * 2), SEEK_SET);
			certFile=CERT3_PATH;
		break;
		case 4:
			lseek(fh, (CERT_SIZE * 3), SEEK_SET);
			certFile=CERT4_PATH;
		break;
	}
	read(fh, buf, CERT_SIZE);
	close(fh);
	fp=fopen(certFile, "w");
	if(!fp)
		return 0;
	fprintf(fp, buf);
	fclose(fp);	
	return 1;
}
		
static int getCertIssuer(int certNum, char *issuer)
{
	X509 *cert;
	FILE *fp;
	char *head=NULL, *tail=NULL;
	char data[DATA_LENGTH];
	struct stat buf;
	
	switch(certNum){
		case 1:
			if(stat(CERT1_PATH, &buf) == -1 && errno == ENOENT)
				flash_cert_read(certNum);
			fp=fopen(CERT1_PATH,"r");
		break;
		case 2:
			if(stat(CERT2_PATH, &buf) == -1 && errno == ENOENT)
				flash_cert_read(certNum);
			fp=fopen(CERT2_PATH,"r");
		break;
		case 3:
			if(stat(CERT3_PATH, &buf) == -1 && errno == ENOENT)
				flash_cert_read(certNum);
			fp=fopen(CERT3_PATH,"r");
		break;
		case 4:
			if(stat(CERT4_PATH, &buf) == -1 && errno == ENOENT)
				flash_cert_read(certNum);
			fp=fopen(CERT4_PATH,"r");
		break;		
	}
	
	if(!fp)
		return 0;
	cert=PEM_read_X509(fp, NULL, NULL, NULL);
	if(!cert){
		fclose(fp);
		return 0;
	}
	X509_NAME_oneline(X509_get_issuer_name(cert), data, DATA_LENGTH);
	fclose(fp);
	X509_free(cert);
	/*parse the issuer*/
	head=strstr(data,"CN=");
	if(head != NULL)
		tail=strchr(head,'/');
	if(tail != NULL)
		*tail='\0';
	strcpy(issuer,head+strlen("CN="));
	return 1;
}

static int getCertDate(int certNum, char *issueDate, int type)
{
	X509 *cert;
	FILE *fp;
	ASN1_TIME *cert_time=NULL;
	struct stat buf;
	
	switch(certNum){
		case 1:
			if(stat(CERT1_PATH, &buf) == -1 && errno == ENOENT)
				flash_cert_read(certNum);
			fp=fopen(CERT1_PATH,"r");
		break;
		case 2:
			if(stat(CERT2_PATH, &buf) == -1 && errno == ENOENT)
				flash_cert_read(certNum);
			fp=fopen(CERT2_PATH,"r");
		break;
		case 3:
			if(stat(CERT3_PATH, &buf) == -1 && errno == ENOENT)
				flash_cert_read(certNum);
			fp=fopen(CERT3_PATH,"r");
		break;
		case 4:
			if(stat(CERT4_PATH, &buf) == -1 && errno == ENOENT)
				flash_cert_read(certNum);
			fp=fopen(CERT4_PATH,"r");
		break;		
	}
	
	if(!fp)
		return 0;
	cert=PEM_read_X509(fp, NULL, NULL, NULL);
	if(!cert){
		fclose(fp);
		return 0;
	}
	if(type == ISSUE_DATE)
		cert_time = X509_get_notBefore(cert);
	else
		cert_time = X509_get_notAfter(cert);
	if(!cert_time){
		fclose(fp);
		X509_free(cert);
		return 0;
	}
	/*parse the time format to yy/mm/dd*/
	issueDate[0]=cert_time->data[0];
	issueDate[1]=cert_time->data[1];
	issueDate[2]='/';
	issueDate[3]=cert_time->data[2];
	issueDate[4]=cert_time->data[3];
	issueDate[5]='/';
	issueDate[6]=cert_time->data[4];
	issueDate[7]=cert_time->data[5];
	issueDate[8]='\0';
	fclose(fp);
	X509_free(cert);
	return 1;
}

static int clear_cert(int certNum)
{
	int buf[CERT_SIZE]={'\0'};
	int	fp, result;
	
	printf("%s:%dcert_%d\n",__FUNCTION__,__LINE__,certNum);
	fp=open(CERT_DEV_PATH, O_RDWR|O_CREAT|O_TRUNC);
	if (fp == -1)
		return 0;
	result=lseek(fp, (CERT_SIZE * (certNum - 1)), SEEK_SET);
	if(-1 == result){
		perror("lseek fail!");
		return 0;
	}
	if(write(fp, buf, CERT_SIZE) != CERT_SIZE){
		printf("size not match!\n");
		return 0;
	}
	close(fp);
	sync();
	return 1;
}
     
void asp_voip_TLSCertUpload(webs_t wp, char_t *path, char_t *query)
{
#if defined(CONFIG_APP_BOA)
// TODO
#else
	char_t	*strData=NULL;
	int	fp, certNum;
	char tmpBuf[200];
	char *buf=NULL;
	off_t result;
		
	if(0 == wp->lenPostData){
		strcpy(tmpBuf,T("Upload file is empty!"));
		ERR_MSG(tmpBuf);
		printf("data error!\n");
		return;
	}
	
	if(wp->lenPostData > CERT_SIZE){
		strcpy(tmpBuf,T("Your CA is too large!"));
		ERR_MSG(tmpBuf);
		printf("data error!\n");
		return;
	}
	
	if(strcmp(websGetVar(wp,T("submit_cert1"),T("")),"UPLOAD")==0){
		unlink(CERT1_PATH);
		certNum=1;
	}else if(strcmp(websGetVar(wp,T("submit_cert2"),T("")),"UPLOAD")==0){
		unlink(CERT2_PATH);
		certNum=2;
	}else if(strcmp(websGetVar(wp,T("submit_cert3"),T("")),"UPLOAD")==0){
		unlink(CERT3_PATH);
		certNum=3;
	}else if(strcmp(websGetVar(wp,T("submit_cert4"),T("")),"UPLOAD")==0){
		unlink(CERT4_PATH);
		certNum=4;
	}else{
		printf("no of ca file\n");
		return;
	}
	
	/*clear the flash seg. before write*/
	if(!clear_cert(certNum))
		return;
	buf = (char *)malloc(wp->lenPostData + 1);
	if(NULL == buf){
		printf("memory allocate fail\n");
		return;
	}
	memcpy(buf, wp->postData, wp->lenPostData);
	/*add the eof for writting to the file*/
	buf[wp->lenPostData]=EOF;
	fp=open(CERT_DEV_PATH, O_RDWR|O_CREAT|O_TRUNC);
	if (fp == -1){
		strcpy(tmpBuf,T("Write cert to flash FAIL!"));
		ERR_MSG(tmpBuf);
		printf("error: cannot create file\n");
		return;
	}
	result=lseek(fp, (CERT_SIZE * (certNum - 1)), SEEK_SET);
	if(-1 == result){
		perror("lseek fail!");
		return;
	}
	if(write(fp, buf, wp->lenPostData + 1) != wp->lenPostData + 1){
		printf("size not match!\n");
		return;
	}
	close(fp);
	free(buf);
	chmod(CERT_DEV_PATH, DEFFILEMODE);
	sync();
	sprintf(tmpBuf,"/voip_tls.asp?cert=%d",certNum);
	strData = websGetVar(wp, T("submit-url"), T(tmpBuf));
	UPLOAD_MSG(strData);
#endif
}

int asp_voip_TLSGetCertInfo(int eid, webs_t wp, int argc, char_t **argv)
{
	int certNum;
	
	certNum = atoi(websGetVar(wp, T("cert"), "0"));
	if (strcmp(argv[0], "certNum")==0)
	{
		websWrite(wp, "%d", certNum);
	}else if(strcmp(argv[0], "fxs")==0){
		switch(certNum)
		{
			case 1:
			case 2:
				websWrite(wp, "%d", 0);
			break;
			case 3:
			case 4:
				websWrite(wp, "%d", 1);
			break;
		}
	}else if(strcmp(argv[0], "proxy")==0){
		switch(certNum)
		{
			case 1:
			case 3:
				websWrite(wp, "%d", 0);
			break;
			case 2:
			case 4:
				websWrite(wp, "%d", 1);
			break;
		}
	}else if(strcmp(argv[0], "issuer")==0){
		char issuer[DATA_LENGTH];
		
		if(getCertIssuer(certNum, issuer))
			websWrite(wp, "%s", issuer);
		else
			websWrite(wp, "%s", "");
	}else if(strcmp(argv[0], "issuerDate")==0){
		char issueDate[DATA_LENGTH]={'\0'};
		
		if(getCertDate(certNum,issueDate,ISSUE_DATE))
			websWrite(wp, "%s", issueDate);
		else
			websWrite(wp, "%s", "");
	}else if(strcmp(argv[0], "validDate")==0){
		char validDate[DATA_LENGTH]={'\0'};
		
		if(getCertDate(certNum,validDate,VALID_DATE))
			websWrite(wp, "%s", validDate);
		else
			websWrite(wp, "%s", "");
	}else{
		return -1;
	}
	
	return 0;
}
