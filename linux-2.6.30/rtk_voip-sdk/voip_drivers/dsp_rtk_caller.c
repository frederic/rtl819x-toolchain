// caller.c : Defines the entry point for the console application.
//
#include <linux/string.h>
#include <linux/kernel.h>
//#include <asm/param.h>
//#include "si3210init.h"
#include "fsk.h"
//#include "spi.h"
#include "voip_params.h"	// for tone define.
#include "../voip_manager/voip_mgr_events.h"
#include "../voip_dsp/include/fskcid_gen.h"
#include "../include/rtk_voip.h"
#include "../include/voip_types.h"
#include "../include/voip_control.h"
//#include "Legerity88221.h"	// for legerity 88221 caller id
#include "dsp_rtk_caller.h"
#include "dsp_rtk_mux.h"
#include "snd_mux_slic.h"
#include "snd_define.h"
#include "voip_init.h"
#include "voip_proc.h"
#include "voip_timer.h"
// Notice that the 0x24 is the length of the caller ID frame.
// You would need to change this if you wanted to change the length of the
// message
#ifdef CONFIG_RTK_VOIP_LED
#include "./led.h"
#endif
#include "con_mux.h"
#include "con_register.h"

unsigned char init ;// this will indicate if there is a missed interrupt
static timetick_t FirstRingOffTimeOut[DSP_RTK_CH_NUM];
static unsigned char RingGenAfterCid[DSP_RTK_CH_NUM] = {0};

TstDtmfClid dtmf_cid_info[DSP_RTK_CH_NUM];

int fsk_spec_areas[DSP_RTK_CH_NUM] = {[0 ... DSP_RTK_CH_NUM-1] = 0x108}; // Bellcore, Priori 1st Ring, Auto SLIC action
// bit 0-2: FSK Type (dch)
// bit 3: Normal Ring
// bit 4: Fsk Alert Tone
// bit 5: Short Ring
// bit 6: Line Reverse
// bit 7: Date, Time Sync and Name
// bit 8: Auto SLIC Action

char fsk_spec_mode=0;	// 0 : Bellcore
// 1 : ETSI
// 2 : BT
// 3 : NTT

#ifdef FSK_TYPE2_ACK_CHECK
unsigned int fsk_cid_type2_ack[DSP_RTK_CH_NUM] = {0};	// (dch)
#endif

TstVoipFskClid cid_info[DSP_RTK_CH_NUM];

//unsigned char Voip_ringcfg[PCM_CH_NUM];

extern char fsk_cid_state[DSP_RTK_CH_NUM];//={0};		// for FSK CID (dch)

void fsk_cid_onhook_process_only_dsp( unsigned int chid);
void fsk_cid_onhook_process( unsigned int chid);

//-------------Caller ID (FSK) parameter----------//
/* Legerity 88221 slic */
/* FSK for specified country. GR_30 for America, ITU_V23 for Europ */
#define GR_30_FRQC		0x1777
#define GR_30_AMPC		0x27D4
#define GR_30_FRQD		0x0CCD
#define GR_30_AMPD		0x27D4

#define ITU_V23_FRQC		0x1666
#define ITU_V23_AMPC		0x27D4
#define ITU_V23_FRQD		0x0DDE
#define ITU_V23_AMPD		0x27D4

#if 0//def CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221
static void Legerity_FSK_CallerID_coeff(unsigned char slic_id, Le88xxx *data);
static unsigned char Legerity_FSK_CallerID_ready(unsigned char slic_id);
static void Legerity_FSK_CallerID_data(unsigned char slic_id, Le88xxx *data);
static void Legerity_generator_C_D_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re);
#endif

char SiLabsID2[256] = {0};


void dtmf_cid_init(unsigned int chid)
{
	dtmf_cid_info[chid].bAuto_Ring = 1;		// default auto Ring
	dtmf_cid_info[chid].bAuto_SLIC_action = 1;	// default auto SLIC action
	dtmf_cid_info[chid].bBefore1stRing = 1;		// default before 1st Ring
	dtmf_cid_info[chid].bAuto_StartEnd = 0;		// default non-auto
	dtmf_cid_info[chid].start_digit = 'A'-'A';	// default A
	dtmf_cid_info[chid].end_digit = 'C'-'A';	// default C
	dtmf_cid_info[chid].on_duration = 8;		// default 80 msec
	dtmf_cid_info[chid].pause_duration = 8;		// default 80 msec
	dtmf_cid_info[chid].pre_silence_duration = 30;	// default 300 msec
	dtmf_cid_info[chid].end_silence_duration = 30;	// default 300 msec
	memset(dtmf_cid_info[chid].data, 0 , DTMF_CLID_SIZE*sizeof(unsigned char));
}

#if 0	// chmap comment this 
void waitForInterrupt (unsigned int chid)	// hcv, wait proslic OSC1 intr. (by paral port) for reload then clear interrupt
{
	/* Wait for an Interrupt from the ProSLIC => oscillator loaded */
	//if (Interrupt() && init !=0)	// get intr. flag from paral port access, 1 : intr. event happen, 0 : no intr.
	if ( (readDirectReg(chid, 18)&0x01) && init !=0 )
	{
		PRINT_MSG(" %1.1x",init) ;
		init = 2;
	}
	if (init == 0)
		init=1; /* init has 3 states 0 => fsk initialized
						     1 => fsk did first interrupt
						     2 => got premature interrupt
				*/
	//while (!Interrupt()) ;	// get intr. flag from paral port access, 1 : intr. event happen, 0 : no intr.
	while (!(readDirectReg(chid, 18)&0x01)) ;
	writeDirectReg(chid, 18,0x01); /*  Status Register #1  clear interrupt*/
}
#endif

#if 0	// chmap comment this 
void fskByte(unsigned int chid, unsigned char c)	// hcv, send FSK one byte with start & stop bit
{
	unsigned int i;

	writeDirectReg(chid, FSK_DATA,0); // reg52 FSK data register, send Start bit (space)
	waitForInterrupt(chid) ;    	// wait proslic OSC1 intr. (by paral port) for reload then clear interrupt

	for (i=0;i<8;i++)	// send FSK byte from bit0 to bit7
	{
		writeDirectReg(chid, FSK_DATA,c);
		c>>=1;	// next bit
		waitForInterrupt(chid);	// wait proslic OSC1 intr. (by paral port) for reload then clear interrupt
	}
	writeDirectReg(chid, FSK_DATA,1);	// reg52 FSK data register, send Stop bit (mark)
	waitForInterrupt(chid);	// wait proslic OSC1 intr. (by paral port) for reload then clear interrupt
}
#endif

