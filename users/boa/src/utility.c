/*
 *      Utiltiy function to communicate with TCPIP stuffs
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: utility.c,v 1.18 2009/09/04 06:02:14 keith_huang Exp $
 *
 */

/*-- System inlcude files --*/
#include <stdio.h>
#include <stdarg.h> /* Keith add for tr069 --start */
#include <sys/sysinfo.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/ioctl.h>
//#include <net/if.h>
#include <linux/wireless.h>
#include <dirent.h>
#include <time.h>
/*-- Local include files --*/
#include "apmib.h"
#include "utility.h"

#ifdef __i386__
#define _LITTLE_ENDIAN_
#endif

/*-- Local constant definition --*/
#define _PATH_PROCNET_ROUTE	"/proc/net/route"
#define _PATH_PROCNET_DEV	"/proc/net/dev"
#define _PATH_RESOLV_CONF	"/etc/resolv.conf"

/* -- Below define MUST same as /linux2.4.18/drivers/net/rtl865x/eth865x.c */
#define RTL8651_IOCTL_GETWANLINKSTATUS 2000
#define RTL8651_IOCTL_GETLANLINKSTATUS 2102
#define RTL8651_IOCTL_GET_ETHER_EEE_STATE 2105
#define RTL8651_IOCTL_GET_ETHER_BYTES_COUNT 2106

/* Keep this in sync with /usr/src/linux/include/linux/route.h */
#define RTF_UP			0x0001          /* route usable                 */
#define RTF_GATEWAY		0x0002          /* destination is a gateway     */

#define READ_BUF_SIZE	50

/*-- Local routine declaration --*/
static int get_dev_fields(int type, char *bp, struct user_net_device_stats *pStats);
static char *get_name(char *name, char *p);

/***************************************************BT********/
#ifdef CONFIG_RTL_BT_CLIENT
static const char* const GCOMMANDS[] = {"GET","GETCLIENTSINFO","POST", "AUTH", "SETLIMITS", "GETLIMITS", "CLIENTQUIT", "CLIENTUPDATE", "CLIENTPAUSE", "GETTORRENTS", "TORRENTSTART", "TORRENTDELETE", "GETDETAILS", "SUBMITURL", "GETSTATS", "SETFILES", "GETTDETAILS", "SUBMITWDOWNLOAD", "GETWGETS", "STOPWGET", "STARTWGET", "GETWDETAILS", "WGETDELETE"};
int char_replace(char *str, char a, char b)
{
	int len=0;
	int i;
	if(NULL == str)
		return -1;
	len=strlen(str);
	for(i=0;i<len;i++)
	{
		if(str[i]==a)
			str[i]=b;
	}
	return 0;
}

int dirExits(char *path)
{
	DIR* dir = opendir(path);
	if (dir == NULL) {
		perror("opendir");
		return 0;
	}
	closedir(dir);
	return 1;
}
int readline(int s, char* buff, int buffsize)
{
	int read = 0;
	buff[0]=0;
	buffsize--;//terminating '\0'
	do {
		int r = recv(s, &buff[read], 1, 0);
		if (r == -1)
		{
			printf("error in readline (recv)\n");
			return -1;
		} else if (r == 0) {
			return read;
		}
		read+=r;
		buffsize--;
	} while( buff[read-1] != '\n' && buffsize != 0);
	buff[read] = 0;
	if(read > 1)
		return read;
	else
		return 0;
}

/* Format
  *"%s\t%u\t%u\t%u\t%llu\t%d\t\n", f->filename, f->fileno, f->n_have, f->n_pieces, f->filesize, f->download)
  *
  */
int bt_parsefile(char *buf,int size, struct ctfile_t  *file)
{
	int count=0;
	char *ptr;
	ptr=strtok(buf,"\t");
	while(ptr)
	{
		if(0==count)
			file->filename=strdup(ptr);
		else if(1==count)
			file->fileno=atoi(ptr);
		else if(2==count)
			file->n_have=atoi(ptr);
		else if(3==count)
			file->n_pieces=atoi(ptr);
		else if(4==count)
			file->filesize=atoll(ptr);
		else if(5==count)
			file->download=atoi(ptr);
		count++;
		ptr=strtok(NULL,"\t");
	}
	return 0;
}
/*Format
 * %s\t%s\t%u\t%u\t%llu\t%u\t%u\t%llu\t%llu\t%d\t%u\t%u\t%u\t%u\t%u\t%hd\t%u\t%u\t%llu\t\n",
 *     fname, cls[i].severity != -1 ? cls[i].msg : "", cls[i].seeders, cls[i].leechers, cls[i].size,
 *	cls[i].dl_rate, cls[i].ul_rate, cls[i].dl_total, cls[i].ul_total, i, cls[i].dl_limit, cls[i].ul_limit, cls[i].n_have, cls[i].n_total,
 *     cls[i].piece_size, cls[i].paused, cls[i].total_time, cls[i].seed_time, cls[i].dsize
 *
 */
int bt_parseCtorrent(char *buf, int size, struct ctorrent_t  *tp)
{
	int count=0;
	char *ptr;
	ptr=strtok(buf,"\t");
	while(ptr)
	{
		if(0==count)
			tp->fname=strdup(ptr);
		else if(1==count)
			tp->msg=strdup(ptr);
		else if(2==count)
			tp->seeders=atoi(ptr);
		else if(3==count)
			tp->leechers=atoi(ptr);
		else if(4==count)
			tp->size=atoll(ptr);
		else if(5==count)
			tp->dl_rate=atoi(ptr);
		else if(6==count)
			tp->ul_rate=atoi(ptr);
		else if(7==count)
			tp->dl_total=atoll(ptr);
		else if(8==count)
			tp->ul_total=atoll(ptr);
		else if(9==count)
			tp->index=atoi(ptr);
		else if(10==count)
			tp->dl_limit=atoi(ptr);		
		else if(11==count)
			tp->ul_limit=atoi(ptr);
		else if(12==count)
			tp->n_have=atoi(ptr);
		else if(13==count)
			tp->n_total=atoi(ptr);
		else if(14==count)
			tp->piece_size=atoi(ptr);
		else if(15==count)
			tp->paused=atoi(ptr);
		else if(16==count)
			tp->total_time=atoi(ptr);
		else if(17==count)
			tp->seed_time=atoi(ptr);
		else if(18==count)
			tp->dsize=atoll(ptr);
		count++;
		ptr=strtok(NULL,"\t");
	}
	return 0;
}

int parse_torrentDetails()
{
	return 0;
}
/*int len = asprintf(&msg, "%s\t%hd\t%d\t%d\t%u\t%u\t%u\t\n", fname,
  *				    trs[i].status, trs[i].ctorrent, i, trs[i].n_have, trs[i].n_total, trs[i].piece_size);
  *
  */
int parse_torrents(char *buf, struct torrent_t *torrentp)
{
	int count=0;
	char *ptr;
	ptr=strtok(buf,"\t");
	while(ptr)
	{
		if(0==count)
			torrentp->name=strdup(ptr);
		else if(1==count)
			torrentp->status=atoi(ptr);
		else if(2==count)
			torrentp->ctorrent=atoi(ptr);
		else if(3==count)
			torrentp->index=atoi(ptr);
		else if(4==count)
			torrentp->n_have=atoi(ptr);
		else if(5==count)
			torrentp->n_total=atoi(ptr);
		else if(6==count)
			torrentp->piece_size=atoi(ptr);
		count++;
		ptr=strtok(NULL,"\t");
	}
	return 0;
}
/*Create Socket*/
int bt_socket()
{
	int sock;
	int length;
	int ret;
	int reuse = 1;
	struct sockaddr_in addr;
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
	addr.sin_port=htons(SERVER_PORT);
	length=sizeof(addr);
	sock=socket(AF_INET,SOCK_STREAM,0);
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
	ret=connect(sock,&addr,length);
	if(ret <0)
	{
		printf("connect error\n");
	}
	else
	{
		return sock;
	}
	return 0;
}
void bt_get()
{
	char msg[512];
	int sock,nread;
	sock=bt_socket();
	write(sock,"GET",3);
	write(sock,"\n\n",2);
	while((nread=readline(sock,msg,sizeof(msg)-1))>0)
	{	
	}
	close(sock);
	return ;
}

int bt_getClientsInfo(struct ctorrent_t  *ctorrentp, int max)
{
	char buffer[1024];
	char msg[512];
	int count,nread;
	int sock;
	memset(msg,0x0,sizeof(msg));
	sock=bt_socket(sock,msg,strlen(msg));
	write(sock,"GETCLIENTSINFO",strlen("GETCLIENTSINFO"));
	write(sock,"\n\n",2);	
	count=0;
	while((nread=readline(sock,buffer,sizeof(buffer)))>0)
	{
		bt_parseCtorrent(buffer,nread,ctorrentp+count);
		count++;
		if(count >= max)
			break;
	}
	close(sock);
	return count;
}

/* Foramt: "SETLIMITS /ulimit/dlimit/writeconf/AC"
  *Return Format: OK or FAIL
  */
void bt_setLimits(int ulimit, int dlimit,int writeconf)
{
	int sock,nread;
	char cmd[128],buffer[256];
	sprintf(cmd,"SETLIMITS /%d/%d/%d",ulimit,dlimit,writeconf);
	write(sock,"\n\n",2);	
	sock=bt_socket();
	write(sock,cmd,strlen(cmd));
	/*if ok http ok will be returned*/
	while((nread=readline(sock,buffer,sizeof(buffer)))>0)
	{
		/**/
	}
	return;
}

/*Foramt: "GETLIMITS /AC"
  *Return Format: "ulimt/dlimit\n\n"
  */
void bt_getLimits(int *ulimitp, int *dlimitp)
{
	char buffer[32];
	int sock, nread;
	char *ptr;
	sock=bt_socket();
	write(sock,"GETLIMITS",strlen("GETLIMITS"));
	write(sock,"\n\n",2);
	while((nread=readline(sock,buffer,sizeof(buffer)))>0)
	{
		ptr=strtok(buffer,"\t");
		*ulimitp=atoi(ptr);
		ptr=strtok(NULL,"\t");
		*dlimitp=atoi(ptr);
	}
	close(sock);
	return;
}

/*Format: "CLIENTQUIT /CLINETNO/AC"	sscanf(request, "%*s /%d/%32s", &client, auth)
  *
  */
void bt_clientQuit(int index)
{
	int sock,nread;
	char buffer[512];
	char cmd[128];
	sprintf(cmd, "CLIENTQUIT /%d",index);
	sock=bt_socket();
	write(sock,cmd,strlen(cmd));
	write(sock,"\n\n",2);
	while((nread=readline(sock,buffer,sizeof(buffer)))>0)
	{
	
	}
	close(sock);
	return;
}

/*Format: "CLIENTUPDATE /CLIENTNO/AC"	sscanf(request, "%*s /%d/%32s", &client, auth);
  *
  */
void bt_clientUpdate(int index)
{
	int sock,nread;
	char cmd[128];
	char buffer[512];
	sock=bt_socket();
	sprintf(cmd, "CLIENTUPDATE /%d",index);
	write(sock,cmd,strlen(cmd));
	write(sock,"\n\n",2);
	while((nread=readline(sock,buffer,sizeof(buffer)))>0)
	{
	
	}
	close(sock);
	return;
}

/*Format:"CLIENTPAUSE /CLIENTNO/AC"  sscanf(request, "%*s /%d/%32s", &client, auth);
  *
  */

void bt_clientPause(int index)
{
	char cmd[32];
	char buffer[512];
	int sock,nread;
	sock=bt_socket();
	sprintf(cmd,"CLIENTPAUSE /%d",index);
	write(sock,cmd,strlen(cmd));
	write(sock,"\n\n",2);
	while((nread=readline(sock,buffer,sizeof(buffer)))>0)
	{
	}
	close(sock);
	return;
}
/*Format: "GETTORRENTS /AC" sscanf(request, "GETTORRENTS /%32s", ac);
  *
  */
int bt_getTorrents(struct torrent_t *torrentp, int max)
{
	int sock,nread;
	int count;
	char buffer[512];
	sock=bt_socket();
	write(sock,"GETTORRENTS",strlen("GETTORRENTS"));
	write(sock,"\n\n",2);	
	count=0;
	while((nread=readline(sock,buffer,sizeof(buffer)))>0)
	{
		parse_torrents(buffer,torrentp+count);
		count++;
		if(count >= max)
			break;
	}
	close(sock);
	return count;
}


void dumpfile(struct ctfile_t *file,int filecount)
{
        int i;
        for(i=0;i<filecount;i++)
        {
                printf("filename %s\n",file[i].filename);
                printf("filesize %lu\n",file[i].filesize);
                printf("file download %d\n",file[i].download);
 
        }
}

void dumpTorrents(struct torrent_t *torrentp, int count)
{
	int i;
	printf("dump Torrent\n");
	for(i=0;i<count;i++)
	{
		printf("%d: \n",i);
		printf("name %s\n",torrentp[i].name);
		printf("status %d\n",torrentp[i].status);
		printf("ctorrent %d\n",torrentp[i].ctorrent);
		printf("dfiles %s\n",torrentp[i].dfiles);
		printf("n_have %d\n",torrentp[i].n_have);
		printf("n_total %d\n",torrentp[i].n_total);
		printf("piece_size %d\n",torrentp[i].piece_size);
		printf("index %d\n",torrentp[i].index);
	}
}

void dumpClients(struct ctorrent_t *ctorrentp, int count)
{
	int i;
	printf("dump clients\n");
	for(i=0;i<count;i++)
	{
		printf("%d: \n",i);
		printf("fname %s",ctorrentp[i].fname);
	}
}

/*Format "TORRENTSTART /torrent.no/AC" sscanf(request, "%*s /%d/%32s", &torrent, auth);
  *
  */
void bt_startTorrent(int index)
{
	char cmd[32];
	char buffer[512];
	int sock,nread;
	sock=bt_socket();
	sprintf(cmd,"TORRENTSTART /%d",index);
	write(sock,cmd,strlen(cmd));
	write(sock,"\n\n",2);	
	while((nread=readline(sock,buffer,sizeof(buffer)))>0)
	{
	
	}
	close(sock);
	return;
}

/*Format "TORRENTDELETE /Torrent ID/DelData/AC" 	sscanf(request, "%*s /%d/%d/%32s", &torrent, &del, auth);
  *
  */
