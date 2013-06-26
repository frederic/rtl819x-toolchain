#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/types.h>
#include <linux/netlink.h>

#include <sys/stat.h>
#include <sys/wait.h>

#include "built_time"

//DONE: H21
//TODO: apmib should be shared library
//DONE: libusb should be shared library
//TODO: get usb info via libusb
//DONE: tune ppp interface MTU, refer set_wan.c
		//RunSystemCmd(NULL_FILE, "ifconfig", "ppp0", "mtu", tmp_args, "txqueuelen", "25",NULL_STR);
//DONE: special parameter in usb3g.chat & enable.pin, enable.userpass
//DONE: not execute, while web apply reboot
//DONE: remove mac entry in the web -> wan -> status

//--------------------------------------------------------------
#define DEMON		1
#define OUTPUT		0

//--------------------------------------------------------------
#define FAILURE		0
#define SUCCESS		1

#define DEV_NONE    0
#define DEV_ADD		1
#define DEV_REMOVE	2

#define VENDOR		"Vendor"
#define PRODID		"ProdID"
#define LINUX_VID	"1d6b"

#define PPP_OPTION	"/var/usb3g.option"
#define PPP_CHAT	"/var/usb3g.chat"

#define MAX_RETRY   30
#define OFFHUB_THD  3

#define BUSYWAIT(X)	{while (X) { ; }}

enum functions {
	FUNC_NONE = 0,
	FUNC_SCSI,
	FUNC_TTY,
	FUNC_MAX
};

//int connect_type = 0;
int g_pwroff     = 0;

//--------------------------------------------------------------
#define DEF_BH_SHOWFLOW 0

#if DEF_BH_SHOWFLOW
#define BH_SHOWFLOW printf
#else
#define BH_SHOWFLOW(format, args...)
#endif

#define THIS_FILE "mnet.c"

//#define BH_DBG printf
#define BH_DBG(format, args...)
//BH_DBG("[bruce:%s:%d]\n", __FUNCTION__, __LINE__);


#define WORKAROUND_FOR_MPS 1
//--------------------------------------------------------------
struct usb3g_info_s {
	char pin[5];
	char apn[20];
	char dialnum[12];
	char user[32];
	char pass[32];
	char conntype[2];
	char idle[6];
	char mtu[6];
};

//--------------------------------------------------------------
int isFileExisted(char *filename)
{
	struct stat fst;
	if ( stat(filename, &fst) < 0)
		return FAILURE; //not exist
	return SUCCESS;
}

int isPPPRunning(void)
{
	FILE   *fp;
	char buffer[8];
	/* check ppp pid */
	fp = fopen("/var/run/ppp0.pid", "r");
	if (fp ==NULL)
		return FAILURE;
		
	if (fgets(buffer, sizeof(buffer),fp))
		if (-1 != getpgid(atoi(buffer)))
			return FAILURE;

	return SUCCESS;
}

int onoff_usbhub(void)
{
	int sys_ret = 0;

	BH_SHOWFLOW("[%s:%s:%d] turn off & on the power of hub\n", THIS_FILE, __FUNCTION__, __LINE__);
#if 1
    sys_ret = system("hub-ctrl -b 1 -d 1 -P 1 -p");
	sleep(1);
    sys_ret = system("hub-ctrl -b 1 -d 1 -P 1 -p 1");
	sleep(1);
#elif 0
	sys_ret = system("hub-ctrl -P 1");
	sleep(1);
	sys_ret = system("hub-ctrl -P 1 -p 1");
	sleep(1);
#else
	system("iwpriv wlan0 write_mem dw,b8021054,1,100000");
#endif
	return SUCCESS;
}

int need_a_poweroff(void)
{
	BH_SHOWFLOW("[%s:%s:%d]\n", THIS_FILE, __FUNCTION__, __LINE__);
	g_pwroff = 0;
	onoff_usbhub();
	return 0;
}

#define USB3G_CONN   1
#define USB3G_DISC   2
#define USB3G_INIT   3
#define USB3G_DIAL   4
#define USB3G_REMOVE 5

void export_status(int stat)
{
	switch (stat) {
	case USB3G_CONN:
		system("echo \"conn\" > /var/usb3g.stat");
		break;
	case USB3G_DISC:
		system("echo \"disc\" > /var/usb3g.stat");
		break;
	case USB3G_INIT:
		system("echo \"init\" > /var/usb3g.stat");
		break;
	case USB3G_DIAL:
		system("echo \"dial\" > /var/usb3g.stat");
		break;
	case USB3G_REMOVE:
		system("echo \"remove\" > /var/usb3g.stat");
		break;
	}
}

