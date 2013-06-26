/*
 *      Web server handler routines for TCP/IP stuffs
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *      Authors: Dick Tam	<dicktam@realtek.com.tw>
 *
 */





/*-- System inlcude files --*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <time.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <net/if.h>



/*-- Local inlcude files --*/
#include "apmib.h"
#include "boa.h"
#include "apform.h"
#include "utility.h"

#define	CONFIG_DIR	"/var/config"
#define CA_FNAME	CONFIG_DIR"/cacert.pem"
#define CERT_FNAME	CONFIG_DIR"/client.pem"
#define strACSURLWrong  "ACS's URL can't be empty!"
#define strSSLWrong "CPE does not support SSL! URL should not start with 'https://'!"
#define strSetACSURLerror "Set ACS's URL error!"
#define strSetUserNameerror "Set User Name error!"
#define strSetPasserror "Set Password error!"
#define strSetInformEnableerror "Set Inform Enable error!"
#define strSetInformIntererror "Set Inform Interval error!"
#define strSetConReqUsererror "Set Connection Request UserName error!"
#define strSetConReqPasserror "Set Connection Request Password error!"
#define strSetCWMPFlagerror "Set CWMP_FLAG error!"
#define strGetCWMPFlagerror "Get CWMP_FLAG error!"
#define strUploaderror "Upload error!"
#define strMallocFail "malloc failure!"
#define strArgerror "Insufficient args\n"
#define strSetCerPasserror  "Set CPE Certificat's Password error!"

const char IFCONFIG[] = "/bin/ifconfig";
const char IPTABLES[] = "/bin/iptables";
const char ARG_T[] = "-t";
const char FW_PREROUTING[] = "PREROUTING";
const char ARG_I[] = "-i";
const char LANIF[] = "br0";
const char ARG_TCP[] = "TCP";
const char FW_DPORT[] = "--dport";
const char RMACC_MARK[] = "0x1000";


#define RECONNECT_MSG(url) { \
	req_format_write(wp, ("<html><body><blockquote><h4>Change setting successfully!" \
                "<form><input type=button value=\"  OK  \" OnClick=window.location.replace(\"%s\")></form></blockquote></body></html>"), url);\
}


#define UPLOAD_MSG(url) { \
	req_format_write(wp, ("<html><body><blockquote><h4>Upload a file successfully!" \
                "<form><input type=button value=\"  OK  \" OnClick=window.location.replace(\"%s\")></form></blockquote></body></html>"), url);\
}

#define CWMPPID  "/var/run/cwmp.pid"
void off_tr069()
{
	int cwmppid=0;
	int status;

	cwmppid = getPid((char*)CWMPPID);

	printf("\ncwmppid=%d\n",cwmppid);

	if(cwmppid > 0)
		kill(cwmppid, 15);

}

int startCWMP(void)
{
	char vChar=0;
	char strPort[16];
	unsigned int conreq_port=0;
	unsigned int cwmp_flag;

/*	
	//lan interface enable or disable
	apmib_get(CWMP_LAN_IPIFENABLE, (void *)&vChar);
	if(vChar==0)
	{
		va_cmd(IFCONFIG, 2, 1, "br0", "down");
		//system("ifconfig ");
		printf("Disable br0 interface\n");
	}
	
	//eth0 interface enable or disable
	apmib_get(CWMP_LAN_ETHIFENABLE, (void *)&vChar);
	if(vChar==0)
	{
		va_cmd(IFCONFIG, 2, 1, "eth0", "down");
		printf("Disable eth0 interface\n");
	}
*/
	/*add a wan port to pass */
	apmib_get(MIB_CWMP_CONREQ_PORT, (void *)&conreq_port);
	if(conreq_port==0) conreq_port=7547;
	sprintf(strPort, "%u", conreq_port );
	va_cmd(IPTABLES, 15, 1, ARG_T, "mangle", "-A", (char *)FW_PREROUTING,
		(char *)ARG_I, "!", (char *)LANIF, "-p", (char *)ARG_TCP,
		(char *)FW_DPORT, strPort, "-j", (char *)"MARK", "--set-mark", RMACC_MARK);

	/*start the cwmpClient program*/


	apmib_get(MIB_CWMP_FLAG, (void *)&cwmp_flag);
	if( cwmp_flag&CWMP_FLAG_AUTORUN )
		va_cmd( "/bin/cwmpClient", 0, 0 );


	return 0;
}

