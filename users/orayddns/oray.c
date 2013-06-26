#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <get_connection.h>
#include <ret_codes.h>
#include "oray.h"

#ifdef _MIPS_
#define SWAP32(n)							\
(((n) << 24) | (((n) & 0xff00) << 8) | (((n) >> 8) & 0xff00) | ((n) >> 24))
#else
#define SWAP32(n) (n)
#endif

char usage[]="-u username -p passowd -h help -H host -v vesion";
print_usage(const char *app, FILE *fp)
{
	fprintf(fp,"%s:\n",app);
	fprintf(fp,"usage %s\n",usage);
}
print_version( FILE *fp)
{
	fprintf(fp,"oray 0.0\n");
}
/*Implementation of Oray DDNS*/
static int get_flags(struct arguments *args, int argc, char *argv[])
{

	int c;

	for(;;) {

		int option_index = 0;
		static struct option long_options[] = {
			{ "user",		1, 0, 'u' },
			{ "help",		0, 0, 'h' },
 			{ "password",	1, 0, 'p' },
			{ "host",		1, 0, 'H' },
			{ "version",	0, 0, 'v' },
			{ NULL,		0, 0, 0  }
		};

		c = getopt_long(argc, argv, "u:p:H:hv",
				long_options, &option_index);

		if(c == -1) break;

		switch(c) {
		case 'u':
			strcpy(args->user,optarg);
			break;
		case 'p':
			strcpy(args->password,optarg);
			break;
		case 'H':
			strcpy(args->phServer,optarg);
			break;
		case 'h':
			print_usage(argv[0], stdout);
			exit(0);
		case 'v':
			print_version(stdout);
			exit(0);
		}
	}
	
	return RET_OK;

}

char * trim(char * ptr)
{
    int start,end,i;
    if (ptr)
    {
  for(start=0; isspace(ptr[start]); start++)
      ;
  for(end=strlen(ptr)-1; isspace(ptr[end]); end--)
      ;
  for(i=start; i<=end; i++)
      ptr[i-start]=ptr[i];
  ptr[end-start+1]='\0';
  return (ptr);
    }
    else
  return NULL;
}

//thing to do. get, register, then update
/* Send a http request to get then server's name and type of user
  * IN	: s - socket handle
  *
  *Out	:
  *Return :
  */
int oray_InfoRequest(const int s,struct arguments *args)
{
	char buf1[1024];
	int i;
	unsigned char digest[16];
	char message[BUFSIZE];

	memset(buf1,0x0,1024);
	memset(message,0x0,BUFSIZE);
	sprintf(buf1,ORAY_XML_HEADER);
#ifdef ORAY_ID_USED
	sprintf(buf1+strlen(buf1),ORAY_XML_BODY_ID,args->id);
#endif

	md5_buffer(args->password,strlen(args->password),digest);
	sprintf(buf1+strlen(buf1),ORAY_XML_BODY_USER,args->user);
	sprintf(buf1+strlen(buf1),"%s",ORAY_XML_BODY_PASSWORD_HEADER);

	for(i=0;i<16;i++)
	{
		sprintf(buf1+strlen(buf1),"%02x",digest[i]);
	}
	sprintf(buf1+strlen(buf1),"%s",ORAY_XML_BODY_PASSWORD_TAILER);
	sprintf(buf1+strlen(buf1),ORAY_XML_TAILER,ORAY_CLIENT_VERSION);
	
	sprintf(message,ORAY_HTTP_HEADER);
	sprintf(message+strlen(message),ORAY_HTTP_CONTENT_LENGTH,strlen(buf1));
	sprintf(message+strlen(message),ORAY_HTTP_TAILER);
	strcat(message+strlen(message),buf1);

	/*send request*/
	if(write(s, message, strlen(message)) == -1) {
		printf( "write() failed");
		return RET_WARNING;
	}
	return RET_OK;
}

