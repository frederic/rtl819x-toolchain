/*
 *	Generic parts
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/llc.h>
#include <net/llc.h>
#include <net/stp.h>

#include "br_private.h"

#include <linux/proc_fs.h>
#if defined (CONFIG_RTL865X_LANPORT_RESTRICTION)
#include <net/rtl/features/lan_restrict.h>

#endif
#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT) || defined(CONFIG_RTL_ULINKER)
extern int __init br_filter_init(void);
extern void __exit br_filter_exit(void);
#endif

#if defined(CONFIG_HTTP_FILE_SERVER_SUPPORT) || defined(CONFIG_RTL_USB_UWIFI_HOST_SPEEDUP)
#include <linux/inetdevice.h>
#define NIPQUAD_HTTP(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]
struct proc_dir_entry *http_file_server_root=NULL;
struct proc_dir_entry *http_file_server_root_res1=NULL;
extern unsigned int _lan_ip;
extern unsigned int _lan_mask;
void get_lan_ip_mask(void)
{
	struct in_device *in_dev;	
	struct net_device *landev;
	struct in_ifaddr *ifap = NULL;
	
      	if ((landev = __dev_get_by_name(&init_net, "br0")) != NULL){
		in_dev=(struct in_device *)(landev->ip_ptr);
		if (in_dev != NULL) {
			for (ifap=in_dev->ifa_list; ifap != NULL; ifap=ifap->ifa_next) {
				if (strcmp("br0", ifap->ifa_label) == 0){
					_lan_ip = ifap->ifa_address;
					_lan_mask = ifap->ifa_mask;
					return; 
				}
			}
		}
	}	
}
static int getLanIp_read(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{

      int len=0;

      len = sprintf(page, "Lan IP=%u.%u.%u.%u\n", NIPQUAD_HTTP(_lan_ip));

      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
   	
      return len;

}
static int getLanIp_write(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
      char tmpbuf[80];

      if (count < 2)
	    return -EFAULT;
	    
	if (buffer && !copy_from_user(tmpbuf, buffer, 80))  {
		get_lan_ip_mask();
		return count;
	}	
      return -EFAULT;
}
int __init http_file_init(void)
{
#if defined(CONFIG_PROC_FS)

	struct proc_dir_entry *devices = NULL,*pvc = NULL,*svc = NULL;
	http_file_server_root = proc_mkdir("http_file",NULL);
	if (!http_file_server_root){
		printk("create folder fail\n");
		return -ENOMEM;
	}
	http_file_server_root_res1 = create_proc_entry("getLanIp", 0, http_file_server_root);
	if (http_file_server_root_res1) {
		http_file_server_root_res1->write_proc = getLanIp_write;
		http_file_server_root_res1->read_proc = getLanIp_read;
	}
#endif
	return 0;
}

void __exit http_file_exit(void)
{
#if defined(CONFIG_PROC_FS)
	if (http_file_server_root_res1) {
		remove_proc_entry("getLanIp", http_file_server_root);
		http_file_server_root_res1 = NULL;
	}
	remove_proc_entry("http_file",NULL);
#endif 
}

#endif






#if defined (CONFIG_RTL_IGMP_SNOOPING)
#include <linux/inetdevice.h>
#include <net/rtl/rtl865x_igmpsnooping_glue.h>
#include <net/rtl/rtl865x_igmpsnooping.h>
#include <net/rtl/rtl865x_multicast.h>
#include <net/rtl/rtl865x_netif.h>
#include <net/rtl/rtl_nic.h>
#endif
int IGMPProxyOpened = 0;

int (*br_should_route_hook)(struct sk_buff *skb);

static const struct stp_proto br_stp_proto = {
	.rcv	= br_stp_rcv,
};

static struct pernet_operations br_net_ops = {
	.exit	= br_net_exit,
};

//it's for host-slave hostapd EAP hack  amd inband hostip hack
#if defined (CONFIG_RTL_EAP_RELAY) || defined (CONFIG_RTK_INBAND_HOST_HACK)
struct proc_dir_entry *procHostMac=NULL;
unsigned char inband_Hostmac[6]={0x00,0xE0,0x4C,0x81,0x96,0xC1}; 

static int br_hostmac_read_proc(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{

      int len;

      len = sprintf(page, "%x:%x:%x:%x:%x:%x\n", inband_Hostmac[0],inband_Hostmac[1]
	  										, inband_Hostmac[2],inband_Hostmac[3]
	  										, inband_Hostmac[4],inband_Hostmac[5]);


      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;

}

static int br_hostmac_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
      char br_tmp[48]; 
       int mac[6];		  
	
      if (count < 2) 
	    return -EFAULT;

	
      if (buffer && !copy_from_user(&br_tmp, buffer, 48)) {
	  	int num=0;	
		int i=0;
             num = sscanf(br_tmp, "%d:%d:%d:%d:%d:%d",&mac[0],&mac[1]
	  										, &mac[2],&mac[3]
	  										, &mac[4],&mac[5]);
            
		for(i=0;i<6;i++)
			inband_Hostmac[i] = (unsigned char)(mac[i] & 0x000000ff);	  	 
            
                if (num !=  6) {
                        printk("br_hostmac_write_proc invalid vlan parameter!\n");
                }
	    return count;
      }
      return -EFAULT;
}
#endif

#if defined (CONFIG_RTK_INBAND_HOST_HACK)
struct proc_dir_entry *procHostIp=NULL;
unsigned int inband_HostIP=0xC0A801FE;
int br_hackMac_enable=0; //only wps will need hack now!!!
extern void init_listen_endpoint(); //from br_input.c
static int br_hostip_read_proc(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{

      int len;

      len = sprintf(page, "%d %d.%d.%d.%d\n", br_hackMac_enable,(unsigned char)((inband_HostIP>>24)&0x000000ff),
								  	(unsigned char)((inband_HostIP>>16)&0x000000ff),
								  	(unsigned char)((inband_HostIP>>8)&0x000000ff),
								  	(unsigned char)((inband_HostIP>>0)&0x000000ff)
								  	);


      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;

      return len;

}

static int br_hostip_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
      char br_tmp[48]; 
       unsigned int tmp_ip[4];		  
	
      if (count < 2) 
	    return -EFAULT;

	
      if (buffer && !copy_from_user(&br_tmp, buffer, 48)) {
	  	int num=0;	
             num = sscanf(br_tmp, "%d %d.%d.%d.%d",&br_hackMac_enable,&tmp_ip[0],&tmp_ip[1]
	  										, &tmp_ip[2],&tmp_ip[3]);
            inband_HostIP =  ((tmp_ip[0] << 24) & 0xff000000) |
						((tmp_ip[1] << 16) & 0x00ff0000) |
						((tmp_ip[2] << 8) & 0x0000ff00) |
						((tmp_ip[3] << 0) & 0x000000ff) ;
							
  		  		init_listen_endpoint();	//in br_input.c	            
                if (num !=  5) {
                        printk("br_hostip_write_proc invalid vlan parameter!\n");
                }

	    return count;
      }
      return -EFAULT;
}
#endif

#if defined(CONFIG_RTL_WLAN_BLOCK_RELAY)
int rtl_wlan_block_relay_enable=0;
struct proc_dir_entry *procwlanblockrelay=NULL;
static int br_wlanblockread_proc(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{
	int len;
	len = sprintf(page, "wlanblock:%c\n\n",rtl_wlan_block_relay_enable+'0');
	if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;
}
static int br_wlanblockwrite_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
      unsigned char block_tmp; 
      if (count < 2) 
	    return -EFAULT;
      
	if (buffer && !copy_from_user(&block_tmp, buffer, 1)) {
		rtl_wlan_block_relay_enable = block_tmp - '0';
		if(rtl_wlan_block_relay_enable)
		{
			rtl_wlan_block_relay_enable=1;
		}
		else
			rtl_wlan_block_relay_enable=0;	
	    	return count;
      }
	return -EFAULT;
}
#endif
#if defined (CONFIG_RTL_IGMP_SNOOPING)

struct proc_dir_entry *procIgmpSnoop=NULL;
int igmpsnoopenabled=0;	// Should be 0(default), set 1 when igmpproxy up!
extern struct net_bridge *bridge0;
static int br_igmpSnoopRead_proc(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{

      int len;
      int i,j,k;
   
    len = sprintf(page, "igmpsnoopenabled:%c\n\n", igmpsnoopenabled + '0');
    len += sprintf(page+len, "bridge multicast fdb:\n");


	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct net_bridge_fdb_entry *f;
		struct hlist_node *h, *n;
		
		j=0;
		hlist_for_each_entry_safe(f, h, n, &bridge0->hash[i], hlist) {
			if(MULTICAST_MAC(f->addr.addr) )
			{
				  len += sprintf(page+len,"[%d][%d]mCastMac:0x%x:%x:%x:%x:%x:%x,use_count is %d,ageing_timer is %lu\n",
					i,j,f->addr.addr[0],f->addr.addr[1],f->addr.addr[2],f->addr.addr[3],f->addr.addr[4],f->addr.addr[5],f->use_count,(jiffies<f->ageing_timer) ?0:(jiffies-f->ageing_timer));
				
				for(k=0;k<FDB_IGMP_EXT_NUM;k++)
				{
					if(f->igmp_fdb_arr[k].valid)
					{
						len += sprintf(page+len,"\t<%d>clientMac:0x%x:%x:%x:%x:%x:%x,ageing_time:%lu\n",
						k,f->igmp_fdb_arr[k].SrcMac[0],f->igmp_fdb_arr[k].SrcMac[1],f->igmp_fdb_arr[k].SrcMac[2],f->igmp_fdb_arr[k].SrcMac[3],f->igmp_fdb_arr[k].SrcMac[4],f->igmp_fdb_arr[k].SrcMac[5],(jiffies<f->igmp_fdb_arr[k].ageing_time) ?0:(jiffies-f->igmp_fdb_arr[k].ageing_time));
					}
				}

				j++;
				len += sprintf(page+len,"--------------------------------------------\n");
			}

		}
		
	}

	
      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;

}

static int br_igmpSnoopWrite_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
      unsigned char br_tmp; 
      if (count < 2) 
	    return -EFAULT;
      
	if (buffer && !copy_from_user(&br_tmp, buffer, 1)) {
		igmpsnoopenabled = br_tmp - '0';
		if(igmpsnoopenabled)
		{
			igmpsnoopenabled=1;
		}
		else
		{
#if defined (CONFIG_RTL_HARDWARE_MULTICAST)		
			rtl865x_reinitMulticast();
#endif
			rtl_flushAllIgmpRecord();
		}
	    return count;
      }
      return -EFAULT;
}

static struct proc_dir_entry *procIgmpDb;
extern int igmp_show(struct seq_file *s, void *v);
extern int igmp_write(struct file *file, const char __user *buffer, size_t count, loff_t *data);
int igmp_db_open(struct inode *inode, struct file *file)
{
        return(single_open(file, igmp_show, NULL));
}

int igmp_db_write(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
         igmp_write(file, buffer, count,data);
}

struct file_operations igmp_db_seq_file_operations = {
        .open           = igmp_db_open,
        .read           = seq_read,
        .write		= igmp_db_write,
        .llseek         = seq_lseek,
        .release        = single_release,
};

struct proc_dir_entry *procMCastFastFwd=NULL;
int ipMulticastFastFwd=0;
int needCheckMfc=0;
static int br_mCastFastFwdRead_proc(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{

      int len;

      len = sprintf(page, "%c,%c\n", ipMulticastFastFwd + '0',needCheckMfc + '0');


      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;

}

static int br_mCastFastFwdWrite_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	unsigned int tmp=0; 
	char 		tmpbuf[512];
	char		*strptr;
	char		*tokptr;
	

	if (count < 2) 
		return -EFAULT;

	if (buffer && !copy_from_user(tmpbuf, buffer, count)) 
	{
		tmpbuf[count] = '\0';

		strptr=tmpbuf;

		
		tokptr = strsep(&strptr,",");
		if (tokptr==NULL)
		{
			tmp=simple_strtol(strptr, NULL, 0);
			printk("tmp=%d\n",tmp);
			if(tmp==0)
			{
				ipMulticastFastFwd=0;
			}
			return -EFAULT;
		}
		
		ipMulticastFastFwd = simple_strtol(tokptr, NULL, 0);
		//printk("ipMulticastFastFwd=%d\n",ipMulticastFastFwd);
		if(ipMulticastFastFwd)
		{
			ipMulticastFastFwd=1;
		}

		tokptr = strsep(&strptr,",");
		if (tokptr==NULL)
		{
			return -EFAULT;
		}
		
		needCheckMfc = simple_strtol(tokptr, NULL, 0);

		if(needCheckMfc)
		{
			needCheckMfc=1;
		}

		return count;
	}
	return -EFAULT;
}

struct proc_dir_entry *procIgmpQuery=NULL;
int igmpQueryEnabled=0;	
static int br_igmpQueryRead_proc(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{

      int len;

      len = sprintf(page, "%c\n", igmpQueryEnabled + '0');


      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;

}

static int br_igmpQueryWrite_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
      unsigned char tmp; 
      if (count < 2) 
	    return -EFAULT;
      
	if (buffer && !copy_from_user(&tmp, buffer, 1)) {
		igmpQueryEnabled = tmp - '0';
		if(igmpQueryEnabled)
		{
			igmpQueryEnabled=1;
		}
		else
		{
			igmpQueryEnabled=0;
		}
	    return count;
      }
      return -EFAULT;
}


#if defined (CONFIG_RTL_QUERIER_SELECTION)

#define MAX_QUERIER_RECORD 8
#define QUERIER_EXPIRED_TIME 255

struct querierInfo
{
	unsigned int version;
	unsigned char devName[IFNAMSIZ];
	unsigned int querierIp[4];
	unsigned int lastJiffies;
};

struct querierInfo querierInfoList[MAX_QUERIER_RECORD];

int br_initQuerierInfo(void)
{

	memset(querierInfoList, 0, sizeof(querierInfoList));
	return 0;
	
}

int br_updateQuerierInfo(unsigned int version, unsigned char *devName, unsigned int* querierIp)
{
	int i;
	unsigned long oldestJiffies;
	unsigned long oldestIdx=0;
	
	if(querierIp==NULL)
	{
		return -1;	
	}
	
	if((version!=4) && (version!=6))
	{
		return -1;	
	}
	
	/*find matched one*/
	for(i=0; i<MAX_QUERIER_RECORD; i++)
	{
		if((querierInfoList[i].version==version))
		{
			if( (version==4) && (querierInfoList[i].querierIp[0]==querierIp[0]))
			{
				strcpy(querierInfoList[i].devName,devName);
				querierInfoList[i].lastJiffies=jiffies;
				return 0;
	
			}

			if((version ==6) && (memcmp(querierInfoList[i].querierIp, querierIp, 16) ==0)) 
			{
				strcpy(querierInfoList[i].devName,devName);
				querierInfoList[i].lastJiffies=jiffies;
				return 0;
			}
		}
	
	}
	
	/*no matched one, find an empty one*/
	for(i=0; i<MAX_QUERIER_RECORD; i++)
	{
		if(querierInfoList[i].version==0)
		{
			querierInfoList[i].version=version;
			if(version==4)
			{
				querierInfoList[i].querierIp[0]=querierIp[0];
			}
			else if (version ==6)
			{
				memcpy(querierInfoList[i].querierIp, querierIp, 16);
			}
			strcpy(querierInfoList[i].devName,devName);
			querierInfoList[i].lastJiffies=jiffies;
			
			return 0;
		}
	}
	
	/*all entries are used, find oldest one*/
	oldestJiffies=querierInfoList[0].lastJiffies;
	oldestIdx=0;
	for(i=0; i<MAX_QUERIER_RECORD; i++)
	{
		if(time_before(querierInfoList[i].lastJiffies,oldestJiffies))
		{
			oldestJiffies=querierInfoList[i].lastJiffies;
			oldestIdx=i;
		}
	}
	
	querierInfoList[oldestIdx].version=version;
	if(version==4)
	{
		querierInfoList[oldestIdx].querierIp[0]=querierIp[0];
	}
	else if (version ==6)
	{
		memcpy(querierInfoList[oldestIdx].querierIp, querierIp, 16);
	}
	
	strcpy(querierInfoList[oldestIdx].devName,devName);
	querierInfoList[oldestIdx].lastJiffies=jiffies;
	
	return 0;

	
}

