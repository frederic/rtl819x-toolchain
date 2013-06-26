#include <linux/config.h>
#include <linux/signal.h>
#include <linux/netdevice.h>
#include <asm/io.h>
#include "rtk_voip.h"
#include "voip_types.h"
#include "t38_handler.h"
//#include "pcm_interface.h"
#include "voip_init.h"
#include "voip_proc.h"
#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
#include "resampler.h"
#endif
#include "codec_descriptor.h"
#include "silence_det.h"

#define FAX_TX_GAINUP 0		/* can be gain up 1~3 */
#define FAX_RX_GAINDN 0		/* can be gain down ? */

#ifdef T38_STAND_ALONE_HANDLER

#ifdef REDUCE_PCM_FIFO_MEMCPY
extern uint32* pRxBufTmp; // For reducing memcpy
extern uint32* pTxBufTmp;
#endif

int32 PCM_handler_T38( unsigned int chid )
{
	/* 
	 * Make sure ( t38RunningState[ chid ] == T38_START ) 
	 */
	extern int32 energy_det( int chid , short *buffer, int buflen );
	extern uint32 rxBuf_NB[ PCM_PERIOD_10MS_SIZE / 4 ]; 
	//unsigned long flags;
	extern uint32 T38_API_EncodeDecodeProcessAndDoSend( uint32 , const unsigned short *,	unsigned short *);
#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
  #ifdef AUDIOCODES_VOIP
	const int bWideband = 0;
	unsigned short *pRxBufTmp_nb = (unsigned short *)pRxBufTmp;
  #else
	const int bWideband = ( VoipChannelSampleRate[ chid ] == SAMPLE_WIDE_BAND ? 1 : 0 );
	unsigned short *pRxBufTmp_nb = (unsigned short *)pRxBufTmp;
  #endif
#else
	const int bWideband = 0;
	unsigned short * const pRxBufTmp_nb = (unsigned short *)pRxBufTmp;
#endif
	int32 energy_pcm_in, energy_pcm_out;
	
	
	/************ T.38 encode & ecode **********************/
#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
	if( bWideband ) {
		resampler_process_int_ex(pResampler_down_st, 
			T38HDR_RESAMPLER_OFFSET + chid, // see MAX_NB_CHANNELS to know detail  
			( const int16_t * )pRxBufTmp, PCM_PERIOD_10MS_SIZE/2*2, 
			( int16_t * )rxBuf_NB, PCM_PERIOD_10MS_SIZE/2,
			"T38HDR-down error = %d\n" );
		
		pRxBufTmp_nb = (unsigned short *)rxBuf_NB;
	}
#endif
	
	energy_pcm_in = energy_det(chid, (short *)(pRxBufTmp_nb), PCM_PERIOD_10MS_SIZE/2);
	
#ifdef REDUCE_PCM_FIFO_MEMCPY
#if FAX_RX_GAINDN
	int j;
	for( j = 0; j < 80; j ++ )
		*( ( int16 * )pRxBufTmp + j ) >>= FAX_RX_GAINDN;
#endif
	T38_API_EncodeDecodeProcessAndDoSend( chid, (unsigned short *)pRxBufTmp_nb, (unsigned short *)pTxBufTmp );
#if FAX_TX_GAINUP
	int i;
	for( i = 0; i < 80; i ++ )
		*( ( uint16 * )pTxBufTmp + i ) <<= FAX_TX_GAINUP;
#endif
#else
	???
#endif
	
	energy_pcm_out = energy_det(chid, (short *)(pTxBufTmp), PCM_PERIOD_10MS_SIZE/2);
	
	record_silence_det( chid, energy_pcm_in, energy_pcm_out );
	
#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
	if( bWideband ) {
		memcpy( rxBuf_NB, pTxBufTmp, PCM_PERIOD_10MS_SIZE );
		
		resampler_process_int_ex(pResampler_up_st, 
			T38HDR_RESAMPLER_OFFSET + chid, // see MAX_NB_CHANNELS to know detail  
			( const int16_t * )rxBuf_NB, PCM_PERIOD_10MS_SIZE/2, 
			( int16_t * )pTxBufTmp, PCM_PERIOD_10MS_SIZE/2*2,
			"T38HDR-up error = %d\n" );
	}
#endif

#if 0	// chmap 
#ifdef SUPPORT_PCM_FIFO
	//printk("%d ", tx_fifo_cnt_w[chid]);
	save_flags(flags); cli();
#ifdef REDUCE_PCM_FIFO_MEMCPY
	if (pcm_write_tx_fifo_done(chid))
#else
	if (pcm_write_tx_fifo(chid, &TxBufTmp[chid][0]))
#endif
	{
		printk("TF(T.38)\n");
	}
	restore_flags(flags);
#endif
#endif

	return 0;
}

int voip_t38_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int ch;
	int n = 0;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	if( IS_CH_PROC_DATA( data ) ) {
		ch = CH_FROM_PROC_DATA( data );
		n = sprintf( buf, "channel=%d\n", ch );
		n += sprintf( buf + n, "Running state=%d\n", t38RunningState[ ch ] );
		n += sprintf( buf + n, "Fax Tx/Rx Gain=%d/%d\n", FAX_TX_GAINUP, FAX_RX_GAINDN);

	} else {
	}
	
	*eof = 1;
	return n;
}

int __init voip_t38_proc_init( void )
{
	create_voip_channel_proc_read_entry( "t38", voip_t38_read_proc );

	return 0;
}

void __exit voip_t38_proc_exit( void )
{
	remove_voip_channel_proc_entry( "t38" );
}

voip_initcall_proc( voip_t38_proc_init );
voip_exitcall( voip_t38_proc_exit );

#endif /* T38_STAND_ALONE_HANDLER */

