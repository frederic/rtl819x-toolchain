#ifndef __IVR_NET_CFG_H__
#define __IVR_NET_CFG_H__

#define NETWORK_SETTINGS_IN_RAM

typedef enum {
	FORWARD_TYPE_IMMEDIATE,
	FORWARD_TYPE_BUSY,
	FORWARD_TYPE_NO_ANSWER,
} forward_type_t;

#if 0
#if !defined( CONFIG_RTK_VOIP_DRIVERS_PCM8651 ) && ( BUILD_MODE == 2 )
#include "../../../goahead-2.1.1/LINUX/apmib.h"		/* OPMODE_T */
#endif
#endif

/* ************** Network Parameters In RAM ************** */
#ifdef NETWORK_SETTINGS_IN_RAM
extern void IvrClearNetworkSettingsInRam( void );
#endif

/* ************** Change Network Settings ************** */
/* IVR_COMMAND_DHCP_CLIENT */
extern int IvrSetNetworkDhcpClientCfg( void );

/* IVR_COMMAND_SET_FIXED_IP */
extern int IvrSetNetworkFixedIpCfg( const unsigned char *pszFixedIP );

/* IVR_COMMAND_SET_NETMASK */
extern int IvrSetNetworkNetmaskCfg( const unsigned char *pszNetmask );

/* IVR_COMMAND_SET_GATEWAY */
extern int IvrSetNetworkGatewayCfg( const unsigned char *pszGateway );

/* IVR_COMMAND_SET_DNS0 */
extern int IvrSetNetworkDnsCfg( const unsigned char *pszDNS );

/* IVR_COMMAND_SET_PPTP_IP */
extern int IvrSetNetworkPPTPIpCfg( const unsigned char *pszPptpIP );

/* IVR_COMMAND_SET_PPTP_SERVER */
extern int IvrSetNetworkPPTPServerCfg( const unsigned char *pszPptpServer );

/* ************** System Settings ************** */
/* IVR_COMMAND_SAVE_AND_REBOOT */
extern int IvrSystemSaveAndRebootCfg( void );

/* IVR_COMMAND_REBOOT */
extern int IvrSystemReboot( void );

/* IVR_COMMAND_RESET_TO_DEFAULT */
extern int IvrSystemResetToDefaultCfg( void );

/* ************** Change VoIP Settings ************** */	
/* IVR_COMMAND_CODEC_SEQUENCE */
extern int IvrSetVoipCodecSequenceCfg( unsigned int chid, unsigned char firstCodec );

/* IVR_COMMAND_HANDSET_GAIN */
extern int IvrSetVoipHandsetGainCfg( unsigned int chid, unsigned char gain );

/* IVR_COMMAND_HANDSET_VOLUME */
extern int IvrSetVoipHandsetVolumeCfg( unsigned int chid, unsigned char volume );

/* IVR_COMMAND_SPEAKER_VOLUME_GAIN */
extern int IvrSetVoipSpeakerVolumeGainCfg( unsigned int chid, unsigned char gain );

/* IVR_COMMAND_MIC_VOLUME_GAIN */
extern int IvrSetVoipMicVolumeVolumeCfg( unsigned int chid, unsigned char gain );

/* IVR_COMMAND_ENABLE_CALL_WAITING / IVR_COMMAND_DISABLE_CALL_WAITING */
extern int IvrSetVoipEnableCallWaitingCfg( unsigned int chid, unsigned char enable );

/* IVR_COMMAND_FORWARD_SETTING */
extern int IvrSetVoipForwardSettingCfg( unsigned int chid, forward_type_t type, 
								 unsigned char noAnswerTime,
								 const unsigned char *pszForwardPhoneNumber );

/* IVR_COMMAND_DISABLE_FORWARD_SETTING */
extern int IvrSetVoipDisableForwardSettingCfg( unsigned int chid );

/* IVR_COMMAND_DEFAULT_PROXY */
extern int IvrSetVoipDefaultProxyCfg( unsigned int chid, unsigned char defaultProxy );

/* IVR_COMMAND_SET_SIP_PORT */
extern int IvrSetVoipSipPortCfg( unsigned int chid, unsigned short sipPort );


/* ************** Auto Config Settings ************** */
/* IVR_COMMAND_HTTP_SERVER_IP */
extern int IvrSetAutoConfigHttpServerIpCfg( const unsigned char *pszServerIP );

/* IVR_COMMAND_TFTP_SERVER_IP */
extern int IvrSetAutoConfigTftpServerIpCfg( const unsigned char *pszServerIP );

/* IVR_COMMAND_FTP_SERVER_IP */
extern int IvrSetAutoConfigFtpServerIpCfg( const unsigned char *pszServerIP );
	
/* IVR_COMMAND_AUTO_CONFIG_MODE */	
extern int IvrSetAutoConfigModeCfg( const unsigned char mode );

#endif /* __IVR_NET_CFG_H__ */