int br_querierSelection(struct net_bridge *br,unsigned int ipVer)	
{
	int i;
	int ret=1;
	struct net_device* brDev = NULL;
	struct in_device *in_dev;	
	struct net_device *landev;
	struct in_ifaddr *ifap = NULL;
	unsigned int brIpAddr=0;
	unsigned char brIpv6Addr[16]={	0xfe,0x80,0x00,0x00,				/*source address*/
									0x00,0x00,0x00,0x00,				/*be zero*/	
									0x00,0x00,0x00,					/*upper 3 bytes mac address |0x02*/ 
									0xff,0xfe,						/*fixed*/
									0x00,0x00,0x00					/*lowert 3 bytes mac address*/	};

	
	if(br==NULL)
	{
		return 1;
	}
	
	brDev = br->dev;

	if(ipVer==4)
	{
		/*get bridge ip address*/
		if ((landev = __dev_get_by_name(&init_net, RTL_PS_BR0_DEV_NAME)) != NULL){
			in_dev=(struct net_device *)(landev->ip_ptr);
			if (in_dev != NULL) {
				for (ifap=in_dev->ifa_list; ifap != NULL; ifap=ifap->ifa_next) {
					if (strcmp(RTL_PS_BR0_DEV_NAME, ifap->ifa_label) == 0)
					{
							memcpy(&brIpAddr,&ifap->ifa_address,4);
					}
				}
				
			}
		}

		for(i=0; i<MAX_QUERIER_RECORD; i++)
		{
			if(	(querierInfoList[i].version==4)&&
				time_after(querierInfoList[i].lastJiffies+QUERIER_EXPIRED_TIME*HZ,jiffies)&&
				(querierInfoList[i].querierIp < brIpAddr))
			{
				ret=0;
			}
		}
		
	}
	else if (ipVer==6)
	{
	
		memcpy(&brIpv6Addr[8],brDev->dev_addr,3);		/*generate br link-local ipv6 address*/
		brIpv6Addr[8]=brIpv6Addr[8]|0x02;		
		memcpy(&brIpv6Addr[13],&brDev->dev_addr[3],3);	
		#if 1
		printk("br0 ipv6 address is:\n");

		{
			int j;
			for(j=0; j<16; j++)	
			{
				printk("%x",brIpv6Addr[j]);
				if((j!=0) &&(j%4==0))
				{
					printk("-");
				}
			}
			printk("\n");
		}
		#endif
		
		for(i=0; i<MAX_QUERIER_RECORD; i++)
		{
			if(	(querierInfoList[i].version==6)&&
				time_after(querierInfoList[i].lastJiffies+QUERIER_EXPIRED_TIME*HZ,jiffies))
			{
				if(memcmp(querierInfoList[i].querierIp, brIpv6Addr, 16)<0)
				{
					ret=0;
				}
			}
		}
		
		
	}
	
	return ret;
}