void kill_ppp_inet(void)
{
	system("killall ppp_inet >/dev/null 2>&1");
	system("killall pppd >/dev/null 2>&1");
	system("rm /etc/ppp/connectfile >/dev/null 2>&1");
}

int get_tty(char *ttyitf)
{
    FILE   *fp;
    char   buffer[64];
	int    sys_ret = 0;

	sys_ret = system("ls /sys/bus/usb-serial/devices > /var/tty-devices");
	fp = fopen("/var/tty-devices", "r");
	if (fp !=NULL && NULL != fgets(buffer, sizeof(buffer),fp)) {
		char *ptr = strstr(buffer, " ");
		if (ptr != NULL)
			return FAILURE;

		ptr = '\0';
		strcpy(ttyitf, buffer);
		BH_SHOWFLOW("[%s:%s:%d] first one is /dev/%s\n", THIS_FILE, __FUNCTION__, __LINE__, ttyitf);
		sys_ret = system("rm /var/tty-devices >/dev/null 2>&1");
		fclose(fp);

		return SUCCESS;
	}

	sys_ret = system("rm /var/tty-devices >/dev/null 2>&1");
	fclose(fp);
	return FAILURE;
}

int switch_to_modem(char *ttyitf)
{
    FILE   *fp;
    char   buffer[128];
	int    sys_ret = 0, idx = 0;

	if (FAILURE == isFileExisted("/var/usb_modeswitch.conf"))
		return FAILURE;

	export_status(USB3G_INIT);
	sys_ret = system("usb_modeswitch -c /var/usb_modeswitch.conf");

	if (get_tty(ttyitf)) {
		fp = fopen("/var/usb3g.tty", "r");
		if (fp==NULL)
			return FAILURE;

		fgets(buffer, sizeof(buffer),fp);
		sscanf(ttyitf, "ttyUSB%d", &idx);
		sprintf(ttyitf,"ttyUSB%d", idx+atoi(buffer));
		BH_SHOWFLOW("usb_modeswitch done! and ttyitf is [%s], idx[%d]\n", ttyitf, idx);
		sys_ret = system("rm /var/usb_modeswitch.conf >/dev/null 2>&1");
		fclose(fp);
		return SUCCESS;
	}

	return FAILURE;
}

/* TODO: apmib should be shared library */
int get_mib(char *val, char *mib)
{
	FILE *fp;
 	char buf[32];

	sprintf(buf, "flash get %s", mib);
    fp = popen(buf, "r");
	if (fp==NULL)
		return FAILURE;

	if (NULL == fgets(buf, sizeof(buf),fp)) {
		pclose(fp);
		return FAILURE;
	}

	strcpy(val, strstr(buf, "\"")+1);
	val[strlen(val)-2] = '\0';
	pclose(fp);
	return SUCCESS;
}

int get_mib_para(struct usb3g_info_s *info)
{
	get_mib(info->pin,		"USB3G_PIN");
	get_mib(info->apn,		"USB3G_APN");
	get_mib(info->dialnum,	"USB3G_DIALNUM");
	get_mib(info->user,		"USB3G_USER");
	get_mib(info->pass,		"USB3G_PASS");
	get_mib(info->conntype, "USB3G_CONN_TYPE");
	get_mib(info->idle,		"USB3G_IDLE_TIME");
	get_mib(info->mtu,		"USB3G_MTU_SIZE");
	
	return SUCCESS;
}

