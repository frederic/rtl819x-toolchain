/*
 *      Handling routines for Mesh in 802.11 TX
 *
 *      PS: All extern function in ../8190n_headers.h
 */
#define _MESH_TX_C_

#ifdef CONFIG_RTL8192CD
#include "../rtl8192cd/8192cd_cfg.h"
#include "../rtl8192cd/8192cd.h"
#include "../rtl8192cd/8192cd_headers.h"
#else
#include "../rtl8190/8190n_cfg.h"
#include "../rtl8190/8190n.h"
#include "../rtl8190/8190n_headers.h"
#endif
#include "./mesh.h"
#include "./mesh_route.h"

#ifdef CONFIG_RTK_MESH

// 8190n_tx.c
#define RET_AGGRE_ENQUE			1
#define RET_AGGRE_DESC_FULL		2

__inline__ void ini_txinsn(struct tx_insn* txcfg, DRV_PRIV *priv)
{
	txcfg->is_11s = 1;
	txcfg->mesh_header.mesh_flag= 0;
	txcfg->mesh_header.TTL = _MESH_HEADER_TTL_;
	txcfg->mesh_header.segNum = getMeshSeq(priv);		
}


				 
int fire_data_frame(struct sk_buff *skb, struct net_device *dev, struct tx_insn* txinsn) 
{
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	unsigned long		flags;
	txinsn->q_num = BE_QUEUE;        // using low queue for data queue
	skb->cb[1] = 0;
	// txinsn->q_num   = 0; //using low queue for data queue
	txinsn->fr_type = _SKB_FRAME_TYPE_;
	txinsn->pframe  = skb;

#ifdef MESH_AMSDU
	struct stat_info *pstat = get_stainfo(priv, txinsn->nhop_11s);
	if ( pstat && (pstat->aggre_mthd == AGGRE_MTHD_MSDU) && (pstat->amsdu_level > 0)
#ifdef SUPPORT_TX_MCAST2UNI
		&& skb->cb[2] != (char)0xff
#endif
		) {
		int ret = amsdu_check(priv, skb, pstat, txinsn);

		if (ret == RET_AGGRE_ENQUE)
			goto stop_proc;

		if (ret == RET_AGGRE_DESC_FULL)
			goto free_and_stop;
	}
#endif	

	SAVE_INT_AND_CLI(flags);
	txinsn->phdr = (UINT8 *)get_wlanllchdr_from_poll(priv);

	if (txinsn->phdr == NULL) {
		DEBUG_ERR("Can't alloc wlan header!\n");
		goto xmit_11s_skb_fail;
	}

	if (skb->len > priv->pmib->dot11OperationEntry.dot11RTSThreshold)
		txinsn->retry = priv->pmib->dot11OperationEntry.dot11LongRetryLimit;
	else
		txinsn->retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;

	memset((void *)txinsn->phdr, 0, sizeof(struct wlanllc_hdr));

	/* Set Frame Type (Data Frame) */
	SetFrameSubType(txinsn->phdr, WIFI_11S_MESH);

	if (OPMODE & WIFI_AP_STATE) {
		SetFrDs(txinsn->phdr);
		SetToDs(txinsn->phdr);
	}
	else
		DEBUG_WARN("non supported mode yet!\n");

	if (WLAN_TX(priv, txinsn) == CONGESTED)
	{
xmit_11s_skb_fail:
		netif_stop_queue(dev);
		priv->ext_stats.tx_drops++;
		DEBUG_WARN("TX DROP: Congested!\n");
#ifdef _11s_TEST_MODE_
		if(!memcmp("JasonRelay", priv->pmib->dot1180211sInfo.mesh_reservedstr1, 10)
		&& memcmp(skb->data+14+2, "TEST_TRAFFIC", 12)==0  )
			priv->mesh_stats.rx_errors ++;
#endif		
		if (txinsn->phdr)
			release_wlanllchdr_to_poll(priv, txinsn->phdr);
		if (skb)
			rtl_kfree_skb(priv, skb, _SKB_TX_);
		RESTORE_INT(flags);
		return 0;
	}
	RESTORE_INT(flags);
	return 1;

#ifdef MESH_AMSDU	
free_and_stop:		

#ifdef RTL8190_FASTEXTDEV_FUNCALL
		rtl865x_extDev_kfree_skb(skb, FALSE);
#else
		dev_kfree_skb_any(skb);
#endif
stop_proc:
	return 1;
#endif
	
}

int notify_path_found(unsigned char *destaddr, DRV_PRIV *priv) 
{
	struct sk_buff *pskb;
	struct mesh_rreq_retry_entry * retryEntry;
  
	struct sk_buff * buf [NUM_TXPKT_QUEUE]; // To record the ALL popped-up skbs at one time, because we don't want enable spinlock for dev_queue_xmit
	unsigned long flags;
	int i=0;
	
	for(;i<NUM_TXPKT_QUEUE;i++)
		buf[i] = NULL;
		
	MESH_LOCK(lock_Rreq, flags);

	retryEntry= (struct mesh_rreq_retry_entry*) priv->mesh_rreq_retry_queue->search_entry(priv->mesh_rreq_retry_queue,destaddr);
	if(retryEntry == NULL) { // aodv_expire tx it
		MESH_UNLOCK(lock_Rreq, flags);
		return 0;
	}
	
	{
		struct path_sel_entry *pEntry;	
		pEntry = pathsel_query_table( priv, destaddr );
		if( pEntry == (struct path_sel_entry *)-1) 
		{
			struct proxy_table_entry*	pProxyEntry;
			pProxyEntry = (struct proxy_table_entry*) HASH_SEARCH(priv->proxy_table, destaddr);
			if(pProxyEntry != NULL)
				pEntry = pathsel_query_table( priv, pProxyEntry->owner );
		}		
		if(pEntry != (struct path_sel_entry *)-1)
		{
			pEntry->start = retryEntry->createTime;
			pEntry->end = jiffies;
		}
	}
	
	pskb = (struct sk_buff*)deque(priv,&(retryEntry->ptr->head),&(retryEntry->ptr->tail),(unsigned int)retryEntry->ptr->pSkb,NUM_TXPKT_QUEUE);

#if 0 
	if (pskb) {
		//to be deal of getting data from skb queue, and send to all portals
		LOG_MESH_MSG("found path to:%02X:%02X:%02X:%02X:%02X:%02X\n",
			destaddr[0], destaddr[1], destaddr[2], destaddr[3], destaddr[4], destaddr[5]);	
	}
#endif    

	//printk("Notified found ->>>>>> %ld \n", jiffies);

	i=0;
	while (pskb != NULL) {     //time out and it will clean up hashtable and send data to all portals.
		buf[i++] = pskb;
		pskb = (struct sk_buff*)deque(priv,&(retryEntry->ptr->head),&(retryEntry->ptr->tail),(unsigned int)retryEntry->ptr->pSkb,NUM_TXPKT_QUEUE);
	}
	priv->mesh_rreq_retry_queue->delete_entry(priv->mesh_rreq_retry_queue,retryEntry->MACAddr);
    
	MESH_UNLOCK(lock_Rreq, flags);

	for(i=0;i<NUM_TXPKT_QUEUE;i++) {
		pskb = buf[i];
		if(pskb == NULL)
			break;
// Gakki 
		{
			DECLARE_TXINSN(txinsn);
			// the data which transfer in mesh don't need update in proxy table
			if(dot11s_datapath_decision(pskb, /*priv->mesh_dev,*/ &txinsn,0))
				fire_data_frame(pskb, priv->mesh_dev, &txinsn);
		}
	}

	return 0;
}


