/*
 *      Handling routines for Mesh in 802.11 RX
 * 
 *      PS: All extern function in ../8190n_headers.h
 */
#define _MESH_RX_C_

#ifdef CONFIG_RTL8192CD
#include "../rtl8192cd/8192cd.h"
#include "../rtl8192cd/8192cd_headers.h"
#else
#include "../rtl8190/8190n.h"
#include "../rtl8190/8190n_headers.h"
#endif
#include "./mesh.h"
#include "./mesh_route.h"

#ifdef CONFIG_RTK_MESH


int pid_pathsel = 0;

#ifdef _11s_TEST_MODE_
int pid_receiver = 0;
#endif


/*
	Check the "to_fr_ds" field:

						FromDS =1
						ToDS =1
	NOTE: The functuion duplicate from rtl8190_rx_dispatch_wds (8190n_rx.c)
*/
int rx_dispatch_mesh(DRV_PRIV *priv, struct rx_frinfo *pfrinfo)
{
	int reuse = 1;
	unsigned char *pframe = get_pframe(pfrinfo);
	unsigned int frtype = GetFrameType(pframe);
	unsigned short frame_type = GetFrameSubType(pframe);
	
	struct net_device *dev = priv->mesh_dev;
	unsigned char	*da = pfrinfo->da;
	int fragnum;

	if (memcmp(GET_MY_HWADDR, GetAddr1Ptr(pframe), MACADDRLEN)
		&& !IS_MCAST(GetAddr1Ptr(pframe))	)
	{
		DEBUG_INFO("Rx a 11s packet but which addr1 is not matched own, drop it!\n");
		goto out;
	}

	if (!netif_running(dev)) {
		DEBUG_ERR("Rx a 11s packet but which interface is not up, drop it!\n");
		goto out;
	}
	
	fragnum = GetFragNum(pframe);
	if (GetMFrag(pframe) || fragnum) {
		DEBUG_ERR("Rx a fragment 11s packet, drop it!\n");
		goto out;
	}

	if ( (frtype != WIFI_DATA_TYPE) && (frtype != WIFI_EXT_TYPE)) {
		DEBUG_ERR("Rx a Mesh packet but which type is not valid, drop it!\n");
		goto out;
	}
	
	// Unsupported (8186): 802.11s action frame, data frame --> type = 11 (i.e., WIFI_EXT_TYPE)
	// Hence, we had to use WIFI_DATA_TYPE in 8186, 8187B and now 8190.
	if (frtype == WIFI_DATA_TYPE) {	
		struct stat_info	*pstat;
		unsigned long		flags;
						
		SAVE_INT_AND_CLI(flags);
		pstat = get_stainfo(priv, GetAddr2Ptr(pframe));
		
		// action which has lowest state is "Link state announcement"
		if((pstat == NULL) 
				|| !((pstat->mesh_neighbor_TBL.State == MP_SUPERORDINATE_LINK_UP) || (pstat->mesh_neighbor_TBL.State == MP_SUBORDINATE_LINK_UP)
				|| (pstat->mesh_neighbor_TBL.State == MP_SUBORDINATE_LINK_DOWN_E))) // Recived Local Link State Announcement from  DOWN to UP
		{
		
			RESTORE_INT(flags);
			if( !IS_MCAST(da) && frame_type == WIFI_11S_MESH && pstat == NULL)
				issue_deauth_MP(priv, (void *)GetAddr2Ptr(pframe), _RSON_UNSPECIFIED_, 1);
//
			DEBUG_ERR("Rx a Mesh packet but there does not have a corresponding station info\n");
			goto out;
		}
		RESTORE_INT(flags);
		pfrinfo->is_11s = 1;
		
#ifndef MESH_BOOTSEQ_STRESS_TEST	// Stop update expire when stress test.
		// chris: update neighbor timer
		pstat->mesh_neighbor_TBL.expire = jiffies + MESH_EXPIRE_TO;
#endif
	} 
					
	// if( frame_type == WIFI_ACTION ) { // In D1.06 next line replace by this line (And remove SetFrameSubType(pframe, WIFI_ACTION);) by popen
	if( frame_type == WIFI_11S_MESH_ACTION ) 
	{
		int TTL;
		unsigned short seq;
		//struct path_sel_entry *pPathselEntry;
						
		seq = *((unsigned short*)(pframe+32));
		TTL = *GetMeshHeaderTTLWithOutQOS(pframe);
					
		if(TTL == 0)	// filter packets due to duplicate
			goto out;
						
		*GetMeshHeaderTTLWithOutQOS(pframe) = TTL-1;

		/* Totti 2008.07.30 test Pause filter.
		if(IS_MCAST(da) && !chkMeshSeq(priv, GetAddr4Ptr(pframe),seq))	// filter packets due to duplicate
			 goto out;
		*/
	
		//pPathselEntry = priv->pathsel_table->search_entry(priv->pathsel_table, sa);

// Gallardo test 2008.0901	
//		if (NULL != pPathselEntry)
//			pPathselEntry->update_time = xtime;
						
		SetFrameSubType(pframe, WIFI_WMM_ACTION); // it's easier for mgt_handler

		// IF Draft modify Action frame to Date frame, Remove line between here to goto out ?
#ifdef RTL8190_DIRECT_RX
		DRV_RX_MGNTFRAME(priv, NULL, pfrinfo);
#else
		list_add_tail(&(pfrinfo->rx_list), &(priv->rx_mgtlist));
#endif
		reuse = 0;
		goto	out;	// Success finish, reuse = 0 (Because not Date frame) (Remove to here)
	}

	else if( frame_type == WIFI_11S_MESH ) 
	{
	
		memcpy(pfrinfo->prehop_11s, GetAddr2Ptr(pframe), MACADDRLEN);
		
/*	
		struct path_sel_entry *pPathselEntry;
		
#ifdef MESH_AMSDU
		if(((*GetQosControl(pframe))& BIT(7))==0)
#endif
		{			
			unsigned short seq=0;
			int TTL=0;				

			unsigned char *meshHdrPtr; 
			meshHdrPtr = getMeshHeader(priv, priv->pmib->dot11sKeysTable.dot11Privacy, pframe);	   	
			if( meshHdrPtr ) 
			{
				seq = *((unsigned short*)(meshHdrPtr+2));
				TTL = *((unsigned char*) (meshHdrPtr+1));
				if(TTL == 0)	// filter packets due to duplicate
					goto out;
				
				*(meshHdrPtr+1)=TTL-1;
				memcpy(&(pfrinfo->mesh_header), meshHdrPtr, sizeof(struct lls_mesh_header) );
			}		
			// Usually, IS_MCAST(A3) = IS_MCAST(A1) for a multicast frame
			// However, when using unicast to simulate multicast, A3 is unicast but A1 is not (it is not implemented now)
			// On the other hand, if a maliciuos host issues multicast for A1 but unicast for A3, a flooding would happen if not chcking A1
			if((IS_MCAST(da)||IS_MCAST(GetAddr1Ptr(pframe))) && !chkMeshSeq(priv, GetAddr4Ptr(pframe),seq))	// filter packets due to duplicate
				goto out;
		}
	
		pPathselEntry = priv->pathsel_table->search_entry(priv->pathsel_table, sa);	
		if (NULL != pPathselEntry)
			pPathselEntry->update_time = xtime;							
*/		
	} 
	else { 
		DEBUG_ERR("Rx a 11s packet but which type is not DATA, drop it!\n");
		goto out;
	} 

#ifdef RTL8190_DIRECT_RX
	DRV_RX_DATA(priv, NULL, pfrinfo);
#else
	list_add_tail(&pfrinfo->rx_list, &priv->rx_datalist);
#endif
	reuse = 0;
out:
	return reuse;
}



