/*
 *      Handling routines for Mesh in 802.11 Proc
 *
 *      PS: All extern function in ../8190n_headers.h
 */
#define _MESH_PROC_C_

#ifdef CONFIG_RTL8192CD
#include "../rtl8192cd/8192cd.h"
#include "../rtl8192cd/8192cd_headers.h"
#else
#include "../8190n.h"
#include "../8190n_headers.h"
#endif
#include "./mesh.h"
#include "./mesh_route.h"
#ifdef MESH_USE_METRICOP
#include "mesh_11kv.h"
#ifdef __KERNEL__
#include <linux/init.h>
#include <asm/uaccess.h>
#endif
#endif

#if defined(DBG_NCTU_MESH) || defined(_MESH_DEBUG_)
#ifdef __KERNEL__
#include <linux/init.h>
#include <asm/uaccess.h>
#endif
#include <linux/module.h>
#endif

#ifdef CONFIG_RTK_MESH

/*
 *	Note : These define copy from 8190n_proc.c !!
*/
#define PRINT_ONE(val, format, line_end) { 		\
	pos += sprintf(&buf[pos], format, val);		\
	if (line_end)					\
		strcat(&buf[pos++], "\n");		\
}

#define PRINT_ARRAY(val, format, len, line_end) { 	\
	int index;					\
	for (index=0; index<len; index++)		\
		pos += sprintf(&buf[pos], format, val[index]); \
	if (line_end)					\
		strcat(&buf[pos++], "\n");		\
							\
}

#define PRINT_SINGL_ARG(name, para, format) { \
	PRINT_ONE(name, "%s", 0); \
	PRINT_ONE(para, format, 1); \
}

#define PRINT_ARRAY_ARG(name, para, format, len) { \
	PRINT_ONE(name, "%s", 0); \
	PRINT_ARRAY(para, format, len, 1); \
}

#define CHECK_LEN { \
	len += size; \
	pos = begin + len; \
	if (pos < offset) { \
		len = 0; \
		begin = pos; \
	} \
	if (pos > offset + length) \
		goto _ret; \
}

#if (MESH_DBG_LV & MESH_DBG_COMPLEX)

#define MESH_PROC_SME_TEST_FILENAME  "mesh_dst_mac"

int mesh_test_sme_proc_read(char *buffer, char **buffer_location, off_t offset, int buffer_length, int *eof, void *data)
{
	int ret;

	if (offset > 0) {
		/* we have finished to read, return 0 */
		ret  = 0;
	} else {
		char str[20];
		
		sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X\r\n", 
				mesh_test_dst_mac[0]&0xff, mesh_test_dst_mac[1]&0xff, mesh_test_dst_mac[2]&0xff,
				mesh_test_dst_mac[3]&0xff, mesh_test_dst_mac[4]&0xff, mesh_test_dst_mac[5]&0xff);
			
		/* fill the buffer, return the buffer size */
		memcpy(buffer, str, 19); // 6*2 + 5 + 2
		ret = 19;
	}
	return ret;
}

int mesh_test_sme_proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char local_buf[19];
	char *ptr;
	
	int idx = 0;
	unsigned char mac[6];

	/* get buffer size */
	unsigned long local_buf_size = count;
	
	memset(local_buf, 0, sizeof(local_buf));
			
	if (count < 11 ) { // a:b:c:d:e:f
		return -EFAULT;
	}
	
	if (count > 18 ) {
		local_buf_size = 18;
	}
	
	/* write data to the buffer */
	if ( copy_from_user((void *)local_buf, (const void *)buffer, local_buf_size) ) {
		printk("*** mesh_test_sme_proc_write: copy from user error\r\n");
		return -EFAULT;
	}
	
	memset(mac, 0, sizeof(mac));
	ptr = local_buf;
	while(*ptr)
	{
		unsigned char val = 0;
		
		if(idx>5)
			break;
			
		if(*ptr == ':')
		{
			ptr++;
			idx++;
			continue;
		}
		if( (*ptr>='0') && (*ptr<='9') )
		{
			val = *ptr - '0';
		}
		else if( (*ptr>='A') && (*ptr<='F') )
		{
			val = *ptr - 'A' + 10;
		}
		else if( (*ptr>='a') && (*ptr<='f') )
		{
			val = *ptr - 'a' + 10;
		}
		else
		{
			ptr++;
			continue;
		}
		mac[idx] = mac[idx]*16+val;

		ptr++;
	}
	memcpy(mesh_test_dst_mac, mac, 6);
	
	return local_buf_size;

}

#endif // (MESH_DBG_LV & MESH_DBG_COMPLEX)


/*
 *	@brief	Printout assigned mesh MP neighbor table
 *		PS: Modify from dump__one_stainfo
 *
 *	@param	Unknow
 *
 *	@retval	int: pos:Unknow
 */