void toAllPortal(struct sk_buff *pskb, DRV_PRIV *priv)
{	
	struct net_device		*dev = priv->mesh_dev;
	unsigned char 			*eth_src;
	struct proxy_table_entry*	pProxyEntry;
	int k;
			
	// Some potential bug would happen here , due to the reuse of txinsn
	// e.g., txinsn->llc is not initialized in firetx for non-aggregated + non-standard-ethtype frame
	// However, it's not always neccessary, when amsdu is triggered, it would use a different txinsn
	DECLARE_TXINSN(txinsn);

	if((dev == 0) || (!pskb) ){
		if(pskb)
			dev_kfree_skb_any(pskb);
		return;
	}

	ini_txinsn(&txinsn, priv);

	memcpy(txinsn.mesh_header.DestMACAddr, pskb->data, MACADDRLEN);
	memcpy(txinsn.mesh_header.SrcMACAddr, pskb->data+MACADDRLEN, MACADDRLEN);
	txinsn.mesh_header.mesh_flag = 0x01;

	// rule of A4/A6:
	// if there is an entry containing eth_src in Proxy Table,  A4=proxy.owner, A6=eth_src
	//    else if there is an entry containing eth_src in Path Sel Table, A4 = A6 = eth_src
	//    else A4 = A6 = eth_src
	// ==> Hence, A6 always be eth_src, and A4 is eth_src except existing a proxy table entry
	
	eth_src = pskb->data+MACADDRLEN;
	pProxyEntry = (struct proxy_table_entry*) HASH_SEARCH(priv->proxy_table, eth_src);
	if(pProxyEntry != NULL) {
		pProxyEntry->update_time = xtime;
		memcpy(eth_src, pProxyEntry->owner, MACADDRLEN);
	}
	
	// NOTE: DO NOT filter packets that SA is myself, because it can't accept root forwarding (e.g., ROOT-SRC(MPP1)-MPP2)
	for (k = 0; k < MAX_MPP_NUM; k++) 
	{
		if (priv->pann_mpp_tb->pann_mpp_pool[k].flag == 1) 
		{
			struct path_sel_entry *pEntry;
			
			pEntry = pathsel_query_table( priv ,priv->pann_mpp_tb->pann_mpp_pool[k].mac ); // chuangch 2007.09.14
			if(pEntry != (struct path_sel_entry *)-1) // has valid route path 
			{ 
				struct sk_buff *pnewskb;
				pnewskb = skb_copy(pskb, GFP_ATOMIC);

				if(!pnewskb) {
					DEBUG_ERR("Can't alloc skb to portal!\n");
					dev_kfree_skb_any(pskb);
					return;
				}

				memcpy(pnewskb->data, priv->pann_mpp_tb->pann_mpp_pool[k].mac, MACADDRLEN);
				txinsn.mesh_header.segNum = getMeshSeq(priv);
				txinsn.is_11s = 1;
//				chkMeshSeq(priv, pnewskb->data+MACADDRLEN,txinsn.mesh_header.segNum);
				memcpy(txinsn.nhop_11s,pEntry->nexthopMAC,MACADDRLEN);
				fire_data_frame(pnewskb, dev, &txinsn);
			} 
			else
				// not have valid route path 
				GEN_PREQ_PACKET(priv->pann_mpp_tb->pann_mpp_pool[k].mac, priv, 0);
		}
	} 

	dev_kfree_skb_any(pskb);

	return;
}

#ifdef	_11s_TEST_MODE_
unsigned char *get_galileo_from_poll(DRV_PRIV *priv)
{
	return get_buf_from_poll(priv, &(priv->pshare->galileo_list), (unsigned int *)(&priv->pshare->galileo_poll->count));
}

void release_galileo_to_poll(DRV_PRIV *priv, unsigned char *pbuf)
{
	release_buf_to_poll(priv, pbuf, &(priv->pshare->galileo_list), (unsigned int *)(&priv->pshare->galileo_poll->count));
}



void issue_test_traffic(struct sk_buff *skb)
{
	DRV_PRIV *priv = (DRV_PRIV *)skb->dev->priv;
	DECLARE_TXINSN(txinsn);
	struct tx_insn* ptxinsn=&txinsn;
	ini_txinsn(ptxinsn, priv);	
	skb_pull(skb, 14);	
	memcpy(ptxinsn->nhop_11s, skb->data, WLAN_ADDR_LEN);
	memcpy(skb->data+MACADDRLEN, GET_MY_HWADDR,MACADDRLEN);
	chkMeshSeq(priv, skb->data+MACADDRLEN, ptxinsn->mesh_header.segNum);
	memcpy(skb->data+14+26, (void*)(&jiffies), sizeof(long));
	if( skb->len > 80)
		memcpy(skb->data+48, (void*)&priv->pmib->dot1180211sInfo.mesh_reserved1, 30 );
	
	fire_data_frame(skb, skb->dev, ptxinsn);
}


struct stat_info* findNextSta(struct list_head *phead, struct list_head** plist, void* preHop)
{
	struct stat_info	*pstat =NULL;
	while(*plist != phead) 
	{
		pstat = list_entry(*plist, struct stat_info, mesh_mp_ptr);
		*plist = (*plist)->next;
		if( memcmp(pstat->hwaddr, preHop, MACADDRLEN))
			return pstat;		
	}
	return NULL;
}


