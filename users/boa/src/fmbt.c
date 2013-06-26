/*This file handles BT webpage form request
  *
  */
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
#include "boa.h"
#include "asp_page.h"
#include "apmib.h"
#include "apform.h"
#include "utility.h"

#ifdef CONFIG_RTL_BT_CLIENT
/* Down Up Dir and Download Limit Upload limt and refresh time
  *Fiv MIB Save To Flash
  *Shell Script should call the dctcs(need to check dir exits)
  *
  */
void formBTBasicSetting(request *wp, char *path, char *query)
{
	char *downdir;
	char *updir;
	char *strptr;
	char *nextwebpage;
	int ulimit;
	int dlimit;
	int refreshtime;
	int enabled;
	int pid;
	char tmpBuf[256];
	nextwebpage=req_get_cstream_var(wp, ("nextwebpage"),"");
	downdir=req_get_cstream_var(wp, ("btdownloaddir"),"");
	updir=req_get_cstream_var(wp, ("btuploaddir"),"");
	char_replace(downdir,'\\', '/');
	char_replace(updir,'\\', '/');
	if(!dirExits(downdir) ||!dirExits(updir))
	{
		ERR_MSG("Directory Not Exists!!!");
		return;
	}
	apmib_set(MIB_BT_UPLOAD_DIR,updir);
	apmib_set(MIB_BT_DOWNLOAD_DIR,downdir);
	strptr=req_get_cstream_var(wp, ("totalulimit"),"");
	if(strptr)
		ulimit=atoi(strptr);
	apmib_set(MIB_BT_TOTAL_ULIMIT,&ulimit);
	strptr=req_get_cstream_var(wp, ("totaldlimit"),"");
	if(strptr)
		dlimit=atoi(strptr);
	apmib_set(MIB_BT_TOTAL_DLIMIT,&dlimit);
	strptr=req_get_cstream_var(wp, ("refreshtime"),"");
	if(strptr)
		refreshtime=atoi(strptr);
	apmib_set(MIB_BT_REFRESH_TIME,&refreshtime);
	strptr=req_get_cstream_var(wp, ("bt_enabled"),"");
	if(strptr)
		enabled=atoi(strptr);
	apmib_set(MIB_BT_ENABLED,&enabled);
	
	/*Save to flash*/
	apmib_update(CURRENT_SETTING);

	/*run dctcs shell*/
#ifndef NO_ACTION
	pid = fork();
        if (pid) {
	      	waitpid(pid, NULL, 0);
	}
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _BT_SCRIPT_PROG);
		execl(tmpBuf, _BT_SCRIPT_PROG, NULL);
             exit(1);
        }
#endif
	send_redirect_perm(wp, nextwebpage);
}

/*Index Format: /index_1/index_2/index_3......*/
unsigned char * getbtclientIndex(unsigned char *indexstr, int *index)
{
	unsigned char buffer[5];
	unsigned char *ptr=indexstr;
	unsigned char *tmpptr=buffer;
	
	if('/' == (*ptr))
		ptr++;
	else
		return NULL;
	
	while(*ptr && ( '/' != *ptr))
	{
		*tmpptr++=*ptr++;
	}
	*tmpptr=0;
	(*index)=atoi((char *)buffer);
	return ptr;
}
/*Show Bt Torrents and add/del/update.... 
  *
  */