#ifdef	_11s_TEST_MODE_
void GEN_TEST_PACKET_ACK(struct rtl8190_priv *priv, int n)
{
	unsigned char data[20];
	struct task_struct *p;

	memset(data, 0, 20);	
	sprintf(data, "%d", n);
	DOT11_EnQueue2((unsigned long)priv, priv->receiver_queue, data, strlen(data)+1 );
                
    if(pid_receiver != 0){
        read_lock(&tasklist_lock); 
        p = find_task_by_pid(pid_receiver);
        read_unlock(&tasklist_lock);
        if(p)
                send_sig(SIGUSR2, p, 0); 	
        else
                pid_receiver = 0;        
    }
}
#endif

#ifdef MESH_AMSDU
static void process_11s_amsdu(struct rtl8190_priv *priv, struct stat_info *pstat, struct rx_frinfo *pfrinfo)
{
	unsigned char	*pframe;
	struct sk_buff	*pskb  = NULL, *pnewskb = NULL;
	unsigned char	*next_head;
	int 			rest, agg_pkt_num=0, i, privacy;
	unsigned int	subfr_len, padding;
	unsigned const char rfc1042_ip_header[8]={0xaa,0xaa,0x03,00,00,00,0x08,0x00};

	pframe = get_pframe(pfrinfo);
	pskb = get_pskb(pfrinfo);

	rest = pfrinfo->pktlen - pfrinfo->hdr_len;
	next_head = pframe + pfrinfo->hdr_len;

	if (GetPrivacy(pframe)) {
		privacy = get_sta_encrypt_algthm(priv, pstat);
		if ((privacy == _CCMP_PRIVACY_) || (privacy == _TKIP_PRIVACY_)) {
			rest -= 8;
			next_head += 8;
		}
		else {	// WEP
			rest -= 4;
			next_head += 4;
		}
	}
	while (rest > WLAN_ETHHDR_LEN) {
		pnewskb = skb_clone(pskb, GFP_ATOMIC);
		if (pnewskb) {
			pnewskb->data = next_head;		
			subfr_len = (*(next_head + MACADDRLEN*2) << 8) + (*(next_head + MACADDRLEN*2 + 1));
			pnewskb->len = WLAN_ETHHDR_LEN + subfr_len;
			pnewskb->tail = pnewskb->data + pnewskb->len;

			if (!memcmp(rfc1042_ip_header, pnewskb->data+WLAN_ETHHDR_LEN, 8)) {
				for (i=0; i<MACADDRLEN*2; i++)
					pnewskb->data[19-i] = pnewskb->data[11-i];
				pnewskb->data += 8;
				pnewskb->len -= 8;
			}
			else
				strip_amsdu_llc(priv, pnewskb, pstat);
			agg_pkt_num++;

			
			{
				struct rx_frinfo newfrinfo;
				struct	MESH_HDR* mhdr = (struct MESH_HDR*) (pnewskb->data+sizeof(struct wlan_ethhdr_t));
				memset(&newfrinfo, 0, sizeof(newfrinfo));
				newfrinfo.is_11s = 8;
				newfrinfo.to_fr_ds = 3;
				newfrinfo.pktlen = pnewskb->len;
				newfrinfo.da = (pnewskb->data);
				newfrinfo.sa = (pnewskb->data+MACADDRLEN);
				newfrinfo.pskb = pnewskb;
				if( mhdr->TTL-- ==0 || ( IS_MCAST(newfrinfo.da) && !chkMeshSeq(priv, newfrinfo.sa, mhdr->segNum)))
					rtl_kfree_skb(priv, pnewskb, _SKB_RX_);
				else
				{
					struct path_sel_entry *pPathselEntry;		
					pPathselEntry = priv->pathsel_table->search_entry(priv->pathsel_table, newfrinfo.sa); 
					if (NULL != pPathselEntry)
						pPathselEntry->update_time = xtime; 					
					memcpy(&(newfrinfo.mesh_header), mhdr, sizeof(struct MESH_HDR));
					if (process_11s_datafrme(priv, &newfrinfo, pstat) == FAIL) 
						rtl_kfree_skb(priv, pnewskb, _SKB_RX_);
				}				
			}

			padding = 4 - ((WLAN_ETHHDR_LEN + subfr_len) % 4);
			if (padding == 4)
				padding = 0;
			rest -= (WLAN_ETHHDR_LEN + subfr_len + padding);
			next_head += (WLAN_ETHHDR_LEN + subfr_len + padding);
		}
		else {
			// Can't get new skb header, drop this packet
			break;
		}
	}


	// clear saved shortcut data
#if defined(RX_SHORTCUT) && defined(RX_RL_SHORTCUT)
	if (pstat->rx_payload_offset)
		pstat->rx_payload_offset = 0;
#endif


	rtl_kfree_skb(priv, pskb, _SKB_RX_);
}
#endif