void signin_txdesc_m2u(DRV_PRIV *priv, struct tx_insn* txcfg, char isTestTraffic)
{
	struct stat_info	*pstat;
	struct list_head	*phead, *plist;
		
	phead= &priv->mesh_mp_hdr;
	plist = phead->next;
	txcfg->is_11s = 17;
	txcfg->need_ack = 1;			
	txcfg->q_num = (txcfg->fr_type == _SKB_FRAME_TYPE_)? VI_QUEUE : MANAGE_QUE_NUM;;
	pstat = findNextSta(phead, &plist, txcfg->prehop_11s);
	
	while( pstat )
	{
		memcpy(GetAddr1Ptr(txcfg->phdr), pstat->hwaddr, MACADDRLEN); 
		txcfg->pstat = pstat;

		if(priv->pmib->dot1180211sInfo.mesh_reserved2&1024)					
			txcfg->lowest_tx_rate = txcfg->tx_rate = get_tx_rate(priv, pstat);
	
		pstat = findNextSta(phead, &plist, txcfg->prehop_11s);
		
		if( pstat == NULL)
			txcfg->is_11s = 1;

		SIGNINTX(priv, txcfg);

		if( pstat != NULL)
		{					
			UINT8 *pwlhdr = (txcfg->fr_type == _SKB_FRAME_TYPE_) ? get_wlanllchdr_from_poll(priv) : get_wlanhdr_from_poll(priv);
			if(!pwlhdr)	break;
			memcpy((void *)pwlhdr, (void *)(txcfg->phdr), txcfg->hdr_len+((txcfg->fr_type == _SKB_FRAME_TYPE_) ? 8 : 0)); 
			txcfg->phdr = pwlhdr;
		}					
		if( isTestTraffic )
			priv->mesh_stats.rx_bytes++;			
	}

	if( txcfg->is_11s == 17)
	{
		txcfg->is_11s =1;

		SIGNINTX(priv, txcfg);

		if( isTestTraffic )
			priv->mesh_stats.rx_bytes++;	
	}	
	
}


// Galileo 2008.07.30
const int MaxInterVal  = 10;

void galileo_timer(unsigned long task_priv)
{
	struct Galileo* gakki = (struct Galileo*)task_priv;
	DRV_PRIV *priv = (DRV_PRIV*)gakki->priv;

	if (gakki)
	{
		if((gakki->txcfg.fr_type == _SKB_FRAME_TYPE_)
			&& (( !memcmp("JasonRelay",  priv->pmib->dot1180211sInfo.mesh_reservedstr1, 10)
			|| !memcmp("JasonSender", priv->pmib->dot1180211sInfo.mesh_reservedstr1, 11)  )))
		{
			unsigned char *frame =  ((struct sk_buff *)(gakki->txcfg.pframe))->data;
			if( memcmp(frame+6, "TEST_TRAFFIC", 12)==0 )
				priv->mesh_stats.rx_bytes++;
		}

		if( --gakki->tx_count )
		{
			unsigned char r =0;
			if( priv->pmib->dot1180211sInfo.mesh_reserved2&64)
			{
				get_random_bytes(&r, 1);
				r %= MaxInterVal;
			}
			SIGNINTX(priv, &gakki->txcfg);
			mod_timer(&gakki->expire_timer, jiffies + r+1);
		}
		else
		{
			del_timer(&gakki->expire_timer);
			gakki->txcfg.is_11s =1;
			SIGNINTX(priv, &gakki->txcfg);
			release_galileo_to_poll(priv, (unsigned char*)gakki);
		}
	}
}



void signin_txdesc_multiTime(DRV_PRIV *priv, struct tx_insn* txcfg)
{
//	short count = (priv->pmib->dot1180211sInfo.mesh_reserved1>>8)& 0xff;
	short count = priv->pmib->dot1180211sInfo.mesh_reserved1;
	struct Galileo* gakki ;
	char totti=0;

	// stats
	if(  (txcfg->fr_type == _SKB_FRAME_TYPE_)
		&& (( !memcmp("JasonRelay",  priv->pmib->dot1180211sInfo.mesh_reservedstr1, 10)
	   	   || !memcmp("JasonSender", priv->pmib->dot1180211sInfo.mesh_reservedstr1, 11)  )))
	{
		unsigned char *frame =	((struct sk_buff *)(txcfg->pframe))->data;
		if( memcmp(frame+6, "TEST_TRAFFIC", 12)==0 )
		{
			priv->mesh_stats.rx_bytes++;
			totti = 1;
		}
	}
		
	if( count >0 /* && (txcfg->fr_type == _SKB_FRAME_TYPE_)*/ )
	{
		if( priv->pmib->dot1180211sInfo.mesh_reserved2&128)
		{
			if( totti )
				priv->mesh_stats.rx_bytes += count;		
			txcfg->is_11s =48;
			while( count--)
				SIGNINTX(priv, txcfg);	
			txcfg->is_11s =1;
			SIGNINTX(priv, txcfg);				
		}
		else		
		{	
			gakki =  (struct Galileo*) get_galileo_from_poll(priv);
			if( gakki )
			{	
				unsigned char r=0;	
				if( priv->pmib->dot1180211sInfo.mesh_reserved2&64)
				{
					get_random_bytes(&r, 1);
					r %= MaxInterVal;
				}				
				txcfg->is_11s =48;
				memcpy(&gakki->txcfg, txcfg, sizeof(struct tx_insn));
				gakki->tx_count = count;
				gakki->expire_timer.data = (unsigned long) gakki;
				gakki->expire_timer.expires = jiffies + r+1;
				mod_timer(&gakki->expire_timer, gakki->expire_timer.expires);
			}
			SIGNINTX(priv, txcfg);	
		}
		
	}else
		SIGNINTX(priv, txcfg);	
}



void signin_txdesc_galileo(DRV_PRIV *priv, struct tx_insn* txcfg)
{
	// 11s multicast 
	if( IS_MCAST(GetAddr1Ptr(txcfg->phdr)) && txcfg->is_11s )
	{
		char isTestTraffic = 0;

// DHCP trace start		
		if( txcfg->fr_type == _SKB_FRAME_TYPE_ && !memcmp("XMT", priv->pmib->dot1180211sInfo.mesh_reservedstr1, 3) )
		{
			unsigned char srcMAC[6]= { 0 }; //,  *frame = ((struct sk_buff *)(txcfg->pframe))->data;				
			mac12_to_6(priv->pmib->dot1180211sInfo.mesh_reservedstr1+3, srcMAC );
		
			if(!memcmp(GetAddr4Ptr(txcfg->phdr), srcMAC, 6)&& 
				( (txcfg->fr_len>331 && txcfg->fr_len<364)
				||(txcfg->fr_len>576 && txcfg->fr_len<591)	)	)
			{

				printk("$(%d,%d)\n", txcfg->fr_len, txcfg->hdr_len);
	//			for(j=0; j<30; j++)
	//				printk("%02X ", frame[j]&0xff);
	//			printk("\n");
			}
		}
// end	
		if(  (txcfg->fr_type == _SKB_FRAME_TYPE_)
			&& (( !memcmp("JasonRelay",  priv->pmib->dot1180211sInfo.mesh_reservedstr1, 10)
			|| !memcmp("JasonSender", priv->pmib->dot1180211sInfo.mesh_reservedstr1, 11)  )))
		{
			unsigned char *frame =	((struct sk_buff *)(txcfg->pframe))->data;
			if( !memcmp(frame+6, "TEST_TRAFFIC", 12) )
			{
				priv->mesh_stats.tx_packets++;
				isTestTraffic = 1;
			}
		}

		// multi-time broadcast
		if( priv->pmib->dot1180211sInfo.mesh_reserved2&32)
			signin_txdesc_multiTime(priv, txcfg);	
		else
		{	
			unsigned char action = *((unsigned char*)txcfg->pframe+6+4+1);
					
			if( (priv->pmib->dot1180211sInfo.mesh_reserved2&2 && (txcfg->fr_type == _SKB_FRAME_TYPE_) )
				||(priv->pmib->dot1180211sInfo.mesh_reserved2&1 && ( txcfg->fr_type == _PRE_ALLOCMEM_ && 
				( action == ACTION_FIELD_PANN || action == ACTION_FIELD_RREQ  || action ==ACTION_FIELD_RANN))))

		// multi-unicast			
				signin_txdesc_m2u(priv, txcfg, isTestTraffic);				
			else	
			{
				SIGNINTX(priv, txcfg);
				if( isTestTraffic )
					priv->mesh_stats.rx_bytes++;
			}			
		}
	}
	else
		SIGNINTX(priv, txcfg);
}

