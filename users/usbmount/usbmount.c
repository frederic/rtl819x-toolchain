/* 
USB Mount Handler
andrew 2008/02/21

handles linux 2.6 hotplug event
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
//#include <linux/config.h>
//#include <config/autoconf.h>
#include <sys/mount.h>
#include <sys/time.h>
//#define  CONFIG_CT_USBMOUNT_DIRECTORY
/*

Environment Variables:

ARGS 1
DEVPATH
ACTION

Partition Numbering..
sda		0
sda1		1
.....
sda15	15
sdb		16
sdb1		17
.....
sdb15	31


*/

//#undef devel_debug 
#define devel_debug 1

#ifdef devel_debug
static char cmd_bufferx[200];
static unsigned int cmd_counter = 0;
#endif

#if 1
#define ASSERT(x)  if (!(x)) fprintf(stderr, "Assert fail at %s:%d\n", __FUNCTION__, __LINE__)
#define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#else
#define ASSERT(x)
#define DEBUG(...)
#endif
#define NTFS_3G 1

static char CMD_UMOUNT_FMT[] = "/bin/umount %s";
static char CMD_MOUNT_FMT[] = "/bin/mount -t %s /dev/%s %s";
#if NTFS_3G
//static char  CMD_MOUNT_FMT_NTFS[] = "/bin/ntfs-3g /dev/%s %s -o silent,umask=0000";
static char  CMD_MOUNT_FMT_NTFS[] = "/bin/ntfs-3g /dev/%s %s -o force";
static char CMD_MOUNT_FMT_NTFS_KILL[] = "/etc/killntfs %s/%s%d";
#endif
static char CMD_MKDIR[]="mkdir %s";
// configure this!!
//static char MNT_ROOT[] = "/tmp/usb"; // directory where sda,sda1.. directory will be created for mounting.
#ifdef CONFIG_CT_USBMOUNT_DIRECTORY
#define MNT_ROOT "/mnt"
static char MNT_FILE[] = "/tmp/usb/mnt_map"; // file contains 32bit INT representing mounted paritions.
#else
#define MNT_ROOT "/var/tmp/usb"
static char MNT_FILE[] = MNT_ROOT"/mnt_map"; // file contains 32bit INT representing mounted paritions.
#endif

struct action_table {
	char *action_str;
	int	(*action_func)(int, char **, char *);
};

unsigned int file_read_number(const char *filename) 
{
	FILE *fp;
	char buffer[10];
	unsigned int mask = 0;
	
	fp = fopen(filename, "r");
	
	if (!fp) {		
		return 0;
	}

	fgets(buffer, sizeof(buffer), fp);
	if (1 != sscanf(buffer, "%u", &mask)) {
		goto out;
	}
	fclose(fp);
	return mask;
	
out:
	fclose(fp);
	return 0;
	
}




static const char *basename(const char *path) {	
	const char *s = path, *tmp;
	DEBUG("%s(1)\n", __FUNCTION__);
	
	#if 0//def devel_debug
		snprintf(cmd_bufferx, sizeof(cmd_bufferx), 
			"echo \"BASE:%s \" >> /tmp/log", 
			 __FUNCTION__);
		system(cmd_bufferx);
                #endif
	
	
	while ((tmp = strchr(s, '/'))!= 0) {
		DEBUG("%s(2 %s)\n", __FUNCTION__, tmp);
		
		#if 0//def devel_debug
		snprintf(cmd_bufferx, sizeof(cmd_bufferx), 
			"echo \"2 BASE:%s \" >> /tmp/log", 
			 tmp);
		system(cmd_bufferx);
                #endif
		
		
		s = &tmp[1];
	}
	DEBUG("%s(3 %s)\n", __FUNCTION__, s);
	if (strlen(s))
		return s;
	return 0;
}