#ifdef RX_RL_SHORTCUT
void mesh_shortcut_update(DRV_PRIV *priv, struct rx_frinfo *pfrinfo)
{
/*
	unsigned char *pframe = get_pframe(pfrinfo);
	struct stat_info *pstat = get_stainfo(priv, GetAddr2Ptr(pframe));
	
	if (memcmp(GetAddr4Ptr(pframe), pstat->rx_wlanhdr.wlanhdr.addr4, MACADDRLEN)){
		return -1;
	}
	if (memcmp(GetAddr3Ptr(pframe), GET_MY_HWADDR, MACADDRLEN)) {
		
		pEntry = priv->pathsel_table->search_entry(priv->pathsel_table, GetAddr3Ptr(pframe));
		if (pEntry) {	
			//printk("pEntry -> nh %02x \n", pEntry->nexthopMAC[5]);
		}
		else
		return -1;
		
	}
	
	
	meshHdrPtr = getMeshHeader(priv, priv->pmib->dot11sKeysTable.dot11Privacy, pframe); 
	if( meshHdrPtr ) 
	{
		if((int)(*(meshHdrPtr+1)) == 0) // *(meshHdrPtr+1)) = TTL; filter packets due to duplicate 
			return -1;
		
		
		memcpy(&(pfrinfo->mesh_header), meshHdrPtr, sizeof(struct lls_mesh_header) );

	} 
	else {
		return -1;
	}
	
	
	if (( pfrinfo->mesh_header.mesh_flag)&1) //only shorcut 6 address
	{
		if (memcmp(pfrinfo->mesh_header.DestMACAddr, pstat->rx_wlanhdr.wlanhdr.meshhdr.DestMACAddr, 6) ||
			memcmp(pfrinfo->mesh_header.SrcMACAddr, pstat->rx_wlanhdr.wlanhdr.meshhdr.SrcMACAddr, 6)) {
				return -1;
			}
	} else {
		return -1;
	}
*/	
	struct path_sel_entry *pPathselEntry;
	struct proxy_table_entry	*pProxyEntry ;

	//6 addresss format, update pathsel and proxy timer 
	pPathselEntry = priv->pathsel_table->search_entry(priv->pathsel_table, pfrinfo->sa); 
	if (pPathselEntry != NULL)
		pPathselEntry->update_time = xtime; 
	
	pProxyEntry = (struct proxy_table_entry*) HASH_SEARCH(priv->proxy_table, pfrinfo->mesh_header.SrcMACAddr);
	if (pProxyEntry != NULL)
		pProxyEntry->update_time = xtime;
	
	pProxyEntry = (struct proxy_table_entry*) HASH_SEARCH(priv->proxy_table, pfrinfo->mesh_header.DestMACAddr); 
	if (pProxyEntry != NULL) 
		pProxyEntry->update_time = xtime;
	
	//return 1;

}

