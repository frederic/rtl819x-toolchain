#include <sys/ioctl.h>
#include <net/if.h> 
#include <stdarg.h>
#include <linux/in.h>
#include "voip_manager.h"
#include "ivrnetcfg.h"
#include "ivrhandler.h"
#include "ivripc.h"

extern uint64 g_VoIP_Feature;	/* declared in voip_manager() */
extern voip_flash_share_t *g_pVoIPShare;
extern int g_nMaxCodec;;

#if CONFIG_RTK_VOIP_PACKAGE_865X
	#include "../../../../linux-2.4.x/drivers/net/re865x/version.h"
	#define FIRMWARE_VERSION PKG_VERSION
#elif CONFIG_RTK_VOIP_PACKAGE_867X
	#define FIRMWARE_VERSION "1.3.7-0111"
#else
	#include "../../../goahead-2.1.1/LINUX/version.c"	/* reference to fwVersion */
	#define FIRMWARE_VERSION	fwVersion
#endif


enum {
	IVR_COMMAND_DHCP_CLIENT		= 111,
	IVR_COMMAND_SET_FIXED_IP	= 112,
	IVR_COMMAND_SET_NETMASK		= 113,
	IVR_COMMAND_SET_GATEWAY		= 114,
	IVR_COMMAND_SET_DNS0		= 115,
	IVR_COMMAND_SET_PPTP_IP			= 116,
	IVR_COMMAND_SET_PPTP_SERVER		= 118,
	IVR_COMMAND_VOICE_IP_ADDRESS		= 120,
	IVR_COMMAND_VOICE_IP_TYPE			= 121,
	IVR_COMMAND_VOICE_SIP_REGISTER_ID	= 122,
	IVR_COMMAND_VOICE_NETMASK			= 123,
	IVR_COMMAND_VOICE_GATEWAY			= 124,
	IVR_COMMAND_VOICE_DNS				= 125,
	IVR_COMMAND_VOICE_LAN_IP_ADDRESS	= 126,
	IVR_COMMAND_VOICE_FIRMWARE_VERSION	= 128,
	IVR_COMMAND_CODEC_SEQUENCE	= 130,
	IVR_COMMAND_HANDSET_GAIN	= 131,
	IVR_COMMAND_HANDSET_VOLUME	= 132,
	IVR_COMMAND_SPEAKER_VOLUME_GAIN	= 133,
	IVR_COMMAND_MIC_VOLUME_GAIN		= 134,
	IVR_COMMAND_ENABLE_CALL_WAITING		= 138,
	IVR_COMMAND_DISABLE_CALL_WAITING	= 139,
	IVR_COMMAND_FORWARD_SETTING			= 140,
	IVR_COMMAND_DISABLE_FORWARD_SETTING	= 141,
	IVR_COMMAND_DEFAULT_PROXY	= 150,
	IVR_COMMAND_SET_SIP_PORT	= 151,
	IVR_COMMAND_HTTP_SERVER_IP		= 153,
	/*IVR_COMMAND_HTTPS_SERVER_IP		= 154,*/
	IVR_COMMAND_TFTP_SERVER_IP		= 155,
	IVR_COMMAND_FTP_SERVER_IP		= 156,
	IVR_COMMAND_AUTO_CONFIG_MODE	= 157,
	IVR_COMMAND_SAVE_AND_REBOOT		= 195,
	IVR_COMMAND_REBOOT				= 196,
	IVR_COMMAND_RESET_TO_DEFAULT	= 198,
};

typedef enum {
	ETH_REQ_IP_ADDR,
	ETH_REQ_IP_TYPE,
	ETH_REQ_NETMASK,
	ETH_REQ_GATEWAY,
	ETH_REQ_DNS,
	ETH_REQ_LAN_IP_ADDR,
} eth_req_t;

typedef union {
	unsigned char handsetGainOrVolume;
	unsigned char speakerOrMicGain;
	unsigned char callWaiting;
	struct {
		forward_type_t forwardType;
		unsigned char noAnswerTime;
		unsigned char *pszForwardNo;
	};
	unsigned char firstCodec;
	unsigned char *pszIpAddr;
	unsigned char defaultProxy;
	unsigned char autoConfigMode;
	unsigned short sipPort;
} cmd_operand_t;

