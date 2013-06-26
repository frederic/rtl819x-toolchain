#ifndef RTK_TRAP_H
#define RTK_TRAP_H

#include <linux/skbuff.h> /* for struct sk_buff */
#include <linux/in.h> /* for struct sockaddr_in*/

#include "../include/voip_types.h"
#include "../voip_manager/voip_mgr_netfilter.h"

#ifdef CONFIG_RTK_VOIP_SRTP
#include "srtp.h"
#endif

enum 
{
	RTK_TRAP_ACCEPT=0,	//done
	RTK_TRAP_CONTINUE,	//continue to do this packet
	RTK_TRAP_DROP,		//drop the packet
	RTK_TRAP_NONE		//send to upper layer
};

struct RTK_TRAP_profile
{
	uint32 ip_src_addr;
	uint32 ip_dst_addr;
	uint16 udp_src_port;
	uint16 udp_dst_port;
	uint8 protocol; 	//tcp or udp
	uint8 c_id;
	uint32 m_id;
	uint32 s_id;
	uint32 rx_packets;
	uint32 rx_bytes;
	uint32 tx_packets;
	uint32 tx_bytes;
	struct socket *serv_sock;
	struct sockaddr_in serv;
	int32 (*rtk_trap_callback)(uint8 ch_id, uint32 media_type, void *ptr_data, uint32 data_len, uint32 flags);
	/* P.S. rtk_trap_callback -> dsp_main.c: DSP_pktRx() */
	struct net_device *dev;
	struct RTK_TRAP_profile *next;	

#ifdef CONFIG_RTK_VOIP_SRTP
	// NOTE: Only "SUPPORT_RTCP disabled" is implemented now.
	uint32 applySRTP;     // we shall apply SRTP to RTP packets only. Not including T.38 packets!
	
	uint32 crypto_type; // cipher type/key_len and auth type/key_len/tag_len
	uint8 local_inline_key[30]; //16-byte (128 bit) master key with a 14-byte salting key
	uint8 remote_inline_key[30]; //16-byte (128 bit) master key with a 14-byte salting key
	srtp_policy_t tx_policy;
	srtp_policy_t rx_policy;	
	srtp_ctx_t *tx_srtp_ctx;
	srtp_ctx_t *rx_srtp_ctx;
#endif

#ifdef SUPPORT_VOICE_QOS
	uint8 tos;	
#endif
};
//extern int32 rtk_trap_register(TstVoipMgrSession *stVoipMgrSession, uint8 ch_id, int32 m_id, int32(*callback)(uint8t ch_id, int32 m_id, void *ptr_data, uint32 data_len, uint32 flags));
extern int32 rtk_trap_register(TstVoipMgrSession *stVoipMgrSession, uint8 c_id, uint32 m_id, uint32 s_id, int32(*callback)(uint8 ch_id, int32 m_id, void *ptr_data, uint32 data_len, uint32 flags));

//extern int32 rtk_trap_unregister(uint8 ch_id);
extern int32 rtk_trap_unregister(uint32 s_id);
extern int get_filter(uint8 ch_id, struct RTK_TRAP_profile *myptr);

#endif
