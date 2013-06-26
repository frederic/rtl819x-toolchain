#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

/* included in rtk_voip/include/ */
#if 1
#include "voip_manager.h"
#else
#include "type.h"
#include "voip_params.h"
#include "voip_control.h"
#endif

#include "voip_ioctl.h"

int rtk_SetIvrPlayG72363( int chid, unsigned int nCount, const unsigned char *pData )
{
	TstVoipPlayIVR_G72363 stVoipPlayIVR;
	
	if( nCount > MAX_FRAMES_OF_G72363 )
		nCount = MAX_FRAMES_OF_G72363;
	
	stVoipPlayIVR.ch_id = chid;
	stVoipPlayIVR.type = IVR_PLAY_TYPE_G723_63;
	stVoipPlayIVR.nFramesCount = nCount;
	memcpy( stVoipPlayIVR.data, pData, nCount * 24 );

    SETSOCKOPT(VOIP_MGR_PLAY_IVR, &stVoipPlayIVR, TstVoipPlayIVR_G72363, 1);
    
    printf( "IVR playing time: %u0 ms\n", stVoipPlayIVR.playing_period_10ms );
    printf( "\tCopied data:%d\n", stVoipPlayIVR.nRetCopiedFrames );
    
    return stVoipPlayIVR.nRetCopiedFrames;
}

int thread_done[ 2 ] = { 0, 0 };

void * main_thread( void *arg )
{
	FILE *fp;
	unsigned char buffer[ 24 * 10 ];
	unsigned int cRead, cWritten, shift;
	int chid = ( int )arg;
	
	if( ( fp = fopen( "723_raw", "rb" ) ) == NULL ) {
		printf( "Open error\n" );
		goto label_main_thread_done;;
	}
	
	while( 1 ) {
		
		cRead = fread( buffer, 24, 10, fp );
		shift = 0;
		
		if( cRead == 0 )	/* seen as eof */
			break;

lable_put_g72363_data:
		cWritten = rtk_SetIvrPlayG72363( chid, cRead, buffer + shift );
		//printf( "Write:%d, %d\n", cRead, shift );
		
		/* buffer is full */
		if( cWritten < cRead ) {
			
			
			printf( "Buffer is full.. Wait one second...\n" );
			//printf( "[%d:%d:%d]\n", cRead, cWritten, shift );
			rtk_SetIvrPlayG72363( chid, 0, buffer );	/* show current playing time */
			
			sleep( 1 );	
			
			cRead -= cWritten;
			shift += cWritten * 24;
			
			goto lable_put_g72363_data;
		}
	}
	
	fclose( fp );


label_main_thread_done:	
	thread_done[ chid ] = 1;
	
	//return 0;
	return NULL;
}


int main( int argc, char **argv )
{
	pthread_t thPlayG723;
	int ret;

	/* thread for channel 0 */
	ret = pthread_create( &thPlayG723, NULL,
                    (void*)&main_thread, (void*) 0);

	printf( "pthread_create 0: %d\n", ret );

	/* one channel only? */
	if( argc >= 2 && argv[ 1 ][ 0 ] == '1' )
		thread_done[ 1 ] = 1;
	
	/* thread for channel 1 */
	ret = pthread_create( &thPlayG723, NULL,
                    (void*)&main_thread, (void*) 1);

	printf( "pthread_create 1: %d\n", ret );

	while( !thread_done[ 0 ] || !thread_done[ 1 ] ) {
		printf( "wait complete.\n" );
		sleep( 1 );
	}

	return 0;
}