inline int IsNumericalChar( unsigned char ch )
{
	if( ch >= '0' && ch <= '9' )
		return 1;
		
	return 0;
}

const unsigned char pIpSpeechFormat[] = { "%u.%u.%u.%u" };
const unsigned char pZerosIpSpeech[] = { "0.0.0.0" };
const unsigned char pStringSpeechFormat[] = { "%s" };

static void IvrPlaySpeechFormat( unsigned int chid, const unsigned char *format, ... )
{
	unsigned char buffer[ 256 ];
	va_list va_ivr;
	
	va_start( va_ivr, format ); 
	
	vsprintf( buffer, format, va_ivr );
		
	rtk_IvrStartPlaying( chid, IVR_DIR_LOCAL, buffer );
}

static int IvrPlayEthInfoSpeech( unsigned int chid, eth_req_t req )
{
    struct ifreq ifr;
    int skfd, ret = 0;
    struct sockaddr_in *addr;
    unsigned char buffer[ 4 ];
    unsigned char dhcp;
#if 0
 #if BUILD_MODE == 2	/* GW */
 	OPMODE_T opmode;
 #endif
#endif

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

	switch( req ) {
	case ETH_REQ_LAN_IP_ADDR:	/* LAN interface */
		if( g_pVoIPShare->net_cfg.ivr_lan_interface[ 0 ] == '\x0' )
			return 0;
			
		strcpy( ifr.ifr_name, g_pVoIPShare->net_cfg.ivr_lan_interface);
		break;
	default:					/* WAN interface */
   		strcpy( ifr.ifr_name, g_pVoIPShare->net_cfg.ivr_interface);
   		break;
   	}

#if 0
 #if BUILD_MODE == 1	/* AP */
    strcpy( ifr.ifr_name, WAN_INTERFACE );
 #elif BUILD_MODE == 2	/* GW */
 	opmode = GetGatewayOperationMode();
	
	if( opmode == BRIDGE_MODE ) {		/* bridge */
		printf( "Op Mode: bridge\n" );
    	strcpy( ifr.ifr_name, "br0" );
    } else if( opmode == WISP_MODE ) {	/* wisp */
		printf( "Op Mode: WISP\n" );
    	strcpy( ifr.ifr_name, "wlan0" );    	
 	} else {							/* gateway */
		printf( "Op Mode: gateway\n" );
    	strcpy( ifr.ifr_name, WAN_INTERFACE );
    }
 #elif BUILD_MODE == 3	/* VPN */
	???
 #endif
#endif

    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
		return 0;

	switch( req ) {
	case ETH_REQ_IP_ADDR:
	case ETH_REQ_LAN_IP_ADDR:
		if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0) {
			addr = ((struct sockaddr_in *)&ifr.ifr_addr);
			IvrPlaySpeechFormat( chid, pIpSpeechFormat, 
								 *( ( unsigned char * )&addr->sin_addr + 0 ),
								 *( ( unsigned char * )&addr->sin_addr + 1 ),
								 *( ( unsigned char * )&addr->sin_addr + 2 ),
								 *( ( unsigned char * )&addr->sin_addr + 3 ) );			
			ret = 1;
		} else {
			/* If error, IVR speech '0.0.0.0' */
			IvrPlaySpeechFormat( chid, pZerosIpSpeech );
		}

		break;
		
	case ETH_REQ_IP_TYPE:
		if( net_cfg_flash_read( NET_CFG_DHCP, &dhcp ) ) {
			if( dhcp )	/* DHCP client */
				buffer[ 0 ] = IVR_TEXT_ID_DHCP;
			else		/* fix ip (it may be a DHCP server, fix ip, or ...) */
				buffer[ 0 ] = IVR_TEXT_ID_FIX_IP;
			
			buffer[ 1 ] = '\x0';
			
			IvrPlaySpeechFormat( chid, buffer );
			
			ret = 1;
		}
		break;
		
	case ETH_REQ_NETMASK:
		if (ioctl(skfd, SIOCGIFNETMASK, &ifr) == 0) {
			addr = ((struct sockaddr_in *)&ifr.ifr_addr);
			IvrPlaySpeechFormat( chid, pIpSpeechFormat, 
								 *( ( unsigned char * )&addr->sin_addr + 0 ),
								 *( ( unsigned char * )&addr->sin_addr + 1 ),
								 *( ( unsigned char * )&addr->sin_addr + 2 ),
								 *( ( unsigned char * )&addr->sin_addr + 3 ) );			
			ret = 1;
		} else {
			/* If error, IVR speech '0.0.0.0' */
			IvrPlaySpeechFormat( chid, pZerosIpSpeech );
		}
		break;
		
	case ETH_REQ_GATEWAY:
		if( net_cfg_flash_read( NET_CFG_GATEWAY, buffer ) ) {
			IvrPlaySpeechFormat( chid, pIpSpeechFormat, 
								 buffer[ 0 ],
								 buffer[ 1 ],
								 buffer[ 2 ],
								 buffer[ 3 ] );
			ret = 1;
		}
		break;
		
	case ETH_REQ_DNS:
		if( net_cfg_flash_read( NET_CFG_DNS, buffer ) ) {
			IvrPlaySpeechFormat( chid, pIpSpeechFormat, 
								 buffer[ 0 ],
								 buffer[ 1 ],
								 buffer[ 2 ],
								 buffer[ 3 ] );
			ret = 1;
		}
		break;
				
	default:
		break;
	}
	
	close( skfd );
	return ret;
}