static int _get_partition_id(const char *dev, const char *root) {
	char p;
	if (strncmp(dev, root, strlen(root)) == 0) {
		p = dev[3];
		if (p)
			return (p - '0');
		else
			return 0;
	}
	return -1;
}
void put_debug_message(char *message)
{
	char cmdBuff[512]={0};
	
sprintf(cmdBuff, "echo \"%s\" >> /var/usbmount_debug", message);
system(cmdBuff);

}
static unsigned int get_partition_id(const char *dev) {
	int tmp;
	int rv=0; 
	unsigned char char_start=0;
	unsigned char dev_str[4]={'s','d',0,0};
	unsigned int index=0;
#if 1
	for(char_start='a';char_start<='z';char_start++){
		dev_str[2]=char_start;
		index=index+1;
		if(index % 2 ==0){
			if (strcmp(dev_str, dev)==0){
					return 1 << 16;
			}	
		if(!strncmp(dev, dev_str, strlen(dev_str))){
			rv = sscanf(dev+strlen(dev_str), "%u", &tmp);
			if (rv> 0 ) {
				return 1 << (tmp+16);
			}	
		}
			
		}else{
			if (strcmp(dev_str, dev)==0){
					return 1;
			}	
			if(!strncmp(dev, dev_str, strlen(dev_str))){
				rv = sscanf(dev+strlen(dev_str), "%u", &tmp);
				if (rv> 0 ) {
					return 1 << tmp;
				}	
			}
		}
	}
	
#else
	if (strcmp("sda", dev)==0)
		return 1;

	rv = sscanf(dev, "sda%u", &tmp);
	if (rv==1) {
		return 1 << tmp;
	}
	
	if (strcmp("sdb", dev)==0)
		return 1 << 16;
	
	rv = sscanf(dev, "sdb%u", &tmp);
	if (rv==1) {
		return 1 << (tmp + 16);
	}
#endif
	/*
	tmp = _get_partition_id(dev, "sda");
	ASSERT(tmp >= 0);
	if (tmp >= 0) {
		return 1 << tmp;
	}

	
	tmp = _get_partition_id(dev, "sdb");
	ASSERT(tmp >= 0);
	if (tmp >= 0) {
		return 1 << (tmp + 16);
	}
	*/

	return 0;
}

static unsigned int get_umount_partition_id(const char *dev) {
	FILE * fp;
	char buffer[10];
	unsigned int mask = 0;
	int ret=0;
	
	/*
	fp = fopen(MNT_FILE, "r");
	if (!fp) {		
		return ret;
	}

	fgets(buffer, sizeof(buffer), fp);
	sscanf(buffer, "%u", &mask);
	fclose(fp);
	*/
	mask = file_read_number(MNT_FILE);
	

	if(!strcmp(dev, "sda")){
		ret = mask&0xffff;
	}else if(!strcmp(dev, "sdb")){
		ret = mask&0xffff0000;
	} else {
		ret = mask;
	}
	
	return ret;
}

static int try_mount(const char *devnode, const char *mnt) {
	char cmd_buffer[220];
	const char *fstypes[] = { "vfat", "ntfs", 0 };
	const char *fstype;
	int rv, idx;
	//int retry = 0;
	struct timeval expiry, now;
	DEBUG("%s(1) %s, %s\n", __FUNCTION__, devnode, mnt);

	//do {
		
	for (idx = 0; fstypes[idx] != 0; idx++) {
		char source[80];
		fstype =fstypes[idx];
		snprintf(source, sizeof source, "/dev/%s", devnode);

		gettimeofday(&expiry, 0);		
		expiry.tv_sec += 1; // expire in 1 sec..
		srand(expiry.tv_usec);
		do {
#if NTFS_3G
	if(idx==0)
	{			
		rv = mount(source, mnt, fstype, 
			 MS_NODIRATIME | MS_NOATIME, 
			0);
  	}
	else
        {
		snprintf(cmd_buffer, sizeof(cmd_buffer), CMD_MOUNT_FMT_NTFS,devnode,mnt);
	  DEBUG("CMD: %s\n\n", cmd_buffer);
	        rv = system(cmd_buffer);
		#ifdef devel_debug
		snprintf(cmd_bufferx, sizeof(cmd_bufferx), 
			"echo \"CMD:%s ret %d \" >> /tmp/log", 
			 cmd_buffer,rv);
		system(cmd_bufferx);
                #endif
        }
#else
	rv = mount(source, mnt, fstype,
                         MS_NODIRATIME | MS_NOATIME,
                        0);
#endif
		if (rv==0)
			break;
		usleep(rand() % 500000);
		gettimeofday(&now, 0);
		} while(timercmp(&expiry, &now, >));
		/*
		snprintf(cmd_buffer, sizeof(cmd_buffer), CMD_MOUNT_FMT,
			fstype, devnode, mnt);
		rv = system(cmd_buffer);
		*/
		//ASSERT(rv != 127);
		//ASSERT(rv >= 0);
		if (rv == 0)
			return rv;
		else
			rv = errno;		

		#ifdef devel_debug
		snprintf(cmd_bufferx, sizeof(cmd_bufferx), 
			"echo \"(%u) E mount %s to %s(%s) ret %d \" >> /tmp/log", 
			++cmd_counter, devnode, mnt, fstype,  rv);
		system(cmd_bufferx); 
		#endif
	}
	
	//usleep(10000);
	//}while (retry++ < 10);
	return rv;

	#if 0
	snprintf(cmd_buffer, sizeof(cmd_buffer), CMD_MOUNT_FMT,
		"vfat",
		devnode,
		mnt);

	DEBUG("CMD: %s\n\n", cmd_buffer);
	rv = system(cmd_buffer);
	ASSERT(rv != 127);
	ASSERT(rv >= 0);
	fprintf(stderr, "mount returned %d\n", rv);

	if (rv == 0)
		return rv;

	snprintf(cmd_buffer, sizeof(cmd_buffer), CMD_MOUNT_FMT,
		"ntfs",
		devnode,
		mnt);
	
	DEBUG("CMD: %s\n\n", cmd_buffer);
	rv = system(cmd_buffer);
	ASSERT(rv != 127);
	ASSERT(rv >= 0);
	fprintf(stderr, "mount returned %d\n", rv);
	return rv;
	#endif
	
}