int oray_InfoReply(const int s,struct arguments *args)
{
	char server_msg[BUFSIZE];
	char resultstr[8];
	char ipStr[16];
	char *pstart, *pend;
	int result;
	memset(server_msg,0x0,BUFSIZE);
	if(oray_tcpread(s, server_msg, sizeof(server_msg) - 1) < 0) {
		printf( "read() failed");
		return RET_WARNING;
       }

	/*parse message:GetMiscInfoResult ClientIP PHServer  UserType   */
	if(strstr(server_msg, "HTTP/1.1 200 OK") ||
           strstr(server_msg, "HTTP/1.0 200 OK") ) {

		pstart = strstr(server_msg, MISCINFORESULTHEADER);
		pend = strstr(pstart,MISCINFORESULTTAILER);
		memset(resultstr,0x0,sizeof(result));
		memcpy(resultstr,pstart+strlen(MISCINFORESULTHEADER),pend-pstart-strlen(MISCINFORESULTHEADER));
		result=atoi(resultstr);
		if(0==result)
		{
			pstart=strstr(pend,CLIENTIPHEADER);
			pend = strstr(pstart,CLIENTIPTAILER);
			memset(ipStr,0x0,sizeof(ipStr));
			memcpy(ipStr,pstart+strlen(CLIENTIPHEADER),pend-(pstart+strlen(CLIENTIPHEADER)));
			args->clientIP = inet_addr(ipStr);

			pstart=strstr(pend,PHSEVERHEADER);
			pend = strstr(pstart,PHSEVERTAILER);
			memcpy(args->phServer,pstart+strlen(PHSEVERHEADER),pend-(pstart+strlen(PHSEVERHEADER)));
			trim(args->phServer);
			pstart=strstr(pend,USERTYPEHEADER);
			args->userType = *(unsigned char *)(pstart+strlen(USERTYPEHEADER))-'0';
		}
		else if(1002 == result)
		{
			/*get info failed*/
			return RET_ERROR;
		}
	} else {
		printf("Internal Server Error");
		return RET_ERROR;
	}

}
/*Register the Domain name by tcp connections
  *In		:
  *Out	:
  *Return :
  */
int oray_getServCmd(unsigned char *msg)
{
	char buf[4];
	if(strlen(msg)<3)
		return -1;
	memcpy(buf,msg,3);
	buf[4]='\0';
	return atoi(buf);
}

int oray_sendAuth(const int s, struct arguments *args)
{
	char msg[64];
	memset(msg,0x0,sizeof(msg));
	sprintf(msg,"AUTH ROUTER\r\n");
	if(write(s, msg, strlen(msg)) == -1) {
		printf("write() failed");
		return RET_WARNING;
	}
}
static void string_trim_right_crlf(unsigned char *s)
{
    long len;
    len = strlen(s);
    unsigned char *p;
    p = s;
    p += len;
    p--; //

    while( ( (*p == '\r') || (*p == '\n')) && (p>=s)) {
        *p = 0;
        p--;
    }
}

dumphex(unsigned char *buf, int len)
{
	int i=0;
	printf("dump hex:\n");
	for(i=0;i<len;i++)
	{
		if(!((i+1) % 16))
			printf("\n");
		printf("%02x ",buf[i]);
	}
	printf("\n");
}
int oray_getServChallenge(unsigned char *server_msg, int len, struct arguments *args)
{
	int keylen=0;
	char *ptr;
	if(len > 64)
		printf("Server's challenge is too long (>64)\n");
	string_trim_right_crlf(server_msg);
	keylen=ILibBase64Decode(server_msg+4, strlen(server_msg)-4, &ptr);
	memcpy(args->challengeKey,ptr,keylen);
	args->keylen=keylen;
	free(ptr);
//	dumphex(args->challengeKey,strlen(args->challengeKey));
	return RET_OK;
}

/*mark_debug NOTE!!!! Base64 copy from mini_upnp*/
static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