static int GetTwoDigitsOperand( unsigned int cmd_len, unsigned char *pDialCode,
								unsigned char *pDigitsOperand )
{
	if( cmd_len != 5 )
		return 0;		/* bad length */

	if( !IsNumericalChar( pDialCode[ 3 ] ) ||
		!IsNumericalChar( pDialCode[ 4 ] ) )
	{
		return 0;		/* bad oeprand */
	}
	
	*pDigitsOperand = ( pDialCode[ 3 ] - '0' ) * 10 +
					  ( pDialCode[ 4 ] - '0' );	

	return 1;
}

static int CheckIpAddressOperand( unsigned int cmd_len, 
								  const unsigned char *pDialCode,
								  unsigned char bMask )
{
	unsigned int i;
	unsigned int nNumDot = 0;
	
	unsigned int nCurDigit = 0;
	unsigned int nCurDigitCount = 0;
	
	unsigned char ch;
	
	for( i = 3; i < cmd_len; i ++ ) {
		
		ch = pDialCode[ i ];
		
		if( IsNumericalChar( ch ) ) {
			
			nCurDigitCount ++;
			nCurDigit *= 10;
			nCurDigit += ( unsigned int )( ch - '0' );
			
			if( nCurDigitCount > 3 )
				return 0;	/* bad format */
				
			if( nNumDot < 3 ) {
				if( nCurDigit > 255 )
					return 0;
			} else {	/* nNumDot == 3 */
				if( bMask ) {	/* 4th digit of mask can be 0 ~ 255 */
					if( nCurDigit > 255 )
						return 0;
				} else {		/* 4th digit of IP can be 1 ~ 254 */
					if( nCurDigit > 254 )
						return 0;
				}
			}
				
		} else if( ch == '.' ) {
			
			if( nCurDigitCount == 0 )
				return 0;	/* bad format */
			
			nNumDot ++;
			
			nCurDigit = 0;
			nCurDigitCount = 0;
			
			if( nNumDot > 3 )
				return 0;	/* not a IPv4 foramt */
			
		} else
			return 0;	/* unexpected character */
	}
	
	/* IPv4 format */
	if( nNumDot != 3 || nCurDigitCount == 0 )
		return 0;	/* not a IPv4 format */
	
	if( !bMask && nCurDigit == 0 )
		return 0;	/* 4th digit of IP can be 1 ~ 254 */
	
	return 1;
}

static unsigned short CheckAndGetNumericalOperand( const unsigned char *pDialCode )
{
	const unsigned char *pSrc = pDialCode;
	unsigned char ch;
	
	while( ( ch = *pSrc ++ ) ) {
		if( ch >= '0' && ch <= '9' )
			continue;
	
		return 0;
	}
	
	return ( unsigned short )atoi( pDialCode );
}