#ifdef _CWMP_WITH_SSL_
//copy from fmmgmt.c
//find the start and end of the upload file.
FILE * uploadGetCert(request *wp, unsigned int *startPos, unsigned *endPos)
{
#if 0 //keith remove
	FILE *fp=NULL;
	struct stat statbuf;
	unsigned char c, *buf;
	char boundary[80];


	//if (wp->method == M_POST)
	if (wp->flags & WEBS_POST_REQUEST)
	{
		int i;

		fstat(wp->post_data_fd, &statbuf);
		lseek(wp->post_data_fd, SEEK_SET, 0);

		printf("file size=%d\n",statbuf.st_size);
		fp=fopen(wp->post_file_name,"rb");
		if(fp==NULL) goto error;

		memset( boundary, 0, sizeof( boundary ) );
		if( fgets( boundary,80,fp )==NULL ) goto error;
		if( boundary[0]!='-' || boundary[1]!='-') goto error;

		i= strlen( boundary ) - 1;
		while( boundary[i]=='\r' || boundary[i]=='\n' )
		{
			boundary[i]='\0';
			i--;
		}
		printf( "boundary=%s\n", boundary );
	}
	else goto error;


   	//printf("_uploadGet\n");
   	do
   	{
		if(feof(fp))
		{
			printf("Cannot find start of file\n");
			goto error;
		}
		c= fgetc(fp);
		if (c!=0xd)
			continue;
		c= fgetc(fp);
		if (c!=0xa)
			continue;
		c= fgetc(fp);
		if (c!=0xd)
			continue;
		c= fgetc(fp);
		if (c!=0xa)
			continue;
		break;
	}while(1);
	(*startPos)=ftell(fp);

   	if(fseek(fp,statbuf.st_size-0x200,SEEK_SET)<0)
      		goto error;

	do
	{
		if(feof(fp))
		{
			printf("Cannot find end of file\n");
			goto error;
		}
		c= fgetc(fp);
		if (c!=0xd)
			continue;
		c= fgetc(fp);
		if (c!=0xa)
			continue;

		{
			int i, blen;

			blen= strlen( boundary );
			for( i=0; i<blen; i++)
			{
				c= fgetc(fp);
				//printf("%c(%u)", c, c);
				if (c!=boundary[i])
				{
					ungetc( c, fp );
					break;
				}
			}
			//printf("\r\n");
			if( i!=blen ) continue;
		}

		break;
	}while(1);
	(*endPos)=ftell(fp)-strlen(boundary)-2;

   	return fp;
error:
#endif //	#if 0 //keith remove

   	return NULL;
}

#endif //#ifdef _CWMP_WITH_SSL_

