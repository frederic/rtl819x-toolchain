#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <signal.h>
#include <errno.h>
#include "voip_flash.h"
#include "voip_types.h"
#include "voip_params.h"
#include "voip_control.h"
#include "voip_feature.h"

#ifdef SUPPORT_VOIP_FLASH_WRITE
#if CONFIG_RTK_VOIP_PACKAGE_865X
	#include "rtl_cfgmgr.h"
	#include "rtl_board.h"
	#include <sys/shm.h>
#elif CONFIG_RTK_VOIP_PACKAGE_867X
	#include <sys/shm.h>
	#include "mib.h"
#else
	#include "apmib.h"	// CONFIG_DATA_T
#endif
#endif /* SUPPORT_VOIP_FLASH_WRITE */

#include "voip_flash_client.h"

#include "voip_ioctl.h"

static int sem_id = -1;
static int shm_id = -1;
static voip_flash_share_t *g_shmVoIPCfg = NULL;
static voip_flash_share_t *g_pVoIPShare = NULL;
static int g_nShareSize = 0;

#ifdef SUPPORT_VOIP_FLASH_WRITE
static int voip_flash_client_write_init( cid_t cid );
static void voip_flash_client_write_close( void );
#endif

static int sem_wait(const int sem_num)
{
	struct sembuf sem_b;

	sem_b.sem_num = sem_num;
	sem_b.sem_op = -1;
	sem_b.sem_flg = SEM_UNDO;
	return semop(sem_id, &sem_b, 1);
}

static int sem_signal(const int sem_num)
{
	struct sembuf sem_b;

	sem_b.sem_num = sem_num;
	sem_b.sem_op = 1;
	sem_b.sem_flg = SEM_UNDO;
	return semop(sem_id, &sem_b, 1);
}

int voip_flash_client_update()
{
	if (sem_wait(VOIP_SEM_MUTEX) == -1)
		return -1;

	// update data from share memory 
	memcpy(g_pVoIPShare, g_shmVoIPCfg, g_nShareSize);

	if (sem_signal(VOIP_SEM_MUTEX) == -1)
		return -1;

	return 0;
}

void voip_flash_client_close()
{
	if (g_pVoIPShare != NULL)
	{
		free(g_pVoIPShare);
		g_pVoIPShare = NULL;
	}

	if (g_shmVoIPCfg != NULL)
	{
		if (shmdt(g_shmVoIPCfg) == -1)
			fprintf(stderr, "shmdt failed: %s\n", strerror(errno));

		g_shmVoIPCfg = NULL;
	}

#ifdef SUPPORT_VOIP_FLASH_WRITE
	voip_flash_client_write_close();
#endif
}

int voip_flash_client_init(voip_flash_share_t **ppVoIPCfg, cid_t cid)
{
   	TstVoipFeature stVoipFeature;
	int MaxVoIPPorts;
	key_t key;
	static int nRetry = 0;

	//SETSOCKOPT(VOIP_MGR_GET_FEATURE, &stVoipFeature, TstVoipFeature, 1);
	//memcpy( &g_VoIP_Feature, &stVoipFeature, sizeof( TstVoipFeature ) );
	MaxVoIPPorts = RTK_VOIP_CH_NUM(g_VoIP_Feature);
	g_nShareSize = VOIP_SHARE_SIZE(MaxVoIPPorts);

	fprintf(stderr, "voip flash client share size = %d\n", g_nShareSize);

	key = ftok(VOIP_PATHNAME, VOIP_FLASH_VER);
	if (key == -1)
	{
		fprintf(stderr, "ftok failed(%s)\n", strerror(errno));
		return -1;
	}

	while (1)
	{
		sem_id = semget(key, 2, 0);
		if (sem_id != -1)
			break;

		if (errno != ENOENT)
		{
			fprintf(stderr, "semget failed(%s)\n", strerror(errno));
			return -1;
		}

		fprintf(stdout, "waiting voip flash server start... (%d)\n", nRetry);
		if (nRetry++ < 3)
			sleep(1);
		else
			sleep(3);
	}

	fprintf(stdout, "waiting voip flash server init...)\n");
	// waiting web init done (copy flash ok)
	sem_wait(VOIP_SEM_EVENT);

	// mapping share memory
	shm_id = shmget(key, g_nShareSize, 0);
	if (shm_id == -1)
	{
		fprintf(stderr, "shmget failed: %s\n", strerror(errno));
		goto voip_flash_client_start_failed;
	}

	g_shmVoIPCfg = shmat(shm_id, NULL, SHM_RDONLY);
	if (g_shmVoIPCfg == (void *)-1)
	{
		fprintf(stderr, "shmat failed: %s\n", strerror(errno));
		goto voip_flash_client_start_failed;
	}

	g_pVoIPShare = (voip_flash_share_t *) malloc(g_nShareSize);
	if (g_pVoIPShare == NULL)
	{
		fprintf(stderr, "malloc voip cfg failed\n");
		goto voip_flash_client_start_failed;
	}

	*ppVoIPCfg = g_pVoIPShare;

	voip_flash_client_update();

#ifdef SUPPORT_VOIP_FLASH_WRITE
	voip_flash_client_write_init( cid );
#endif

	return 0;

voip_flash_client_start_failed:
	voip_flash_client_close();
	return -1;
}