static int dump_mesh_one_mpinfo(int num, struct stat_info *pstat, char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	int pos = 0,i = 0;
	signed long tmp;

	unsigned char network;
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;

	tmp = (signed long)(pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod - jiffies);
	if (0 > tmp)
		tmp = 0;
	
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
		network = WIRELESS_11A;
	else if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) {
		if (!isErpSta(pstat))
			network = WIRELESS_11B;
		else {
			network = WIRELESS_11G;
			for (i=0; i<STAT_OPRATE_LEN; i++) {
				if (is_CCK_rate(STAT_OPRATE[i])) {
					network |= WIRELESS_11B;
					break;
				}
			}
		}
	}
	else // 11B only
		network = WIRELESS_11B;
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
		if (pstat->ht_cap_len)
			network |= WIRELESS_11N;
	}
	
	PRINT_ONE(num,  " %d: Mesh MP_info...", 1);
	PRINT_SINGL_ARG("    state: ", pstat->mesh_neighbor_TBL.State, "%x");
	PRINT_ARRAY_ARG("    hwaddr: ",	pstat->hwaddr, "%02x", MACADDRLEN);
	if( !(network&(~WIRELESS_11B)) )
	{
		PRINT_ONE("    mode: 11b","%s",1);
	}
	else if( !(network&(~(WIRELESS_11G|WIRELESS_11B))) )
	{
		PRINT_ONE("    mode: 11g","%s",1);
	}
	else if( !(network&(~(WIRELESS_11N|WIRELESS_11G|WIRELESS_11B))) )
	{
		PRINT_ONE("    mode: 11n","%s",1);
	}
	else
	{
		PRINT_ONE("    mode: 11a","%s",1);
	}

	PRINT_SINGL_ARG("    Tx Packets: ", pstat->tx_pkts, "%u");
	PRINT_SINGL_ARG("    Rx Packets: ", pstat->rx_pkts, "%u");
	PRINT_SINGL_ARG("    Authentication: ", ((pstat->state & WIFI_AUTH_SUCCESS) ? 1 : 0), "%d");
	PRINT_SINGL_ARG("    Assocation: ", ((pstat->state & WIFI_ASOC_STATE) ? 1 : 0), "%d");
	PRINT_SINGL_ARG("    LocalLinkID: ", pstat->mesh_neighbor_TBL.LocalLinkID, "%lu");	// %lu=unsigned long
	PRINT_SINGL_ARG("    PeerLinkID: ", pstat->mesh_neighbor_TBL.PeerLinkID, "%lu");		// %lu=unsigned long
	PRINT_SINGL_ARG("    operating_CH: ", pstat->mesh_neighbor_TBL.Co, "%u");
	PRINT_SINGL_ARG("    CH_precedence: ", pstat->mesh_neighbor_TBL.Pl, "%lu");		// %lu=unsigned long
	//PRINT_SINGL_ARG("    R: ", pstat->mesh_neighbor_TBL.r, "%u");
	if( is_MCS_rate(pstat->current_tx_rate) )
	{
		PRINT_SINGL_ARG("    R: ", MCS_DATA_RATEStr[(pstat->ht_current_tx_info&BIT(0))?1:0][(pstat->ht_current_tx_info&BIT(1))?1:0][pstat->mesh_neighbor_TBL.r], "%s");
	}
	else
	{
		PRINT_SINGL_ARG("    R: ", pstat->mesh_neighbor_TBL.r, "%u");
	}
	PRINT_SINGL_ARG("    Ept: ", pstat->mesh_neighbor_TBL.ept, "%u");
	PRINT_SINGL_ARG("    rssi: ", pstat->mesh_neighbor_TBL.Q, "%u");
	PRINT_SINGL_ARG("    expire_Establish(jiffies): ", (pstat->mesh_neighbor_TBL.expire - jiffies), "%ld");		// %lu=unsigned long
	//PRINT_SINGL_ARG("                    (mSec): ", ((pstat->mesh_neighbor_TBL.expire - jiffies)*(1000/HZ)), "%ld");
	PRINT_SINGL_ARG("                    (Sec): ", ((pstat->mesh_neighbor_TBL.expire - jiffies)/100), "%ld");
	PRINT_SINGL_ARG("    expire_BootSeq & LLSA(jiffies): ", tmp, "%ld");		// %lu=unsigned long
	PRINT_SINGL_ARG("                         (mSec): ", ((tmp)*(1000/HZ)), "%ld");
	PRINT_SINGL_ARG("    retry: ", pstat->mesh_neighbor_TBL.retry, "%d");

//	PRINT_ONE("", "%s", 1);		// Closed, Because Throughput statistics (sounder)

	return pos;
}

#ifdef MESH_BOOTSEQ_AUTH
/*
 *	@brief	Printout mesh MP neighbor table in Auth list
 *		PS: Modify from rtl8190_proc_stainfo
 *
 *	@param	Unknow
 *
 *	@retval	int: pos:Unknow
 */
int mesh_auth_mpinfo(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	int len = 0;
	off_t begin = 0;
	off_t pos = 0;
	int size, num=1;
	struct list_head *phead, *plist;
	struct stat_info *pstat;

	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		sprintf(buf, "-- Mesh MP Auth Peer info table -- \n");
		size = strlen(buf);
		CHECK_LEN;

		phead = &priv->mesh_auth_hdr;
		if (!netif_running(dev) || list_empty(phead))
			goto _ret;

		plist = phead->next;
		while (plist != phead) {
			pstat = list_entry(plist, struct stat_info, mesh_mp_ptr);
			size = dump_mesh_one_mpinfo(num++, pstat, buf+len, start, offset, length,
						eof, data);
			CHECK_LEN;

			plist = plist->next;
		}
	}
	else {
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);
		return	pos;
	}		
	*eof = 1;

_ret:
	*start = buf + (offset - begin);	/* Start of wanted data */
	len -= (offset - begin);	/* Start slop */
	if (len > length)
		len = length;	/* Ending slop */

	return len;
}	
#endif

/*
 *	@brief	Printout mesh MP neighbor table unEstablish list
 *		PS:Modify from rtl8190_proc_stainfo 
 *
 *	@param	Unknow
 *
 *	@retval	int: pos:Unknow
 */