///////////////////////////////////////////////////////////////////
void formTR069Config(request *wp, char *path, char *query)
{
	char	*strData;
	char tmpBuf[100];
	unsigned char vChar;
	unsigned int cwmp_flag;
	int vInt;
	// Mason Yu
	char changeflag=0;
	unsigned int informEnble;
	unsigned int informInterv;
	char cwmp_flag_value=1;
	char tmpStr[256+1];
	int cur_port;
	char isDisConReqAuth=0;

//displayPostDate(wp->post_data);

#ifdef _CWMP_WITH_SSL_
	//CPE Certificat Password
	strData = req_get_cstream_var(wp, ("CPE_Cert"), (""));
	if( strData[0] )
	{
		strData = req_get_cstream_var(wp, ("certpw"), (""));

		changeflag = 1;
		if ( !apmib_set( MIB_CWMP_CERT_PASSWORD, (void *)strData))
		{
			strcpy(tmpBuf, strSetCerPasserror);
			goto setErr_tr069;
		}
		else
			printf("Debug Test!\n");
		goto end_tr069;
	}
#endif

	strData = req_get_cstream_var(wp, ("url"), (""));
	//if ( strData[0] )
	{
		if ( strlen(strData)==0 )
		{
			strcpy(tmpBuf, (strACSURLWrong));
			goto setErr_tr069;
		}
#ifndef _CWMP_WITH_SSL_
		if ( strstr(strData, "https://") )
		{
			strcpy(tmpBuf, (strSSLWrong));
			goto setErr_tr069;
		}
#endif
		if ( !apmib_set( MIB_CWMP_ACS_URL, (void *)strData))
		{
			strcpy(tmpBuf, (strSetACSURLerror));
			goto setErr_tr069;
		}
	}

	strData = req_get_cstream_var(wp, ("username"), (""));
	//if ( strData[0] )
	{
		if ( !apmib_set( MIB_CWMP_ACS_USERNAME, (void *)strData)) {
			strcpy(tmpBuf, (strSetUserNameerror));
			goto setErr_tr069;
		}
	}

	strData = req_get_cstream_var(wp, ("password"), (""));
	//if ( strData[0] )
	{
		if ( !apmib_set( MIB_CWMP_ACS_PASSWORD, (void *)strData)) {
			strcpy(tmpBuf, (strSetPasserror));
			goto setErr_tr069;
		}
	}

	strData = req_get_cstream_var(wp, ("enable"), (""));
	if ( strData[0] ) {
		informEnble = (strData[0]=='0')? 0:1;
		apmib_get( MIB_CWMP_INFORM_ENABLE, (void*)&vInt);
		if(vInt != informEnble){
			int allow=1;
			changeflag = 1;
			if ( !apmib_set( MIB_CWMP_INFORM_ENABLE, (void *)&informEnble)) {
				strcpy(tmpBuf, (strSetInformEnableerror));
				goto setErr_tr069;
			}
		}
	}

	strData = req_get_cstream_var(wp, ("interval"), (""));
	if ( strData[0] ) {
		informInterv = atoi(strData);
		
		if(informEnble == 1){
			apmib_get( MIB_CWMP_INFORM_INTERVAL, (void*)&vInt);

			if(vInt != informInterv){
				changeflag = 1;
				if ( !apmib_set( MIB_CWMP_INFORM_INTERVAL, (void *)&informInterv)) {
					strcpy(tmpBuf, (strSetInformIntererror));
					goto setErr_tr069;
				}
			}
		}
	}

#ifdef _TR069_CONREQ_AUTH_SELECT_
	strData = req_get_cstream_var(wp, ("disconreqauth"), (""));
	if ( strData[0] ) {
		cwmp_flag=0;
		vChar=0;

		if( apmib_get( MIB_CWMP_FLAG2, (void *)&cwmp_flag ) )
		{
			changeflag = 1;

			if(strData[0]=='0')
				cwmp_flag = cwmp_flag & (~CWMP_FLAG2_DIS_CONREQ_AUTH);
			else{
				cwmp_flag = cwmp_flag | CWMP_FLAG2_DIS_CONREQ_AUTH;
				isDisConReqAuth = 1;
			}

			if ( !apmib_set( MIB_CWMP_FLAG2, (void *)&cwmp_flag)) {
				strcpy(tmpBuf, (strSetCWMPFlagerror));
				goto setErr_tr069;
			}
		}else{
			strcpy(tmpBuf, (strGetCWMPFlagerror));
			goto setErr_tr069;
		}
	}
#endif

	//if connection reuqest auth is enabled, don't handle conreqname & conreqpw to keep the old values
	if(!isDisConReqAuth)
	{
		strData = req_get_cstream_var(wp, ("conreqname"), (""));
		//if ( strData[0] )
		{
			if ( !apmib_set( MIB_CWMP_CONREQ_USERNAME, (void *)strData)) {
				strcpy(tmpBuf, (strSetConReqUsererror));
				goto setErr_tr069;
			}
		}

		strData = req_get_cstream_var(wp, ("conreqpw"), (""));
		//if ( strData[0] )
		{
			if ( !apmib_set( MIB_CWMP_CONREQ_PASSWORD, (void *)strData)) {
				strcpy(tmpBuf, (strSetConReqPasserror));
				goto setErr_tr069;
			}
		}
	}//if(isDisConReqAuth)

	strData = req_get_cstream_var(wp, ("conreqpath"), (""));
	//if ( strData[0] )
	{
		apmib_get( MIB_CWMP_CONREQ_PATH, (void *)tmpStr);
		if (strcmp(tmpStr,strData)!=0){
			changeflag = 1;
			if ( !apmib_set( MIB_CWMP_CONREQ_PATH, (void *)strData)) {
				strcpy(tmpBuf, ("Set Connection Request Path error!"));
				goto setErr_tr069;
			}
		}
	}

	strData = req_get_cstream_var(wp, ("conreqport"), (""));
	if ( strData[0] ) {
		cur_port = atoi(strData);
		apmib_get( MIB_CWMP_CONREQ_PORT, (void *)&vInt);
		if ( vInt != cur_port ) {
			changeflag = 1;
			if ( !apmib_set( MIB_CWMP_CONREQ_PORT, (void *)&cur_port)) {
				strcpy(tmpBuf, ("Set Connection Request Port error!"));
				goto setErr_tr069;
			}
		}
	}

/*for debug*/
	strData = req_get_cstream_var(wp, ("dbgmsg"), (""));
	if ( strData[0] ) {
		cwmp_flag=0;
		vChar=0;

		if( apmib_get( MIB_CWMP_FLAG, (void *)&cwmp_flag ) )
		{
			if(strData[0]=='0')
				cwmp_flag = cwmp_flag & (~CWMP_FLAG_DEBUG_MSG);
			else
				cwmp_flag = cwmp_flag | CWMP_FLAG_DEBUG_MSG;

			if ( !apmib_set( MIB_CWMP_FLAG, (void *)&cwmp_flag)) {
				strcpy(tmpBuf, (strSetCWMPFlagerror));
				goto setErr_tr069;
			}
		}else{
			strcpy(tmpBuf, (strGetCWMPFlagerror));
			goto setErr_tr069;
		}
	}

#ifdef _CWMP_WITH_SSL_
	strData = req_get_cstream_var(wp, ("certauth"), (""));
	if ( strData[0] ) {
		cwmp_flag=0;
		vChar=0;

		if( apmib_get( MIB_CWMP_FLAG, (void *)&cwmp_flag ) )
		{
			if(strData[0]=='0')
				cwmp_flag = cwmp_flag & (~CWMP_FLAG_CERT_AUTH);
			else
				cwmp_flag = cwmp_flag | CWMP_FLAG_CERT_AUTH;

			changeflag = 1;
			if ( !apmib_set( MIB_CWMP_FLAG, (void *)&cwmp_flag)) {
				strcpy(tmpBuf, (strSetCWMPFlagerror));
				goto setErr_tr069;
			}
		}else{
			strcpy(tmpBuf, (strGetCWMPFlagerror));
			goto setErr_tr069;
		}
	}
#endif

	strData = req_get_cstream_var(wp, ("sendgetrpc"), (""));
	if ( strData[0] ) {
		cwmp_flag=0;
		vChar=0;

		if( apmib_get( MIB_CWMP_FLAG, (void *)&cwmp_flag ) )
		{
			if(strData[0]=='0')
				cwmp_flag = cwmp_flag & (~CWMP_FLAG_SENDGETRPC);
			else
				cwmp_flag = cwmp_flag | CWMP_FLAG_SENDGETRPC;

			if ( !apmib_set(MIB_CWMP_FLAG, (void *)&cwmp_flag)) {
				strcpy(tmpBuf, (strSetCWMPFlagerror));
				goto setErr_tr069;
			}
		}else{
			strcpy(tmpBuf, (strGetCWMPFlagerror));
			goto setErr_tr069;
		}
	}

	strData = req_get_cstream_var(wp, ("skipmreboot"), (""));
	if ( strData[0] ) {
		cwmp_flag=0;
		vChar=0;

		if( apmib_get( MIB_CWMP_FLAG, (void *)&cwmp_flag ) )
		{
			if(strData[0]=='0')
				cwmp_flag = cwmp_flag & (~CWMP_FLAG_SKIPMREBOOT);
			else
				cwmp_flag = cwmp_flag | CWMP_FLAG_SKIPMREBOOT;

			if ( !apmib_set( MIB_CWMP_FLAG, (void *)&cwmp_flag)) {
				strcpy(tmpBuf, (strSetCWMPFlagerror));
				goto setErr_tr069;
			}
		}else{
			strcpy(tmpBuf, (strGetCWMPFlagerror));
			goto setErr_tr069;
		}
	}

	strData = req_get_cstream_var(wp, ("delay"), (""));
	if ( strData[0] ) {
		cwmp_flag=0;
		vChar=0;

		if( apmib_get( MIB_CWMP_FLAG, (void *)&cwmp_flag ) )
		{
			if(strData[0]=='0')
				cwmp_flag = cwmp_flag & (~CWMP_FLAG_DELAY);
			else
				cwmp_flag = cwmp_flag | CWMP_FLAG_DELAY;

			if ( !apmib_set( MIB_CWMP_FLAG, (void *)&cwmp_flag)) {
				strcpy(tmpBuf, (strSetCWMPFlagerror));
				goto setErr_tr069;
			}
		}else{
			strcpy(tmpBuf, (strGetCWMPFlagerror));
			goto setErr_tr069;
		}
	}
	strData = req_get_cstream_var(wp, ("autoexec"), (""));
	if ( strData[0] ) {
		cwmp_flag=0;
		vChar=0;

		if( apmib_get( MIB_CWMP_FLAG, (void *)&cwmp_flag ) )
		{
			int onoff_tr069 = 0;
			if(strData[0]=='0') {
				if ( cwmp_flag & CWMP_FLAG_AUTORUN )
					changeflag = 1;

				cwmp_flag = cwmp_flag & (~CWMP_FLAG_AUTORUN);
				cwmp_flag_value = 0;
			}else {
				if ( !(cwmp_flag & CWMP_FLAG_AUTORUN) )
					changeflag = 1;

				cwmp_flag = cwmp_flag | CWMP_FLAG_AUTORUN;
				cwmp_flag_value = 1;
			}

			if ( !apmib_set( MIB_CWMP_FLAG, (void *)&cwmp_flag)) {
				strcpy(tmpBuf, (strSetCWMPFlagerror));
				goto setErr_tr069;
			}
			
			onoff_tr069 = (cwmp_flag & CWMP_FLAG_AUTORUN)==0?0:1;
			apmib_set( MIB_CWMP_ENABLED, (void *)&onoff_tr069);
			
		}else{
			strcpy(tmpBuf, (strGetCWMPFlagerror));
			goto setErr_tr069;
		}
	}
/*end for debug*/
end_tr069:

	// Mason Yu
#ifdef APPLY_CHANGE
	if ( changeflag ) {
		if ( cwmp_flag_value == 0 ) {  // disable TR069
			off_tr069();
		} else {                       // enable TR069
			off_tr069();
			if (-1==startCWMP()){
				strcpy(tmpBuf, ("Start tr069 Fail *****"));
				printf("Start tr069 Fail *****\n");
				goto setErr_tr069;
			}
		}
	}
#endif

// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	apmib_update_web(CURRENT_SETTING);
	
	strData = req_get_cstream_var(wp, ("submit-url"), (""));
	RECONNECT_MSG(strData);// display reconnect msg to remote
	return;



setErr_tr069:
	ERR_MSG(tmpBuf);
}


