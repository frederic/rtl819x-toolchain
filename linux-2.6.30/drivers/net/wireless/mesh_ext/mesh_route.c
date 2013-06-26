/*
 *      Handling routines for Mesh in 802.11 SME (Station Management Entity)
 *
 *      PS: All extern function in ../8190n_headers.h
 */
#define _MESH_ROUTE_C_

#ifdef CONFIG_RTL8192CD
#include "../rtl8192cd/8192cd.h"
#include "../rtl8192cd/8192cd_headers.h"
#else
#include "../rtl8190/8190n.h"
#include "../rtl8190/8190n_headers.h"
#endif
#include "./mesh_route.h"


#ifdef CONFIG_RTK_MESH

unsigned short getMeshSeq(DRV_PRIV *priv)
{
	unsigned long flags;
	SAVE_INT_AND_CLI(flags);
	if(priv->pshare->meshare.seq == 0xffff)
		priv->pshare->meshare.seq = 1;
	else
		priv->pshare->meshare.seq++;
	RESTORE_INT(flags);
	return priv->pshare->meshare.seq;
}

//pepsi
#ifdef PU_STANDARD
UINT8 getPUSeq(DRV_PRIV *priv)
{
	unsigned long flags;
	SAVE_INT_AND_CLI(flags);
	if(priv->pshare->meshare.PUseq == 0xff)
		priv->pshare->meshare.PUseq = 1U;
	else
		priv->pshare->meshare.PUseq++;
	RESTORE_INT(flags);
	return priv->pshare->meshare.PUseq;
}
#endif


// return 0: duplicate
// return 1: ok
unsigned short chkMeshSeq(DRV_PRIV *priv, unsigned char *srcMac, unsigned short seq)
{
	unsigned short idx1 = (*(srcMac+4) & (SZ_HASH_IDX1-1)) ^ (*(srcMac+5) & (SZ_HASH_IDX1-1));
	unsigned short idx2 = seq & (SZ_HASH_IDX2 - 1);
	
	if(priv->pshare->meshare.RecentSeq[idx1][idx2] == seq)
	{
		return 0;
	}
	priv->pshare->meshare.RecentSeq[idx1][idx2] = seq;
	return 1;
}


void insert_PREQ_entry(char *targetMac, DRV_PRIV *priv)
{
	struct mesh_rreq_retry_entry *retryEntry;
	retryEntry= (struct mesh_rreq_retry_entry*) priv->mesh_rreq_retry_queue->search_entry(priv->mesh_rreq_retry_queue, targetMac);
	if(	retryEntry== NULL)
	{
		if(priv->RreqEnd == priv->RreqBegin)
		{
			u8 *oldmac = priv->RreqMAC[priv->RreqEnd];
			retryEntry= (struct mesh_rreq_retry_entry*) priv->mesh_rreq_retry_queue->search_entry(priv->mesh_rreq_retry_queue, oldmac);
		
			// retryEntry might be null for the first time
			// It has a potential bug when search_entry retun NULL (why?) but it it indeed a new round
			if(retryEntry) 
			{ 
				if(retryEntry->ptr)
				{
					struct sk_buff *poldskb;
					poldskb = (struct sk_buff*)deque(priv,&(retryEntry->ptr->head),&(retryEntry->ptr->tail),(unsigned int)retryEntry->ptr->pSkb,NUM_TXPKT_QUEUE);
					while(poldskb)
					{
						dev_kfree_skb_any(poldskb);
						poldskb = (struct sk_buff*)deque(priv,&(retryEntry->ptr->head),&(retryEntry->ptr->tail),(unsigned int)retryEntry->ptr->pSkb,NUM_TXPKT_QUEUE);
					}
				}
				priv->mesh_rreq_retry_queue->delete_entry(priv->mesh_rreq_retry_queue,retryEntry->MACAddr);
				priv->RreqBegin = (priv->RreqBegin + 1)%AODV_RREQ_TABLE_SIZE;
			}
		}

		memcpy(priv->RreqMAC[(priv->RreqEnd)], targetMac, MACADDRLEN);		
		(priv->RreqEnd) = ((priv->RreqEnd) + 1)%AODV_RREQ_TABLE_SIZE;
		priv->mesh_rreq_retry_queue->insert_entry(priv->mesh_rreq_retry_queue, targetMac, NULL);
		retryEntry= (struct mesh_rreq_retry_entry*) priv->mesh_rreq_retry_queue->search_entry(priv->mesh_rreq_retry_queue, targetMac);						
	}
	if(retryEntry) 
	{
		retryEntry->TimeStamp=retryEntry->createTime=jiffies;		
		retryEntry->Retries = 0;		// fix: 00000043 2007/11/29
		memcpy(retryEntry->MACAddr, targetMac, MACADDRLEN);						
	}
}