int mesh_unEstablish_mpinfo(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	int len = 0;
	off_t begin = 0;
	off_t pos = 0;
	int size, num=1;
	struct list_head *phead, *plist;
	struct stat_info *pstat;

	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		sprintf(buf, "-- Mesh MP unEstablish Peer info table -- \n");
		size = strlen(buf);
		CHECK_LEN;

		phead = &priv->mesh_unEstablish_hdr;
		if (!netif_running(dev) || list_empty(phead))
			goto _ret;

		plist = phead->next;
		while (plist != phead) {
			pstat = list_entry(plist, struct stat_info, mesh_mp_ptr);
			size = dump_mesh_one_mpinfo(num++, pstat, buf+len, start, offset, length,
						eof, data);
			CHECK_LEN;

			plist = plist->next;
		}
	}
	else {
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);
		return	pos;
	}		
	*eof = 1;

_ret:
	*start = buf + (offset - begin);	/* Start of wanted data */
	len -= (offset - begin);	/* Start slop */
	if (len > length)
		len = length;	/* Ending slop */

	return len;
}	



/*
 *	@brief	Printout sta_info all of mesh MP neighbor flow, Throughput statistics (sounder)
 *
 *	@param	unknow
 *
 *	@retval	int: pos:unknow
 */
static int dump_mesh_one_mpflow_neighbor(int num, struct stat_info *pstat, char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;

	int pos = 0;

	if(priv->mesh_log)
	{
		PRINT_SINGL_ARG("    log_time: ",		priv->log_time, "%lu");
		PRINT_SINGL_ARG("    rx_packets: ",		pstat->rx_pkts, "%u");
		PRINT_SINGL_ARG("    rx_bytes: ",		pstat->rx_bytes,"%u");
		PRINT_SINGL_ARG("    tx_packets: ",		pstat->tx_pkts, "%u");
		PRINT_SINGL_ARG("    tx_bytes: ",		pstat->tx_bytes,"%u");
	}
	else
	{
		PRINT_SINGL_ARG("    log_time: ",		-99, "%d");
		PRINT_SINGL_ARG("    rx_packets: ",		-99, "%d");
		PRINT_SINGL_ARG("    rx_bytes: ",		-99, "%d");
		PRINT_SINGL_ARG("    tx_packets: ",		-99, "%d");
		PRINT_SINGL_ARG("    tx_bytes: ",		-99, "%d");
	}
	PRINT_ONE("", "%s", 1);

	return pos;
}



/*
 *	@brief	Printout sta_info all of mesh MP neighbor flow,  Throughput statistics (sounder)
 *
 *	@param	unknow
 *
 *	@retval	int: pos:unknow
 */
int dump_mesh_one_mpflow_sta(int num, struct stat_info *pstat, char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;

	struct list_head *phead, *plist;
	struct stat_info *pstat1;
	int result = 0;
	int pos = 0;

	if(priv->mesh_log)
	{
		PRINT_SINGL_ARG("    log_time: ",		priv->log_time, "%lu");
		PRINT_SINGL_ARG("    rx_packets: ",		pstat->rx_pkts, "%u");
		PRINT_SINGL_ARG("    rx_bytes: ",		pstat->rx_bytes,"%u");
		PRINT_SINGL_ARG("    tx_packets: ",		pstat->tx_pkts, "%u");
		PRINT_SINGL_ARG("    tx_bytes: ",		pstat->tx_bytes,"%u");

		phead = &priv->mesh_mp_hdr;

		plist = phead->next;
		while (plist != phead) 
		{
			pstat1 = list_entry(plist, struct stat_info, mesh_mp_ptr);
			if(pstat->hwaddr == pstat1->hwaddr )
			{
				result = 1;
				break;
			}
	 		plist = plist->next;
		}

		PRINT_SINGL_ARG("    mesh_node: ",		result,"%u");
	}
	else
	{
		PRINT_SINGL_ARG("    log_time: ",		-99, "%d");
		PRINT_SINGL_ARG("    rx_packets: ",		-99, "%d");
		PRINT_SINGL_ARG("    rx_bytes: ",		-99, "%d");
		PRINT_SINGL_ARG("    tx_packets: ",		-99, "%d");
		PRINT_SINGL_ARG("    tx_bytes: ",		-99, "%d");
		PRINT_SINGL_ARG("    mesh_node: ",		-99, "%d");
	}
	PRINT_ONE("", "%s", 1);

	return pos;
}



/*
 *	@brief	Printout mesh MP neighbor table in association successful
 *		PS:Modify from rtl8190_proc_stainfo
 *
 *	@param	Unknow
 *
 *	@retval	int: pos:Unknow
 */
int mesh_assoc_mpinfo(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	int len = 0;
	off_t begin = 0;
	off_t pos = 0;
	int size, num=1;
	struct list_head *phead, *plist;
	struct stat_info *pstat;
	
	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		sprintf(buf, "-- Mesh MP Association peer info table -- \n");
		size = strlen(buf);
		CHECK_LEN;

		phead = &priv->mesh_mp_hdr;
		if (!netif_running(dev) || list_empty(phead))
			goto _ret;

		plist = phead->next;
		while (plist != phead) {
			pstat = list_entry(plist, struct stat_info, mesh_mp_ptr);
			size = dump_mesh_one_mpinfo(num++, pstat, buf+len, start, offset, length,
						eof, data);
			CHECK_LEN;

			// 3 line for Throughput statistics (sounder)	
			size = dump_mesh_one_mpflow_neighbor(num, pstat, buf+len, start, offset, length,
						eof, data);
			CHECK_LEN;

			plist = plist->next;
		}
	} else {
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);
		return	pos;
	}
	*eof = 1;

_ret:
	*start = buf + (offset - begin);	/* Start of wanted data */
	len -= (offset - begin);	/* Start slop */
	if (len > length)
		len = length;	/* Ending slop */

	return len;
}