/*******************************************************/
/*show extra fileds at tr069config.htm*/
/*******************************************************/
#ifdef _CWMP_WITH_SSL_


void formTR069CPECert(request *wp, char *path, char *query)
{
	char	*strData;
	char tmpBuf[100];
	FILE	*fp=NULL,*fp_input;
	unsigned char *buf;
	unsigned int startPos,endPos,nLen,nRead;
	if ((fp = uploadGetCert(wp, &startPos, &endPos)) == NULL)
	{
		strcpy(tmpBuf, (strUploaderror));
 		goto setErr_tr069cpe;
 	}

	nLen = endPos - startPos;
	//printf("filesize is %d\n", nLen);
	buf = malloc(nLen+1);
	if (!buf)
	{
		strcpy(tmpBuf, (strMallocFail));
 		goto setErr_tr069cpe;
 	}

	fseek(fp, startPos, SEEK_SET);
	nRead = fread((void *)buf, 1, nLen, fp);
	buf[nRead]=0;
	if (nRead != nLen)
 		printf("Read %d bytes, expect %d bytes\n", nRead, nLen);

	//printf("write to %d bytes from %08x\n", nLen, buf);

	fp_input=fopen(CERT_FNAME,"w");
	if (!fp_input)
		printf("create %s file fail!\n", CERT_FNAME);
	fprintf(fp_input,buf);
	printf("create file %s\n", CERT_FNAME);
	free(buf);
	fclose(fp_input);

#if 0 //keith
	if( va_cmd( "/bin/flatfsd",1,1,"-s" ) )
		printf( "[%d]:exec 'flatfsd -s' error!",__FILE__ );
#endif
	off_tr069();

	if (startCWMP() == -1)
	{
		strcpy(tmpBuf, ("Start tr069 Fail *****"));
		printf("Start tr069 Fail *****\n");
		goto setErr_tr069cpe;
	}

	strData = req_get_cstream_var(wp, ("submit-url"), ("/tr069config.htm"));
	UPLOAD_MSG(strData);// display reconnect msg to remote
	return;

setErr_tr069cpe:
	ERR_MSG(tmpBuf);
}