#if 0
void sendProSLICID (unsigned int chid, char mode, unsigned char msg_type, char *str, char *str2, char * cid_name)	// hcv, generate Caller ID (SiLabsID2) and initial FSK reg. then send Caller ID FSK byte
{
	int i/*, j*/;
	unsigned char sum;

	int ch_seizure_cnt=30;	// hc+ 0220
	int mark_sig_cnt=150;	// hc+ 0220

	char nullfskdate[9]={0,0,0,0,0,0,0,0,0};
	extern void fskInitialization (unsigned int chid);
	// Sandro+ 2006/03/13 for NTT
	switch (fsk_spec_areas[chid]&7)	// hc+ 0220
	{
	case FSK_Bellcore:
	case FSK_ETSI:
	case FSK_BT:
		if (msg_type == FSK_MSG_CALLSETUP)
			if (fsk_spec_areas[chid]&0x80)
				CIDMessage1(str, str2, cid_name);
			else
				CIDMessage1(str, nullfskdate, nullfskdate);
		else if (msg_type == FSK_MSG_MWSETUP)
			VMWIMessage(str);
		break;
	case FSK_NTT:
		if (msg_type == FSK_MSG_CALLSETUP)
			if (fsk_spec_areas[chid]&0x80)
				CIDMessage2(str, str2, mode);
			else
				CIDMessage2(str, nullfskdate, mode);
		else if (msg_type == FSK_MSG_MWSETUP)
			VMWIMessage(str); //?
		break;
	}

	PRINT_MSG("\n <hc> SiLabsID2 = ");
	i = 0;
	while (SiLabsID2[i] !=0 || ((SiLabsID2[0] == (char)FSK_MSG_MWSETUP) && (i==4)) )
	{
		PRINT_MSG("%02x ", 0xff & SiLabsID2[i]);
		i++;
	}
	PRINT_MSG("\n <hc> len = %d", SiLabsID2[1]);
	PRINT_MSG("\ncaller id setting = %d\n",fsk_spec_areas[chid]);
	sum= checkSum(SiLabsID2);	// two's complement chksum byte of str SiLabsID2
	/*Step One ~ Two (AN32) */
	fskInitialization(chid);	// set/enable OSC1 active timer, then set FSK Amp/Freq of space/mark, set FSK as Rx path

	/*Step Three - Send the Caller ID Frame (AN32)*/
	// step1. channel seizure 30 continuous bytes of 55h (01010101)
	switch (fsk_spec_areas[chid]&7)	// hc+ 0220
	{
	case FSK_Bellcore: 	// Bellcore
	case FSK_ETSI:		// ETSI
		if (mode)
			ch_seizure_cnt = 0;		// 0 bytes
		else
			ch_seizure_cnt = 30;	// 30 bytes
		break;
	case FSK_BT:  		// BT
		if (mode)
			ch_seizure_cnt = 0;		// 0 bytes
		else
			ch_seizure_cnt = 20;	// 96~315 bits, set middle bytes as 20
		break;
	case FSK_NTT: 		// NTT
		//ch_seizure_cnt = 30;
		ch_seizure_cnt = 0;	// NTT does not have seizure
		break;
	}
	for ( i=0 ; i<ch_seizure_cnt; i++)		// hc$ 0220
		fskByte(chid, 'U');	// send FSK one byte with start & stop bit

	// step2. mark signal consist of a block of 180+-25 or 80+-25 mark bit
	switch (fsk_spec_areas[chid]&7)	// hc+ 0220
	{
	case FSK_Bellcore: 	// Bellcore
		mark_sig_cnt = 150;	// 126~186 bits, set as middle 150 bits
		break;
	case FSK_ETSI:		// ETSI
		mark_sig_cnt = 80;	// 180+/-25 or 80+/-25 bits, set as 80 bits
		break;
	case FSK_BT:  		// BT
		mark_sig_cnt = 70;	// 55~90 bits, set as middle 70 bits
		break;
	case FSK_NTT:  		// NTT
		mark_sig_cnt = 72;
		break;
	}
	for ( i=0 ; i<mark_sig_cnt; i++ )   // hc$ 0220 wait bits worth
	{
		writeDirectReg(chid, FSK_DATA, 1);
		waitForInterrupt(chid);	// wait proslic OSC1 intr. (by paral port) for reload then clear interrupt
	}
	// step3. send SiLabID2 string
	i=0;
	while  ((SiLabsID2[i] != 0) || ((SiLabsID2[0] == (char)FSK_MSG_MWSETUP) && (i==4)))
		fskByte(chid, SiLabsID2[i++]);	// send FSK one byte with start & stop bit

	// step4. send chksum
	fskByte(chid, sum);	// send FSK one byte with start & stop bit
}
#endif


#if 0 // not used
void fsk_gen_msg(uint32 chid, char mode, unsigned char msg_type, char *str, char *str2, char *cid_name)
{
	uint sid;
	int i;
	unsigned long flags;
	ring_struct ringing;

		save_flags(flags);
		cli();

	fsk_spec_mode = fsk_spec_areas[chid];

	// bkup orginal reg
	//bkup_reg();

	// set linefeed
	//writeDirectReg(chid, 64,2);// forward ON-HOOK transmission

	if ((fsk_spec_mode&7) == FSK_NTT)//gavin
	{
		if (mode == 0) // on-hook
		{
			ntt_skip_dc_loop[chid]=1;
			OnHookLineReversal(chid, 1);
			SendNTTCAR(chid);//gavin
		}
	}
	else
	{
		// fsk_spec_areas[]:
		// bit0-2: FSK Type
		// bit 3: Caller ID Prior First Ring
		// bit 4: Dual Tone before Caller ID (Fsk Alert Tone)
		// bit 5: Short Ring before Caller ID
		// bit 6: Reverse Polarity before Caller ID (Line Reverse)
		// bit 7: FSK Date & Time Sync and Display Name

		if (mode == 0) // on-hook
		{
			if (fsk_spec_mode&0x40) // Line Reverse
			{
				OnHookLineReversal(chid, 1);
				mdelay(150);		// 150ms
			}
			else
			{
				OnHookLineReversal(chid, 0);
			}

			if (fsk_spec_mode&0x20) // short ring 250ms.
			{	
				ringing.CH = chid;
				ringing.ring_set = 1;
				FXS_Ring(&ringing); 	// ringing
				mdelay(250); 		// delay for a while
				ringing.ring_set = 0;
				FXS_Ring(&ringing); 	// stop ringing
				
				if (fsk_spec_mode&0x40)	// Line Reverse
				{
					OnHookLineReversal(chid, 1);
				}
				else
				{
					OnHookLineReversal(chid, 0);
				}
			}

			if (fsk_spec_mode&0x10) // Dual Tone
			{
					restore_flags(flags);
				sid = chanInfo_GetTranSessionID(chid);

				hc_SetPlayTone(chid, sid, DSPCODEC_TONE_FSK_ALERT, true, DSPCODEC_TONEDIRECTION_LOCAL);

				if ((fsk_spec_areas[chid]&7) == FSK_Bellcore)
					mdelay(80);
				else
					mdelay(100);

				hc_SetPlayTone(chid, sid, DSPCODEC_TONE_FSK_ALERT, 0, DSPCODEC_TONEDIRECTION_LOCAL);

					save_flags(flags);
					cli();
				}

			if (!(fsk_spec_mode&0x08)) // Caller ID after First Ring
			{
				ringing.CH = chid;
				ringing.ring_set = 1;
				FXS_Ring(&ringing); // ringing
				mdelay(10);

				ringing.CH = chid;
				while (FXS_Check_Ring(chid, &ringing))
					mdelay(10);

				mdelay(250); // delay for a while  250ms + 250ms = 500ms
				if (fsk_spec_mode&0x40) // Line Reverse
				{
					OnHookLineReversal(chid, 1);
				}
				else
				{
					OnHookLineReversal(chid, 0);
				}
			}
		}
	}
	
	if (1)	// soft gen
	{
		restore_flags(flags);
		genSoftFskCID(chid);// hc+ generate Caller ID
	}
	else
	{
		writeDirectReg(chid, INTRPT_MASK3,   0);	// reg 23,0x0 	intr. disable
		writeDirectReg(chid, INTRPT_STATUS3, 0xff);	// reg 20,0xff	intr. clear  pending
		writeDirectReg(chid, INTRPT_MASK2,   0);
		writeDirectReg(chid, INTRPT_STATUS2, 0xff);
		writeDirectReg(chid, INTRPT_MASK1,   0);
		writeDirectReg(chid, INTRPT_STATUS1, 0xff);

		if (((fsk_spec_mode&7) == FSK_NTT)&& (mode == 0))	//gavin
			mdelay(800);		// 0.8sec
		else
			mdelay(250);  		// one second ?, 250 millisecond spacing between the ringing and the caller id

		/*Step One ~ Four(AN32)*/
		sendProSLICID(chid, mode, msg_type, str, str2, cid_name);// hc+ generate Caller ID (SiLabsID2) and initial FSK reg. then send Caller ID FSK byte

		disableOscillators(chid);

		writeDirectReg(chid, INTRPT_MASK3,  0xff);	// reg 23,0x0 	intr. enable
		writeDirectReg(chid, INTRPT_STATUS3,0xff);	// reg 20,0xff	intr. clear pending
		writeDirectReg(chid, INTRPT_MASK2,  0xff);
		writeDirectReg(chid, INTRPT_STATUS2,0xff);
		writeDirectReg(chid, INTRPT_MASK1,  0xff);
		writeDirectReg(chid, INTRPT_STATUS1,0xff);

		if ((((fsk_spec_mode&7) == FSK_NTT)||(fsk_spec_mode&0x40))&& (mode == 0))//gavin
		{
			OnHookLineReversal(chid, 0);
		}

		// restore all reg
		//restore_reg();

		if ((fsk_spec_mode&7) == FSK_NTT)
			mdelay(100);		// 0.1sec

		PRINT_MSG("\nReg108=%x\n", readDirectReg(chid, 108));
		PRINT_MSG("Reg32=%x\n", readDirectReg(chid, 32));
	}

	if ((fsk_spec_mode&7) == FSK_NTT)//gavin
	{
		if (mode == 0) // on-hook
		{
			unsigned char status;

			for (i=0 ; i<70 ; i++  )//delay 7sec for NTT phone OFF-hook -> ON-hook.
			{
				status =1;

					save_flags(flags);cli();

				
#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 )
				status = readDirectReg(chid, 68)&0x01; /* 1:off-hook  0:on-hook */
#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88266)
				Le88xxx data;
				readLegerityReg( chid, 0x4F, &data);

				if (chid == 0)
					status = data.byte1&0x01;
				else if (chid == 1)
					status = data.byte2&0x01;
#endif
					restore_flags(flags);

				if (!status)
				{
					break;
				}
				mdelay(100);		// 0.1sec
			}


			ntt_skip_dc_loop[chid]=0;

		}
	}


}
#endif


