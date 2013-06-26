#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>

static char *tmp_file = "/tmp/tmp.txt";

//#define CBN_SPEC

#define _DHCPD_PID_PATH "/var/run"
#define _DHCPD_PROG_NAME "udhcpd"
#define _CONFIG_SCRIPT_PROG "init.sh"

#ifdef CONFIG_RTL8186_KB
#include <time.h>
#if 0
#define DEBUG_PRINT printf

#else
#define DEBUG_PRINT
#endif

typedef struct ps_info {
	unsigned int resv:2;
	unsigned int flag1:1;
	unsigned int flag2:1;
	unsigned int flag3:1;
	unsigned int flag4:1;
	unsigned int fTime:9;
	unsigned int tTime:9;
	unsigned int date:8;
}ps_info_T;

static int pre_enable_state = -1;

#else /*Main Trunk*/ 


#if 0
#define DEBUG_PRINT printf

#else
#define DEBUG_PRINT
#endif

#include <time.h>
typedef struct ps_info {
	unsigned int fTime;
	unsigned int tTime;
	unsigned int date;
}ps_info_T;

enum _WLAN_INTERFACE_ {
	WLAN_IF_ROOT,
	WLAN_IF_VXD,
	WLAN_IF_VA0,
	WLAN_IF_VA1,
	WLAN_IF_VA2,
	WLAN_IF_VA3,
	WLAN_IF_TOTAL
};

enum _ETH_INTERFACE_ {
	ETH_IF_0,
	ETH_IF_2,
	ETH_IF_3,
	ETH_IF_4,	
	ETH_IF_TOTAL
};

#define MAX_SCHEDULE_NUM 10 /* everyday, Mon~Sun */
#define MAX_WLAN_INT_NUM 2 /*we assume Max 2 WLAN interface */
static int pre_enable_state = -1;
static int pre_enable_state_new[MAX_WLAN_INT_NUM] = {-1,-1};

#endif // CONFIG_RTL8186_KB

#if 0
static int test_gpio(void)
{	
	FILE *fp;
	int i;
	fp=fopen("/proc/rf_switch","r");
	fscanf(fp,"%d",&i);
	fclose(fp); 
	return i;	
}
#endif

static int get_flash_int_value(char *keyword, int *pVal)
{
	char tmpbuf[100], *ptr;
	FILE *fp;
	
	sprintf(tmpbuf, "flash get %s > %s", keyword, tmp_file);
	system(tmpbuf);
		
	fp = fopen(tmp_file, "r");
	if (fp == NULL) {
		printf("read tmp file [%s] failed!\n", tmp_file);
		return 0;
	}
	fgets(tmpbuf, 100, fp);
	fclose(fp);	
	
	ptr = strchr(tmpbuf, '=');
	if (ptr == NULL) {
		printf("read %s value failed!\n", keyword);		
		return 0;			
	}
	
	*pVal = atoi(ptr+1);
	return 1;	
}

static is_wlan_exist(int index)
{
	char tmpbuf[100], *ptr;
	char wlanIntname[10];
	FILE *fp;
	
	sprintf(tmpbuf, "ifconfig > %s", tmp_file);
	system(tmpbuf);
	
	fp = fopen(tmp_file, "r");
	if (fp == NULL)
		return 0;
	sprintf(wlanIntname,"wlan%d",index);
	while (fgets(tmpbuf, 100, fp)) {
		ptr = strstr(tmpbuf, wlanIntname);	
		if (ptr) {
			if (strlen(ptr) <= 5)
				break;
			if (*(ptr+5) != '-')
				break;
		}
	}
	fclose(fp);
	
	return (ptr ? 1 : 0);
}