void formTR069CACert(request *wp, char *path, char *query)
{
	char	*strData;
	char tmpBuf[100];
	FILE	*fp=NULL,*fp_input;
	unsigned char *buf;
	unsigned int startPos,endPos,nLen,nRead;
	if ((fp = uploadGetCert(wp, &startPos, &endPos)) == NULL)
	{
		strcpy(tmpBuf, (strUploaderror));
 		goto setErr_tr069ca;
 	}

	nLen = endPos - startPos;
	//printf("filesize is %d\n", nLen);
	buf = malloc(nLen+1);
	if (!buf)
	{
		strcpy(tmpBuf, (strMallocFail));
 		goto setErr_tr069ca;
 	}

	fseek(fp, startPos, SEEK_SET);
	nRead = fread((void *)buf, 1, nLen, fp);
	buf[nRead]=0;
	if (nRead != nLen)
 		printf("Read %d bytes, expect %d bytes\n", nRead, nLen);

	//printf("write to %d bytes from %08x\n", nLen, buf);

	fp_input=fopen(CA_FNAME,"w");
	if (!fp_input)
		printf("create %s file fail!\n", CA_FNAME );
	fprintf(fp_input,buf);
	printf("create file %s\n",CA_FNAME);
	free(buf);
	fclose(fp_input);

#if 0 //keith
	if( va_cmd( "/bin/flatfsd",1,1,"-s" ) )
		printf( "[%d]:exec 'flatfsd -s' error!",__FILE__ );
#endif
	off_tr069();

	if (startCWMP() == -1)
	{
		strcpy(tmpBuf, ("Start tr069 Fail *****"));
		printf("Start tr069 Fail *****\n");
		goto setErr_tr069ca;
	}

	strData = req_get_cstream_var(wp, ("submit-url"), ("/tr069config.htm"));

	UPLOAD_MSG(strData);// display reconnect msg to remote
	return;

setErr_tr069ca:
	ERR_MSG(tmpBuf);
}

