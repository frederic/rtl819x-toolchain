#include <stdio.h>
#include <errno.h> 
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/reboot.h>
#include <arpa/inet.h> 

#include "kernel_config.h"
#include "../../../goahead-2.1.1/LINUX/apmib.h"
#include "voip_manager.h"
#include "voip_debug.h"

#define FWUPDATE_FIFO "/var/run/fwupdate.fifo"

void print_msg(int dbg, unsigned char* module,unsigned char* string, ...)
{
	char buffer[100];
	va_list marker;
	
   	va_start( marker, string );
	vsprintf(buffer, string, marker);
	va_end( marker );
	
	rtk_print(dbg, module, buffer);
}


#define FILEPATH "/tmp/"
#define FWFILE_EXTNAME ".dat"
#define CHANGEVERSION "flash voip set VOIP.FW_UPDATE_FW_VERSION "

#define LOCK   	"#190#"
#define UPDATE 	"#160#"

#define STRINGLENGTH 50
#define LONGSTRINGLENGTH 100
#define WAITINGTIME 10

#define POWERON 0
#define SCHEDULE 1
#define BOTH 2

struct FileInfo{
	char fileName[STRINGLENGTH];
	char version[STRINGLENGTH];
};

static struct FileInfo allFile[5];
static char  *Readbuf;
static struct stat status;

static voip_flash_share_t *g_pVoIPShare;
static voipCfgParam_t* g_pVoIPCfg;

void sig_alm(int signo)
{
	reboot( RB_AUTOBOOT);
}

static int fwChecksumOk(char *data, int len)
{
	unsigned short sum=0;
	int i;

	for (i=0; i<len; i+=2) {
		sum += *((unsigned short *)&data[i]);
	}
	return( (sum==0) ? 1 : 0);
}

static int checkFile(char *fwFilename)
{	
	//write the firmware image to flash.
	int len, flag;
	FILE *fp=NULL;
	IMG_HEADER_Tp pHeader;
	int head_offset;
	int readsize;
	
        print_msg(RTK_DBG_INFO, "MISC", "Ready to check firmware file <%s>\n",fwFilename);
	
	len = flag = head_offset= readsize= 0;
	
	/*open the image file*/
	fp=fopen(fwFilename, "rb");
	if (fp == NULL) 
	{
                print_msg(RTK_DBG_WARNING, "MISC", "Image file open fail<%s>\n" , fwFilename );
		return -1;
	}
	/* count the size of the image */
	if(fstat(fileno(fp), &status) < 0){
                print_msg(RTK_DBG_WARNING,"MISC", "Error: stat failed\n");
		fclose(fp);
		return -1;
	}
	
	/* set the buffer */
	Readbuf =(char *)malloc(status.st_size);
       
        readsize = fread(Readbuf, 1, status.st_size, fp);
        if(readsize != status.st_size){
                print_msg(RTK_DBG_WARNING, "MISC","Error: fread failed\n");
                free(Readbuf);
		fclose(fp);
		unlink(fwFilename);
		return -1;
	}
        
	fclose(fp);
	unlink(fwFilename);

	//support multi header
	while(head_offset < status.st_size){
                print_msg(RTK_DBG_INFO, "MISC", "head_offset : %x sizeof fp %x \n", head_offset, status.st_size);
		
		if (!memcmp(&Readbuf[head_offset], FW_HEADER, SIGNATURE_LEN)||
			!memcmp(&Readbuf[head_offset], FW_HEADER_WITH_ROOT, SIGNATURE_LEN))
	  		flag=1;
		else if (!memcmp(&Readbuf[head_offset], WEB_HEADER, SIGNATURE_LEN))
    			flag = 2;
		else if (!memcmp(&Readbuf[head_offset], ROOT_HEADER, SIGNATURE_LEN))
    			flag = 3;
    		else {
                         print_msg(RTK_DBG_WARNING, "MISC", "Invalid file format! \n");
			free(Readbuf);
			return -1;	
		}
                print_msg(RTK_DBG_INFO, "MISC", "flag: %d\n", flag);
		
		/* get the header */
		pHeader = (IMG_HEADER_Tp) &Readbuf[head_offset];
	
		len = pHeader->len;
		
                
		if(len > 0x800000){ 
                        print_msg(RTK_DBG_WARNING, "MISC", "Image len exceed max size 0x800000 ! len=0x%x", len);
			free(Readbuf);
			return -1;
    		}
		if ( (flag == 1) || (flag == 3)) {
			if(!fwChecksumOk(&Readbuf[sizeof(IMG_HEADER_T) + head_offset],len)){	
                                print_msg(RTK_DBG_WARNING, "MISC", "firmware upgrade checksum fail 1 \n");
				free(Readbuf);
				return -1;
			}
		}else{
			if(!CHECKSUM_OK(&Readbuf[sizeof(IMG_HEADER_T) + head_offset],len) ){
                                print_msg(RTK_DBG_WARNING, "MISC", "firmware upgrade checksum fail 2\n");
				free(Readbuf);
				return -1;
			}
		}
		head_offset += (len + sizeof(IMG_HEADER_T));	
       	}
        print_msg(RTK_DBG_INFO, "MISC", " %s File Check OK! \n",fwFilename);
	return 0;
}