/*
 *	@brief	Control flow Throughput statistics (sounder)
 *		
 *	@param	unknow
 *
 *	@retval	int: count:unknow
 */
int mesh_proc_flow_stats_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;

	struct list_head *phead, *plist;
	struct stat_info *pstat;

	//if( priv->mesh_log )
        if(*buffer == '0') 
	{		
		// turn off log function
		priv->mesh_log = 0;
		priv->log_time = 0;
	}
	else
	{
		// reset all log variable
		phead = &priv->asoc_list;
		if (!netif_running(dev) || list_empty(phead))
			goto _ret;

		plist = phead->next;
		
		while (plist != phead) 
		{
			pstat = list_entry(plist, struct stat_info, asoc_list);

			pstat->rx_pkts  = 0;
			pstat->rx_bytes = 0;
			pstat->tx_pkts  = 0; 
			pstat->tx_bytes = 0;
			
			plist = plist->next;
		}
		priv->mesh_log = 1;
	}
_ret:

	return count;
}

/*
 *	@brief	Printout flow Throughput statistics (sounder)
 *		
 *	@param	unknow
 *
 *	@retval	int: pos:unknow
 */
int mesh_proc_flow_stats(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;

	off_t pos = 0;

	if( priv->mesh_log )
	{
		PRINT_ONE("1", "%s", 1);
	}
	else
	{
		PRINT_ONE("0", "%s", 1);
	}

	return	pos;
}


/*
 *	@brief	Print all about of mesh statistics and parameter.
 *		
 *	@param	Unknow
 *
 *	@retval	int: pos:Unknow
 */
int mesh_stats(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	int pos = 0;
	
	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		PRINT_ONE("  Systems Statistics...", "%s", 1);
		PRINT_SINGL_ARG("    OPMODE:       ", OPMODE, "%X");
		PRINT_SINGL_ARG("    jiffies:      ", jiffies, "%lu");		// %lu=unsigned long
		
		PRINT_ONE("  Mesh Networks Statistics...", "%s", 1);
		
		PRINT_SINGL_ARG("    tx_packets:   ", priv->mesh_stats.tx_packets, "%lu");
		PRINT_SINGL_ARG("    tx_bytes:     ", priv->mesh_stats.tx_bytes, "%lu");
		PRINT_SINGL_ARG("    tx_errors:    ", priv->mesh_stats.tx_errors, "%lu");
		PRINT_SINGL_ARG("    rx_packets:   ", priv->mesh_stats.rx_packets, "%lu");
		PRINT_SINGL_ARG("    rx_bytes:     ", priv->mesh_stats.rx_bytes, "%lu");
		PRINT_SINGL_ARG("    rx_errors:    ", priv->mesh_stats.rx_errors, "%lu");
		PRINT_SINGL_ARG("    rx_crc_errors: ", priv->mesh_stats.rx_crc_errors, "%lu");

		PRINT_ONE("  WLAN Mesh Capability...", "%s", 1);	
		PRINT_SINGL_ARG("    Version:          ", priv->mesh_Version, "%u");
		PRINT_ONE("    Peer_CAP:", "%s", 1);
		PRINT_SINGL_ARG("      Capacity:       ", MESH_PEER_LINK_CAP_NUM(priv), "%hd");
		PRINT_SINGL_ARG("      Flags:          ", (priv->mesh_PeerCAP_flags & MESH_PEER_LINK_CAP_FLAGS_MASK), "%hX");
		PRINT_SINGL_ARG("    Power_save_CAP:   ", priv->mesh_PowerSaveCAP, "%X");
		PRINT_SINGL_ARG("    SYNC_CAP:         ", priv->mesh_SyncCAP, "%X");
		PRINT_SINGL_ARG("    MDA_CAP:          ", priv->mesh_MDA_CAP, "%X");
		PRINT_SINGL_ARG("    ChannelPrecedence:", priv->mesh_ChannelPrecedence, "%lu");

#ifdef _MESH_ACL_ENABLE_
		PRINT_ONE("  Access Control List (ACL) cache content...", "%s", 1);
		PRINT_ARRAY_ARG("    MAC:          ", priv->meshAclCacheAddr, "%02x", MACADDRLEN);
		PRINT_SINGL_ARG("    Mode:         ", priv->meshAclCacheMode, "%d");
#endif

	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);

	return pos;
}

