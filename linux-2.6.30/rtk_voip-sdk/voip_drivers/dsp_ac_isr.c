#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_control.h"

#include "section_def.h"

#include "con_register.h"
#include "dsp_ac_define.h"


void isr_ac_bus_tx_start( voip_dsp_t *this )
{
}

void isr_ac_bus_tx_process_pre( struct voip_dsp_s *this )
{
}

void isr_ac_bus_tx_process( voip_dsp_t *this, uint16 *pcm_tx )
{
	extern void ACMWModemTx(char chid, short *pOutBuff);
	const char chid = this ->dch;
	
	ACMWModemTx(chid, pcm_tx);
}

void isr_ac_bus_tx_process_post( struct voip_dsp_s *this )
{
}

void isr_ac_bus_rx_start( voip_dsp_t *this )
{
}

static int32 PCM_RX(uint32 chid, uint32 *rxBuf, const uint16 *lec_ref);

void isr_ac_bus_rx_process( voip_dsp_t *this, uint16 *pcm_rx, const uint16 *lec_ref )
{
	PCM_RX( this ->dch, ( uint32 * )pcm_rx, lec_ref );
}

// --------------------------------------------------------
// older function   
// --------------------------------------------------------



extern void ACMWModemRx(char chid, short *pInBuff);
#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
extern void ACMWLEC_g168(char chid, short *pRin, short *pSin, short *pEx,int iphone_handfree);	//iphone_handfree: 1-AEC, 0-LEC
#else
extern void ACMWLEC_g168(char chid, short *pRin, short *pSin, short *pEx);
#endif


__pcmIsr02 static int32 PCM_RX(uint32 chid, uint32 *rxBuf, const uint16 *lec_ref)
{
	extern unsigned char support_lec_g168[];
	int /*i,*/ ii, j, SessNum;
	uint32 rx_rp, ssid;
	unsigned int stmp;
	Word16 s1, s2;
	const int i = 0;

#ifdef PCM_PERIOD
	//rx_rp = rxpage[chid] * (40*PCM_PERIOD);	//count page0/page1 offset
	rx_rp = 0;
#else
#ifdef SUPPORT_CUT_DSP_PROCESS
	rx_rp = rxpage[chid] * 40;	//count page0/page1 offset

#else
	rx_rp = rxpage[chid] * (nPcmIntPeriod[chid] << 2);	//count page0/page1 offset
#endif
#endif

#ifdef SUPPORT_PCM_FIFO //==========================================================//

   	//for (i=0; i < PCM_PERIOD; i++)
	{

		/*** Check if Rx FIFO full or not ***/
		//if (((rx_fifo_cnt_w[chid]+1)%PCM_FIFO_SIZE) == rx_fifo_cnt_r[chid]) 
		//{
			// Full
#ifndef AUDIOCODES_VOTING_MECHANISM
		//	printk("RF(%d) ", chid);
#endif
		//	break;
		//}

#ifdef RTK_VOICE_RECORD
		if ((stVoipdataget[chid].write_enable==4)||(stVoipdataget[chid].write_enable==2))
		{
			memcpy(&stVoipdataget[chid].rxbuf[stVoipdataget[chid].rx_writeindex],rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2),160);
			stVoipdataget[chid].rx_writeindex= (stVoipdataget[chid].rx_writeindex+160)%DATAGETBUFSIZE;
		}
#endif

		
#if ACMW_MODEM_RX_BEFORE_LEC
		ACMWModemRx(chid, (Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)));
#endif
		
	/**********************************************
		 *                                    *
		 * 	Line Echo Canceller	      *
		 *                                    *
		 ****************************************************/
		if (support_lec_g168[chid] == 1)
		{
			if( 0 ) {//rx_mute[chid] == 1)	// bus_fifo do it 
#ifdef USE_MEM64_OP
				//memset64s(rx_fifo[chid][rx_fifo_cnt_w[chid]], 0,  PCM_PERIOD_10MS_SIZE/2); 
#else				
				//memset(rx_fifo[chid][rx_fifo_cnt_w[chid]], 0,  PCM_PERIOD_10MS_SIZE); 
#endif				
			}
			else
			{
#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE			
				extern int iphone_handfree;
				extern int pcm_ch_for_DAA[];
				if (((iphone_handfree == 1) && (pcm_ch_for_DAA[chid] != 1)) 
					|| (pcm_ch_for_DAA[chid] == 1))
				{
#endif

#ifndef LEC_USE_CIRC_BUF
				ACMWLEC_g168( chid, (Word16*)&LEC_RinBuf[chid][80*PCM_PERIOD*(FSIZE-sync_point[chid]+1)+i*PCM_PERIOD_10MS_SIZE/2],
				   					(Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)),
#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
									(Word16*)(rx_fifo[chid][rx_fifo_cnt_w[chid]]), iphone_handfree);
#else
									(Word16*)(rx_fifo[chid][rx_fifo_cnt_w[chid]]));
#endif
#else
				ACMWLEC_g168( chid, //(Word16*)&LEC_Rin_CircBuf[chid][80*PCM_PERIOD*LEC_buf_rindex[chid] +i*PCM_PERIOD_10MS_SIZE/2+sync_sample_offset_daa[chid]], 
									lec_ref,
									(Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)), 
#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE			
									//(Word16*)(rx_fifo[chid][rx_fifo_cnt_w[chid]]), iphone_handfree);
									(Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)), iphone_handfree);