static int upgradeFirmware(int index)
{

	//write the firmware image to flash.
	int fh ,startAddr, numLeft, len, numWrite, flag;
	 fh = startAddr = numLeft = len = numWrite = flag = 0;

	IMG_HEADER_Tp pHeader;
	int head_offset=0;
	char command[LONGSTRINGLENGTH] = {0};
	
        print_msg(RTK_DBG_INFO, "MISC", "Ready to upgrade the new firmware <%d>\n", index);
	
	
	sprintf(command,"%s %s", CHANGEVERSION, allFile[index].version);
	system(command);
	//support multi header
	while(head_offset < status.st_size){
		
		if (!memcmp(&Readbuf[head_offset], FW_HEADER, SIGNATURE_LEN)||
			!memcmp(&Readbuf[head_offset], FW_HEADER_WITH_ROOT, SIGNATURE_LEN))
	  		flag=1;
		else if (!memcmp(&Readbuf[head_offset], WEB_HEADER, SIGNATURE_LEN))
    			flag = 2;
		else if (!memcmp(&Readbuf[head_offset], ROOT_HEADER, SIGNATURE_LEN))
    			flag = 3;
    		else {
                        print_msg(RTK_DBG_WARNING, "MISC", "Invalid file format!");
			free(Readbuf);
			return -1;	
		}
		
		/* get the header */
		pHeader = (IMG_HEADER_Tp) &Readbuf[head_offset];
	
		len = pHeader->len;
		
		if(flag == 3)
			fh = open(FLASH_DEVICE_NAME1, O_RDWR | O_SYNC);
		else
			fh = open(FLASH_DEVICE_NAME, O_RDWR | O_SYNC);
		
		if(fh == -1){
                        print_msg(RTK_DBG_WARNING, "MISC", "Error in opening flash deviece!\n");
			free(Readbuf);
			return -1;
		}
		/*get the burn address*/
		startAddr = pHeader->burnAddr;
                print_msg(RTK_DBG_INFO, "MISC", "start address is %x\n", startAddr);
                print_msg(RTK_DBG_INFO, "MISC", "ROOT_IMAGE_OFFSET is %x\n", ROOT_IMAGE_OFFSET);
                if(flag == 3){
                	//startAddr = 0;
                	startAddr -= ROOT_IMAGE_OFFSET;
                	if(startAddr < 0 )
				startAddr = 0;
                }
                print_msg(RTK_DBG_INFO, "MISC", "start address is %x\n", startAddr);
		
		numLeft = len + sizeof(IMG_HEADER_T) ;
		
		lseek(fh, startAddr, SEEK_SET);
		if(flag == 3)
		{
			numLeft -=  sizeof(IMG_HEADER_T);
			head_offset +=  sizeof(IMG_HEADER_T);		
		}
		
		numWrite = write(fh, &Readbuf[head_offset], numLeft);
			
		if(numWrite ==-1)
		{
                        print_msg(RTK_DBG_WARNING, "MISC", "numWrite ==-1 error: %s \n",strerror(errno));
		}
		numLeft -= numWrite;
		head_offset += numWrite;
		
                print_msg(RTK_DBG_INFO, "MISC", "Last numLeft %d numWrite %d \n ", numLeft, numWrite);   
		if(numLeft>0){
                	free(Readbuf);
                        print_msg(RTK_DBG_WARNING, "MISC", "Error in write F/W to memory\n");
                        close(fh);
                	return -1;
                }
                
                print_msg(RTK_DBG_INFO, "MISC", "head_offset : %x \n", head_offset);
        }
	close(fh);
	free(Readbuf);


        print_msg(RTK_DBG_INFO, "MISC", "Success Update F/W \n");
	
	return 0;
} 