void gen_ppp_option(char *ttyitf)
{
    FILE *fp;
	struct usb3g_info_s info;
    char buf[128];
	char commentAPN[4];
#if WORKAROUND_FOR_MPS
	char commentCIMI[4];
	char vendorID[6];
	char productID[6];
#endif

	get_mib_para(&info);
	BH_SHOWFLOW("[%s:%s:%d] PPP parameter: apn[%s], dialnum[%s]\n", THIS_FILE, __FUNCTION__, __LINE__, info.apn, info.dialnum);

#if WORKAROUND_FOR_MPS
	get_device_info_from_proc(vendorID, productID);
#endif
	
    buf[0] = '\0';
	//if (!strcmp(info.dialnum, "#777"))
#if WORKAROUND_FOR_MPS
	if (strcmp(vendorID, "feed")) /* for mobilepeak */
#endif
		sprintf(buf+strlen(buf), "'OK' \"ATQ0 V1 E1  S0=0 &C0 &D2\"\n");
    if (strlen(info.pin) != 0)
        sprintf(buf+strlen(buf), "'OK' 'AT+CPIN=%s'\n", info.pin);
#if WORKAROUND_FOR_MPS
    if (!strcmp(vendorID, "feed")) /* for mobilepeak */
        sprintf(buf+strlen(buf), "'OK' 'AT%%B'\n");//TODO
#endif
    if (buf == NULL)
        sprintf(buf, "\n");

	commentAPN[0] = '\0';
	if (!strcmp(info.dialnum, "#777"))
		sprintf(commentAPN, "#");

#if WORKAROUND_FOR_MPS
	commentCIMI[0] = '\0';
	if (!strcmp(vendorID, "feed")) /* for mobilepeak */
		sprintf(commentCIMI, "#");
#endif

	fp = fopen(PPP_CHAT, "w");
	fprintf(fp,
			"ABORT 'NO DIAL TONE'\n"
			"ABORT 'NO ANSWER'\n"
			"ABORT 'NO CARRIER'\n"
			"ABORT DELAYED\n"
			"ABORT 'COMMAND NOT SUPPORT'\n"
			"\n"
			"'' ''\n"
			"'' 'ATZ'\n"
            "%s" //buf: PIN
			"\n"
			"SAY 'show device infomation...\\n'\n"
			"'OK' 'ATI'\n"
			"\n"
			"SAY 'show SIM CIMI...\\n'\n"
	#if WORKAROUND_FOR_MPS
			"%s" //commentCIMI, mobilepeak doesn't have cimi yet
	#endif
			"'OK' 'AT+CIMI'\n"
			"\n"
			"SAY 'set APN...\\n'\n"
			"%s" //commentAPN: enable APN or not
			"'OK' 'AT+CGDCONT=1,\"IP\",\"%s\"'\n"
			"\n"
			"SAY 'dial...\\n\\n'\n"
			"'OK' 'ATD%s'\n"
			"'CONNECT' ''\n",
            buf,
	#if WORKAROUND_FOR_MPS
			commentCIMI,
	#endif
			commentAPN, info.apn, info.dialnum);

	fclose(fp);

    buf[0] = '\0';
	if (atoi(info.conntype) == 1) {
        sprintf(buf+strlen(buf), "demand\n");
        sprintf(buf+strlen(buf), "idle %s\n", info.idle);
    }
    if (strlen(info.user) != 0)
        sprintf(buf+strlen(buf), "user %s\n", info.user);
    if (strlen(info.pass) != 0)
        sprintf(buf+strlen(buf), "password %s\n", info.pass);
    if (strlen(info.mtu) != 0) {
        sprintf(buf+strlen(buf), "mtu %s\n", info.mtu);
		sprintf(buf+strlen(buf), "mru %s\n", info.mtu);
	}
    if (buf == NULL)
        sprintf(buf, "\n");

	fp = fopen(PPP_OPTION, "w");
	fprintf(fp,
			//"-detach\n"
			"hide-password\n"
			"persist\n"
			"nodetach\n"
			"lcp-echo-interval 20\n"
			"lcp-echo-failure 3\n"
			"holdoff 2\n"
			"connect-delay 100\n"
			"noauth\n"
			"/dev/%s\n"
			"115200\n"
			"debug\n"
			"defaultroute\n"
			"ipcp-accept-local\n"
			"ipcp-accept-remote\n"
			"usepeerdns\n"
			"crtscts\n"
			"lock\n"
			"noccp\n"
			"noipdefault\n"
            "%s" //buf: ondemand, user, password, mtu
			"connect '/bin/chat -v -t10 -f %s'\n",
			ttyitf, buf, PPP_CHAT);

    fclose(fp);
}