void fsk_gen_init(uint32 chid)
{
	memset(&cid_info[chid], 0, sizeof(TstVoipFskClid));
	init_softfskcidGen(chid);
	fsk_cid_state[chid] = 0;
	//printk("==>fsk_gen_init\n");
}

unsigned int gtype2_ack_waiting_time[DSP_RTK_CH_NUM];	/* delay for waiting ack unit: ms */
unsigned int gtype2_delay_after_ack[DSP_RTK_CH_NUM];	/* delay after ack and before type 2 fsk unit: ms */
unsigned int gtype1_cas_time[DSP_RTK_CH_NUM];		/* */
unsigned int gtype1_delay_after_cas[DSP_RTK_CH_NUM];
unsigned int gtype2_waiting_for_ack_time[DSP_RTK_CH_NUM];
void fsk_cid_process( void )
{
	unsigned int chid;
	unsigned int sid;

	//extern void pcm_disableChan(unsigned int chid);

#ifdef CONFIG_RTK_VOIP_LED
	extern void led_state_watcher(unsigned int chid);
#endif
	extern uint32 chanInfo_GetTranSessionID(uint32 chid);
	extern void hc_SetPlayTone(uint32 chid, uint32 sid, uint32 nTone, uint bFlag, uint path);
#ifdef FSK_TYPE2_ACK_CHECK
	#define ACK_WAITING_TIME	20	//unit: 10ms
	#define FSK2_SEND_DELAY		80	//unit: ms

	extern volatile char fsk_alert_done[];
	static timetick_t check_cnt[DSP_RTK_CH_NUM]={0};
	static int check_type2_ack[DSP_RTK_CH_NUM]={0};
	static int wait_ack[DSP_RTK_CH_NUM]={0};
	static int fsk_cid_process_get_type2_ack[DSP_RTK_CH_NUM]={0};
	static timetick_t fsk2_send_dealy[DSP_RTK_CH_NUM] = {0};
#endif

	for(chid =0 ; chid < dsp_rtk_ch_num ; chid ++)
	{
		extern TstVoipFskClid cid_info[];
		
		if ( (fsk_cid_state[chid] == 1) && (cid_info[chid].cid_setup == 0))
		{
			fsk_gen(chid);
		}
			
		if (  (cid_info[chid].cid_setup ==1) )
		{

#ifdef FSK_TYPE2_ACK_CHECK
			if ((fsk_spec_areas[chid]&7) != FSK_NTT)
			{
				if ((cid_info[chid].cid_mode == 1) /*type2*/ && (fsk_cid_process_get_type2_ack[chid] == 0) )
				{
					/*** th: wait 100ms for check type2 ack ***/
					if ((check_type2_ack[chid] == 0) && (wait_ack[chid] == 0))
					{
						check_cnt[chid] = timetick;
						wait_ack[chid] = 1;
						//printk("%d ", check_cnt[chid]);
					}
		
					//if ((jiffies - check_cnt[chid]) > ACK_WAITING_TIME)
					//if ((jiffies - check_cnt[chid]) > (gtype2_ack_waiting_time[chid]/10))
					if ((timetick - check_cnt[chid]) > (gtype2_ack_waiting_time[chid]))
					{
						check_type2_ack[chid] = 1;
						check_cnt[chid] = 0;
						wait_ack[chid] = 0;
						//printk("check_type2_ack[%d] = %d\n", chid, check_type2_ack[chid]);
					}
					/*******************************************/
	
					if ( check_type2_ack[chid] == 1)
					{
						if (fsk_cid_type2_ack[chid] == 0)
						{
							/** Time out, don't get ACK from phone **/
							//PRINT_MSG("No Ack, type2 cid process terminated!\n");
							PRINT_Y("No Ack, type2 cid process terminated!\n");
							cid_info[chid].cid_setup = 0;
							check_type2_ack[chid] = 0;
							fsk_alert_done[chid] = 0;
							fsk_cid_state[chid]=0;  // clear it
							//printk("z");
							sid = chanInfo_GetTranSessionID(chid);
							hc_SetPlayTone(chid, sid, DSPCODEC_TONE_FSK_MUTE, 0, DSPCODEC_TONEDIRECTION_LOCAL);
							if (fsk_cid_state[chid] == 0) {
								voip_event_dsp_in( chid, chid, VEID_DSP_FSK_CLID_TYPE2_NO_ACK );
							}
							continue;
						}
						else
						{
							/**  Get ACK from phone,   **/
							//fsk_cid_type2_ack[chid] = 0; //move to the bottom of fsk_cid_process
							check_type2_ack[chid] = 0;
							fsk_cid_process_get_type2_ack[chid] = 1;
							//fsk2_send_dealy[chid] = jiffies + (HZ*FSK2_SEND_DELAY/1000);
							//fsk2_send_dealy[chid] = jiffies + (HZ*gtype2_delay_after_ack[chid]/1000)-7;
							//fsk2_send_dealy[chid] = timetick + gtype2_delay_after_ack[chid]-70;

							// The time between TE-ACK recognition and the start of FSK modulation transmission							
							fsk2_send_dealy[chid] = timetick + gtype2_delay_after_ack[chid] - 7;	// unit: ms							
						}
					}
					else
					{
						/** waiting for ACK from phone **/
						//printk("->b\n");
						continue;
					}
				
				}
			}

			if (fsk_cid_process_get_type2_ack[chid] == 1)
			{
				if (!timetick_after(timetick, fsk2_send_dealy[chid]))
				{
					/** Delay after ACK not timeout **/
					continue;
				}
			}
#endif
			if (cid_info[chid].cid_complete == 0)
			{
				if ( cid_info[chid].cid_mode == 0)/* on-hook*/
				{
					if (fsk_spec_areas[chid]&0x100) // auto SLIC action
						fsk_cid_onhook_process( chid);
					else
						fsk_cid_onhook_process_only_dsp( chid );
				}
				else
				{
					sid = chanInfo_GetTranSessionID(chid);
					hc_SetPlayTone(chid, sid, DSPCODEC_TONE_FSK_MUTE, 0, DSPCODEC_TONEDIRECTION_LOCAL);
					genSoftFskCID(chid);// generate Caller ID
					cid_info[chid].cid_complete = 1;					
					//printk("y");
				}
			}
			else /* cid_info[chid].cid_complete */
			{
				if (fsk_cid_state[chid]==0)
				{
					unsigned long flags;
					
					save_flags(flags); cli();

					if ( cid_info[chid].cid_mode == 0)/* on-hook*/
					{
						// TH: Check if off-hook, if ture, not disable PCM.
#if 1
						if (fsk_spec_areas[chid]&0x100) // auto SLIC action
						{
							const uint32 cch = chid;
							if( !SLIC_Get_Hook_Status( cch, 0 ) ) {
								con_enable_cch( cch, 0 );
							}
						}
#else
						if (fsk_spec_areas[chid]&0x100) // auto SLIC action
						{
							if (!SLIC_Get_Hook_Status(cch, 0))
							{
								pcm_disableChan(chid);
								pcm_flag[chid] = 0;
								SLIC_Set_PCM_state(cch, SLIC_PCM_OFF);
							}
						}
						//else
							//PRINT_R("off-hook, not disable pcm\n");
#endif
					}
					ntt_skip_dc_loop[chid]=0;
					cid_info[chid].cid_setup = 0;

#ifdef FSK_TYPE2_ACK_CHECK
					fsk_alert_done[chid] = 0;
					fsk_cid_type2_ack[chid] = 0;
					fsk_cid_process_get_type2_ack[chid] = 0;
#endif
					restore_flags(flags);
				}
			}/* if (cid_info[chid].cid_complete == 0) */
		}/* if (cid_info[chid].cid_setup ==1) */

		if ((fsk_spec_areas[chid]&0x100) && (RingGenAfterCid[chid] == 1))
		{
			if (timetick_after(timetick, FirstRingOffTimeOut[chid])) // after 1st Ring Off cadence time out, gen 2nd Ring.
			{
				const uint32 cch = chid;
				
				extern unsigned char ioctrl_ring_set[];
				if ((ioctrl_ring_set[cch]&0x2) && (!(ioctrl_ring_set[cch]&0x1)) && (!(fsk_spec_areas[chid]&0x08)))
				{
					/* App set Ring disable, so DSP no need to Ring atuomatically. */
					//printk("1:==>%d\n", ioctrl_ring_set[chid]);
				}
				else
				{
					if ((ioctrl_ring_set[cch]&0x2) && (!(ioctrl_ring_set[cch]&0x1))) // CID prior 1st Ring
					{
						/* App set Ring disable, so DSP no need to Ring atuomatically. */
						//printk("2:==>%d\n", ioctrl_ring_set[chid]);
					}
					else
					{
#if 0
						TstVoipSlicRing stVoipSlicRing;
						stVoipSlicRing.ch_id = chid;
						stVoipSlicRing.ring_set = 1;
						FXS_Ring(&stVoipSlicRing);
#else
						const uint32 cch = chid;
						FXS_Ring( cch, 1 );
#endif
						PRINT_MSG("DSP Ring, ch=%d\n", chid);
					}
				}
				RingGenAfterCid[chid] = 0;
				ioctrl_ring_set[cch]&=(~0x2);
				//printk("c:ioctrl_ring_set[%d]=%d\n", chid, ioctrl_ring_set[chid]);
			}
		}

	}/* chid for loop */

}

