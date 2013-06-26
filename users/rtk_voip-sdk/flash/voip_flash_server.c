#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include "voip_flash.h"
#include "voip_types.h"
#include "voip_params.h"
#include "voip_control.h"
#if CONFIG_RTK_VOIP_PACKAGE_867X
#include "web_voip.h"
#endif /* CONFIG_RTK_VOIP_PACKAGE_867X */
#include "voip_ioctl.h"

static int sem_id= -1;
static int shm_id = -1;
static voip_flash_share_t *g_shmVoIPCfg = NULL;

#ifdef SUPPORT_VOIP_FLASH_WRITE
static int voip_flash_server_write_thread_start();
static void voip_flash_server_write_thread_stop();
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

int voip_flash_server_update()
{
	voip_flash_share_t aVoIPShare;
	int bTheSameVoIPSetting = 0;

	// get voip share data from flash
	if (voip_flash_server_read(&aVoIPShare) == -1)
	{
		fprintf(stderr, "Get VoIP flash failed\n");
		return -1;
	}

	if (sem_wait(VOIP_SEM_MUTEX) == -1)
		return -1;

	if (memcmp(&g_shmVoIPCfg->voip_cfg, &aVoIPShare.voip_cfg, sizeof(voipCfgParam_t)) == 0)
	{
		bTheSameVoIPSetting = 1;
	}
#if CONFIG_RTK_VOIP_PACKAGE_865X
	else
	{
		// save current VoIP Settings to file
		web_voip_saveConfig();
	}
#elif CONFIG_RTK_VOIP_PACKAGE_867X
	else{
		voipCfgAll_t	VoIPallconfig; 
		voipCfgParam_t *pVoIPCfg;
		if (voip_flash_get(&pVoIPCfg) != 0)
			return -1;

		memset(&VoIPallconfig, 0, sizeof(VoIPallconfig));
		memcpy(&VoIPallconfig.current_setting,pVoIPCfg, sizeof(voipCfgParam_t));
		VoIPallconfig.mode |= VOIP_CURRENT_SETTING ;
		//if(-1 == flash_voip_export_to_file(pVoIPCfg, VOIP_CONFIG_PATH, 1))
		if(-1 == flash_voip_export_to_file(&VoIPallconfig, VOIP_CONFIG_PATH, 1))
			fprintf(stderr,"flash voiop export to file fail\n");
	}
#endif

	// copy flash data to share memory
	memcpy(g_shmVoIPCfg, &aVoIPShare, sizeof(*g_shmVoIPCfg));

	if (sem_signal(VOIP_SEM_MUTEX) == -1)
		return -1;

	return bTheSameVoIPSetting;
}

void voip_flash_server_stop()
{
	if (g_shmVoIPCfg != NULL)
	{
		if (shmdt(g_shmVoIPCfg) == -1)
			fprintf(stderr, "shmdt failed: %s\n", strerror(errno));
	}

	if (shm_id != -1)
	{
		if (shmctl(shm_id, IPC_RMID, 0) == -1)
			fprintf(stderr, "shmctl failed: %s\n", strerror(errno));
	}

	if (sem_id != -1)
	{
		if (semctl(sem_id, 0, IPC_RMID) == -1)
			fprintf(stderr, "semctl failed: %s\n", strerror(errno));
	}

#ifdef SUPPORT_VOIP_FLASH_WRITE
	voip_flash_server_write_thread_stop();
#endif
}