static int fxsEvent(){
	
	print_msg(RTK_DBG_INFO, "MISC", "inFxsEvent() \n");
	
	int g_VoIPPorts;
	int i;
	SIGNSTATE val;
	char dtmf_val[] = {'0','1','2','3','4','5','6','7','8','9','0','*','#'};
	char input[20]={0};
	int index=0;
	
	//g_VoIPPorts = RTK_VOIP_SLIC_NUM(g_VoIP_Feature);
	g_VoIPPorts = RTK_VOIP_CH_NUM(g_VoIP_Feature);
	
	// when Status =1 ==> input
	int Status = 1;
	
	while(Status){
		for (i=0; i<g_VoIPPorts; i++)
		{
			if( !RTK_VOIP_IS_SLIC_CH( i, g_VoIP_Feature ) )
				continue;
				
			rtk_GetFxsEvent(i, &val);
			switch (val)
			{
				case SIGN_KEY1:
				case SIGN_KEY2:
				case SIGN_KEY3:
				case SIGN_KEY4:
				case SIGN_KEY5:
				case SIGN_KEY6:
				case SIGN_KEY7:
				case SIGN_KEY8:
				case SIGN_KEY9:
				case SIGN_KEY0:
				case SIGN_STAR:
				case SIGN_HASH:
					
					input[index]=dtmf_val[val];
					index++;
					if(Status == 1){
						if(!strncmp(LOCK,input,5))
						{
                                                        print_msg(RTK_DBG_INFO, "match %s\n",input);
							memset(input, 0 ,20);
							index = 0;
							
							rtk_SetPlayTone(i, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
							
							Status = 2;
						}
					}
					else// Status :2
					{
						if(!strncmp(UPDATE,input,5))
						{
                                                        print_msg(RTK_DBG_INFO, "match %s\n",input);
							Status = 0;
						}
					}
					break;
				case SIGN_ONHOOK:
					if(Status == 1){
						rtk_SetPlayTone(i, 0, DSPCODEC_TONE_ROH, 0, DSPCODEC_TONEDIRECTION_LOCAL);
						memset(input, 0 ,20);
						index = 0;
						rtk_Onhook_Action(i);
					}
					else{// Status :2
						rtk_Onhook_Action(i);
						return 0;
					}
					break;
				case SIGN_OFFHOOK:
					rtk_Offhook_Action(i);
					if(Status == 1){
						//rtk_SetRingFXS(i, 0);
						rtk_SetPlayTone(i, 0, DSPCODEC_TONE_ROH, 1, DSPCODEC_TONEDIRECTION_LOCAL);
					}
					
					break;
				case SIGN_NONE:
					break;
				case SIGN_OFFHOOK_2:
					break;
				default:
                                        print_msg(RTK_DBG_WARNING, "MISC", "unknown(%d)\n", val);
			}
	
			usleep(100000 / CON_CH_NUM); // 100 ms
		}
	}

	return 1;
}

static int cmd_killproc(int mode)
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
	
	printf("***%s:%s:%d***\n",__FILE__,__FUNCTION__,__LINE__);

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
		
		printf("kill webs \n");
		if ((fp = fopen(webPid, "r")) != NULL) {
			fgets(line, sizeof(line), fp);
			if ( sscanf(line, "%d", &pid) ) {
				if (pid > 1)
					kill(pid, SIGTERM);
			}
			fclose(fp);
			unlink("/bin/webs");
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

static int downloadFwFile(int fwNum)
{
	
	char SysCommand[LONGSTRINGLENGTH] = {0};
	char FwFile[STRINGLENGTH] = {0};
	char FullFwFile[STRINGLENGTH] = {0};
	int index = 0;
	
	signal(SIGALRM, sig_alm);
	
	cmd_killproc(1);
	cmd_killproc(0);
	
        print_msg(RTK_DBG_INFO, "MISC", "fwNum: %d \n", fwNum);
	
	
	while(index < fwNum){
		
		memset(    FwFile, 0, STRINGLENGTH);
		memset(FullFwFile, 0, STRINGLENGTH);
		memset(SysCommand, 0, LONGSTRINGLENGTH);
		
		sprintf(FwFile,"%s%s%s",allFile[index].fileName, allFile[index].version, FWFILE_EXTNAME);
		sprintf(FullFwFile,"%s%s",FILEPATH,FwFile);
		sprintf(SysCommand,"/bin/fwdownload.sh %s %s",FwFile,FullFwFile);
                print_msg(RTK_DBG_INFO, "SysCommand: %s \n", SysCommand);
		system(SysCommand);
		
		if(checkFile(FullFwFile) == -1)
		{
                        print_msg(RTK_DBG_WARNING, "MISC", "Error: file fail\n");
			index++;
			continue;
		}
		
		upgradeFirmware(index);
		index++;
	}

	
	alarm(2);
	return 0;
}

static int downloadVersion(){

	FILE *fp=NULL;
	unsigned char VerFile[STRINGLENGTH] = {0};
	unsigned char FullVerFile[STRINGLENGTH]={0};

	unsigned char SysCommand[LONGSTRINGLENGTH] = {0};
	unsigned char ReadFile[LONGSTRINGLENGTH]={0};
	
	unsigned char FwFile[STRINGLENGTH] = {0};
	unsigned char Ver[STRINGLENGTH]={0};
	
	
	unsigned long version, dev_version;
	
	int fwNum = 0;
	memset(allFile, 0 , 5 *sizeof(struct FileInfo));
	
	
        print_msg(RTK_DBG_INFO, "MISC", "start Download Version script\n");
	// x_ver.dat
	sprintf(VerFile,"%s%s", g_pVoIPCfg->fw_update_file_prefix,"_ver.dat");
	
	// /tmp/x_ver.dat
	sprintf(FullVerFile,"%s%s",FILEPATH,VerFile);
	
	sprintf(SysCommand,"/bin/fwdownload.sh %s %s",VerFile, FullVerFile);
	system(SysCommand);
	
        print_msg(RTK_DBG_INFO, "MISC", "command: %s \n", SysCommand);
	
        /*open the file*/
        fp = fopen(FullVerFile, "r");
        
	if (fp == NULL) 
	{
                print_msg(RTK_DBG_WARNING, "MISC", "Error: Image file open fail for VerFile\n");
                unlink(FullVerFile);
		return -1;
	}
	
       	
        while (fgets(ReadFile,LONGSTRINGLENGTH-1,fp)!=NULL){
                print_msg(RTK_DBG_INFO, "MISC","Read %s \n", ReadFile);
        	sscanf(ReadFile,"Version:%s NAME:%s",Ver,FwFile);
                print_msg(RTK_DBG_INFO, "MISC", "Version File: Version:%s NAME:%s \n", Ver ,FwFile);
        	
        	version = atol(Ver);
		dev_version = atol(g_pVoIPCfg-> fw_update_fw_version);
        	
                print_msg(RTK_DBG_INFO, "MISC", "Version File: %ld, Dev Version: %ld \n",version ,dev_version);
        	if(version <= dev_version)
        	{
                        print_msg(RTK_DBG_WARNING, "MISC", "Device Ver is newer  \n");
        		continue;
        	}
        	strcpy(allFile[fwNum].version, Ver);
        	strcpy(allFile[fwNum].fileName, FwFile);
        	
        	
        	
        	fwNum++;
        	if(fwNum == 5)
        		break;
        }
       
        int i = 0;
       	for(i = 0 ; i< fwNum; i++)
       	{
                print_msg(RTK_DBG_INFO, "MISC", "%d Name: %s Version %s \n", i, FullVerFile, allFile[i].version);
       	}
	fclose(fp);
        unlink(FullVerFile);
         
         if(fwNum > 0){
         	if(g_pVoIPCfg->fw_update_auto)
         		goto FWUPDATE;
	         else
	         {
	         	system("killall solar_monitor");
			system("killall solar");
			unlink("/bin/solar");
			unlink("/bin/solar_monitor");
		        if(fxsEvent())
		        {
		 		goto FWUPDATE;		
		        }
		        system("/bin/solar_monitor &");
	         	system("/bin/solar &");
	         }
         }
         print_msg(RTK_DBG_WARNING, "MISC", "Do not update\n");
       	 return 0;

FWUPDATE:
	downloadFwFile(fwNum);
    print_msg(RTK_DBG_INFO, "MISC", "FW update all finish\n");
	system("reboot");
	return 0;
}   

#define HTTP_AGENT_NAME         "RTK-FW" 
static int setFWupdateTimeToWeb(void){

	const char pHttpHeader[] = { "GET /goform/voip_fw_set HTTP/1.1\nUser-Agent: " HTTP_AGENT_NAME "\n\n" };
	unsigned char pHttpContent[ 512 ] = {0};
	
	int sockfd;
	struct sockaddr_in dest;
	int ret = 0;

	sprintf(pHttpContent, pHttpHeader);
	
	//======== socket ======//
	if( ( sockfd = socket( PF_INET, SOCK_STREAM, 0 ) ) == -1 )
		return 0;
	
	bzero(&dest, sizeof(dest));
	dest.sin_family = PF_INET;
	dest.sin_port = htons(80);
	inet_aton("localhost", &dest.sin_addr);
	
	if( connect(sockfd, ( struct sockaddr * )&dest, sizeof(dest)) == -1 ) {
		ret = 1;
		goto label_close_socket;
	}
	if( send(sockfd, pHttpContent, strlen( pHttpContent ), 0) == -1 ){
		ret = 1;
	}
	
	
label_close_socket:
	close( sockfd );

	return ret;
}


static int channelState(void){

	int chid;
	TstVoipCfg stVoipCfg = {0};
	
	int ret = 0;
	
	for(chid = 0 ;chid < CON_CH_NUM; chid++ )
	{
		if( !RTK_VOIP_IS_SLIC_CH( chid, g_VoIP_Feature ) )
			continue;
		
		stVoipCfg.ch_id = chid;
		rtk_Set_GetPhoneStat(&stVoipCfg);
		ret += stVoipCfg.cfg;
	}
	return ret;
}

static char *pidfile = "/var/run/fwupdate.pid";

static int pidfile_acquire(char *pidfile)
{
        int pid_fd;

        if(pidfile == NULL)
               return -1;

        pid_fd = open(pidfile, O_CREAT | O_WRONLY, 0644);
        if (pid_fd < 0)
               printf("Unable to open pidfile %s\n", pidfile);
        else
               lockf(pid_fd, F_LOCK, 0);

        return pid_fd;
}

static void pidfile_write_release(int pid_fd)
{
        FILE *out;

        if(pid_fd < 0)
               return;

        if((out = fdopen(pid_fd, "w")) != NULL) {
               fprintf(out, "%d\n", getpid());
               fclose(out);
        }
        lockf(pid_fd, F_UNLCK, 0);
        close(pid_fd);
}


int main(int argc, char *argv[])
{
	
	time_t now;
	int err;
	fd_set fdset;
	struct timeval tv;
	int h_max;
	int h_control;
	int download_time = 0;
    // destroy old process and create a PID file
    {
            int pid_fd;
            FILE *fp;
            char line[20];
            pid_t pid;

            if ((fp = fopen(pidfile, "r")) != NULL) {
                    fgets(line, sizeof(line), fp);
                    if (sscanf(line, "%d", &pid)) {
                           if (pid > 1){
                                   kill(pid, SIGTERM);
                           }
                    }
                    fclose(fp);
            }

            pid_fd = pidfile_acquire(pidfile);
            if (pid_fd < 0)
                    return 0;

		if (daemon(0,1) == -1) {
                printf("fork auth error!\n");
                exit(1);
        }

                    
            pidfile_write_release(pid_fd);
    }
	if (access(FWUPDATE_FIFO, F_OK) == -1)
	{
        	if (mkfifo(FWUPDATE_FIFO, 0755) == -1)
	        	print_msg(RTK_DBG_WARNING, "MISC", "access %s failed: %s\n", FWUPDATE_FIFO, strerror(errno));
	}
	
	h_control = open(FWUPDATE_FIFO, O_RDWR);
    	if (h_control == -1)
        	print_msg(RTK_DBG_WARNING, "MISC", "open %s failed\n", FWUPDATE_FIFO);
        
     
	if (voip_flash_client_init(&g_pVoIPShare, VOIP_FLASH_WRITE_CLIENT_FWUPDATE) == -1)
	{
                print_msg(RTK_DBG_WARNING, "MISC", "voip_flash_client_init failed.\n");
		return -1;
	}
	g_pVoIPCfg = &g_pVoIPShare->voip_cfg;
	
	if( (g_pVoIPCfg->fw_update_mode!=0) && g_pVoIPCfg ->fw_update_power_on != SCHEDULE)
	{
                print_msg(RTK_DBG_INFO, "MISC", "Powe on  to FW update \n");
		while(downloadVersion() && download_time++<3)
		{
			sleep(5);
		}
	}
	
	while(1){
		FD_ZERO(&fdset);
        	FD_SET(h_control, &fdset);
        	h_max = h_control;
		
		tv.tv_sec = WAITINGTIME;
		tv.tv_usec = 0;
		
		err = select(h_max + 1, &fdset, NULL, NULL, &tv);
		
		if ((err == -1) && (errno == EINTR || errno == EAGAIN))
			continue;
		if (err == 0) // timeout
		{
			time( &now );
			if( g_pVoIPCfg->fw_update_mode != 0 && g_pVoIPCfg ->fw_update_power_on != POWERON){
				if(now > g_pVoIPCfg ->fw_update_next_time)
				{
	                                print_msg(RTK_DBG_INFO, "MISC", "Time to update \n");
					if(channelState())
					{	
	                                        print_msg(RTK_DBG_WARNING, "MISC", "Some phones off-hook!! \n");
						continue;
					}
					//update next update time 		
					if(setFWupdateTimeToWeb()){
	                                        print_msg(RTK_DBG_WARNING, "MISC", " setFWupdateTimeToWeb fail \n");
						return -1;
					}
					downloadVersion();
				}
			}
		}
		else if (err > 0) // has input
		{
			if (FD_ISSET(h_control, &fdset))
			{
				char buffer[512];
				char *s;
				err = read(h_control, buffer, sizeof(buffer));
				if (err == 0) 
				{
					close(h_control);
					h_control = open(FWUPDATE_FIFO, O_RDWR);
					if (h_control == -1)
						print_msg(RTK_DBG_WARNING, "MISC", "open %s failed\n", FWUPDATE_FIFO);
				}
				else if (err > 0)
				{
					if (err == sizeof(buffer))
						print_msg(RTK_DBG_WARNING, "MISC", "buffer is full\n");
					else
						buffer[err] = 0; // add null terminated

					s = strtok(buffer, "\n");
					while(s)
					{
						voip_flash_client_update();			
						s = strtok(NULL, "\n");
					}	
				}
			}	
		}
		else
		{
			if ((err == -1) && (errno == EINTR || errno == EAGAIN))
				continue;
			else
				print_msg(RTK_DBG_WARNING, "MISC", "Error in select(): %s\n",strerror(errno));
		}
	}
	voip_flash_client_close();
	return 0;
}