struct proc_dir_entry *procIgmpQuerierInfo=NULL;

static int br_igmpQuerierInfoRead_proc(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{

	int len=0;
	int i;
	unsigned long elapseJiffies;
	for(i=0; i<MAX_QUERIER_RECORD; i++)
	{
		if((querierInfoList[i].version==4) && time_after(querierInfoList[i].lastJiffies+QUERIER_EXPIRED_TIME*HZ,jiffies))
		{
			if(jiffies > querierInfoList[i].lastJiffies)
			{
				elapseJiffies=jiffies-querierInfoList[i].lastJiffies;
			}
			else
			{
				elapseJiffies=jiffies+((unsigned long)0xFFFFFFFF-querierInfoList[i].lastJiffies)+1;
			}

			len += sprintf(page+len, "%s %d.%d.%d.%d %u\n",
				querierInfoList[i].devName,
				((querierInfoList[i].querierIp[0]>>24)&0xFF) ,
				((querierInfoList[i].querierIp[0]>>16)&0xFF),
				((querierInfoList[i].querierIp[0]>>8)&0xFF),
				(querierInfoList[i].querierIp[0]&0xFF),elapseJiffies/HZ);
		}
	}
	
	
	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;
	return len;

}

static int br_igmpQuerierInfoWrite_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	unsigned char tmp[64]; 

	if (count < 2) 
	{
		return -EFAULT;
	}
	
	if(count >sizeof(tmp))
	{
		return -EFAULT;
	}
	  
	
    return 0;
}