static int ParseIvrInstructionFormat( unsigned int ivr_cmd, 
									  unsigned int cmd_len,	
									  unsigned char *pDialCode,
									  cmd_operand_t *pOperand )
{
	/* cmd_len is length of command + operands */
	unsigned char temp;
	unsigned short temp2;
	int nMaxCodecSeq;

	switch( ivr_cmd ) {
		/* These contain IP parameter. */
		case IVR_COMMAND_SET_FIXED_IP:
		case IVR_COMMAND_SET_NETMASK:
		case IVR_COMMAND_SET_GATEWAY:
		case IVR_COMMAND_SET_DNS0:
		case IVR_COMMAND_HTTP_SERVER_IP:
		case IVR_COMMAND_TFTP_SERVER_IP:
		case IVR_COMMAND_FTP_SERVER_IP:
		case IVR_COMMAND_SET_PPTP_SERVER:
		case IVR_COMMAND_SET_PPTP_IP:

			if( !CheckIpAddressOperand( cmd_len, pDialCode, 
							( ivr_cmd == IVR_COMMAND_SET_NETMASK ) ) )
			{
				return 0;	/* bad operand */
			}
			
			pOperand ->pszIpAddr = &pDialCode[ 3 ];
			break;

#if 0	// pptp with dynamic or static IP 
		case IVR_COMMAND_SET_PPTP_IP:
		
			if( pDialCode[ 3 ] == '0' ) {
				if( cmd_len != 4 )
					return 0;	/* bad length */
					
				pOperand ->pszIpAddr = NULL;
			} else if( pDialCode[ 3 ] == '1' ) {
				if( !CheckIpAddressOperand( cmd_len - 1, pDialCode + 1, 0 ) )
					return 0;	/* bad operand */
				
				pOperand ->pszIpAddr = &pDialCode[ 4 ];
			} else
				return 0;	/* bad operand */
				
			break;
#endif
		
		/* Codec sequence (bring a codec to be first one) */
		case IVR_COMMAND_CODEC_SEQUENCE:
		
			if( !GetTwoDigitsOperand( cmd_len, pDialCode, &temp ) )
				return 0;	/* bad operand */
			
			nMaxCodecSeq = g_nMaxCodec;
			
			if( g_VoIP_Feature & CODEC_G723_SUPPORT )
				nMaxCodecSeq += 1;	/* G.723 has 2 bitrate */
				
			if( g_VoIP_Feature & CODEC_iLBC_SUPPORT )
				nMaxCodecSeq += 1;	/* iLBC has 2 bitrate */
	
			if( temp < 1 || temp > nMaxCodecSeq )
				return 0;	/* codec range: 1 ~ g_nMaxCodec + n */
		
			pOperand ->firstCodec = temp - 1;
			break;
		
		/* Adjust Volume */
		case IVR_COMMAND_HANDSET_GAIN:
		case IVR_COMMAND_HANDSET_VOLUME:

			if( !GetTwoDigitsOperand( cmd_len, pDialCode, &temp ) )
				return 0;	/* bad operand */
			
			if( temp < 1 || temp > 10 )
				return 0;	/* volume range: 1 ~ 10 */
			
			pOperand ->handsetGainOrVolume = temp;
			break;
		
		/* Adjust Gain */
		case IVR_COMMAND_SPEAKER_VOLUME_GAIN:
		case IVR_COMMAND_MIC_VOLUME_GAIN:
		
			if( !GetTwoDigitsOperand( cmd_len, pDialCode, &temp ) )
				return 0;	/* bad operand */
			
			if( temp > 63 )
				return 0;	/* volume range: 0 ~ 63 */
			
			pOperand ->speakerOrMicGain = temp;
			break;
		
		/* Forwarding */
		case IVR_COMMAND_FORWARD_SETTING:
		
			if( cmd_len < 5 )
				return 0;	/* command is too short */
		
			switch( pDialCode[ 3 ] ) {
			case '1':	/* immediate forward */
				pOperand ->forwardType = FORWARD_TYPE_IMMEDIATE;
				pOperand ->pszForwardNo = &pDialCode[ 4 ];
				break;
			case '2':	/* busy forward */
				pOperand ->forwardType = FORWARD_TYPE_BUSY;
				pOperand ->pszForwardNo = &pDialCode[ 4 ];
				break;
			case '3':	/* no answer forward */
				if( cmd_len < 7 )
					return 0;	/* 2 digits no answer time */
				
				/* borrow to get no answer time. */
				if( !GetTwoDigitsOperand( 5, pDialCode + 1, &temp ) )
					return 0;	/* bad operand */
				
				pOperand ->forwardType = FORWARD_TYPE_NO_ANSWER;
				pOperand ->noAnswerTime = temp;
				pOperand ->pszForwardNo = &pDialCode[ 6 ];
				break;
			default:
				return 0;	/* bad type */
			}			
			
			break;

		/* rock: add default proxy */
		case IVR_COMMAND_DEFAULT_PROXY:
			printf("cmd_len = %d, pDialCode = %s\n",
				cmd_len, pDialCode);

			if (cmd_len != 4)
				return 0;		/* bad length */

			if (!IsNumericalChar(pDialCode[3]))
				return 0;		/* bad oeprand */

			temp = pDialCode[3] - '0';
			if( temp > MAX_PROXY - 1 )
				return 0;	/* volume range: 0 ~ MAX_PROXY - 1 */
			
			pOperand ->defaultProxy = temp;
			break;
		
		case IVR_COMMAND_SET_SIP_PORT:
			if( cmd_len < 7 || cmd_len > 8 )
				return 0;	/* bad length, port range 5000~10000 */
				
			temp2 = CheckAndGetNumericalOperand( &pDialCode[ 3 ] );
			
			if( temp2 < 5000 || temp2 > 10000 )
				return 0;	/* bad range */
				
			pOperand ->sipPort = temp2;
			break;
		
		case IVR_COMMAND_AUTO_CONFIG_MODE:
			
			if (cmd_len != 4)
				return 0;		/* bad length */

			if (!IsNumericalChar(pDialCode[3]))
				return 0;		/* bad oeprand */
			
			temp = pDialCode[3] - '0';
			if( temp > 3 )
				return 0;	/* volume range: 0: disable, 1: http, 2: TFTP, 3: FTP */
			
			pOperand ->autoConfigMode = temp;
			break;

		/* Length of other commands has to be 3. */
		default:
			if( cmd_len != 3 )
				return 0;	/* bad format */
			break;
	}
	
	return 1;	/* pass check */
}