int mesh_pathsel_routetable_info(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	int len = 0;
	off_t begin = 0;
	off_t pos = 0;
	struct path_sel_entry * ptable;
	int i=0, portal_n=0, j=0;
	
	struct pann_mpp_tb_entry * mpptable;
	
	//chuangch 2007.09.14
	//ptable = get_g_pathtable();
	int num = 1;
	
	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		// sprintf(buf, "-- Mesh Pathselection Route info table -- \n");
		buf[0] = 0;
		if (!netif_running(dev) )
			goto _ret;
	
		//by brian, show the mp itself
		PRINT_ONE(num,  " %d: Mesh route table info...", num++);
		PRINT_ONE("    destMAC: My-self", "%s", 1);
		PRINT_ONE("    nexthopMAC: ---", "%s", 1);

		if( priv->pmib->dot1180211sInfo.mesh_portal_enable )
		{
		  PRINT_ONE("    portal enable: yes", "%s", 1);
    }
		else
		{
		  PRINT_ONE("    portal enable: no", "%s", 1);
    }
	
		PRINT_ONE("    dsn: ---", "%s", 1);
		PRINT_ONE("    metric: ---", "%s", 1);
		PRINT_ONE("    hopcount: ---", "%s", 1);
	
		PRINT_ONE("    start: ---", "%s" ,1);
		PRINT_ONE("    end: ---", "%s", 1);
		PRINT_ONE("    diff: ---", "%s", 1);
		PRINT_ONE("    flag: ---", "%s", 1);
			
		PRINT_ONE("", "%s", 1);
				
		mpptable = (struct pann_mpp_tb_entry *) &(priv->pann_mpp_tb->pann_mpp_pool);
		
		ptable = (struct path_sel_entry*)priv->pathsel_table->entry_array[i].data;
		for(i=0;i<(1 << priv->pathsel_table->table_size_power);i++)
		{
		  int isPortal=0;
			if(priv->pathsel_table->entry_array[i].dirty != 0)
			{			
				ptable = (struct path_sel_entry*)priv->pathsel_table->entry_array[i].data;
				PRINT_ONE(num,  " %d: Mesh route table info...", num++);
//				PRINT_SINGL_ARG("    isvalid: ", ptable->isvalid, "%x");
				PRINT_ARRAY_ARG("    destMAC: ",	ptable->destMAC, "%02x", MACADDRLEN);
				PRINT_ARRAY_ARG("    nexthopMAC: ",	ptable->nexthopMAC, "%02x", MACADDRLEN);
			
				for( j=0; j<MAX_MPP_NUM ;j++ )
				{
				  if( mpptable[j].flag && !memcmp(mpptable[j].mac, ptable->destMAC, MACADDRLEN) )
				  {
				    isPortal = 1;
				    break;
          }
        }
        if( isPortal )
        {
          PRINT_ONE("    portal enable: yes", "%s", 1);
        }
        else
        {
          PRINT_ONE("    portal enable: no", "%s", 1);
        } 
			
				PRINT_SINGL_ARG("    dsn: ", ptable->dsn, "%u");	// %lu=unsigned long
				PRINT_SINGL_ARG("    metric: ", ptable->metric, "%u");
				PRINT_SINGL_ARG("    hopcount: ", ptable->hopcount, "%u");		// %lu=unsigned long
//				PRINT_SINGL_ARG("    modify_time: ", ptable->modify_time, "%u");
			
				PRINT_SINGL_ARG("    start: ", ptable->start, "%u");
				PRINT_SINGL_ARG("    end: ", ptable->end, "%u");
				PRINT_SINGL_ARG("    diff: ", ptable->end-ptable->start, "%u");
				PRINT_SINGL_ARG("    flag: ", ptable->flag, "%d");
			
				PRINT_ONE("", "%s", 1);
#if 0 // stanley
				{
					unsigned char *p = ptable->destMAC;
					printk("path to: %02X%02X%02X%02X%02X%02X\n", *(p), *(p+1), *(p+2), *(p+3), *(p+4), *(p+5));
				}
#endif
			}	
		}	
	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);
	
	*eof = 1;

_ret:
	
	*start = buf + (offset - begin);	/* Start of wanted data */

	len = strlen(buf);

	len -= (offset - begin);	/* Start slop */
	if (len > length)
		len = length;	/* Ending slop */

	return len;
}

/*
 *	@brief	Printout 802.11s pathselection proxy table value
 *
 *	@param	Unknow
 *
 *	@retval	int: pos:Unknow
 */
int mesh_proxy_table_info(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{

	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	int len = 0;
	off_t begin = 0;
	off_t pos = 0;
	struct proxy_table_entry * ptable_entry;
	int i=0;

	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		// sprintf(buf, "-- Mesh Proxy info -- \n");
		buf[0] = 0;
		if (!netif_running(dev) )
			goto _ret;
	
		//ptable = get_g_pathtable();
		int num = 1;
		for(i = 0; i < (1 << priv->proxy_table->table_size_power); i++)
		{
			if(priv->proxy_table->entry_array[i].dirty != 0)
			{
				ptable_entry = (struct proxy_table_entry*)priv->proxy_table->entry_array[i].data;
				PRINT_ONE(num,  " %d: Mesh proxy table info...", num++);
				
				PRINT_ARRAY_ARG("    STA_MAC: ",	ptable_entry->sta, "%02x", MACADDRLEN);
				PRINT_ARRAY_ARG("    OWNER_MAC: ",	ptable_entry->owner, "%02x", MACADDRLEN);				
			}
		}	
	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);
	
	*eof = 1;

_ret:
	
	*start = buf + (offset - begin);	// Start of wanted data

	len = strlen(buf);

	len -= (offset - begin);	// Start slop 
	if (len > length)
		len = length;	// Ending slop

	return len;
}


/*
 *	@brief	Printout 802.11s pathselection rtl8190_root_info
 *
 *	@param	Unknow
 *
 *	@retval	int: pos:Unknow
 */
int mesh_root_info(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{

	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	int len = 0;
	off_t begin = 0;
	off_t pos = 0;

	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		// sprintf(buf, "-- Mesh Root Info -- \n");
		buf[0] = 0;
		if (!netif_running(dev) )
			goto _ret;
		
		PRINT_ARRAY_ARG("    ROOT_MAC: ",	priv->root_mac, "%02x", MACADDRLEN);
	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);

	*eof = 1;

_ret:
	
	*start = buf + (offset - begin);	// Start of wanted data

	len = strlen(buf);

	len -= (offset - begin);	// Start slop 
	if (len > length)
		len = length;	// Ending slop

	return len;
}

/*
 *	@brief	Printout 802.11s pathselection portal  table value
 *
 *	@param	Unknow
 *
 *	@retval	int: pos:Unknow
 */