int ShowACSCertCPE(request *wp)
{
	int nBytesSent=0;
	unsigned char vChar=0;
	int isEnable=0;
	unsigned int cwmp_flag;

	if ( apmib_get( MIB_CWMP_FLAG, (void *)&cwmp_flag) )
		if ( (cwmp_flag & CWMP_FLAG_CERT_AUTH)!=0 )
			isEnable=1;

	nBytesSent += req_format_write(wp, ("  <tr>\n"));
	nBytesSent += req_format_write(wp, ("      <td width=\"30%%\"><font size=2><b>ACS Certificates CPE:</b></td>\n"));
	nBytesSent += req_format_write(wp, ("      <td width=\"70%%\"><font size=2>\n"));
	nBytesSent += req_format_write(wp, ("      <input type=\"radio\" name=certauth value=0 %s >No&nbsp;&nbsp;\n"), isEnable==0?"checked":"" );
	nBytesSent += req_format_write(wp, ("      <input type=\"radio\" name=certauth value=1 %s >Yes\n"), isEnable==1?"checked":"" );
	nBytesSent += req_format_write(wp, ("      </td>\n"));
	nBytesSent += req_format_write(wp, ("  </tr>\n"));

//		"\n"), isEnable==0?"checked":"", isEnable==1?"checked":"" );

	return nBytesSent;
}