unsigned char GetDhcpValueToSetFixedIP( void )
{
#if 0
	int ret;
	DHCP_T dhcp;
	
	ret = apmib_get( MIB_DHCP, &dhcp );
	
	if( ret ) {
		/* DHCP client -> Disable */
		if( dhcp == DHCP_CLIENT )
			dhcp = DHCP_DISABLED;
	} else
		dhcp = DHCP_DISABLED;
	
	return dhcp;
#else
	return NET_DHCP_DISABLED;
#endif
}

#if 0
int GetGatewayOperationMode( void )
{
	/* 
	 *     Mode     LAN   WAN
	 * ----------  ----- ------
	 * 0: gateway   br0   eth1   <-- WAN, eth1
	 * 1: bridge    br0   N/A    <-- LAN, br0
	 * 2: wisp      br0   wlan0  <-- WAN, wlan0
	 */
 	OPMODE_T opmode;
 
 	/* refresh flash content */
	// TODO: remove apmib_reinit
	//apmib_reinit(); 
 	
 	if ( !apmib_get( MIB_OP_MODE, (void *)&opmode) )
		return 0;

	return opmode;
}
#endif

int net_cfg_flash_read( net_cfg_t cfg, void *pCfgData )
{
#if 0
	int id, ret;
	DHCP_T dhcp;
 #if defined(  HOME_GATEWAY	) && !defined( VPN_SUPPORT )
 	OPMODE_T opmode;
 	
 	opmode = GetGatewayOperationMode();
 	
 	switch( opmode ) {
	case BRIDGE_MODE:
		printf( "Op Mode: bridge\n" );
		break;
		
	case WISP_MODE:
		printf( "Op Mode: WISP\n" );
		break;
		
	default:
		printf( "Op Mode: gateway\n" );
		break;
 	}
 #else
	/* refresh flash content */
	// TODO: remove apmib_reinit 
	//apmib_reinit(); 
 #endif
#endif
 	
	switch( cfg ) {
	case NET_CFG_DHCP:
#if 0
 #ifdef HOME_GATEWAY
  #ifdef VPN_SUPPORT	/* VPN */
  		// TODO: confirm follows 
  		ret = apmib_get( MIB_WAN_DHCP, &dhcp );
  #else					/* GW */
  		if( opmode == BRIDGE_MODE )
  			ret = apmib_get( MIB_DHCP, &dhcp );
  		else
 			ret = apmib_get( MIB_WAN_DHCP, &dhcp );
  #endif
 #else					/* AP */
		ret = apmib_get( MIB_DHCP, &dhcp );
 #endif
		printf( "DHCP: %d (%d)\n", ( int )dhcp, ret );
		
		if( dhcp == DHCP_CLIENT )
			*( ( unsigned char *)pCfgData ) = 1;
		else
			*( ( unsigned char *)pCfgData ) = 0;

		return ret;
#else
		*( ( unsigned char *)pCfgData ) = g_pVoIPShare->net_cfg.dhcp;
		return 1;
#endif

	case NET_CFG_IP:
#if 0
 #ifdef HOME_GATEWAY
  #ifdef VPN_SUPPORT	/* VPN */
  		// TODO: confirm follows 
  		id = MIB_WAN_IP_ADDR;
  #else					/* GW */
  		if( opmode == BRIDGE_MODE )
			id = MIB_IP_ADDR;
		else
			id = MIB_WAN_IP_ADDR;
  #endif
 #else					/* AP */
		id = MIB_IP_ADDR;
 #endif
		break;
#else
		*( ( unsigned long *)pCfgData ) = g_pVoIPShare->net_cfg.ip;
		return 1;
#endif

	case NET_CFG_NETMASK:
#if 0
 #ifdef HOME_GATEWAY
  #ifdef VPN_SUPPORT	/* VPN */
  		// TODO: confirm follows 
  		id = MIB_WAN_SUBNET_MASK;
  #else					/* GW */
  		if( opmode == BRIDGE_MODE )
			id = MIB_SUBNET_MASK;
		else
			id = MIB_WAN_SUBNET_MASK;
  #endif
 #else					/* AP */
		id = MIB_SUBNET_MASK;
 #endif
		break;
#else
		*( ( unsigned long *)pCfgData ) = g_pVoIPShare->net_cfg.netmask;
		return 1;
#endif

	case NET_CFG_GATEWAY:
#if 0
 #ifdef HOME_GATEWAY
  #ifdef VPN_SUPPORT	/* VPN */
  		// TODO: confirm follows 
  		id = MIB_WAN_DEFAULT_GATEWAY;
  #else					/* GW */
  		if( opmode == BRIDGE_MODE )
			id = MIB_DEFAULT_GATEWAY;
		else
			id = MIB_WAN_DEFAULT_GATEWAY;
  #endif
 #else					/* AP */
		id = MIB_DEFAULT_GATEWAY;
 #endif
		break;
#else
		*( ( unsigned long *)pCfgData ) = g_pVoIPShare->net_cfg.gateway;
		return 1;
#endif

	case NET_CFG_DNS:
#if 0
 #ifdef HOME_GATEWAY
  #ifdef VPN_SUPPORT	/* VPN */
  		// TODO: confirm follows 
  		id = MIB_WAN_DNS1;
  #else					/* GW */
  		if( opmode != BRIDGE_MODE )
			id = MIB_WAN_DNS1;
		else
  #endif
 #endif
 #ifndef VPN_SUPPORT
 		{				/* AP */
			if( ret = apmib_get( MIB_DHCP, &dhcp ) ) {
				/* DNS is valid only if DHCP server */
				if( dhcp == DHCP_SERVER )
					id = MIB_WAN_DNS1;
				else {
					*( ( unsigned long * )pCfgData ) = 0;
					return 1;
				}
			}
 		}
 #endif
		break;
#else
		*( ( unsigned long *)pCfgData ) = g_pVoIPShare->net_cfg.dns;
		return 1;
#endif

	default:
		return 0;
	}
	
#if 0
	return apmib_get( id, pCfgData );
#else
	return 0;
#endif
}