#endif



int issue_11s_mesh_action(struct sk_buff *skb, struct net_device *dev)
{
	DRV_PRIV *priv = (DRV_PRIV *)dev->priv;
	DECLARE_TXINSN(txinsn);
	
	unsigned char *pframe = skb->data+14, *pbuf = NULL;
//	struct stat_info	*pstat;
	int len;
	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;	
	txinsn.is_11s = 1;
	txinsn.fixed_rate = 1;	
	
// chkMeshSeq ??    

	// construct mesh_header of txinsn
	len = (*(GetMeshHeaderFlagWithoutQOS(pframe))& 0x01) ? 16 :4;	//if 6 address is enable, the bit 0 of AE will set to 1,(b7 b6 b5...b0) 
	memcpy(&(txinsn.mesh_header), GetMeshHeaderFlagWithoutQOS(pframe),len);
	
	/* header clean to "0" */
	if( skb->len > (14+WLAN_HDR_A3_LEN)) {

		txinsn.phdr = get_wlanhdr_from_poll(priv);
		pbuf = txinsn.pframe  = get_mgtbuf_from_poll(priv);
		
		if(pbuf == 0 || txinsn.phdr==0)
			goto issue_11s_mesh_actio_FAIL;
	
		// only copy the first 3 address + zero-valued seq 
		memset((void *)(txinsn.phdr), 0, sizeof (struct wlan_hdr));
		memcpy((void *)txinsn.phdr, pframe, WLAN_HDR_A3_LEN);
		
		txinsn.fr_len = skb->len -(14+WLAN_HDR_A3_LEN);
		memcpy(pbuf, pframe + WLAN_HDR_A3_LEN , txinsn.fr_len );
/*
		pstat = get_stainfo(priv, GetAddr1Ptr(pframe)); 
		if (pstat)
		{			
			txinsn.tx_rate = get_tx_rate(priv, pstat);
			txinsn.lowest_tx_rate = get_lowest_tx_rate(priv, pstat, txinsn.tx_rate);
		}else*/
			txinsn.lowest_tx_rate = txinsn.tx_rate = find_rate(priv, NULL, 0, 1);		
	} else
		return 0;

	if (skb->len > priv->pmib->dot11OperationEntry.dot11RTSThreshold)
		txinsn.retry = priv->pmib->dot11OperationEntry.dot11LongRetryLimit;
	else
		txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;

	if (WLAN_TX(priv, &txinsn) == CONGESTED) {
		netif_stop_queue(dev);
		priv->ext_stats.tx_drops++;
		DEBUG_WARN("TX DROP: Congested!\n");
issue_11s_mesh_actio_FAIL:
		
		if (txinsn.phdr)
			release_wlanhdr_to_poll(priv, txinsn.phdr);
		
		if (txinsn.pframe)
			release_mgtbuf_to_poll(priv, txinsn.pframe);
		
		return 0;
	}

	dev_kfree_skb_any(skb);
	
#ifdef __KERNEL__
	dev->trans_start = jiffies;
#endif

	return 1;
}

void do_aodv_routing(DRV_PRIV *priv, struct sk_buff *skb, unsigned char *Mesh_dest)
{
	unsigned long		flags ;
	struct mesh_rreq_retry_entry *retryEntry;
	int result=0;
	
	MESH_LOCK(lock_Rreq, flags);
					
	retryEntry= (struct mesh_rreq_retry_entry*) priv->mesh_rreq_retry_queue->search_entry(priv->mesh_rreq_retry_queue,Mesh_dest);
				
	// with buffer mechanism and AODV timeout flow
	if (retryEntry == NULL) // new AODV path
	{
		GEN_PREQ_PACKET(Mesh_dest, priv, 1);
		retryEntry= (struct mesh_rreq_retry_entry*) priv->mesh_rreq_retry_queue->search_entry(priv->mesh_rreq_retry_queue,Mesh_dest);
		if(retryEntry==NULL )
		{
			MESH_UNLOCK(lock_Rreq, flags);
			dev_kfree_skb_any(skb);
			return;
		}
			
		result = enque(priv,&(retryEntry->ptr->head),&(retryEntry->ptr->tail), (unsigned int)retryEntry->ptr->pSkb, NUM_TXPKT_QUEUE,(void*)skb);

		if(result == FALSE)
		{
			struct sk_buff *poldskb;
					
			poldskb = (struct sk_buff*)deque(priv,&(retryEntry->ptr->head),&(retryEntry->ptr->tail),(unsigned int)retryEntry->ptr->pSkb,NUM_TXPKT_QUEUE);
			if(poldskb)
				dev_kfree_skb_any(poldskb);
				
			result = enque(priv,&(retryEntry->ptr->head),&(retryEntry->ptr->tail),(unsigned int)retryEntry->ptr->pSkb,NUM_TXPKT_QUEUE,(void*)skb);
			if(result == FALSE)
				dev_kfree_skb_any(skb);
		}
			
	}
	else { 
		result = enque(priv,&(retryEntry->ptr->head),&(retryEntry->ptr->tail), (unsigned int)retryEntry->ptr->pSkb,NUM_TXPKT_QUEUE,(void*)skb);
		if(result == FALSE)
		{
			struct sk_buff *poldskb;
			
			poldskb = (struct sk_buff*)deque(priv,&(retryEntry->ptr->head),&(retryEntry->ptr->tail),(unsigned int)retryEntry->ptr->pSkb,NUM_TXPKT_QUEUE);
			
			if(poldskb)
				dev_kfree_skb_any(poldskb);
				
			result = enque(priv,&(retryEntry->ptr->head),&(retryEntry->ptr->tail),(unsigned int)retryEntry->ptr->pSkb,NUM_TXPKT_QUEUE,(void*)skb);
			if(result == FALSE)
				dev_kfree_skb_any(skb);
		}
	}
		
	MESH_UNLOCK(lock_Rreq, flags);
	return;
}