static unsigned int _mount(unsigned int parition_map, const char *base, const char *mnt_path) {
	char dev_buffer[10];
	char mnt_buffer[30];
	int tmp;
	unsigned int mnt_map = 0;
#ifdef CONFIG_CT_USBMOUNT_DIRECTORY
	char usbnum = (!strcmp(base,"sda")) ? 1 : 2;
#endif

	if (0==parition_map)
		return 0;
	
	strcpy(dev_buffer, base);
#ifdef CONFIG_CT_USBMOUNT_DIRECTORY
	snprintf(mnt_buffer, sizeof(mnt_buffer), "%s/usb%d_1", mnt_path, usbnum);
#else
	snprintf(mnt_buffer, sizeof(mnt_buffer), "%s/%s", mnt_path, base);
#endif

	for (tmp = 0; tmp < 16; tmp++) {
		if (!(parition_map & (1 << tmp)))
			continue;
		if (tmp) {
			snprintf(dev_buffer, sizeof(dev_buffer), "%s%d", base, tmp);
#ifdef CONFIG_CT_USBMOUNT_DIRECTORY
			snprintf(mnt_buffer, sizeof(mnt_buffer), "%s/usb%d_%d", mnt_path, usbnum, tmp);
#else
			snprintf(mnt_buffer, sizeof(mnt_buffer), "%s/%s%d", mnt_path, base, tmp);
#endif
		
		mkdir(mnt_buffer, 0755);
		if (try_mount(dev_buffer, mnt_buffer)==0) {
			mnt_map |= (1 << tmp);
		} else {
			rmdir(mnt_buffer); 
		}
		}else{
			if(tmp==0){
				snprintf(dev_buffer, sizeof(dev_buffer), "%s", base);
				snprintf(mnt_buffer, sizeof(mnt_buffer), "%s/%s", mnt_path, base);
				mkdir(mnt_buffer, 0755);
				if (try_mount(dev_buffer, mnt_buffer)==0) {
					mnt_map |= (1 << tmp);
				}else{
					rmdir(mnt_buffer); 
				}
			}
		}
	}

	return mnt_map;
}


static unsigned int fs_mount(unsigned int parition_id,const char *dev) {
	unsigned int mnt_map = 0;
	unsigned char char_start=0;
	unsigned int index=0;
	unsigned char dev_str[4]={'s','d',0,0};
	//parition_id &= ~10001;
#if 0
	mnt_map = _mount(parition_id & 0xffff, "sda", MNT_ROOT);
	mnt_map |= _mount(parition_id >> 16, "sdb", MNT_ROOT) << 16;
#else
	for(char_start='a';char_start<='z';char_start++){
		dev_str[2]=char_start;
		index=index+1;
		if(index % 2 ==0){
			if (strncmp(dev, dev_str, strlen(dev_str))==0){
				mnt_map |= _mount(parition_id >> 16, dev_str, MNT_ROOT) << 16;
				break;
			}
		}else{
			if (strncmp(dev, dev_str, strlen(dev_str))==0){
				mnt_map = _mount(parition_id & 0xffff, dev_str, MNT_ROOT);
				break;
			}
		}
	}
#endif
	return mnt_map;	
}