struct proc_dir_entry *procMldQuerierInfo=NULL;

static int br_mldQuerierInfoRead_proc(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{

	int len=0;
	int i;
	unsigned long elapseJiffies;

	for(i=0; i<MAX_QUERIER_RECORD; i++)
	{
		if((querierInfoList[i].version==6) && time_after(querierInfoList[i].lastJiffies+QUERIER_EXPIRED_TIME*HZ,jiffies))
		{
			if(jiffies > querierInfoList[i].lastJiffies)
			{
				elapseJiffies=jiffies-querierInfoList[i].lastJiffies;
			}
			else
			{
				elapseJiffies=jiffies+((unsigned long)0xFFFFFFFF-querierInfoList[i].lastJiffies)+1;
			}

			len += sprintf(page+len, "%s %x-%x-%x-%x %u\n",
				querierInfoList[i].devName,
				querierInfoList[i].querierIp[0],
				querierInfoList[i].querierIp[1],
				querierInfoList[i].querierIp[2],
				querierInfoList[i].querierIp[3],elapseJiffies/HZ);
		}
	}

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;
	return len;

}

static int br_mldQuerierInfoWrite_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	unsigned char tmp[64]; 

	if (count < 2) 
	{
		return -EFAULT;
	}
	
	if(count >sizeof(tmp))
	{
		return -EFAULT;
	}
	  
	
    return 0;
}

#endif


struct proc_dir_entry *procIgmpVersion=NULL;
int igmpVersion=2;
static int br_igmpVersionRead_proc(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{

      int len;

      len = sprintf(page, "%c\n", igmpVersion + '0');


      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;

}

static int br_igmpVersionWrite_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
      unsigned char tmp; 
      if (count < 2) 
	    return -EFAULT;
      
	if (buffer && !copy_from_user(&tmp, buffer, 1)) {
		igmpVersion = tmp - '0';
		if(igmpVersion>=3)
		{
			igmpVersion=3;
		}
		else if (igmpVersion<=2)
		{
			igmpVersion=2;
		}
		else
		{
			igmpVersion=2;
		}
	    return count;
      }
      return -EFAULT;
}


/*igmpv3 general query*/
static unsigned char igmpV3QueryBuf[64]={	0x01,0x00,0x5e,0x00,0x00,0x01,		/*destination mac*/
									0x00,0x00,0x00,0x00,0x00,0x00,		/*offset:6*/
									0x08,0x00,						/*offset:12*/
									0x46,0x00,0x00,0x24,				/*offset:14*/
									0x00,0x00,0x40,0x00,				/*offset:18*/
									0x01,0x02,0x00,0x00,				/*offset:22*/
									0x00,0x00,0x00,0x00,				/*offset:26,source ip*/
									0xe0,0x00,0x00,0x01,				/*offset:30,destination ip*/
									0x94,0x04,0x00,0x00,				/*offset:34,router alert option*/
									0x11,0x01,0x00,0x00,				/*offset:38*/
									0x00,0x00,0x00,0x00,				/*offset:42,queried multicast ip address*/
									0x0a,0x3c,0x00,0x00,				/*offset:46*/
									0x00,0x00,0x00,0x00,				/*offset:50*/
									0x00,0x00,0x00,0x00,				/*offset:54*/
									0x00,0x00,0x00,0x00,				/*offset:58*/
									0x00,0x00							/*offset:62*/
									
								};			



/*igmpv2 general query*/
static unsigned char igmpV2QueryBuf[64]={	0x01,0x00,0x5e,0x00,0x00,0x01,		/*destination mac*/
									0x00,0x00,0x00,0x00,0x00,0x00,		/*offset:6*/
									0x08,0x00,						/*offset:12*/
									0x45,0x00,0x00,0x1c,				/*offset:14*/
									0x00,0x00,0x40,0x00,				/*offset:18*/
									0x01,0x02,0x00,0x00,				/*offset:22*/
									0x00,0x00,0x00,0x00,				/*offset:26*/
									0xe0,0x00,0x00,0x01,				/*offset:30*/
									0x11,0x01,0x0c,0xfa,				/*offset:34*/
									0x00,0x00,0x00,0x00,				/*offset:38*/
									0x00,0x00,0x00,0x00,				/*offset:42*/
									0x00,0x00,0x00,0x00,				/*offset:46*/
									0x00,0x00,0x00,0x00,				/*offset:50*/
									0x00,0x00,0x00,0x00,				/*offset:54*/
									0x00,0x00,0x00,0x00,				/*offset:58*/
									0x00,0x00							/*offset:62*/
									
								};			



static unsigned short  br_ipv4Checksum(unsigned char *pktBuf, unsigned int pktLen)
{
	/*note: the first bytes of  packetBuf should be two bytes aligned*/
	unsigned int  checksum=0;
	unsigned int  count=pktLen;
	unsigned short   *ptr= (unsigned short *)pktBuf;	
	
	 while(count>1)
	 {
		  checksum+= ntohs(*ptr);
		  ptr++;
		  count -= 2;
	 }
	 
	if(count>0)
	{
		checksum+= *(pktBuf+pktLen-1)<<8; /*the last odd byte is treated as bit 15~8 of unsigned short*/
	}

	/* Roll over carry bits */
	checksum = (checksum >> 16) + (checksum & 0xffff);
	checksum += (checksum >> 16);

	/* Return checksum */
	return ((unsigned short) ~ checksum);

}

