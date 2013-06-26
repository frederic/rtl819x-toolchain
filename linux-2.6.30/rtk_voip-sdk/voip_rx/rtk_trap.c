/**************************************************
 * Realtek VoIP suite
 * Bruce (kfchang@realtek.com.tw)
 * ***********************************************/



#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/if_ether.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/inet.h>
#include <linux/version.h>

#include <asm/uaccess.h>

#include "rtk_voip.h"
#include "rtk_trap.h"
#include "voip_types.h"
//#include "../voip_manager/voip_mgr_netfilter.h"
#include "voip_init.h"
#include "voip_proc.h"

#ifdef CONFIG_RTK_VOIP_SRTP
#include "cp3_profile.h"
#include "srtp.h"
#endif

#ifdef T38_STAND_ALONE_HANDLER
//#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#include "../voip_drivers/t38_handler.h"
//#else
//#include "../voip_manager/voip_mgr_help_host.h"
//#endif
#endif

struct RTK_TRAP_profile *filter[MAX_RTP_TRAP_SESSION]={0};

#define VOIP_RX_IN_UDP // move rtk_trap() from /net/core/dev.c to /net/ipv4/udp.c

#if defined(VOIP_RX_IN_UDP)
int (*rtk_trap_hook)(struct sk_buff *skb) = NULL;
#endif

//int (*rtk_trap_hook)(struct sk_buff *skb) = NULL;
extern int (*rtk_trap_hook)(struct sk_buff *skb);
struct RTK_TRAP_profile *header;

#ifndef AUDIOCODES_VOIP
uint32 nRxRtpStatsCountByte[MAX_DSP_RTK_CH_NUM];
uint32 nRxRtpStatsCountPacket[MAX_DSP_RTK_CH_NUM];
uint32 nRxRtpStatsLostPacket[MAX_DSP_RTK_CH_NUM];
#endif

#ifdef SUPPORT_VOIP_DBG_COUNTER
extern uint32 gVoipCounterEnable;
extern void RTP_rx_count(uint32 sid);
#endif

#ifdef CONFIG_RTK_VOIP_SRTP
err_status_t status;

// For kernel SRTP test only
/*char key[30] = { 0, 1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,  \
		20, 21, 22, 23, 24, 25, 26, 27, 28, 29};*/
void srtp_polity_init(struct RTK_TRAP_profile *ptr)
{
        ptr->applySRTP = 1;
	// TX
	crypto_policy_set_rtp_default(&ptr->tx_policy.rtp);
	crypto_policy_set_rtcp_default(&ptr->tx_policy.rtcp);
	ptr->tx_policy.ssrc.type  = ssrc_any_outbound;
	ptr->tx_policy.key = (uint8_t *)ptr->local_inline_key;
	//ptr->tx_policy.key  = (uint8_t *) key;
	
	ptr->tx_policy.next = NULL;
	ptr->tx_policy.rtp.sec_serv = sec_serv_conf | sec_serv_auth;
	//ptr->tx_policy.rtcp.sec_serv = sec_serv_none;  /* we don't do RTCP anyway */
	ptr->tx_policy.rtcp.sec_serv = sec_serv_conf_and_auth;
	status = srtp_create(&ptr->tx_srtp_ctx, &ptr->tx_policy);
	if (status) {
		PRINT_R("error: srtp_create() failed with code %d\n",status);
	}

	// RX
	crypto_policy_set_rtp_default(&ptr->rx_policy.rtp);
	crypto_policy_set_rtcp_default(&ptr->rx_policy.rtcp);
	ptr->rx_policy.ssrc.type  = ssrc_any_inbound;
	ptr->rx_policy.key = (uint8_t *)ptr->remote_inline_key;
	//ptr->rx_policy.key  = (uint8_t *) key;
	
	ptr->rx_policy.next = NULL;
	ptr->rx_policy.rtp.sec_serv = sec_serv_conf | sec_serv_auth;
	//ptr->rx_policy.rtcp.sec_serv = sec_serv_none;  /* we don't do RTCP anyway */
	ptr->rx_policy.rtcp.sec_serv = sec_serv_conf_and_auth;
	status = srtp_create(&ptr->rx_srtp_ctx, &ptr->rx_policy);
    	if (status) {
		PRINT_R("error: srtp_create() failed with code %d\n",status);
	}	

}
#endif /* CONFIG_RTK_VOIP_SRTP */