// ---------------------------------------------------------------
// Flash Write  
// ---------------------------------------------------------------
#ifdef SUPPORT_VOIP_FLASH_WRITE
#include <sys/msg.h>

static int qidVoipFlashClientWriteSnd;
static int qidVoipFlashClientWriteRcv;
static cid_t cidVoipFlashClientWriteRcv;

static int voip_flash_client_write_init( cid_t cid )
{
	key_t key;
	//VoipFlashWriteMsg_t msgSuper;

	/* Get qid of server */
	key = ftok( VOIP_FLASH_WRITE_PATHNAME, VOIP_FLASH_WRITE_SERVER );
	
	if( ( qidVoipFlashClientWriteSnd = msgget( key, 0666 | IPC_CREAT ) ) == -1 ) {
		fprintf( stderr, "Create flash client write snd event queue fail.\n" );
	}

	/* set qid of client */
	if( ( cidVoipFlashClientWriteRcv = cid ) >= MAX_VOIP_FLASH_WRITE_CLIENT ) {
		printf( "An invalid flash write id.\n" );
		goto label_flash_write_init_done;
	}
	
	key = ftok( VOIP_FLASH_WRITE_PATHNAME, cidVoipFlashClientWriteRcv + VOIP_FLASH_WRITE_CLIENT_BASE );
	
	if( ( qidVoipFlashClientWriteRcv = msgget( key, 0666 | IPC_CREAT ) ) == -1 ) {
		fprintf( stderr, "Create flash client write rcv event queue fail.\n" );
	}
	
	fprintf( stderr, "qidVoipFlashClientWriteRcv:%d(%d)\n", qidVoipFlashClientWriteRcv, cidVoipFlashClientWriteRcv );
	
	/* clean all rcv msg */
	// TODO: need it ?
	//while( msgrcv( qidVoipFlashClientWriteRcv, &msgSuper, VFWM_PAYLOAD_SIZE( msgSuper ), 0, IPC_NOWAIT ) >=0 )
	//	fprintf( stderr, "Clean flash client write rcv buffer.\n" );

label_flash_write_init_done:
	
	return 0;
}