void bt_deleteTorrent(int index, int delData)
{
	char cmd[32];
	char buffer[512];
	int sock,nread;
	sock=bt_socket();
	sprintf(cmd,"TORRENTDELETE /%d/%d",index,delData);
	write(sock,cmd,strlen(cmd));
	write(sock,"\n\n",2);	
	while((nread=readline(sock,buffer,sizeof(buffer)))>0)
	{
	
	}
	close(sock);
	return;
}

/*Format "GETDETAILS /CTorrent Index/AC" sscanf(request, "GETDETAILS /%d/%32s", &ci, ac);
  *Format Return: see bt_parsefile
  */
int  bt_getDetails(int index, struct ctfile_t *file, int max)
{
	char cmd[32];
	char msg[512];
	int sock, count, nread;
	sock=bt_socket();
	sprintf(cmd,"GETDETAILS /%d",index);
	write(sock,cmd,strlen(cmd));
	write(sock,"\n\n",2);	
	count=0;
	while((nread=readline(sock,msg,sizeof(msg)-1))>0)
	{
		bt_parsefile(msg,nread,file+count);
		count++;
		if(count >= max)
			break;
	}
	close(sock);
	return count;
}
/*Format "GETSTATS /AC"	sscanf(request, "GETSTATS /%32s", ac);
  *Return Format:"%llu\t%llu\t%u\t%u\t%llu\t%llu\t\n", stats.up, stats.down, stats.totaltime,
  *			stats.sumtorrents, dsize, dfree);
  */
void bt_getStats(struct stats_t *statusp)
{
	int sock,nread;
	char *ptr;
	char buffer[128];
	int count;
	sock=bt_socket();
	write(sock,"GETSTATS",strlen("GETSTATS"));
	write(sock,"\n\n",2);	
	count=0;
	while((nread=readline(sock,buffer,sizeof(buffer))) > 0)
	{
		/*parse status*/
		ptr=strtok(buffer,"\t");
		while(ptr)
		{
			if(0==count)
			{
				statusp->up=atoll(ptr);
			}
			else if(1==count)
			{
				statusp->down=atoll(ptr);
			}
			else if(2==count)
			{
				statusp->totaltime=atoi(ptr);
			}
			else if(3==count)
			{
				statusp->sumtorrents=atoi(ptr);
			}
			else if(4==count)
			{
				statusp->dsize=atoll(ptr);
			}
			else if(5==count)
			{
				statusp->dfree=atoll(ptr);
			}
			count++;
			ptr=strtok(NULL,"\t");
		}
	}
	close(sock);
	return;
}
/*Format: SETFILES /AC/CLIENT ID/LEN/fileno/fileno/fileno/filen0
  *
  */
void bt_setfiles(int clientindex, int len, char *indexstr)
{
	int sock,nread;
	int i;
	char cmd[256];
	char buffer[512];
	sock=bt_socket();
	sprintf(cmd,"SETFILES /%d/%d",clientindex,len);
	strcat(cmd,indexstr);
	write(sock,cmd,strlen(cmd));
	write(sock,"\n\n",2);
	while((nread=readline(sock,buffer,sizeof(buffer))) > 0)
	{
	}
	close(sock);
	return;
}

/*Foramt: "GETTDETAILS /AC/length/Torrent.no/Torrent.no/....." sscanf(request, "GETTDETAILS /%32s/", ac);
  *Return Format: refer to Source Code
  */
void bt_getTDetails(char *ac,int torrent[], int size)
{
	int sock,nread,i;
	char buffer[512];
	char cmd[512];
	sock=bt_socket();
	sprintf(cmd,"GETTDETAILS /%d",size);
	for(i=0;i++;i<size)
	{
		sprintf(cmd+strlen(cmd),"/%d",torrent[i]);
	}
	write(sock,cmd,strlen(cmd));
	write(sock,"\n\n",2);
	while((nread=readline(sock,buffer,sizeof(buffer))) > 0)
	{
		parse_torrentDetails();
	}
	close(sock);
	return;
}

int bt_saveTorrentfile(char *filepath, unsigned char * buffer, int len)
{
	FILE *fp;
	fp=fopen(filepath,"w");
	if(NULL == fp)
	{
		printf("Can not create bt file\n");
		return -1;
	}
	fwrite(buffer,len,1,fp);
	fclose(fp);
	return 0;
}

#endif
/***************************************************BT end****/
/*------------------------------------------------------------------*/
/*
 * Wrapper to extract some Wireless Parameter out of the driver
 */
static inline int
iw_get_ext(int                  skfd,           /* Socket to the kernel */
           char *               ifname,         /* Device name */
           int                  request,        /* WE ID */
           struct iwreq *       pwrq)           /* Fixed part of the request */
{
  /* Set device name */
  strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
  /* Do the request */
  return(ioctl(skfd, request, pwrq));
}


/////////////////////////////////////////////////////////////////////////////
int getWlStaNum( char *interface, int *num )
{
#ifndef NO_ACTION
    int skfd=0;
    unsigned short staNum;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );
      return -1;
	}
    wrq.u.data.pointer = (caddr_t)&staNum;
    wrq.u.data.length = sizeof(staNum);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLSTANUM, &wrq) < 0){
    	 close( skfd );
	return -1;
	}
    *num  = (int)staNum;

    close( skfd );
#else
    *num = 0 ;
#endif

    return 0;
}
#ifdef CONFIG_RTL_WAPI_SUPPORT
/*
*   parameters:
*   certsInfo (output): to store user cert information get from from CERTS_DATABASE
*   count (input): number of entries in CERTS_DATABASE
*
*   return 0: success; return -1: failed
*/
#define SUCCESS 0
#define FAILED -1
int getCertsDb(CERTS_DB_ENTRY_Tp certsInfo, int count, int *realcount)
{
        FILE *fp;
        time_t  now, expired_tm;
        struct tm *tnow;
        struct tm tm_time;
 
        struct stat status;
        int readSize;
        int ret, toRet;
        int i,intVal;
        long longVal;
        char *p1, *p2, *ptr;
 
        char buffer[100];
        char tmpBuf[100];//Added for test
        char tmpBuf2[3];//Added for test
 
        if ( stat(CERTS_DATABASE, &status) < 0)
        {
                printf("%s(%d): %s not exist.\n",__FUNCTION__,__LINE__, CERTS_DATABASE);//Added for test
                toRet=FAILED;
                goto err;
        }
//        printf("%s(%d)\n",__FUNCTION__,__LINE__);//Added for tes
        fp = fopen(CERTS_DATABASE, "r");
        if (!fp) {
                printf("open %s error.\n", CERTS_DATABASE);//Added for test
                toRet=FAILED;
                goto err;
        }
 
        p1=NULL;
        p2=NULL;
        for(i=0;i<count;i++)
        {
        	   *realcount=i;
                if(!fgets(buffer, sizeof(buffer), fp))
                {
//			printf("%s(%d): file end.\n",__FUNCTION__,__LINE__);//Added for test
                        toRet=SUCCESS;
                        goto err;
                }
//                printf("%s(%d),i=%d,buffer=%s, len=%d\n",__FUNCTION__,__LINE__,i,buffer, strlen(buffer));//Added for test
 
                //To set cert type, 0: X.509 (only at preset)
                certsInfo[i].certType=0;
 
                //dumpHex(buffer, strlen(buffer)+1);
                if(buffer[0]=='E')
                {
                        //Expired
                        certsInfo[i].certStatus=1;
                }
                else if(buffer[0]=='R')
                {
                        //Revoked
                        certsInfo[i].certStatus=2;
                }
                else
                {
                        //Valid
                        certsInfo[i].certStatus=0;
                }
                //printf("%s(%d): certsInfo[i].certStatus=%d.\n",__FUNCTION__,__LINE__, certsInfo[i].certStatus);//Added for test
 
                //To parse exipred time
                p1=strchr(buffer,'\t');
                if(p1==NULL)
                {
                        printf("%s(%d): strchr failed.\n",__FUNCTION__,__LINE__);//Added for test
                        toRet=FAILED;
                        goto err;
                }
                p1++;
                p2=strchr(p1,'\t');
                if(p2==NULL)
                {
                        printf("%s(%d): strchr failed.\n",__FUNCTION__,__LINE__);//Added for test
                        toRet=FAILED;
                        goto err;
                }
                if(p2>p1)
                {
                        memset(tmpBuf, 0, sizeof(tmpBuf));
                        //memset(tmpBuf2, 0, sizeof(tmpBuf2));
                        strncpy(tmpBuf, p1, p2-p1);
                        //printf("%s(%d): tmpBuf=%s.\n",__FUNCTION__,__LINE__, tmpBuf);//Added for test
                        memset(&tm_time, 0 , sizeof(tm_time));
 
                        //?????
                        //tm_time.tm_isdst=-1;

                        //To get year value
                        memset(tmpBuf2, 0, sizeof(tmpBuf2));
                        strncpy(tmpBuf2, tmpBuf, 2);
                        intVal=atoi(tmpBuf2);
                        if(intVal>=70)
                        {
                                //year: 1970 ~ 1999
                                //year - 1900
                                tm_time.tm_year=(intVal+1900)-1900;
                        }
                        else
                        {
                                //year: 2000 ~ 2069
                                //year - 1900
                                tm_time.tm_year=(intVal+2000)-1900;
                        }
 
                        //To get month value
                        memset(tmpBuf2, 0, sizeof(tmpBuf2));
                        strncpy(tmpBuf2, &tmpBuf[2], 2);
                        tm_time.tm_mon=atoi(tmpBuf2)-1;
 
                        //To get day value
                        memset(tmpBuf2, 0, sizeof(tmpBuf2));
                        strncpy(tmpBuf2, &tmpBuf[4], 2);
                        tm_time.tm_mday=atoi(tmpBuf2);
 
                        //To get hour value
                        memset(tmpBuf2, 0, sizeof(tmpBuf2));
                        strncpy(tmpBuf2, &tmpBuf[6], 2);
                        tm_time.tm_hour=atoi(tmpBuf2);
 
                        //To get minute value
                        memset(tmpBuf2, 0, sizeof(tmpBuf2));
                        strncpy(tmpBuf2, &tmpBuf[8], 2);
                        tm_time.tm_min=atoi(tmpBuf2);
 
                        //To get second value
                        memset(tmpBuf2, 0, sizeof(tmpBuf2));
                        strncpy(tmpBuf2, &tmpBuf[10], 2);
                        tm_time.tm_sec=atoi(tmpBuf2);
 
                //      printf("(0): %d %d %d %d %d %d, tm_isdst=%d\n", 1900+tm_time.tm_year,tm_time.tm_mon+1,tm_time.tm_mday,tm_time.tm_hour,tm_time.tm_min,tm_time.tm_sec, tm_time.tm_isdst);//Added for test
 
                        expired_tm = mktime(&tm_time);
                        if(expired_tm < 0){
                                printf("Error:set Time Error for tm!\n");//Added for test
                                toRet=FAILED;
                                goto err;
                        }
                //      printf("%s(%d): expired_tm=%ld.\n",__FUNCTION__,__LINE__,expired_tm);//Added for test
 
 
                        now=time(0);
                        tnow=localtime(&now);
                //      printf("now=%ld, %d %d %d %d %d %d, tm_isdst=%d\n",now, 1900+tnow->tm_year,tnow->tm_mon+1,tnow->tm_mday,tnow->tm_hour,tnow->tm_min,tnow->tm_sec, tnow->tm_isdst);//Added for test
 
                        longVal=difftime(expired_tm,now);
//                        printf("The difference is: %ld seconds\n",longVal);
                        if(longVal<=0)
                                certsInfo[i].validDaysLeft=0;
                        else
                                certsInfo[i].validDaysLeft=(unsigned short)(longVal/ONE_DAY_SECONDS)+1;
 
                //      printf("%s(%d): certsInfo[%d].validDaysLeft=%d.\n",__FUNCTION__,__LINE__,i, certsInfo[i].validDaysLeft);//Added for test
 
//                      printf("%s(%d): tmpBuf2=%s.\n",__FUNCTION__,__LINE__, tmpBuf2);//Added for test
                }
 
                //To parse revoked time(Not used now)
                p1=p2;
                p1++;
                p2=strchr(p1,'\t');
                if(p2==NULL)
                {
                        printf("%s(%d): strchr failed.\n",__FUNCTION__,__LINE__);//Added for test
                        toRet=FAILED;
                        goto err;
                }
                if(p2>p1)
                {
                        memset(tmpBuf, 0, sizeof(tmpBuf));
                        strncpy(tmpBuf, p1, p2-p1);
//                     printf("%s(%d): tmpBuf=%s.\n",__FUNCTION__,__LINE__, tmpBuf);//Added for test
                }
 
                //To parse serial
                p1=p2;
                p1++;
                p2=strchr(p1,'\t');
                if(p2==NULL)
                {
                        printf("%s(%d): strchr failed.\n",__FUNCTION__,__LINE__);//Added for test
                        toRet=FAILED;
                        goto err;
                }
                if(p2>p1)
                {
                        memset(tmpBuf, 0, sizeof(tmpBuf));
                        strncpy(tmpBuf, p1, p2-p1);
//                     printf("%s(%d):serial tmpBuf=%s.\n",__FUNCTION__,__LINE__, tmpBuf);//Added for test
			    certsInfo[i].serial=strtol(tmpBuf, (char **)NULL,16);
//                     ret=str2hex(tmpBuf, &certsInfo[i].serial);
//                     printf("%s(%d), ret=%d, certsInfo[%d].serial=0x%x\n",__FUNCTION__,__LINE__, ret, i,certsInfo[i].serial);//Added for test
#if 0
                        if(ret==FAILED)
                        {
                                printf("%s(%d), str2hex failed.\n",__FUNCTION__,__LINE__);//Added for test
                                toRet=FAILED;
                                goto err;
                        }
#endif						
                }
                //To parse total valid days
                p1=p2;
                p1++;
                p2=strchr(p1,'\t');
                if(p2==NULL)
                {
                        printf("%s(%d): strchr failed.\n",__FUNCTION__,__LINE__);//Added for test
                        toRet=FAILED;
                        goto err;
                }
                if(p2>p1)
                {
                        memset(tmpBuf, 0, sizeof(tmpBuf));
                        strncpy(tmpBuf, p1, p2-p1);
                        //printf("%s(%d):total valid days tmpBuf=%s.\n",__FUNCTION__,__LINE__, tmpBuf);//Added for test
                        certsInfo[i].validDays=(unsigned short)atoi(tmpBuf);
                        //printf("%s(%d):certsInfo[%d].validDays=%d.\n",__FUNCTION__,__LINE__, i, certsInfo[i].validDays);//Added for test
                }

		if((certsInfo[i].validDaysLeft>certsInfo[i].validDays)
			||((certsInfo[i].certStatus==1)&&certsInfo[i].validDaysLeft>0))
		{
			//printf("%s(%d), warning: system time setting is not correct.\n",__FUNCTION__,__LINE__);//Added for test
			//To indicate our system hasn't sync time yet
			sprintf(tmpBuf,"echo \"1\" > %s", SYS_TIME_NOT_SYNC_CA);
			system(tmpBuf);
			//End indication
			certsInfo[i].validDaysLeft=0;
		}
 
                //To parse user name
                p1=p2;
                p1++;
                p2=strchr(p1,'\n');
                if(p2==NULL)
                {
                        printf("%s(%d): strchr failed.\n",__FUNCTION__,__LINE__);//Added for test
                        toRet=FAILED;
                        goto err;
                }
                if(p2>p1)
                {
                        memset(tmpBuf, 0, sizeof(tmpBuf));
                        strncpy(tmpBuf, p1, p2-p1);
                        //printf("%s(%d):user name tmpBuf=%s.\n",__FUNCTION__,__LINE__, tmpBuf);//Added for test
                        ptr=NULL;
                        ptr=strstr(p1, "CN=");
                        if(p2==NULL)
                        {
                                printf("%s(%d): strstr failed.\n",__FUNCTION__,__LINE__);//Added for test
                                toRet=FAILED;
                                goto err;
                        }
                        ptr+=3;//Point to user name
                        memset(certsInfo[i].userName, 0, sizeof(certsInfo[i].userName));
                        strncpy(certsInfo[i].userName, ptr, p2-ptr);
                        //printf("%s(%d):certsInfo[%d].userName=%s.\n",__FUNCTION__,__LINE__, i, certsInfo[i].userName);//Added for test
                }
 
//              p1=buffer;
//              p2=strstr(p1,"\t");
//              printf("%s(%d),i=%d,p2=%s, len=%d\n",__FUNCTION__,__LINE__,i,buffer, strlen(p2));//Added for test
        }
 
        toRet=SUCCESS;
 
err:
        if(fp!=NULL)
                fclose(fp);
//        printf("%s(%d), toRet=%d\n",__FUNCTION__,__LINE__,toRet);//Added for tes
        return toRet;
}
static int searchCertStatus(CERTS_DB_ENTRY_Tp all, CERTS_DB_ENTRY_Tp cert,int status, int count)
{
	int i=0;
	int cnt=0;
	for(i=0;i<count;i++)
	{
		if(all[i].certStatus == status)
		{
			memcpy(cert+cnt,all+i,sizeof(CERTS_DB_ENTRY_T));
			cnt++;
		}
	}
	return cnt;
}