int32 rtk_trap_register(TstVoipMgrSession *stVoipMgrSession, uint8 c_id, uint32 m_id, uint32 s_id, int32(*callback)(uint8 ch_id, int32 m_id, void *ptr_data, uint32 data_len, uint32 flags))
{

	struct RTK_TRAP_profile *ptr;
#if 0
	PRINT_MSG("ip_src_addr = %d\n", stVoipMgrSession->ip_src_addr);
	PRINT_MSG("ip_dst_addr = %d\n", stVoipMgrSession->ip_dst_addr);
	PRINT_MSG("udp_src_port = %d\n", stVoipMgrSession->udp_src_port);
	PRINT_MSG("udp_dst_port = %d\n", stVoipMgrSession->udp_dst_port);
	PRINT_MSG("protocol = %d\n", stVoipMgrSession->protocol);
#endif
#ifdef REG_INIT_SOCKET //for UDP socket send
	TstUDPSession stUDPSession;
	stUDPSession.ip_src_addr = stVoipMgrSession->ip_src_addr;
	stUDPSession.ip_dst_addr = stVoipMgrSession->ip_dst_addr;
	stUDPSession.udp_src_port= stVoipMgrSession->udp_src_port;
	stUDPSession.udp_dst_port= stVoipMgrSession->udp_dst_port;
	udp_socket_establish( &stUDPSession);

#endif
	
//#ifndef AUDIOCODES_VOIP
	if(s_id <0 || s_id > 2*DSP_SS_NUM)
//#else
//	if(s_id <0 || s_id > SESS_NUM)
//#endif
	{
		PRINT_R("s_id %d non-support now!(support 0~%d)\n", s_id, DSP_SS_NUM-1);
		return -1;
	}
	
	if(filter[s_id]!=0) {
		PRINT_R("s_id %d in used, please unregister first\n", s_id);
		return -1;
	}

	if(header == NULL)
	{
		PRINT_MSG("path 0\n");
		header=(struct RTK_TRAP_profile *) kmalloc(sizeof(struct RTK_TRAP_profile), GFP_ATOMIC);	
#ifdef CONFIG_RTK_VOIP_SRTP
		header->applySRTP=0;
#endif
		if(header == NULL)
		{
			PRINT_R("RTK TRAP allocate memory error !!\n");
			return -1;
		}
		header->ip_src_addr = stVoipMgrSession->ip_src_addr;
		header->ip_dst_addr = stVoipMgrSession->ip_dst_addr;
		header->udp_src_port = stVoipMgrSession->udp_src_port;
		header->udp_dst_port = stVoipMgrSession->udp_dst_port;
		header->protocol = stVoipMgrSession->protocol;
#ifdef SUPPORT_VOICE_QOS
		header->tos = stVoipMgrSession->tos;
#endif
		header->c_id = c_id;
		header->m_id = m_id;
		header->s_id = s_id;
		header->rx_packets = header->rx_bytes = 0;
		header->tx_packets = header->tx_bytes = 0;
		header->rtk_trap_callback=(void *)callback;
#ifdef CONFIG_RTK_VOIP_SRTP
		if(stVoipMgrSession->remoteCryptAlg == HMAC_SHA1){
			PRINT_MSG("+++++line:%d, function:%s+++++\n",__LINE__,__FUNCTION__);
			memcpy(header->local_inline_key, stVoipMgrSession->localSrtpKey, 30);
			memcpy(header->remote_inline_key, stVoipMgrSession->remoteSrtpKey, 30);
			srtp_polity_init(header);
	}
#endif		
		header->next=NULL;
		ptr = header;
	}	
	else
	{
		PRINT_MSG("path 1\n");
		ptr=header;

		while(ptr!=NULL && ptr->next!=NULL)
		{
			ptr=ptr->next;	
		}

		ptr->next=(struct RTK_TRAP_profile *)kmalloc(sizeof(struct RTK_TRAP_profile), GFP_ATOMIC);	
#ifdef CONFIG_RTK_VOIP_SRTP
		ptr->next->applySRTP=0;
#endif
		if(ptr->next == NULL) 
		{
			PRINT_R("RTK TRAP allocate memory error !!\n");
			return -1;
		}
		ptr->next->ip_src_addr = stVoipMgrSession->ip_src_addr;
		ptr->next->ip_dst_addr = stVoipMgrSession->ip_dst_addr;
		ptr->next->udp_src_port = stVoipMgrSession->udp_src_port;
		ptr->next->udp_dst_port = stVoipMgrSession->udp_dst_port;
		ptr->next->protocol = stVoipMgrSession->protocol;
#ifdef SUPPORT_VOICE_QOS
		ptr->next->tos = stVoipMgrSession->tos;
#endif
		ptr->next->c_id= c_id;
		ptr->next->m_id= m_id;
		ptr->next->s_id= s_id;
		ptr->next->rx_packets = ptr->next->rx_bytes = 0;
		ptr->next->tx_packets = ptr->next->tx_bytes = 0;
#ifdef CONFIG_RTK_VOIP_SRTP
		if(stVoipMgrSession->remoteCryptAlg == HMAC_SHA1){
			PRINT_MSG("+++++line:%d, function:%s+++++\n",__LINE__,__FUNCTION__);
			memcpy(ptr->next->local_inline_key, stVoipMgrSession->localSrtpKey, 30);
			memcpy(ptr->next->remote_inline_key, stVoipMgrSession->remoteSrtpKey, 30);
			srtp_polity_init(ptr->next);
		}
#endif		
		ptr->next->rtk_trap_callback=(void *)callback;
		ptr->next->next=NULL;

	}
	filter[s_id] = ptr;
	PRINT_MSG("establish complete\n");
	return 0;
}


