#include <linux/string.h>
#include "rtk_voip.h"
#include "voip_types.h"

#include "section_def.h"

#include "con_register.h"

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651
#define SYNC_POINT 3					/* sync point(8651B GNET EVB) =  3 pages + 8 samples ~ 60ms ago */
#define SYNC_SAMPLE 8
#define SYNC_POINT_DAA 3
#define SYNC_SAMPLE_DAA 8
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
#ifndef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
#define SYNC_POINT 3					/* sync point(8186 V210, V110, V100) =  3 pages + 8 samples ~ 40ms ago */
#ifdef AUDIOCODES_VOIP
#define SYNC_SAMPLE 8
#else
#define SYNC_SAMPLE 8
#endif
#define SYNC_POINT_DAA 3
#define SYNC_SAMPLE_DAA 8
#else
#define SYNC_POINT 3					/* sync point(8186 IP Phone V101) =  2 pages + 60 samples */
#ifdef AUDIOCODES_VOIP
#define SYNC_SAMPLE 8
#else
#define SYNC_SAMPLE 8
#endif
#ifdef AUDIOCODES_VOIP
#define SYNC_POINT_DAA 3
#define SYNC_SAMPLE_DAA 0
#else
#define SYNC_POINT_DAA 2
#define SYNC_SAMPLE_DAA 60
#endif
#endif // CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
#endif // CONFIG_RTK_VOIP_DRIVERS_PCM8186

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8671
#define SYNC_POINT 3
#define SYNC_SAMPLE 8
#define SYNC_POINT_DAA 3
#define SYNC_SAMPLE_DAA 8
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8672
#define SYNC_POINT 3
#define SYNC_SAMPLE 8
#define SYNC_POINT_DAA 3
#define SYNC_SAMPLE_DAA 8
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8676
#define SYNC_POINT 3
#define SYNC_SAMPLE 8
#define SYNC_POINT_DAA 3
#define SYNC_SAMPLE_DAA 8
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM865xC
#ifdef AUDIOCODES_VOIP
#define SYNC_POINT 3
#define SYNC_SAMPLE 4	
#define SYNC_POINT_DAA 3
#define SYNC_SAMPLE_DAA 4
#else
#define SYNC_POINT 3
#define SYNC_SAMPLE 8	
#define SYNC_POINT_DAA 3
#define SYNC_SAMPLE_DAA 8				
#endif
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY
#ifdef AUDIOCODES_VOIP
#define SYNC_POINT 3
#define SYNC_SAMPLE 4	
#define SYNC_POINT_DAA 3
#define SYNC_SAMPLE_DAA 4
#else
#define SYNC_POINT 3
#define SYNC_SAMPLE 8	
#define SYNC_POINT_DAA 3
#define SYNC_SAMPLE_DAA 8
#endif
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM89xxC
#ifdef AUDIOCODES_VOIP
#define SYNC_POINT 3
#define SYNC_SAMPLE 4	
#define SYNC_POINT_DAA 3
#define SYNC_SAMPLE_DAA 4
#else
#define SYNC_POINT 3
#define SYNC_SAMPLE 8	
#define SYNC_POINT_DAA 3
#define SYNC_SAMPLE_DAA 8
#endif
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM89xxD
#ifdef AUDIOCODES_VOIP
#define SYNC_POINT 3
#define SYNC_SAMPLE 4	
#define SYNC_POINT_DAA 3
#define SYNC_SAMPLE_DAA 4
#else
#define SYNC_POINT 3
#define SYNC_SAMPLE 8	
#define SYNC_POINT_DAA 3
#define SYNC_SAMPLE_DAA 8
#endif
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
  #ifdef CONFIG_RTK_VOIP_DRIVERS_CODEC_WM8510
  #define CODEC_GROUP_DELAY 40
  #elif defined( CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621 )
  #define CODEC_GROUP_DELAY 10
  #endif
#else
#define CODEC_GROUP_DELAY 0
#endif

#ifdef SUPPORT_LEC_G168_ISR
#define FSIZE 4

__attribute__((aligned(8)))
#ifndef LEC_USE_CIRC_BUF
__pcmifdata00 static Word16 LEC_RinBuf[CON_CH_NUM][80*PCM_PERIOD*FSIZE*MAX_BAND_FACTOR];   /* buf size = 80*4*PCM_PERIOD(ex: 2) samples = 80ms */  
#endif

#ifdef LEC_USE_CIRC_BUF
static Word16 LEC_Rin_CircBuf[CON_CH_NUM][80*PCM_PERIOD*FSIZE*MAX_BAND_FACTOR+SYNC_SAMPLE+CODEC_GROUP_DELAY];/*using circular buffer, size = 80*4*PCM_PERIOD(ex: 2) samples = 80ms*/ /* add SYNC_SAMPLE to write data */
static Word16 LEC_buf_windex[CON_CH_NUM];                                  /* circular buffer write index */
static Word16 LEC_buf_rindex[CON_CH_NUM];                                  /* circular buffer read index */
#endif   //end of LEC_USE_CIRC_BUF

