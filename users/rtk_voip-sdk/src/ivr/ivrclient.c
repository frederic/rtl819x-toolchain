#include <sys/msg.h>
#include <errno.h>
#include <stdio.h>
#include "ivripc.h"
#include "ivrclient.h"

#ifndef IVRSERVER_SO
static int qidServer, qidClient;
static int bServerConnected;
#endif

void InitIvrClient( void )
{
#ifndef IVRSERVER_SO
	key_t key;
	int retry = 3;
	
	bServerConnected = 0;

	while( retry ) {
		key = ftok( IVR_SERVER_PATHNAME, IVR_SERVER_VERSION );
	
		if( ( qidServer = msgget( key, 0666 /* | IPC_CREAT | IPC_EXCL */ ) ) == -1 ) {
			printf( "msgget(S) fail: %X, %d:%s\n", key, errno, strerror( errno ) );
			goto label_msgget_fail;
		}
		
		key += IVR_CLIENT_KEY_OFFSET;
		
		if( ( qidClient = msgget( key, 0666 /* | IPC_CREAT | IPC_EXCL */ ) ) == -1 ) {
			printf( "msgget(C) fail: %X, %d: %s\n", key, strerror( errno ) );
			goto label_msgget_fail;
		}
		
		/* check if correct? */
		if( qidServer != -1 && qidClient != -1 ) {
			bServerConnected = 1;
			break;
		}

label_msgget_fail:		
		printf( "Wait for IVR server...(%d)\n", retry );
		
		if( -- retry )
			sleep( 1 );
	}
#endif
}

#ifdef IVRSERVER_SO
// define in libivrserver.so 
extern int IvrMessageSendRecv_IPC( void *msg, size_t size );
#else
static int IvrMessageSendRecv_IPC( void *msg, size_t size )
{
	ivr_ipc_ret_t ivr_ret;

	/* send a message */
	if( msgsnd( qidServer, msg, size, 0 ) == -1 ) {
		printf( "msgsnd fail: %d: %s\n", errno, strerror( errno ) );
	}	
	
	/* wait for resopnse */
	if( msgrcv( qidClient, &ivr_ret, IVR_RET_DATA_SIZE, 0, 0 ) == -1 ) {
		printf( "msgrcv fail: %d: %s\n", errno, strerror( errno ) );
	}
	
	return ivr_ret.ret;
}
#endif

int IsInstructionForIVR_IPC( ivr_ipc_msg_t *msg )
{
#ifndef IVRSERVER_SO
	/* Not connected */
	if( !bServerConnected )
		return 0;
#endif
	
	return IvrMessageSendRecv_IPC( msg, sizeof( do_ivr_ins_t ) );
}

void SendGlobalConstantToIVR_IPC( int nMaxCodec, const int *pMapSupportedCodec )
{
	ivr_ipc_msg_t msg;
	int i;
	
#ifndef IVRSERVER_SO
	/* Not connected */
	if( !bServerConnected )
		return;
#endif
	
	msg.message_type	= IVR_IPC_MSG_SEND_GLOBAL;

	msg.send_global.nMaxCodec = nMaxCodec;
	
	for( i = 0; i < _CODEC_MAX; i ++ )
		msg.send_global.mapSupportedCodec[ i ] = pMapSupportedCodec[ i ];
	
	
	IvrMessageSendRecv_IPC( &msg, sizeof( send_global_t ) );
}

void NoticeIVRUpdateFlash_IPC( void )
{
	ivr_ipc_msg_t msg;

#ifndef IVRSERVER_SO
	/* Not connected */
	if( !bServerConnected )
		return;
#endif

	msg.message_type	= IVR_IPC_MSG_UPDATE_FLASH;	
	
	msg.update_flash.reserved = 0;

	IvrMessageSendRecv_IPC( &msg, sizeof( update_flash_t ) );
}