void formBTClientSetting(request *wp, char *path, char *query)
{
	char *strptr;
	char *clientptr;
	char *tiptr;
	char *operation;
	char *nextwebpage;
	int index;
	nextwebpage=req_get_cstream_var(wp, ("nextwebpage"),"");
	operation=req_get_cstream_var(wp, ("operation"),"");
	//printf("operation value: %s\n",operation);
	/*get index index format /index_1/index_2/...*/
	clientptr=req_get_cstream_var(wp,("clientsindex"),"");
	tiptr=req_get_cstream_var(wp,("torrentsindex"),"");
	//printf("clientptr value: %s\n",clientptr);
	//printf("tiptr value: %s\n",tiptr);
	/*start del should use tiptr*/
	/*pause update adn quit shoud use clientptr*/
	/*start*/
	if(!strcmp(operation,"start"))
	{	
		/*start only one a time*/
		strptr=tiptr;
		while ((strptr=(char *)getbtclientIndex((unsigned char *)strptr,&index)))
		{
			if(index >= 0)
				bt_startTorrent(index);
		}
	}
	/*pause*/
	else if(!strcmp(operation,"pause"))
	{
		strptr=clientptr;
		while ((strptr=(char *)getbtclientIndex((unsigned char *)strptr,&index)))
		{
			if(index >= 0)
				bt_clientPause(index);
		}
	}
	/*stop*/
	else if(!strcmp(operation,"stop"))
	{
		strptr=clientptr;
		while ((strptr=(char *)getbtclientIndex((unsigned char *)strptr,&index)))
		{
			if(index >= 0)
				bt_clientQuit(index);
		}
	}
	/*update*/
	else if(!strcmp(operation,"update"))
	{
		strptr=clientptr;
		while ((strptr=(char *)getbtclientIndex((unsigned char *)strptr,&index)))
		{
			if(index >= 0)
				bt_clientUpdate(index);
		}
	}
	/*delete. delelte torrent or files*/
	else if(!strcmp(operation,"delete"))
	{
		strptr=tiptr;
		while ((strptr=(char *)getbtclientIndex((unsigned char *)strptr,&index)))
		{
			if(index >= 0)
				bt_deleteTorrent(index,0);
		}
	}
	else if(!strcmp(operation,"deleteallfiles"))
	{
		strptr=clientptr;
		while ((strptr=(char *)getbtclientIndex((unsigned char *)strptr,&index)))
		{
			if(index >= 0)
				bt_deleteTorrent(index,1);
		}
	}
	/*details*/
	else if(!strcmp(operation,"details"))
	{
		
	}
	/*info*/
	else if(!strcmp(operation,"info"))
	{
		
	}
	send_redirect_perm(wp,nextwebpage);
}
/*Setting BT files to Download
  *
  */
 void formBTFileSetting(request *wp, char *path, char *query)
{	
	char *strptr;
	char *filestr;
	char *nextwebpage;
	int len;
	int clientindex;
	char tmpbuf[128];
	
	nextwebpage=req_get_cstream_var(wp, ("nextwebpage"),"");
	strptr=req_get_cstream_var(wp,("clientindex"),"");
	if(strptr)
		clientindex=atoi(strptr);
	/*get fileindex*/
	filestr=req_get_cstream_var(wp,("selectedfiles"),"");
	/*get client index*/
	strptr=req_get_cstream_var(wp,("selectednum"),"");
	if(strptr)
		len=atoi(strptr);
	/*call setfile function*/
	bt_setfiles(clientindex, len,filestr);

	/*take a break ~!~*/
	sleep(1);
	strcpy(tmpbuf,nextwebpage);
	sprintf(tmpbuf+strlen(tmpbuf),"?ctorrent=%d",clientindex);
	send_redirect_perm(wp,tmpbuf);
}

/*New BT Torrent*/
void formBTNewTorrent(request *wp, char *path, char *query)
{
	char filepath[128];
	char *strptr;
	char *filename;
	char *nextwebpage;
	nextwebpage=req_get_cstream_var(wp, "submit-url","");
	strptr=req_get_cstream_var(wp,("filename"),"");
	char_replace(strptr,'\\','/');
	filename=strrchr(strptr, '/');
	if(filename == NULL)
	{
		printf("ERROR, filename NULL\n");
		return;
	}
	//printf("filename %s \n",filename);
	if(!apmib_get(MIB_BT_UPLOAD_DIR,filepath))
	{
		ERR_MSG("Get seeds directory failed");
		return;
	}
	if(!dirExits(filepath))
	{
		ERR_MSG("Seeds Directory Not Exists");
		return;
	}
	strcat(filepath,filename);
	//printf("filepath %s\n",filepath);
	bt_saveTorrentfile(filepath,wp->post_data, wp->post_data_len);
	send_redirect_perm(wp,nextwebpage);
}
#endif