int dial_ppp(void)
{
    char cmd[32], buf[4];
	int  sys_ret = 0, recheck = 5, connect_type = -1;
	
	get_mib(buf, "USB3G_CONN_TYPE");
	connect_type = atoi(buf);

	//sleep(1);
	
    if (connect_type == 2)
        return SUCCESS;

	export_status(USB3G_DIAL);
	//TODO: check ppp_inet
	kill_ppp_inet();
	sprintf(cmd, "ppp_inet -t 16 -c %d -x", connect_type);
printf("---> %s\n", cmd);
	sys_ret = system(cmd);

	/* check connect */
	while (recheck--)
	{
		if (SUCCESS == isFileExisted("/etc/ppp/link"))
			break;
		else
			sleep(1);
	}

	if (recheck<=0)
		export_status(USB3G_DISC);

	return SUCCESS;
}

int get_device_info_from_proc(char *vender, char *prodid)
{
	FILE *fp;
	char buffer[128];
    char *ptr  = NULL;

    fp = popen("cat /proc/bus/usb/devices", "r");
	if (fp==NULL)
		return FAILURE;

    while (NULL != fgets(buffer, sizeof(buffer),fp))
    {
        ptr = strstr(buffer, VENDOR);
        if (ptr) {
            if (strstr(buffer, LINUX_VID))
                /* if vender == LINUX, get next */
                continue;
            else {
                sscanf(ptr, "Vendor=%s ProdID=%s Rev*", vender, prodid);
                return SUCCESS;
            }
        }
    }
    pclose(fp);

	return FAILURE;
}

int exec_action_for_tty(int action, char *ttyitf)
{
	char buffer[64];
    char vender[6];
    char prodid[6];
	FILE *fp;
	int idx =0;

	if (action == DEV_ADD) {
		if (get_tty(ttyitf)) {
			if (FAILURE == isFileExisted("/var/usb3g.tty"))
			{
				get_device_info_from_proc(vender, prodid);
				sprintf(buffer, "usb_modeswitch -f %s:%s", vender, prodid);
				system(buffer);
				if (SUCCESS == isFileExisted("/var/usb3g.tty")) {
					fp = fopen("/var/usb3g.tty", "r");
					fgets(buffer, sizeof(buffer),fp);
					sscanf(ttyitf, "ttyUSB%d", &idx);
					sprintf(ttyitf,"ttyUSB%d", idx+atoi(buffer));
					fclose(fp);
				}
			}
			else {
				fp = fopen("/var/usb3g.tty", "r");
				fgets(buffer, sizeof(buffer),fp);
				sscanf(ttyitf, "ttyUSB%d", &idx);
				sprintf(ttyitf,"ttyUSB%d", idx+atoi(buffer));		
				fclose(fp);
			}
		}

		system("mdev -s");//
		sleep(2);//

		BH_SHOWFLOW("ttyitf[%s], idx[%d]\n", ttyitf, idx);
		gen_ppp_option(ttyitf);
		dial_ppp();
		return SUCCESS;
	}
    else if (action == DEV_REMOVE) {
        BH_SHOWFLOW("[%s:%s:%d] do nothing\n", THIS_FILE, __FUNCTION__, __LINE__);
		kill_ppp_inet();
		export_status(USB3G_REMOVE);
        return SUCCESS;
    }

	return FAILURE;
}

int exec_action(int action, int retry, char *ttyitf)
{
	/* MUST BE storage device !!! */
	if (action == DEV_ADD) {
		BH_SHOWFLOW("[%s:%s:%d] action == DEV_ADD\n", THIS_FILE, __FUNCTION__, __LINE__);
		if (switch_to_modem(ttyitf)) {
			gen_ppp_option(ttyitf);
			return dial_ppp();
		}
	}
	else if (action == DEV_REMOVE) {
		BH_SHOWFLOW("[%s:%s:%d] do nothing\n", THIS_FILE, __FUNCTION__, __LINE__);
		export_status(USB3G_REMOVE);
		return SUCCESS;
	}

	return FAILURE;
}

int find_profile(char *profile)
{
    FILE *fp;
    char buffer[128];
	int sys_ret = 0;

    fp = popen("ls -a /etc/usb_modeswitch.d", "r");
    while (NULL != fgets(buffer, sizeof(buffer),fp))
    {
        if (strstr(buffer, profile))
        {
            sprintf(buffer, "cp /etc/usb_modeswitch.d/%s /var/usb_modeswitch.conf", profile);
            sys_ret = system(buffer);
            pclose(fp);
			return SUCCESS;
        }
    }
    pclose(fp);
    return FAILURE;
}