int ShowMNGCertTable(request *wp)
{
	int nBytesSent=0;
	char buffer[256]="";

	apmib_get(MIB_CWMP_CERT_PASSWORD,buffer);

	nBytesSent += req_format_write(wp, ("\n"
		"<table border=0 width=\"500\" cellspacing=4 cellpadding=0>\n"
		"  <tr><hr size=1 noshade align=top></tr>\n"
		"  <tr>\n"
		"      <td width=\"30%%\"><font size=2><b>Certificat Management:</b></td>\n"
		"      <td width=\"70%%\"><b></b></td>\n"
		"  </tr>\n"
		"\n"));


	nBytesSent += req_format_write(wp, ("\n"
		"  <tr>\n"
		"      <td width=\"30%%\"><font size=2><b>CPE Certificat Password:</b></td>\n"
		"      <td width=\"70%%\">\n"
		"		<form action=/boafrm/formTR069Config method=POST name=\"cpe_passwd\">\n"
		"		<input type=\"text\" name=\"certpw\" size=\"24\" maxlength=\"64\" value=\"%s\">\n"
		"		<input type=\"submit\" value=\"Apply\" name=\"CPE_Cert\">\n"
		"		<input type=\"reset\" value=\"Undo\" name=\"reset\">\n"
		"		<input type=\"hidden\" value=\"/tr069config.htm\" name=\"submit-url\">\n"
		"		</form>\n"
		"      </td>\n"
		"  </tr>\n"
		"\n"), buffer);

	nBytesSent += req_format_write(wp, ("\n"
		"  <tr>\n"
		"      <td width=\"30%%\"><font size=2><b>CPE Certificat:</b></td>\n"
		"      <td width=\"70%%\"><font size=2>\n"
		"           <form action=/boafrm/formTR069CPECert method=POST enctype=\"multipart/form-data\" name=\"cpe_cert\">\n"
		"           <input type=\"file\" name=\"binary\" size=24>&nbsp;&nbsp;\n"
		"           <input type=\"submit\" value=\"Upload\" name=\"load\">\n"
		"           </form>\n"
		"      </td>\n"
		"  </tr>\n"
		"\n"));

	nBytesSent += req_format_write(wp, ("\n"
		"  <tr>\n"
		"      <td width=\"30%%\"><font size=2><b>CA Certificat:</b></td>\n"
		"      <td width=\"70%%\"><font size=2>\n"
		"           <form action=/boafrm/formTR069CACert method=POST enctype=\"multipart/form-data\" name=\"ca_cert\">\n"
		"           <input type=\"file\" name=\"binary\" size=24>&nbsp;&nbsp;\n"
		"           <input type=\"submit\" value=\"Upload\" name=\"load\">\n"
		"           </form>\n"
		"      </td>\n"
		"  </tr>\n"
		"\n"));

	nBytesSent += req_format_write(wp, ("\n"
		"</table>\n"
		"\n"));


	return nBytesSent;
}
#endif //#ifdef _CWMP_WITH_SSL_