int mesh_portal_table_info(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	int len = 0;
	off_t begin = 0;
	off_t pos = 0;
	struct pann_mpp_tb_entry * ptable;
	int i=0;
	
	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		// sprintf(buf, "-- Mesh Portal info table -- \n");
		buf[0] = 0;
		if (!netif_running(dev) )
			goto _ret;
	
		ptable = (struct pann_mpp_tb_entry *) &(priv->pann_mpp_tb->pann_mpp_pool);
		int num = 1;

		PRINT_SINGL_ARG("Portal enable: ", priv->pmib->dot1180211sInfo.mesh_portal_enable, "%u");

		if(priv->pmib->dot1180211sInfo.mesh_portal_enable)
		{			
			PRINT_ONE(num,  " %d: Mesh portal table info...", num++);
			if( priv->pmib->dot1180211sInfo.mesh_portal_enable )
			{
				PRINT_ARRAY_ARG("    PortalMAC: ",	priv->pmib->dot11OperationEntry.hwaddr, "%02x", MACADDRLEN);		
				PRINT_SINGL_ARG("    timeout: ", 99, "%u");
				PRINT_SINGL_ARG("    seqNum: ", 99, "%u");			
				PRINT_ONE("", "%s", 1);
			}
		}
		
		for(i=0;i<MAX_MPP_NUM;i++)
		{
			if(ptable[i].flag)
			{			
				PRINT_ONE(num,  " %d: Mesh portal table info...", num++);				
				PRINT_ARRAY_ARG("    PortalMAC: ",	ptable[i].mac, "%02x", MACADDRLEN);		
				PRINT_SINGL_ARG("    timeout: ", ptable[i].timeout, "%u");	// %lu=unsigned long
				PRINT_SINGL_ARG("    seqNum: ", ptable[i].seqNum, "%u");
//				PRINT_SINGL_ARG("    flag: ", ptable[i].flag, "%u");		// %lu=unsigned long
//				PRINT_SINGL_ARG("    modify_time: ", ptable[i].modify_time, "%u");
			
				PRINT_ONE("", "%s", 1);
#if 0 // stanley
				printk("portal: %02X%02X%02X%02X%02X%02X\n", *(ptable[i].mac), *(ptable[i].mac+1), *(ptable[i].mac+2), *(ptable[i].mac+3), *(ptable[i].mac+4), *(ptable[i].mac+5));
#endif
			}	
		}	
	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);

	*eof = 1;

_ret:
	
	*start = buf + (offset - begin);	// Start of wanted data

	len = strlen(buf);

	len -= (offset - begin);	// Start slop 
	if (len > length)
		len = length;	// Ending slop

	return len;
}

#ifdef MESH_USE_METRICOP
int mesh_metric_w (struct file *file, const char *buffer, unsigned long count, void *data)
{
	unsigned char local_buf[5];
	unsigned int i;

	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;

	if ( copy_from_user((void *)local_buf, (const void *)buffer, count ))
	{ return count; }

	//printk("@@XDXD OLD metricid %d\n", priv->mesh_fake_mib.metricID);

	sscanf(local_buf, "%d", &i);
	priv->mesh_fake_mib.metricID = (UINT8)i;
	//printk("@@XDXD new metricid %d\n", priv->mesh_fake_mib.metricID);

	return count;
}
int mesh_metric_r(char *buffer, char **buffer_location, off_t offset, int buffer_length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;

	int ret;
	if (offset > 0) {
		/* we have finished to read, return 0 */
		ret  = 0;
	} else {
		sprintf(buffer, "metric method=%d\n", priv->mesh_fake_mib.metricID);
		ret = strlen(buffer);
	}
	return ret;
}
#endif // MESH_USE_METRICOP

#if DBG_NCTU_MESH
int g_nctu_mesh_dbg = 0;
int mesh_setDebugLevel (struct file *file, const char *buffer, unsigned long count, void *data)
{
	unsigned char x[10];
	unsigned char local_buf[19];
	if ( copy_from_user((void *)local_buf, (const void *)buffer, count) ) {
		return count;
	}

	if(count>0)
	{
		memset(x, 0, sizeof(x));
		memcpy(x, local_buf, (count>(sizeof(x)-1))?sizeof(x)-1:count);

		int len = strlen(x);
		int i;

		g_nctu_mesh_dbg = 0;
		for(i=0;i<len;i++)
		{
			if((*(x+i)>'9') || (*(x+i)<'0'))
				continue;
			g_nctu_mesh_dbg = g_nctu_mesh_dbg*10 + (*(x+i)-'0');
		}
	}
	printk("@@@ debug level: %d\n", g_nctu_mesh_dbg);

	return count;
}

/*
	0: br_input: all packets
	1: icmp module
	2: arp module
	3: eth0 - icmp, arp
	4: tx: icmp, arp
	5: rx: icmp, arp
	8: fake hangup (by using beacon_hangup)
	9: tx trace
	10: tx misc error
*/

int mesh_hasDebugLevel (int lv) {
	return (g_nctu_mesh_dbg & (1<<lv));
}
int mesh_clearDebugLevel (int lv) {
	g_nctu_mesh_dbg ^= (1<<lv);
	return g_nctu_mesh_dbg;
}
EXPORT_SYMBOL(hasMeshDebugLevel);


static atomic_t allskbs[20];
static unsigned char tagmag[2] = {0xA5, 0x5A};
static unsigned char tagmag2[2] = {0x46, 0x6d};