int32 rtk_trap_unregister(uint32 s_id)
{
	struct RTK_TRAP_profile *ptr, *ptr1;

	ptr=header;
	ptr1=header;

//#ifndef AUDIOCODES_VOIP
	if(s_id <0 || s_id > 2*DSP_SS_NUM)
//#else
//	if(s_id <0 || s_id > SESS_NUM)
//#endif
	{
		PRINT_MSG("s_id %d non-support now!(support 0~%d)\n", s_id, DSP_SS_NUM-1);
		return -1;
	}

	if(filter[s_id]==0) {
		PRINT_MSG("s_id %d non-used now, can't unregister\n", s_id);
		return -1;
	}
	while(ptr != NULL) {
		if(ptr->s_id==s_id) {
			filter[s_id]=0;
			if(ptr!=header) {
				ptr1->next=ptr->next;
			} else {
				header=header->next;
			}
#ifdef CONFIG_RTK_VOIP_SRTP
			if( 1 == ptr->applySRTP){
				status = srtp_dealloc(ptr->rx_srtp_ctx);
		    		if (status) {
					PRINT_MSG("error: srtp_dealloc() failed with code %d\n",status);
				}	
				status = srtp_dealloc(ptr->tx_srtp_ctx);
		    		if (status) {
					PRINT_MSG("error: srtp_dealloc() failed with code %d\n",status);
				}	
				memset(ptr, 0, sizeof(ptr));
			}
#endif			
			kfree(ptr);
			return 0;
		}
		ptr1 = ptr;
		ptr = ptr->next;
	}
	PRINT_MSG("no found\n");
	return -1;
}