#endif

// Gakki
/*
	pfrinfo->is_11s =1  => 802.11 header
	pfrinfo->is_11s =8  => 802.3  header + mesh header
*/
int process_11s_datafrme(DRV_PRIV *priv, struct rx_frinfo *pfrinfo, struct stat_info *pstat)
{
	unsigned char 	*pframe;
	unsigned int  	privacy, res;
	struct sk_buff	 *pskb  = NULL, *pnewskb = NULL;
	struct stat_info *dapstat = NULL;

	if(pstat == NULL && !IS_MCAST(pfrinfo->da) )
		return FAIL;

	pframe = get_pframe(pfrinfo);
	pskb = get_pskb(pfrinfo);
	pskb->dev = priv->mesh_dev;

/*
#if defined(MESH_AMSDU) || defined(_11s_TEST_MODE_)
	memcpy(pfrinfo->prehop_11s, pstat->hwaddr, MACADDRLEN);
#endif
*/
	// Process A-MSDU
	if (pfrinfo->is_11s&1)	{
		if((*GetQosControl(pframe))& BIT(7)) {
#ifdef MESH_AMSDU
			process_11s_amsdu(priv, pstat, pfrinfo);
			return SUCCESS;
#else
		return FAIL;
#endif
		} else 	{
			struct path_sel_entry *pPathselEntry;
			unsigned short seq=0;
			int TTL=0;
			unsigned char *meshHdrPtr; 
			
			meshHdrPtr = getMeshHeader(priv, priv->pmib->dot11sKeysTable.dot11Privacy, pframe); 	
			if( meshHdrPtr ) 
			{
				seq = *((unsigned short*)(meshHdrPtr+2));
				TTL = *((unsigned char*) (meshHdrPtr+1));
				if(TTL == 0)	// filter packets due to duplicate
					return FAIL;
				
				*(meshHdrPtr+1)=TTL-1;
				memcpy(&(pfrinfo->mesh_header), meshHdrPtr, sizeof(struct lls_mesh_header) );
			}		
			// Usually, IS_MCAST(A3) = IS_MCAST(A1) for a multicast frame
			// However, when using unicast to simulate multicast, A3 is unicast but A1 is not (it is not implemented now)
			// On the other hand, if a maliciuos host issues multicast for A1 but unicast for A3, a flooding would happen if not chcking A1
			if((IS_MCAST(pfrinfo->da)||IS_MCAST(GetAddr1Ptr(pframe))) && !chkMeshSeq(priv, GetAddr4Ptr(pframe),seq)) // filter packets due to duplicate
				return FAIL;
		
			pPathselEntry = priv->pathsel_table->search_entry(priv->pathsel_table, pfrinfo->sa); 
			if (NULL != pPathselEntry)
				pPathselEntry->update_time = xtime; 
		}
	}

#ifdef	_11s_TEST_MODE_
	if( memcmp(pframe+32+6, "TEST_TRAFFIC", 12)==0 )
	{
		int x2=0;
		pframe += 32+6+12;
		while (*++pframe == 0x20);
		while (*pframe != 0x2a) 
			x2 = 10 * x2 + (*pframe++- '0');

		if( x2<=3000 && x2>0 && (++priv->rvTestPacket[--x2]==1) )
			priv->mesh_stats.rx_packets++;
		++priv->mesh_stats.rx_errors;

		if(  memcmp("JasonSender", priv->pmib->dot1180211sInfo.mesh_reservedstr1, 11)==0 )
			goto dropPkt;
		else if( memcmp("JasonRver",   priv->pmib->dot1180211sInfo.mesh_reservedstr1, 9)==0 )
		{
			GEN_TEST_PACKET_ACK(priv, x2);
dropPkt:			
			return FAIL;
		}
	}
#endif

	privacy = IS_MCAST(pfrinfo->da) ? _NO_PRIVACY_ : get_sta_encrypt_algthm(priv, pstat);

	if(IS_MCAST(pfrinfo->da))
	{
		unsigned char *addr6 = pfrinfo->mesh_header.SrcMACAddr;
		//try to learn proxied PC from multicast frame, brian 2011-10-07
		if(pfrinfo->mesh_header.mesh_flag&1)
		{
			if(memcmp(pfrinfo->sa, addr6, MACADDRLEN)) { // addr4 <> addr 6, save it to proxy table
#ifdef PU_STANDARD // for this mode, only "refresh" record
				struct proxy_table_entry*	pProxyEntry = NULL; 		
				pProxyEntry = (struct proxy_table_entry*) HASH_SEARCH(priv->proxy_table, addr6);	
				if(pProxyEntry && ( memcmp(pProxyEntry->owner, pfrinfo->sa, MACADDRLEN)==0 ) )
					pProxyEntry->update_time = xtime;
#ifdef PU_A4A6   // for this mode, "update" record
				if( pProxyEntry == NULL) {
					struct proxy_table_entry Entry;							
					memcpy(Entry.sta, addr6, MACADDRLEN);
					memcpy(Entry.owner, pfrinfo->sa, MACADDRLEN);
					Entry.update_time = xtime;
					HASH_INSERT(priv->proxy_table, Entry.sta, &Entry);
					proxy_debug("%s %d Insert Proxy table: %02x:%02x:%02x:%02x:%02x:%02x/%02x:%02x:%02x:%02x:%02x:%02x\n",
						__FUNCTION__,__LINE__,Entry.owner[0],Entry.owner[1],Entry.owner[2],Entry.owner[3],Entry.owner[4],Entry.owner[5],
						Entry.sta[0],Entry.sta[1],Entry.sta[2],Entry.sta[3],Entry.sta[4],Entry.sta[5]);
				}
#endif
#endif
			}
		}
	
		if (!priv->pmib->dot11OperationEntry.block_relay)
		{
			// copy the content of pskb into a new memory (pnewskb)
			pnewskb = skb_copy(pskb, GFP_ATOMIC);
			if (pnewskb) {
	#if 1
				struct rx_frinfo newfrinfo;
			
				memset(&newfrinfo, 0, sizeof(newfrinfo));
				newfrinfo.hdr_len = get_hdrlen(priv, pnewskb->data);
				newfrinfo.da = get_da(pnewskb->data);
				newfrinfo.sa = get_sa(pnewskb->data);

				newfrinfo.is_11s = pfrinfo->is_11s;
				newfrinfo.to_fr_ds = 3;
				newfrinfo.pktlen = pnewskb->len;
				newfrinfo.pskb = pnewskb;
				if (is_qos_data(pnewskb->data))
					newfrinfo.tid = GetTid(pnewskb->data);	
				memcpy(&(newfrinfo.mesh_header), &(pfrinfo->mesh_header), sizeof(struct  MESH_HDR));
	#endif					

				if (relay_11s_dataframe(pnewskb, /*pskb->dev,*/ privacy, &newfrinfo))
				{
					rtl_kfree_skb(priv, pnewskb, _SKB_TX_);
				}
			}
		}
		goto receive_packet;
	} 
	else			
	{

		int Is_6AddrFormat = ( pfrinfo->mesh_header.mesh_flag)&1;
		unsigned char *addr5 = pfrinfo->mesh_header.DestMACAddr;
		unsigned char *addr6 = pfrinfo->mesh_header.SrcMACAddr;
				
		if(Is_6AddrFormat)
		{ 
			//if frame is 6 addresss format, update proxy table	
			if(memcmp(pfrinfo->sa, addr6, MACADDRLEN)) { // addr4 <> addr 6, save it to proxy table
#ifdef PU_A4A6   // for this mode, "update" record
				struct proxy_table_entry	Entry;							
				memcpy(Entry.sta, addr6, MACADDRLEN);
				memcpy(Entry.owner, pfrinfo->sa, MACADDRLEN);
				Entry.update_time = xtime;
				HASH_INSERT(priv->proxy_table, Entry.sta, &Entry);
				proxy_debug("%s %d Insert Proxy table: %02x:%02x:%02x:%02x:%02x:%02x/%02x:%02x:%02x:%02x:%02x:%02x\n",
						__FUNCTION__,__LINE__,Entry.owner[0],Entry.owner[1],Entry.owner[2],Entry.owner[3],Entry.owner[4],Entry.owner[5],
						Entry.sta[0],Entry.sta[1],Entry.sta[2],Entry.sta[3],Entry.sta[4],Entry.sta[5]);
#else
#ifdef PU_STANDARD // for this mode, only "refresh" record
				struct proxy_table_entry*	pProxyEntry = NULL;			
				pProxyEntry = (struct proxy_table_entry*) HASH_SEARCH(priv->proxy_table, addr6);	
				if(pProxyEntry && ( memcmp(pProxyEntry->owner, pfrinfo->sa, MACADDRLEN)==0 ) )
					pProxyEntry->update_time = xtime;
#endif
#endif
			}
		}

		if( memcmp(pfrinfo->da, GET_MY_HWADDR, MACADDRLEN)== 0)
		{
			// dest is me, and I am root
			if( memcmp(priv->root_mac, GET_MY_HWADDR, MACADDRLEN) == 0 ) 
			{
				struct proxy_table_entry*	pProxyEntry = NULL;			
				struct path_sel_entry *pEntry = NULL;
				
				pProxyEntry = (struct proxy_table_entry*) HASH_SEARCH(priv->proxy_table, addr5);
				if(pProxyEntry != NULL) // addr5 is a proxy sta
				{
					pProxyEntry->update_time = xtime;
					memcpy(pfrinfo->da, pProxyEntry->owner, MACADDRLEN);
					if(memcmp(pProxyEntry->owner, GET_MY_HWADDR, MACADDRLEN)== 0) 
					{
						if (0 != memcmp(pfrinfo->sa, GET_MY_HWADDR, MACADDRLEN)) 
						{
#if 0
							LOG_MESH_MSG("root bridge pkt to LAN:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X (source: %02X:%02X:%02X:%02X:%02X:%02X)\n", 
								addr5[0], addr5[1], addr5[2], addr5[3], addr5[4], addr5[5], 
								addr6[0], addr6[1], addr6[2], addr6[3], addr6[4], addr6[5]);
#endif								
							goto receive_packet;
						} 
						else 
						 	// I am the owner of dest. However, I am also the sender of the packet.
							// e.g., still not knowing the owner(myself)-dest, so I issued the packet
							//rtl_kfree_skb(priv, pskb, _SKB_RX_);
							return FAIL;
						
					}
					else 
					// stanley
					{
#if 0
						printk("root relay pkt to dev:%02X:%02X:%02X:%02X:%02X:%02X (source:%02X:%02X:%02X:%02X:%02X:%02X)\n", 
							addr5[0], addr5[1], addr5[2], addr5[3], addr5[4], addr5[5], 
							addr6[0], addr6[1], addr6[2], addr6[3], addr6[4], addr6[5]);
#endif
						if(relay_11s_dataframe(pskb, /*pskb->dev,*/ privacy, pfrinfo)) 
						{
#if 0
							LOG_MESH_MSG("root relay pkt to dev:%02X:%02X:%02X:%02X:%02X:%02X (source:%02X:%02X:%02X:%02X:%02X:%02X)\n", 
								addr5[0], addr5[1], addr5[2], addr5[3], addr5[4], addr5[5], 
								addr6[0], addr6[1], addr6[2], addr6[3], addr6[4], addr6[5]);
#endif								
							rtl_kfree_skb(priv, pskb, _SKB_TX_);
						}
					}
				} 
				else 	// for pProxyEntry == 0
				{ 
					pEntry = pathsel_query_table( priv, addr5 );
					if(pEntry != (struct path_sel_entry *)-1) // add5 is an MP
					{ 
						memcpy(pfrinfo->da, addr5, MACADDRLEN);					
#if 0 // stanley
						printk("root relay pkt to MP:%02X:%02X:%02X:%02X:%02X:%02X (source:%02X:%02X:%02X:%02X:%02X:%02X)\n", 
							addr5[0], addr5[1], addr5[2], addr5[3], addr5[4], addr5[5], 
							addr6[0], addr6[1], addr6[2], addr6[3], addr6[4], addr6[5]);
#endif
						if(relay_11s_dataframe(pskb, /*pskb->dev,*/ privacy, pfrinfo))
							rtl_kfree_skb(priv, pskb, _SKB_TX_);					
#if 0
						LOG_MESH_MSG("root relay pkt to MP:%02X:%02X:%02X:%02X:%02X:%02X (source:%02X:%02X:%02X:%02X:%02X:%02X)\n", 
							addr5[0], addr5[1], addr5[2], addr5[3], addr5[4], addr5[5], 
							addr6[0], addr6[1], addr6[2], addr6[3], addr6[4], addr6[5]);
#endif							
						
					} 
					else  // pProxyEntry == 0 && pEntry = -1, addr5 is unknown
					{
						unsigned char toUpper = 0, freeSkb=1;
			
						// note: Mac addr of br0 and msh0 might be different.
						//       Hence, even the packet was issued by myself, addr4 might not equal to addr6
						//       That's why we keep a seperated 'toUpper' flag
						if(memcmp(pfrinfo->sa, GET_MY_HWADDR, MACADDRLEN))
	  						toUpper = 1;
						
						res = skb_p80211_to_ether(pskb->dev, privacy, pfrinfo);
						if (res == FAIL) {
							priv->ext_stats.rx_data_drops++;
							DEBUG_ERR("RX DROP: skb_p80211s_to_ether fail!\n");
							return FAIL;
						}

						// 802.3 format, I am not dest
						if( memcmp(pskb->data, GET_MY_HWADDR, MACADDRLEN)) 
						{
							if( toUpper )
							{
								// copy the content of pskb into a new memory (pnewskb)
								pnewskb = skb_copy(pskb, GFP_ATOMIC);
								if(pnewskb)
								{
									do_aodv_routing(priv, pnewskb, pnewskb->data);
								}
							}
							else
							{							
								do_aodv_routing(priv, pskb, pskb->data);
								freeSkb =0;
							}								
						}
						
						// filter packets that SA is myself
						if (toUpper)
							rtl_netif_rx(priv, pskb, pstat);	//also let bridge flooding to all interface besides mesh0
						else if( freeSkb )
							rtl_kfree_skb(priv, pskb, _SKB_RX_);
					} 
				} 
				return SUCCESS;		
			} 
			else
			// dest is me, but I am not root
			{
				static unsigned char zero_addr[MACADDRLEN] = {0}; 
				
				// filter packets that SA is myself 
				if ( memcmp(pfrinfo->sa, GET_MY_HWADDR, MACADDRLEN) == 0)
					return FAIL;
	
				//when the packets from root to dest, we need let dest triger AODV protocl to find src
				if(memcmp(priv->root_mac, zero_addr, MACADDRLEN) != 0)
				{
					struct path_sel_entry *pEntry = NULL;							
					pEntry = pathsel_query_table( priv, pfrinfo->sa );
					if(pEntry == (struct path_sel_entry *)-1) {
						unsigned long		flags ;
					
						MESH_LOCK(lock_Rreq, flags);
						GEN_PREQ_PACKET(pfrinfo->sa, priv, 1);
						MESH_UNLOCK(lock_Rreq, flags);								
						{
#if 0
							LOG_MESH_MSG("RX fire AODV to find:%02X:%02X:%02X:%02X:%02X:%02X\n",
								pfrinfo->da[0], pfrinfo->da[1], pfrinfo->da[2], pfrinfo->da[3], pfrinfo->da[4], pfrinfo->da[5]);
#endif								
						}
					} 
				} 
#ifdef PU_STANDARD_RX
  				if( Is_6AddrFormat && memcmp(GET_MY_HWADDR, addr5, MACADDRLEN) )
				{
					struct proxyupdate_table_entry Entry;
					struct proxy_table_entry *pEntry=NULL;
					struct path_sel_entry *pdstEntry=NULL;

					//check if addr5 is my associating STA
					if(get_stainfo(priv, addr5)!=NULL)
						goto receive_packet;

					// check if addr5 is any other AP's STA
					pEntry = HASH_SEARCH(priv->proxy_table, addr5);
					if( pEntry )
					{
						// case: my bridged PC
						//     da=me, STA = NULL, proxy != NULL 
						if(memcmp(GET_MY_HWADDR, pEntry->owner, MACADDRLEN)==0) 
							goto receive_packet;

						// case: I know the proxied node resided in (but not behind me)
						//     da=me, STA =NULL, proxy->owner=other node -> proxy update:add
						else 
						{
							Entry.PUflag = PU_add;
							memcpy((void *)Entry.proxymac, pEntry->owner, MACADDRLEN);
						}
					}
					else					
					{
						// case: unknown entry
						//  da=me, STA=NULL, proxy = NULL
						//  it might be an un-recorded bridged PC happens for pEntry = NULL
						Entry.PUflag = PU_delete;
						memcpy((void *)Entry.proxymac, GET_MY_HWADDR, MACADDRLEN);
					}
					
					pdstEntry = pathsel_query_table( priv, pfrinfo->sa);
					if( pdstEntry != (struct path_sel_entry *) -1 ){
						Entry.isMultihop = pdstEntry->hopcount ;
						memcpy(Entry.nexthopmac ,pdstEntry->nexthopMAC ,MACADDRLEN);
					}
					else{
						memset((void *)Entry.nexthopmac ,0xff ,MACADDRLEN);
						Entry.isMultihop =0;
					}
					memcpy(Entry.destproxymac ,pfrinfo->sa ,MACADDRLEN);
					memcpy((void *)Entry.proxiedmac, addr5 ,MACADDRLEN);								
					Entry.retry = 1U;
					Entry.PUSN = getPUSeq(priv);
					Entry.STAcount = 0x0001;
					Entry.update_time = xtime;
					issue_proxyupdate_MP(priv,&Entry);
				}

#endif // PU_STANDARD_RX

				goto receive_packet;
			}							
		} // if( !memcmp(pfrinfo->da, GET_MY_HWADDR, MACADDRLEN) )
		else if (priv->pmib->dot11OperationEntry.block_relay == 0)
		{  // dest is not me & relay is allow
			dapstat = get_stainfo(priv, pfrinfo->da);
			if(dapstat != NULL && isSTA(dapstat))
			{
				// filter packets that SA is myself 
				if ( memcmp(pfrinfo->sa, GET_MY_HWADDR, MACADDRLEN) == 0)
					return FAIL;
				else
				{
receive_packet:
					res = skb_p80211_to_ether(pskb->dev, privacy, pfrinfo);
					if (res == FAIL) {
						priv->ext_stats.rx_data_drops++;
						DEBUG_ERR("RX DROP: skb_p80211_to_ether fail!\n");
						return FAIL;
					}
					rtl_netif_rx(priv, pskb, dapstat);
					return SUCCESS;					
				}
			} 
		} 
		if (relay_11s_dataframe(pskb, /*pskb->dev,*/ privacy, pfrinfo))
			rtl_kfree_skb(priv, pskb, _SKB_RX_);	
		return SUCCESS;
	}
}