static int searchCertName(CERTS_DB_ENTRY_Tp all, CERTS_DB_ENTRY_Tp cert,char *buffer, int count)
{
	int i=0;
	int cnt=0;
	for(i=0;i<count;i++)
	{
		if(!strcmp(all[i].userName, buffer))
		{
			memcpy(cert+cnt,all+i,sizeof(CERTS_DB_ENTRY_T));
			cnt++;
		}
	}
	return cnt;
}
static int searchCertSerial(CERTS_DB_ENTRY_Tp  all, CERTS_DB_ENTRY_Tp cert, unsigned long serial, int count)
{
	int i=0;
	int cnt=0;
	for(i=0;i<count;i++)
	{
		if(all[i].serial==serial)
		{
			memcpy(cert+cnt,all+i,sizeof(CERTS_DB_ENTRY_T));
			cnt++;
		}
	}
	return cnt;
}

int searchWapiCert(CERTS_DB_ENTRY_Tp cert, int index, char *buffer)
{
	int status=0;
	int all=0;
	int count=0,searchCnt=0;
	int retVal;
	unsigned long serial=0;
	CERTS_DB_ENTRY_Tp allCert=(CERTS_DB_ENTRY_Tp)malloc(sizeof(CERTS_DB_ENTRY_T)*128);
	retVal=getCertsDb(allCert,128,&count);
	if(retVal < 0)
	{
		return 0;
	}
	if(count == 0)
	{
		free(allCert);
		return count;
	}
	switch (index)
	{
		case 5:
			/*0 actived, 2 revoked*/
			status=buffer[0]-'0';
			searchCnt=searchCertStatus(allCert,cert,status,count);
			break;
		case 4:
			/*now only support x.509*/
			all=1;
			break;
		case 3:
			/*username is in buffer*/
			searchCnt=searchCertName(allCert,cert,buffer,count);
			break;
		case 2:
			/*serail  in buffer in ASCII*/
			serial=strtol(buffer,(char **)NULL,16);
			searchCnt=searchCertSerial(allCert,cert,serial,count);
			break;
		case 1:		
			/*All*/
			all=1;
			break;
		default:
			/*all*/
			all=1;
			break;
	}
	if(all)
	{
		memcpy(cert,allCert,count*sizeof(CERTS_DB_ENTRY_T));
		searchCnt=count;
	}
	free(allCert);
	return searchCnt;
}
#endif
/////////////////////////////////////////////////////////////////////////////
int getWlStaInfo( char *interface,  WLAN_STA_INFO_Tp pInfo )
{
#ifndef NO_ACTION
    int skfd=0;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );
        return -1;
	}
    wrq.u.data.pointer = (caddr_t)pInfo;
    wrq.u.data.length = sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1);
    memset(pInfo, 0, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1));

    if (iw_get_ext(skfd, interface, SIOCGIWRTLSTAINFO, &wrq) < 0){
    	close( skfd );
		return -1;
	}
    close( skfd );
#else
    return -1;
#endif
    return 0;
}

// ==== inserted by GANTOE for site survey 2008/12/26 ==== 
#ifdef CONFIG_RTK_MESH 
int setWlJoinMesh (char *interface, unsigned char* MeshId, int MeshId_Len, int Channel, int Reset) 
{ 
	int skfd; 
	struct iwreq wrq; 
	struct 
	{
		unsigned char *meshid;
		int meshid_len, channel, reset; 
	}mesh_identifier; 
  
	skfd = socket(AF_INET, SOCK_DGRAM, 0); 
    
	/* Get wireless name */ 
	if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) 
	/* If no wireless name : no wireless extensions */ 
		return -1; 

	mesh_identifier.meshid = MeshId; 
	mesh_identifier.meshid_len = MeshId_Len; 
	mesh_identifier.channel = Channel;
	mesh_identifier.reset = Reset;
	wrq.u.data.pointer = (caddr_t)&mesh_identifier; 
	wrq.u.data.length = sizeof(mesh_identifier); 

	if (iw_get_ext(skfd, interface, SIOCJOINMESH, &wrq) < 0) 
		return -1; 

	close( skfd ); 

	return 0; 
} 

// This function might be removed when the mesh peerlink precedure has been completed
int getWlMeshLink (char *interface, unsigned char* MacAddr, int MacAddr_Len) 
{ 
	int skfd; 
	struct iwreq wrq; 
  
	skfd = socket(AF_INET, SOCK_DGRAM, 0); 
    
	/* Get wireless name */ 
	if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) 
	/* If no wireless name : no wireless extensions */ 
		return -1; 

	wrq.u.data.pointer = (caddr_t)MacAddr; 
	wrq.u.data.length = MacAddr_Len; 

	if (iw_get_ext(skfd, interface, SIOCCHECKMESHLINK, &wrq) < 0) 
		return -1; 

	close( skfd ); 

	return 0; 
} 

// This function might be removed when the mesh peerlink precedure has been completed
int getWlMib (char *interface, unsigned char* Oid, int Oid_Len) 
{ 
	int skfd, ret = -1; 
	struct iwreq wrq; 
  
	skfd = socket(AF_INET, SOCK_DGRAM, 0); 
    
	/* Get wireless name */ 
	if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0) 
	/* If no wireless name : no wireless extensions */ 
		return -1; 

	wrq.u.data.pointer = (caddr_t)Oid; 
	wrq.u.data.length = Oid_Len; 
	ret = iw_get_ext(skfd, interface, RTL8190_IOCTL_GET_MIB, &wrq);

	close( skfd ); 
	if(ret < 0)
		return -1;
	return ret; 
} 
#endif 
// ==== GANTOE ==== 
/////////////////////////////////////////////////////////////////////////////
int getWlSiteSurveyResult(char *interface, SS_STATUS_Tp pStatus )
{
#ifndef NO_ACTION
    int skfd=0;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );
        return -1;
	}
    wrq.u.data.pointer = (caddr_t)pStatus;

    if ( pStatus->number == 0 )
    	wrq.u.data.length = sizeof(SS_STATUS_T);
    else
        wrq.u.data.length = sizeof(pStatus->number);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLGETBSSDB, &wrq) < 0){
    	close( skfd );
	return -1;
	}
    close( skfd );
#else
	return -1 ;
#endif

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
int getWlJoinRequest(char *interface, pBssDscr pBss, unsigned char *res)
{
#ifndef NO_ACTION
    int skfd=0;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0)
      /* If no wireless name : no wireless extensions */
        return -1;

    wrq.u.data.pointer = (caddr_t)pBss;
    wrq.u.data.length = sizeof(BssDscr);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLJOINREQ, &wrq) < 0)
	return -1;

    close( skfd );
    
    *res = *(unsigned char *)&wrq.u.data.pointer[0];
#else
    return -1;
#endif

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
int getWlJoinResult(char *interface, unsigned char *res)
{
    int skfd;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );
        return -1;
	}
    wrq.u.data.pointer = (caddr_t)res;
    wrq.u.data.length = 1;

    if (iw_get_ext(skfd, interface, SIOCGIWRTLJOINREQSTATUS, &wrq) < 0){
    	close( skfd );
	return -1;
	}
    close( skfd );

    return 0;
}



/////////////////////////////////////////////////////////////////////////////
int getWlSiteSurveyRequest(char *interface, int *pStatus)
{
#ifndef NO_ACTION
    int skfd=0;
    struct iwreq wrq;
    unsigned char result;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
      /* If no wireless name : no wireless extensions */
      close( skfd );
        return -1;
	}
    wrq.u.data.pointer = (caddr_t)&result;
    wrq.u.data.length = sizeof(result);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLSCANREQ, &wrq) < 0){
    	close( skfd );
	return -1;
	}
    close( skfd );

    if ( result == 0xff )
    	*pStatus = -1;
    else
	*pStatus = (int) result;
#else
	*pStatus = -1;
#endif
#ifdef CONFIG_RTK_MESH 
	// ==== modified by GANTOE for site survey 2008/12/26 ==== 
	return (int)*(char*)wrq.u.data.pointer; 
#else
	return 0;
#endif
}

/////////////////////////////////////////////////////////////////////////////
int getWlBssInfo(char *interface, bss_info *pInfo)
{
#ifndef NO_ACTION
    int skfd=0;
    struct iwreq wrq;



    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0)
#ifdef VOIP_SUPPORT 
	{
		// rock: avoid status page error if no wlan interface
		memset(pInfo, 0, sizeof(bss_info));
        return 0;
	}
#else
      /* If no wireless name : no wireless extensions */
      {
      	 close( skfd );
        return -1;
      }
#endif

    wrq.u.data.pointer = (caddr_t)pInfo;
    wrq.u.data.length = sizeof(bss_info);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLGETBSSINFO, &wrq) < 0){
    	 close( skfd );
	return -1;
	}
    close( skfd );
#else
    memset(pInfo, 0, sizeof(bss_info)); 
#endif

    return 0;
}



/////////////////////////////////////////////////////////////////////////////
int getInAddr( char *interface, ADDR_T type, void *pAddr )
{
    struct ifreq ifr;
    int skfd=0, found=0;
    struct sockaddr_in *addr;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return 0;
		
    strcpy(ifr.ifr_name, interface);
    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0){
    	close( skfd );
		return (0);
	}
    if (type == HW_ADDR) {
    	if (ioctl(skfd, SIOCGIFHWADDR, &ifr) >= 0) {
		memcpy(pAddr, &ifr.ifr_hwaddr, sizeof(struct sockaddr));
		found = 1;
	}
    }
    else if (type == IP_ADDR) {
	if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0) {
		addr = ((struct sockaddr_in *)&ifr.ifr_addr);
		*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);
		found = 1;
	}
    }
    else if (type == SUBNET_MASK) {
	if (ioctl(skfd, SIOCGIFNETMASK, &ifr) >= 0) {
		addr = ((struct sockaddr_in *)&ifr.ifr_addr);
		*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);
		found = 1;
	}
    }
    close( skfd );
    return found;

}

/////////////////////////////////////////////////////////////////////////////
extern pid_t find_pid_by_name( char* pidName)
{
	DIR *dir;
	struct dirent *next;

	dir = opendir("/proc");
	if (!dir) {
		printf("Cannot open /proc");
		return 0;
	}

	while ((next = readdir(dir)) != NULL) {
		FILE *status;
		char filename[READ_BUF_SIZE];
		char buffer[READ_BUF_SIZE];
		char name[READ_BUF_SIZE];

		/* Must skip ".." since that is outside /proc */
		if (strcmp(next->d_name, "..") == 0)
			continue;

		/* If it isn't a number, we don't want it */
		if (!isdigit(*next->d_name))
			continue;

		sprintf(filename, "/proc/%s/status", next->d_name);
		if (! (status = fopen(filename, "r")) ) {
			continue;
		}
		if (fgets(buffer, READ_BUF_SIZE-1, status) == NULL) {
			fclose(status);
			continue;
		}
		fclose(status);

		/* Buffer should contain a string like "Name:   binary_name" */
		sscanf(buffer, "%*s %s", name);
		if (strcmp(name, pidName) == 0) {
		//	pidList=xrealloc( pidList, sizeof(pid_t) * (i+2));
			return((pid_t)strtol(next->d_name, NULL, 0));

		}
	}
	if ( strcmp(pidName, "init")==0)
		return 1;

	return 0;
}


