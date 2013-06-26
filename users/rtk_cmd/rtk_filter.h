#ifndef _RTK_FILTER_H
#define _RTK_FILTER_H
/************************************************************/
//FILTER
#define ETH_ALEN 6
#define MAX_URL_KEY_LEN		64
//add/delete/flush/enableLog/disableLog 
#define FILTER_ADD 		1
#define FILTER_DEL 		2
#define FILTER_FLUSH 	3
#define FILTER_EN_LOG 	4
#define FILTER_DIS_LOG 	5
#define FILTER_SHOW 	6

typedef struct filter_flag{
	unsigned int ip_filter:4;			/*0:no ip filter, bit0~1: source ip filter, bit0:1 source ip filter, bit1:1 ip range filter
											bit2~bit3: destination ip filter, bit2:1 destination ip filter, bit3:1 ip range filter*/
	unsigned int port_filter:4;		/*0:no port filter, bit0~1: source port filter, bit0:1 source port filter, bit1:1 port range filter
											bit2~bit3: destination port filter, bit2:1 destination port filter, bit3:1 dst port range filter*/
	unsigned int mac_filter:4;			/*0:no mac filter, bit0~1: source mac filter, bit0:1 is source mac filter, bit1: mac range filter
											bit2~bit3: destination mac filter, bit2:1 destination mac filter, bit3:1 mac range filter*/
	unsigned int protocol_filter:2;	/*0:no protocol filter, bit0:1 tcp filter, bit1:udp filter*/														
	unsigned int url_filter:2;			/*0:no url filter, 1:match in url 2:url_key is part of url, 3:char match*/
	unsigned int src_phy_port_filter:8;
	unsigned int schedule_filter:2;	/*0:no schedule filter, 1:forever filter, 2:filter accord to timer*/
	unsigned int priority:3;	
	unsigned int action:3;				/*0:drop, 1:fastpath, 2:linux_protocol_stack, 3:mark, 4:finish 5:user_register*/
	unsigned int mark;
}filter_flag_s;

typedef struct mac_mask_filter{
	unsigned char mac[ETH_ALEN];
	unsigned char mask[ETH_ALEN];
}mac_filter_s, *mac_filter_p;

typedef struct mac_range_filter{
	unsigned char smac[ETH_ALEN];
	unsigned char emac[ETH_ALEN];
}mac_range_s, *mac_range_p;

typedef struct ip_mask_filter{
	struct in_addr	addr;
	struct in_addr	mask;
}ip_filter_s;

typedef struct ip_range_filter{
	struct in_addr	saddr;
	struct in_addr	eaddr;
}ip_range_filter_s;

typedef struct port_filter{
	unsigned int sport;
	unsigned int eport;
}port_filter_s;
typedef struct schedule_filter{
	unsigned char	day_mask;
	unsigned char	all_hours;
	unsigned int	stime;
	unsigned int	etime;
}schedule_filter_s, *schedule_filter_p;

typedef struct url_filter{
	unsigned char  url_key[MAX_URL_KEY_LEN];
}url_filter_s, *url_filter_p;

typedef struct tcp_ip_filter_ap{
	union	{
		mac_filter_s	mac_mask;
		mac_range_s 	mac_range;	
	}nf_mac;
	union {
		ip_filter_s 		ip_mask;
		ip_range_filter_s	ip_range;
	}nf_ip;
	port_filter_s	nf_port;	
}tcp_ip_filter_ap_s, *tcp_ip_filter_ap_p;
typedef struct filter_rule_ap{
	filter_flag_s		nf_flag;
	tcp_ip_filter_ap_s		nf_src_filter;	/*include the src MAC, src IP and src port*/
	tcp_ip_filter_ap_s		nf_dst_filter;	/*include the dst MAC, dst IP and dst port*/
	schedule_filter_s	nf_schedule;	
	url_filter_s		nf_url;
}filter_rule_ap_s, *filter_rule_ap_p;

typedef struct filter_info{
	int action;
	filter_rule_ap_s  filter_data;
}filter_info_s, *filter_info_p;

#define FILTER_MAX_PAYLOAD 1024 /* maximum payload size*/
struct filter_respond_struct
{
	int flag;
	char data[MAX_PAYLOAD];
};

int rtk_filter_rule_transform(char *user_rule, filter_rule_ap_p fastpath_filter_rule);
int rtk_filter_parse(int _num, char* _param[]);
#endif