int get_filter(uint8 ch_id, struct RTK_TRAP_profile *myptr)
{
	struct RTK_TRAP_profile *ptr;

        ptr=header;
	while( ptr !=NULL )
        {
                if(ptr->c_id!=ch_id){
                  ptr = ptr->next;
                }
		myptr = ptr;
                return 0;
        }
	return -1;
}

inline int rtk_trap_callback_trap( const struct RTK_TRAP_profile *ptr, 
								   const struct sk_buff *skb )
{
#ifdef T38_STAND_ALONE_HANDLER
	if( t38RunningState[ ptr->c_id ] == T38_START &&
		ptr ->s_id < RTCP_SID_OFFSET )
	{
	#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
		struct iphdr * const iph = ip_hdr(skb);
		const uint32 udp_len = (iph->tot_len) - 28;
	#else
		const uint32 udp_len = ((skb->nh.iph)->tot_len) - 28;
	#endif
		unsigned char * const udp_data = skb->data + sizeof(struct udphdr);
		
		const uint32 chid = ptr->c_id;
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
		const uint32 sid = ptr->s_id;
#endif
		
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
		extern void ipc_Host2DSP_T38Tx( uint32 chid_pkt, uint32 sid_pkt, void* packet, uint32 pktLen );

		ipc_Host2DSP_T38Tx( chid, sid, udp_data, udp_len );
#else
	#if 1
		T38_API_PutPacket( chid, udp_data, udp_len );
	#else
    #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
		struct iphdr *iph = ip_hdr(skb);
		T38_API_PutPacket( ptr->c_id, skb->data + sizeof(struct udphdr), (iph->tot_len) - 28 );
    #else
		T38_API_PutPacket( ptr->c_id, skb->data + sizeof(struct udphdr), ((skb->nh.iph)->tot_len) - 28 );
    #endif
    #endif
#endif
		
		return 1;
	}
#endif /* T38_STAND_ALONE_HANDLER */

	return 0;
}