/*
	pfrinfo->is_11s =1  => 802.11 header
	pfrinfo->is_11s =8  => 802.3  header + mesh header
*/
int relay_11s_dataframe(struct sk_buff *skb, /*struct net_device *dev,*/ int privacy, struct rx_frinfo *pfrinfo)
{	
	DRV_PRIV *priv = (DRV_PRIV *)skb->dev->priv;
#ifdef RX_RL_SHORTCUT	
	struct stat_info	*pstat = get_stainfo(priv, GetAddr2Ptr(skb->data));
#endif
	
	DECLARE_TXINSN(txinsn);

// Gakki 
	if (IS_MCAST(pfrinfo->da)) {
		memcpy(txinsn.nhop_11s, pfrinfo->da, MACADDRLEN);		
#ifdef	_11s_TEST_MODE_
		memcpy(txinsn.prehop_11s, pfrinfo->prehop_11s, MACADDRLEN);
#endif
  	} 
	else 
  	{
		unsigned char *destaddr= pfrinfo->da;
		struct path_sel_entry *pEntry;	
/*
#ifndef MESH_AMSDU	
		unsigned char *prehopaddr[MACADDRLEN];
		memcpy(prehopaddr, GetAddr2Ptr(skb->data), MACADDRLEN);
		memcpy(GetAddr2Ptr(skb->data),GET_MY_HWADDR,MACADDRLEN);
#endif
*/
		pEntry = pathsel_query_table( priv, destaddr );		
		if(pEntry == (struct path_sel_entry *)-1) // not have valid route path
		{
			DOT11s_GEN_RERR_PACKET rerr_event;			
			memset((void*)&rerr_event, 0x0, sizeof(DOT11s_GEN_RERR_PACKET));
			rerr_event.EventId = DOT11_EVENT_PATHSEL_GEN_RERR;
			rerr_event.IsMoreEvent = 0;
			memcpy(rerr_event.MyMACAddr,  GET_MY_HWADDR ,MACADDRLEN); 
			memcpy(rerr_event.SorNMACAddr,  pfrinfo->da ,MACADDRLEN);
			memcpy(rerr_event.DataDestMAC,  pfrinfo->sa ,MACADDRLEN);

			// this field will be used by Path Selection daemon for the following case 
			// when MP want to generate RERR to data source but no path to the data source now, it will send the RERR which set Addr2 = Prehop.

//#ifdef MESH_AMSDU
			memcpy(rerr_event.PrehopMAC, pfrinfo->prehop_11s, MACADDRLEN);
/*#else
			memcpy(rerr_event.PrehopMAC, prehopaddr, MACADDRLEN);
#endif*/
			rerr_event.TTL = _MESH_HEADER_TTL_;
			rerr_event.Seq_num = getMeshSeq(priv);
			rerr_event.Flag = 2;// flag = 2 means this MP doesn't have the nexthop information for the destination in pathseleciton table
			{ // only record the different ones
				static u8 chd[6] = {0};
				if(memcmp(chd, destaddr, 6)) {
					// printk("tx relay, fire AODV to find:%02X:%02X:%02X:%02X:%02X:%02X!\n",
#if 0
					LOG_MESH_MSG("tx relay, fire AODV to find:%02X:%02X:%02X:%02X:%02X:%02X!\n",
						destaddr[0],destaddr[1],destaddr[2],destaddr[3],destaddr[4],destaddr[5]);
#endif						
					memcpy(chd, destaddr,6);
				}
			}
			DOT11_EnQueue2((unsigned long)priv, priv->pathsel_queue, (unsigned char*)&rerr_event, sizeof(DOT11s_GEN_RERR_PACKET));
			notifyPathSelection();
			return 1;

		} // if(pEntry == (struct path_sel_entry *)-1)
		else
		{
			memcpy(txinsn.nhop_11s, pEntry->nexthopMAC, MACADDRLEN);
		}
	} // unicast packet
	

// Gakki
#ifdef MESH_AMSDU
	if(pfrinfo->is_11s&1) 
#endif
	{
		//unsigned char Mesh_dest[MACADDRLEN<<1];
		//memcpy(Mesh_dest, pfrinfo->da, MACADDRLEN);
		//memcpy(Mesh_dest + MACADDRLEN, pfrinfo->sa, MACADDRLEN);	
		pfrinfo->is_11s = 2;
		if(skb_p80211_to_ether(skb->dev, privacy, pfrinfo) == FAIL) // for e.g., CISCO CDP which has unsupported LLC's vendor ID
			return 1;

		if( pfrinfo->mesh_header.mesh_flag & 0x01 ) {
			//memcpy(skb->data, Mesh_dest, MACADDRLEN<<1);
#if defined(RX_SHORTCUT) && defined(RX_RL_SHORTCUT)		
			if (!IS_MCAST(pfrinfo->da) && !priv->pmib->dot11OperationEntry.disable_rxsc && pstat) {
				//memcpy(pstat->rxsc_nexthopMAC, txinsn.nhop_11s, MACADDRLEN);
				//memcpy(&(pstat->rx_ethhdr),Mesh_dest, MACADDRLEN<<1);
				memcpy(&(pstat->rx_ethhdr),skb->data, MACADDRLEN<<1);
		
				//memcpy((void *)&pstat->rx_wlanhdr.wlanhdr.meshhdr.DestMACAddr, (const void *)pfrinfo->mesh_header.DestMACAddr, WLAN_ETHADDR_LEN);
				//memcpy((void *)&pstat->rx_wlanhdr.wlanhdr.meshhdr.SrcMACAddr, (const void *)pfrinfo->mesh_header.SrcMACAddr, WLAN_ETHADDR_LEN);

			}
#endif
		}
	}

	//memcpy( &(txinsn.mesh_header), &(pfrinfo->mesh_header), sizeof(struct MESH_HDR));

#ifdef	_11s_TEST_MODE_	

	if(!memcmp("RLY", priv->pmib->dot1180211sInfo.mesh_reservedstr1, 3) )
	{
		unsigned char destMAC[6]= { 0 }, *m1= pfrinfo->mesh_header.DestMACAddr;
		mac12_to_6(priv->pmib->dot1180211sInfo.mesh_reservedstr1+3, destMAC );
		
		if( m1[-4] && !(memcmp(m1, destMAC, 6) & memcmp(m1+6, destMAC, 6)) )
		{
			LOG_MESH_MSG("relay: %02X %02X %02X %02X [%02X %02X %02X %02X %02X %02X]%02X %02X %02X %02X %02X %02X\n",
			m1[-4]&0xff, m1[-3]&0xff, m1[-2]&0xff, m1[-1]&0xff,
			m1[0]&0xff, m1[1]&0xff, m1[2]&0xff, m1[3]&0xff, m1[4]&0xff, m1[5]&0xff,
			m1[6]&0xff, m1[7]&0xff, m1[8]&0xff, m1[9]&0xff, m1[10]&0xff, m1[11]&0xff 	);	
		}
	}
#endif

	//txinsn.is_11s = pfrinfo->is_11s;
	txinsn.is_11s = RELAY_11S;

	fire_data_frame(skb, priv->mesh_dev, &txinsn);

#ifdef __KERNEL__
	priv->mesh_dev->trans_start = jiffies;
#endif
	return 0;
}