int IsInstructionForIVR( const do_ivr_ins_t *msg )
{
	unsigned int ivr_cmd, cmd_len;
	int i;
	unsigned char ch;
	cmd_operand_t cmd_operands;
	int bPlayBusyTone = 0;
	
	/* Instruction start with exactly one '#' */
	if( msg ->dial_initial_hash != 1 )
		return 0;
	
	/* Length of command equal to 3 */
	if( ( cmd_len = msg ->digit_index ) < 3 )
		return 0;
	
	/* command are 3 digits */
	for( i = 0; i < 3; i ++ ) {
		ch = msg ->dial_code[ i ];
		if( !IsNumericalChar( ch ) )
			return 0;
	}

	/* convert string command to decimal */
	ivr_cmd = ( unsigned int )( msg ->dial_code[ 0 ] - '0' ) * 100 + 
			  ( unsigned int )( msg ->dial_code[ 1 ] - '0' ) * 10 + 
			  ( unsigned int )( msg ->dial_code[ 2 ] - '0' );

	/* Check command format and retrieve operands: Length of most commands is 3 */
	if( !ParseIvrInstructionFormat( ivr_cmd, cmd_len, 
									&msg ->dial_code[ 0 ],
									&cmd_operands ) ) 
	{
		printf( "IVR bad instruction: %d len=%d\n", ivr_cmd, cmd_len );
		return 0;	/* Not match instruction format. */
	}

	/* Do IVR command */
	switch( ivr_cmd ) {
	/* ************** Change Network Settings ************** */
	case IVR_COMMAND_DHCP_CLIENT:
		IvrSetNetworkDhcpClientCfg();
		break;
		
	case IVR_COMMAND_SET_FIXED_IP:
		IvrSetNetworkFixedIpCfg( cmd_operands.pszIpAddr );
		break;
		
	case IVR_COMMAND_SET_NETMASK:
		IvrSetNetworkNetmaskCfg( cmd_operands.pszIpAddr );
		break;
		
	case IVR_COMMAND_SET_GATEWAY:
		IvrSetNetworkGatewayCfg( cmd_operands.pszIpAddr );
		break;
		
	case IVR_COMMAND_SET_DNS0:
		IvrSetNetworkDnsCfg( cmd_operands.pszIpAddr );
		break;
	
	case IVR_COMMAND_SET_PPTP_IP:
		IvrSetNetworkPPTPIpCfg( cmd_operands.pszIpAddr );
		break;
				
	case IVR_COMMAND_SET_PPTP_SERVER:
		IvrSetNetworkPPTPServerCfg( cmd_operands.pszIpAddr );
		break;
	
	/* ************** Voice Network Settings ************** */
	case IVR_COMMAND_VOICE_IP_ADDRESS:
		IvrPlayEthInfoSpeech( msg ->chid, ETH_REQ_IP_ADDR );
		goto label_wait_for_voice_ivr_complete;
		break;
		
	case IVR_COMMAND_VOICE_IP_TYPE:
		IvrPlayEthInfoSpeech( msg ->chid, ETH_REQ_IP_TYPE );
		goto label_wait_for_voice_ivr_complete;
		break;
		
	case IVR_COMMAND_VOICE_SIP_REGISTER_ID:
		IvrPlaySpeechFormat( msg ->chid, pStringSpeechFormat, 
							 msg ->login_id );
		goto label_wait_for_voice_ivr_complete;
		break;
		
	case IVR_COMMAND_VOICE_NETMASK:
		IvrPlayEthInfoSpeech( msg ->chid, ETH_REQ_NETMASK );
		goto label_wait_for_voice_ivr_complete;
		break;
		
	case IVR_COMMAND_VOICE_GATEWAY:
		IvrPlayEthInfoSpeech( msg ->chid, ETH_REQ_GATEWAY );
		goto label_wait_for_voice_ivr_complete;
		break;
		
	case IVR_COMMAND_VOICE_DNS:
		IvrPlayEthInfoSpeech( msg ->chid, ETH_REQ_DNS );
		goto label_wait_for_voice_ivr_complete;
		break;
		
	case IVR_COMMAND_VOICE_LAN_IP_ADDRESS:
		if( IvrPlayEthInfoSpeech( msg ->chid, ETH_REQ_LAN_IP_ADDR ) )
			goto label_wait_for_voice_ivr_complete;
		else
			return 0;
		break;
		
	case IVR_COMMAND_VOICE_FIRMWARE_VERSION:
		IvrPlaySpeechFormat( msg ->chid, pStringSpeechFormat, 
							 FIRMWARE_VERSION );
		goto label_wait_for_voice_ivr_complete;
		break;
		
	/* ************** Change VoIP Settings ************** */
	case IVR_COMMAND_CODEC_SEQUENCE:
		IvrSetVoipCodecSequenceCfg( msg ->chid, cmd_operands.firstCodec );
		bPlayBusyTone = 1;
		break;
		
	case IVR_COMMAND_HANDSET_GAIN:
		IvrSetVoipHandsetGainCfg( msg ->chid, cmd_operands.handsetGainOrVolume );
		bPlayBusyTone = 1;
		break;
		
	case IVR_COMMAND_HANDSET_VOLUME:
		IvrSetVoipHandsetVolumeCfg( msg ->chid, cmd_operands.handsetGainOrVolume );		
		bPlayBusyTone = 1;
		break;

	case IVR_COMMAND_SPEAKER_VOLUME_GAIN:
		IvrSetVoipSpeakerVolumeGainCfg( msg ->chid, cmd_operands.speakerOrMicGain );
		bPlayBusyTone = 1;
		break;
		
	case IVR_COMMAND_MIC_VOLUME_GAIN:
		IvrSetVoipMicVolumeVolumeCfg( msg ->chid, cmd_operands.speakerOrMicGain );		
		bPlayBusyTone = 1;
		break;
		
	case IVR_COMMAND_ENABLE_CALL_WAITING:
		IvrSetVoipEnableCallWaitingCfg( msg ->chid, 1 /* enable */ );
		bPlayBusyTone = 1;
		break;
		
	case IVR_COMMAND_DISABLE_CALL_WAITING:
		IvrSetVoipEnableCallWaitingCfg( msg ->chid, 0 /* disable */ );
		bPlayBusyTone = 1;
		break;
		
	case IVR_COMMAND_FORWARD_SETTING:
		IvrSetVoipForwardSettingCfg( msg ->chid, cmd_operands.forwardType,
									 cmd_operands.noAnswerTime,
									 cmd_operands.pszForwardNo );
		bPlayBusyTone = 1;
		break;
		
	case IVR_COMMAND_DISABLE_FORWARD_SETTING:
		IvrSetVoipDisableForwardSettingCfg( msg ->chid );
		bPlayBusyTone = 1;
		break;
		
	/* rock: add default proxy */
	case IVR_COMMAND_DEFAULT_PROXY:
		IvrSetVoipDefaultProxyCfg(msg->chid, cmd_operands.defaultProxy);
		bPlayBusyTone = 1;
		break;
		
	case IVR_COMMAND_SET_SIP_PORT:
		IvrSetVoipSipPortCfg(msg->chid, cmd_operands.sipPort);
		bPlayBusyTone = 1;
		break;

	/* ************** Auto Config Settings ************** */
	case IVR_COMMAND_HTTP_SERVER_IP:
		IvrSetAutoConfigHttpServerIpCfg( cmd_operands.pszIpAddr );
		bPlayBusyTone = 1;
		break;
		
	case IVR_COMMAND_TFTP_SERVER_IP:
		IvrSetAutoConfigTftpServerIpCfg( cmd_operands.pszIpAddr );
		bPlayBusyTone = 1;
		break;
		
	case IVR_COMMAND_FTP_SERVER_IP:
		IvrSetAutoConfigFtpServerIpCfg( cmd_operands.pszIpAddr );
		bPlayBusyTone = 1;
		break;

	case IVR_COMMAND_AUTO_CONFIG_MODE:
		IvrSetAutoConfigModeCfg( cmd_operands.autoConfigMode );
		bPlayBusyTone = 1;
		break;

	/* ************** System Settings ************** */
	case IVR_COMMAND_SAVE_AND_REBOOT:
		IvrSystemSaveAndRebootCfg();
		bPlayBusyTone = 1;
		break;
		
	case IVR_COMMAND_REBOOT:
		IvrSystemReboot();
		break;
		
	case IVR_COMMAND_RESET_TO_DEFAULT:
		IvrSystemResetToDefaultCfg();
		bPlayBusyTone = 1;
		break;

	default:
		return 0;	/* unknwon command */
	}

	/* TODO: if it restart solar, how to deal with? */
#if 0
	printf( "IVR command: %d\n", ivr_cmd );

	SetSysState( msg ->chid, SYS_STATE_IDLE );

	return 1;	/* yes, IVR instruction */
#endif

label_wait_for_voice_ivr_complete:
	printf( "IVR command: %d\n", ivr_cmd );

#if 0
	if( bPlayBusyTone ) {
		/* Play 'busy' tone. */
		rtk_SetPlayTone( msg ->chid, msg ->sid, 
					DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
		SetSysState( msg ->chid, SYS_STATE_VOICE_IVR_DONE );
	} else {
		/* Play 'dial' tone, or voice IVR */
		SetSysState( msg ->chid, SYS_STATE_VOICE_IVR );
	}
#else
	if( bPlayBusyTone ) {
		/* Play 'busy' tone. */
		return IVR_IPC_RET_BUSY_TONE;	/* yes, IVR instruction */
	}
	
	/* Play 'dial' tone, or voice IVR */
	return IVR_IPC_RET_VOICE_CFG;		/* yes, IVR instruction */
#endif
}

void InitializeIVR( void )
{
#ifdef NETWORK_SETTINGS_IN_RAM
	IvrClearNetworkSettingsInRam();
#endif
}

