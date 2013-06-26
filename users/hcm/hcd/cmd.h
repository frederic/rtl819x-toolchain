/*
  *   cmd header for 8198
  *
  *	Copyright (C)2008, Realtek Semiconductor Corp. All rights reserved.
  *
  *	$Id: cmd.h,v 1.5 2010/12/29 09:58:22 brian Exp $
  */

#ifndef INCLUDE_CMD_H
#define INCLUDE_CMD_H

// command ID
#define id_set_mib						0x01
#define id_get_mib						0x02
#define id_sysinit							0x03
#define id_getstainfo						0x0a
#define id_getassostanum					0x0b
#define id_getbssinfo						0x06
#define id_getstats						0x07
#define id_getlanstatus					0x08
#define id_trigger_wps					0x09
#define id_ioctl						0x05
#define id_syscall						0x04
#define id_firm_upgrade						0x20

#define id_set_mibs						0x1a


#define LAST_ENTRY_ID 0 //LAST_ENTRY_ID

#define MAX_HOST_CMD_LEN  65536

#define CMD_DEF(name, func) \
	{id_##name, #name,  func}

// CMD table for action
struct cmd_entry {                          
	int 		id;				// cmd id
	char 	*name;		// cmd name in string                 			
	int		 (*func)(char *data,int len);   //the fuction for act 
};           	


struct user_net_device_stats {
    unsigned long long rx_packets;	/* total packets received       */
    unsigned long long tx_packets;	/* total packets transmitted    */
    unsigned long long rx_bytes;	/* total bytes received         */
    unsigned long long tx_bytes;	/* total bytes transmitted      */
    unsigned long rx_errors;	/* bad packets received         */
    unsigned long tx_errors;	/* packet transmit problems     */
    unsigned long rx_dropped;	/* no space in linux buffers    */
    unsigned long tx_dropped;	/* no space available in linux  */
    unsigned long rx_multicast;	/* multicast packets received   */
    unsigned long rx_compressed;
    unsigned long tx_compressed;
    unsigned long collisions;

    /* detailed rx_errors: */
    unsigned long rx_length_errors;
    unsigned long rx_over_errors;	/* receiver ring buff overflow  */
    unsigned long rx_crc_errors;	/* recved pkt with crc error    */
    unsigned long rx_frame_errors;	/* recv'd frame alignment error */
    unsigned long rx_fifo_errors;	/* recv'r fifo overrun          */
    unsigned long rx_missed_errors;	/* receiver missed packet     */
    /* detailed tx_errors */
    unsigned long tx_aborted_errors;
    unsigned long tx_carrier_errors;
    unsigned long tx_fifo_errors;
    unsigned long tx_heartbeat_errors;
    unsigned long tx_window_errors;
};

#define RTL819X_IOCTL_READ_PORT_STATUS			(SIOCDEVPRIVATE + 0x01)	
#define RTL819X_IOCTL_READ_PORT_STATS	              (SIOCDEVPRIVATE + 0x02)	
#define _PATH_PROCNET_DEV	"/proc/net/dev"

struct lan_port_status {
    unsigned char link;
    unsigned char speed;
    unsigned char duplex;
    unsigned char nway;    	
}; 

struct port_statistics {
	unsigned int  rx_bytes;
	unsigned int  rx_unipkts;
	unsigned int  rx_mulpkts;
	unsigned int  rx_bropkts;
	unsigned int  rx_discard;
	unsigned int  rx_error;
	unsigned int  tx_bytes;
	unsigned int  tx_unipkts;
	unsigned int  tx_mulpkts;
	unsigned int  tx_bropkts;
	unsigned int  tx_discard;
	unsigned int  tx_error;
};

/*
typedef struct wlan_rate{
	unsigned int id;
	unsigned char rate[20];
}WLAN_RATE_T, *WLAN_RATE_Tp;
*/

int do_cmd(int id , char *cmd ,int cmd_len ,int relply);
int do_action(int id , char *cmd ,int cmd_len ,int relply);


#endif 