static void enable_wlan(int index) 
{
	int wlan_disabled;
	int wlan_wds_enabled=0;
	int wlan_wds_num=0;
	int i;
	char cmdBuffer[128];
#ifdef CONFIG_RTL8186_KB
	int guest_disabled;
#else
	int repeader_enabled;
	int guest0_disabled;
	int guest1_disabled;
	int guest2_disabled;
	int guest3_disabled;
#endif

//	DEBUG_PRINT("Enable wlan!\n");
	sprintf(cmdBuffer,"wlan%d WLAN_DISABLED",index);
	get_flash_int_value(cmdBuffer, &wlan_disabled);
	
	sprintf(cmdBuffer,"wlan%d WDS_ENABLED",index);
	get_flash_int_value(cmdBuffer, &wlan_wds_enabled);

	sprintf(cmdBuffer,"wlan%d WDS_NUM",index);
	get_flash_int_value(cmdBuffer, &wlan_wds_num);
	
#ifdef CONFIG_RTL8186_KB
	sprintf(cmdBuffer,"wlan%d-va0 WLAN_DISABLED",index);
	get_flash_int_value(cmdBuffer, &guest_disabled);
#else
	sprintf(cmdBuffer,"wlan%d-va0 WLAN_DISABLED",index);
	get_flash_int_value(cmdBuffer, &guest0_disabled);

	sprintf(cmdBuffer,"wlan%d-va1 WLAN_DISABLED",index);
	get_flash_int_value(cmdBuffer, &guest1_disabled);
	
	sprintf(cmdBuffer,"wlan%d-va2 WLAN_DISABLED",index);
	get_flash_int_value(cmdBuffer, &guest2_disabled);
	
	sprintf(cmdBuffer,"wlan%d-va3 WLAN_DISABLED",index);
	get_flash_int_value(cmdBuffer, &guest3_disabled);
	
	sprintf(cmdBuffer,"REPEATER_ENABLED%d",index+1);
	get_flash_int_value(cmdBuffer, &repeader_enabled);
#endif

	if (!wlan_disabled){
		sprintf(cmdBuffer,"ifconfig wlan%d up",index);
		system(cmdBuffer);
		if(wlan_wds_enabled==1){
				for(i=0;i<wlan_wds_num;i++){
					sprintf(cmdBuffer, "ifconfig wlan%d-wds%d up",index, i);
					system(cmdBuffer);
				}
		}
	}
#ifdef CONFIG_RTL8186_KB
	if (!guest_disabled) 
	{
		sprintf(cmdBuffer,"ifconfig wlan%d-va0 up",index);
		system(cmdBuffer);
	}
#else	

	if (!guest0_disabled) 
	{
		sprintf(cmdBuffer,"ifconfig wlan%d-va0 up",index);
		system(cmdBuffer); 
	}
	if (!guest1_disabled) 
	{
		sprintf(cmdBuffer,"ifconfig wlan%d-va1 up",index);
		system(cmdBuffer); 
	}
	if (!guest2_disabled) 
	{
		sprintf(cmdBuffer,"ifconfig wlan%d-va2 up",index);
		system(cmdBuffer); 
	}
	if (!guest3_disabled) 
	{
		sprintf(cmdBuffer,"ifconfig wlan%d-va3 up",index);
		system(cmdBuffer); 
	}
	if (repeader_enabled) 
	{
		sprintf(cmdBuffer,"ifconfig wlan%d-vxd up",index);
		system(cmdBuffer); 
	}
#endif	
}

static void disable_wlan(int index)
{
	int wlan_disabled;
	int wlan_wds_enabled=0;
	int wlan_wds_num=0;
	int i;
	char cmdBuffer[128];
#ifdef CONFIG_RTL8186_KB
	int guest_disabled;
#else
	int guest0_disabled;
	int guest1_disabled;
	int guest2_disabled;
	int guest3_disabled;
	int repeader_enabled;
#endif

//	DEBUG_PRINT("Disable wlan!\n");
	sprintf(cmdBuffer,"wlan%d WLAN_DISABLED",index);
	get_flash_int_value(cmdBuffer, &wlan_disabled);
	
	sprintf(cmdBuffer,"wlan%d WDS_ENABLED",index);
	get_flash_int_value(cmdBuffer, &wlan_wds_enabled);
	
	sprintf(cmdBuffer,"wlan%d WDS_NUM",index);
	get_flash_int_value(cmdBuffer, &wlan_wds_num);
	
#ifdef CONFIG_RTL8186_KB
	sprintf(cmdBuffer,"wlan%d-va0 WLAN_DISABLED",index);
	get_flash_int_value(cmdBuffer, &guest_disabled);
#else
	sprintf(cmdBuffer,"wlan%d-va0 WLAN_DISABLED",index);
	get_flash_int_value(cmdBuffer, &guest0_disabled);

	sprintf(cmdBuffer,"wlan%d-va1 WLAN_DISABLED",index);
	get_flash_int_value(cmdBuffer, &guest1_disabled);

	sprintf(cmdBuffer,"wlan%d-va2 WLAN_DISABLED",index);
	get_flash_int_value(cmdBuffer, &guest2_disabled);

	sprintf(cmdBuffer,"wlan%d-va3 WLAN_DISABLED",index);
	get_flash_int_value(cmdBuffer, &guest3_disabled);

	sprintf(cmdBuffer,"REPEATER_ENABLED%d",index+1);
	get_flash_int_value(cmdBuffer, &repeader_enabled);
#endif

	if (!wlan_disabled) {
		sprintf(cmdBuffer,"iwpriv wlan%d set_mib keep_rsnie=1",index);
		system(cmdBuffer);

		sprintf(cmdBuffer,"ifconfig wlan%d down",index);
		system(cmdBuffer);

		if(wlan_wds_enabled==1){
				for(i=0;i<wlan_wds_num;i++){
					sprintf(cmdBuffer, "ifconfig wlan%d-wds%d down",index, i);
					system(cmdBuffer);
				}
		}
	}

#ifdef CONFIG_RTL8186_KB
	if (!guest_disabled) {
		sprintf(cmdBuffer,"iwpriv wlan%d-va0 set_mib keep_rsnie=1",index);
		system(cmdBuffer);

		sprintf(cmdBuffer,"ifconfig wlan%d-va0 down",index);
		system(cmdBuffer);
	}
#else
	if (!guest0_disabled) {
		sprintf(cmdBuffer,"iwpriv wlan%d-va0 set_mib keep_rsnie=1",index);		
		system(cmdBuffer);
		
		sprintf(cmdBuffer,"ifconfig wlan%d-va0 down",index);
		system(cmdBuffer);
	}

	if (!guest1_disabled) {
		sprintf(cmdBuffer,"iwpriv wlan%d-va1 set_mib keep_rsnie=1",index);
		system(cmdBuffer);

		sprintf(cmdBuffer,"ifconfig wlan%d-va1 down",index);
		system(cmdBuffer);
	}

	if (!guest2_disabled) {
		sprintf(cmdBuffer,"iwpriv wlan%d-va2 set_mib keep_rsnie=1",index);		
		system(cmdBuffer);

		sprintf(cmdBuffer,"ifconfig wlan%d-va2 down",index);
		system(cmdBuffer);
	}

	if (!guest3_disabled) {
		sprintf(cmdBuffer,"iwpriv wlan%d-va3 set_mib keep_rsnie=1",index);
		system(cmdBuffer);

		sprintf(cmdBuffer,"ifconfig wlan%d-va3 down",index);
		system(cmdBuffer);
	}

	if (repeader_enabled) {
		sprintf(cmdBuffer,"iwpriv wlan%d-vxd set_mib keep_rsnie=1",index);
		system(cmdBuffer);

		sprintf(cmdBuffer,"ifconfig wlan%d-vxd down",index);
		system(cmdBuffer);
	}
#endif	
}