static unsigned char* br_generateIgmpQuery(struct net_bridge * br)
{
	struct net_device* brDev = NULL;
	unsigned short checkSum=0;
	struct in_device *in_dev;	
	struct net_device *landev;
	struct in_ifaddr *ifap = NULL;
	
      
	if(br==NULL)
	{
		return NULL;
	}
	
	brDev = br->dev;
	if(igmpVersion==3)
	{
		memcpy(&igmpV3QueryBuf[6],brDev->dev_addr,6);			/*set source mac address*/
	}
	else
	{
		memcpy(&igmpV2QueryBuf[6],brDev->dev_addr,6);			/*set source mac address*/
	}
	
	/*set source ip address*/
	if ((landev = __dev_get_by_name(&init_net, RTL_PS_BR0_DEV_NAME)) != NULL){
		in_dev=(struct net_device *)(landev->ip_ptr);
		if (in_dev != NULL) {
			for (ifap=in_dev->ifa_list; ifap != NULL; ifap=ifap->ifa_next) {
				if (strcmp(RTL_PS_BR0_DEV_NAME, ifap->ifa_label) == 0){
					if(igmpVersion==3)
					{
						memcpy(&igmpV3QueryBuf[26],&ifap->ifa_address,4);
					}
					else
					{
						memcpy(&igmpV2QueryBuf[26],&ifap->ifa_address,4);
					}
					
				}
			}
			
		}
	}
	else
	{
		return NULL;
	}
	
    #ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	if(!strcmp(RTL_PS_BR1_DEV_NAME, br->dev->name))
	{
		if (landev = brDev){
			in_dev=(struct net_device *)(landev->ip_ptr);
			if (in_dev != NULL) {
				for (ifap=in_dev->ifa_list; ifap != NULL; ifap=ifap->ifa_next) {
					if (strcmp(br->dev->name, ifap->ifa_label) == 0){
						if(igmpVersion==3)
						{
							memcpy(&igmpV3QueryBuf[26],&ifap->ifa_address,4);
						}
						else
						{
							memcpy(&igmpV2QueryBuf[26],&ifap->ifa_address,4);
						}
					}
				}
			}
		}
	}
        #endif
	if(igmpVersion==3)
	{
		igmpV3QueryBuf[24]=0;
		igmpV3QueryBuf[25]=0;
	}
	else
	{
		igmpV2QueryBuf[24]=0;
		igmpV2QueryBuf[25]=0;
	}
	
	if(igmpVersion==3)
	{
		checkSum=br_ipv4Checksum(&igmpV3QueryBuf[14],24);
	}
	else
	{
		checkSum=br_ipv4Checksum(&igmpV2QueryBuf[14],20);
	}

	if(igmpVersion==3)
	{
		igmpV3QueryBuf[24]=(checkSum&0xff00)>>8;
		igmpV3QueryBuf[25]=(checkSum&0x00ff);

	}
	else
	{
		igmpV2QueryBuf[24]=(checkSum&0xff00)>>8;
		igmpV2QueryBuf[25]=(checkSum&0x00ff);

	}
	

	if(igmpVersion==3)
	{
		igmpV3QueryBuf[40]=0;
		igmpV3QueryBuf[41]=0;
		checkSum=br_ipv4Checksum(&igmpV3QueryBuf[38],12);
		igmpV3QueryBuf[40]=(checkSum&0xff00)>>8;
		igmpV3QueryBuf[41]=(checkSum&0x00ff);
	}
	else
	{
		igmpV2QueryBuf[36]=0;
		igmpV2QueryBuf[37]=0;
		checkSum=br_ipv4Checksum(&igmpV2QueryBuf[34],8);
		igmpV2QueryBuf[36]=(checkSum&0xff00)>>8;
		igmpV2QueryBuf[37]=(checkSum&0x00ff);
	}

	if(igmpVersion==3)
	{
		return igmpV3QueryBuf;
	}
	else
	{
		return igmpV2QueryBuf;
	}
	
	return NULL;
}


void br_igmpQueryTimerExpired(unsigned long arg)
{
	struct net_bridge *br = (struct net_bridge*) arg;
	unsigned char *igmpBuf=NULL;
	struct sk_buff *skb;
	struct sk_buff *skb2;
	struct net_bridge_port *p, *n;
	struct net_bridge_port *prev;
	unsigned int fwdCnt=0;

#ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
	if(IGMPProxyOpened && strcmp(br->dev->name,RTL_PS_BR1_DEV_NAME))
#else
	if(IGMPProxyOpened)
#endif
	{
		return ;
	}
	
	if(igmpQueryEnabled==0)
	{
		return;
	}

	
#if defined (CONFIG_RTL_QUERIER_SELECTION)
	if(br_querierSelection(br,4)==0)
	{
		return;
	}
#endif	

	skb=dev_alloc_skb(1024);
	if(skb==NULL)
	{
		return;
	}

	memset(skb->data,64,0);
	igmpBuf=br_generateIgmpQuery(br);
	if(igmpBuf==NULL)
	{
		return;
	}

	memcpy(skb->data,igmpBuf,64);

	skb->len = 0;
	if(igmpVersion==3)
	{
		skb_put(skb, 50);
	}
	else
	{
		skb_put(skb, 42);
	}
	
	skb->dev=br->dev;
	
	prev = NULL;
	fwdCnt=0;
	list_for_each_entry_safe(p, n, &br->port_list, list) 
	{ 
		if ((p->state == BR_STATE_FORWARDING) && (strncmp(p->dev->name, "peth",4)!=0) && (strncmp(p->dev->name, "pwlan",5)!=0)) 
		{
			if (prev != NULL) 
			{                                                                                       
				if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) 
				{
					br->dev->stats.tx_dropped++;
					kfree_skb(skb);
					return 0;
				} 
				skb2->dev=prev->dev;
				#if defined(CONFIG_COMPAT_NET_DEV_OPS)
				prev->dev->hard_start_xmit(skb2, prev->dev);
				#else
				prev->dev->netdev_ops->ndo_start_xmit(skb2,prev->dev);
				#endif                  
				fwdCnt++;
			}
				                                                                               
			prev = p;
		}
	}

	if (prev != NULL) 
	{
		skb->dev=prev->dev;
	       #if defined(CONFIG_COMPAT_NET_DEV_OPS)
		prev->dev->hard_start_xmit(skb, prev->dev);
		#else
		prev->dev->netdev_ops->ndo_start_xmit(skb,prev->dev);
		#endif                            
		fwdCnt++;
	}

	if(fwdCnt==0)
	{
		/*to avoid memory leak*/
		kfree_skb(skb);
	}
	return;
}


#if defined (CONFIG_RTL_MLD_SNOOPING)
struct proc_dir_entry *procMldQuery=NULL;
int mldQueryEnabled=0;	
static int br_mldQueryRead_proc(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{

      int len;

      len = sprintf(page, "%c\n", mldQueryEnabled + '0');


      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;

}

static int br_mldQueryWrite_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
      unsigned char tmp; 
      if (count < 2) 
	    return -EFAULT;
      
	if (buffer && !copy_from_user(&tmp, buffer, 1)) {
		mldQueryEnabled = tmp - '0';
		if(mldQueryEnabled)
		{
			mldQueryEnabled=1;
		}
		else
		{
			mldQueryEnabled=0;
		}
	    return count;
      }
      return -EFAULT;
}