void fsk_cid_onhook_process_only_dsp( unsigned int chid)
{
	// fsk_spec_areas[]:
	// bit0-2: FSK Type
	// bit 3: Caller ID Prior First Ring	NOT suppot in fsk_cid_onhook_process_only_dsp
	// bit 4: Dual Tone before Caller ID (Fsk Alert Tone)
	// bit 5: Short Ring before Caller ID	NOT suppot in fsk_cid_onhook_process_only_dsp
	// bit 6: Reverse Polarity before Caller ID (Line Reverse)	NOT suppot in fsk_cid_onhook_process_only_dsp
	// bit 7: FSK Date & Time Sync and Display Name
	const uint32 cch = chid;
	uint sid;
	extern uint32 chanInfo_GetTranSessionID(uint32 chid);
	extern void hc_SetPlayTone(uint32 chid, uint32 sid, uint32 nTone, uint bFlag, uint path);


	if ( (fsk_spec_areas[chid]&7) == FSK_NTT )
	{
		switch (cid_info[chid].cid_states)
		{
			case 0 :
				genSoftFskCID(chid);// generate Caller ID
				PRINT_MSG("RR2,%lx]", timetick);
				cid_info[chid].cid_complete = 1;
				break;
			
		}
	}
	else
	{
		switch (cid_info[chid].cid_states)
		{
			case 0 :	/* init state */
				if (fsk_spec_areas[chid] &0x10) // Dual tone
				{
					sid = chanInfo_GetTranSessionID(chid);
					hc_SetPlayTone(chid, sid, DSPCODEC_TONE_FSK_ALERT, true, DSPCODEC_TONEDIRECTION_LOCAL);
					
					//if ((fsk_spec_areas[chid]&7) == FSK_Bellcore)
					//	cid_info[chid].time_out = jiffies + (HZ*80/1000) ;/*time out value = 250msec, */
					//else
					//	cid_info[chid].time_out = jiffies + (HZ*100/1000) ;/*time out value = 250msec, */
					//cid_info[chid].time_out = jiffies + (HZ*gtype1_cas_time[chid]/1000) -1;
					cid_info[chid].time_out = timetick + gtype1_cas_time[chid] -10;
					cid_info[chid].cid_states ++;
				}
				else
					cid_info[chid].cid_states += 4; /* skip play tone */
				break;		
			case 1 :
				if (timetick_after(timetick,cid_info[chid].time_out) ) //time_after(a,b): returns true if the time a is after time b.
				{
					sid = chanInfo_GetTranSessionID(chid);
					hc_SetPlayTone(chid, sid, DSPCODEC_TONE_FSK_ALERT, 0, DSPCODEC_TONEDIRECTION_LOCAL);
					cid_info[chid].cid_states ++;
				}
				break;
			case 2 :
				//cid_info[chid].time_out = jiffies + (HZ*gtype1_delay_after_cas[chid]/1000) -3;//5;
				cid_info[chid].time_out = timetick + gtype1_delay_after_cas[chid] -30;//50;
				cid_info[chid].cid_states ++;
				break;
			case 3 :
				//silence after cas tone
				if (timetick_after(timetick,cid_info[chid].time_out) ) //time_after(a,b): returns true if the time a is after time b.
				{
					cid_info[chid].cid_states ++;
				}
				break;
			case 4 :
				genSoftFskCID(chid);// generate Caller ID
				cid_info[chid].cid_complete = 1;
				break;
		}
	}
	
}