#ifdef LEC_G168_ISR_SYNC_P
static short vpat[16]={32767,30272,23170,12539,0,-12539,-23170,-30272,-32767,-30272,-23170,-12539, 0,12539,23170,30272 };
static char sync = 0;
static int cnt_time=0, sync_frame=0, sync_frame_R=0, sync_start=0;
static int level = 2000;
#endif

static int sync_point[CON_CH_NUM] = {0};
static int sync_sample[CON_CH_NUM] = {0};
static int sync_sample_offset_daa[CON_CH_NUM] = {0};
#endif


void lec_buf_init_sync_point( voip_con_t *this )
{
	const uint32 cch = this ->cch;
	
	if( Is_DAA_snd( this ->snd_ptr ) ) {
		sync_point[ cch ] = SYNC_POINT_DAA;
		sync_sample[ cch ] = SYNC_SAMPLE_DAA;
		sync_sample_offset_daa[ cch ] = (80*SYNC_POINT+SYNC_SAMPLE) - (80*SYNC_POINT_DAA+SYNC_SAMPLE_DAA);
	} else {
		sync_point[ cch ] = SYNC_POINT;
		sync_sample[ cch ] = SYNC_SAMPLE;
		sync_sample_offset_daa[ cch ] = 0;
	}
}

void lec_buf_flush( voip_con_t *this )
{
#ifdef LEC_USE_CIRC_BUF	
	const uint32 cch = this ->cch;
	
	LEC_buf_windex[cch] =sync_point[cch]-1;
	//LEC_buf_windex[cch] =2;    /* initial value=2, tested in 8186 and 8651B */
	LEC_buf_rindex[cch] =0;
#endif
}

void isr_lec_buf_inc_windex( voip_con_t *this )
{
	// once TX ISR 
	const uint32 cch = this ->cch;
	
#ifdef LEC_USE_CIRC_BUF		
	LEC_buf_windex[cch]=(LEC_buf_windex[cch]+1)%FSIZE;
#endif
}

void isr_lec_buf_shift_frame( voip_con_t *this )
{
	// once TX ISR 
	
#ifdef SUPPORT_LEC_G168_ISR	 
#ifndef LEC_USE_CIRC_BUF
	const uint32 cch = this ->cch;
	int fi;	
	/* shift the LEC_RinBuf to one left frame */
#if 1
	for (fi =0; fi < FSIZE-1; fi++)	
    	{
#ifdef USE_MEM64_OP
    		memcpy64s(&LEC_RinBuf[cch][80*PCM_PERIOD*fi * this ->band_factor_var], &LEC_RinBuf[cch][80*PCM_PERIOD*(fi+1) * this ->band_factor_var], 80*PCM_PERIOD * this ->band_factor_var /*2-byte*/);
#else
    		memcpy(&LEC_RinBuf[cch][80*PCM_PERIOD*fi * this ->band_factor_var], &LEC_RinBuf[cch][80*PCM_PERIOD*(fi+1) * this ->band_factor_var], 80*PCM_PERIOD*2 * this ->band_factor_var /*byte*/);
#endif		
    	}
#else
    	memmove(&LEC_RinBuf[cch][0], &LEC_RinBuf[cch][80*PCM_PERIOD*1 * this ->band_factor_var], 80*PCM_PERIOD*2*(FSIZE-1) * this ->band_factor_var /*byte*/);
#endif
#endif // !LEC_USE_CIRC_BUF
#endif // SUPPORT_LEC_G168_ISR
}