#if 0
/////////////////////////////////////////////////////////////////////////////
int setInAddr(char *interface, ADDR_T type, void *addr)
{
	struct ifreq ifr;
	struct sockaddr_in sin;
	int sock, cmd, ret;

	if ( type == IP_ADDR )
		cmd = SIOCSIFADDR;
	else if ( type == SUBNET_MASK )
		cmd = SIOCSIFNETMASK;
	else
		return -1;

	memset(&ifr, 0, sizeof(struct ifreq));

	sock = socket(PF_INET, SOCK_DGRAM, 0);
	strcpy(ifr.ifr_name, interface);
	memset(&sin, 0, sizeof(struct sockaddr));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = *((unsigned long *)addr);
	memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));
	if ( (ret=ioctl(sock, cmd, &ifr)) < 0)
		goto set_exit;

	// up interface
	strcpy(ifr.ifr_name, interface);
	if ((ret = ioctl(sock, SIOCGIFFLAGS, &ifr)) < 0)
		goto set_exit;

	strcpy(ifr.ifr_name, interface);
	ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
	ret = ioctl(sock, SIOCSIFFLAGS, &ifr);

set_exit:
	close(sock);

	return ret;
}
#endif

/////////////////////////////////////////////////////////////////////////////
int getDefaultRoute(char *interface, struct in_addr *route)
{
	char buff[1024], iface[16];
	char gate_addr[128], net_addr[128], mask_addr[128];
	int num, iflags, metric, refcnt, use, mss, window, irtt;
	FILE *fp = fopen(_PATH_PROCNET_ROUTE, "r");
	char *fmt;
	int found=0;
	unsigned long addr;

	if (!fp) {
       		printf("Open %s file error.\n", _PATH_PROCNET_ROUTE);
		return 0;
    	}

	fmt = "%16s %128s %128s %X %d %d %d %128s %d %d %d";

	while (fgets(buff, 1023, fp)) {
		num = sscanf(buff, fmt, iface, net_addr, gate_addr,
		     		&iflags, &refcnt, &use, &metric, mask_addr, &mss, &window, &irtt);
		if (num < 10 || !(iflags & RTF_UP) || !(iflags & RTF_GATEWAY) || strcmp(iface, interface))
	    		continue;
		sscanf(gate_addr, "%lx", &addr );
		*route = *((struct in_addr *)&addr);

		found = 1;
		break;
	}

    	fclose(fp);
    	return found;
}

#if 0
/////////////////////////////////////////////////////////////////////////////
int deleteDefaultRoute(char *dev, void *route)
{
	struct rtentry rt;
	int skfd;
	struct sockaddr_in *pAddr;

	/* Clean out the RTREQ structure. */
	memset((char *) &rt, 0, sizeof(struct rtentry));

	/* Fill in the other fields. */
 	rt.rt_flags = RTF_UP | RTF_GATEWAY;
	rt.rt_dev = dev;
	pAddr= (struct sockaddr_in *)&rt.rt_dst;
	pAddr->sin_family = AF_INET;

	pAddr= (struct sockaddr_in *)&rt.rt_gateway;
	pAddr->sin_family = AF_INET;
	pAddr->sin_addr.s_addr = *((unsigned long *)route);

	/* Create a socket to the INET kernel. */
	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("Create socket error");
		return -1;
    	}

	if (ioctl(skfd, SIOCDELRT, &rt) < 0) {
		printf("Delete route [SIOCDELRT] error!");
		close(skfd);
		return -1;;
	}

	/* Close the socket. */
 	close(skfd);
    	return (0);
}

/////////////////////////////////////////////////////////////////////////////
int addDefaultRoute(char *dev, void *route)
{
	struct rtentry rt;
	int skfd;
	struct sockaddr_in *pAddr;

	/* Clean out the RTREQ structure. */
	memset((char *) &rt, 0, sizeof(struct rtentry));

	/* Fill in the other fields. */
 	rt.rt_flags = RTF_UP | RTF_GATEWAY;
	rt.rt_dev = dev;
	pAddr= (struct sockaddr_in *)&rt.rt_dst;
	pAddr->sin_family = AF_INET;

	pAddr= (struct sockaddr_in *)&rt.rt_gateway;
	pAddr->sin_family = AF_INET;
	pAddr->sin_addr.s_addr = *((unsigned long *)route);

	/* Create a socket to the INET kernel. */
	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("Create socket error");
		return -1;
    	}

	if (ioctl(skfd, SIOCADDRT, &rt) < 0) {
		printf("Add route [SIOCADDRT] error!");
		close(skfd);
		return -1;
	}

	/* Close the socket. */
 	close(skfd);
    	return (0);
}
#endif

/////////////////////////////////////////////////////////////////////////////
int getStats(char *interface, struct user_net_device_stats *pStats)
{
 	FILE *fh;
  	char buf[512];
	int type;

	fh = fopen(_PATH_PROCNET_DEV, "r");
	if (!fh) {
		printf("Warning: cannot open %s\n",_PATH_PROCNET_DEV);
		return -1;
	}
	fgets(buf, sizeof buf, fh);	/* eat line */
	fgets(buf, sizeof buf, fh);

  	if (strstr(buf, "compressed"))
		type = 3;
	else if (strstr(buf, "bytes"))
		type = 2;
	else
		type = 1;

	while (fgets(buf, sizeof buf, fh)) {
		char *s, name[40];
		s = get_name(name, buf);
		if ( strcmp(interface, name))
			continue;
		get_dev_fields(type, s, pStats);
		fclose(fh);
		return 0;
    	}
	fclose(fh);
	return -1;
}

/////////////////////////////////////////////////////////////////////////////
static char *get_name(char *name, char *p)
{
    while (isspace(*p))
	p++;
    while (*p) {
	if (isspace(*p))
	    break;
	if (*p == ':') {	/* could be an alias */
	    char *dot = p, *dotname = name;
	    *name++ = *p++;
	    while (isdigit(*p))
		*name++ = *p++;
	    if (*p != ':') {	/* it wasn't, backup */
		p = dot;
		name = dotname;
	    }
	    if (*p == '\0')
		return NULL;
	    p++;
	    break;
	}
	*name++ = *p++;
    }
    *name++ = '\0';
    return p;
}

////////////////////////////////////////////////////////////////////////////////
static int get_dev_fields(int type, char *bp, struct user_net_device_stats *pStats)
{
    switch (type) {
    case 3:
	sscanf(bp,
	"%Lu %Lu %lu %lu %lu %lu %lu %lu %Lu %Lu %lu %lu %lu %lu %lu %lu",
	       &pStats->rx_bytes,
	       &pStats->rx_packets,
	       &pStats->rx_errors,
	       &pStats->rx_dropped,
	       &pStats->rx_fifo_errors,
	       &pStats->rx_frame_errors,
	       &pStats->rx_compressed,
	       &pStats->rx_multicast,

	       &pStats->tx_bytes,
	       &pStats->tx_packets,
	       &pStats->tx_errors,
	       &pStats->tx_dropped,
	       &pStats->tx_fifo_errors,
	       &pStats->collisions,
	       &pStats->tx_carrier_errors,
	       &pStats->tx_compressed);
	break;

    case 2:
	sscanf(bp, "%Lu %Lu %lu %lu %lu %lu %Lu %Lu %lu %lu %lu %lu %lu",
	       &pStats->rx_bytes,
	       &pStats->rx_packets,
	       &pStats->rx_errors,
	       &pStats->rx_dropped,
	       &pStats->rx_fifo_errors,
	       &pStats->rx_frame_errors,

	       &pStats->tx_bytes,
	       &pStats->tx_packets,
	       &pStats->tx_errors,
	       &pStats->tx_dropped,
	       &pStats->tx_fifo_errors,
	       &pStats->collisions,
	       &pStats->tx_carrier_errors);
	pStats->rx_multicast = 0;
	break;

    case 1:
	sscanf(bp, "%Lu %lu %lu %lu %lu %Lu %lu %lu %lu %lu %lu",
	       &pStats->rx_packets,
	       &pStats->rx_errors,
	       &pStats->rx_dropped,
	       &pStats->rx_fifo_errors,
	       &pStats->rx_frame_errors,

	       &pStats->tx_packets,
	       &pStats->tx_errors,
	       &pStats->tx_dropped,
	       &pStats->tx_fifo_errors,
	       &pStats->collisions,
	       &pStats->tx_carrier_errors);
	pStats->rx_bytes = 0;
	pStats->tx_bytes = 0;
	pStats->rx_multicast = 0;
	break;
    }
    return 0;
}


/////////////////////////////////////////////////////////////////////////////
int getWdsInfo(char *interface, char *pInfo)
{

#ifndef NO_ACTION
    int skfd;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0)
      /* If no wireless name : no wireless extensions */
        return -1;

    wrq.u.data.pointer = (caddr_t)pInfo;
    wrq.u.data.length = MAX_WDS_NUM*sizeof(WDS_INFO_T);

    if (iw_get_ext(skfd, interface, SIOCGIWRTLGETWDSINFO, &wrq) < 0)
	return -1;

    close( skfd );
#else
    memset(pInfo, 0, MAX_WDS_NUM*sizeof(WDS_INFO_T)); 
#endif

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
int getMiscData(char *interface, struct _misc_data_ *pData)
{

#ifndef NO_ACTION
    int skfd;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* Get wireless name */
    if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0)
      /* If no wireless name : no wireless extensions */
        return -1;

    wrq.u.data.pointer = (caddr_t)pData;
    wrq.u.data.length = sizeof(struct _misc_data_);

    if (iw_get_ext(skfd, interface, SIOCGMISCDATA, &wrq) < 0)
		return -1;

    close(skfd);
#else
    memset(pData, 0, sizeof(struct _misc_data_)); 
#endif

    return 0;
}

/*      IOCTL system call */
static int re865xIoctl(char *name, unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3)
{
  unsigned int args[4];
  struct ifreq ifr;
  int sockfd;

  args[0] = arg0;
  args[1] = arg1;
  args[2] = arg2;
  args[3] = arg3;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      perror("fatal error socket\n");
      return -3;
    }
  
  strcpy((char*)&ifr.ifr_name, name);
  ((unsigned int *)(&ifr.ifr_data))[0] = (unsigned int)args;

  if (ioctl(sockfd, SIOCDEVPRIVATE, &ifr)<0)
    {
      perror("device ioctl:");
      close(sockfd);
      return -1;
    }
  close(sockfd);
  return 0;
} /* end re865xIoctl */

#ifdef HOME_GATEWAY
#ifdef CONFIG_SMART_REPEATER
int getWispRptIface(char**pIface,int wlanId)
{
	int rptEnabled=0,wlanMode=0,opMode=0;
	char wlan_wanIfName[16]={0};
	if(wlanId == 0)
		apmib_get(MIB_REPEATER_ENABLED1, (void *)&rptEnabled);
	else if(1 == wlanId)
		apmib_get(MIB_REPEATER_ENABLED2, (void *)&rptEnabled);
	else return -1;
	apmib_get(MIB_OP_MODE,(void *)&opMode);
	if(opMode!=WISP_MODE)
		return -1;
	apmib_save_wlanIdx();
	
	sprintf(wlan_wanIfName,"wlan%d",wlanId);
	SetWlan_idx(wlan_wanIfName);
	//for wisp rpt mode,only care root ap
	apmib_get(MIB_WLAN_MODE, (void *)&wlanMode);
	if(AP_MODE==wlanMode && rptEnabled)
	{
		if(wlanId == 0)
			*pIface = "wlan0-vxd";
		else if(1 == wlanId)
			*pIface = "wlan1-vxd";
		else return -1;
	}else
	{
		char * ptmp = strstr(*pIface,"-vxd");
		if(ptmp)
			memset(ptmp,0,sizeof(char)*strlen("-vxd"));
	}
	apmib_recov_wlanIdx();
	return 0;
}

#endif
int getWanInfo(char *pWanIP, char *pWanMask, char *pWanDefIP, char *pWanHWAddr)
{
	DHCP_T dhcp;
	OPMODE_T opmode=-1;
	unsigned int wispWanId=0;
	char *iface=NULL;
	struct in_addr	intaddr;
	struct sockaddr hwaddr;
	unsigned char *pMacAddr;
	
	if ( !apmib_get( MIB_WAN_DHCP, (void *)&dhcp) )
		return -1;
  
  if ( !apmib_get( MIB_OP_MODE, (void *)&opmode) )
		return -1;

	if( !apmib_get(MIB_WISP_WAN_ID, (void *)&wispWanId))
		return -1;
	
	if ( dhcp == PPPOE || dhcp == PPTP || dhcp == L2TP || dhcp == USB3G ) { /* # keith: add l2tp support. 20080515 */
#ifdef MULTI_PPPOE
	if(dhcp == PPPOE){
		extern char  ppp_iface[32];
		iface = ppp_iface;
	}
#else
	iface = "ppp0";
#endif
		
		if ( !isConnectPPP() )
			iface = NULL;
	}
	else if (opmode == WISP_MODE){
		if(0 == wispWanId)
			iface = "wlan0";
		else if(1 == wispWanId)
			iface = "wlan1";
#ifdef CONFIG_SMART_REPEATER
		if(getWispRptIface(&iface,wispWanId)<0)
					return -1;
#endif			
	}
	else
		iface = "eth1";
	
	if(opmode != WISP_MODE)
	{
		if(iface){
			if(getWanLink("eth1") < 0){
				sprintf(pWanIP,"%s","0.0.0.0");
			}
		}	
	}
	
	if ( iface && getInAddr(iface, IP_ADDR, (void *)&intaddr ) )
		sprintf(pWanIP,"%s",inet_ntoa(intaddr));
	else
		sprintf(pWanIP,"%s","0.0.0.0");

	if ( iface && getInAddr(iface, SUBNET_MASK, (void *)&intaddr ) )
		sprintf(pWanMask,"%s",inet_ntoa(intaddr));
	else
		sprintf(pWanMask,"%s","0.0.0.0");
	
	if ( iface && getDefaultRoute(iface, &intaddr) )
		sprintf(pWanDefIP,"%s",inet_ntoa(intaddr));
	else
		sprintf(pWanDefIP,"%s","0.0.0.0");	

	//To get wan hw addr
	if(opmode == WISP_MODE) {
		if(0 == wispWanId)
			iface = "wlan0";
		else if(1 == wispWanId)
			iface = "wlan1";
#ifdef CONFIG_SMART_REPEATER
		if(getWispRptIface(&iface,wispWanId)<0)
					return -1;
#endif			
	}	
	else
		iface = "eth1";
	
	if ( getInAddr(iface, HW_ADDR, (void *)&hwaddr ) ) 
	{
		pMacAddr = (unsigned char *)hwaddr.sa_data;
		sprintf(pWanHWAddr,"%02x:%02x:%02x:%02x:%02x:%02x",pMacAddr[0], pMacAddr[1],pMacAddr[2], pMacAddr[3], pMacAddr[4], pMacAddr[5]);
	}
	else
		sprintf(pWanHWAddr,"%s","00:00:00:00:00:00");

	return 0;
}
#endif