int mesh_incMySkb(UINT8 type)
{
	if(type>=(sizeof(allskbs)/sizeof(atomic_t)))
		return 0;
	atomic_inc(&allskbs[type]);
	return atomic_read(&allskbs[type]);
}
int mesh_isMySkb(struct sk_buff *skb)
{
	UINT8 type;
	if(memcmp(&(skb->cb[15]), tagmag, sizeof(tagmag)))
		return -1;
	if(memcmp(&(skb->cb[15+sizeof(tagmag)+sizeof(UINT8)]), tagmag2, sizeof(tagmag2)))
		return -1;
	type = skb->cb[15+sizeof(tagmag)];
	if(type>=(sizeof(allskbs)/sizeof(atomic_t)))
		return -1;
	return (int)type;
}

int mesh_decMySkb(struct sk_buff *skb)
{
	UINT8 type;
	if(memcmp(&(skb->cb[15]), tagmag, sizeof(tagmag)))
		return 0;
	if(memcmp(&(skb->cb[15+sizeof(tagmag)+sizeof(UINT8)]), tagmag2, sizeof(tagmag2)))
		return 0;

	memset(&(skb->cb[15]), 0, sizeof(tagmag));
	memset(&(skb->cb[15+sizeof(tagmag)+sizeof(UINT8)]), 0, sizeof(tagmag2));

	type = skb->cb[15+sizeof(tagmag)];
	if(type>=(sizeof(allskbs)/sizeof(atomic_t)))
		return 0;
	atomic_dec(&allskbs[type]);
	return atomic_read(&allskbs[type]);
}

int mesh_tagMySkb(struct sk_buff *skb, UINT8 type)
{
	// if(isMySkb(skb))
		mesh_decMySkb(skb);
	memcpy(&(skb->cb[15]), tagmag, sizeof(tagmag));
	memcpy(&(skb->cb[15+sizeof(tagmag)+sizeof(UINT8)]), tagmag2, sizeof(tagmag2));

	skb->cb[15+sizeof(tagmag)] = type;
	return mesh_incMySkb(type);
}

int mesh_showSpecificSkbs(UINT8 type)
{
	return atomic_read(&allskbs[type]);
}

// EXPORT_SYMBOL(tagMySkb);
EXPORT_SYMBOL(mesh_incMySkb);
EXPORT_SYMBOL(mesh_decMySkb);
EXPORT_SYMBOL(mesh_isMySkb);
int mesh_showAllSkbs(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	int i;
	for (i=0;i<(sizeof(allskbs)/sizeof(int));i++)
	{
		printk("Type %d: count=%d   ", i, atomic_read(&allskbs[i]));
		if(i && ((i%3) == 0)) printk("\n");
	}

	return 0;
}

#endif // DBG_NCTU_MESH

#ifdef _MESH_DEBUG_
/*
 *	@brief	Ckear ALL stat_info (debug only)
 *		
 *	@param	Unknow
 *
 *	@retval	int: pos:Unknow
 */
static int mesh_proc_clear_table(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	struct stat_info	*pstat;
	int pos = 0;

	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		
#ifdef MESH_BOOTSEQ_AUTH
		while (!list_empty(&priv->mesh_auth_hdr)) {
			pstat = list_entry(priv->mesh_auth_hdr.next, struct stat_info, mesh_mp_ptr);
			free_stainfo(priv, pstat);
		}
#endif

		while (!list_empty(&priv->mesh_unEstablish_hdr)) {
			pstat = list_entry(priv->mesh_unEstablish_hdr.next, struct stat_info, mesh_mp_ptr); 
			free_stainfo(priv, pstat);
		}

		while (!list_empty(&priv->mesh_mp_hdr)) {
			pstat = list_entry(priv->mesh_mp_hdr.next, struct stat_info, mesh_mp_ptr); 
			free_stainfo(priv, pstat);
		}
		
		mesh_set_PeerLink_CAP(priv, GET_MIB(priv)->dot1180211sInfo.mesh_max_neightbor);	// Reset connection number

		PRINT_ONE("  All table clear ok...", "%s", 1);
	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);
	
	return pos;
}


int mesh_ProcMACParser(const char *buffer, unsigned long *count)
{
	char local_buf[19];
	char *ptr;
	UINT8 mac[MACADDRLEN];
	
	int idx = 0;

	memset(local_buf, 0, sizeof(local_buf));
			
	if (*count < 11 ) { // a:b:c:d:e:f
		return -EFAULT;
	}
	
	if (*count > 18 ) {
		*count = 18;
	}
	
	/* write data to the buffer */
	if ( copy_from_user((void *)local_buf, (const void *)buffer, *count) ) {
		printk("*** mesh_test_sme_proc_write: copy from user error\r\n");
		return -EFAULT;
	}
	
	memset(mac, 0, sizeof(mac));
	ptr = local_buf;
	while(*ptr)
	{
		unsigned char val = 0;
		
		if(idx>5)
			break;
			
		if(*ptr == ':')
		{
			ptr++;
			idx++;
			continue;
		}
		if( (*ptr>='0') && (*ptr<='9') )
		{
			val = *ptr - '0';
		}
		else if( (*ptr>='A') && (*ptr<='F') )
		{
			val = *ptr - 'A' + 10;
		}
		else if( (*ptr>='a') && (*ptr<='f') )
		{
			val = *ptr - 'a' + 10;
		}
		else
		{
			ptr++;
			continue;
		}
		mac[idx] = mac[idx]*16+val;

		ptr++;
	}
	memcpy(mesh_proc_MAC, mac, MACADDRLEN);
	
	return SUCCESS;

}


static int mesh_setMACAddr(struct file *file, const char *buffer, unsigned long count, void *data)
{
	mesh_ProcMACParser(buffer, &count);
	
	MESH_DEBUG_MSG("Set MAC Address %02X:%02X:%02X:%02X:%02X:%02X is OK !!\n",
			mesh_proc_MAC[0], mesh_proc_MAC[1], mesh_proc_MAC[2], mesh_proc_MAC[3], mesh_proc_MAC[4], mesh_proc_MAC[5]);

	return count;
}