void GEN_PREQ_PACKET(char *targetMac, DRV_PRIV *priv, char insert)
{
		DOT11s_GEN_RREQ_PACKET rreq_event;
		memset((void*)&rreq_event, 0x0, sizeof(DOT11s_GEN_RREQ_PACKET));
		rreq_event.EventId = DOT11_EVENT_PATHSEL_GEN_RREQ;	
		rreq_event.IsMoreEvent = 0;
		memcpy(rreq_event.MyMACAddr,  GET_MY_HWADDR ,MACADDRLEN);
		memcpy(rreq_event.destMACAddr,  targetMac ,MACADDRLEN);
		rreq_event.TTL = _MESH_HEADER_TTL_;
		rreq_event.Seq_num = getMeshSeq(priv);
		DOT11_EnQueue2((unsigned long)priv, priv->pathsel_queue, (unsigned char*)&rreq_event, sizeof(DOT11s_GEN_RREQ_PACKET));
		notifyPathSelection();
		if(insert)
			insert_PREQ_entry(targetMac, priv);
}
		   

//modify by Joule for MESH HEADER
unsigned char* getMeshHeader(DRV_PRIV *priv, int wep_mode, unsigned char* pframe)
{
	INT		payload_offset;
	struct wlan_llc_t      *e_llc;
	struct wlan_snap_t     *e_snap;
	int wlan_pkt_format = WLAN_PKT_FORMAT_OTHERS;

	payload_offset = get_hdrlen(priv, pframe);

	if (GetPrivacy(pframe)) {
		if (((wep_mode == _WEP_40_PRIVACY_) || (wep_mode == _WEP_104_PRIVACY_))) {
			payload_offset += 4;
		}
		else if ((wep_mode == _TKIP_PRIVACY_) || (wep_mode == _CCMP_PRIVACY_)) {
			payload_offset += 8;
		}
		else {
			DEBUG_ERR("unallowed wep_mode privacy=%d\n", wep_mode);
			return NULL;
		}
	}

	e_llc = (struct wlan_llc_t *) (pframe + payload_offset);
	e_snap = (struct wlan_snap_t *) (pframe + payload_offset + sizeof(struct wlan_llc_t));

	if (e_llc->dsap==0xaa && e_llc->ssap==0xaa && e_llc->ctl==0x03) {

		if ( !memcmp(e_snap->oui, oui_rfc1042, WLAN_IEEE_OUI_LEN)) {
			wlan_pkt_format = WLAN_PKT_FORMAT_SNAP_RFC1042;
			if( !memcmp(&e_snap->type, SNAP_ETH_TYPE_IPX, 2) )
				wlan_pkt_format = WLAN_PKT_FORMAT_IPX_TYPE4;
			else if( !memcmp(&e_snap->type, SNAP_ETH_TYPE_APPLETALK_AARP, 2))
				wlan_pkt_format = WLAN_PKT_FORMAT_APPLETALK;
		}
		else if ( !memcmp(e_snap->oui, SNAP_HDR_APPLETALK_DDP, WLAN_IEEE_OUI_LEN) &&
					!memcmp(&e_snap->type, SNAP_ETH_TYPE_APPLETALK_DDP, 2) )
				wlan_pkt_format = WLAN_PKT_FORMAT_APPLETALK;
		else if ( !memcmp( e_snap->oui, oui_8021h, WLAN_IEEE_OUI_LEN))
			wlan_pkt_format = WLAN_PKT_FORMAT_SNAP_TUNNEL;
	}

	if ( (wlan_pkt_format == WLAN_PKT_FORMAT_SNAP_RFC1042)
			|| (wlan_pkt_format == WLAN_PKT_FORMAT_SNAP_TUNNEL) ) {
		payload_offset +=  sizeof(struct wlan_llc_t) + sizeof(struct wlan_snap_t);
	}

	return pframe+payload_offset;
}

void notifyPathSelection(void)
{ 
        struct task_struct *p;
        
        if(pid_pathsel != 0){
                read_lock(&tasklist_lock); 
                p = find_task_by_vpid(pid_pathsel);
                read_unlock(&tasklist_lock);
                if(p)
                {
                        // printk("send signal from kernel\n");
                        send_sig(SIGUSR1,p,0); 
                }
                else {
                        pid_pathsel = 0;
                }
        }
}