/* ethernet port link status */
int getEth0PortLink(unsigned int port_index)
{
    int    ret=-1;
    unsigned int    args[0];
        
    ret = port_index;
    re865xIoctl("eth0", RTL8651_IOCTL_GETLANLINKSTATUS, (unsigned int)(args), 0, (unsigned int)&ret);
    if(ret ==0){
    	return 1;
    }else if(ret < 0){
    	return 0;
    }
    return 0;
}

#if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
/* RTL8651_IOCTL_GET_ETHER_EEE_STATE */
int getEthernetEeeState(unsigned int port_index)
{
      unsigned int    ret=0;
      unsigned int    args[0];
 			
 			re865xIoctl("eth0", RTL8651_IOCTL_GET_ETHER_EEE_STATE, (unsigned int)(args), 0, (unsigned int)&ret);      
    	
			//if(ret & (1<<(3+port_index*4)))
			if(ret & (1<<(1+port_index*4)))
      	return 1;
      else
      	return 0;
}
#endif //#if defined(CONFIG_RTL_8196C)

/* RTL8651_IOCTL_GET_ETHER_BYTES_COUNT */
unsigned int getEthernetBytesCount(unsigned int port_index)
{
      unsigned int    ret=0;
      unsigned int    args[0];
 			
 			ret = port_index;
 			re865xIoctl("eth0", RTL8651_IOCTL_GET_ETHER_BYTES_COUNT, (unsigned int)(args), 0, (unsigned int)&ret);      
    	
      return ret;
}


#define RTL8651_IOCTL_GETWANLINKSTATUS 2000
/* Wan link status detect */
int getWanLink(char *interface)
{
        unsigned int    ret;
        unsigned int    args[0];

        re865xIoctl(interface, RTL8651_IOCTL_GETWANLINKSTATUS, (unsigned int)(args), 0, (unsigned int)&ret) ;
        return ret;
}

#ifdef UNIVERSAL_REPEATER
/////////////////////////////////////////////////////////////////////////////
int isVxdInterfaceExist(char *interface)
{
#ifndef NO_ACTION	
	int skfd, ret;  
	struct ifreq ifr;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

	strcpy(ifr.ifr_name, interface);
	if ((ret = ioctl(skfd, SIOCGIFFLAGS, &ifr)) < 0)
		ret = 0;
	else {
		if (!(ifr.ifr_flags & IFF_UP))
			ret = 0;	
		else
			ret = 1;
     }
	close(skfd);
	
	return ret;	
#else	

	return 0;
#endif	
	
}
#endif // UNIVERSAL_REPEATER

#if 1
int displayPostDate(char *postDate)
{
	char *strP;
	int	len=0;
	char str[200];

	memset(str, 0x00, sizeof(str));
	
	if (postDate != NULL)
	{
		printf("\r\n --- postDate ---\r\n");
		strP = postDate;
		
		while(*strP != '\0')
		{
			while(*strP != '&' && *strP != '\0')
			{
				strP++;
				len++;
			}
			strncpy(str,postDate, len);
			printf(" %s\r\n",str);

			if(*strP == '\0')
			{
			
			}
			else
			{
				postDate = ++strP;
				len = 0;
				memset(str, 0x00, sizeof(str));
			}
		}

		printf(" ----------------\r\n");
	}
	return 0;
}
#endif

int fwChecksumOk(char *data, int len)
{
	unsigned short sum=0;
	int i;

	for (i=0; i<len; i+=2) {
#ifdef _LITTLE_ENDIAN_
		sum += WORD_SWAP( *((unsigned short *)&data[i]) );
#else
		sum += *((unsigned short *)&data[i]);
#endif

	}
	return( (sum==0) ? 1 : 0);
}

///////////////////////////////////////////////////////////////////////////////


void kill_processes(void)
{


	printf("upgrade: killing tasks...\n");
	
	kill(1, SIGTSTP);		/* Stop init from reforking tasks */
	kill(1, SIGSTOP);		
	kill(2, SIGSTOP);		
	kill(3, SIGSTOP);		
	kill(4, SIGSTOP);		
	kill(5, SIGSTOP);		
	kill(6, SIGSTOP);		
	kill(7, SIGSTOP);		
	//atexit(restartinit);		/* If exit prematurely, restart init */
	sync();

	signal(SIGTERM,SIG_IGN);	/* Don't kill ourselves... */
	setpgrp(); 			/* Don't let our parent kill us */
	sleep(1);
	signal(SIGHUP, SIG_IGN);	/* Don't die if our parent dies due to
					 * a closed controlling terminal */
	
}

#ifndef NO_ACTION

static int daemonKilled = 0;
//static void killDaemon()
void killDaemon(int wait)
{
#ifndef WIFI_SIMPLE_CONFIG
    int pid;
#endif
		
    if (daemonKilled)
    	return;

    daemonKilled = 1;
#if 1   
    FILE *stream;
    system("ps > /var/alive_daemons");
    stream = fopen ( "/var/alive_daemons", "r" );
    if ( stream != NULL ) {
        char *strtmp, *pid_token=NULL;
        char line[100], cmdBuf[50];

        while (fgets(line, sizeof(line), stream))
        {
            strtmp = line;
            while(*strtmp == ' ')
                strtmp++;
            if(	(strstr(strtmp,"boa") != 0) 
            		|| (strstr(strtmp,"/bin/sh") != 0) 
            		|| (strstr(strtmp,"ps") != 0) 
            		|| (strstr(strtmp, "root") == 0)
#if defined(CONFIG_APP_FWD) // fwd is daemon to write firmware in flash at last.
            		|| (strstr(strtmp, "fwd") != 0)
#endif            		
            )
                continue;
            else
            {
                pid_token = strtok(strtmp, " ");
                if((pid_token != NULL) && (strlen(pid_token) >= 3))
                {
                    sprintf(cmdBuf,"kill -9 %s > /dev/null", pid_token);
                    system(cmdBuf);
                }
            }
        }
        fclose(stream);
    }
    system("rm -rf /var/alive_daemons");
#elif WIFI_SIMPLE_CONFIG

	system("killall -9 sleep 2> /dev/null");
	system("killsh.sh");	// kill all running script	
       system("killall -9 routed 2> /dev/null");
	system("killall -9 pppoe 2> /dev/null");
	system("killall -9 pppd 2> /dev/null");
	system("killall -9 pptp 2> /dev/null");
	system("killall -9 dnrd 2> /dev/null");
	system("killall -9 ntpclient 2> /dev/null");
	system("killall -9 miniigd 2> /dev/null");
	system("killall -9 lld2d 2> /dev/null");
	system("killall -9 l2tpd 2> /dev/null");	
	system("killall -9 udhcpc 2> /dev/null");	
	system("killall -9 udhcpd 2> /dev/null");	
	system("killall -9 reload 2> /dev/null");		
	system("killall -9 iapp 2> /dev/null");	
	system("killall -9 wscd 2> /dev/null");
	system("killall -9 mini_upnpd 2> /dev/null");
	system("killall -9 iwcontrol 2> /dev/null");
	system("killall -9 auth 2> /dev/null");
	system("killall -9 disc_server 2> /dev/null");
	system("killall -9 igmpproxy 2> /dev/null");
	system("echo 1,0 > /proc/br_mCastFastFwd ");
	system("killall -9 syslogd 2> /dev/null");
	system("killall -9 klogd 2> /dev/null");
	system("killall -9 ntfs-3g 2> /dev/null");
#else

    pid = find_pid_by_name("udhcpc");
    if (pid > 0)
    	kill(pid, SIGKILL);

    pid = find_pid_by_name("udhcpd");
    if (pid > 0)
    	kill(pid, SIGKILL);

    pid = find_pid_by_name("iwcontrol");
    if (pid > 0)
    	kill(pid, SIGKILL);

    pid = find_pid_by_name("iapp");
    if (pid > 0)
    	kill(pid, SIGKILL);

    pid = find_pid_by_name("auth");
    if (pid > 0)
    	kill(pid, SIGKILL);
#ifdef HOME_GATEWAY

    //sc_yang
    pid = find_pid_by_name("pppoe.sh");
    if (pid > 0)
        kill(pid, SIGKILL);

    // kill all upnpd 
    while((pid = find_pid_by_name("upnpd")) > 0)
    {
	    if (pid > 0)
		kill(pid, SIGKILL);
    }

    pid = find_pid_by_name("pppd");
    if (pid > 0)
        kill(pid, SIGKILL);

    pid = find_pid_by_name("pppoe");
    if (pid > 0)
        kill(pid, SIGKILL);

    pid = find_pid_by_name("dnrd");
    if (pid > 0)
        kill(pid, SIGKILL);

    pid = find_pid_by_name("disc_server");
    if (pid > 0)
        kill(pid, SIGKILL);

	if (wait)
	    sleep(5);
#endif
#endif
	
// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
	system("killall -9 snmpd 2> /dev/null");
	system("killall -9 solar_monitor 2> /dev/null");
	system("killall -9 solar 2> /dev/null");
	system("killall -9 dns_task 2> /dev/null");
	system("killall -9 ivrserver 2> /dev/null");
	system("killall -9 fwupdate 2> /dev/null");
	system("killall -9 netlink 2> /dev/null");
#endif

#ifdef CONFIG_SNMP
	system("killall -9 snmpd 2> /dev/null");
#endif
}
#endif // NO_ACTION

int cmd_killproc(int mode)
{
	char *solarPid = "/var/run/solar.pid";
	char *dnsTaskPid="/var/run/dns_task.pid";
	char *lld2dPid="/var/run/lld2d-br0.pid";
	char *webPid = "/var/run/webs.pid";
	char *wscdPid = "/var/run/wscd-wlan0.pid";
	char *ivrserverPid ="/var/run/ivrserver.pid";
	char *iwcontrolPid ="/var/run/iwcontrol.pid";
	char *iappPid ="/var/run/iapp.pid";
	char line[20];
	pid_t pid;
	FILE *fp=NULL;
	
//	printf("***%s:%s:%d***\n",__FILE__,__FUNCTION__,__LINE__);

	if(mode == 1){ // before download
		system("killall solar_monitor");
		printf("kill solar \n");
		if ((fp = fopen(solarPid, "r")) != NULL) {
			fgets(line, sizeof(line), fp);
			if ( sscanf(line, "%d", &pid) ) {
				if (pid > 1)
					kill(pid, SIGTERM);
			}
			fclose(fp);
			unlink("/bin/solar");
			unlink("/bin/solar_monitor");
		}
	#ifdef CONFIG_RTK_MTD_ROOT
		system("rm /var/web -rf");		
	#else
		system("rm /web -rf");
	#endif	
		sync();
		sleep(1);
	}else{
		printf("kill process\n");
		kill(1, SIGTSTP);		/* Stop init from reforking tasks */
		kill(1, SIGSTOP);		
		kill(2, SIGSTOP);		
		kill(3, SIGSTOP);		
		kill(4, SIGSTOP);		
		kill(5, SIGSTOP);		
		kill(6, SIGSTOP);		
		kill(7, SIGSTOP);
		
		printf("kill boa \n");
		if ((fp = fopen(webPid, "r")) != NULL) {
			fgets(line, sizeof(line), fp);
			if ( sscanf(line, "%d", &pid) ) {
				if (pid > 1)
					kill(pid, SIGTERM);
			}
			fclose(fp);
			unlink("/bin/boa");
		}
		printf("kill wscd \n");
		if ((fp = fopen(wscdPid, "r")) != NULL) {
			fgets(line, sizeof(line), fp);
			if ( sscanf(line, "%d", &pid) ) {
				if (pid > 1)
					kill(pid, SIGTERM);
			}
			fclose(fp);
			unlink("/bin/wscd");
		}
		printf("kill ivrserver \n");
		if ((fp = fopen(ivrserverPid, "r")) != NULL) {
			fgets(line, sizeof(line), fp);
			if ( sscanf(line, "%d", &pid) ) {
				if (pid > 1)
					kill(pid, SIGTERM);
			}
			fclose(fp);
			unlink("/bin/ivrserver");
		}
		printf("kill dns_task\n");
		if ((fp = fopen(dnsTaskPid, "r")) != NULL) {
			fgets(line, sizeof(line), fp);
			if ( sscanf(line, "%d", &pid) ) {
				if (pid > 1)
					kill(pid, SIGTERM);
			}
			fclose(fp);
			unlink("/bin/dns_task");
		}
		printf("kill lld2d \n");
		if ((fp = fopen(lld2dPid, "r")) != NULL) {
			fgets(line, sizeof(line), fp);
			if ( sscanf(line, "%d", &pid) ) {
				if (pid > 1)
					kill(pid, SIGTERM);
			}
			fclose(fp);
			unlink("/bin/lld2d");
		}
		printf("kill iwcontrol \n");
		if ((fp = fopen(iwcontrolPid, "r")) != NULL) {
			fgets(line, sizeof(line), fp);
			if ( sscanf(line, "%d", &pid) ) {
				if (pid > 1)
					kill(pid, SIGTERM);
			}
			fclose(fp);
			unlink("/bin/iwcontrol");
		}
		printf("kill iapp \n");
		if ((fp = fopen(iappPid, "r")) != NULL) {
			fgets(line, sizeof(line), fp);
			if ( sscanf(line, "%d", &pid) ) {
				if (pid > 1)
					kill(pid, SIGTERM);
			}
			fclose(fp);
			unlink("/bin/iapp");
		}
			//atexit(restartinit);		/* If exit prematurely, restart init */
		sync();

		signal(SIGTERM,SIG_IGN);	/* Don't kill ourselves... */
		setpgrp(); 			/* Don't let our parent kill us */
		sleep(1);
		signal(SIGHUP, SIG_IGN);	/* Don't die if our parent dies due to				 * a closed controlling terminal */
	}
	return 0;
}

#ifdef CONFIG_APP_TR069

int cmd_reboot()
{
	printf("***%s:%s:%d***\n",__FILE__,__FUNCTION__,__LINE__);
#ifndef NO_ACTION
	system("reboot");
	exit(0);
#endif
	return 0;
}