/****************************************************************/
int rtk_trap(struct sk_buff *skb)
{
	struct RTK_TRAP_profile *ptr;
	//struct preprocess_result *rst;
	//struct net_device *rx_dev;

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	struct udphdr *uh;
	struct iphdr *iph;
#endif
	uint32 src_ip;
	uint32 dst_ip;
	uint16 src_port;
	uint16 dst_port;
	uint8 proto;
	//Tuint16 tmp;

	//PRINT_MSG("skb enter rtk_trap= %x\n",skb);
#ifndef VOIP_RX_IN_UDP
	skb->mac.ethernet=((void *)skb->data -14);
#endif	
	//rx_dev = skb->dev;
	ptr = header;
	while(ptr!=NULL) 
	{
	  //PRINT_MSG("packet enter filter\n");
#ifndef VOIP_RX_IN_UDP
	  tmp = ntohs((skb->mac.ethernet)->h_proto);
	  if ( 0x0800 != tmp ) {
		  //PRINT_MSG("h_proto = %x\n",tmp);
	    break;
	  }

	  skb->nh.iph=(void *)((skb->data));
#endif	  
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	iph = ip_hdr(skb);
	proto = iph->protocol;
	src_ip = iph->saddr;
	dst_ip = iph->daddr;
#else
	  proto = (skb->nh.iph)->protocol;
	  src_ip = ((skb->nh.iph)->saddr);
	  dst_ip = ((skb->nh.iph)->daddr);
#endif

#ifndef RTP_SRC_IP_CHECK
	  if ( dst_ip != ptr->ip_dst_addr ||
	       proto != ptr->protocol
	     )
#else
	  if ( src_ip != ptr->ip_src_addr ||
	       dst_ip != ptr->ip_dst_addr ||
	       proto != ptr->protocol
	     )
#endif
	  {
	    //PRINT_MSG("src_ip = %x\n",src_ip);
	    //PRINT_MSG("dst_ip = %x\n",dst_ip);
	    //PRINT_MSG("proto = %x\n",proto);
	    ptr=ptr->next;
	    continue;
	  }
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	uh = udp_hdr(skb);
	src_port =(uint16) (uh->source);
	dst_port =(uint16) (uh->dest);
#else
	  skb->h.uh=(struct udphdr *)((char *)(skb->nh.iph)+20);
	  src_port =(uint16) ((skb->h.uh)->source);
	  dst_port =(uint16) ((skb->h.uh)->dest);
#endif
#ifndef RTP_SRC_PORT_CHECK
	  if ( dst_port != ptr->udp_dst_port )
#else /*RTP_SRC_PORT_CHECK*/
	  if ( src_port != ptr->udp_src_port ||
	       dst_port != ptr->udp_dst_port 
	     )
#endif /*RTP_SRC_PORT_CHECK*/
	  {
	    //PRINT_MSG("src_port = %x\n",src_port);
	    //PRINT_MSG("dst_port = %x\n",dst_port);
	    //PRINT_MSG("src_port = %x\n",ptr->udp_src_port);
	    ptr=ptr->next;
	    continue;
	  }
	  
	  ptr ->rx_packets ++;
  #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	  ptr ->rx_bytes += (iph->tot_len) - 28;	/* UDP payload is excluded */
  #else
	  ptr ->rx_bytes += ((skb->nh.iph)->tot_len) - 28;	/* UDP payload is excluded */
  #endif
		  
#ifndef AUDIOCODES_VOIP
  #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	  nRxRtpStatsCountByte[ ptr->c_id ] += (iph->tot_len) - 28;	/* UDP payload is excluded */
  #else
	  nRxRtpStatsCountByte[ ptr->c_id ] += ((skb->nh.iph)->tot_len) - 28;	/* UDP payload is excluded */
  #endif
	  nRxRtpStatsCountPacket[ ptr->c_id ] ++;
#ifdef SUPPORT_VOIP_DBG_COUNTER
	if (gVoipCounterEnable)
		RTP_rx_count(ptr->s_id);
#endif
#endif
	  
	  //PRINT_MSG("RTK TRAP catch one packet!!\n");
	  //rst = (struct preprocess_result *)preprocess(skb);
	  //PRINT_MSG("CH = %x\n");
	  //ptr->rtk_trap_callback(CH, 0, rst->ptr_data, rst->data_len, 0);
#if 1
	  /* thlin: use sid as input to call-back function, so the call-back function DSP_pktRx() in dsp_main.c gets sid instead of mid.*/
#ifdef VOIP_RX_IN_UDP	  
  #ifdef CONFIG_RTK_VOIP_SRTP
	  void *srtp_hdr = skb->data + sizeof(struct udphdr);
    #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	  int pkt_octet_len = (iph->tot_len) - 28;
    #else
	  int pkt_octet_len = ((skb->nh.iph)->tot_len) - 28;
    #endif
    #ifdef FEATURE_COP3_PROFILE	  
	  unsigned long flags;
	  save_flags(flags); cli();
	  ProfileEnterPoint(PROFILE_INDEX_TEMP0);
    #endif	  
	  /* apply srtp */
	  if (ptr->applySRTP){
    	#ifndef AUDIOCODES_VOIP
          	extern int rtcp_sid_offset;
          	//if((dst_port == 9001) || (dst_port == 9003) || (dst_port == 9005) || (dst_port == 9007))
          	if(ptr->s_id >= rtcp_sid_offset)
	#else
		if(((ptr->s_id)%2) == 1 ) // ACMW RTP sid = 2*CH, RTCP sid = 2*CH + 1
	#endif
          		status = srtp_unprotect_rtcp(ptr->rx_srtp_ctx,srtp_hdr,&pkt_octet_len);
		else
			status = srtp_unprotect(ptr->rx_srtp_ctx, srtp_hdr, &pkt_octet_len);
	  }
    #ifdef FEATURE_COP3_PROFILE	  
	  ProfileExitPoint(PROFILE_INDEX_TEMP0);
	  restore_flags(flags);
	  ProfilePerDump(PROFILE_INDEX_TEMP0, 1000);
    #endif	  
	  if (status) {
	  	PRINT_MSG(":src:%d-dest:%d\n",src_port,dst_port);
	    PRINT_MSG("error: srtp unprotection failed with code %d%s\n", status,
		    status == err_status_replay_fail ? " (replay check failed)" :
		    status == err_status_auth_fail ? " (auth check failed)" : "");	  
	  } else {	
    #if ! defined (AUDIOCODES_VOIP)		
		if( !rtk_trap_callback_trap( ptr, skb ) ) //  ??? bad packet ptr/size??
		#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	 		ptr->rtk_trap_callback(ptr->c_id, ptr->s_id, srtp_hdr, pkt_octet_len, iph->ttl);
		#else
	 		ptr->rtk_trap_callback(ptr->c_id, ptr->s_id, srtp_hdr, pkt_octet_len, skb->nh.iph->ttl);
		#endif
    #else
		if( !rtk_trap_callback_trap( ptr, skb ) ) // ??? bad packet ptr/size??
		#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
			ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, srtp_hdr, pkt_octet_len, iph->ttl); //TODO yg
		#else
			ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, srtp_hdr, pkt_octet_len, skb->nh.iph->ttl); //TODO yg
		#endif
    #endif
	  }
  #else // !CONFIG_RTK_VOIP_SRTP
    #if ! defined (AUDIOCODES_VOIP)
	  if( !rtk_trap_callback_trap( ptr, skb ) ) //check for T.38 stand alone
      #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->s_id, skb->data + sizeof(struct udphdr), (iph->tot_len) - 28, iph->ttl);
      #else
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->s_id, skb->data + sizeof(struct udphdr), ((skb->nh.iph)->tot_len) - 28, skb->nh.iph->ttl);
      #endif
    #else
	  if( !rtk_trap_callback_trap( ptr, skb ) ) //check for T.38 stand alone
      #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, skb->data + sizeof(struct udphdr), (iph->tot_len) - 28, iph->ttl); //TODO yg
      #else
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, skb->data + sizeof(struct udphdr), ((skb->nh.iph)->tot_len) - 28, skb->nh.iph->ttl); //TODO yg
      #endif
    #endif
  #endif // CONFIG_RTK_VOIP_SRTP