void fsk_cid_onhook_process( unsigned int chid)
{
	// fsk_spec_areas[]:
	// bit0-2: FSK Type
	// bit 3: Caller ID Prior First Ring
	// bit 4: Dual Tone before Caller ID (Fsk Alert Tone)
	// bit 5: Short Ring before Caller ID
	// bit 6: Reverse Polarity before Caller ID (Line Reverse)
	// bit 7: FSK Date & Time Sync and Display Name
	// bit 8: Auto SLIC Action
	const uint32 cch = chid;

	extern unsigned int SendNTTCAR_check(unsigned int chid, unsigned long time_out);
	extern uint32 chanInfo_GetTranSessionID(uint32 chid);
	extern void hc_SetPlayTone(uint32 chid, uint32 sid, uint32 nTone, uint bFlag, uint path);
	if ( (fsk_spec_areas[chid]&7) == FSK_NTT )
	{
		switch (cid_info[chid].cid_states)
		{
			case 0 :	/* init state */
				ntt_skip_dc_loop[chid]=1;
				OnHookLineReversal(cch, 1);
				SendNTTCAR(cch);
				//cid_info[chid].time_out = jiffies + HZ*6 ;/*time out value = 6sec, */
				cid_info[chid].time_out = timetick + 6000 ;/*time out value = 6sec (6000ms), */
				cid_info[chid].cid_states ++;
				PRINT_MSG("RR0,%lx]", timetick);
				break;
			case 1 :	/* wait NTT CAR , phone off-hook, or time out */
				if(SendNTTCAR_check(cch, cid_info[chid].time_out))
					cid_info[chid].cid_states ++;
				break;
			case 2 :
				genSoftFskCID(chid);// generate Caller ID
				PRINT_MSG("RR2,%lx]", timetick);
				cid_info[chid].cid_states ++;
				//cid_info[chid].time_out = jiffies + HZ*7 ;/*time out value = 7sec, */
				cid_info[chid].time_out = timetick + 7000 ;/*time out value = 7sec (7000ms), */
				break;
			case 3 :
				if( SLIC_Get_Hook_Status(cch, 0) ) /* wait 7 sec to on-hook */ /* 1:off-hook  0:on-hook */
				{
					if (timetick_after(timetick,cid_info[chid].time_out) ) //time_after(a,b): returns true if the time a is after time b.
					{
						extern unsigned char ioctrl_ring_set[];

						cid_info[chid].cid_complete = 1;
						if (cid_info[chid].cid_msg_type == FSK_MSG_CALLSETUP)
						{
							ioctrl_ring_set[cch] = 1 + (0x1<<1); //enable ring
							//FirstRingOffTimeOut[chid] = jiffies + (HZ*1500/1000);   // add delay between CID and 1st Ring
							FirstRingOffTimeOut[chid] = timetick + 1500;   // add delay between CID and 1st Ring
							RingGenAfterCid[chid] = 1;
						}
					}
				}
				else
				{
					extern unsigned char ioctrl_ring_set[];

					cid_info[chid].cid_complete = 1;
					if (cid_info[chid].cid_msg_type == FSK_MSG_CALLSETUP)
					{
						ioctrl_ring_set[cch] = 1 + (0x1<<1); //enable ring
						//FirstRingOffTimeOut[chid] = jiffies + (HZ*1500/1000);   // add delay between CID and 1st Ring
						FirstRingOffTimeOut[chid] = timetick + 1500;   // add delay between CID and 1st Ring
						RingGenAfterCid[chid] = 1;
					}
				}

				break;				
		}
	}
	else
	{
		ring_struct ringing;
		uint sid;

		//PRINT_Y("st=%d\n", cid_info[chid].cid_states);

		switch (cid_info[chid].cid_states)
		{
			case 0 :	/* init state */
				if (fsk_spec_areas[chid] &0x40)//Line Reverse
				{
					OnHookLineReversal(cch, 1);
					//cid_info[chid].time_out = jiffies + (HZ*150/1000) ;/*time out value = 150msec, */
					cid_info[chid].time_out = timetick + 150 ;/*time out value = 150msec, */
					cid_info[chid].cid_states++;
				}
				else
				{
					OnHookLineReversal(cch, 0);
					//cid_info[chid].time_out = jiffies + (HZ*150/1000) ;/*time out value = 150msec, */
					cid_info[chid].time_out = timetick + 150 ;/*time out value = 150msec, */
					cid_info[chid].cid_states += 2; /* skip waiting 150ms */
				}
				break;
			case 1 :
				if (timetick_after(timetick,cid_info[chid].time_out) ) //time_after(a,b): returns true if the time a is after time b.
				{
					cid_info[chid].cid_states++;
				}
				break;
			case 2 :
				if (fsk_spec_areas[chid] &0x20)	// short ring 250ms.
				{
#if 0
					ringing.CH = chid;
					ringing.ring_set = 1;
					FXS_Ring((ring_struct *)&ringing); // ringing
#else
					FXS_Ring( cch, 1 );
#endif
					//cid_info[chid].time_out = jiffies + (HZ*250/1000) ;/*time out value = 250msec, */
					cid_info[chid].time_out = timetick + 250 ;/*time out value = 250msec, */
					cid_info[chid].cid_states++;
				}
				else
				{
					cid_info[chid].cid_states += 3; /* skip waiting 150ms */
				}
				break;
			case 3 :
				if (timetick_after(timetick,cid_info[chid].time_out) ) //time_after(a,b): returns true if the time a is after time b.
				{
#if 0
					ringing.CH = chid;
					ringing.ring_set = 0;
					FXS_Ring((ring_struct *)&ringing);
#else
					FXS_Ring( cch, 0 );
#endif

					//cid_info[chid].time_out = jiffies + (HZ*650/1000) ;/*time out value = 650msec, */
					cid_info[chid].time_out = timetick + 650 ;/*time out value = 650msec, */
					cid_info[chid].cid_states++;
				}
				break;
			case 4 : //TH: need delay between short ring off and Line-OHT to avoid noise occasionally
				if (timetick_after(timetick,cid_info[chid].time_out) ) 
				{
					if (fsk_spec_areas[chid]&0x40)
						OnHookLineReversal(cch, 1);
					else
						OnHookLineReversal(cch, 0);
					cid_info[chid].cid_states++;
				}
				break;
			case 5 :
				if (fsk_spec_areas[chid] &0x10) // Dual tone
				{
					sid = chanInfo_GetTranSessionID(chid);
					hc_SetPlayTone(chid, sid, DSPCODEC_TONE_FSK_ALERT, true, DSPCODEC_TONEDIRECTION_LOCAL);
					
					//if ((fsk_spec_areas[chid]&7) == FSK_Bellcore)
					//	cid_info[chid].time_out = jiffies + (HZ*80/1000) ;/*time out value = 250msec, */
					//else
					//	cid_info[chid].time_out = jiffies + (HZ*100/1000) ;/*time out value = 250msec, */

					//cid_info[chid].time_out = jiffies + (HZ*gtype1_cas_time[chid]/1000) -1;
					cid_info[chid].time_out = timetick + gtype1_cas_time[chid] -10;
					cid_info[chid].cid_states ++;
				}
				else
					cid_info[chid].cid_states += 4; /* skip play tone */

				break;
					
			case 6 :
				if (timetick_after(timetick,cid_info[chid].time_out) ) //time_after(a,b): returns true if the time a is after time b.
				{
					sid = chanInfo_GetTranSessionID(chid);
					hc_SetPlayTone(chid, sid, DSPCODEC_TONE_FSK_ALERT, 0, DSPCODEC_TONEDIRECTION_LOCAL);
					cid_info[chid].cid_states ++;
				}
				break;
			case 7 :
				//cid_info[chid].time_out = jiffies + (HZ*gtype1_delay_after_cas[chid]/1000) -5;
				cid_info[chid].time_out = timetick + gtype1_delay_after_cas[chid] -50;
				cid_info[chid].cid_states ++;
				break;
			case 8 :
				//silence after cas tone
				if (timetick_after(timetick,cid_info[chid].time_out) ) //time_after(a,b): returns true if the time a is after time b.
				{
					cid_info[chid].cid_states ++;
				}
				break;
			case 9 :
				if ( ! (fsk_spec_areas[chid] & 0x08) ) //Caller ID after First Ring
				{
					cid_info[chid].cid_states ++;
					// Must add delay to check Ring status correctly 
					//cid_info[chid].time_out = jiffies + (HZ*200/1000);
					cid_info[chid].time_out = timetick + 200;	// 200ms 

					if (cid_info[chid].cid_msg_type == FSK_MSG_CALLSETUP)
					{
						extern unsigned char ioctrl_ring_set[];
#if 0
						ringing.CH = chid;
						ringing.ring_set = 1;
						FXS_Ring((ring_struct *)&ringing); // ringing
#else
						ringing.ring_set = 1;
						FXS_Ring( cch, 1 );
#endif

						ioctrl_ring_set[cch] = ringing.ring_set + (0x1<<1);
						//printk("s6:ioctrl_ring_set[%d]=%d\n", chid, ioctrl_ring_set[chid]);
						PRINT_MSG("1st Ring\n");
					}
#ifdef CONFIG_RTK_VOIP_LED
					extern volatile unsigned int daa_ringing;
					extern volatile unsigned int fxs_ringing[];
					if (daa_ringing == 0)
						fxs_ringing[chid] = 1;
					led_state_watcher(chid);
#endif

				}
				else
				{
					//Caller ID priori First Ring
					extern unsigned char ioctrl_ring_set[];
					ringing.ring_set = 1;
					cid_info[chid].cid_states += 2;
					ioctrl_ring_set[cch] = ringing.ring_set + (0x1<<1);//Must to keep DSP auto-Ring normal.
					//cid_info[chid].time_out = jiffies;// must, because case 7 check time out
					cid_info[chid].time_out = timetick;// must, because case 7 check time out
				}
				break;
			case 10:
				ringing.CH = chid;

				if (timetick_after(timetick,cid_info[chid].time_out) )
				{
					if (! FXS_Check_Ring(cch))	// 1st Ring stop
					{
						// Add delay between 1st Ring off and CLID gen start
						//cid_info[chid].time_out = jiffies + (HZ*gDelayAfert1stRing[chid]/1000) ;
						cid_info[chid].time_out = timetick + gDelayAfert1stRing[chid] ;
						cid_info[chid].cid_states ++;
					}
				}
				break;
			case 11 :
				if (timetick_after(timetick,cid_info[chid].time_out) )
				{
						if (SLIC_Get_Hook_Status(cch, 0) == 0)	// On-hook
						{
							cid_info[chid].cid_states ++;
							
							if (fsk_spec_areas[chid]&0x40)
								OnHookLineReversal(cch, 1);
							else
								OnHookLineReversal(cch, 0);
						}
						else
						{
							fsk_gen_init(chid);
							//PRINT_Y("init_softfskcidGen(%d)\n", chid);
							PRINT_MSG("Stop fsk gen due to off-hook(%d)\n", chid);
							return;
						}
				}
				break;
			case 12 :
				if ( ! (fsk_spec_areas[chid] & 0x08) ) //Caller ID after First Ring
				{
					// wait for delay between 1st Ring and CLID time out
					if (0 == timetick_after(timetick,cid_info[chid].time_out))
						break;
				}
				
				genSoftFskCID(chid);// generate Caller ID
				
				cid_info[chid].cid_complete = 1;
				
				if (fsk_spec_areas[chid] & 0x08) //Caller ID Prior First Ring
				{
					unsigned int cid_process_delay;
					cid_process_delay = gChannelSeizureDuration[chid] + gMarkDuration[chid] + gMsgDuration[chid];
					if (cid_info[chid].cid_msg_type == FSK_MSG_CALLSETUP)
					{
						//FirstRingOffTimeOut[chid] = jiffies + (HZ*(350+cid_process_delay)/1000);
						FirstRingOffTimeOut[chid] = timetick + 350+cid_process_delay;
						RingGenAfterCid[chid] = 1;
					}
				}
				else //Caller ID After First Ring
				{
					extern int gRingCadOff[];
					unsigned int cid_process_delay;
					cid_process_delay = gChannelSeizureDuration[chid] + gMarkDuration[chid] + gMsgDuration[chid];
					//PRINT_R("%d, %d, %d\n", gChannelSeizureDuration[chid], gMarkDuration[chid], gMsgDuration[chid]);
					
					if (cid_info[chid].cid_msg_type == FSK_MSG_CALLSETUP)
					{
						
						if (gRingCadOff[cch] > (gDelayAfert1stRing[chid] + gDelayBefore2ndRing[chid] + cid_process_delay /*clid delay*/)) {
							//FirstRingOffTimeOut[chid] = jiffies + (HZ*(gRingCadOff[cch]-gDelayAfert1stRing[chid])/1000) ;
							FirstRingOffTimeOut[chid] = timetick + gRingCadOff[cch]-gDelayAfert1stRing[chid] ;
						} else {
							//FirstRingOffTimeOut[chid] = jiffies + (HZ*(gDelayBefore2ndRing[chid] + cid_process_delay /*clid delay*/)/1000) ;
							FirstRingOffTimeOut[chid] = timetick + gDelayBefore2ndRing[chid] + cid_process_delay /*clid delay*/ ;
						}
						//PRINT_R("%d, %d, %d\n", gRingCadOff[chid], gDelayAfert1stRing[chid], gDelayBefore2ndRing[chid]);
						RingGenAfterCid[chid] = 1;
					}
				}
				break;
		}
	}
}