int do_cmd(const char *filename, char *argv [], int dowait)
{
#ifndef NO_ACTION
	pid_t pid, wpid;
	int stat=0, st;
	
	if((pid = vfork()) == 0) {
		/* the child */
		char *env[3];
		
		signal(SIGINT, SIG_IGN);
		argv[0] = (char *)filename;
		env[0] = "PATH=/bin:/usr/bin:/etc:/sbin:/usr/sbin";
		env[1] = NULL;

		execve(filename, argv, env);

		printf("exec %s failed\n", filename);
		_exit(2);
	} else if(pid > 0) {
		if (!dowait)
			stat = 0;
		else {
			/* parent, wait till rc process dies before spawning */
			while ((wpid = wait(&st)) != pid)
				if (wpid == -1 && errno == ECHILD) { /* see wait(2) manpage */
					stat = 0;
					break;
				}
		}
	} else if(pid < 0) {
		printf("fork of %s failed\n", filename);
		stat = -1;
	}
	return st;
#else
	return 0;
#endif
}

int va_cmd(const char *cmd, int num, int dowait, ...)
{
	va_list ap;
	int k;
	char *s;
	char *argv[24];
	int status;
	
	va_start(ap, dowait);
	
	for (k=0; k<num; k++)
	{
		s = va_arg(ap, char *);
		argv[k+1] = s;
	}
	
	argv[k+1] = NULL;
	status = do_cmd(cmd, argv, dowait);
	va_end(ap);
	
	return status;
}

#if 0	// pkshih: within tr104 
/* send the request to solar */
void cwmpSendRequestToSolar(void){
	
}

/*open the connection from solar to cwmpclient*/
void cwmp_solarOpen( void )
{
	
}

/*close the connection from solar to cwmpclient*/
void cwmp_solarClose(void){

}
#endif

#ifndef NO_ACTION

const char STR_DISABLE[] = "Disabled";
const char STR_ENABLE[] = "Enabled";
const char STR_AUTO[] = "Auto";
const char STR_MANUAL[] = "Manual";
const char STR_ERR[] = "err";

const char *dhcp_mode[] = {
	"None", "DHCP Relay", "DHCP Server"
};

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
		//fprintf(stderr, "fail to get mem %s:%d\n", __FUNCTION__, __LINE__);
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

int getWlanMib(int wlanRootIndex, int wlanValIndex, int id, void *value)
{
	int root_old, val_old;

	root_old = wlan_idx;
	val_old = vwlan_idx;

	wlan_idx = wlanRootIndex;
	vwlan_idx = wlanValIndex;
	
	apmib_get(id, (void *)value);
	
	wlan_idx = root_old;
	vwlan_idx = val_old;
	
	return 0;
}

int setWlanMib(int wlanRootIndex, int wlanValIndex, int id, void *value)
{
	int root_old, val_old;

	root_old = wlan_idx;
	val_old = vwlan_idx;

	wlan_idx = wlanRootIndex;
	vwlan_idx = wlanValIndex;
	
	apmib_set(id, (void *)value);
	
	wlan_idx = root_old;
	vwlan_idx = val_old;
	
	return 0;
}

char wlan_ifname[20]={0};
int getWlanBssInfo(int wlanRootIndex, int wlanValIndex, void *bss)
{
	int root_old, val_old;
	
	root_old = wlan_idx;
	val_old = vwlan_idx;
	
	wlan_idx = wlanRootIndex;
	vwlan_idx = wlanValIndex;
	
	if (vwlan_idx == 0)
	{
		sprintf(wlan_ifname, "wlan%d", wlan_idx);
	}
	else if (vwlan_idx > 0 && vwlan_idx < 5) //repeater is 5; wds is 6
	{
		sprintf(wlan_ifname, "wlan%d-va%d", wlan_idx, vwlan_idx-1);
	}
	else if (vwlan_idx == 5) //repeater is 5; wds is 6
	{
		sprintf(wlan_ifname, "wlan%d-vxd", wlan_idx);
	}
	else if (vwlan_idx == 6) //repeater is 5; wds is 6
	{
		sprintf(wlan_ifname, "wlan%d-wds", wlan_idx);
	}

	if( getWlBssInfo(wlan_ifname, bss) !=0 )
	{
		wlan_idx = root_old;
		vwlan_idx = val_old;
		return -1;		
	}
	
	wlan_idx = root_old;
	vwlan_idx = val_old;
		
	return 0;
}

#endif // NO_ACTION

static int updateConfigIntoFlash(unsigned char *data, int total_len, int *pType, int *pStatus)
{
	int len=0, status=1, type=0, ver, force;
	PARAM_HEADER_Tp pHeader;
#ifdef COMPRESS_MIB_SETTING
	COMPRESS_MIB_HEADER_Tp pCompHeader;
	unsigned char *expFile=NULL;
	unsigned int expandLen=0;
	int complen=0;
#endif
	char *ptr;

	do {
#ifdef COMPRESS_MIB_SETTING
		pCompHeader =(COMPRESS_MIB_HEADER_Tp)&data[complen];
#ifdef _LITTLE_ENDIAN_
		pCompHeader->compRate = WORD_SWAP(pCompHeader->compRate);
		pCompHeader->compLen = DWORD_SWAP(pCompHeader->compLen);
#endif
		/*decompress and get the tag*/
		expFile=malloc(pCompHeader->compLen*pCompHeader->compRate);
		if(NULL==expFile)
		{
			printf("malloc for expFile error!!\n");
			return 0;
		}
		expandLen = Decode(data+complen+sizeof(COMPRESS_MIB_HEADER_T), pCompHeader->compLen, expFile);
		pHeader = (PARAM_HEADER_Tp)expFile;
#else
		pHeader = (PARAM_HEADER_Tp)&data[len];
#endif
		
#ifdef _LITTLE_ENDIAN_
		pHeader->len = WORD_SWAP(pHeader->len);
#endif
		len += sizeof(PARAM_HEADER_T);

		if ( sscanf(&pHeader->signature[TAG_LEN], "%02d", &ver) != 1)
			ver = -1;
			
		force = -1;
		if ( !memcmp(pHeader->signature, CURRENT_SETTING_HEADER_TAG, TAG_LEN) )
			force = 1; // update
		else if ( !memcmp(pHeader->signature, CURRENT_SETTING_HEADER_FORCE_TAG, TAG_LEN))
			force = 2; // force
		else if ( !memcmp(pHeader->signature, CURRENT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN))
			force = 0; // upgrade

		if ( force >= 0 ) {
#if 0
			if ( !force && (ver < CURRENT_SETTING_VER || // version is less than current
				(pHeader->len < (sizeof(APMIB_T)+1)) ) { // length is less than current
				status = 0;
				break;
			}
#endif

#ifdef COMPRESS_MIB_SETTING
			ptr = expFile+sizeof(PARAM_HEADER_T);
#else
			ptr = &data[len];
#endif

#ifdef COMPRESS_MIB_SETTING
#else
			DECODE_DATA(ptr, pHeader->len);
#endif
			if ( !CHECKSUM_OK(ptr, pHeader->len)) {
				status = 0;
				break;
			}
#ifdef _LITTLE_ENDIAN_
			swap_mib_word_value((APMIB_Tp)ptr);
#endif

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
			flash_voip_import_fix(&((APMIB_Tp)ptr)->voipCfgParam, &pMib->voipCfgParam);
#endif

#ifdef COMPRESS_MIB_SETTING
			apmib_updateFlash(CURRENT_SETTING, &data[complen], pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T), force, ver);
#else
			apmib_updateFlash(CURRENT_SETTING, ptr, pHeader->len-1, force, ver);
#endif

#ifdef COMPRESS_MIB_SETTING
			complen += pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T);
			if(expFile)
			{
				free(expFile);
				expFile=NULL;
			}
#else
			len += pHeader->len;
#endif
			type |= CURRENT_SETTING;
			continue;
		}


		if ( !memcmp(pHeader->signature, DEFAULT_SETTING_HEADER_TAG, TAG_LEN) )
			force = 1;	// update
		else if ( !memcmp(pHeader->signature, DEFAULT_SETTING_HEADER_FORCE_TAG, TAG_LEN) )
			force = 2;	// force
		else if ( !memcmp(pHeader->signature, DEFAULT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) )
			force = 0;	// upgrade

		if ( force >= 0 ) {
#if 0
			if ( (ver < DEFAULT_SETTING_VER) || // version is less than current
				(pHeader->len < (sizeof(APMIB_T)+1)) ) { // length is less than current
				status = 0;
				break;
			}
#endif

#ifdef COMPRESS_MIB_SETTING
			ptr = expFile+sizeof(PARAM_HEADER_T);
#else
			ptr = &data[len];
#endif

#ifdef COMPRESS_MIB_SETTING
#else
			DECODE_DATA(ptr, pHeader->len);
#endif
			if ( !CHECKSUM_OK(ptr, pHeader->len)) {
				status = 0;
				break;
			}

#ifdef _LITTLE_ENDIAN_
			swap_mib_word_value((APMIB_Tp)ptr);
#endif

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
			flash_voip_import_fix(&((APMIB_Tp)ptr)->voipCfgParam, &pMibDef->voipCfgParam);
#endif

#ifdef COMPRESS_MIB_SETTING
			apmib_updateFlash(DEFAULT_SETTING, &data[complen], pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T), force, ver);
#else
			apmib_updateFlash(DEFAULT_SETTING, ptr, pHeader->len-1, force, ver);
#endif

#ifdef COMPRESS_MIB_SETTING
			complen += pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T);
			if(expFile)
			{
				free(expFile);
				expFile=NULL;
			}	
#else
			len += pHeader->len;
#endif
			type |= DEFAULT_SETTING;
			continue;
		}

		if ( !memcmp(pHeader->signature, HW_SETTING_HEADER_TAG, TAG_LEN) )
			force = 1;	// update
		else if ( !memcmp(pHeader->signature, HW_SETTING_HEADER_FORCE_TAG, TAG_LEN) )
			force = 2;	// force
		else if ( !memcmp(pHeader->signature, HW_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) )
			force = 0;	// upgrade

		if ( force >= 0 ) {
#if 0
			if ( (ver < HW_SETTING_VER) || // version is less than current
				(pHeader->len < (sizeof(HW_SETTING_T)+1)) ) { // length is less than current
				status = 0;
				break;
			}
#endif
#ifdef COMPRESS_MIB_SETTING
			ptr = expFile+sizeof(PARAM_HEADER_T);
#else
			ptr = &data[len];
#endif
			

#ifdef COMPRESS_MIB_SETTING
#else
			DECODE_DATA(ptr, pHeader->len);
#endif
			if ( !CHECKSUM_OK(ptr, pHeader->len)) {
				status = 0;
				break;
			}
#ifdef COMPRESS_MIB_SETTING
			apmib_updateFlash(HW_SETTING, &data[complen], pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T), force, ver);
#else
			apmib_updateFlash(HW_SETTING, ptr, pHeader->len-1, force, ver);
#endif

#ifdef COMPRESS_MIB_SETTING
			complen += pCompHeader->compLen+sizeof(COMPRESS_MIB_HEADER_T);
			if(expFile)
			{
				free(expFile);
				expFile=NULL;
			}
#else
			len += pHeader->len;
#endif

			type |= HW_SETTING;
			continue;
		}
	}
#ifdef COMPRESS_MIB_SETTING	
	while (complen < total_len);
#else
	while (len < total_len);
#endif
	if(expFile)
	{
		free(expFile);
		expFile=NULL;
	}

	*pType = type;
	*pStatus = status;
#ifdef COMPRESS_MIB_SETTING	
	return complen;
#else
	return len;
#endif
}

int doFirmwareUpgrade(char *upload_data, int upload_len, int is_root, char *buffer)
{
int head_offset=0 ;
int isIncludeRoot=0;
 int		 len;
    int          locWrite;
    int          numLeft;
    int          numWrite;
    IMG_HEADER_Tp pHeader;
	int flag=0, startAddr=-1, startAddrWeb=-1;
	int update_fw=0, update_cfg=0;
#ifdef __mips__
    int fh;
#else
    FILE *fp;
    char *bn = NULL;
#endif
	unsigned char cmdBuf[30];

	system("ifconfig br0 down 2> /dev/null");


while(head_offset <   upload_len) {
    locWrite = 0;
    pHeader = (IMG_HEADER_Tp) &upload_data[head_offset];
    len = pHeader->len;
#ifdef _LITTLE_ENDIAN_
    len  = DWORD_SWAP(len);
#endif    
    numLeft = len + sizeof(IMG_HEADER_T) ;
    
    // check header and checksum
    if (!memcmp(&upload_data[head_offset], FW_HEADER, SIGNATURE_LEN) || !memcmp(&upload_data[head_offset], FW_HEADER_WITH_ROOT, SIGNATURE_LEN))
    	flag = 1;
    else if (!memcmp(&upload_data[head_offset], WEB_HEADER, SIGNATURE_LEN))
    	flag = 2;
    else if (!memcmp(&upload_data[head_offset], ROOT_HEADER, SIGNATURE_LEN))
    {
    	flag = 3;
    	isIncludeRoot = 1;
    }
    else if (
#ifdef COMPRESS_MIB_SETTING
				!memcmp(&upload_data[head_offset], COMP_HS_SIGNATURE, COMP_SIGNATURE_LEN) ||
				!memcmp(&upload_data[head_offset], COMP_DS_SIGNATURE, COMP_SIGNATURE_LEN) ||
				!memcmp(&upload_data[head_offset], COMP_CS_SIGNATURE, COMP_SIGNATURE_LEN)
#else	
    	 	!memcmp(&upload_data[head_offset], CURRENT_SETTING_HEADER_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], CURRENT_SETTING_HEADER_FORCE_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], CURRENT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], DEFAULT_SETTING_HEADER_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], DEFAULT_SETTING_HEADER_FORCE_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], DEFAULT_SETTING_HEADER_UPGRADE_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], HW_SETTING_HEADER_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], HW_SETTING_HEADER_FORCE_TAG, TAG_LEN) ||
				!memcmp(&upload_data[head_offset], HW_SETTING_HEADER_UPGRADE_TAG, TAG_LEN)