#ifdef DEMON
char *id_pend_zero(char *id, char *buf)
{
	char *ptr = NULL;
	int pend_zero = 0, i;

	ptr = strstr(buf, "/");
	if (ptr) {
		ptr[0] = '\0';	

		pend_zero = 4-strlen(buf);
		strcpy(id+pend_zero, buf);
		if (pend_zero>0) {
			for (i = 0; i<pend_zero; i++)
				id[i] = '0';
		}
	}
	return ptr;
}

int parse_hotplug_info(char *buf, int len, char *devinfo, int *action)
{
    char *ptr;
	int i = 0, subsystem = 0, devtype = 0, product = 0;
	
	*action   = DEV_NONE;
	devtype   = 0;
	product   = 0;
	subsystem = 0;

	while (i < len) {
		if (*action == DEV_NONE) {
            if (strstr(buf+i,"ACTION=add"))
                *action = DEV_ADD;
            else if (strstr(buf+i,"ACTION=remove"))
                *action = DEV_REMOVE;
        }
		else if (subsystem == 0) {
            if (strstr(buf+i,"SUBSYSTEM=usb"))
                subsystem = 1;
		}
        else if (devtype == 0) {
			if (strstr(buf+i, "DEVTYPE=usb_device"))
				devtype = 1;
        }
        else if (*action != DEV_NONE && devtype == 1) {
            ptr = strstr(buf+i, "PRODUCT=");
            if (ptr) {
				char vid[5], pid[5];
				memset(vid, 0, sizeof(vid));
				memset(pid, 0, sizeof(pid));
				ptr = ptr+sizeof("PRODUCT=")-1;
				strncpy(devinfo, ptr, strlen(ptr));
				ptr = id_pend_zero(vid, devinfo);
				ptr = id_pend_zero(pid, ptr+1);		
				memset(devinfo, 0, sizeof(devinfo));
				sprintf(devinfo, "%s:%s\0", vid, pid);
				return find_profile(devinfo);
            }
        }

		i += strlen(buf+i) + 1;
	}

	return FAILURE;
}

int parse_hotplug_tty_info(char *buf, int len, char *devinfo, int *action)
{
	int i = 0, subsystem = 0, major = 0;

	*action   = DEV_NONE;
	while (i < len) {
		if (*action == DEV_NONE) {
            if (strstr(buf+i,"ACTION=add"))
                *action = DEV_ADD;
            else if (strstr(buf+i,"ACTION=remove"))
                *action = DEV_REMOVE;
        }
		else if (subsystem == 0) {
            if (strstr(buf+i,"SUBSYSTEM=tty"))
                subsystem = 1;
		}
        else if (major == 0) {
			if (strstr(buf+i, "MAJOR=188"))
				return SUCCESS;
        }

		i += strlen(buf+i) + 1;
	}

	return FAILURE;
}

int run_fork(int func, int action)
{
	int retry = MAX_RETRY;
	pid_t pid = 0;
	char ttyitf[8];

	BH_SHOWFLOW("[%s:%s:%d]\n", THIS_FILE, __FUNCTION__, __LINE__);
	
	if ((pid = fork())< 0)
		printf("process[%d] fail to fork child\n", getpid());
	else if (pid == 0) {
		if ((pid = fork())< 0)
			printf("process[%d] fail to fork child\n", getpid());
		else if (pid > 0)
			exit(0);
		else {
            if (SUCCESS == isFileExisted("/var/mnet_fork")) {
				BH_SHOWFLOW("[%s:%s:%d] /var/mnet_fork is existed, exit(0)\n", THIS_FILE, __FUNCTION__, __LINE__);
				exit(0);
            }
            else
				system("echo \"\" > /var/mnet_fork >/dev/null 2>&1");
			
			if (func == FUNC_SCSI)
				exec_action(action, retry, ttyitf);
			else if (func == FUNC_TTY)
				exec_action_for_tty(action, ttyitf);

			system("rm /var/mnet_fork >/dev/null 2>&1");
			exit(0);
		}
	}
	waitpid(pid, NULL, 0);
	return SUCCESS;
}

int run_fork_modeswitch(void)
{
	pid_t pid = 0;
	char vender[6];
	char prodid[6];
	char buffer[32];

	if ((pid = fork())< 0)
		printf("process[%d] fail to fork child\n", getpid());
	else if (pid == 0) {
		if ((pid = fork())< 0)
			printf("process[%d] fail to fork child\n", getpid());
		else if (pid > 0)
			exit(0);
		else {
			get_device_info_from_proc(vender, prodid);
			sprintf(buffer, "usb_modeswitch -c /etc/usb_modeswitch.d/%s:%s  >/dev/null 2>&1", vender, prodid);
			system(buffer);
			exit(0);
		}
	}
	waitpid(pid, NULL, 0);
	return SUCCESS;
}