#ifdef _TR069_CONREQ_AUTH_SELECT_
int ShowAuthSelect(request *wp)
{
	int nBytesSent=0;
	unsigned char vChar=0;
	int isDisable=0;
	unsigned int cwmp_flag;

	if ( apmib_get( MIB_CWMP_FLAG2, (void *)&cwmp_flag) )
		if ( (cwmp_flag & CWMP_FLAG2_DIS_CONREQ_AUTH)!=0 )
			isDisable=1;

	nBytesSent += req_format_write(wp, ("  <tr>\n"));
	nBytesSent += req_format_write(wp, ("      <td width=\"30%%\"><font size=2><b>Authentication:</b></td>\n"));
	nBytesSent += req_format_write(wp, ("      <td width=\"70%%\"><font size=2>\n"));
	nBytesSent += req_format_write(wp, ("      <input type=\"radio\" name=disconreqauth value=1 %s onClick=\"return authSel()\">Disabled&nbsp;&nbsp;\n"), isDisable==1?"checked":"" );
	nBytesSent += req_format_write(wp, ("      <input type=\"radio\" name=disconreqauth value=0 %s onClick=\"return authSel()\">Enabled\n"), isDisable==0?"checked":"" );
	nBytesSent += req_format_write(wp, ("      </td>\n"));
	nBytesSent += req_format_write(wp, ("  </tr>\n"));

	return nBytesSent;
}
int ShowAuthSelFun(request *wp)
{
	int nBytesSent=0;
	nBytesSent += req_format_write(wp, ("function authSel()\n"));
	nBytesSent += req_format_write(wp, ("{\n"));
	nBytesSent += req_format_write(wp, ("		if ( document.tr069.disconreqauth[0].checked ) {\n"));
	nBytesSent += req_format_write(wp, ("			disableTextField(document.tr069.conreqname);\n"));
	nBytesSent += req_format_write(wp, ("			disableTextField(document.tr069.conreqpw);\n"));
	nBytesSent += req_format_write(wp, ("		} else {\n"));
	nBytesSent += req_format_write(wp, ("			enableTextField(document.tr069.conreqname);\n"));
	nBytesSent += req_format_write(wp, ("			enableTextField(document.tr069.conreqpw);\n"));
	nBytesSent += req_format_write(wp, ("		}\n"));
	nBytesSent += req_format_write(wp, ("}\n"));
	return nBytesSent;
}
#endif

int TR069ConPageShow(request *wp, int argc, char **argv)
{
	int nBytesSent=0;
	char *name;
	unsigned int cwmp_flag;
	
	//printf("get parameter=%s\n", argv[0]);
	name = argv[0];
	if (name == NULL) {
   		fprintf(stderr, strArgerror);
   		return -1;
   	}

#ifdef _CWMP_WITH_SSL_
	if ( !strcmp(name, ("ShowACSCertCPE")) )
		return ShowACSCertCPE( wp );
	else if ( !strcmp(name, ("ShowMNGCertTable")) )
		return ShowMNGCertTable( wp );
#endif

#ifdef _TR069_CONREQ_AUTH_SELECT_
	if ( !strcmp(name, ("ShowAuthSelect")) )
		return ShowAuthSelect( wp );
	if ( !strcmp(name, ("ShowAuthSelFun")) )
		return ShowAuthSelFun( wp );
	if ( !strcmp(name, ("DisConReqName")) ||
             !strcmp(name, ("DisConReqPwd"))   )
        {
		unsigned char vChar=0;
		int isDisable=0;

		if ( apmib_get( MIB_CWMP_FLAG2, (void *)&cwmp_flag) )
			if ( (cwmp_flag & CWMP_FLAG2_DIS_CONREQ_AUTH)!=0 )
				isDisable=1;
		if(isDisable) return req_format_write(wp, "disabled");
	}
#endif

	return nBytesSent;
}

