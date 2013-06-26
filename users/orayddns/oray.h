#ifndef __ORAY_H__
#define __ORAY_H__
#include <ret_codes.h>
#define ORAYDNSHOST "phservice2.oray.net"
//#define ORAYDNSHOST1 "phservice5.oray.net"
#define ORAYDNSHOST1 "hphwebservice.oray.net"
#define HTTPPORT 80
#define CONTROLPORT 6060
#define UPDATEPORT 6060

#define ORAY_ERROR		"<error>"
#define ORAY_MSG		"<message>"
#define ORAY_RESULTDATA		"<resultData>"
#define ORAY_ID		"<id>"
#define ORAY_PWD		"<pwd>"
#define ORAY_DOMAIN		"<domain>"
#define ORAY_HOST		"<host>"

#define ORAY_CLIENT_UPDATE_OPCODE 10
#define ORAY_SERVER_UPDATE_RESOPCODE 50
#define ORAY_SERVER_UPDATE_FAIL_OPCODE 1000
#define ORAY_CLIENT_WRITEOFF_OPCODE 11
#define ORAY_SERVER_WRITEOFF_RESOPCODE 51
#define ORAY_SERVER_WRITE_FAIL_OPCODE 1001

#define ORAY_DLINK_ID 9190
#define ORAY_CLIENTINFO	0x20063150
#define ORAY_CHALLENGEKEY 0x10101010
#define ORAY_UDP_TIMEOUT_SEC 10
#define ORAY_TCP_TIMEOUT_SEC 30

#define ORAY_CLIENT_VERSION "9.1.9.0"

#define BUFSIZE		2048

#define ORAY_HTTP_HEADER "POST /userinfo.asmx HTTP/1.1\r\n\
Host: phservice2.oray.net\r\n\
Content-Type: text/xml; charset=utf-8\r\n"

#define ORAY_HTTP_CONTENT_LENGTH "Content-Length: %d\r\n"

#define ORAY_HTTP_TAILER "SOAPAction: \"http://tempuri.org/GetMiscInfo\"\r\n\r\n"

#define ORAY_XML_HEADER "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n\
<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n\
<soap:Header>\r\n\
<AuthHeader xmlns=\"http://tempuri.org/\">\r\n"

#define ORAY_XML_BODY_ID "<ID>%d</ID>\r\n"

#define ORAY_XML_BODY_USER "<Account>%s</Account>\r\n"
#define ORAY_XML_BODY_PASSWORD_HEADER "<Password>"
#define ORAY_XML_BODY_PASSWORD_TAILER "</Password>\r\n"

#define ORAY_XML_TAILER "</AuthHeader>\r\n\
</soap:Header>\r\n\
<soap:Body>\r\n\
<GetMiscInfo xmlns=\"http://tempuri.org/\">\r\n\
<clientversion>%s</clientversion>\r\n\
</GetMiscInfo>\r\n\
</soap:Body>\r\n\
</soap:Envelope>\r\n"

#define MISCINFORESULTHEADER "<GetMiscInfoResult>"
#define MISCINFORESULTTAILER "</GetMiscInfoResult>"
#define CLIENTIPHEADER "<ClientIP>"
#define CLIENTIPTAILER "</ClientIP>"
#define PHSEVERHEADER "<PHServer>"
#define PHSEVERTAILER "</PHServer>"
#define USERTYPEHEADER "<UserType>"
#define USERTYPETAILER "</UserType>"

#define ORAY_DOMAIN_NAME_MAX 5

#define ORAY_DDNS_STATUS "/var/ddns_status"

 typedef  enum {
  	ORAY_INIT=0,
	ORAY_AUTH_SEND,
	ORAY_AUTH_RESP_SEND,
	ORAY_AUTH_SUCCESS,
	ORAY_REG_SEND,
	ORAY_REG_SUCCESS,
	ORAY_DISCONNECT,
	ORAY_CLOSED
 } ORAY_LOGIN_STAT;

typedef enum {
	ORAY_LOGINSUCCESS=0,
	ORAY_QUITFAIL,
	ORAY_LOGINFAIL,
	ORAY_UPDATE
} ORAY_STAT;

typedef struct _orayDomainName
{
		char name[64];
		char valid;
		char registered;
} orayDomainName,*orayDomainNamep;



typedef struct _oray_update_packet
{
	long sessionnumber;
	unsigned long opcode;
	unsigned long seq;
	long checksum;
	unsigned long padding;
}  orayUpdatePkt, *orayUpdatePktp;


typedef struct _oray_update_res_packet
{
	long sessionnumber;
	long opcode;
	long seq;
	long checksum;
	long padding;
	unsigned long clientIP;
} orayUpdateResPkt, *orayUpdateResPktp;

struct arguments {
	char user[64];
	char password[64];
	unsigned long id;
	char phServer[64];
	unsigned long clientIP;
	unsigned char userType;
	char challengeKey[64];
	int keylen;
	orayDomainName domainName[ORAY_DOMAIN_NAME_MAX];
	long sessionNumber;
	unsigned long initSeq;
	unsigned long clientSeq;
	unsigned long servSeq;
	int orayState;
	int updatefailcnt;
};
static int get_flags(struct arguments *args, int argc, char *argv[]);
#endif //__ORAY_H__