struct proc_dir_entry *procMldVersion=NULL;
int mldVersion=2;
static int br_mldVersionRead_proc(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{

      int len;

      len = sprintf(page, "%c\n", mldVersion + '0');


      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;

}

static int br_mldVersionWrite_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
      unsigned char tmp; 
      if (count < 2) 
	    return -EFAULT;
      
	if (buffer && !copy_from_user(&tmp, buffer, 1)) {
		mldVersion = tmp - '0';
		if(mldVersion>=2)
		{
			mldVersion=2;
		}
		else if (mldVersion<=1)
		{
			mldVersion=1;
		}
		else
		{
			mldVersion=2;
		}
	    return count;
      }
      return -EFAULT;
}

static unsigned char mldQueryBuf[90]={	0x33,0x33,0x00,0x00,0x00,0x01,		/*destination mac*/
									0x00,0x00,0x00,0x00,0x00,0x00,		/*source mac*/	/*offset:6*/
									0x86,0xdd,						/*ether type*/	/*offset:12*/
									0x60,0x00,0x00,0x00,				/*version(1 byte)-traffic cliass(1 byte)- flow label(2 bytes)*/	/*offset:14*/
									0x00,0x20,0x00,0x01,				/*payload length(2 bytes)-next header(1 byte)-hop limit(value:1 1byte)*//*offset:18*/
									0xfe,0x80,0x00,0x00,				/*source address*/	/*offset:22*/
									0x00,0x00,0x00,0x00,				/*be zero*/	/*offset:26*/
									0x00,0x00,0x00,					/*upper 3 bytes mac address |0x02*/ /*offset:30*/
									0xff,0xfe,						/*fixed*/
									0x00,0x00,0x00,					/*lowert 3 bytes mac address*/	 /*offset:35*/
									0xff,0x02,0x00,0x00,				/*destination address is fixed as FF02::1*/	/*offset:38*/
									0x00,0x00,0x00,0x00,			
									0x00,0x00,0x00,0x00,			
									0x00,0x00,0x00,0x01,			
									0x3a,0x00,						/*icmp type(1 byte)-length(1 byte)*/	 /*offset:54*/
									0x05,0x02,0x00,0x00,				/*router alert option*/
									0x01,0x00,						/*padN*/
									0x82,0x00,						/*type(query:0x82)-code(0)*/	/*offset:62*/
									0x00,0x00,						/*checksum*/	/*offset:64*/
									0x00,0x0a,						/*maximum reponse code*/
									0x00,0x00,						/*reserved*/
									0x00,0x00,0x00,0x00,				/*multicast address,fixed as 0*/
									0x00,0x00,0x00,0x00,			
									0x00,0x00,0x00,0x00,			
									0x00,0x00,0x00,0x00,
									0x0a,0x3c,0x00,0x00
								};			

static unsigned char ipv6PseudoHdrBuf[40]=	{	
									0xfe,0x80,0x00,0x00,				/*source address*/
									0x00,0x00,0x00,0x00,			
									0x00,0x00,0x00,0xff,			
									0xfe,0x00,0x00,0x00,			 	
									0xff,0x02,0x00,0x00,				/*destination address*/
									0x00,0x00,0x00,0x00,		
									0x00,0x00,0x00,0x00,			
									0x00,0x00,0x00,0x01,				
									0x00,0x00,0x00,0x18,				/*upper layer packet length*/
									0x00,0x00,0x00,0x3a					/*zero padding(3 bytes)-next header(1 byte)*/
									};		

static unsigned short br_ipv6Checksum(unsigned char *pktBuf, unsigned int pktLen, unsigned char  *ipv6PseudoHdrBuf)
{
	unsigned int  checksum=0;
	unsigned int count=pktLen;
	unsigned short   *ptr;

	/*compute ipv6 pseudo-header checksum*/
	ptr= (unsigned short  *) (ipv6PseudoHdrBuf);	
	for(count=0; count<20; count++) /*the pseudo header is 40 bytes long*/
	{
		  checksum+= ntohs(*ptr);
		  ptr++;
	}
	
	/*compute the checksum of mld buffer*/
	 count=pktLen;
	 ptr=(unsigned short  *) (pktBuf);	
	 while(count>1)
	 {
		  checksum+= ntohs(*ptr);
		  ptr++;
		  count -= 2;
	 }
	 
	if(count>0)
	{
		checksum+= *(pktBuf+pktLen-1)<<8; /*the last odd byte is treated as bit 15~8 of unsigned short*/
	}

	/* Roll over carry bits */
	checksum = (checksum >> 16) + (checksum & 0xffff);
	checksum += (checksum >> 16);

	/* Return checksum */
	return ((uint16) ~ checksum);
	
}
static unsigned char* br_generateMldQuery(struct net_bridge * br)
{
	struct net_device* brDev = NULL;
	unsigned short checkSum=0;
	if(br==NULL)
	{
		return NULL;
	}
	
	brDev = br->dev;
	
	memcpy(&mldQueryBuf[6],brDev->dev_addr,6);			/*set source mac address*/
	
	memcpy(&mldQueryBuf[30],brDev->dev_addr,3);		/*set  mld query packet source ip address*/
	mldQueryBuf[30]=mldQueryBuf[30]|0x02;		
	memcpy(&mldQueryBuf[35],&brDev->dev_addr[3],3);		

	
	memcpy(ipv6PseudoHdrBuf,&mldQueryBuf[22],16);			/*set pseudo-header source ip*/
	if(mldVersion==2)
	{
		mldQueryBuf[19]=	0x24;
	}
	else
	{
		mldQueryBuf[19]=	0x20;
	}

	mldQueryBuf[64]=0;/*reset checksum*/
	mldQueryBuf[65]=0;
	if(mldVersion==2)
	{
		ipv6PseudoHdrBuf[35]=28;
		checkSum=br_ipv6Checksum(&mldQueryBuf[62],28,ipv6PseudoHdrBuf);
	}
	else
	{
		ipv6PseudoHdrBuf[35]=24;
		checkSum=br_ipv6Checksum(&mldQueryBuf[62],24,ipv6PseudoHdrBuf);
	}
	
	
	mldQueryBuf[64]=(checkSum&0xff00)>>8;
	mldQueryBuf[65]=(checkSum&0x00ff);
	return mldQueryBuf;
	
	
}