int voip_flash_server_start()
{
	const VoipFeature_t voip_feature = g_VoIP_Feature;
	VoipFeature_t voip_feature_in_flash;
	key_t key;
	voipCfgParam_t *cfg;

	if( voip_flash_get( &cfg ) < 0 ) {
		return -1;
	} else {
		voip_feature_in_flash = 
					VOIP_FLASH_2_SYSTEM_FEATURE( cfg ->feature, 
												 cfg ->extend_feature );
	}
	
	if ( voip_feature != voip_feature_in_flash )
	{
		fprintf(stderr, "VoIP Feature is invalid (%llx vs %llx)\n",
			voip_feature_in_flash, voip_feature);
		return -1;
	}

	// create 2 semaphore: 
	//	0: event semaphore, using for checking server init is ok
	//	1: mutex semaphore, using for protect share memory (voip flash)
	key = ftok(VOIP_PATHNAME, VOIP_FLASH_VER);
	if (key == -1)
	{
		fprintf(stderr, "ftok failed: %s\n", strerror(errno));
		return -1;
	}

	sem_id = semget(key, 2, 0666 | IPC_CREAT);
	if (sem_id == -1)
	{
		fprintf(stderr, "semget failed: %s\n", strerror(errno));
		return -1;
	}
	
	// init mutex semaphore
	if (semctl(sem_id, VOIP_SEM_MUTEX, SETVAL, 1) == -1)
	{
		fprintf(stderr, "semctl SETVAL failed: %s\n", strerror(errno));
		goto voip_flash_server_start_failed;
	}
	
	fprintf(stderr, "voip flash server share size = %d\n", sizeof(voip_flash_share_t));

	// create share memory (voip flash)
	shm_id = shmget(key, sizeof(voip_flash_share_t), 0666 | IPC_CREAT);
	if (shm_id == -1)
	{
		fprintf(stderr, "shmget failed: %s\n", strerror(errno));
		goto voip_flash_server_start_failed;
	}

	g_shmVoIPCfg = shmat(shm_id, NULL, 0);
	if (g_shmVoIPCfg == (void *)-1)
	{
		fprintf(stderr, "shmat failed: %s\n", strerror(errno));
		goto voip_flash_server_start_failed;
	}

	// copy flash data to share memory
	voip_flash_server_update();

	sleep(1);

	// signal waiting client
	sem_signal(VOIP_SEM_EVENT);
	sem_signal(VOIP_SEM_EVENT);	/* allow 2 client instances */
#ifdef SUPPORT_VOIP_FLASH_WRITE	
	sem_signal(VOIP_SEM_EVENT);	/* allow 3 client instances */
#endif
	sem_signal(VOIP_SEM_EVENT);	/* allow 4 client instances: fwupdate */

#ifdef SUPPORT_VOIP_FLASH_WRITE	
	voip_flash_server_write_thread_start();
#endif
	
	return 0;

voip_flash_server_start_failed:
	voip_flash_server_stop();
	return -1;
}

// ---------------------------------------------------------------
// Flash Write  
// ---------------------------------------------------------------

#ifdef SUPPORT_VOIP_FLASH_WRITE
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#if CONFIG_RTK_VOIP_PACKAGE_865X
	#include "rtl_cfgmgr.h"
	#include "rtl_board.h"
	#include <sys/shm.h>
#elif CONFIG_RTK_VOIP_PACKAGE_867X
	#include <sys/shm.h>
	#include "mib.h"
#else
	#include "apmib.h"
#endif

#include "voip_flash_client.h"

static int qidVoipFlashServerWriteRcv;
static int qidVoipFlashServerWriteAck[ MAX_VOIP_FLASH_WRITE_CLIENT ];
static int bVoipFlashServerRunning = 1;

static void VoipFlashServerWriteCoreThread( void *ptr );

static int voip_flash_server_write_thread_start()
{
#if 1	// use fork instead of pthread 
	pid_t pid;
	
	if( ( pid = fork() ) ) {
		return 1;
	}

	VoipFlashServerWriteCoreThread( NULL );
	exit(0);
#else	
	pthread_t thFlashWrite;

	pthread_create( &thFlashWrite, NULL,
                    (void*)&VoipFlashServerWriteCoreThread, (void*) NULL);
#endif

	return 1;
}

static void voip_flash_server_write_thread_stop()
{
	VoipFlashWriteMsgNop_t msgNop;
	
	bVoipFlashServerRunning = 0;
	
	msgsnd( qidVoipFlashServerWriteRcv, &msgNop, VFWM_PAYLOAD_SIZE( msgNop ), 0 );
}

static void InitializeVoipFlashServerWrite( void )
{
	key_t key;
	unsigned int i;

	key = ftok( VOIP_FLASH_WRITE_PATHNAME, VOIP_FLASH_WRITE_SERVER );
	
	if( ( qidVoipFlashServerWriteRcv = msgget( key, 0666 | IPC_CREAT ) ) == -1 ) {
		fprintf( stderr, "Create flash server write rcv event queue fail.\n" );
	}

	for( i = 0; i < MAX_VOIP_FLASH_WRITE_CLIENT; i ++ ) {
		key = ftok( VOIP_FLASH_WRITE_PATHNAME, i + VOIP_FLASH_WRITE_CLIENT_BASE );
		
		if( ( qidVoipFlashServerWriteAck[ i ] = msgget( key, 0666 | IPC_CREAT ) ) == -1 ) {
			fprintf( stderr, "Create flash server write ack event queue fail(%u).\n", i );
		}
	}
}

