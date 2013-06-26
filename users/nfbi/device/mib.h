/*
  *   MIB header file for RTL8197B
  *
  *	Copyright (C)2008, Realtek Semiconductor Corp. All rights reserved.
  *
  *	$Id: mib.h,v 1.9 2009/04/15 03:50:48 michael Exp $
  */

#ifndef INCLUDE_MIB_H
#define INCLUDE_MIB_H

/*================================================================*/
/* Constant Definitions */

#define MAX_PSK_LEN				(64+1)
#define MAX_WEP_KEY_LEN	13

// command ID
#define id_cmd_timeout						0x00
#define id_mii_pause_enable   		        0x01
#define id_eth_pause_enable   		        0x02
#define id_cpu_suspend_enable             	0x03
#define id_phy_reg_poll_time			    0x04
#define id_write_memory					    0x05
#define id_read_memory					    0x06
#define id_fw_version				        0x07
#define id_wlan_mac_addr				    0x08
#define id_wlan_link_down_time		        0x10
#define id_channel							0x11
#define id_ssid								0x12
#define id_bssid2join						0x13
#define id_regdomain						0x14
#define id_autorate							0x15
#define id_fixrate							0x16
#define id_authtype							0x17
#define id_encmode							0x18
#define id_wepdkeyid						0x19
#define id_psk_enable						0x1a
#define id_wpa_cipher						0x1b
#define id_wpa2_cipher						0x1c
#define id_passphrase						0x1d
#define id_wepkey1							0x1e
#define id_wepkey2							0x1f
#define id_wepkey3							0x20
#define id_wepkey4							0x21
#define id_opmode							0x22
#define id_rtsthres							0x23
#define id_fragthres						0x24
#define id_shortretry						0x25
#define id_longretry						0x26
#define id_band								0x27
#define id_macclone_enable			        0x28
#define id_clone_mac_addr				    0x29
#define id_qos_enable						0x2a
#define id_use40M							0x2b
#define id_shortGI20M						0x2c
#define id_shortGI40M						0x2d
#define id_aggregation						0x2e
#define id_power_save_enable		        0x2f
#define id_sens								0x30
#define id_txpower_cck						0x31
#define id_txpower_ofdm					    0x32
#define id_txpower_ht						0x33
#define id_get_wlan_info					0x34
#define id_request_scan					    0x35
#define id_get_scan_result				    0x36
#define id_cfgwrite	    			        0x50
#define id_cfgread   				        0x51
#define id_priv_shortretry 			        0x52
#define id_priv_longretry  			        0x53

// private mib id
#define id_set_host_pid						0x100		// set host daemon pid
#define id_jump								0x101
             
#define FIELD_OFFSET(type, field)	((unsigned long)(long *)&(((type *)0)->field))
#define _OFFSET(field)	((int)FIELD_OFFSET(struct mib,field))
             
#define CMD_DEF(name, type, def, start, end, action) \
	{id_##name, #name, type,  _OFFSET(name), def, start, end, action}
             
// access mib flag definition
enum {
	ACCESS_MIB_SET=0x01, 
	ACCESS_MIB_GET=0x02, 
#ifdef CMD_LINE
	ACCESS_MIB_BY_NAME=0x04, 
#endif
	ACCESS_MIB_BY_ID=0x08,
	ACCESS_MIB_BAD_CMD=0x10,
	ACCESS_MIB_ACTION=0x20,
};

						 
/*================================================================*/
/* Structure Definitions */
             
struct mac_addr {
	unsigned char addr[6];
};           
				    								    
struct mib {                                    
	unsigned char cmd_timeout;
	unsigned char mii_pause_enable;
	unsigned char eth_pause_enable;
	unsigned char cpu_suspend_enable;
	unsigned char phy_reg_poll_time;
	int write_memory;
	int read_memory;
	unsigned char fw_version[16];
	struct mac_addr wlan_mac_addr;
	unsigned char wlan_link_down_time;
	unsigned char channel;
	unsigned char ssid[33];
	struct mac_addr bssid2join;
	unsigned char regdomain;
	unsigned char autorate;
	unsigned int fixrate;
	unsigned char authtype;
	unsigned char encmode;
	unsigned char wepdkeyid;
	unsigned char psk_enable;
	unsigned char wpa_cipher;
	unsigned char wpa2_cipher;
	unsigned char passphrase[MAX_PSK_LEN];
	unsigned char wepkey1[MAX_WEP_KEY_LEN];
	unsigned char wepkey2[MAX_WEP_KEY_LEN];
	unsigned char wepkey3[MAX_WEP_KEY_LEN];
	unsigned char wepkey4[MAX_WEP_KEY_LEN];
	unsigned char opmode;
	unsigned short rtsthres;
	unsigned short fragthres;
	unsigned char shortretry;
	unsigned char longretry;
	unsigned char band;
	unsigned char macclone_enable;
	struct mac_addr clone_mac_addr;
	unsigned char qos_enable;
	unsigned char use40M;
	unsigned char shortGI20M;
	unsigned char shortGI40M;
	unsigned char aggregation;
	unsigned char power_save_enable;
	unsigned char sens;
	unsigned char txpower_cck;
	unsigned char txpower_ofdm;
	unsigned char txpower_ht;
	int get_wlan_info;
	int request_scan;
	int get_scan_result;
#ifdef RT_WLAN
	int cfgwrite;
	int cfgread;
	unsigned char priv_shortretry;
	unsigned char priv_longretry;
#endif
#ifdef JUMP_CMD
	int jump;
#endif
};                                              
                                                  
//                                                  
typedef enum  {                 
	BYTE_T,							// byte
	WORD_T,						// word
	INT_T,							// int
	INT_BIT_T,					// int in bitmask value
	BYTE_6_T,						// 6 bytes
	STRING_T,						// string
	BYTE_13_T,					// 13 bytes
	LAST_ENTRY
} TYPE_T;   

typedef enum  {                 
	ACT_MIB_RW,				// MIB read/write
	ACT_IOCTL,					// do ioctl
	ACT_MIB_RW_IOCTL	// MIB read/write + ioctl
} ACTION_T;
                                                  
// CMD table
struct cmd_table_entry {                          
	int 				id;				// cmd id
	char 			*name;		// cmd name in string                 	
	TYPE_T 		type;			// cmd value type
	int			 	offset;		// offset in struct mib      
	char 			*def;			// default value
	int				start, end;	// value range 
	ACTION_T	action;		// action flag
};                                                
                                                  
/*================================================================*/
/* Export Routines */

extern int assign_initial_value(struct mib *pmib);
extern void dump_all_mib(struct mib *pmib, int flag, int show_value);
extern int access_mib(struct mib *pmib, int flag, char *nameid, void *data1, void *data2);
extern void set_init_mib_to_driver(struct mib *pmib, int action);

#endif // INCLUDE_MIB_H