static unsigned int _umount(unsigned int parition_map, const char *base, const char *mnt_path) {
	//char dev_buffer[10];
	char mnt_buffer[30];
	char cmd_buffer[100];
	int tmp, rv;
	unsigned int mnt_map = 0;
#ifdef CONFIG_CT_USBMOUNT_DIRECTORY
	char usbnum = (!strcmp(base,"sda")) ? 1 : 2;
#endif

#ifdef CONFIG_CT_USBMOUNT_DIRECTORY
	snprintf(mnt_buffer, sizeof(mnt_buffer), "%s/usb%d_1", mnt_path, usbnum);
	//snprintf(cmd_buffer, sizeof(cmd_buffer), "/bin/umount %s/usb%d_0", mnt_path, usbnum);
#else
	snprintf(mnt_buffer, sizeof(mnt_buffer), "%s/%s", mnt_path, base);
	//snprintf(cmd_buffer, sizeof(cmd_buffer), "/bin/umount %s/%s", mnt_path, base);
#endif

	for (tmp = 0; tmp < 16; tmp++) {
		if (!(parition_map & (1 << tmp)))
			continue;
		
		if (tmp) {
#ifdef CONFIG_CT_USBMOUNT_DIRECTORY
			snprintf(mnt_buffer, sizeof(mnt_buffer), "%s/usb%d_%d", mnt_path, usbnum, tmp);
			//snprintf(cmd_buffer, sizeof(cmd_buffer), "/bin/umount %s/usb%d_%d", mnt_path, usbnum, tmp);
#else
			snprintf(mnt_buffer, sizeof(mnt_buffer), "%s/%s%d", mnt_path, base, tmp);
			//snprintf(cmd_buffer, sizeof(cmd_buffer), "/bin/umount %s/%s%d", mnt_path, base, tmp);
#endif
		
		fsync();
		rv = umount(mnt_buffer);
		if (rv == 0) {
			mnt_map |= (1 << tmp);
			rmdir(mnt_buffer);
		}
		}else{
			if(tmp==0){
				sprintf(mnt_buffer, "%s/%s", mnt_path, base);
				fsync();
				rv = umount(mnt_buffer);
				rmdir(mnt_buffer);
			}
		}
		/*
		if (system(cmd_buffer) == 0) {			
			mnt_map |= (1 << tmp);
			rmdir(mnt_buffer);
		}
		*/
	}

	return mnt_map;
}


static int file_update(unsigned int *new_mask, int adding) {
	FILE *fp;
	char buffer[10];
	unsigned int mask = 0;
	
	/*
	fp = fopen(MNT_FILE, "r");
	
	if (!fp) {		
		goto write_it;
	}

	fgets(buffer, sizeof(buffer), fp);
	sscanf(buffer, "%u", &mask);
	fclose(fp);
	*/
	mask = file_read_number(MNT_FILE);

write_it:
	if (adding)
		mask |= *new_mask;
	else
		mask &= ~(*new_mask);

	fp = fopen(MNT_FILE, "w");
	if (!fp) {
		fprintf(stderr, "fail to create %s\n", MNT_FILE);
		return -1;
	}
	fprintf(fp, "%u", mask);
	fclose(fp);

	*new_mask = mask;
	
	return 0;		
}