/*
 *	@brief	MESH MP time aging expire
 *
 *	@param	task_priv: priv
 *
 *	@retval	void
 */
 // chuangch 10.19
 void route_maintenance(DRV_PRIV *priv)
 { 
	 const int tbl_sz = 1 << priv->pathsel_table->table_size_power;
	 int i;
	 #ifdef __LINUX_2_6__
	 struct timespec now = xtime;
	 #else
	 struct timeval now = xtime;
	 #endif
	  unsigned long		 flags ;
	  
	 for (i = 0; i < tbl_sz; i++){
		 if(priv->pathsel_table->entry_array[i].dirty){

// Gallardo test 2008.0901			
			struct path_sel_entry *entry = (struct path_sel_entry*)priv->pathsel_table->entry_array[i].data;

			 if( entry->flag==0 && (((entry->metric > ((int)(entry->hopcount))<<8 )
			 		&& (now.tv_sec - entry->routeMaintain.tv_sec > HWMP_PREQ_REFRESH_PERIOD ))
			 	|| (now.tv_sec - entry->routeMaintain.tv_sec > HWMP_PREQ_REFRESH_PERIOD2)))
			 {				
				 entry->routeMaintain = xtime;
				 MESH_LOCK(lock_Rreq, flags);
				 GEN_PREQ_PACKET( entry->destMAC, priv, 1);
				 MESH_UNLOCK(lock_Rreq, flags);
			 }
		 }
	 }
 }
 
 /*
 void rreq_retry(struct rtk8185_priv *priv, struct mesh_rreq_retry_entry *retryEntry){ 
 // chuangch
	 GEN_PREQ_PACKET(retryEntry->MACAddr, priv, 0);
	 retryEntry->Retries++;
	 retryEntry->TimeStamp=jiffies;
	 return;
 }*/
 
 void aodv_expire(DRV_PRIV *priv)
 {
	 struct sk_buff *pskb;
	 struct mesh_rreq_retry_entry *retryEntry;
	 struct sk_buff* buf[AODV_RREQ_TABLE_SIZE]; // this version only send ONE packet for each queue
	 unsigned long flags;
	 int i=0, j=AODV_RREQ_TABLE_SIZE;		 
 
	 while (j>0)
		 buf[--j] = NULL;
		 
	 MESH_LOCK(lock_Rreq, flags);
		 
	 for(i=(priv->RreqBegin);i!=(priv->RreqEnd);i=((i+1)%AODV_RREQ_TABLE_SIZE)) 
	 {
		 retryEntry= (struct mesh_rreq_retry_entry*) priv->mesh_rreq_retry_queue->search_entry(priv->mesh_rreq_retry_queue,priv->RreqMAC[i]);
		 if(retryEntry==NULL) {
			 if( i!=priv->RreqBegin )
				 memcpy(priv->RreqMAC[ i ], priv->RreqMAC[priv->RreqBegin], MACADDRLEN);			 
			 priv->RreqBegin = (priv->RreqBegin+1)%AODV_RREQ_TABLE_SIZE;
			 continue;
		 }
 
		 if(time_after(jiffies,(retryEntry->TimeStamp)+ HWMP_NETDIAMETER_TRAVERSAL_TIME)) {
			 if(retryEntry->ptr==NULL)	
			 {
				 priv->mesh_rreq_retry_queue->delete_entry(priv->mesh_rreq_retry_queue,retryEntry->MACAddr);
				 if( i!=priv->RreqBegin )
					 memcpy(priv->RreqMAC[ i ], priv->RreqMAC[priv->RreqBegin], MACADDRLEN);			 
				 priv->RreqBegin = (priv->RreqBegin+1)%AODV_RREQ_TABLE_SIZE;
			 }
			 else if (retryEntry->Retries > HWMP_MAX_PREQ_RETRIES )
			 {
				 pskb=(struct sk_buff*)deque(priv,&(retryEntry->ptr->head),&(retryEntry->ptr->tail),(unsigned int)retryEntry->ptr->pSkb,NUM_TXPKT_QUEUE);
				 while(pskb)
				 {
					 dev_kfree_skb_any(pskb);
					 pskb=(struct sk_buff*)deque(priv,&(retryEntry->ptr->head),&(retryEntry->ptr->tail),(unsigned int)retryEntry->ptr->pSkb,NUM_TXPKT_QUEUE);
				 }
				 priv->mesh_rreq_retry_queue->delete_entry(priv->mesh_rreq_retry_queue,retryEntry->MACAddr);
				 if( i!=priv->RreqBegin )
					 memcpy(priv->RreqMAC[ i ], priv->RreqMAC[priv->RreqBegin], MACADDRLEN);				 
				 priv->RreqBegin = (priv->RreqBegin+1)%AODV_RREQ_TABLE_SIZE;
			 }
			 else
			 {
				 pskb = (struct sk_buff*)deque(priv,&(retryEntry->ptr->head),&(retryEntry->ptr->tail),(unsigned int)retryEntry->ptr->pSkb,NUM_TXPKT_QUEUE);
 
				 // add by chuangch 0928
				 retryEntry->TimeStamp=jiffies;
				 retryEntry->Retries++;
				 
				 GEN_PREQ_PACKET(retryEntry->MACAddr, priv, 0);
 
				 if (pskb) {
					 buf[j++] = pskb; // we call toAllPortal later
					 if(j==AODV_RREQ_TABLE_SIZE) // no more round
						 break;
				 }
				   
				 if(retryEntry->ptr->head == retryEntry->ptr->tail)
				 {
					 priv->mesh_rreq_retry_queue->delete_entry(priv->mesh_rreq_retry_queue,retryEntry->MACAddr);
						 
					 // change with the first entry
					 if( i!=priv->RreqBegin )
						 memcpy(priv->RreqMAC[i], priv->RreqMAC[priv->RreqBegin], MACADDRLEN);
 
					 priv->RreqBegin = (priv->RreqBegin+1)%AODV_RREQ_TABLE_SIZE;
				 }
			 } // (retryEntry->ptr!=NULL) and (not too old)
		 } // if(time_after) 
	 } // end of for(i=(priv->RreqBegin);i<AODV_RREQ_TABLE_SIZE;i++)
 
	 MESH_UNLOCK(lock_Rreq, flags);
		 
	 for(j=0;j<AODV_RREQ_TABLE_SIZE;j++)
	 {
		 pskb = buf[j];
			 
		 if(pskb ==NULL)
			 goto j_no_more;
	 
		 //sending the data to all portals
		 toAllPortal(pskb,priv);
	 }
 
 j_no_more:
	 return;
	 
 }
 