#else // !VOIP_RX_IN_UDP
    #if ! defined (AUDIOCODES_VOIP)
	  if( !rtk_trap_callback_trap( ptr, skb ) )  ??? bad packet ptr/size??
      #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	    ptr->rtk_trap_callback(ptr->c_id, ptr->s_id, skb->data + 28, (iph->tot_len) - 28, iph->ttl);
      #else
	    ptr->rtk_trap_callback(ptr->c_id, ptr->s_id, skb->data + 28, ((skb->nh.iph)->tot_len) - 28, skb->nh.iph->ttl);
      #endif
    #else
	  if( !rtk_trap_callback_trap( ptr, skb ) )  ??? bad packet ptr/size??
      #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, skb->data + 28, (iph->tot_len) - 28, iph->ttl); //TODO yg
      #else
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, skb->data + 28, ((skb->nh.iph)->tot_len) - 28, skb->nh.iph->ttl); //TODO yg
      #endif
    #endif
#endif // VOIP_RX_IN_UDP	  
#else
	  if( !rtk_trap_callback_trap( ptr, skb ) )  ??? bad packet ptr/size??
    #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, skb->data + 28, (iph->tot_len) - 28, iph->ttl);
    #else
	  	ptr->rtk_trap_callback(ptr->c_id, ptr->m_id, skb->data + 28, ((skb->nh.iph)->tot_len) - 28, skb->nh.iph->ttl);
    #endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)
	  skb_unlink(skb);
#endif
	  kfree_skb(skb);
	  //dev_put(rx_dev);	
	  return RTK_TRAP_ACCEPT;
	}
	return RTK_TRAP_NONE;	
}	