static unsigned int fs_umount(unsigned int parition_id,const char *dev) {
	unsigned int mnt_map = 0;
	unsigned char char_start=0;
	unsigned int index=0;
	unsigned char dev_str[4]={'s','d',0,0};
	DEBUG("%s(1, 0x%x)\n", __FUNCTION__, parition_id);
#if 0	
	mnt_map = _umount(parition_id & 0xffff, "sda", MNT_ROOT);
	mnt_map |= _umount(parition_id >> 16, "sdb", MNT_ROOT) << 16;

#else

	for(char_start='a';char_start<='z';char_start++){
		dev_str[2]=char_start;
		index=index+1;
		if(index % 2 ==0){
			if (strncmp(dev, dev_str, strlen(dev_str))==0){
				mnt_map |= _umount(parition_id >> 16, dev_str, MNT_ROOT) << 16;
				break;
			}
		}else{
			if (strncmp(dev, dev_str, strlen(dev_str))==0){
				mnt_map = _umount(parition_id & 0xffff, dev_str, MNT_ROOT);
				break;
			}
		}
	}
#endif
	return mnt_map;
}


static int action_add(int argc, char **argv, char *devpath) {
	unsigned int id, mnt_map;
#ifdef CONFIG_BOA_WEB_E8B_CH
#ifdef CONFIG_USER_BFTPD_BFTPD
    char *myprog = "/bin/ftp_manage";
    char *myargs[2];
    char *myenv[2];
#endif
#endif	
	id = get_partition_id(basename(devpath));
	
	if (id) {
		mnt_map = fs_mount(id,basename(devpath));

		if (mnt_map) {
			char cmd_buffer[30];

			//snprintf(cmd_buffer, sizeof(cmd_buffer), "echo %d > %s", mnt_map, MNT_FILE);
			//system(cmd_buffer);
			file_update(&mnt_map, 1);
		}
	}

#ifdef CONFIG_BOA_WEB_E8B_CH
#ifdef CONFIG_USER_BFTPD_BFTPD

    myargs[0] = myprog;
    myargs[1] = NULL;

    myenv[0] = "PATH=/bin:/usr/bin:/etc:/sbin:/usr/sbin";
    myenv[1] = NULL;
    if(fork() == 0){
    	execve(myprog, myargs, myenv);
	exit(0);
    }
#endif
#endif

	return 0;
}

static int action_remove(int argc, char **argv, char *devpath) {

	unsigned int id, mnt_map;
#ifdef CONFIG_BOA_WEB_E8B_CH
#ifdef CONFIG_USER_BFTPD_BFTPD

    char *myprog = "/bin/ftp_manage";
    char *myargs[2];
    char *myenv[2];
#endif
#endif
	id = get_partition_id(basename(devpath));
	//id = get_umount_partition_id(basename(devpath));
	printf("\nID=%x\n");
	if (id) {
		mnt_map = fs_umount(id, basename(devpath));
		if (mnt_map) {
			file_update(&mnt_map, 0);
			if (mnt_map==0)
				unlink(MNT_FILE);
		}

	}

#ifdef CONFIG_BOA_WEB_E8B_CH
#ifdef CONFIG_USER_BFTPD_BFTPD

    myargs[0] = myprog;
    myargs[1] = NULL;

    myenv[0] = "PATH=/bin:/usr/bin:/etc:/sbin:/usr/sbin";
    myenv[1] = NULL;
    if(fork() == 0){
    	execve(myprog, myargs, myenv);
	exit(0);
    }
#endif
#endif
}


static const struct action_table hotplug_table[] = {
	{ "mount", action_add },
	{ "add", action_add },
	{ "remove", action_remove },	
	{ 0 },
};



int main(int argc, char **argv) {
	
	char *devpath;
	char *action;
	const struct action_table *p;

	ASSERT(argc > 1);
	if (argc<= 1) {
		DEBUG("not enough arg\n");
		return -1;
	}
	
	devpath = getenv("DEVPATH");
	ASSERT(devpath != 0);
	
	action = getenv("ACTION");
	ASSERT(action != 0);

	if (strcmp(argv[1], "block")) {
		DEBUG("will not handle %s\n", argv[1]);
		return -1;
	}

	if ((action == 0) || (devpath == 0)){
		printf("required env var missing\n");
		return -1;
	}
	snprintf(cmd_bufferx, sizeof(cmd_bufferx), 
			"echo \"D=(%s) A=(%s) A1=%s \" >> /tmp/log", 
			devpath, action, argv[1]);
		system(cmd_bufferx);

	for (p = &hotplug_table[0]; p->action_str; p++) {
		if (strcmp(action, p->action_str)==0) {
			return (p->action_func(argc-1, &argv[1], devpath));
		}
	}
	return 0;
}