/* encode 3 8-bit binary bytes as 4 '6-bit' characters */
static void ILibencodeblock( unsigned char in[3], unsigned char out[4], int len )
{
	out[0] = cb64[ in[0] >> 2 ];
	out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
	out[2] = (unsigned char) (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
	out[3] = (unsigned char) (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}

/*! \fn ILibBase64Encode(unsigned char* input, const int inputlen, unsigned char** output)
	\brief Base64 encode a stream adding padding and line breaks as per spec.
	\par
	\b Note: The encoded stream must be freed
	\param input The stream to encode
	\param inputlen The length of \a input
	\param output The encoded stream
	\returns The length of the encoded stream
*/
int ILibBase64Encode(unsigned char* input, const int inputlen, unsigned char** output)
{
	unsigned char* out=NULL;
	unsigned char* in;
	
	*output = (unsigned char*)malloc(((inputlen * 4) / 3) + 5);
	out = *output;
	if (out == NULL)
		return 0;
	in  = input;
	
	if (input == NULL || inputlen == 0)
	{
		*output = NULL;
		return 0;
	}
	
	while ((in+3) <= (input+inputlen))
	{
		ILibencodeblock(in, out, 3);
		in += 3;
		out += 4;
	}
	if ((input+inputlen)-in == 1)
	{
		ILibencodeblock(in, out, 1);
		out += 4;
	}
	else
	if ((input+inputlen)-in == 2)
	{
		ILibencodeblock(in, out, 2);
		out += 4;
	}
	*out = 0;
	
	return (int)(out-*output);
}

/* Decode 4 '6-bit' characters into 3 8-bit binary bytes */
static void ILibdecodeblock( unsigned char in[4], unsigned char out[3] )
{
	out[ 0 ] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
	out[ 1 ] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
	out[ 2 ] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);
}

/*! \fn ILibBase64Decode(unsigned char* input, const int inputlen, unsigned char** output)
	\brief Decode a base64 encoded stream discarding padding, line breaks and noise
	\par
	\b Note: The decoded stream must be freed
	\param input The stream to decode
	\param inputlen The length of \a input
	\param output The decoded stream
	\returns The length of the decoded stream
*/
int ILibBase64Decode(unsigned char* input, const int inputlen, unsigned char** output)
{
	unsigned char* inptr;
	unsigned char* out=NULL;
	unsigned char v;
	unsigned char in[4];
	int i, len;
	
	if (input == NULL || inputlen == 0)
	{
		*output = NULL;
		return 0;
	}
	
	*output = (unsigned char*)malloc(((inputlen * 3) / 4) + 4);
	out = *output;
	if (out == NULL)
		return 0;
	inptr = input;
	
	while( inptr <= (input+inputlen) )
	{
		for( len = 0, i = 0; i < 4 && inptr <= (input+inputlen); i++ )
		{
			v = 0;
			while( inptr <= (input+inputlen) && v == 0 ) {
				v = (unsigned char) *inptr;
				inptr++;
				v = (unsigned char) ((v < 43 || v > 122) ? 0 : cd64[ v - 43 ]);
				if( v ) {
					v = (unsigned char) ((v == '$') ? 0 : v - 61);
				}
			}
			if( inptr <= (input+inputlen) ) {
				len++;
				if( v ) {
					in[ i ] = (unsigned char) (v - 1);
				}
			}
			else {
				in[i] = 0;
			}
		}
		if( len )
		{
			ILibdecodeblock( in, out );
			out += len-1;
		}
	}
	*out = 0;
	return (int)(out-*output);
}

int oray_sendChallengeResp(const int s, struct arguments *args)
{
	int len=0;
	char msg[256];
	char msg1[256];
	int shift=0;
	char response[16];
	long encryptedKey;
	long int clientinfo=0;
	long servertime;
	long a1,a2,a3;
	unsigned char *ptr;
	int enlen;

	memset(msg,0x0,sizeof(msg));
	memcpy((void *)&servertime,args->challengeKey+6,sizeof(servertime));
	
	servertime=SWAP32((unsigned long)servertime);
	servertime |=~(long int)(ORAY_CHALLENGEKEY);
	shift=(int)(servertime % 30);

	// shift fixup, for ARM
	if (shift < 0) {
		a1 = servertime << (0 - shift);
		a3 = ~(0xffffffff << (0 - shift));
		a2 = servertime >> (32 + shift);
	}		
	else {
		a1 = servertime << (32 - shift);
		a2 = servertime >> shift;
		a3 = ~(0xffffffff << (32 - shift));
	}	

	encryptedKey = a1 | (a2 & a3);

	sprintf(msg,"%s ",args->user);
	len +=strlen(msg);

	encryptedKey=SWAP32((unsigned long)encryptedKey);
	memcpy(msg+len,&encryptedKey,4);
	len+=4;
	
	clientinfo=SWAP32((unsigned long)ORAY_CLIENTINFO);
	memcpy(msg+len,&clientinfo,4);
	len+=4;

	/*Add encrypted password*/
	//dumphex(args->challengeKey,strlen(args->challengeKey));
	hmac_md5(args->challengeKey,16,args->password,strlen(args->password), response);
	
	memcpy(msg+len,response,16);
	len+=16;

	memset(msg1,0x0,sizeof(msg1));
	if(enlen=ILibBase64Encode(msg, len, &ptr))
	{
		memcpy(msg1,ptr,enlen);
		free(ptr);
	}
	
	strcat(msg1,"\r\n");
	
	if(write(s, msg1, strlen(msg1)) == -1) {
		printf("write() failed");
		return RET_WARNING;
	}
	return RET_OK;
}

int oray_getDomainFromServ(unsigned char *serv_msg, int len, struct arguments *args)
{
	int i=0;
	char *pstart, *pend;
	if(0==strncmp(serv_msg+3,"\r\n.\r\n",strlen("\r\n.\r\n")))
	{
		/*no domain name registered*/
		return RET_OK;
	}
	pstart=strstr(serv_msg,"\r\n");
	while(pstart &&(unsigned long)pstart < (unsigned long)(serv_msg+len-strlen("\r\n.\r\n")) && i < ORAY_DOMAIN_NAME_MAX)
	{
		pstart+=2;
		pend=strstr(pstart,"\r\n");
		if(NULL==pend)
			break;
		memcpy(args->domainName[i].name,pstart,pend-pstart);
		args->domainName[i].valid=1;
		args->domainName[i].registered=0;
		pstart=pend;
		i++;
	}
	return RET_OK;
}

int oray_regDoaminName(const int s, struct arguments *args)
{
	int i;
	char msg[512];
	memset(msg,0,sizeof(msg));
	for(i=0;i<ORAY_DOMAIN_NAME_MAX;i++)
	{
		if(args->domainName[i].valid  && !args->domainName[i].registered)
		{
			sprintf(msg+strlen(msg),"REGI A %s\r\n",args->domainName[i].name);
		}
	}
	sprintf(msg+strlen(msg),"%s\r\n","CNFM");
	
	if(write(s, msg, strlen(msg)) == -1) {
		printf( "write() failed");
		return RET_WARNING;
	}
	return RET_OK;
}

int dumpRegisterStatus(struct arguments *args)
{
	int i=0;
	for(i=0;i<ORAY_DOMAIN_NAME_MAX;i++)
	{
		printf("name: %s\n",args->domainName[i].name);
		printf("valid: %d\n",args->domainName[i].valid);
		printf("registered: %d\n",args->domainName[i].registered);
	}
	printf("sessionnumber %d\n",args->sessionNumber);
	printf("initSeq %d\n",args->initSeq);
}
int oray_getRegRespone(unsigned char *serv_msg, int len,struct arguments *args)
{
	int i;
	char *pstart, *pend;
	char buf[12];
	pstart=serv_msg;
	i=0;
	while(pstart && (unsigned long)pstart< (unsigned long)(serv_msg+len) && (i<ORAY_DOMAIN_NAME_MAX))
	{
		if(args->domainName[i].valid && !args->domainName[i].registered)
		{
			if(0==strncmp(pstart,"250",strlen("250")))
			{
				/*registered Successfully*/
				args->domainName[i].registered=1;
			}
			else
			{
				args->domainName[i].registered=0;
			}
			/*skip CRLF*/
			pstart=strstr(pstart,"\r\n");
			if(NULL==pstart)
				break;
			pstart+=2;
		}
		/*CNFM*/
		if((ORAY_DOMAIN_NAME_MAX-1)==i)
		{
			if((unsigned long)pstart<(unsigned long)(serv_msg+len))
			{
				if(0==strncmp(pstart,"250",strlen("250")))
				{
					/*4 is len of "250 " */
					pstart+=4;
					pend=buf;
					memset(buf,0x0,sizeof(buf));
					while(*pstart != ' ')
					{
						*pend++=*pstart++;
					}
					args->sessionNumber=atoi(buf);
					memset(buf,0x0,sizeof(buf));
					pstart+=1;
					pend=buf;
					while(*pstart != '\r')
					{
						*pend++=*pstart++;
					}
					args->initSeq=atoi(buf);
					pstart+=2;
				}
				else
				{
					printf("Error CNFM responese\n");
				}
			}
			else
			{
				/*no CNFM respone ? Error*/
				printf("Thers is no CNFM response\n");
			}		
		}
		/*Next one */
		i++;
	}
//	dumpRegisterStatus(args);
}

int oray_sendQuit(const int s, struct arguments *args)
{
	char msg[64];         
        memset(msg,0x0,sizeof(msg));
        sprintf(msg,"QUIT\r\n");
        if(write(s, msg, strlen(msg)) == -1) {
                printf( "write() failed");
                return RET_WARNING;
        }
}

static int oray_udpread(int sock,char *data,int size)
{
	int nread ,ret=1; 	
	struct timeval tv;
	fd_set rfds;	

	FD_ZERO(&rfds);
	FD_SET(sock,&rfds);
	tv.tv_sec = ORAY_UDP_TIMEOUT_SEC;
	tv.tv_usec = 0;

	ret = select(sock+1 ,&rfds,(fd_set *)NULL,(fd_set *)NULL,&tv);

	if(ret == -1) //data rcved!!
	{
		printf("udp rcv select error!! \n");
		return ret;			
	}	
	else if (ret)
	{

		nread = read(sock, data, size);		
	}
	else
	{
		printf("udp rcv timeout!! \n");
		return RET_TIMEOUT;	
	}
	return nread;	
}

static int oray_tcpread(int sock,char *data,int size)
{
	int nread ,ret=1; 	
	struct timeval tv;
	fd_set rfds;	

	FD_ZERO(&rfds);
	FD_SET(sock,&rfds);
	tv.tv_sec = ORAY_TCP_TIMEOUT_SEC;
	tv.tv_usec = 0;

	ret = select(sock+1 ,&rfds,(fd_set *)NULL,(fd_set *)NULL,&tv);

	if(ret == -1) //data rcved!!
	{
		printf("tcp rcv error!!\n");
		return ret;			
	}	
	else if (ret)
	{

		nread = read(sock, data, size);
	}
	else
	{
		printf("tcp rcv timeout!!\n");
		return RET_TIMEOUT;	
	}
	return nread;	
}

int oray_login(const int s,struct arguments *args)
{
	/**/
	char server_msg[BUFSIZE];
	int len;
	int cmd;
	int ret=ORAY_LOGINFAIL;
	ORAY_LOGIN_STAT status=ORAY_INIT;

	while(1)
	{
		cmd=-1;
		if(ORAY_CLOSED == status)
			break;
		memset(server_msg,0x0,sizeof(server_msg));
		if(RET_TIMEOUT==(len=oray_tcpread(s,server_msg,sizeof(server_msg) - 1)))
		{
			status=ORAY_CLOSED;
			continue;
		}
		if(len > 0)
			cmd=oray_getServCmd(server_msg);
		switch(status)
		{
			case ORAY_INIT:
				if(220 == cmd)
				{
					oray_sendAuth(s,args);
					status=ORAY_AUTH_SEND;
				}
				else
				{
					printf("Error cmd ORAY_INIT\n");
					oray_sendQuit(s,args);
					status=ORAY_DISCONNECT;
				}
				break;;
			case ORAY_AUTH_SEND:
				if(334==cmd)
				{
					oray_getServChallenge(server_msg,len,args);
					oray_sendChallengeResp(s,args);
					status=ORAY_AUTH_RESP_SEND;
				}
				break;
			case ORAY_AUTH_RESP_SEND:
				if(250 ==cmd)
				{
					oray_getDomainFromServ(server_msg,len,args);
					status = ORAY_AUTH_SUCCESS;
					/*total send all send one by one*/
					oray_regDoaminName(s,args);
					status = ORAY_REG_SEND;
				}else if(535 == cmd)
				{
					printf("Authentication failure\n");
					oray_sendQuit(s,args);
					status=ORAY_DISCONNECT;
				}else if(504 == cmd)
				{
					printf("Authentication mechanism unsupported\ns");
					oray_sendQuit(s,args);
					status=ORAY_DISCONNECT;
				}
				break;
			case ORAY_REG_SEND:
				if(250 == cmd)
				{
					/*block ack parse information*/
					oray_getRegRespone(server_msg,len,args);
					status=ORAY_REG_SUCCESS;
					/*disconnect*/
					oray_sendQuit(s,args);
					status=ORAY_DISCONNECT;
					ret=ORAY_LOGINSUCCESS;
				}else if(507 == cmd)
				{
					printf("no name registered.\n");
					oray_sendQuit(s,args);
					status=ORAY_DISCONNECT;
				}else if(508 == cmd)
				{
					printf("Register or unregister domain name error\n");
					oray_sendQuit(s,args);
					status=ORAY_DISCONNECT;
				}
				break;
			case ORAY_DISCONNECT:
				if(221 == cmd)
				{
					status=ORAY_CLOSED;
					/*let ret vaule not changed here*/
				}
				else 
				{
					printf("Error Quit not Success\n");
					if(ORAY_LOGINSUCCESS != ret)
						ret=ORAY_QUITFAIL;
				}
				break;
			default:
				break;
		}
	}	
	return ret;
}

/*update the domain with heart beacon by udp
  *In		:
  *Out	:
  *Return :
  */
int oray_udpRequest(const int s,struct arguments *args, int opcode)
{
	orayUpdatePkt updatepkt;
	char buf[16];
	memset((void *)&updatepkt,0x0,sizeof(orayUpdatePkt));
	updatepkt.sessionnumber=SWAP32((unsigned long)args->sessionNumber);
	//dumphex(&updatepkt,4);	
	updatepkt.opcode=opcode;
	updatepkt.seq=args->clientSeq++;
	updatepkt.checksum=0-(updatepkt.opcode+updatepkt.seq);
	
	CBlowfish(args->challengeKey,strlen(args->challengeKey));
	CBlowfish_EnCode((unsigned char *)(&updatepkt.opcode),buf,16);
			
	/*send msg*/
	memcpy((void *)(&updatepkt.opcode),buf,16);

	/*Endian process*/
	updatepkt.opcode=SWAP32((unsigned long)updatepkt.opcode);
	updatepkt.seq=SWAP32((unsigned long)updatepkt.seq);
	updatepkt.checksum=SWAP32((unsigned long)updatepkt.checksum);
	updatepkt.padding=SWAP32((unsigned long)updatepkt.padding);

//	dumphex(&updatepkt,20);
	
	if(write(s, (void*)(&updatepkt),sizeof(orayUpdatePkt)) == -1) {
		printf( "write() failed");
		return RET_WARNING;
	}
	return RET_OK;
}


int oray_udpRespone(const int s, struct arguments *args, int opcode)
{
	orayUpdateResPkt *updateRes=NULL;
	unsigned char server_msg[64];
	
	memset(server_msg,0x0,sizeof(server_msg));
	if(RET_TIMEOUT==oray_udpread(s,server_msg,sizeof(server_msg) - 1))
	{
		return RET_TIMEOUT;
	}
	updateRes=(orayUpdateResPkt *)(&server_msg);

	/*session number*/
	updateRes->sessionnumber=SWAP32((unsigned long)(updateRes->sessionnumber));
	if(updateRes->sessionnumber != (args->sessionNumber))
	{
		printf("Not Our Session");
		return RET_WARNING;
	}

	/*client ip*/
	if(updateRes->clientIP != args->clientIP)
	{
		printf("Not Our IP");
		return RET_WARNING;
	}
	updateRes->opcode=SWAP32((unsigned long)(updateRes->opcode));
	updateRes->seq=SWAP32((unsigned long)(updateRes->seq));
	updateRes->checksum=SWAP32((unsigned long)(updateRes->checksum));
	updateRes->padding=SWAP32((unsigned long)(updateRes->padding));
	
	CBlowfish(args->challengeKey,strlen(args->challengeKey));
	CBlowfish_DeCode((unsigned char *)(&updateRes->opcode),(unsigned char *)(&updateRes->opcode),16);

	/*checksum*/
	if((updateRes->opcode+updateRes->seq+updateRes->checksum) != 0)
	{
		printf("CheckSum Error");
		return RET_WARNING;
	}

	/*get opcode*/
	if(updateRes->opcode != opcode)
	{
		printf("update failed");
		return RET_WARNING;
	}

	/*get seq number*/
	if(0 == args->servSeq)
	{
		args->servSeq=updateRes->seq;
	}
	else
	if(updateRes->seq > (args->servSeq+3))
	{
		printf("Server sequence number not synced");
		return RET_RESTART;
	}
	args->servSeq=updateRes->seq;

	return RET_OK;
}
int oray_saveInfo(struct arguments *args)
{
	FILE *fp;
	int i;
	fp=fopen(ORAY_DDNS_STATUS,"w");
	if(NULL==fp)
	{
		printf("Can not open %s\n",ORAY_DDNS_STATUS);
	}
	/*status online 4*/
	fprintf(fp,"%d\n",4);
	/*type*/
	fprintf(fp,"%d\n",args->userType);
	for(i=0;i<ORAY_DOMAIN_NAME_MAX;i++)
	{
		if(args->domainName[i].valid)
			fprintf(fp,"%s\n",args->domainName[i].name);
	}
	fclose(fp);
	return RET_OK;
}
int main(int argc, char *argv[])
{      
	struct arguments args;
 	int s, ret, count;
	int updated;
	char buffer[64];
	char *ptr=(char *)buffer;

	(void)memset(&args, 0, sizeof(struct arguments));

	/*Get account and password*/
	if(get_flags(&args, argc, argv) != RET_OK) {
		return RET_WRONG_USAGE;
	}

LOGIN_RESTART:	
	/*0 means connecting*/
	system("echo 0 > /var/ddns_status");

	/*get Info*/
	s = get_connection(ORAYDNSHOST1, HTTPPORT, &ptr);
	if(s == -1) {
		printf("%s: %s", ptr, ORAYDNSHOST);
		/*1 means connect failed*/
		system("echo 1 > /var/ddns_status");
		ret = RET_WARNING;
		sleep(60);
		goto LOGIN_RESTART;
	} else {
		ret = oray_InfoRequest(s, &args);
		if(ret == RET_OK) {
			ret = oray_InfoReply(s, &args);
		}
		(void)close(s);
	}
	
	if(RET_ERROR == ret)
	{
		printf("Can't get the information about your passport, please check your passport!!\n");
		return RET_WARNING;
	}

	args.clientSeq=random();
	
	/*login*/
	s=get_connection(args.phServer, CONTROLPORT, &ptr);
	if(s == -1) {
		printf("%s: %s", ptr, ORAYDNSHOST);
		ret = RET_WARNING;
		sleep(60);
		/**/
		system("echo 2 > /var/ddns_status");
		goto LOGIN_RESTART;
	} else {
		ret = oray_login(s, &args);	
		(void)close(s);
		if(ORAY_QUITFAIL == ret)
			sleep(60);
		if(ORAY_LOGINSUCCESS != ret)
		{
			system("echo 3 > /var/ddns_status");
			goto LOGIN_RESTART;
		}
	}
	system("rm -f /var/ddns_status");
	/*Save info*/
	oray_saveInfo(&args);
	
	/*update*/
	s=get_udpConnection(args.phServer,UPDATEPORT,&ptr);
	if(s == -1) {
		printf("%s: %s", ptr, ORAYDNSHOST);
		ret = RET_WARNING;
	} else {
		while(1)
		{
			oray_udpRequest(s, &args,ORAY_CLIENT_UPDATE_OPCODE);
			ret=oray_udpRespone(s,&args,ORAY_SERVER_UPDATE_RESOPCODE);
			if(RET_RESTART==ret)
			{
				memset(args.domainName,0x0,ORAY_DOMAIN_NAME_MAX*sizeof(orayDomainName));
				args.servSeq=0;
				args.sessionNumber=0;
				goto LOGIN_RESTART;
			}
			else if(RET_WARNING == ret || RET_TIMEOUT == ret)
			{
				args.updatefailcnt++;
				printf("update failed once\n");
				if(args.updatefailcnt >= 3)
					goto LOGIN_RESTART;
			}
			else if(RET_OK == ret)
			{
				/*if update success. clear update failcnt*/
				args.updatefailcnt=0;
			}
			if(0 == args.userType)
			{
				sleep(60);
			}else if(1 == args.userType)
			{
				sleep(30);
			}
		}
	}		

	/*write-off*/
	oray_udpRequest(s, &args,ORAY_CLIENT_WRITEOFF_OPCODE);
	if(RET_OK ==oray_udpRespone(s, &args,ORAY_SERVER_WRITEOFF_RESOPCODE))
	{
		printf("Write off Successed\n");
	}
	system("rm -f /var/ddns_status");
	close(s);
	return ret;
}