#if defined(CONFIG_RTL_8196B) || defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
static int test_gpio(void)
{
	FILE *fp;
	int i;
	fp=fopen("/proc/rf_switch","r");
	if (fp==NULL) return -1;
	fscanf(fp,"%d",&i);
	fclose(fp);
	return i;
}

// Read RF h/w switch (GPIO6) to see if need to disable/enable wlan interface
static int poll_rf_switch(void)
{
	static int rf_enabled=2;
	static int is_wlan_enabled=-1;
	int temp, wlan_disabled;
	temp = test_gpio();

	if  (temp==-1) return -1;
	if (rf_enabled == 2) { // first time
		rf_enabled = temp;
		if(rf_enabled ==1)
			return 1;
		if(rf_enabled ==0)
			return 2;  
	}

	if (temp != rf_enabled) {
		rf_enabled = temp;

		if (!get_flash_int_value("WLAN_DISABLED", &wlan_disabled))
			return -1;

		if (rf_enabled && !wlan_disabled){
				//	enable_wlan();
				return 1; //WLAN is enabled, but we should check the time match in advance
		}else{
			pre_enable_state = 0;
			disable_wlan(WLAN_IF_ROOT);
			return 2; //WLAN disabled 
		}
	}
#if 0 ///GPIO state no change, we donot modify wlan0 interface state, let schedule to modify the state
	else {
		if (is_wlan_enabled < 0) { // first time
			is_wlan_enabled = is_wlan0_exist();
			get_flash_int_value("WLAN_DISABLED", &wlan_disabled);
			printf("is wlan_enabled==%d\n", is_wlan_enabled);
			if (!wlan_disabled && is_wlan_enabled && temp == 0)
				disable_wlan();
			if(is_wlan_enabled==0)	
			return 0;	//first time check gpio status, but we check wlan by scheduld 
			if(is_wlan_enabled ==1)
				return 1;
		}
		//gpio status is not changed
		if(temp ==1)
			return 1;
		else if(temp ==0)
			return 2;
	}
#endif

	//gpio status is not changed
		if(temp ==1)
			return 1;
		else if(temp ==0)
			return 2;
		else
			return 0; //for compile warning only
}
#endif

#define ECO_SUNDAY_MASK		0x00000001
#define ECO_MONDAY_MASK		0x00000002
#define ECO_TUESDAY_MASK	0x00000004
#define ECO_WEDNESDAY_MASK	0x00000008
#define ECO_THURSDAY_MASK	0x00000010
#define ECO_FRIDAY_MASK		0x00000020
#define ECO_SATURDAY_MASK	0x00000040

#ifdef CONFIG_RTL8186_KB
static void dump_ps(struct ps_info *ps)
{
	char tmpbuf[200];


	DEBUG_PRINT("\nps_info=0x%lx\n", *((unsigned long *)ps));
		
	sprintf(tmpbuf, "date (%x): ", ps->date);
	if (ps->date & ECO_SUNDAY_MASK)
		strcat(tmpbuf, "Sunday ");	
	if (ps->date & ECO_MONDAY_MASK)
		strcat(tmpbuf, "Monday ");
	if (ps->date & ECO_TUESDAY_MASK)
		strcat(tmpbuf, "Tuesday ");
	if (ps->date & ECO_WEDNESDAY_MASK)
		strcat(tmpbuf, "Wednesday ");	
	if (ps->date & ECO_THURSDAY_MASK)
		strcat(tmpbuf, "Thursday ");		
	if (ps->date & ECO_FRIDAY_MASK)
		strcat(tmpbuf, "Friday ");
	if (ps->date & ECO_SATURDAY_MASK)
		strcat(tmpbuf, "Saturday ");
	strcat(tmpbuf, "\n");
	DEBUG_PRINT(tmpbuf);

	DEBUG_PRINT("From (%x): %dh %dm\n", ps->fTime, (ps->fTime >> 4)&0x1f, (ps->fTime & 0xf)*5);
	DEBUG_PRINT("To (%x): %dh %dm\n",  ps->tTime, (ps->tTime >> 4)&0x1f, (ps->tTime & 0xf)*5);
	DEBUG_PRINT("flag1: %x\n", ps->flag1);
	DEBUG_PRINT("flag2: %x\n", ps->flag2);
	DEBUG_PRINT("flag3: %x\n", ps->flag3);
	DEBUG_PRINT("flag4: %x\n\n", ps->flag4);
}