static void voip_flash_client_write_close( void )
{
	
}

int voip_flash_client_write( unsigned long offset, unsigned long len, const unsigned char *pData, unsigned int bIgnoreIntr )
{
	VoipFlashWriteMsgWrite_t msgWrite;
	VoipFlashWriteMsg_t msgSuper;
	unsigned long curLen;
	unsigned char *pDestData = ( unsigned char * )&g_pVoIPShare ->voip_cfg + offset;
	
	while( len ) {
		
		/* check length */
		if( len > MAX_WRITE_LENGTH )
			curLen = MAX_WRITE_LENGTH;
		else
			curLen = len;
			
		/* check offset/curLen */
		if( curLen == 0 )
			break;
			
		if( offset >= sizeof( voipCfgParam_t ) )
			break;
			
		if( offset + curLen > sizeof( voipCfgParam_t ) )
			curLen = sizeof( voipCfgParam_t ) - offset;
	
		/* payload */
		msgWrite.type = VFWM_CMD_WRITE;
		msgWrite.len = VFWM_PAYLOAD_SIZE( msgWrite ) - sizeof( msgWrite.len );
		msgWrite.cid = cidVoipFlashClientWriteRcv;
		msgWrite.data_offset = offset;
		msgWrite.data_len = curLen;
		memcpy( msgWrite.data, pData, curLen );
		
		/* also write to client buffer */
		memcpy( pDestData, pData, curLen );

		/* offset is based on voipCfgParam_t */
		msgsnd( qidVoipFlashClientWriteSnd, &msgWrite, VFWM_PAYLOAD_SIZE( msgWrite ), 0 );
	
		/* wait for ack */
		while( 1 ) {
			if( msgrcv( qidVoipFlashClientWriteRcv, &msgSuper, VFWM_PAYLOAD_SIZE( msgSuper ), 0, 0 ) < 0 )
			{
				/*
				 * About bIgnoreIntr: 
				 * see voip_flash_client_validate_writing()
				 */
				if( bIgnoreIntr && errno == EINTR )	/* the process caught a signal */
					continue;
				else {	/* TODO: how to deal with other error?? */
					printf( "Rcv wa error!!!(%d)\n", errno );
					break;
				}
			} else
				break;
		}
		
		if( msgSuper.header.type == VFWM_CMD_WRITE_ACK )
			printf( "Rcv write ack\n" );
		else
			printf( "Not write ack. %ld\n", msgSuper.header.type );
			
		/* post modifier */
		offset += curLen;
		len -= curLen;
		pData += curLen;
		pDestData += curLen;
	} /* until len == 0 */
		
	return 0;
}

int voip_flash_client_validate_writing( unsigned int bIgnoreIntr )
{
	VoipFlashWriteMsgValidate_t msgValidate;
	VoipFlashWriteMsg_t msgSuper;
	
	/* validate */
	msgValidate.type = VFWM_CMD_VALIDATE;
	msgValidate.len = VFWM_PAYLOAD_SIZE( msgValidate ) - sizeof( msgValidate.len );
	msgValidate.cid = cidVoipFlashClientWriteRcv;
	
	msgsnd( qidVoipFlashClientWriteSnd, &msgValidate, VFWM_PAYLOAD_SIZE( msgValidate ), 0 );

	/* wait for ack */
	while( 1 ) {
		if( msgrcv( qidVoipFlashClientWriteRcv, &msgSuper, VFWM_PAYLOAD_SIZE( msgSuper ), 0, 0 ) < 0 )
		{
			/*
			 * About bIgnoreIntr: 
			 * In some process, they will catch a signal, and it will interrupt 
			 * message receiving. 
			 * Thus, one can use this parameter to ignore signals interrupt. 
			 */
			if( bIgnoreIntr && errno == EINTR )	/* the process caught a signal */
				continue;
			else {	/* TODO: how to deal with other error?? */
				printf( "Rcv va error!!!(%d)\n", errno );
				break;
			}
		} else
			break;
	}
	
	if( msgSuper.header.type == VFWM_CMD_VALIDATE_ACK )
		printf( "Rcv validate ack\n" );
	else
		printf( "Not validate write ack. %ld\n", msgSuper.header.type );
		
	return 0;
}