// --------------------------------------------------------
// FSK redirect function 
// --------------------------------------------------------

//extern TstVoipFskClid cid_info[VOIP_CH_NUM];

static void CID_for_FSK_SW(unsigned int chid, char service_type, unsigned char msg_type, TstFskClidData *clid_data)
{
	extern char fsk_cid_state[];
	const uint32 cch = chid;

#if ! defined (AUDIOCODES_VOIP)

	// if (fxs off-hook && type-1 fsk)
	if (fsk_spec_areas[chid]&0x100) // auto SLIC action
	{
		if ((SLIC_Get_Hook_Status(cch, 1) == 1) && (service_type == 0))
		{
			//PRINT_Y("cid send abort\n");
			return;	// don't send cid when phone off-hook
		}
	}
									
	if ( fsk_cid_state[chid] == 0)
	{
		cid_info[chid].cid_mode = service_type;	// Service Type 
		cid_info[chid].cid_msg_type = msg_type;		// Message Type
		memcpy(cid_info[chid].cid_data, clid_data, FSK_MDMF_SIZE*sizeof(TstFskClidData));
#if 0
		int i;
		for (i=0; i<FSK_MDMF_SIZE; i++)
		{
			printk("cid_info[%d].cid_data[%d].type = %d\n", chid, i, cid_info[chid].cid_data[i].type);
			printk("cid_info[%d].cid_data[%d].data = %s\n", chid, i, cid_info[chid].cid_data[i].data);
		}
			
#endif
		fsk_cid_state[chid] = 1;
	}
	else
	{
		// If fsk CID is during transmission, ignore new one.
	}
	
#endif
}