#if defined(MESH_TX_SHORTCUT)
int mesh_txsc_decision(struct tx_insn* cfgNew, struct tx_insn* cfgOld)
{
	//cfgOld&1 to confirm no amsdu last time
	if( (cfgOld->is_11s & 1) && (cfgNew->mesh_header.mesh_flag&1) &&	
		(cfgNew->mesh_header.mesh_flag == cfgOld->mesh_header.mesh_flag) &&
		!memcmp(cfgNew->mesh_header.DestMACAddr, cfgOld->mesh_header.DestMACAddr, MACADDRLEN)   )
	{
		cfgOld->mesh_header.segNum = cfgNew->mesh_header.segNum;
		return 1;
	}
	else
		return 0;
}
#endif

int dot11s_datapath_decision(struct sk_buff *skb, /*struct net_device *dev,*/ struct tx_insn* ptxinsn, int isUpdateProxyTable)
{
	DRV_PRIV *priv = (DRV_PRIV *)skb->dev->priv;
	ini_txinsn(ptxinsn, priv);

	//hex_dump(skb->data,32);

	if(isUpdateProxyTable == 1) {
		unsigned char *Eth_src = skb->data+MACADDRLEN;
		if((!IS_MCAST(Eth_src)) && memcmp(Eth_src, GET_MY_HWADDR, MACADDRLEN)) { // the entry briged by me
			struct proxy_table_entry	Entry;
			memcpy(Entry.sta, Eth_src, MACADDRLEN);
			memcpy(Entry.owner, GET_MY_HWADDR, MACADDRLEN);
			Entry.update_time = xtime;
			HASH_INSERT(priv->proxy_table, Entry.sta, &Entry);
			proxy_debug("%s %d Insert Proxy table: %02x:%02x:%02x:%02x:%02x:%02x/%02x:%02x:%02x:%02x:%02x:%02x\n",
						__FUNCTION__,__LINE__,Entry.owner[0],Entry.owner[1],Entry.owner[2],Entry.owner[3],Entry.owner[4],Entry.owner[5],
						Entry.sta[0],Entry.sta[1],Entry.sta[2],Entry.sta[3],Entry.sta[4],Entry.sta[5]);
		}
	}

	if (IS_MCAST(skb->data))
	{		
		// Note that Addr4 of an 11s broadcast frame is the original packet issuer (i.e., skb->data+MACADDRLEN)
		// When rx receives an 11s broadcast frame, it also check mssh seq by using Addr4 as the search key
		chkMeshSeq(priv, skb->data+MACADDRLEN,ptxinsn->mesh_header.segNum);
		memcpy(ptxinsn->nhop_11s, skb->data, MACADDRLEN);
	}
	else // unicast
	{
		struct path_sel_entry *pEntry;
		struct proxy_table_entry*	pProxyEntry;
		
		memcpy(ptxinsn->mesh_header.DestMACAddr, skb->data, MACADDRLEN);
		memcpy(ptxinsn->mesh_header.SrcMACAddr,  skb->data+MACADDRLEN, MACADDRLEN);

		// search proxy table for dest addr
		pProxyEntry = (struct proxy_table_entry*) HASH_SEARCH(priv->proxy_table, ptxinsn->mesh_header.DestMACAddr);
		txsc_debug("Search MAC:%02x%02x%02x%02x%02x%02x from proxy table\n",
			ptxinsn->mesh_header.DestMACAddr[0],ptxinsn->mesh_header.DestMACAddr[1],ptxinsn->mesh_header.DestMACAddr[2],
			ptxinsn->mesh_header.DestMACAddr[3],ptxinsn->mesh_header.DestMACAddr[4],ptxinsn->mesh_header.DestMACAddr[5]);
		if(pProxyEntry != NULL) // src isn't me or dest can find in proxy table
		{
			// e.g., bridge table had expired (would it happen?)
			if(memcmp(pProxyEntry->owner, GET_MY_HWADDR, MACADDRLEN) == 0) {
				//chris -- clean the entry
				HASH_DELETE(priv->proxy_table, ptxinsn->mesh_header.DestMACAddr);
				dev_kfree_skb_any(skb);
				return 0;
			}
			// The code is important for uni-directional traffic (how often?) to maintain a proxy entry.
			// However, its side effect is to forcedly occupy a proxy entry during the duration of the traffic.
			// pProxyEntry->update_time = xtime;

			ptxinsn->mesh_header.mesh_flag = 0x01;
			memcpy(skb->data, pProxyEntry->owner, MACADDRLEN);
			txsc_debug("found, owner is %02x%02x%02x%02x%02x%02x\n",
				pProxyEntry->owner[0],pProxyEntry->owner[1],pProxyEntry->owner[2],
				pProxyEntry->owner[3],pProxyEntry->owner[4],pProxyEntry->owner[5]);
		}
		
		if(memcmp(ptxinsn->mesh_header.SrcMACAddr, GET_MY_HWADDR, MACADDRLEN)) {
			ptxinsn->mesh_header.mesh_flag = 0x01;

			if(isUpdateProxyTable == 1) {
				struct proxy_table_entry Entry;
				memcpy(Entry.sta, skb->data+MACADDRLEN, MACADDRLEN);
				memcpy(Entry.owner, GET_MY_HWADDR, MACADDRLEN);
				Entry.update_time = xtime;
				//priv->proxy_table->insert_entry (priv->proxy_table, Entry.sta, &Entry); //insert/update proxy table
				//pepsi
				HASH_INSERT(priv->proxy_table, Entry.sta, &Entry);
				proxy_debug("%s %d Insert Proxy table: %02x:%02x:%02x:%02x:%02x:%02x/%02x:%02x:%02x:%02x:%02x:%02x\n",
						__FUNCTION__,__LINE__,Entry.owner[0],Entry.owner[1],Entry.owner[2],Entry.owner[3],Entry.owner[4],Entry.owner[5],
						Entry.sta[0],Entry.sta[1],Entry.sta[2],Entry.sta[3],Entry.sta[4],Entry.sta[5]);
				/*
				//D1.09 Proxy Update Protocol
				//		1. Inform destination MP					
				// *puEntry.PUflag = PU_add;
				puEntry.PUSN = getPUSeq(priv);
				puEntry.STAcount = 0x0001;
				memcpy(puEntry.proxymac, GET_MY_HWADDR, MACADDRLEN);
				memcpy(puEntry.proxiedmac, skb->data+MACADDRLEN, MACADDRLEN);
				memcpy(puEntry.destproxymac, Mesh_dest, MACADDRLEN);* //
				//the rest infomation is finished only if path is valid
				*/
			}
			
			// if end point src is recorded sta, addr4 should be its owner
			pProxyEntry =(struct proxy_table_entry*) HASH_SEARCH(priv->proxy_table, ptxinsn->mesh_header.SrcMACAddr);
			if( pProxyEntry != NULL )	 {	
				pProxyEntry->update_time = xtime;
				memcpy(skb->data+MACADDRLEN, pProxyEntry->owner, MACADDRLEN);
			} 
/*			
			else {
				if(pathsel_query_table( priv, ptxinsn->mesh_header.SrcMACAddr ) == (struct path_sel_entry *)-1)
					memcpy(skb->data+MACADDRLEN, GET_MY_HWADDR, MACADDRLEN);
			}
*/

		} // memcmp(Eth_src,GET_MY_HWADDR,MACADDRLEN) <> 0

		pEntry = pathsel_query_table( priv, skb->data );
		if(pEntry != (struct path_sel_entry *)-1) {// has valid route path 
			memcpy(ptxinsn->nhop_11s, pEntry->nexthopMAC, MACADDRLEN);
			{
				static struct path_sel_entry *lst = NULL;
				if(lst != pEntry) {
// 2008.08.12 totti test
/*
					unsigned char *Eth_dest = ptxinsn->mesh_header.DestMACAddr;
					LOG_MESH_MSG("TX send pkt to %02X:%02X:%02X:%02X:%02X:%02X DIRECTLY\n",
								Eth_dest[0],Eth_dest[1],Eth_dest[2],Eth_dest[3],Eth_dest[4],Eth_dest[5]);
*/								
					lst = pEntry;
				}
			}
				//finish insert information of PU
				/*puEntry.isMultihop = pEntry->hopcount;
				puEntry.update_time = xtime;
				memcpy(puEntry.nexthopmac, pEntry->nexthopMAC, MACADDRLEN);
				HASH_INSERT(priv->proxyupdate_table, &puEntry.PUSN, &puEntry);
				issue_proxyupdate_MP(priv, &puEntry);*/
		} 
		else {// not have valid route path 
			static unsigned char zeroAddr[MACADDRLEN] = { 0 };      // fix: 0000072 2008/02/01
			unsigned char Mesh_dest[MACADDRLEN];
			memcpy(Mesh_dest, skb->data, MACADDRLEN);
			memcpy(skb->data, ptxinsn->mesh_header.DestMACAddr, MACADDRLEN);
			memcpy(skb->data+MACADDRLEN, ptxinsn->mesh_header.SrcMACAddr,MACADDRLEN);
			//hex_dump(&ptxinsn->mesh_header,sizeof(ptxinsn->mesh_header));
		
			if(memcmp(priv->root_mac, zeroAddr, MACADDRLEN) == 0) // doesn't has root info, run AODV routing protocol
			{
				static u8 chd[6] = {0};
				if(memcmp(chd, Mesh_dest, 6)) {
#if 0
					LOG_MESH_MSG("TX, no root info. run AODV to find %02X:%02X:%02X:%02X:%02X:%02X\n",
						Mesh_dest[0], Mesh_dest[1], Mesh_dest[2], Mesh_dest[3], Mesh_dest[4], Mesh_dest[5]);
#endif						
					memcpy(chd, Mesh_dest, 6);
				}
				do_aodv_routing(priv,skb, Mesh_dest);
				return 0;
			}
			else if(memcmp(priv->root_mac, GET_MY_HWADDR, MACADDRLEN) == 0) // i am root, but no path, fire aodv
			{
				static u8 chd[6] = {0};
				if(memcmp(chd, Mesh_dest, 6))
				{
#if 0	
					LOG_MESH_MSG("TX, root fire AODV to find %02X:%02X:%02X:%02X:%02X:%02X\n",
						Mesh_dest[0], Mesh_dest[1], Mesh_dest[2], Mesh_dest[3], Mesh_dest[4], Mesh_dest[5]);
#endif
					memcpy(chd, Mesh_dest, 6);
				}
				do_aodv_routing(priv, skb, Mesh_dest);
				return 0;
			} 
			else  // send to root
			{
				pEntry = pathsel_query_table( priv, priv->root_mac );
	  			if(pEntry != (struct path_sel_entry *)-1) { // has valid route path 
	  				memcpy(ptxinsn->nhop_11s, pEntry->nexthopMAC, MACADDRLEN);
	  				memcpy(skb->data, priv->root_mac, MACADDRLEN);
	  				memcpy(skb->data+MACADDRLEN, GET_MY_HWADDR, MACADDRLEN);
	  				ptxinsn->mesh_header.mesh_flag = 0x01;
					{
						static struct path_sel_entry *lst = NULL;
						if(lst != pEntry) {
#if 0
							unsigned char *Eth_dest = ptxinsn->mesh_header.DestMACAddr;
							LOG_MESH_MSG("TX utilize root to send pkt to %02X:%02X:%02X:%02X:%02X:%02X\n",
								Eth_dest[0], Eth_dest[1], Eth_dest[2], Eth_dest[3], Eth_dest[4], Eth_dest[5]);
#endif
							lst = pEntry;
						}
					}
				} else {// have no valid route path to root
//					LOG_MESH_MSG("TX tree-based routing error- doesn't know root path\n");
					dev_kfree_skb_any(skb);
					return 0;
				}
			}
		} // end of else (not have valid route path)
	} // end unicast

//txsc_path:
	return 1;
}