#ifndef AUDIOCODES_VOIP
void ResetRtpStatsCount( uint32 chid )
{
	extern uint32 nTxRtpStatsCountByte[MAX_DSP_RTK_CH_NUM];
	extern uint32 nTxRtpStatsCountPacket[MAX_DSP_RTK_CH_NUM];
	
	if( chid >= DSP_RTK_CH_NUM )
		return;

	/* Rx RTP statistics */
	nRxRtpStatsCountByte[ chid ] = 0;
	nRxRtpStatsCountPacket[ chid ] = 0;

	/* Tx RTP statistics */
	nTxRtpStatsCountByte[ chid ] = 0;
	nTxRtpStatsCountPacket[ chid ] = 0;

	/* Rx RTP lost */
	nRxRtpStatsLostPacket[ chid ] = 0;
}
#endif

int __init rtk_trap_init_module(void)
{
	extern int (*udp_rtk_trap_hook)(struct sk_buff *skb);
	extern void **udp_rtk_trap_profile_header;
#ifndef AUDIOCODES_VOIP
#if !defined( CONFIG_RTK_VOIP_IPC_ARCH ) || defined( CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP )
	extern void ResetSessionRxStatistics( uint32 sid );
	extern void ResetSessionTxStatistics( uint32 sid );
#endif
#endif

	int i;
	
#if 0
	PRINT_MSG("============= RTK VoIP SUITE ============\n");	
	PRINT_MSG("INITIAL RTP TRAP\n");
	PRINT_MSG("=========================================\n");    	
#endif
	//PRINT_MSG("1.rtk_trap_hook = %x\n", rtk_trap_hook);
	rtk_trap_hook = rtk_trap;
	header = NULL;

	udp_rtk_trap_hook = rtk_trap;
	udp_rtk_trap_profile_header = (void*)&header;	
	
	//PRINT_MSG("2. rtk_trap_hook = %x\n", rtk_trap_hook);

#ifdef CONFIG_RTK_VOIP_SRTP

#ifdef CONFIG_RTK_VOIP_SRTP_HW
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
#ifndef CONFIG_IPSEC
	rtl8186_crypto_init(); 	
#endif	
#endif
#endif // CONFIG_RTK_VOIP_SRTP_HW

  	/* initialize srtp library */
  	status = srtp_init();
  	if (status) {
    		PRINT_MSG("error: srtp initialization failed with error code %d\n", status);
	}	
#endif	

#ifndef AUDIOCODES_VOIP
	for( i = 0; i < DSP_RTK_CH_NUM; i ++ )
		ResetRtpStatsCount( i );
#endif

#ifndef AUDIOCODES_VOIP
#if !defined( CONFIG_RTK_VOIP_IPC_ARCH ) || defined( CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP )
	for( i = 0; i < DSP_RTK_SS_NUM; i ++ ) {
		ResetSessionRxStatistics( i );
		ResetSessionTxStatistics( i );
	}
#endif
#endif
	
	return 0;
}	


void __exit rtk_trap_cleanup_module(void)
{
	rtk_trap_hook = NULL;
	header = NULL;

	PRINT_MSG("============= RTK VoIP SUITE ============\n");	
        PRINT_MSG("Remove RTK TRAP\n");
        PRINT_MSG("=========================================\n");          

}

#ifndef CONFIG_RTK_VOIP_MODULE
voip_initcall(rtk_trap_init_module);
voip_exitcall(rtk_trap_cleanup_module);
#endif