/*+++++add by Jack for tr-069 configuration+++++*/
#if 0	// pkshih: move to goahead 
int cmd_reboot()
{
	printf("***%s:%s:%d***\n",__FILE__,__FUNCTION__,__LINE__);
	system("reboot");
	exit(0);
	return 0;
}

int do_cmd(const char *filename, char *argv [], int dowait)
{
	pid_t pid, wpid;
	int stat=0, st;
	
	if((pid = vfork()) == 0) {
		/* the child */
		char *env[3];
		
		signal(SIGINT, SIG_IGN);
		argv[0] = (char *)filename;
		env[0] = "PATH=/bin:/usr/bin:/etc:/sbin:/usr/sbin";
		env[1] = NULL;

		execve(filename, argv, env);

		printf("exec %s failed\n", filename);
		_exit(2);
	} else if(pid > 0) {
		if (!dowait)
			stat = 0;
		else {
			/* parent, wait till rc process dies before spawning */
			while ((wpid = wait(&st)) != pid)
				if (wpid == -1 && errno == ECHILD) { /* see wait(2) manpage */
					stat = 0;
					break;
				}
		}
	} else if(pid < 0) {
		printf("fork of %s failed\n", filename);
		stat = -1;
	}
	return st;
}

int va_cmd(const char *cmd, int num, int dowait, ...)
{
	va_list ap;
	int k;
	char *s;
	char *argv[24];
	int status;
	
	va_start(ap, dowait);
	
	for (k=0; k<num; k++)
	{
		s = va_arg(ap, char *);
		argv[k+1] = s;
	}
	
	argv[k+1] = NULL;
	status = do_cmd(cmd, argv, dowait);
	va_end(ap);
	
	return status;
}
#endif
/*-----end-----*/

#ifdef CONFIG_RTK_VOIP_PACKAGE_8186

int mib_set_8186( unsigned long id, void *data, unsigned long data_len )
{
	union {
		VoipFlashWriteMsg_t msgSuper;
		VoipFlashWriteMsgMibSet_t msgMibset;
		VoipFlashWriteMsgMibSetAck_t msgMibsetAck;
	} sh;
	unsigned long offset = 0, len = data_len, cur_len;
	
	if( data_len > MAX_MIBMSG_LENGTH ) {
		printf( "mib_set give a too large data_len(%ld>%d)\n", data_len, MAX_MIBMSG_ATOM );
		return 0;
	}
	
	while( len ) {
		
		cur_len = ( len >= MAX_MIBMSG_ATOM ? MAX_MIBMSG_ATOM : len );
		len -= cur_len;
		
		/* fill payload */
		sh.msgMibset.type = VFWM_CMD_MIBSET;
		sh.msgMibset.len = VFWM_PAYLOAD_SIZE( sh.msgMibset ) - sizeof( sh.msgMibset.len )
								- MAX_MIBMSG_ATOM + cur_len;
		sh.msgMibset.cid = cidVoipFlashClientWriteRcv;
		sh.msgMibset.data_id = id;
		sh.msgMibset.data_len = data_len;
		sh.msgMibset.data_offset = offset;
		memcpy( sh.msgMibset.data, 
				( unsigned char * )data + ( offset << MAX_MIBMSG_ORDER ), 
				cur_len );
				
		offset ++;
	
		/* send */
		msgsnd( qidVoipFlashClientWriteSnd, &sh.msgMibset, VFWM_PAYLOAD_SIZE( sh.msgMibset ), 0 );
	
		/* wait for ack */
		while( 1 ) {
			if( msgrcv( qidVoipFlashClientWriteRcv, &sh.msgSuper, VFWM_PAYLOAD_SIZE( sh.msgSuper ), 0, 0 ) < 0 )
			{
				if( /* bIgnoreIntr && */ errno == EINTR )	/* the process caught a signal */
					continue;
				else {	/* TODO: how to deal with other error?? */
					printf( "Rcv va error!!!(%d)\n", errno );
					break;
				}
			} else
				break;
		}
		
		if( sh.msgMibsetAck.type == VFWM_CMD_MIBSET_ACK )
			printf( "Rcv mib_set ack\n" );
		else
			printf( "Not mib_set ack. %ld\n", sh.msgMibsetAck.type );
			
	}

	return sh.msgMibsetAck.result;
}