void br_mldQueryTimerExpired(unsigned long arg)
{
	struct net_bridge *br = (struct net_bridge*) arg;
	struct sk_buff *skb;
	struct sk_buff *skb2;
	struct net_bridge_port *p, *n;
	struct net_bridge_port *prev;
	unsigned int fwdCnt=0;
	unsigned char *mldBuf=NULL;

	if(mldQueryEnabled==0)
	{
		return;
	}
	
#if defined (CONFIG_RTL_QUERIER_SELECTION)
	if(br_querierSelection(br,6)==0)
	{
		return;
	}
#endif	

	skb=dev_alloc_skb(1024);
	if(skb==NULL)
	{
		return;
	}
	
	memset(skb->data,86,0);
	mldBuf=br_generateMldQuery(br);
	if(mldBuf==NULL)
	{
		return;
	}
	
	if(mldVersion==2)
	{
		memcpy(skb->data,mldBuf,90);
		skb->len = 0;
		skb_put(skb, 90);
	}
	else
	{
		memcpy(skb->data,mldBuf,86);
		skb->len = 0;
		skb_put(skb, 86);
	}
 
	prev = NULL;
	fwdCnt=0;
	list_for_each_entry_safe(p, n, &br->port_list, list) 
	{ 
		if ((p->state == BR_STATE_FORWARDING) && (strncmp(p->dev->name, "peth",4)!=0) && (strncmp(p->dev->name, "pwlan",5)!=0)) 
		{
			if (prev != NULL) 
			{                                                                                       
				if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) 
				{
					br->dev->stats.tx_dropped++;
					kfree_skb(skb);
					return 0;
				} 
				skb2->dev=prev->dev;
				#if defined(CONFIG_COMPAT_NET_DEV_OPS)
				prev->dev->hard_start_xmit(skb2, prev->dev);
				#else
				prev->dev->netdev_ops->ndo_start_xmit(skb2,prev->dev);
				#endif                  
				fwdCnt++;
			}
				                                                                               
			prev = p;
		}
	}

	if (prev != NULL) 
	{
		skb->dev=prev->dev;
	       #if defined(CONFIG_COMPAT_NET_DEV_OPS)
		prev->dev->hard_start_xmit(skb, prev->dev);
		#else
		prev->dev->netdev_ops->ndo_start_xmit(skb,prev->dev);
		#endif                            
		fwdCnt++;
	}

	if(fwdCnt==0)
	{
		/*to avoid memory leak*/
		kfree_skb(skb);
	}
	
	return;
}
#endif

static unsigned int mCastQueryTimerCnt=0;
void br_mCastQueryTimerExpired(unsigned long arg)
{
	struct net_bridge *br = (struct net_bridge*) arg;
	
	mod_timer(&br->mCastQuerytimer, jiffies+MCAST_QUERY_INTERVAL*HZ);
    #ifdef CONFIG_RTK_VLAN_WAN_TAG_SUPPORT
		if(!strcmp(br->dev->name,RTL_PS_BR1_DEV_NAME))
		{
			br_igmpQueryTimerExpired(arg);
			return;
		}
	#endif
	
	if(mCastQueryTimerCnt%2==0)
	{
		br_igmpQueryTimerExpired(arg);
	}
	else
	{
		#if defined (CONFIG_RTL_MLD_SNOOPING)
		br_mldQueryTimerExpired(arg);
		#endif
	}
	mCastQueryTimerCnt++;
	
	return;
}

extern struct net_bridge *find_br_by_name(char *name);

void br_signal_igmpProxy(void)
{

	struct task_struct *task;
	struct net_bridge *br;

	br = find_br_by_name(RTL_PS_BR0_DEV_NAME);
	if(br==NULL)
	{
		return;
	}

	if(br->igmpProxy_pid==0)
	{
		return;
	}
	
	read_lock(&tasklist_lock);
//	task = find_task_by_pid(br->igmpProxy_pid);
	task = find_task_by_vpid(br->igmpProxy_pid);
	read_unlock(&tasklist_lock);
	if(task)
	{
		send_sig(SIGUSR2,task,0);
	}
	else {
	    //printk("Path selection daemon pid: %d does not exist\n", br->mesh_pathsel_pid);
	}
}

#endif

struct proc_dir_entry *procIgmpProxy = NULL;
static int br_igmpProxyRead_proc(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{
      int len;
      len = sprintf(page, "%c\n", IGMPProxyOpened + '0');

      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;

}
static int br_igmpProxyWrite_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
    unsigned char chartmp; 
	  
    if (count > 1) {	//call from shell
      	if (buffer && !copy_from_user(&chartmp, buffer, 1)) {
	    	IGMPProxyOpened = chartmp - '0';			
	    }
	}else if(count==1){//call from demon(demon direct call br's ioctl)
			//memcpy(&chartmp,buffer,1);
			if(buffer){
				get_user(chartmp,buffer);	
		    		IGMPProxyOpened = chartmp - '0';
			}else
				return -EFAULT;

	}else{

		return -EFAULT;
	}
	return count;
}