#endif				
		)
    {
		int type, status, cfg_len;
			
		cfg_len = updateConfigIntoFlash(&upload_data[head_offset], 0, &type, &status);
		
		if (status == 0 || type == 0) { // checksum error
			strcpy(buffer, "Invalid configuration file!");
			goto ret_upload;
		}
		else { // upload success
			strcpy(buffer, "Update successfully!");
			head_offset += cfg_len;
			update_cfg = 1;
		}    	
		continue;
    }
    else {
       	sprintf(buffer, "Invalid file format [%s-%d]!",__FILE__,__LINE__);
		goto ret_upload;
    }


    if(len > 0x200000){ //len check by sc_yang
      		sprintf(buffer, "Image len exceed max size 0x200000 ! len=0x%x</b><br>", len);
		goto ret_upload;
    }
    if ( (flag == 1) || (flag == 3)) {
    	if ( !fwChecksumOk(&upload_data[sizeof(IMG_HEADER_T)+head_offset], len)) 
    	{
      		sprintf(buffer, "Image checksum mismatched! len=0x%x, checksum=0x%x</b><br>", len,
			*((unsigned short *)&upload_data[len-2]) );
		goto ret_upload;
	}
    }
    else {
    	char *ptr = &upload_data[sizeof(IMG_HEADER_T)+head_offset];
    	if ( !CHECKSUM_OK(ptr, len) ) 
    	{
     		sprintf(buffer, "Image checksum mismatched! len=0x%x</b><br>", len);
		goto ret_upload;
	}
    }

#ifdef __mips__
    if(flag == 3)
    	fh = open(FLASH_DEVICE_NAME1, O_RDWR);
    else
       fh = open(FLASH_DEVICE_NAME, O_RDWR);

    if ( fh == -1 ) {
#else
    if (flag == 1)
    	bn = "apcode.bin";
    else if (flag == 3)
    	bn = "root.bin" ;
    else
    	bn = "web.gz.up";

    if ((fp = fopen((bn == NULL ? "upldForm.bin" : bn), "w+b")) == NULL) {
#endif
       	strcpy(buffer, "File open failed!");
	goto ret_upload;
    } else {

#ifdef __mips__
	if (flag == 1) {
		if ( startAddr == -1){
			//startAddr = CODE_IMAGE_OFFSET;
			startAddr = pHeader->burnAddr ;
			#ifdef _LITTLE_ENDIAN_
    				startAddr = DWORD_SWAP(startAddr);
    			#endif
		}

	}
	else if (flag == 3) {
		if ( startAddr == -1){
			startAddr = 0; // always start from offset 0 for 2nd FLASH partition
		}
	}
	else {
		if ( startAddrWeb == -1){
			//startAddr = WEB_PAGE_OFFSET;
			startAddr = pHeader->burnAddr ;
			#ifdef _LITTLE_ENDIAN_
    				startAddr = DWORD_SWAP(startAddr);
    			#endif
		}
		else
			startAddr = startAddrWeb;
	}
	lseek(fh, startAddr, SEEK_SET);
	if(flag == 3){
		locWrite += sizeof(IMG_HEADER_T); // remove header
		numLeft -=  sizeof(IMG_HEADER_T);
		system("ifconfig br0 down 2> /dev/null");
		system("ifconfig eth0 down 2> /dev/null");
		system("ifconfig eth1 down 2> /dev/null");
		system("ifconfig ppp0 down 2> /dev/null");
		system("ifconfig wlan0 down 2> /dev/null");
		system("ifconfig wlan0-vxd down 2> /dev/null");				
		system("ifconfig wlan0-va0 down 2> /dev/null");		
		system("ifconfig wlan0-va1 down 2> /dev/null");		
		system("ifconfig wlan0-va2 down 2> /dev/null");		
		system("ifconfig wlan0-va3 down 2> /dev/null");
		system("ifconfig wlan0-wds0 down 2> /dev/null");
		system("ifconfig wlan0-wds1 down 2> /dev/null");
		system("ifconfig wlan0-wds2 down 2> /dev/null");
		system("ifconfig wlan0-wds3 down 2> /dev/null");
		system("ifconfig wlan0-wds4 down 2> /dev/null");
		system("ifconfig wlan0-wds5 down 2> /dev/null");
		system("ifconfig wlan0-wds6 down 2> /dev/null");
		system("ifconfig wlan0-wds7 down 2> /dev/null");

		kill_processes();
		sleep(2);
	}
	
	numWrite = write(fh, &(upload_data[locWrite+head_offset]), numLeft);
#else
	numWrite = fwrite(&(upload_data[locWrite+head_offset]), sizeof(*(upload_data)), numLeft, fp);
#endif
	if (numWrite < numLeft) {
#ifdef __mips__
		sprintf(buffer, "File write failed. locWrite=%d numLeft=%d numWrite=%d Size=%d bytes.", locWrite, numLeft, numWrite, upload_len);

#else
                sprintf(buffer, "File write failed. ferror=%d locWrite=%d numLeft=%d numWrite=%d Size=%d bytes.", ferror(fp), locWrite, numLeft, numWrite, upload_len);
#endif
	goto ret_upload;
	}
	locWrite += numWrite;
 	numLeft -= numWrite;
	sync();
#ifdef __mips__
	//if(flag != 3)
		close(fh);
#else
	fclose(fp);
#endif
	head_offset += len + sizeof(IMG_HEADER_T) ;
	startAddr = -1 ; //by sc_yang to reset the startAddr for next image
	update_fw = 1;
    }
} //while //sc_yang   
#ifndef NO_ACTION

//		alarm(2);
		//system("reboot");
		//for(;;);

#else
#ifdef VOIP_SUPPORT
	// rock: for x86 simulation
	if (update_cfg && !update_fw) {
		if (apmib_reinit()) {
//			reset_user_profile();  // re-initialize user password
		}
//		if(FW_Data)
//			free(FW_Data);
	}
#endif
#endif
  return 1;
  ret_upload:	
  	fprintf(stderr, "%s\n", buffer);	
	return 0;
}

#define BACKUP_RAW_CSCONF

#if defined(BACKUP_RAW_CSCONF)
static unsigned char *compFile = NULL;
#endif

int mib_backup(int backup_target)
{
	int ret = 1;

	//fprintf(stderr,"\r\n __[%s-%u]",__FILE__,__LINE__);
	
#if defined(BACKUP_RAW_CSCONF)
	ret = flash_read_raw_mib(&compFile); /* only backup current setting, don't care backup_target*/
#endif

	return ret;
}

int mib_restore(int restore)
{
	int ret = 1;

	//fprintf(stderr,"\r\nrestore=[%d], __[%s-%u]",restore,__FILE__,__LINE__);
	
#if defined(BACKUP_RAW_CSCONF)
	if (restore) {
		ret = flash_write_raw_mib(&compFile);
		if (ret == 1) {
			if (1!= apmib_reinit())
				ret = -1;
}
	}
#endif
	return ret;
}

#if defined(MOD_FOR_TR098_LANDEVICE)
int getInFlags(char *interface, int *flags)
{
	int skfd, ret = 0;
	struct ifreq ifr;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

	strcpy(ifr.ifr_name, interface);
	if ((ret = ioctl(skfd, SIOCGIFFLAGS, &ifr)) < 0)
		ret = 0;
	else {
 		*flags = ifr.ifr_flags;
		ret = 1;
    }

	close(skfd);
	return ret;	
}
#endif

#endif //#ifdef CONFIG_APP_TR069

unsigned char *gettoken(const unsigned char *str,unsigned int index,unsigned char symbol)
{
	static char tmp[50];
	unsigned char tk[50]; //save symbol index
	char *ptmp;
	int i,j,cnt=1,start,end;
	//scan symbol and save index
	
	memset(tmp, 0x00, sizeof(tmp));
	
	for (i=0;i<strlen((char *)str);i++)
	{          
		if (str[i]==symbol)
		{
			tk[cnt]=i;
			cnt++;
		}
	}
	
	if (index>cnt-1)
	{
		return NULL;
	}
			
	tk[0]=0;
	tk[cnt]=strlen((char *)str);
	
	if (index==0)
		start=0;
	else
		start=tk[index]+1;

	end=tk[index+1];
	
	j=0;
	for(i=start;i<end;i++)
	{
		tmp[j]=str[i];
		j++;
	}
		
	return (unsigned char *)tmp;
}

unsigned int getWLAN_ChipVersion()
{
	FILE *stream;
	char buffer[128];
	typedef enum { CHIP_UNKNOWN=0, CHIP_RTL8188C=1, CHIP_RTL8192C=2, CHIP_RTL8192D=3} CHIP_VERSION_T;
	CHIP_VERSION_T chipVersion = CHIP_UNKNOWN;	

	sprintf(buffer,"/proc/wlan%d/mib_rf",wlan_idx);
	stream = fopen (buffer, "r" );
	if ( stream != NULL )
	{		
		char *strtmp;
		char line[100];
								 
		while (fgets(line, sizeof(line), stream))
		{
			
			strtmp = line;
			while(*strtmp == ' ')
			{
				strtmp++;
			}
			

			if(strstr(strtmp,"RTL8192SE") != 0)
			{
				chipVersion = CHIP_UNKNOWN;
			}
			else if(strstr(strtmp,"RTL8188C") != 0)
			{
				chipVersion = CHIP_RTL8188C;
			}
			else if(strstr(strtmp,"RTL8192C") != 0)
			{
				chipVersion = CHIP_RTL8192C;
			}
			else if(strstr(strtmp,"RTL8192D") !=0)
			{
				chipVersion = CHIP_RTL8192D;
			}
		}			
		fclose ( stream );
	}

	return chipVersion;


}

int isFileExist(char *file_name)
{
	struct stat status;

	if ( stat(file_name, &status) < 0)
		return 0;

	return 1;
}

int SetWlan_idx(char * wlan_iface_name)
{
	int idx;
	
		idx = atoi(&wlan_iface_name[4]);
		if (idx >= NUM_WLAN_INTERFACE) {
				printf("invalid wlan interface index number!\n");
				return 0;
		}
		wlan_idx = idx;
		vwlan_idx = 0;
	
#ifdef MBSSID		
		
		if (strlen(wlan_iface_name) >= 9 && wlan_iface_name[5] == '-' &&
				wlan_iface_name[6] == 'v' && wlan_iface_name[7] == 'a') {
				idx = atoi(&wlan_iface_name[8]);
				if (idx >= NUM_VWLAN_INTERFACE) {
					printf("invalid virtual wlan interface index number!\n");
					return 0;
				}
				
				vwlan_idx = idx+1;
				idx = atoi(&wlan_iface_name[4]);
				wlan_idx = idx;
		}
#endif		

#ifdef UNIVERSAL_REPEATER
				if (strlen(wlan_iface_name) >= 9 && wlan_iface_name[5] == '-' &&
						!memcmp(&wlan_iface_name[6], "vxd", 3)) {
					vwlan_idx = NUM_VWLAN_INTERFACE;
					idx = atoi(&wlan_iface_name[4]);
					wlan_idx = idx;
				}
#endif				

//printf("\r\n wlan_iface_name=[%s],wlan_idx=[%u],vwlan_idx=[%u],__[%s-%u]\r\n",wlan_iface_name,wlan_idx,vwlan_idx,__FILE__,__LINE__);

return 1;		
}

short whichWlanIfIs(PHYBAND_TYPE_T phyBand)
{
	int i;
	int ori_wlan_idx=wlan_idx;
	int ret=-1;
	
	for(i=0 ; i<NUM_WLAN_INTERFACE ; i++)
	{
		unsigned char wlanif[10];
		memset(wlanif,0x00,sizeof(wlanif));
		sprintf((char *)wlanif, "wlan%d",i);
		if(SetWlan_idx((char *)wlanif))
		{
			int phyBandSelect;
			apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&phyBandSelect);
			if(phyBandSelect == phyBand)
			{
				ret = i;
				break;			
			}
		}						
	}
	
	wlan_idx=ori_wlan_idx;
	return ret;		
}