static void TerminateVoipFlashServerWrite( void )
{
	unsigned int i;

	if( qidVoipFlashServerWriteRcv != -1 ) {
		if( msgctl( qidVoipFlashServerWriteRcv, IPC_RMID, 0 ) == -1 ) {
			fprintf( stderr, "msgctl(flash write rcv) fail: %d\n", errno );
        }
	}

	for( i = 0; i < MAX_VOIP_FLASH_WRITE_CLIENT; i ++ ) {
		if( qidVoipFlashServerWriteAck[ i ] != -1 ) {
			if( msgctl( qidVoipFlashServerWriteAck[ i ], IPC_RMID, 0 ) == -1 ) {
				fprintf( stderr, "msgctl(flash write ack) fail: %d, %u\n", errno, i );
	        }
		}
	}
}

static void VoipFlashServerWriteCoreThread( void *ptr )
{
	static union {
		VoipFlashWriteMsg_t msgSuper;
		VoipFlashWriteMsgWrite_t msgWrite;
		VoipFlashWriteMsgValidate_t msgValidate;
		VoipFlashWriteMsgMibSet_t msgMibSet;
		VoipFlashWriteMsgMibGet_t msgMibGet;
		VoipFlashWriteMsgMibUpdate_t msgMibUpdate;
	} shr;	/* SHare with Receiving */
	static union {
		VoipFlashWriteMsgWriteAck_t msgWriteAck;
		VoipFlashWriteMsgValidateAck_t msgValidateAck;
		VoipFlashWriteMsgMibSetAck_t msgMibSetAck;
		VoipFlashWriteMsgMibGetAck_t msgMibGetAck;
		VoipFlashWriteMsgMibUpdateAck_t msgMibUpdateAck;
	} shs;	/* SHare with Sending */
	cid_t cid;
	static unsigned char mibBuffer[ MAX_VOIP_FLASH_WRITE_CLIENT ][ MAX_MIBMSG_LENGTH ];
	unsigned long cur_len = 0;

	/* init */
	InitializeVoipFlashServerWrite();
	
	/* main loop */
	while( bVoipFlashServerRunning ) {
		
		if( msgrcv( qidVoipFlashServerWriteRcv, &shr.msgSuper, VFWM_PAYLOAD_SIZE( shr.msgSuper ), 
					0, 0 ) < 0 )
		{
			continue;
		}
		
		switch( shr.msgSuper.header.type ) {
		case VFWM_CMD_WRITE:

			/* check length */
			if( shr.msgWrite.data_len == 0 )
				goto label_do_write_ack;
		
			/* check offset */
			if( shr.msgWrite.data_offset >= sizeof( voipCfgParam_t ) ) {
				fprintf( stderr, "VFWM_CMD_WRITE: offset is too large. (%ld,%d)\n", shr.msgWrite.data_offset, sizeof( voipCfgParam_t ) );
				
				//#define MY_FIELD_OFFSET( typ, fld )		( ( unsigned long )&( ( ( typ * )0 ) ->fld ) )
				//fprintf( stderr, "      all\tAP\tUI\n" );
				//fprintf( stderr, "Size: %d\t%d\t%d\n", sizeof( APMIB_T ), sizeof( voipCfgParam_t ), sizeof( ui_falsh_layout_t ) );
				//fprintf( stderr, "Offset: %d\t%ld\t%ld\n", 0, MY_FIELD_OFFSET( APMIB_T, voipCfgParam ), MY_FIELD_OFFSET( APMIB_T, voipCfgParam.ui ) );
				//#undef MY_FIELD_OFFSET
				goto label_do_write_ack;
			}
			
			/* check overwrite */
			if( shr.msgWrite.data_offset + shr.msgWrite.data_len > sizeof( voipCfgParam_t ) ) {
				fprintf( stderr, "VFWM_CMD_WRITE: overwrite\n" );
				shr.msgWrite.data_len = sizeof( voipCfgParam_t ) - shr.msgWrite.data_offset;
			}
			
			//printf( "test before:%lX\n", *( ( unsigned long * )&pMib ->voipCfgParam.ui ) );

			/* offset is related to pMib ->voipCfgParam */			
			memcpy( ( unsigned char * )&pMib ->voipCfgParam + 
												shr.msgWrite.data_offset, 
					shr.msgWrite.data, shr.msgWrite.data_len );
			
			//printf( "test after:%lX\n", *( ( unsigned long * )&pMib ->voipCfgParam.ui ) );
					
label_do_write_ack:
			
			printf( "Write\n" );

			/* write ack */
			shs.msgWriteAck.type = VFWM_CMD_WRITE_ACK;
			cid = shs.msgWriteAck.cid = shr.msgWrite.cid;
			
			if( cid >= MAX_VOIP_FLASH_WRITE_CLIENT )
				fprintf( stderr, "cid too large 1\n" );
			else if( msgsnd( qidVoipFlashServerWriteAck[ cid ], &shs.msgWriteAck, VFWM_PAYLOAD_SIZE( shs.msgWriteAck ), 0 ) < 0 ) 
			{
				fprintf( stderr, "Snd wa error!!!(%d)\n", errno );
			}

			break;
						
		case VFWM_CMD_VALIDATE:
			/* write to flash */
			//memcpy(&pMib->voipCfgParam, &cfg_all->current_setting, sizeof(voipCfgParam_t));
			//err = 
			apmib_updateFlash(CURRENT_SETTING, (char *) pMib, sizeof(APMIB_T), 2, CURRENT_SETTING_VER);
			/* update share memory */
			voip_flash_server_update();
			
			// FIXME: 
			/* notify other process to update flash content */
			
			//printf( "[write thread APMIB_T size: %d]\n", sizeof(APMIB_T) );
			
			//printf( "test validate:%lX\n", *( ( unsigned long * )&pMib ->voipCfgParam.ui ) );
			
			printf( "validate\n" );

			/* validate ack */
			shs.msgValidateAck.type = VFWM_CMD_VALIDATE_ACK;
			cid = shs.msgValidateAck.cid = shr.msgValidate.cid;
			
			if( cid >= MAX_VOIP_FLASH_WRITE_CLIENT )
				fprintf( stderr, "cid too large 2\n" );
			else if( msgsnd( qidVoipFlashServerWriteAck[ cid ], &shs.msgValidateAck, VFWM_PAYLOAD_SIZE( shs.msgValidateAck ), 0 ) < 0 )
			{
				fprintf( stderr, "Snd va error!!!(%d)\n", errno );
			}

			break;

  #ifdef CONFIG_RTK_VOIP_PACKAGE_8186
		case VFWM_CMD_MIBSET:
		
			cid = shr.msgMibSet.cid;
		
			if( cid >= MAX_VOIP_FLASH_WRITE_CLIENT ) {
				shs.msgMibSetAck.result = 0;
				fprintf( stderr, "mibset not support cid:%d\n", cid );
				goto label_mib_set_process_done;
			}
		
			/* mib_set */
			if( shr.msgMibSet.data_len > MAX_MIBMSG_ATOM ) {
				if( shr.msgMibSet.data_offset >= MAX_MIBMSG_ATOM_NUM ) {
					shs.msgMibSetAck.result = 0;
					fprintf( stderr, "mibset data_offset is too large\n" );
					goto label_mib_set_process_done;
				} 
				
				if( shr.msgMibSet.data_offset == 
					( shr.msgMibSet.data_len - 1 ) >> MAX_MIBMSG_ORDER )
				{
					/* last atom */
					memcpy( &mibBuffer[ cid ][ shr.msgMibSet.data_offset << MAX_MIBMSG_ORDER ],
							shr.msgMibSet.data,
							MAX_MIBMSG_MOD( shr.msgMibSet.data_len ) );					
				} else {
					/* middle atom, so copy only */
					memcpy( &mibBuffer[ cid ][ shr.msgMibSet.data_offset << MAX_MIBMSG_ORDER ],
							shr.msgMibSet.data,
							MAX_MIBMSG_ATOM );
					
					shs.msgMibSetAck.result = 1;
					goto label_mib_set_process_done;
				}
						
				shs.msgMibSetAck.result =
					apmib_set( shr.msgMibSet.data_id, &mibBuffer[ cid ][ 0 ] );				
				
			} else {
				shs.msgMibSetAck.result =
					apmib_set( shr.msgMibSet.data_id, shr.msgMibSet.data );
			}

label_mib_set_process_done:			
			/* mib_set ack */
			shs.msgMibSetAck.type = VFWM_CMD_MIBSET_ACK;
			shs.msgMibSetAck.len = VFWM_PAYLOAD_SIZE( shs.msgMibSetAck ) - sizeof( shs.msgMibSetAck.len );
			cid = shs.msgMibSetAck.cid = shr.msgMibSet.cid;
			/* shs.msgMibSetAck.result is set before! */
			
			if( cid >= MAX_VOIP_FLASH_WRITE_CLIENT )
				fprintf( stderr, "cid too large 3\n" );
			else if( msgsnd( qidVoipFlashServerWriteAck[ cid ], &shs.msgMibSetAck, VFWM_PAYLOAD_SIZE( shs.msgMibSetAck ), 0 ) < 0 )
			{
				fprintf( stderr, "Snd msa error!!!(%d)\n", errno );
			}
		
			break;

		case VFWM_CMD_MIBGET:

			cid = shr.msgMibGet.cid;
		
			if( cid >= MAX_VOIP_FLASH_WRITE_CLIENT ) {
				shs.msgMibGetAck.result = 0;
				fprintf( stderr, "mibget not support cid:%d\n", cid );
				goto label_mib_get_process_done;
			}

			/* mib_get */
			if( shr.msgMibGet.data_len > MAX_MIBMSG_ATOM ) {
				if( shr.msgMibGet.data_offset >= MAX_MIBMSG_ATOM_NUM ) {
					shs.msgMibGetAck.result = 0;
					fprintf( stderr, "mibget data_offset is too large\n" );
					goto label_mib_get_process_done;
				} 
				
				if( shr.msgMibGet.data_offset == 0 ) {
					/* first atom */
					shs.msgMibGetAck.result = 
						apmib_get( shr.msgMibGet.data_id, &mibBuffer[ cid ][ 0 ] );
						
					if( shs.msgMibGetAck.result == 0 )
						goto label_mib_get_process_done;
				}
				
				/* copy into message structure */
				if( shr.msgMibGet.data_offset == 
					( shr.msgMibGet.data_len - 1 ) >> MAX_MIBMSG_ORDER )
				{
					/* last atom */
					cur_len = MAX_MIBMSG_MOD( shr.msgMibGet.data_len );
				} else {
					/* first or middle atom */
					cur_len = MAX_MIBMSG_ATOM;
				}
				
				memcpy( shs.msgMibGetAck.data,
							&mibBuffer[ cid ][ shr.msgMibGet.data_offset << MAX_MIBMSG_ORDER ],
							cur_len );
				
				shs.msgMibGetAck.result = 1;
			} else {
				cur_len = shr.msgMibGet.data_len;
			
				shs.msgMibGetAck.result = 
					apmib_get( shr.msgMibGet.data_id, shs.msgMibGetAck.data );
			}

label_mib_get_process_done:				
			/* mib_get ack */
			shs.msgMibGetAck.type = VFWM_CMD_MIBGET_ACK;
			shs.msgMibGetAck.data_id = shr.msgMibGet.data_id;
			shs.msgMibGetAck.data_len = shr.msgMibGet.data_len;
			shs.msgMibGetAck.data_offset = shr.msgMibGet.data_offset;
			shs.msgMibGetAck.len = VFWM_PAYLOAD_SIZE( shs.msgMibGetAck ) - sizeof( shs.msgMibGetAck.len )
									- MAX_MIBMSG_ATOM + cur_len;
			cid = shs.msgMibGetAck.cid = shr.msgMibGet.cid;
			
			if( cid >= MAX_VOIP_FLASH_WRITE_CLIENT )
				fprintf( stderr, "cid too large 4\n" );
			else if( msgsnd( qidVoipFlashServerWriteAck[ cid ], &shs.msgMibGetAck, shs.msgMibGetAck.len + sizeof( shs.msgMibGetAck.len ), 0 ) < 0 )
			{
				fprintf( stderr, "Snd mga error!!!(%d)\n", errno );
			}
			
			break;
			
		case VFWM_CMD_MIBUPDATE:
			/* mib_update */
			shs.msgMibUpdateAck.result = 
					apmib_update( shr.msgMibUpdate.update_type );
			
			/* mib_update ack */
			shs.msgMibUpdateAck.type = VFWM_CMD_MIBUPDATE_ACK;
			shs.msgMibUpdateAck.len = VFWM_PAYLOAD_SIZE( shs.msgMibGetAck ) - sizeof( shs.msgMibGetAck.len );
			cid = shs.msgMibUpdateAck.cid = shr.msgMibUpdate.cid;
			
			if( cid >= MAX_VOIP_FLASH_WRITE_CLIENT )
				fprintf( stderr, "cid too large 5\n" );
			else if( msgsnd( qidVoipFlashServerWriteAck[ cid ], &shs.msgMibUpdateAck, VFWM_PAYLOAD_SIZE( shs.msgMibGetAck ), 0 ) < 0 )
			{
				fprintf( stderr, "Snd mua error!!!(%d)\n", errno );
			}
			
			break;
  #endif /* CONFIG_RTK_VOIP_PACKAGE_8186 */
			
		case VFWM_CMD_NOP:
			/* do nothing */
			break;
		
		default:
			fprintf( stderr, "unexpected message: %ld\n", shr.msgSuper.header.type );
			break;
		}
	}

	/* terminate */
	TerminateVoipFlashServerWrite();
}
#endif /* SUPPORT_VOIP_FLASH_WRITE */