void SLIC_gen_FSK_CID(unsigned int chid, char mode, unsigned char msg_type, TstFskClidData *clid_data)
{
#ifndef AUDIOCODES_VOIP
	/* HW FSK CID gen is not supported, so it always use SW FSK CID gen. */
	//CID_for_FSK_SW(chid, mode, msg_type , str, str2, cid_name);
	CID_for_FSK_SW(chid, mode, msg_type, clid_data);
#endif

}

int SLIC_gen_VMWI(unsigned int chid, TstFskClidData *clid_data)
{
#ifndef AUDIOCODES_VOIP

	init_softfskcidGen(chid);
	CID_for_FSK_SW(chid, 0 /*type 1*/, FSK_MSG_MWSETUP, clid_data);

	return 1;
#endif
}

/* legerity88221 fsk caller id gen. */
#if 0//def CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221
//Caller ID main function.Befor calling this function ,remeber to set generator C and D.
void Legerity_FSK_CallerID_main(unsigned char slic_id,char *data, char *data2,char *cid_name, char mode, unsigned char msg_type)
{
	Le88xxx data_temp;
	int i,mark_signal;

	char nullfskdate[9]={0,0,0,0,0,0,0,0,0};

	fsk_spec_mode = fsk_spec_areas[dch];

	/*Initial the frequency and amplitude of generator C and D.For caller ID (FSK)*/
	/*frqC: 2200Hz, ampC: -7dBm0. frqD: 1200Hz, ampD: -7dBm0.*/

	if ((fsk_spec_areas[slic_id]&7) == FSK_Bellcore)
	{
		data_temp.byte1 = GR_30_FRQC>>8;
		data_temp.byte2 = GR_30_FRQC;
		data_temp.byte3 = GR_30_AMPC>>8;
		data_temp.byte4 = GR_30_AMPC;
		data_temp.byte5 = GR_30_FRQD>>8;
		data_temp.byte6 = GR_30_FRQD;
		data_temp.byte7 = GR_30_AMPD>>8;
		data_temp.byte8 = GR_30_AMPD;
		mark_signal = 15;
	}
	else
	{
		data_temp.byte1 = ITU_V23_FRQC>>8;
		data_temp.byte2 = ITU_V23_FRQC;
		data_temp.byte3 = ITU_V23_AMPC>>8;
		data_temp.byte4 = ITU_V23_AMPC;
		data_temp.byte5 = ITU_V23_FRQD>>8;
		data_temp.byte6 = ITU_V23_FRQD;
		data_temp.byte7 = ITU_V23_AMPD>>8;
		data_temp.byte8 = ITU_V23_AMPD;
		mark_signal = 8;
	}


	Legerity_generator_C_D_coeff(slic_id, &data_temp, 0);


	switch (fsk_spec_mode&7)	// hc+ 0220
	{
	case FSK_Bellcore:
	case FSK_ETSI:
	case FSK_BT:
		if (msg_type == FSK_MSG_CALLSETUP)
			if (fsk_spec_mode&0x80)
				CIDMessage1(data, data2, cid_name);
			else
				CIDMessage1(data, nullfskdate, nullfskdate);
		else if (msg_type == FSK_MSG_MWSETUP)
			VMWIMessage(data);
		break;
	case FSK_NTT:
		if (msg_type == FSK_MSG_CALLSETUP)
			if (fsk_spec_mode&0x80)
				CIDMessage2(data, data2, mode);
			else
				CIDMessage2(data, nullfskdate, mode);
		else if (msg_type == FSK_MSG_MWSETUP)
			VMWIMessage(data); //?
		break;
	}


	//force slic into ACTIVE_MID_BATTERY state(on hook transmission)
	Legerity_system_state( slic_id, ACTIVE_MID_BATTERY, 0);

	for (i=0;i<65000000;i++);//65000000~70000000

	//channel seizure signal
	data_temp.byte1 = 0x02;
	Legerity_FSK_CallerID_coeff(slic_id, &data_temp);
	data_temp.byte1 = 'U';
	for (i=0;i<30;i++)
	{
		Legerity_FSK_CallerID_data(slic_id, &data_temp);
		Legerity_FSK_CallerID_ready(slic_id);
	}


	//mark signal
	/*
	*#ifdef GR_30
	*	mark_signal = 15;
	*#endif
	*#ifdef ITU_V23
	*	mark_signal = 8;
	*#endif
	*/




	data_temp.byte1 = 0x06;
	Legerity_FSK_CallerID_coeff(slic_id, &data_temp);
	data_temp.byte1 = 0xFF;
	for (i=0;i<mark_signal;i++)
	{
		Legerity_FSK_CallerID_data(slic_id, &data_temp);
		Legerity_FSK_CallerID_ready(slic_id);
	}

	//message
	data_temp.byte1 = 0x02;
	Legerity_FSK_CallerID_coeff(slic_id, &data_temp);
	i = 0;
	while (SiLabsID2[i] != 0)
	{
		data_temp.byte1 = SiLabsID2[i++];
		Legerity_FSK_CallerID_data(slic_id, &data_temp);
		Legerity_FSK_CallerID_ready(slic_id);
	}

	//checksum
	data_temp.byte1 = checkSum(SiLabsID2);
	Legerity_FSK_CallerID_data(slic_id, &data_temp);
	Legerity_FSK_CallerID_ready(slic_id);

	//disable Caller ID
	data_temp.byte1 = 0x01;
	Legerity_FSK_CallerID_coeff(slic_id, &data_temp);

	for (i=0;i<25000000;i++);//25000000

	return;
}


//Caller ID parameter setting.
//Mark signal data.byte1=0x06, others data.byte1=0x02.
static void Legerity_FSK_CallerID_coeff(unsigned char slic_id, Le88xxx *data)
{

	writeLegerityReg( slic_id, 0xEA, data);
}

