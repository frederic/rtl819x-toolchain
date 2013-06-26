#include "rtk_voip.h"
#include "voip_types.h"
#include "section_def.h"

#include "con_register.h"


char fsk_cid_enable[MAX_DSP_AC_CH_NUM]={0};	// dch

static char chanAcEnabled[ MAX_DSP_AC_CH_NUM ];

void dsp_ac_chanEnable( uint32 dch, char enable )
{
	chanAcEnabled[ dch ] = enable;
}

static void ACMWPCM_handler(char chanEnabled[]);

__pcmImem void dsp_ac_bus_handler( void )
{
	ACMWPCM_handler(chanAcEnabled);
}

/////////////////////////////////////////////////////////////////////
/* Most of below comes from linux-2.6.30\acmw_lx\pcm865x.c, and do some modification */
#define ACMW_MAX_NUM_CH		MAX_DSP_AC_CH_NUM

#ifdef T38_STAND_ALONE_HANDLER
//#include "../rtk_voip/include/voip_types.h"
#include "../rtk_voip/voip_drivers/t38_handler.h"

uint32* pRxBufTmp; // For reducing memcpy
uint32* pTxBufTmp;
#endif

#define PCM_10MSLEN_INSHORT 80

//int rtkVoipIsEthernetDsp = 0;

typedef volatile struct 
{
	unsigned short *RxBuff;
	unsigned short *TxBuff;
}Tacmw_transiverBuff;

static Tacmw_transiverBuff transiverBuff[ACMW_MAX_NUM_CH];

extern int ACMWPcmProcess(const Tacmw_transiverBuff *transiverBuff, const unsigned int maxCh, const unsigned int size);
extern unsigned long  Ac49xUserDef_DisableInterrupts(void);
extern void Ac49xUserDef_RestoreInterrupts(unsigned long flags);


static __pcmImem void ACMWPCM_handler(char chanEnabled[])
{
	extern voip_dsp_t *get_voip_ac_dsp( uint32 dch );
	
	unsigned int pcmChid;
	unsigned long saved_flags;
	
	while(1)
	{
  		saved_flags = Ac49xUserDef_DisableInterrupts();
		for(pcmChid=0; pcmChid<ACMW_MAX_NUM_CH; pcmChid++)
		{
			voip_dsp_t *p_dsp = get_voip_ac_dsp( pcmChid );
			voip_con_t *p_con = p_dsp ->con_ptr;

			transiverBuff[pcmChid].TxBuff = NULL;
			transiverBuff[pcmChid].RxBuff = NULL;
			
			if( p_dsp ->enabled == 0 )
				continue;

#ifdef T38_STAND_ALONE_HANDLER
			if ((chanEnabled[pcmChid] == 0) || (t38RunningState[pcmChid] == T38_START))
				continue;
#else
			if (chanEnabled[pcmChid] == 0)
				continue;
#endif
			
			//if(pcm_get_read_rx_fifo_addr(pcmChid, (void*)&transiverBuff[pcmChid].RxBuff) )
			if( ( ( transiverBuff[pcmChid].RxBuff = 
						p_con ->con_ops ->dsp_read_bus_rx_get_addr( p_con ) ) 
				== NULL ) ||
				( ( transiverBuff[pcmChid].TxBuff =
						p_con ->con_ops ->dsp_write_bus_tx_get_addr( p_con ) )
				== NULL ) )
			{
				Ac49xUserDef_RestoreInterrupts(saved_flags);
				return 0;
			}
			
			//pcm_get_write_tx_fifo_addr(pcmChid, (void*)&transiverBuff[pcmChid].TxBuff);
		}
		
		Ac49xUserDef_RestoreInterrupts(saved_flags);
				
#ifdef T38_STAND_ALONE_HANDLER
		
		int chid;
		for(chid=0; chid<ACMW_MAX_NUM_CH; chid++)
		{
			voip_dsp_t *p_dsp = get_voip_ac_dsp( chid );
			voip_con_t *p_con = p_dsp ->con_ptr;
			
			if( p_dsp ->enabled == 0 )
				continue;

			if( t38RunningState[chid] == T38_START )
			{
				saved_flags = Ac49xUserDef_DisableInterrupts();
#ifdef CONFIG_RTK_VOIP_DRIVERS_IIS
				//if( pcm_get_read_rx_fifo_addr(chid, (void*)&piis_RxBufTmp))
#else
				//if( pcm_get_read_rx_fifo_addr(chid, (void*)&pRxBufTmp))
#endif
				if( ( ( pRxBufTmp = 
							p_con ->con_ops ->dsp_read_bus_rx_get_addr( p_con ) ) 
					== NULL ) ||
					( ( pTxBufTmp =
							p_con ->con_ops ->dsp_write_bus_tx_get_addr( p_con ) )
					== NULL ) )
				{
					Ac49xUserDef_RestoreInterrupts(saved_flags);
					return 0;
				}

#ifdef CONFIG_RTK_VOIP_DRIVERS_IIS
				//pcm_get_write_tx_fifo_addr(chid, &piis_TxBufTmp);
#else
				//pcm_get_write_tx_fifo_addr(chid, &pTxBufTmp);
#endif

				Ac49xUserDef_RestoreInterrupts(saved_flags);
				
				PCM_handler_T38(chid);
				
				saved_flags = Ac49xUserDef_DisableInterrupts();
				//pcm_read_rx_fifo_done(chid); 
				p_con ->con_ops ->dsp_write_bus_tx_done( p_con );
				p_con ->con_ops ->dsp_read_bus_rx_done( p_con );
				Ac49xUserDef_RestoreInterrupts(saved_flags);
			}
		}

#endif	

		ACMWPcmProcess( &transiverBuff[0], ACMW_MAX_NUM_CH, PCM_10MSLEN_INSHORT );
		
		saved_flags = Ac49xUserDef_DisableInterrupts();
		for(pcmChid=0; pcmChid<ACMW_MAX_NUM_CH; pcmChid++)
		{
			voip_dsp_t *p_dsp = get_voip_ac_dsp( pcmChid );
			voip_con_t *p_con = p_dsp ->con_ptr;
			
			if( p_dsp ->enabled == 0 )
				continue;

			if(transiverBuff[pcmChid].TxBuff != NULL) {
				//pcm_write_tx_fifo_done(pcmChid);
				p_con ->con_ops ->dsp_write_bus_tx_done( p_con );
			}

			if(transiverBuff[pcmChid].RxBuff != NULL) {
				//pcm_read_rx_fifo_done(pcmChid);
				p_con ->con_ops ->dsp_read_bus_rx_done( p_con );
			}

		}
		Ac49xUserDef_RestoreInterrupts(saved_flags);
	}

	return 0;

}

