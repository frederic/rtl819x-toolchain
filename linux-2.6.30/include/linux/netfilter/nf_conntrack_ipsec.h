#ifndef _NF_CONNTRACK_IPSEC_H
#define _NF_CONNTRACK_IPSEC_H
#ifdef __KERNEL__

#define MAX_PORTS 8

#define IPSEC_PORT      500
#define IPPROTO_ESP     0x32
#define IPSEC_FREE      0
#define IPSEC_USED      1
#define IPSEC_IDLE_TIME 600
#define MaxSession      16

struct isakmp_data_s {
	u_int64_t	icookie;	/* initiator cookie */
	u_int64_t	rcookie;	/* responder cookie */

	u_int32_t peer_ip;
	u_int32_t local_ip;
	u_int32_t alias_ip;

	u_int32_t	ospi;
	u_int32_t	ispi;

	u_int16_t state;
	u_int16_t idle_timer;

	struct nf_conn	*pctrack;
};


extern unsigned int (*nf_nat_ipsec_inbound_hook)
				(struct sk_buff *skb,
				struct nf_conn *ct,
				 enum ip_conntrack_info ctinfo,
				 struct nf_conntrack_expect *exp);

extern unsigned int (*nf_nat_ipsec_outbound_hook)
				(struct sk_buff *skb,
				struct nf_conn *ct,
				 enum ip_conntrack_info ctinfo,
				 struct nf_conntrack_expect *exp);

extern unsigned int (*nf_nat_esp_hook)
				(struct sk_buff *skb,
				struct nf_conn *ct,
				 enum ip_conntrack_info ctinfo,
				 struct nf_conntrack_expect *exp);

#endif /* __KERNEL__ */
#endif /* _NF_CONNTRACK_IPSEC_H */