int demon(void)
{
    struct sockaddr_nl nls;
    struct pollfd pfd;
    char buf[512], devinfo[20];
	int action = DEV_NONE;
	int func   = 0;
	int len    = 0;

	memset(&nls, 0, sizeof(nls));
	nls.nl_family = AF_NETLINK;
	nls.nl_pid = getpid();
	nls.nl_groups = -1;

	pfd.events = POLLIN;
	pfd.fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (pfd.fd == -1) {
		printf("Not root\n");
		exit(1);
	}

	if (bind(pfd.fd, (void*)&nls, sizeof(nls))) {
		printf("bind failed\n");
		exit(1);
	}

	if (g_pwroff == 1) {
		run_fork_modeswitch();
		//need_a_poweroff();
	}
	
	while (-1 != poll(&pfd, 1, -1)) {
		func = FUNC_NONE;
		len  = recv(pfd.fd, buf, sizeof(buf), MSG_DONTWAIT);
		if (len == -1) {
			printf("recv failed\n");
			//exit(1);
		}

		/* get device info */
		if (parse_hotplug_info(buf, len, devinfo, &action)) {
			BH_SHOWFLOW("\t*** Device %s[%d] ***\n", devinfo, action);
			func = FUNC_SCSI;
		}
		else if (parse_hotplug_tty_info(buf, len, devinfo, &action))
			func = FUNC_TTY;

		if (func==FUNC_SCSI || func==FUNC_TTY) {
			run_fork(func, action);
		}
	}
	printf("mnet demon end...\n");

	return SUCCESS;
}
#endif /* #ifdef DEMON */

#ifdef OUTPUT
int output(void)
{
    struct sockaddr_nl nls;
    struct pollfd pfd;
    char buf[512];

	memset(&nls, 0, sizeof(nls));
	nls.nl_family = AF_NETLINK;
	nls.nl_pid = getpid();
	nls.nl_groups = -1;

	pfd.events = POLLIN;
	pfd.fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (pfd.fd == -1) {
		printf("Not root\n");
		exit(1);
	}

	if (bind(pfd.fd, (void*)&nls, sizeof(nls))) {
		printf("bind failed\n");
		exit(1);
	}
	while (-1 != poll(&pfd, 1, -1)) {
		int i, len = recv(pfd.fd, buf, sizeof(buf), MSG_DONTWAIT);
		if (len == -1) {
			printf("recv failed\n");
			//exit(1);
		}
		i = 0;
		while (i < len) {
			printf("%s\n", buf + i);
			i += strlen(buf+i) + 1;
		}
	}
	printf("mnet output end...\n");

	return SUCCESS;
}
#endif /* #ifdef OUTPUT */

int main(int argc, char *argv[])
{
	printf("\nmnet built on %s\n\n", BUILT_TIME);

#ifdef DEMON
	if (argc > 1 && !strcmp(argv[1], "-d")) {
		char ttyitf[8];
		//char buf[4];

        system("mdev -s");
        sleep(2);
        
		/* if usb was learnt as tty device at bootup */
		if (SUCCESS == get_tty(ttyitf)) {
			run_fork(FUNC_TTY, DEV_ADD);
			sleep(1);
			BUSYWAIT(FAILURE == isFileExisted("/var/mnet_fork")); // wait here, till mnet_fork does not exist
  		}
		if (FAILURE == isFileExisted("/var/run/ppp0.pid")) {
			g_pwroff=1;
		}

		demon();
	}
#endif

#ifdef OUTPUT
	if (argc > 1 && !strcmp(argv[1], "-o"))
		output();
#endif

	if (argc > 1 && !strcmp(argv[1], "-p"))
		onoff_usbhub();

	if (argc > 1 && !strcmp(argv[1], "-r")) {
		if (isPPPRunning()) {
			//system("killall pppd >/dev/null 2>&1");
			kill_ppp_inet();
			sleep(3);
		}
		system("iwpriv wlan0 write_mem dw,b8021054,1,100000");
	}

	return 0;
}