#ifdef _11s_TEST_MODE_
int mesh_debug_rx1(DRV_PRIV *priv, struct sk_buff *pskb)
{
	if(!memcmp("NRX", priv->pmib->dot1180211sInfo.mesh_reservedstr1, 3))
	{
		unsigned char destMAC[6]= { 0 };
		mac12_to_6(priv->pmib->dot1180211sInfo.mesh_reservedstr1+3, destMAC );

		if(!memcmp(pskb->data+6, destMAC, 6))
		{
			LOG_MESH_MSG("rx:%02X %02X %02X %02X %02X %02X ] %02X %02X %02X %02X %02X %02X, %s, %d\n",
				pskb->data[0]&0xff, pskb->data[1]&0xff, pskb->data[2]&0xff, pskb->data[3]&0xff, pskb->data[4]&0xff, pskb->data[5]&0xff,
				pskb->data[6]&0xff, pskb->data[7]&0xff, pskb->data[8]&0xff, pskb->data[9]&0xff, pskb->data[10]&0xff, pskb->data[11]&0xff,
				pskb->dev->name, pskb->len);
		}
	}
	return 0;
}

int mesh_debug_rx2(DRV_PRIV *priv, unsigned int cmd)
{
	if( ( !memcmp("JasonRver",  priv->pmib->dot1180211sInfo.mesh_reservedstr1, 9) ||
			!memcmp("JasonRelay", priv->pmib->dot1180211sInfo.mesh_reservedstr1, 10) )
			&& (priv->pmib->dot1180211sInfo.mesh_reserved3 +36 == (cmd & 0xfff) - _CRCLNG_ ))
		priv->mesh_stats.rx_crc_errors++;

	return 0;
}
#endif // _11s_TEST_MODE_

#endif	// CONFIG_RTK_MESH