void isr_lec_buf_tx_process( voip_con_t *this, uint16 *pcm_tx, uint32 step )
{
	const uint32 cch = this ->cch;
	
	//return ;	// test LEC buf 
	
	#ifndef LEC_G168_ISR_SYNC_P
		// do some DSP 
		
    #else	// LEC_G168_ISR_SYNC_P
    uint32 ii;
    
	if (cnt_time < 200)
		memset(pcm_tx, 0, PCM_PERIOD_10MS_SIZE);
	else if (cnt_time >= 200 && cnt_time < 250)
	{
		 		   	
		for (ii=0; ii < PCM_PERIOD_10MS_SIZE; ii++)
		{
			//*((short*)(dst+(i*PCM_PERIOD_10MS_SIZE)+ii)) = vpat[ii%16];
			memset(pcm_tx, vpat[ii%16], 2);
		}
		sync_start=1; 		
	}
	else
		memset(pcm_tx, 0, PCM_PERIOD_10MS_SIZE);
    #endif	// LEC_G168_ISR_SYNC_P
	
    #ifdef SUPPORT_LEC_G168_ISR
        #ifndef LEC_USE_CIRC_BUF
		/* update the tx pcm data to LEC_RinBuf */
#ifdef USE_MEM64_OP
		memcpy64s(&LEC_RinBuf[cch][80*PCM_PERIOD*(FSIZE-1) * this ->band_factor_var + i*PCM_PERIOD_10MS_SIZE/2], 
			pcm_tx, PCM_PERIOD_10MS_SIZE/2 * this ->band_factor_var);
                        //tx_fifo[chid][tx_fifo_cnt_r[chid]], PCM_PERIOD_10MS_SIZE/2);
#else
		memcpy(&LEC_RinBuf[cch][80*PCM_PERIOD*(FSIZE-1) * this ->band_factor_var + i*PCM_PERIOD_10MS_SIZE/2], 
                        pcm_tx, PCM_PERIOD_10MS_SIZE * this ->band_factor_var);
                        //tx_fifo[chid][tx_fifo_cnt_r[chid]], PCM_PERIOD_10MS_SIZE);
#endif                        
        #else
		memcpy(&LEC_Rin_CircBuf[cch][80*PCM_PERIOD*LEC_buf_windex[cch] * this ->band_factor_var + step*PCM_PERIOD_10MS_SIZE/2 + sync_sample[cch] + CODEC_GROUP_DELAY], 
                       	pcm_tx, PCM_PERIOD_10MS_SIZE * this ->band_factor_var);
                       	//tx_fifo[chid][tx_fifo_cnt_r[chid]], PCM_PERIOD_10MS_SIZE);
		if( (LEC_buf_windex[cch]==(FSIZE-1))&& (step==(PCM_PERIOD-1)) )
			memcpy(&LEC_Rin_CircBuf[cch][0], &LEC_Rin_CircBuf[cch][80*PCM_PERIOD*(FSIZE) * this ->band_factor_var], (sync_sample[cch]+CODEC_GROUP_DELAY)*2);
        #endif
    #endif
}

void isr_lec_buf_tx_process_post( voip_con_t *this )
{
#ifdef LEC_G168_ISR_SYNC_P
	
	cnt_time++;
	
	if (cnt_time%10 == 0)
		printk("%d ",cnt_time);
	
	if (sync_start ==1 && sync ==0)
		sync_frame++;
#endif	

}

void isr_lec_buf_inc_rindex( voip_con_t *this )
{
	// once RX ISR 
	const uint32 cch = this ->cch;
	
#ifdef LEC_USE_CIRC_BUF
	LEC_buf_rindex[ cch ]=(LEC_buf_rindex[ cch ]+1)%FSIZE;
#endif

#ifdef LEC_G168_ISR_SYNC_P 	
 	if (sync_start ==1 && sync ==0)
		sync_frame_R++;   
#endif
}

const uint16 *isr_lec_buf_get_ref_addr( voip_con_t *this, uint32 step )
{
	const uint32 cch = this ->cch;

#ifndef LEC_USE_CIRC_BUF
	return &LEC_RinBuf[cch][80*PCM_PERIOD*(FSIZE-sync_point[cch])* this ->band_factor_var +step*PCM_PERIOD_10MS_SIZE/2-sync_sample[cch]];
#else
	return &LEC_Rin_CircBuf[cch][80*PCM_PERIOD*LEC_buf_rindex[cch] * this ->band_factor_var +step*PCM_PERIOD_10MS_SIZE/2+sync_sample_offset_daa[cch]];
#endif
}

void isr_lec_buf_sync_p( voip_con_t *this, const uint16 *pcm_rx, uint32 step )
{
#ifdef LEC_G168_ISR_SYNC_P
	uint32 ii;
	uint32 stmp;
	const uint32 *rxBuf = ( const uint32 * )pcm_rx;
	short s1, s2;
	
	if (sync == 0)
	{
		for (ii=0; ii< 40; ii++)
		{
			stmp = *(rxBuf+/*(i*PCM_PERIOD_10MS_SIZE>>2)*/+ii);
			
			s1 = (short)((stmp >> 16) & 0x0000ffff);
			s2 = (short)((stmp & 0x0000ffff));
			
			//printk("%d ", s1);
			
			if (cnt_time > 100 && sync != 1)
			{	
				
				if ( s1 > level || s2 >level || s1 < -level || s2 < -level )
				{
					sync =1;
					printk("****** sync! sync_frame=%d, sync_frame_R=%d, step=%d, ii=%d ******\n", sync_frame, sync_frame_R-1, step, ii);
					
				}
			}
			
			if (sync ==1)
			{
				printk("%d\n", s1);
				printk("%d\n", s2);
			}
		}
	}    		
#endif
}