static int __init br_init(void)
{
	int err;

	err = stp_proto_register(&br_stp_proto);
	if (err < 0) {
		printk(KERN_ERR "bridge: can't register sap for STP\n");
		return err;
	}

	err = br_fdb_init();
	if (err)
		goto err_out;

	err = register_pernet_subsys(&br_net_ops);
	if (err)
		goto err_out1;

#if defined (CONFIG_RTL865X_LANPORT_RESTRICTION)
	lan_restrict_init();
#endif

#if defined (CONFIG_RTL_EAP_RELAY) || defined (CONFIG_RTK_INBAND_HOST_HACK)
		procHostMac = create_proc_entry("br_hostmac", 0, NULL);
		if (procHostMac) {
		    procHostMac->read_proc = br_hostmac_read_proc;
		    procHostMac->write_proc = br_hostmac_write_proc;
		}
#endif

#if defined (CONFIG_RTK_INBAND_HOST_HACK)
		procHostIp = create_proc_entry("br_hostip", 0, NULL);
		if (procHostIp) {
		    procHostIp->read_proc = br_hostip_read_proc;
		    procHostIp->write_proc = br_hostip_write_proc;
		}
#endif


#if defined(CONFIG_PROC_FS) && defined (CONFIG_RTL_WLAN_BLOCK_RELAY)
		procwlanblockrelay = create_proc_entry("br_wlanblock", 0, NULL);
		if(procwlanblockrelay) {
			procwlanblockrelay->read_proc = br_wlanblockread_proc;
			procwlanblockrelay->write_proc = br_wlanblockwrite_proc;
		}
#endif

#if defined (CONFIG_PROC_FS) && defined (CONFIG_RTL_IGMP_SNOOPING)	
		procIgmpSnoop = create_proc_entry("br_igmpsnoop", 0, NULL);
		if (procIgmpSnoop) {
		    procIgmpSnoop->read_proc = br_igmpSnoopRead_proc;
		    procIgmpSnoop->write_proc = br_igmpSnoopWrite_proc;
		}

		procIgmpDb=create_proc_entry("br_igmpDb", 0, NULL);
		if(procIgmpDb != NULL)
		{
			procIgmpDb->proc_fops = &igmp_db_seq_file_operations;

		}

		procMCastFastFwd= create_proc_entry("br_mCastFastFwd", 0, NULL);
		if (procMCastFastFwd) {
		    procMCastFastFwd->read_proc = br_mCastFastFwdRead_proc;
		    procMCastFastFwd->write_proc = br_mCastFastFwdWrite_proc;
		}

		procIgmpVersion= create_proc_entry("br_igmpVersion", 0, NULL);
		if (procIgmpVersion) {
		    procIgmpVersion->read_proc = br_igmpVersionRead_proc;
		    procIgmpVersion->write_proc = br_igmpVersionWrite_proc;
		}
		
		procIgmpQuery= create_proc_entry("br_igmpquery", 0, NULL);
		if (procIgmpQuery) {
		    procIgmpQuery->read_proc = br_igmpQueryRead_proc;
		    procIgmpQuery->write_proc = br_igmpQueryWrite_proc;
		}
#if defined (CONFIG_RTL_QUERIER_SELECTION)		
		br_initQuerierInfo();
		procIgmpQuerierInfo = create_proc_entry("br_igmpQuerierInfo", 0, NULL);
		if (procIgmpQuerierInfo) {
			procIgmpQuerierInfo->read_proc = br_igmpQuerierInfoRead_proc;
			procIgmpQuerierInfo->write_proc = br_igmpQuerierInfoWrite_proc;
		}

		procMldQuerierInfo = create_proc_entry("br_mldQuerierInfo", 0, NULL);
		if (procMldQuerierInfo) {
			procMldQuerierInfo->read_proc = br_mldQuerierInfoRead_proc;
			procMldQuerierInfo->write_proc = br_mldQuerierInfoWrite_proc;
		}
#endif

#if defined (CONFIG_RTL_MLD_SNOOPING)
		procMldVersion= create_proc_entry("br_mldVersion", 0, NULL);
		if (procMldVersion) {
		    procMldVersion->read_proc = br_mldVersionRead_proc;
		    procMldVersion->write_proc = br_mldVersionWrite_proc;
		}
		procMldQuery= create_proc_entry("br_mldquery", 0, NULL);
		if (procMldQuery) {
		    procMldQuery->read_proc = br_mldQueryRead_proc;
		    procMldQuery->write_proc = br_mldQueryWrite_proc;
		}
#endif
#endif

#if  defined (CONFIG_PROC_FS) 	
		procIgmpProxy = create_proc_entry("br_igmpProxy", 0, NULL);
		if (procIgmpProxy) {
		    procIgmpProxy->read_proc = br_igmpProxyRead_proc;
		    procIgmpProxy->write_proc = br_igmpProxyWrite_proc;
		}
#endif
	err = br_netfilter_init();
	if (err)
		goto err_out2;

	err = register_netdevice_notifier(&br_device_notifier);
	if (err)
		goto err_out3;

	err = br_netlink_init();
	if (err)
		goto err_out4;

	brioctl_set(br_ioctl_deviceless_stub);
	br_handle_frame_hook = br_handle_frame;

	br_fdb_get_hook = br_fdb_get;
	br_fdb_put_hook = br_fdb_put;
#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT) || defined(CONFIG_RTL_ULINKER)
	br_filter_init();
#endif
#if defined(CONFIG_HTTP_FILE_SERVER_SUPPORT) || defined(CONFIG_RTL_USB_UWIFI_HOST_SPEEDUP)
	http_file_init();
#endif	
	return 0;
err_out4:
	unregister_netdevice_notifier(&br_device_notifier);
err_out3:
	br_netfilter_fini();
err_out2:
	unregister_pernet_subsys(&br_net_ops);
err_out1:
	br_fdb_fini();
err_out:
	stp_proto_unregister(&br_stp_proto);
	return err;
}

static void __exit br_deinit(void)
{
	stp_proto_unregister(&br_stp_proto);

#if  defined (CONFIG_PROC_FS) && defined (CONFIG_RTL_IGMP_SNOOPING)
	if (procIgmpSnoop) {
		remove_proc_entry("br_igmpsnoop", procIgmpSnoop);		
		procIgmpSnoop = NULL;
	}

	if(procIgmpDb!=NULL)
	{
		remove_proc_entry("br_igmpDb", procIgmpDb);
	}
	
	if (procMCastFastFwd) {
		remove_proc_entry("br_mCastFastFwd", procMCastFastFwd);		
		procMCastFastFwd = NULL;
	}

	if (procIgmpVersion) {
	 	 remove_proc_entry("br_igmpVersion", procIgmpVersion);		
		procIgmpVersion = NULL;
	}
	
	if (procIgmpQuery) {
	 	 remove_proc_entry("br_igmpquery", procIgmpQuery);		
		procIgmpQuery = NULL;
	}

#if defined (CONFIG_RTL_MLD_SNOOPING)
	if (procMldVersion) {
	 	 remove_proc_entry("br_mldVersion", procMldVersion);		
		procMldVersion = NULL;
	}
	if (procMldQuery) {
		 remove_proc_entry("br_mldquery", procMldQuery);		
		procMldQuery = NULL;
	}
#endif	

#if defined (CONFIG_RTL_QUERIER_SELECTION)		
		if (procIgmpQuerierInfo) {
			remove_proc_entry("br_igmpQuerierInfo", procIgmpQuerierInfo);	
			procIgmpQuerierInfo=NULL;
		}

		if (procMldQuerierInfo) {
			remove_proc_entry("br_mldQuerierInfo", procMldQuerierInfo);	
			procMldQuerierInfo=NULL;
		}
#endif

#endif	

#if  defined (CONFIG_PROC_FS) 
	if (procIgmpProxy) {
		remove_proc_entry("br_igmpProxy", procIgmpProxy);		
		procIgmpProxy = NULL;
	}
#endif
	br_netlink_fini();
	unregister_netdevice_notifier(&br_device_notifier);
	brioctl_set(NULL);

	unregister_pernet_subsys(&br_net_ops);

	synchronize_net();

	br_netfilter_fini();
	br_fdb_get_hook = NULL;
	br_fdb_put_hook = NULL;

	br_handle_frame_hook = NULL;
	br_fdb_fini();
#if defined(CONFIG_DOMAIN_NAME_QUERY_SUPPORT) || defined(CONFIG_RTL_ULINKER)
	br_filter_exit();
#endif
#if defined(CONFIG_HTTP_FILE_SERVER_SUPPORT) || defined(CONFIG_RTL_USB_UWIFI_HOST_SPEEDUP)
	http_file_exit();
#endif	
}

EXPORT_SYMBOL(br_should_route_hook);

module_init(br_init)
module_exit(br_deinit)
MODULE_LICENSE("GPL");
MODULE_VERSION(BR_VERSION);