#ifdef MESH_BOOTSEQ_AUTH
static int mesh_proc_issueAuthReq(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	struct stat_info	*pstat;

	int pos = 0;

	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		pstat = get_stainfo(priv, mesh_proc_MAC);
		if (NULL != pstat) {
			pstat->auth_seq = 1;
			issue_auth(priv, pstat, _STATS_SUCCESSFUL_);
			PRINT_ONE(" Sent Auth Request OK....", "%s", 1);
		} else {
			PRINT_ONE("  MAC not exist !!", "%s", 1);
		}
	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);

	return pos;
}

static int mesh_proc_issueAuthRsp(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	struct stat_info	*pstat;

	int pos = 0;

	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		pstat = get_stainfo(priv, mesh_proc_MAC);
		if (NULL != pstat) {
			pstat->auth_seq = 2;
			issue_auth(priv, pstat, _STATS_SUCCESSFUL_);
			PRINT_ONE(" Sent Auth Response OK....", "%s", 1);
		} else {
			PRINT_ONE("  MAC not exist !!", "%s", 1);
		}
	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);
	
	return pos;
}


static int mesh_proc_issueDeAuth(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	struct stat_info	*pstat;
	
	int pos = 0;
	
	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		pstat = get_stainfo(priv, mesh_proc_MAC);
		
		if (NULL != pstat) {
			issue_deauth_MP(priv, mesh_proc_MAC, _RSON_CLS2_, TRUE);
			PRINT_ONE(" Sent DeAuth OK....", "%s", 1);
		} else
			PRINT_ONE("  MAC not exist !!", "%s", 1);

	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);
	
	return pos;
}
#endif

static int mesh_proc_openConnect(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	struct stat_info	*pstat;

	int pos = 0;
	
	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		struct rx_frinfo	pfrinfo;	//Note: It isn't pointer !!
		
		PRINT_ARRAY_ARG("I will Active peer link MAC:", mesh_proc_MAC, "%02x", MACADDRLEN);
		pfrinfo.sa = mesh_proc_MAC;
		pfrinfo.rssi = priv->mesh_fake_mib.establish_rssi_threshold + 1;	// +1: Ensure connect
		start_MeshPeerLink(priv, &pfrinfo, NULL, 1, priv->pmib->dot11RFEntry.dot11channel);
		pstat = get_stainfo(priv, mesh_proc_MAC);
		
		if (NULL != pstat)
			PRINT_SINGL_ARG("Start active PeerLink: LocalLinkID=", pstat->mesh_neighbor_TBL.LocalLinkID, "%lu");

	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);

	return pos;
}

static int mesh_proc_issueOpen(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	struct stat_info	*pstat;

	int pos = 0;
	
	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		pstat = get_stainfo(priv, mesh_proc_MAC);
		if (NULL != pstat) {
			PRINT_ARRAY_ARG("I will issue Open frame MAC:", mesh_proc_MAC, "%02x", MACADDRLEN);
			issue_assocreq_MP(priv, pstat);
			PRINT_SINGL_ARG("Issue Open: LocalLinkID=", pstat->mesh_neighbor_TBL.LocalLinkID, "%lu");
			PRINT_SINGL_ARG("Issue Open: PeerLinkID=", pstat->mesh_neighbor_TBL.PeerLinkID, "%lu");
		}
	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);

	return pos;
}

static int mesh_proc_issueConfirm(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	struct stat_info	*pstat;

	int pos = 0;

	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		pstat = get_stainfo(priv, mesh_proc_MAC);
		if (NULL != pstat) {
			PRINT_ARRAY_ARG("I will issue Confirm frame MAC:", mesh_proc_MAC, "%02x", MACADDRLEN);
			issue_assocrsp_MP(priv, 0, pstat, WIFI_ASSOCRSP);
			PRINT_SINGL_ARG("Issue Confirm: LocalLinkID=", pstat->mesh_neighbor_TBL.LocalLinkID, "%lu");
			PRINT_SINGL_ARG("Issue Confirm: PeerLinkID=", pstat->mesh_neighbor_TBL.PeerLinkID, "%lu");
		}
	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);

	return pos;
}

static int mesh_proc_issueClose(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	struct stat_info	*pstat;

	int pos = 0;

	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		pstat = get_stainfo(priv, mesh_proc_MAC);
		if (NULL != pstat) {
			PRINT_ARRAY_ARG("I will issue Close frame MAC:", mesh_proc_MAC, "%02x", MACADDRLEN);
			issue_disassoc_MP(priv, pstat, 0, 0);
			PRINT_SINGL_ARG("Issue Close: LocalLinkID=", pstat->mesh_neighbor_TBL.LocalLinkID, "%lu");
			PRINT_SINGL_ARG("Issue Close: PeerLinkID=", pstat->mesh_neighbor_TBL.PeerLinkID, "%lu");
		}
	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);

	return pos;
}

static int mesh_proc_closeConnect(char *buf, char **start, off_t offset, int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;

	int pos = 0;

	if (1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) {
		PRINT_ARRAY_ARG("I will Close peer link MAC:", mesh_proc_MAC, "%02x", MACADDRLEN);
		close_MeshPeerLink(priv, mesh_proc_MAC);
	} else
		PRINT_ONE("  Mesh mode DISABLE !!", "%s", 1);

	return pos;
}

#endif //_MESH_DEBUG_

#endif //  CONFIG_RTK_MESH