void init_mpp_pool(struct mpp_tb* pTB)
{
	int i;
	for (i = 0; i < MAX_MPP_NUM; i++) {
		pTB->pann_mpp_pool[i].flag = 0;
	}
	pTB->pool_count = 0;
}

// following functions modified by chuangch 2007.09.14 for pathsel_talbe being supported for hash function
struct path_sel_entry *pathsel_query_table(DRV_PRIV *priv ,unsigned char destaddr[MACADDRLEN] )
{
	if(priv->pathsel_table->search_entry( priv->pathsel_table, destaddr ))
	{
		return (struct path_sel_entry *)priv->pathsel_table->search_entry( priv->pathsel_table, destaddr );
	}
	else
	{
		return (struct path_sel_entry *)-1;
	}
}
//int pathsel_modify_table_entry( DRV_PRIV *priv, unsigned char destaddr[MACADDRLEN], struct path_sel_entry *pEntry)
int pathsel_modify_table_entry(DRV_PRIV *priv, struct path_sel_entry *pEntry)
{
	int i, tbl_sz;

	tbl_sz = 1 << priv->pathsel_table->table_size_power;

	for(i=0;i<tbl_sz;i++)
	{
		if(priv->pathsel_table->entry_array[i].dirty && 
			// !memcmp(((struct path_sel_entry*)priv->pathsel_table->entry_array[i].data)->destMAC, destaddr, MACADDRLEN))
			!memcmp(((struct path_sel_entry*)priv->pathsel_table->entry_array[i].data)->destMAC, pEntry->destMAC, MACADDRLEN))

		{
			struct path_sel_entry *entry ;
			entry = (struct path_sel_entry*)priv->pathsel_table->entry_array[i].data;

			if( entry->flag ==0 )
				memcpy(entry, pEntry, (int)&((struct path_sel_entry*)0)->start);

//			if (0 == pEntry->isvalid)
//				priv->pathsel_table->entry_array[i].dirty = 0;

// Gallardo test 2008.0901
//			entry->update_time = xtime; 
//			entry->routeMaintain = xtime; 
			
			return i;
		}	
	}
	return -1;
}
 int pathsel_table_entry_insert_tail( DRV_PRIV *priv, struct path_sel_entry *pEntry)
{
	return priv->pathsel_table->insert_entry( priv->pathsel_table, pEntry->destMAC, pEntry);
}
 int pathsel_table_entry_insert_head(struct path_sel_entry *pEntry)
{
	return -1;
}

int remove_path_entry(DRV_PRIV *priv,unsigned char *invalid_addr)
{
	struct proxy_table_entry *pEntry = 0;   
	int i;
        
	//clear conresponding path table entry  
	priv->pathsel_table->delete_entry( priv->pathsel_table, invalid_addr );
        
	//clear conresponding proxy entry               
	for(i=0;i<(1 << priv->proxy_table->table_size_power);i++)
	{
		pEntry = (struct proxy_table_entry *)(priv->proxy_table->entry_array[i].data);
		if(memcmp(invalid_addr,pEntry->owner,MACADDRLEN)==0)
			priv->proxy_table->entry_array[i].dirty = 0;                    
	}
                        
	return 0;
}

#endif	// CONFIG_RTK_MESH