void swapWlanMibSetting(unsigned char wlanifNumA, unsigned char wlanifNumB)
{
	unsigned char *wlanMibBuf=NULL;
	unsigned int totalSize = sizeof(CONFIG_WLAN_SETTING_T)*(NUM_VWLAN_INTERFACE+1); // 4vap+1rpt+1root

//printf("\r\n __[%s-%u]\r\n",__FILE__,__LINE__);

#if CONFIG_APMIB_SHARED_MEMORY == 1
	apmib_sem_lock();
#endif

//printf("\r\n __[%s-%u]\r\n",__FILE__,__LINE__);

	wlanMibBuf = malloc(totalSize); 
#if 0	
	printf("\r\n wlanifNumA=[%u],__[%s-%u]\r\n",wlanifNumA,__FILE__,__LINE__);
	printf("\r\n wlanifNumB=[%u],__[%s-%u]\r\n",wlanifNumB,__FILE__,__LINE__);
	
	printf("\r\n pMib->wlan[wlanifNumA]=[0x%x],__[%s-%u]\r\n",pMib->wlan[wlanifNumA],__FILE__,__LINE__);
	printf("\r\n pMib->wlan[wlanifNumB]=[0x%x],__[%s-%u]\r\n",pMib->wlan[wlanifNumB],__FILE__,__LINE__);
	
	printf("\r\n pMib->wlan[0][0].wlanDisabled=[%u],__[%s-%u]\r\n",pMib->wlan[0][0].wlanDisabled,__FILE__,__LINE__);
	printf("\r\n pMib->wlan[0][0].phyBandSelect=[%u],__[%s-%u]\r\n",pMib->wlan[0][0].phyBandSelect,__FILE__,__LINE__);
	printf("\r\n pMib->wlan[0][0].channel=[%u],__[%s-%u]\r\n",pMib->wlan[0][0].channel,__FILE__,__LINE__);
	
	printf("\r\n pMib->wlan[1][0].wlanDisabled=[%u],__[%s-%u]\r\n",pMib->wlan[1][0].wlanDisabled,__FILE__,__LINE__);
	printf("\r\n pMib->wlan[1][0].phyBandSelect=[%u],__[%s-%u]\r\n",pMib->wlan[1][0].phyBandSelect,__FILE__,__LINE__);
	printf("\r\n pMib->wlan[1][0].channel=[%u],__[%s-%u]\r\n",pMib->wlan[1][0].channel,__FILE__,__LINE__);
#endif			
	if(wlanMibBuf != NULL)
	{
		memcpy(wlanMibBuf, pMib->wlan[wlanifNumA], totalSize);
		memcpy(pMib->wlan[wlanifNumA], pMib->wlan[wlanifNumB], totalSize);
		memcpy(pMib->wlan[wlanifNumB], wlanMibBuf, totalSize);

		free(wlanMibBuf);
	}

#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif

#if defined(WLAN_PROFILE)
	int profile_enabled_id1, profile_enabled_id2;
	int profile_num_id1, profile_num_id2;
	unsigned char *wlProfileBuf;

//printf("\r\n __[%s-%u]\r\n",__FILE__,__LINE__);

	apmib_get(MIB_PROFILE_ENABLED1, (void *)&profile_enabled_id1);
	apmib_get(MIB_PROFILE_ENABLED2, (void *)&profile_enabled_id2);
	apmib_get(MIB_PROFILE_NUM1, (void *)&profile_num_id1);
	apmib_get(MIB_PROFILE_NUM2, (void *)&profile_num_id2);

	apmib_set(MIB_PROFILE_ENABLED1, (void *)&profile_enabled_id2);
	apmib_set(MIB_PROFILE_ENABLED2, (void *)&profile_enabled_id1);
	apmib_set(MIB_PROFILE_NUM1, (void *)&profile_num_id2);
	apmib_set(MIB_PROFILE_NUM2, (void *)&profile_num_id1);

	totalSize = sizeof(WLAN_PROFILE_T)*MAX_WLAN_PROFILE_NUM;
	wlProfileBuf = malloc(totalSize);
	if(wlProfileBuf != NULL)
	{
		memcpy(wlProfileBuf, pMib->wlan_profile_arrary1, totalSize);
		memcpy(pMib->wlan_profile_arrary1, pMib->wlan_profile_arrary2, totalSize);
		memcpy(pMib->wlan_profile_arrary2, wlProfileBuf, totalSize);

		free(wlProfileBuf);
	}

//printf("\r\n __[%s-%u]\r\n",__FILE__,__LINE__);

#endif

#if 0	
	printf("\r\n pMib->wlan[0][0].wlanDisabled=[%u],__[%s-%u]\r\n",pMib->wlan[0][0].wlanDisabled,__FILE__,__LINE__);
	printf("\r\n pMib->wlan[0][0].phyBandSelect=[%u],__[%s-%u]\r\n",pMib->wlan[0][0].phyBandSelect,__FILE__,__LINE__);
	printf("\r\n pMib->wlan[0][0].channel=[%u],__[%s-%u]\r\n",pMib->wlan[0][0].channel,__FILE__,__LINE__);
	
	printf("\r\n pMib->wlan[1][0].wlanDisabled=[%u],__[%s-%u]\r\n",pMib->wlan[1][0].wlanDisabled,__FILE__,__LINE__);
	printf("\r\n pMib->wlan[1][0].phyBandSelect=[%u],__[%s-%u]\r\n",pMib->wlan[1][0].phyBandSelect,__FILE__,__LINE__);
	printf("\r\n pMib->wlan[1][0].channel=[%u],__[%s-%u]\r\n",pMib->wlan[1][0].channel,__FILE__,__LINE__);
#endif	
#ifdef UNIVERSAL_REPEATER
	int rptEnable1, rptEnable2;
	char rptSsid1[MAX_SSID_LEN], rptSsid2[MAX_SSID_LEN];
	
	memset(rptSsid1, 0x00, MAX_SSID_LEN);
	memset(rptSsid2, 0x00, MAX_SSID_LEN);
	
	apmib_get(MIB_REPEATER_ENABLED1, (void *)&rptEnable1);
	apmib_get(MIB_REPEATER_ENABLED2, (void *)&rptEnable2);
	apmib_get(MIB_REPEATER_SSID1, (void *)rptSsid1);
	apmib_get(MIB_REPEATER_SSID2, (void *)rptSsid2);

	apmib_set(MIB_REPEATER_ENABLED1, (void *)&rptEnable2);
	apmib_set(MIB_REPEATER_ENABLED2, (void *)&rptEnable1);
	apmib_set(MIB_REPEATER_SSID1, (void *)rptSsid2);
	apmib_set(MIB_REPEATER_SSID2, (void *)rptSsid1);
#endif
#if VLAN_CONFIG_SUPPORTED 
	unsigned char *vlanMibBuf=NULL;
	totalSize = sizeof(VLAN_CONFIG_T)*5; // 4vap+1root

#if CONFIG_APMIB_SHARED_MEMORY == 1
	apmib_sem_lock();
#endif

//printf("\r\n __[%s-%u]\r\n",__FILE__,__LINE__);

	vlanMibBuf = malloc(totalSize);
	if(vlanMibBuf != NULL)
	{
		memcpy(vlanMibBuf, pMib->VlanConfigArray+4, totalSize);
		memcpy(pMib->VlanConfigArray+4, pMib->VlanConfigArray+9, totalSize);
		memcpy(pMib->VlanConfigArray+9, vlanMibBuf, totalSize);

		free(vlanMibBuf);
	}

#if CONFIG_APMIB_SHARED_MEMORY == 1
		apmib_sem_unlock();
#endif
	
#endif
}

#if defined(CONFIG_REPEATER_WPS_SUPPORT) || defined(POWER_CONSUMPTION_SUPPORT)
WLAN_STATE_T lastWanState = WLAN_OFF;
WLAN_STATE_T updateWlanifState(char *wlanif_name)
{
	FILE *stream;
	int debug_check = 0;
	WLAN_STATE_T wlanState = WLAN_NO_LINK;
	unsigned char tmpStr[100];
	
	memset(tmpStr,0x00,sizeof(tmpStr));
	sprintf((char *)tmpStr,"/proc/%s/sta_info",wlanif_name);

	stream = fopen ((char *)tmpStr, "r" );
	if ( stream != NULL )
	{		
		char *strtmp;
		char line[100];
		while (fgets(line, sizeof(line), stream))
		{
			unsigned char *p;
			strtmp = line;
			
			while(*strtmp == ' ')
				strtmp++;
			
			if(strstr(strtmp,"active") != 0)
			{
				unsigned char str1[10], str2[10];
				
				//-- STA info table -- (active: 1)
				if(debug_check)
					fprintf(stderr,"\r\n [%s]",strtmp);
				
				sscanf(strtmp, "%*[^:]:%[^)]",str1);
				
				p = str1;
				while(*p == ' ')
					p++;										
				
				if(strcmp((char *)p,"0") == 0)
				{
					wlanState = WLAN_NO_LINK;
				}
				else
				{
					wlanState = WLAN_LINK;						
				}										
				
				break;
			}
				
		}
		fclose(stream );
		
	}
	
	if(wlanState == WLAN_LINK)
	{
		bss_info bss;

		getWlBssInfo(wlanif_name, &bss);

		if(bss.state != STATE_CONNECTED && bss.state != STATE_STARTED)
			wlanState = WLAN_NO_LINK;
		
	}
	
	memset(tmpStr,0x00,sizeof(tmpStr));
	if(lastWanState != wlanState)
	{
		lastWanState = wlanState;
		if(wlanState == WLAN_LINK)
			sprintf((char *)tmpStr,"echo \"%s LINK\" > /var/wlan_state",wlanif_name);
		else
			sprintf((char *)tmpStr,"echo \"%s NO_LINK\" > /var/wlan_state",wlanif_name);
		
		system((char *)tmpStr);
	}
	
	return wlanState;
		
}
#endif //#if defined(CONFIG_REPEATER_WPS_SUPPORT) || defined(POWER_CONSUMPTION_SUPPORT)

#if defined(CONFIG_RTL_P2P_SUPPORT)

int getWlP2PScanResult(char *interface, SS_STATUS_Tp pStatus )
{
#ifndef NO_ACTION
  int skfd=0;
  struct iwreq wrq;

  skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
    /* Get wireless name */
  if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
    /* If no wireless name : no wireless extensions */
    close( skfd );
      return -1;
	}
   
  wrq.u.data.pointer = (caddr_t)pStatus;

  if ( pStatus->number == 0 )
  	wrq.u.data.length = sizeof(SS_STATUS_T);
  else
    wrq.u.data.length = sizeof(pStatus->number);

  if (iw_get_ext(skfd, interface, SIOCP2PGETRESULT, &wrq) < 0){
  	close( skfd );
		return -1;
	}
    close( skfd );
#else
	return -1 ;
#endif

    return 0;
}

int getWlP2PScanRequest(char *interface, int *pStatus)
{
#ifndef NO_ACTION
  int skfd=0;
  struct iwreq wrq;
  unsigned char result;

//printf("\r\n __[%s-%u]\r\n",__FILE__,__LINE__);

  skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;

//printf("\r\n __[%s-%u]\r\n",__FILE__,__LINE__);

    /* Get wireless name */
  if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
    /* If no wireless name : no wireless extensions */
    close( skfd );
    return -1;
	}
  wrq.u.data.pointer = (caddr_t)&result;
  wrq.u.data.length = sizeof(result);

//printf("\r\n __[%s-%u]\r\n",__FILE__,__LINE__);

  if (iw_get_ext(skfd, interface, SIOCP2PSCANREQ, &wrq) < 0)
  {
  	close( skfd );
		return -1;
	}
  close( skfd );


  if ( result == 0xff )
  	*pStatus = -1;
  else
		*pStatus = (int) result;
#else
	*pStatus = -1;
#endif // #ifndef NO_ACTION

	return 0;

}

int getWlP2PStateEvent( char *interface, P2P_SS_STATUS_Tp pP2PStatus)
{
#ifndef NO_ACTION
  int skfd=0;
  struct iwreq wrq;

  skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
    /* Get wireless name */
  if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
    /* If no wireless name : no wireless extensions */
    close( skfd );
    return -1;
	}
  wrq.u.data.pointer = (caddr_t)pP2PStatus;
  wrq.u.data.length = sizeof(P2P_SS_STATUS_T);

  if (iw_get_ext(skfd, interface, SIOCP2PPGETEVNIND, &wrq) < 0){
  	 close( skfd );
  	 
		return -1;
	}     


	close( skfd );
#else
	*num = 0 ;
#endif

  return 0;
}


int getClientConnectState(void)
{

	static struct __p2p_state_event P2PStatus_t;
	memset(&P2PStatus_t , 0 ,sizeof(struct __p2p_state_event));

	int skfd=0;
	struct iwreq wrq;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return 0;
	
	/* Get wireless name */
	if ( iw_get_ext(skfd, "wlan0", SIOCGIWNAME, &wrq) < 0){
	    /* If no wireless name : no wireless extensions */
	    close( skfd );
	    return 0;
	}

	wrq.u.data.pointer = (caddr_t)&P2PStatus_t;
	wrq.u.data.length = sizeof(struct __p2p_state_event);

	if (iw_get_ext(skfd, "wlan0", SIOCP2P_REPORT_CLIENT_STATE, &wrq) < 0){
  		close( skfd );  	 
		return 0;
	}     
	close( skfd );

	return P2PStatus_t.p2p_status;
	
}


int sendP2PProvisionCommInfo( char *interface, P2P_PROVISION_COMM_Tp pP2PProvisionComm)
{
#ifndef NO_ACTION
  int skfd=0;
  struct iwreq wrq;

  skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
    /* Get wireless name */
  if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
    /* If no wireless name : no wireless extensions */
    close( skfd );
    return -1;
	}
  wrq.u.data.pointer = (caddr_t)pP2PProvisionComm;
  wrq.u.data.length = sizeof(P2P_PROVISION_COMM_T);

  if (iw_get_ext(skfd, interface, SIOCP2PPROVREQ, &wrq) < 0){
  	 close( skfd );
  	 
		return -1;
	}     


	close( skfd );
#else
	*num = 0 ;
#endif

  return 0;
}


int sendP2PWscConfirm( char *interface, P2P_WSC_CONFIRM_Tp pP2PWscConfirm)
{
#ifndef NO_ACTION
  int skfd=0;
  struct iwreq wrq;

  skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd==-1)
		return -1;
    /* Get wireless name */
  if ( iw_get_ext(skfd, interface, SIOCGIWNAME, &wrq) < 0){
    /* If no wireless name : no wireless extensions */
    close( skfd );
    return -1;
	}
  wrq.u.data.pointer = (caddr_t)pP2PWscConfirm;
  wrq.u.data.length = sizeof(P2P_WSC_CONFIRM_T);

  if (iw_get_ext(skfd, interface, SIOCP2WSCMETHODCONF, &wrq) < 0){
  	 close( skfd );
  	 
		return -1;
	}     


	close( skfd );
#else
	*num = 0 ;
#endif

  return 0;
}
#endif

int rmFile(char * path)
{
	struct stat sbuf;

	if(!path)
	{
		printf("%s:%d input path null!\n",__FUNCTION__,__LINE__);
		return -1;
	}
	bzero(&sbuf,sizeof(stat));
	if (stat(path, &sbuf)!=0)
	{
		printf("%s:%d can't stat '%s'\n",__FUNCTION__,__LINE__,path);
		return -1;
	}
	
	if(sbuf.st_mode & S_IFDIR)
	{
		DIR* pdir=opendir(path);		
		struct dirent *pdirItem=NULL;
		if(pdir==NULL)
		{	
			printf("%s:%d can't open dir '%s'\n",__FUNCTION__,__LINE__,path);
			return -1;
		}
		while((pdirItem=readdir(pdir))!=NULL)
		{
			char* tobeRmName = NULL;
			if(!strcmp(pdirItem->d_name,".")||!strcmp(pdirItem->d_name,".."))
			{
				continue;
			}
			tobeRmName= malloc(strlen(path)+strlen(pdirItem->d_name)+2);
			if(!tobeRmName) return;
			bzero(tobeRmName,(strlen(path)+strlen(pdirItem->d_name)+2));
			sprintf(tobeRmName,"%s/%s",path,pdirItem->d_name);
			if(rmFile(tobeRmName)<0)
			{
				printf("%s:%d rm %s fail\n",__FUNCTION__,__LINE__,tobeRmName);
				return -1;
			}
			if(tobeRmName) free(tobeRmName);
		}
		closedir(pdir);
		if(remove(path)<0)
		{			
			printf("%s:%d rm %s fail!\n",__FUNCTION__,__LINE__,path);
			return -1;
		}
	}else
	{
		if(remove(path)<0)
		{			
			printf("%s:%d rm %s fail!\n",__FUNCTION__,__LINE__,path);
			return -1;
		}
	}
	return 0;
}

int write_line_to_file(char *filename, int mode, char *line_data)
{
	unsigned char tmpbuf[512];
	int fh=0;

	if(mode == 1) {/* write line datato file */
		
		fh = open(filename, O_RDWR|O_CREAT|O_TRUNC);
		
	}else if(mode == 2){/*append line data to file*/
		
		fh = open(filename, O_RDWR|O_APPEND);	
	}
	
	
	if (fh < 0) {
		fprintf(stderr, "Create %s error!\n", filename);
		return 0;
	}


	sprintf(tmpbuf, "%s", line_data);
	write(fh, tmpbuf, strlen(tmpbuf));



	close(fh);
	return 1;
}