int voip_trap_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int n;

	struct RTK_TRAP_profile *ptr;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	n = sprintf( buf, "src			dst	proto	cid	mid	sid	rx		tx\n" );
	
	ptr = header;
	
	while( ptr ) {		
		n += sprintf( buf + n, "%u.%u.%u.%u:%u\t", 
								( ptr ->ip_src_addr >> 24 ) & 0xFF, 
								( ptr ->ip_src_addr >> 16 ) & 0xFF, 
								( ptr ->ip_src_addr >> 8  ) & 0xFF, 
								( ptr ->ip_src_addr >> 0  ) & 0xFF, 
								ptr ->udp_src_port );
		n += sprintf( buf + n, "%u.%u.%u.%u:%u\t", 
								( ptr ->ip_dst_addr >> 24 ) & 0xFF,  
								( ptr ->ip_dst_addr >> 16 ) & 0xFF,  
								( ptr ->ip_dst_addr >> 8  ) & 0xFF,  
								( ptr ->ip_dst_addr >> 0  ) & 0xFF,  
								ptr ->udp_dst_port );
		n += sprintf( buf + n, "%u\t", ptr ->protocol );
		n += sprintf( buf + n, "%u\t%u\t%u\t", ptr ->c_id, ptr ->m_id, ptr ->s_id );
		n += sprintf( buf + n, "%u(%u)\t%u(%u)\n", 
								ptr ->rx_bytes, ptr ->rx_packets, 
								ptr ->tx_bytes, ptr ->tx_packets );
	
		ptr = ptr ->next;
	}
	
	*eof = 1;
	return n;
}

int voip_rtpstats_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
#ifndef AUDIOCODES_VOIP
	extern uint32 nTxRtpStatsCountByte[MAX_DSP_RTK_CH_NUM];
	extern uint32 nTxRtpStatsCountPacket[MAX_DSP_RTK_CH_NUM];

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	extern RtpSessionState sessionState[MAX_DSP_RTK_SS_NUM];
	
	int SessNum;
	int sid;
	int i = 0;
#endif

	int ch;
	int n = 0;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	if( IS_CH_PROC_DATA( data ) ) {
		ch = CH_FROM_PROC_DATA( data );
		n = sprintf( buf, "channel=%d\n", ch );
		/* Rx RTP statistics */
		n += sprintf( buf + n, "RxByte: %u\n", nRxRtpStatsCountByte[ ch ] );
		n += sprintf( buf + n, "RxPacket: %u\n", nRxRtpStatsCountPacket[ ch ] );
		
		/* Tx RTP statistics */
		n += sprintf( buf + n, "TxByte: %u\n", nTxRtpStatsCountByte[ ch ] );
		n += sprintf( buf + n, "TxPacket: %u\n", nTxRtpStatsCountPacket[ ch ] );
		
		/* Rx RTP lost */
		n += sprintf( buf + n, "RxLostPacket: %u\n", nRxRtpStatsLostPacket[ ch ] );
		
#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
		SessNum = chanInfo_GetRegSessionNum(ch);
	
		for(i=0; i<SessNum; i++)
		{
			sid = chanInfo_GetRegSessionID(ch, i);
			if (sessionState[sid] == rtp_session_sendrecv)
				n += sprintf( buf + n, "RTP session %d state: send_receive\n", sid);
			else if (sessionState[sid] == rtp_session_recvonly)
				n += sprintf( buf + n, "RTP session %d state: receive only\n", sid);
			else if (sessionState[sid] == rtp_session_sendonly)
				n += sprintf( buf + n, "RTP session %d state: send only\n", sid);
			else if (sessionState[sid] == rtp_session_inactive)
				n += sprintf( buf + n, "RTP session %d state: inactive\n", sid);
		}
#endif
	} else {
		//ss = SS_FROM_PROC_DATA( data );
		//n = sprintf( buf, "session=%d\n", ss );
	}
	
	*eof = 1;
	return n;
#else
	return 0;
#endif
}

int __init voip_trap_proc_init( void )
{
	create_proc_read_entry( PROC_VOIP_DIR "/trap", 0, NULL, voip_trap_read_proc, NULL );
	
	create_voip_channel_proc_read_entry( "rtpstats", voip_rtpstats_read_proc );
	
	return 0;
}

void __exit voip_trap_proc_exit( void )
{
	remove_voip_proc_entry( PROC_VOIP_DIR "/trap", NULL );
	
	remove_voip_channel_proc_entry( "rtpstats" );
}

voip_initcall_proc( voip_trap_proc_init );
voip_exitcall( voip_trap_proc_exit );