#ifdef  _11s_TEST_MODE_
int mesh_debug_tx1(struct net_device *dev, DRV_PRIV *priv, struct sk_buff *skb)
{
	if(!memcmp("XMT", priv->pmib->dot1180211sInfo.mesh_reservedstr1, 3) )
	{
		unsigned char srcMAC[6]= { 0 };
		mac12_to_6(priv->pmib->dot1180211sInfo.mesh_reservedstr1+3, srcMAC ); 

		if(!memcmp(skb->data+6, srcMAC, 6))
		{
			LOG_MESH_MSG("xmit:%02X %02X %02X %02X %02X %02X [ %02X %02X %02X %02X %02X %02X, %s,%d\n",
				skb->data[0]&0xff, skb->data[1]&0xff, skb->data[2]&0xff, skb->data[3]&0xff, skb->data[4]&0xff, skb->data[5]&0xff,
				skb->data[6]&0xff, skb->data[7]&0xff, skb->data[8]&0xff, skb->data[9]&0xff, skb->data[10]&0xff, skb->data[11]&0xff,
				dev->name, skb->len);

			if((skb->len>341 && skb->len<374)||(skb->len>586 && skb->len<601))
				printk("xmit:%02X %02X %02X %02X %02X %02X [ %02X %02X %02X %02X %02X %02X, %s,%d\n",
					skb->data[0]&0xff, skb->data[1]&0xff, skb->data[2]&0xff, skb->data[3]&0xff, skb->data[4]&0xff, skb->data[5]&0xff,
					skb->data[6]&0xff, skb->data[7]&0xff, skb->data[8]&0xff, skb->data[9]&0xff, skb->data[10]&0xff, skb->data[11]&0xff,
					dev->name, skb->len);
/*
			if(memcmp(skb->data+14, "\x00\x01\x08\x00\x06\x04\x00", 7)==0)
				printk("[%s]",skb->data[21]==2 ? "ARP RSP" : "arp req" ); 
			for(j=0; j<20; j++)
				printk("%02X ", skb->data[j+12]&0xff);
			printk("\n");*/
		}
	}

	if(!memcmp("JasonRelay", priv->pmib->dot1180211sInfo.mesh_reservedstr1, 10)
		&& memcmp(skb->data+14+2, "TEST_TRAFFIC", 12)==0 )
	{
		dev_kfree_skb_any(skb);
		return -1;
	}
	return 0;
}