int mib_get_8186( unsigned long id, void *data, unsigned long data_len )
{
	/* 
	 * NOTE: It will get NOTHING, if one uses this function to 
	 * retrieve MIB_VOIP_CFG due to apmib_get() ignore this ID. 
	 */
	union {
		VoipFlashWriteMsg_t msgSuper;
		VoipFlashWriteMsgMibGet_t msgMibget;
		VoipFlashWriteMsgMibGetAck_t msgMibgetAck;
	} sh;
	unsigned long offset = 0, len = data_len, cur_len;

	if( data_len > MAX_MIBMSG_LENGTH ) {
		printf( "mib_get give a too large data_len(%ld>%d)\n", data_len, MAX_MIBMSG_LENGTH );
		return 0;
	}
	
	while( len ) {
	
		cur_len = ( len >= MAX_MIBMSG_ATOM ? MAX_MIBMSG_ATOM : len );
		len -= cur_len;		
		
		/* fill payload */
		sh.msgMibget.type = VFWM_CMD_MIBGET;
		sh.msgMibget.len = VFWM_PAYLOAD_SIZE( sh.msgMibget ) - sizeof( sh.msgMibget.len );
		sh.msgMibget.cid = cidVoipFlashClientWriteRcv;
		sh.msgMibget.data_id = id;
		sh.msgMibget.data_len = data_len;
		sh.msgMibget.data_offset = offset;
	
		/* send */
		msgsnd( qidVoipFlashClientWriteSnd, &sh.msgMibget, VFWM_PAYLOAD_SIZE( sh.msgMibget ), 0 );
	
		/* wait for ack */
		while( 1 ) {
			if( msgrcv( qidVoipFlashClientWriteRcv, &sh.msgSuper, VFWM_PAYLOAD_SIZE( sh.msgSuper ), 0, 0 ) < 0 )
			{
				if( /* bIgnoreIntr && */ errno == EINTR )	/* the process caught a signal */
					continue;
				else {	/* TODO: how to deal with other error?? */
					printf( "Rcv va error!!!(%d)\n", errno );
					break;
				}
			} else
				break;
		}
		
		if( sh.msgMibgetAck.type == VFWM_CMD_MIBGET_ACK ) {
			printf( "Rcv mib_get ack\n" );
	
			/* copy data from server's ack */
			memcpy( ( unsigned char * )data + ( offset << MAX_MIBMSG_ORDER ), 
					sh.msgMibgetAck.data, 
					cur_len );
		} else {
			printf( "Not mib_get ack. %ld\n", sh.msgMibgetAck.type );
			
			return 0;
		}
		
		offset ++;
	}

	return sh.msgMibgetAck.result;
}

int mib_update_8186( CONFIG_DATA_T /*unsigned long*/ update_type )
{
	union {
		VoipFlashWriteMsg_t msgSuper;
		VoipFlashWriteMsgMibUpdate_t msgMibUpdate;
		VoipFlashWriteMsgMibUpdateAck_t msgMibUpdateAck;
	} sh;

	/* fill payload */
	sh.msgMibUpdate.type = VFWM_CMD_MIBUPDATE;
	sh.msgMibUpdate.len = VFWM_PAYLOAD_SIZE( sh.msgMibUpdate ) - sizeof( sh.msgMibUpdate.len );
	sh.msgMibUpdate.cid = cidVoipFlashClientWriteRcv;
	sh.msgMibUpdate.update_type = update_type;

	/* send */
	msgsnd( qidVoipFlashClientWriteSnd, &sh.msgMibUpdate, VFWM_PAYLOAD_SIZE( sh.msgMibUpdate ), 0 );

	/* wait for ack */
	while( 1 ) {
		if( msgrcv( qidVoipFlashClientWriteRcv, &sh.msgSuper, VFWM_PAYLOAD_SIZE( sh.msgSuper ), 0, 0 ) < 0 )
		{
			if( /* bIgnoreIntr && */ errno == EINTR )	/* the process caught a signal */
				continue;
			else {	/* TODO: how to deal with other error?? */
				printf( "Rcv va error!!!(%d)\n", errno );
				break;
			}
		} else
			break;
	}
	
	if( sh.msgMibUpdateAck.type == VFWM_CMD_MIBUPDATE_ACK )
		printf( "Rcv mib_update ack\n" );
	else
		printf( "Not mib_update ack. %ld\n", sh.msgMibUpdateAck.type );

	return sh.msgMibUpdateAck.result;	
}
#endif	/* CONFIG_RTK_VOIP_PACKAGE_8186 */

#endif /* SUPPORT_VOIP_FLASH_WRITE */