#else
									//(Word16*)(rx_fifo[chid][rx_fifo_cnt_w[chid]]));
									(Word16*)(rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2)));
#endif
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE				
				}
				else
				{
#ifdef USE_MEM64_OP
					//memcpy64s(rx_fifo[chid][rx_fifo_cnt_w[chid]], rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2),  PCM_PERIOD_10MS_SIZE/2);
#else
					//memcpy(rx_fifo[chid][rx_fifo_cnt_w[chid]], rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2),  PCM_PERIOD_10MS_SIZE);
#endif
				
				}
#endif


			}
		}
		else
		{
#if 0
			if(rx_mute[chid] == 1)
#ifdef USE_MEM64_OP
				memset64s(rx_fifo[chid][rx_fifo_cnt_w[chid]], 0,  PCM_PERIOD_10MS_SIZE/2); 
#else
				memset(rx_fifo[chid][rx_fifo_cnt_w[chid]], 0,  PCM_PERIOD_10MS_SIZE); 
#endif				
			else
#ifdef USE_MEM64_OP
				memcpy64s(rx_fifo[chid][rx_fifo_cnt_w[chid]], rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2),  PCM_PERIOD_10MS_SIZE/2);
#else
				memcpy(rx_fifo[chid][rx_fifo_cnt_w[chid]], rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2),  PCM_PERIOD_10MS_SIZE);
#endif				
#endif
		}

#ifdef RTK_VOICE_RECORD	/* Thlin: Record for ACMW LEC Sin, Rin, Residual Echo */

		if(stVoipdataget[chid].write_enable==8)
		{
			memcpy(&stVoipdataget[chid].txbuf[stVoipdataget[chid].rx_writeindex],&LEC_Rin_CircBuf[chid][80*PCM_PERIOD*LEC_buf_rindex[chid] +i*PCM_PERIOD_10MS_SIZE/2+sync_sample_offset_daa[chid]],160);	// Rin
			memcpy(&stVoipdataget[chid].rxbuf[stVoipdataget[chid].rx_writeindex],rxBuf+(i*PCM_PERIOD_10MS_SIZE>>2),160);	// Sin
			if(stVoipdataget[chid].write_enable & 16)
				memcpy(&stVoipdataget[chid].rxbuf2[stVoipdataget[chid].rx_writeindex],rx_fifo[chid][rx_fifo_cnt_w[chid]],160);	// Residual Echo
			
			stVoipdataget[chid].rx_writeindex= (stVoipdataget[chid].rx_writeindex+160)%DATAGETBUFSIZE;
			stVoipdataget[chid].tx_writeindex= stVoipdataget[chid].rx_writeindex;
			if(stVoipdataget[chid].write_enable & 16)
				stVoipdataget[chid].rx_writeindex2= stVoipdataget[chid].rx_writeindex;
		}
#endif

#if !ACMW_MODEM_RX_BEFORE_LEC
		ACMWModemRx(chid, rxBuf /*rx_fifo[chid][rx_fifo_cnt_w[chid]]*/);
#endif

	/**** Count the Rx FIFO write index ****/
	        //rx_fifo_cnt_w[chid] = (rx_fifo_cnt_w[chid] +1)% PCM_FIFO_SIZE;


	}//for (i=0; i < PCM_PERIOD; i++)

#else	//=======================================================================//

#ifdef SUPPORT_CUT_DSP_PROCESS

	for (i=0; i<40; i++)
#else
	for (i=0; i<(int)(nPcmIntPeriod[chid] << 2); i++)
#endif
	{
		RxBufTmp[chid][i] = rxBuf[rx_rp++];
	}

#endif	//SUPPORT_PCM_FIFO
	//=======================================================================//

	//printk("(%d, %d) ", rx_fifo_cnt_w[chid], rx_fifo_cnt_r[chid]);




	return SUCCESS;
}