//Caller ID data is send or not.
//1: means we can write another date into E2h.
static unsigned char Legerity_FSK_CallerID_ready(unsigned char slic_id)
{
	Le88xxx data;

	do
	{
		readLegerityReg( slic_id, 0xEA, &data);
		//printk("caller id=%x\n",data.byte1&0xE0);
	}
	while (!(data.byte1 & 0x20));

	return 1;
}

//Caller ID data
static void Legerity_FSK_CallerID_data(unsigned char slic_id, Le88xxx *data)
{

	writeLegerityReg( slic_id, 0xE2, data);
}


static void Legerity_generator_C_D_coeff(unsigned char slic_id, Le88xxx *data, unsigned char wri_re)
{

	if (!wri_re)
	{
		writeLegerityReg( slic_id, 0xD4, data);
	}
	else
	{
		readLegerityReg( slic_id, 0xD4, data);
#ifdef _Legerity_debug_
		PRINT_MSG("Signal generator C and D parameter: \n");
		int i;
		for (i=0;i<4;i++)
			PRINT_MSG("Data byte%d=0x%x, Data byte%d=0x%x\n",2*i+1,*(&(data->byte1)+i*2),2*i+2,*(&(data->byte1)+i*2+1));
#endif
	}
	PRINT_MSG("Set EGC and EGD of DEh to enable signal generator.\n");
	return;
}
#endif

#if 1	// stop type-1 fsk cid gen when phone off-hook
int stop_type1_fsk_cid_gen_when_phone_offhook( voip_dsp_t *this )
{
	const uint32 dch = this ->dch;
	
	if ((fsk_cid_state[dch] == 1) && (cid_info[dch].cid_mode == 0))
	{
		//extern void pcm_clean_tx_fifo(unsigned int chid);
		//extern void pcm_disableChan(unsigned int chid);
	
		init_softfskcidGen(dch);
		fsk_cid_state[dch] = 0;
		//pcm_clean_tx_fifo(cch);
		//bus_fifo_clean_tx_cch( cch );	// caller do this 
		//pcm_disableChan(cch);
		//PRINT_Y("init_cid(%d)\n", i);
		
		return 1;
	}
	
	return 0;
}
#endif


int voip_clid_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int ch;
	int n = 0;
	TstFskClidPara stFskClidParacurrnet;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	if( IS_CH_PROC_DATA( data ) )
	{
		ch = CH_FROM_PROC_DATA( data );
		n = sprintf( buf, "Channel=%d\n", ch );

		n += sprintf( buf + n, "DTMF Caller ID Setting:\n");
		n += sprintf( buf + n, " - bAuto_Ring = %d (ture=1)\n", dtmf_cid_info[ch].bAuto_Ring);
		n += sprintf( buf + n, " - bAuto_SLIC_action = %d (ture=1)\n", dtmf_cid_info[ch].bAuto_SLIC_action);
		n += sprintf( buf + n, " - bBefore1stRing = %d (ture=1)\n", dtmf_cid_info[ch].bBefore1stRing);
		n += sprintf( buf + n, " - bAuto_StartEnd = %d (ture=0)\n", dtmf_cid_info[ch].bAuto_StartEnd);
		n += sprintf( buf + n, " - start_digit = %c\n", 'A' + dtmf_cid_info[ch].start_digit);
		n += sprintf( buf + n, " - end_digit = %c\n", 'A' + dtmf_cid_info[ch].end_digit);
		//n += sprintf( buf + n, " - data = %s\n", dtmf_cid_info[ch].data);
		
		n += sprintf( buf + n, "FSK Caller ID Setting:\n");
		switch (fsk_spec_areas[ch] & 0x7)
		{
			case 0:
				n += sprintf( buf + n, " - Bellcore\n");
				break;
			case 1:
				n += sprintf( buf + n, " - ETSI\n");
				break;
			case 2:
				n += sprintf( buf + n, " - BT\n");
				break;
			case 3:
				n += sprintf( buf + n, " - NTT\n");
				break;
		}
		n += sprintf( buf + n, " - Caller ID Prior First Ring: %s\n", (fsk_spec_areas[ch]&0x8) ? "ON" : "OFF");
		n += sprintf( buf + n, " - Dual Tone before Caller ID (Fsk Alert Tone): %s\n", (fsk_spec_areas[ch]&0x10) ? "ON" : "OFF");
		n += sprintf( buf + n, " - Short Ring before Caller ID: %s\n", (fsk_spec_areas[ch]&0x20) ? "ON" : "OFF");
		n += sprintf( buf + n, " - Reverse Polarity before Caller ID (Line Reverse): %s\n", (fsk_spec_areas[ch]&0x40) ? "ON" : "OFF");
		n += sprintf( buf + n, " - FSK Date & Time Sync and Display Name: %s\n", (fsk_spec_areas[ch]&0x80) ? "ON" : "OFF");
		n += sprintf( buf + n, " - Auto SLIC Action: %s\n", (fsk_spec_areas[ch]&0x100) ? "ON" : "OFF");


		fsk_cid_para_get(ch, fsk_spec_areas[ch] & 0x7, &stFskClidParacurrnet);
		n += sprintf( buf + n, " - ch_seizure_cnt: %d(only used in type 1)\n", stFskClidParacurrnet.ch_seizure_cnt);
		n += sprintf( buf + n, " - mark_cnt: %d\n", stFskClidParacurrnet.mark_cnt);
		n += sprintf( buf + n, " - delay_after_1st_ring: %d ms\n", stFskClidParacurrnet.delay_after_1st_ring);
		n += sprintf( buf + n, " - delay_before_2nd_ring: %d ms\n", stFskClidParacurrnet.delay_before_2nd_ring);
		n += sprintf( buf + n, " - silence_before_sas: %d ms\n", stFskClidParacurrnet.silence_before_sas);
		n += sprintf( buf + n, " - sas_time: %d ms\n", stFskClidParacurrnet.sas_time);
		n += sprintf( buf + n, " - delay_after_sas: %d ms\n", stFskClidParacurrnet.delay_after_sas);
		n += sprintf( buf + n, " - cas_time: %d ms\n", stFskClidParacurrnet.cas_time);
		n += sprintf( buf + n, " - type1_delay_after_cas: %d ms\n", stFskClidParacurrnet.type1_delay_after_cas);
		n += sprintf( buf + n, " - ack_waiting_time: %d ms\n", stFskClidParacurrnet.ack_waiting_time);
		n += sprintf( buf + n, " - delay_after_ack_recv: %d ms\n", stFskClidParacurrnet.delay_after_ack_recv);
		n += sprintf( buf + n, " - delay_after_type2_fsk: %d ms\n", stFskClidParacurrnet.delay_after_type2_fsk);
		n += sprintf( buf + n, " - type2_waiting_for_ack: %d ms [Read only] \n", gtype2_waiting_for_ack_time[ch]);
	}
	
	*eof = 1;
	return n;
}

int __init voip_clid_proc_init( void )
{
	create_voip_channel_proc_read_entry( "clid", voip_clid_read_proc );
	return 0;
}

void __exit voip_clid_proc_exit( void )
{
	remove_voip_channel_proc_entry( "clid" );
}

voip_initcall_proc( voip_clid_proc_init );
voip_exitcall( voip_clid_proc_exit );