int mesh_debug_tx2( DRV_PRIV *priv, struct sk_buff *skb)
{
	if(memcmp(skb->data, "**************", 14)==0  )
	{
		if(!memcmp("JasonSender", priv->pmib->dot1180211sInfo.mesh_reservedstr1, 11))
			issue_test_traffic(skb);
		else
			dev_kfree_skb_any(skb);
		return -1;
	}
	return 0;
}

int mesh_debug_tx3(struct net_device *dev, DRV_PRIV *priv, struct sk_buff *skb)
{
/*
	struct stat_info *pstat;
	struct list_head *phead = &priv->sleep_list;
	struct list_head *plist = phead->next;

	while(plist != phead)
	{
		pstat = list_entry(plist, struct stat_info, sleep_list);
		plist = plist->next;
		printMac(pstat->hwaddr);
		printk("... sleeping popen\n");
	}
	printHex(skb->data, 20);
	printk("\n...%d,%s\n", skb->len, dev->name);
*/
	return 0;
}

// multicast data frame use fixedTxRate in advanced setting page
int mesh_debug_tx4(DRV_PRIV *priv, struct tx_insn* txcfg)
{
	if(priv->pmib->dot11StationConfigEntry.autoRate==0 && priv->pmib->dot1180211sInfo.mesh_reserved2&4)
		txcfg->tx_rate = get_rate_from_bit_value(priv->pmib->dot11StationConfigEntry.fixedTxRate);
	return 0;
}


int mesh_debug_tx5(DRV_PRIV *priv, struct tx_insn* txcfg)
{
	if( priv->pmib->dot1180211sInfo.mesh_reserved2&16)
	{
		txcfg->tx_rate = (priv->pmib->dot1180211sInfo.mesh_reserved4&128) ? priv->pmib->dot1180211sInfo.mesh_reserved4 : (priv->pmib->dot1180211sInfo.mesh_reserved4 << 1);
		txcfg->fixed_rate = 1;
	}
	else if(priv->pmib->dot11StationConfigEntry.autoRate==0 && priv->pmib->dot1180211sInfo.mesh_reserved2&4)
	{
		txcfg->tx_rate = get_rate_from_bit_value(priv->pmib->dot11StationConfigEntry.fixedTxRate);
		txcfg->fixed_rate = 1;
	}
	return 0;
}

int mesh_debug_tx6(DRV_PRIV *priv, struct tx_insn* txcfg)
{
	if (priv->pmib->dot1180211sInfo.mesh_reserved2&8)
	{
		txcfg->lowest_tx_rate = txcfg->tx_rate = (priv->pmib->dot1180211sInfo.mesh_reserved4&128) ? priv->pmib->dot1180211sInfo.mesh_reserved4 : (priv->pmib->dot1180211sInfo.mesh_reserved4 << 1);
		txcfg->fixed_rate = 1;
	}
	else
	{
		if (IS_MCAST(GetAddr1Ptr(txcfg->phdr)))
		{
			if(priv->pmib->dot1180211sInfo.mesh_reserved2&256)
			{
				txcfg->lowest_tx_rate = txcfg->tx_rate = (priv->pmib->dot1180211sInfo.mesh_reserved4&128) ? priv->pmib->dot1180211sInfo.mesh_reserved4 : (priv->pmib->dot1180211sInfo.mesh_reserved4 << 1);
				txcfg->fixed_rate = 1;
			}
		}
		else
		{
			if(priv->pmib->dot1180211sInfo.mesh_reserved2&512)
			{
				txcfg->lowest_tx_rate = txcfg->tx_rate = (priv->pmib->dot1180211sInfo.mesh_reserved3&128) ? priv->pmib->dot1180211sInfo.mesh_reserved3 : (priv->pmib->dot1180211sInfo.mesh_reserved3 << 1);
				txcfg->fixed_rate = 1;
			}
		}
	}
	return 0;
}

int mesh_debug_tx7(DRV_PRIV *priv,  struct tx_desc *pdesc)
{
/* in 8190se, pdesc has no member named flen
	if( (!memcmp("JasonSender", priv->pmib->dot1180211sInfo.mesh_reservedstr1, 11) ||
			!memcmp("JasonRelay", priv->pmib->dot1180211sInfo.mesh_reservedstr1, 10) )
			&& (priv->pmib->dot1180211sInfo.mesh_reserved3 == (pdesc->flen & 0xfff) - _CRCLNG_ ) )
		priv->mesh_stats.tx_errors++;
*/
	return 0;
}
int mesh_debug_tx8(DRV_PRIV *priv,  struct tx_desc *pdesc)
{
/* in 8190se, pdesc has no member named flen
	if( (!memcmp("JasonSender", priv->pmib->dot1180211sInfo.mesh_reservedstr1, 11) ||
			!memcmp("JasonRelay", priv->pmib->dot1180211sInfo.mesh_reservedstr1, 10) )
			&& (priv->pmib->dot1180211sInfo.mesh_reserved3  == (pdesc->flen & 0xfff) - _CRCLNG_ ) )
		priv->mesh_stats.tx_bytes += retry;
*/
	return 0;
}
int mesh_debug_tx9(struct tx_insn* txcfg, struct tx_desc_info *pdescinfo)
{
	if( txcfg->is_11s & 32)
		pdescinfo->type =_RESERVED_FRAME_TYPE_;
	return 0;
}
int mesh_debug_tx10(struct tx_insn* txcfg, struct tx_desc_info *pndescinfo)
{
	if( txcfg->is_11s & 16)
		pndescinfo->type =_RESERVED_FRAME_TYPE_;
	return 0;
}

#endif // _11s_TEST_MODE_

#endif //  CONFIG_RTK_MESH