static void check_time_and_control_RF(struct ps_info *ps)
{
	int hit_date=0, hit_time=0;
	unsigned long start, end, current;		
	time_t tm;	
	struct tm tm_time;	

	time(&tm);	
	memcpy(&tm_time, localtime(&tm), sizeof(tm_time));

	DEBUG_PRINT(" tm_wday=%d, tm_hour=%d, tm_min=%d, tm_sec=%d\n", tm_time.tm_wday, 
					tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
	
	switch(tm_time.tm_wday) {
		case 0:
			if (ps->date & ECO_SUNDAY_MASK)
				hit_date = 1;
			break;
		case 1:
			if (ps->date & ECO_MONDAY_MASK)
				hit_date = 1;
			break;
		case 2:
			if (ps->date & ECO_TUESDAY_MASK)
				hit_date = 1;
			break;
		case 3:
			if (ps->date & ECO_WEDNESDAY_MASK)
				hit_date = 1;
			break;
		case 4:
			if (ps->date & ECO_THURSDAY_MASK)
				hit_date = 1;
			break;
		case 5:
			if (ps->date & ECO_FRIDAY_MASK)
				hit_date = 1;
			break;
		case 6:
			if (ps->date & ECO_SATURDAY_MASK)
				hit_date = 1;
			break;
	}

	if (ps->flag4)	 // 24 hr
		hit_time = 1;
	else {
		start =  ((ps->fTime >> 4)&0x1f)*3600 + ((ps->fTime & 0xf)*5)*60;
		end =  ((ps->tTime >> 4)&0x1f)*3600 + ((ps->tTime & 0xf)*5)*60;
		current = tm_time.tm_hour*3600 +  tm_time.tm_min*60 +   tm_time.tm_sec; 
		if (current > start && current < end)
			hit_time = 1;	

		DEBUG_PRINT("start=%d, end=%d, current=%d\n", start, end, current); // for debug
	}

	DEBUG_PRINT("pre_enable_state=%d, hit_date=%d, hit_time=%d\n", pre_enable_state, hit_date, hit_time);

	if (pre_enable_state < 0) { // first time
		if (hit_date && hit_time) {
			disable_wlan();
			pre_enable_state = 0;
		}
		else
			pre_enable_state = 1;	
	}
	else {
		if (pre_enable_state && hit_date && hit_time) {
			disable_wlan();
			pre_enable_state = 0;
		}
		else if (!pre_enable_state && (!hit_date || !hit_time)) {
			enable_wlan();		
			pre_enable_state = 1;
		}
	}
}
#else /*main trunk*/

static void parse_schl(char *argv, struct ps_info *ps)
{
	int i, head, tail, value[4];
	char tmpbuf[8];

	head = 0;
	for (i=0; i<4; i++) {
		tail = head + 1;
		while (argv[tail] != ',')
			tail++;
		memset(tmpbuf, 0, sizeof(tmpbuf));
		strncpy(tmpbuf, &(argv[head]), tail-head);
		value[i] = atoi(tmpbuf);
		head = tail + 1;
	}

	ps->fTime = value[1];
	ps->tTime = value[2];
	ps->date  = value[3];
}

static void dump_ps(struct ps_info *ps)
{
	char tmpbuf[200];

	sprintf(tmpbuf, "date (%x): ", ps->date);
	if (ps->date & ECO_SUNDAY_MASK)
		strcat(tmpbuf, "Sunday ");
	if (ps->date & ECO_MONDAY_MASK)
		strcat(tmpbuf, "Monday ");
	if (ps->date & ECO_TUESDAY_MASK)
		strcat(tmpbuf, "Tuesday ");
	if (ps->date & ECO_WEDNESDAY_MASK)
		strcat(tmpbuf, "Wednesday ");
	if (ps->date & ECO_THURSDAY_MASK)
		strcat(tmpbuf, "Thursday ");
	if (ps->date & ECO_FRIDAY_MASK)
		strcat(tmpbuf, "Friday ");
	if (ps->date & ECO_SATURDAY_MASK)
		strcat(tmpbuf, "Saturday ");
	strcat(tmpbuf, "\n");
	DEBUG_PRINT(tmpbuf);

	DEBUG_PRINT("From (%x): %dh %dm\n", ps->fTime, (ps->fTime / 60), (ps->fTime % 60));
	DEBUG_PRINT("To (%x): %dh %dm\n",  ps->tTime, (ps->tTime /60), (ps->tTime % 60));
}


static void check_time_and_control_RF(struct ps_info *ps, int wlan_if, int force_on)
{
	int hit_date=0, hit_time=0;
	unsigned int start, end, current;
	time_t tm;
	struct tm tm_time;

	if (force_on) {
		hit_date = 1;		
		hit_time = 1;			
	}	

	time(&tm);
	memcpy(&tm_time, localtime(&tm), sizeof(tm_time));

	DEBUG_PRINT(" tm_wday=%d, tm_hour=%d, tm_min=%d\n",
		tm_time.tm_wday, tm_time.tm_hour, tm_time.tm_min);

	switch(tm_time.tm_wday) {
		case 0:
			if (ps->date & ECO_SUNDAY_MASK)
				hit_date = 1;
			break;
		case 1:
			if (ps->date & ECO_MONDAY_MASK)
				hit_date = 1;
			break;
		case 2:
			if (ps->date & ECO_TUESDAY_MASK)
				hit_date = 1;
			break;
		case 3:
			if (ps->date & ECO_WEDNESDAY_MASK)
				hit_date = 1;
			break;
		case 4:
			if (ps->date & ECO_THURSDAY_MASK)
				hit_date = 1;
			break;
		case 5:
			if (ps->date & ECO_FRIDAY_MASK)
				hit_date = 1;
			break;
		case 6:
			if (ps->date & ECO_SATURDAY_MASK)
				hit_date = 1;
			break;
	}

	start = ps->fTime;
	end   = ps->tTime;
	current = tm_time.tm_hour * 60 + tm_time.tm_min;
	DEBUG_PRINT("start=%d, end=%d, current=%d\n", start, end, current);

	if (end >= start) {
		if ((current >= start) && (current < end))
			hit_time = 1;
	}
	else { 
		if ((current >= start) || (current < end))
			hit_time = 1;
	}

	DEBUG_PRINT("pre_enable_state=%d, hit_date=%d, hit_time=%d\n", pre_enable_state, hit_date, hit_time);

	if (pre_enable_state < 0) { // first time
		if (hit_date && hit_time) {
			pre_enable_state = 1;
			if (!is_wlan_exist(wlan_if))
				enable_wlan(wlan_if);			
		}
		else {
			disable_wlan(wlan_if);
			pre_enable_state = 0;
		}
	}
	else {
		if (!pre_enable_state && hit_date && hit_time) {
			enable_wlan(wlan_if);
			pre_enable_state = 1;
		}
		else if (pre_enable_state && (!hit_date || !hit_time)) {
			disable_wlan(wlan_if);
			pre_enable_state = 0;
		}
	}
}
#endif // CONFIG_RTL8186_KB

#define MAX_SCHEDULE_NUM 10 /* everyday, Mon~Sun */
#define MAX_WLAN_INT_NUM 2 /*we assume Max 2 WLAN interface */

void chk_wlanSch_ksRule(struct ps_info *ks, int index)
{
	int i=0;
	time_t tm;
	struct tm tm_time;
	struct ps_info *ps;
	int hit_date = 0;
	int hit_time = 0;
		
	time(&tm);
	memcpy(&tm_time, localtime(&tm), sizeof(tm_time));
	
	DEBUG_PRINT(" tm_wday=%d, tm_hour=%d, tm_min=%d\n", tm_time.tm_wday, tm_time.tm_hour, tm_time.tm_min);	

	for(i=0 ; i<MAX_SCHEDULE_NUM ; i++)
	{

		
		if(ks[i].date > 7) // this item is null
		{
			continue;
		}
		else
		{
			ps = &ks[i];
			if(ps->date == 7) // 7:everyday
			{
				hit_date = 1;
			}
			else if(ps->date == tm_time.tm_wday)
			{
				hit_date = 1;
			}
			
			DEBUG_PRINT("\r\n ps.date = [%u], ps.fTime = [%u], ps.tTime = [%u]__[%s-%u]\r\n",ps->date,ps->fTime,ps->tTime,__FILE__,__LINE__);

			int start = ps->fTime;
			int end = ps->tTime;
			int current = tm_time.tm_hour * 60 + tm_time.tm_min;
			DEBUG_PRINT("start=%d, end=%d, current=%d\n", start, end, current);
		
			if (end >= start) {
				if ((current >= start) && (current < end))
					hit_time = 1;
			}
			else {
				if ((current >= start) || (current < end))
					hit_time = 1;
			}			

			if(hit_date && hit_time)
				break;
			
		}								
	}
	
	DEBUG_PRINT("pre_enable_state=%d, hit_date=%d, hit_time=%d\n", pre_enable_state_new[index], hit_date, hit_time);											
	if (pre_enable_state_new[index] < 0) { // first time
		if (hit_date && hit_time) {
			pre_enable_state_new[index] = 1;
			if (!is_wlan_exist(index))
				enable_wlan(index);
		}
		else {
			disable_wlan(index);
			pre_enable_state_new[index] = 0;
		}
	}
	else {
		if (!pre_enable_state_new[index] && hit_date && hit_time) {
			enable_wlan(index);
			pre_enable_state_new[index] = 1;
		}
		else if (pre_enable_state_new[index] && (!hit_date || !hit_time)) {
			disable_wlan(index);
			pre_enable_state_new[index] = 0;
		}
	}
	DEBUG_PRINT("%s %d\n",__FUNCTION__,__LINE__);
}

#ifdef CBN_SPEC
int getPid(char *filename)
{
        struct stat status;
        char buff[100];
        FILE *fp;

        if ( stat(filename, &status) < 0)
                return -1;
        fp = fopen(filename, "r");
        if (!fp) {
                return -1;
        }
        fgets(buff, 100, fp);
        fclose(fp);

        return (atoi(buff));
}

void killSomeDaemon(void)
{
	system("killall -9 sleep 2> /dev/null");
	system("killall -9 routed 2> /dev/null");
//	system("killall -9 pppoe 2> /dev/null");
//	system("killall -9 pppd 2> /dev/null");
//	system("killall -9 pptp 2> /dev/null");
	system("killall -9 dnrd 2> /dev/null");
	system("killall -9 ntpclient 2> /dev/null");
//	system("killall -9 miniigd 2> /dev/null");	//comment for miniigd iptables rule recovery
	system("killall -9 lld2d 2> /dev/null");
//	system("killall -9 l2tpd 2> /dev/null");	
//	system("killall -9 udhcpc 2> /dev/null");	
//	system("killall -9 udhcpd 2> /dev/null");	
//	system("killall -9 reload 2> /dev/null");		
	system("killall -9 iapp 2> /dev/null");	
	system("killall -9 wscd 2> /dev/null");
	system("killall -9 mini_upnpd 2> /dev/null");
	system("killall -9 iwcontrol 2> /dev/null");
	system("killall -9 auth 2> /dev/null");
	system("killall -9 disc_server 2> /dev/null");
	system("killall -9 igmpproxy 2> /dev/null");
	system("killall -9 syslogd 2> /dev/null");
	system("killall -9 klogd 2> /dev/null");
	
	system("killall -9 ppp_inet 2> /dev/null");
	
#ifdef VOIP_SUPPORT
	system("killall -9 snmpd 2> /dev/null");
	system("killall -9 solar_monitor 2> /dev/null");
	system("killall -9 solar 2> /dev/null");
	system("killall -9 dns_task 2> /dev/null");
	system("killall -9 ivrserver 2> /dev/null");
#endif

#ifdef CONFIG_SNMP
	system("killall -9 snmpd 2> /dev/null");
#endif
}

void run_init_script(char *arg)
{
	int pid=0;
	char tmpBuf[100];
	
#ifdef RTK_USB3G
	system("killall -9 mnet 2> /dev/null");
	system("killall -9 hub-ctrl 2> /dev/null");
	system("killall -9 usb_modeswitch 2> /dev/null");
	system("killall -9 ppp_inet 2> /dev/null");
	system("killall -9 pppd 2> /dev/null");
	system("rm /etc/ppp/connectfile >/dev/null 2>&1");
#endif /* #ifdef RTK_USB3G */

#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
	Stop_Domain_Query_Process();
	Reset_Domain_Query_Setting();
#endif

	snprintf(tmpBuf, 100, "%s/%s.pid", _DHCPD_PID_PATH, _DHCPD_PROG_NAME);
	pid = getPid(tmpBuf);
	if ( pid > 0)
		kill(pid, SIGUSR1);
		
	usleep(1000);
	
	if ( pid > 0){
		system("killall -9 udhcpd 2> /dev/null");
		system("rm -f /var/run/udhcpd.pid 2> /dev/null");
	}
	//Patch: kill some daemons to free some RAM in order to call "init.sh gw all" more quickly
	//which need more tests especially for 8196c 2m/16m
	killSomeDaemon();
	system("killsh.sh");	// kill all running script	

// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
	web_restart_solar();
#endif

	pid = fork();
/*	
       	if (pid)
               	waitpid(pid, NULL, 0);
   	else 
*/ 
	if (pid == 0) {
#ifdef HOME_GATEWAY
		sprintf(tmpBuf, "%s gw %s", _CONFIG_SCRIPT_PROG, arg);
#elif defined(VOIP_SUPPORT) && defined(ATA867x)
		sprintf(tmpBuf, "%s ATA867x %s", _CONFIG_SCRIPT_PROG, arg);
#else
		sprintf(tmpBuf, "%s ap %s", _CONFIG_SCRIPT_PROG, arg);
#endif
		sleep(1);
		system(tmpBuf);
		exit(1);
	}
}

void sigHandler_swreinit(int signo)
{
	#define REINIT_WEB_FILE		"/tmp/reinit_web"
	struct stat status;
	int reinit=1;

	if (stat(REINIT_WEB_FILE, &status) == 0) { // file existed
        unlink(REINIT_WEB_FILE);
		reinit = 0;		
	}	
	if (reinit) { // re-init system
#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT)
	   Start_Domain_Query_Process=0;
#endif
#ifndef NO_ACTION
		run_init_script("all");
#endif		
	}
}
#endif	//CBN_SPEC

int main(int argc, char *argv[])
{
	FILE *fp;
	FILE *fp1;
	int gpio_state=-1;
	int last_gpio_state=-1;
	unsigned char keepalive[5];
	keepalive[0]='8';
	keepalive[1]='8';
	keepalive[2]='8';
	int i;
	unsigned char line[20];

#ifdef CONFIG_RTL8186_KB
	struct ps_info ps;
	unsigned long val;

	static int ntp_success=0;

	memset(&ps, '\0', sizeof(ps));
	if (argc > 2 && !strcmp(argv[1], "-e")) {		
		sscanf(argv[2], "%lu", &val);		
		memcpy(&ps, &val,  4);
		dump_ps(&ps);
	}
	if (ps.flag1)
		system("echo u > /proc/gpio");
	else
		system("echo y > /proc/gpio");
#else /*main trunk*/ 
	struct ps_info ps;
	struct ps_info ks[MAX_WLAN_INT_NUM][MAX_SCHEDULE_NUM];
	static int wlan_schl=0, wlan_schk=0, ntp_enable=0, ntp_success=0;
	static int wlan_schkmulti[MAX_WLAN_INT_NUM]={0};
	
	memset(&ps, '\0', sizeof(ps));
	if (argc > 2) {
		for (i=0; i<argc; i++) {
			if (!strcmp(argv[i], "-e")) {
				parse_schl(argv[i+1], &ps);
				dump_ps(&ps);
				wlan_schl = 1;
			}
			if (!strcmp(argv[i], "-k")) 
			{
				FILE *fp;
				char line[200];
				unsigned char filename[20];
				unsigned char tmpfilename[20];

				
				memset(&ks, 0xff, sizeof(ps_info_T)*MAX_SCHEDULE_NUM*MAX_WLAN_INT_NUM);
				memset(filename, 0x00, sizeof(filename));
				sprintf(filename,"%s",argv[++i]);

				if(strlen(filename) == 0)
				{
					DEBUG_PRINT("get wlan schedule file failed!\n");										
				}
				else
				{
					for(i=0;i<MAX_WLAN_INT_NUM;i++)
					{
						sprintf(tmpfilename,"%s%d",filename,i);
						DEBUG_PRINT("filename (%s)\n",tmpfilename);
						fp = fopen(tmpfilename, "r");
						if (fp == NULL)
						{
							DEBUG_PRINT("open wlan schedule file failed!\n");										
						}
						else
						{
							while ( fgets(line, 200, fp) )
							{
								int idx;
								char *str;

								if(strlen(line) != 0)
								{
									char *lineptr = line;
									//strcat(line,",");
									
									str = strsep(&lineptr,",");
																	
									idx = atoi(str);

									idx--;
									
									ks[i][idx].date = atoi(strsep(&lineptr,","));
									ks[i][idx].fTime = atoi(strsep(&lineptr,","));
									ks[i][idx].tTime  = atoi(strsep(&lineptr,","));
									DEBUG_PRINT(" idx=[%u],date=[%u],fTime=[%u],tTime=[%u]__[%s-%u]\r\n",idx,ks[i][idx].date,ks[i][idx].fTime,ks[i][idx].tTime,__FILE__,__LINE__);
									wlan_schk = 1;
									wlan_schkmulti[i]=1;
								}
							}
							
						}
					}
				}
			}
#if defined(CONFIG_RTL865X_KLD)
			if (!strcmp(argv[i], "-f")) {
				parse_schl(argv[i+1], &ps_va0);
				dump_ps(&ps_va0);
				wlan_va0_schl = 1;
			}
			if (!strcmp(argv[i], "-g")) {
				parse_schl(argv[i+1], &ps_eth);
				dump_ps(&ps_eth);			
			}
			if (!strcmp(argv[i], "-p")) {
				sscanf(argv[i+1], "%d", &eth_port);	
			}			
#endif
		}
	}	get_flash_int_value("NTP_ENABLED", &ntp_enable);
#endif
#ifdef CBN_SPEC
	sprintf(line, "%d\n", getpid());
	if ((fp = fopen("/var/run/rc.pid", "w")) == NULL) {
		printf("Can't create PID file!");
		return -1;
	}
	fwrite(line, strlen(line), 1, fp);
	fclose(fp);

	signal(SIGUSR1, sigHandler_swreinit);
#endif	//CBN_SPEC	
	while(1)
	{  	  
#if defined(CONFIG_RTL_8196B)
#else
		fp=fopen("/proc/load_default","r");
		if(fp)	//add by LZQ to check if file be opened success
		{
			fscanf(fp,"%d",&i);
			if (i==1) 
			{   
		    	printf("Going to Reload Default\n");
				system("flash reset");
				system("reboot");
			}
			else if (i==2)
			{   
				system("reboot");
			}
			fclose(fp); 
		}
		else
		{
			//printf("<%s>LZQ: Open  /proc/load_default file error!\n", __FUNCTION__);
		}
#endif
		
#ifdef CONFIG_RTL8186_KB
		if (!ntp_success)
		{
			fp=fopen("/etc/TZ","r");
			if (fp) 
			{
				fclose(fp);
				ntp_success = 1;
			}
		}
		if (ntp_success && ps.flag2) // enable power-saving
			check_time_and_control_RF(&ps);
#else	/*main trunk*/
		if (ntp_enable) {
			if (!ntp_success) {
				fp = fopen("/etc/TZ", "r");
				if (fp) {
					fclose(fp);
					ntp_success = 1;
				}
			}
		}
		if (wlan_schl && (!ntp_enable || ntp_success)) { // enable wlan scheduling
			check_time_and_control_RF(&ps, WLAN_IF_ROOT, 0);
		}
#endif
  		//#else //LZQ
#if defined(CONFIG_RTL8186_TR) || defined(CONFIG_RTL865X_SC)
		fp1 = fopen("/proc/gpio","w");
		fwrite(keepalive,1, 3, fp1);
		fclose(fp1);
#endif  

#if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E)
		/* rtl8196C or rtl8198 do not need to check rf pin status*/
		gpio_state = 1;
		last_gpio_state =gpio_state; 
#else
		gpio_state = poll_rf_switch();
#endif
		DEBUG_PRINT("wlan_schk (%d) wlan_schkmulti (%d) (%d)\n",wlan_schk,wlan_schkmulti[0],wlan_schkmulti[1]);
		//printf("the gpio state =%d\n", gpio_state);
		if(last_gpio_state != gpio_state)
		{
			if(last_gpio_state == -1){ ///This is first state, we should check in advance for schedule, if the gpio is enabled
				if(gpio_state ==1 )
				{
#if !defined(CONFIG_RTL865X_AP)	
					if (ntp_enable) {
						if (!ntp_success) {
							fp = fopen("/etc/TZ", "r");
							if (fp) {
								fclose(fp);
								ntp_success = 1;
							}
						}
					}
#endif
						// enable wlan scheduling
					if ((wlan_schl || wlan_schk) && (!ntp_enable || ntp_success))
					{
						if(wlan_schk == 1)
						{
							for(i=0;i<MAX_WLAN_INT_NUM;i++)
								if(wlan_schkmulti[i])
									chk_wlanSch_ksRule(&ks[i],i);						
						}
						else
						check_time_and_control_RF(&ps, WLAN_IF_ROOT, 0);
					}
					else if ((wlan_schl || wlan_schk) && ntp_enable && !ntp_success)
					{
						if(wlan_schk == 1)
						{
							for(i=0;i<MAX_WLAN_INT_NUM;i++)
								if(wlan_schkmulti[i])
									chk_wlanSch_ksRule(&ks[i],i);
						}
						else
						check_time_and_control_RF(&ps, WLAN_IF_ROOT, 0);
				        }
				}
			}else {  ///It is not first state, we get the result from gpio state
					if(gpio_state ==1 ){ //gpio state is available for time schedule match
						#if !defined(CONFIG_RTL865X_AP)	
								if (ntp_enable) {
									if (!ntp_success) {
										fp = fopen("/etc/TZ", "r");
										if (fp) {
											fclose(fp);
											ntp_success = 1;
										}
									}
								}
						#endif
                                        // enable wlan scheduling
					if ( (wlan_schl || wlan_schk) && (!ntp_enable || ntp_success))
					{						
						if(wlan_schk == 1)
						{
							for(i=0;i<MAX_WLAN_INT_NUM;i++)
								if(wlan_schkmulti[i])
									chk_wlanSch_ksRule(&ks[i],i);
						}
						else
						check_time_and_control_RF(&ps, WLAN_IF_ROOT, 0);
					}
					else if ( (wlan_schl || wlan_schk) && ntp_enable && !ntp_success)
					{						
						if(wlan_schk == 1)
						{
							for(i=0;i<MAX_WLAN_INT_NUM;i++)
								if(wlan_schkmulti[i])					
									chk_wlanSch_ksRule(&ks[i],i);
						}
						else
						check_time_and_control_RF(&ps, WLAN_IF_ROOT, 0);
					}
					else
						enable_wlan(WLAN_IF_ROOT);
						}
			}
			last_gpio_state = gpio_state;
		}
		else
		{
				if(gpio_state ==1 ){
				#if !defined(CONFIG_RTL865X_AP)	
						if (ntp_enable) {
							if (!ntp_success) {
								fp = fopen("/etc/TZ", "r");
								if (fp) {
									fclose(fp);
									ntp_success = 1;
								}
							}
						}
				#endif
				// enable wlan scheduling
				if ( (wlan_schl || wlan_schk) && (!ntp_enable || ntp_success))
				{
					if(wlan_schk == 1)
					{
						for(i=0;i<MAX_WLAN_INT_NUM;i++)
								if(wlan_schkmulti[i])
									chk_wlanSch_ksRule(&ks[i],i);
					}
					else
					check_time_and_control_RF(&ps, WLAN_IF_ROOT, 0);
				}
				else if ((wlan_schl || wlan_schk) && ntp_enable && !ntp_success)
				{
					if(wlan_schk == 1)
					{
						for(i=0;i<MAX_WLAN_INT_NUM;i++)
								if(wlan_schkmulti[i])
									chk_wlanSch_ksRule(&ks[i],i);
					}
					else
						check_time_and_control_RF(&ps, WLAN_IF_ROOT, 0);
			}
				}
		}

		sleep(3);
	}
}
	
