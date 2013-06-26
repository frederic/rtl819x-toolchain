#include <linux/timer.h>
#include <linux/interrupt.h>

#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_params.h"
#include "voip_control.h"
#include "con_register.h"
#include "con_mux.h"
#include "snd_define.h"

#include "dsp_rtk_caller.h"
#include "fsk.h"

#ifndef AUDIOCODES_VOIP
#include "../voip_dsp/include/fskcid_gen.h"
#include "tone_det_i.h"
#include "fsk_det.h"
#else
#include "RTK_AC49xApi_Interface.h"
#include "dsp_ac_define.h"
#endif
#include "../voip_dsp/include/Ntt_sRing_det.h"

#include "voip_init.h"
#include "voip_proc.h"

#include "../voip_manager/voip_mgr_events.h"

#define PHONE_ON_HOOK		0
#define PHONE_OFF_HOOK		1
#define PHONE_FLASH_HOOK	2
#define PHONE_STILL_ON_HOOK	3
#define PHONE_STILL_OFF_HOOK	4
#define FXO_ON_HOOK			6
#define FXO_OFF_HOOK		7
#define	FXO_FLASH_HOOK		8
#define FXO_STILL_ON_HOOK	9
#define FXO_STILL_OFF_HOOK	10

// --------------------------------------------------------
// Polling job - Hook (FXS/DAA status are included)
// --------------------------------------------------------
typedef struct {
	//unsigned char CH;		// CH:0 - 3
	unsigned char change;		// 1: Change. 0: No-Change
	unsigned char hook_status;	// 1: Off-Hook, 0: On-Hook
} hook_struck;

unsigned int flash_hook_time[CON_CH_NUM]; /* flash_hook_time * 10ms */
unsigned int flash_hook_min_time[CON_CH_NUM]; /* flash_hook_min_time * 10ms */
static int HookPollFlag[CON_CH_NUM] = {[0 ... CON_CH_NUM-1] = 1};

static unsigned int on_cnt[CON_CH_NUM];
static unsigned char pre_status[CON_CH_NUM], on_set[CON_CH_NUM], off_set[CON_CH_NUM], check_flash[CON_CH_NUM];

/*
* on_cnt: count how many time "ON-HOOK" event happens.
* on_set: if "ON-HOOK" event happens, on_set is set to 1.
* off_set: if "OFF-HOOK" event happens, off_set is set to 1.
* check_flash: if check_flash is 1, it means Hook_Polling_Silicon() need to detect FLASH HOOK event.
* pre_status: record previous hook status (only record 1: off-hook and 0: on-hook)
*/


// (dch)
static unsigned int stop_poll[MAX_DSP_CH_NUM] = {0}, stop_poll_cnt[MAX_DSP_CH_NUM] = {0};

#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
typedef struct
{
	unsigned short reserved1 __attribute__ ((aligned (16)));   /* aligned in a cache line, good data hit rate */
	unsigned short off_hook_cnt;
	unsigned short current_drop_cnt;
	unsigned short current_Ndrop_cnt;	/* no current drop */
	unsigned short stable_pre_daa_polarity_cnt;
	unsigned short daa_polarity_cnt;
	unsigned char stable_pre_daa_polarity;
	unsigned char init_daa_polarity;
	unsigned char current_drop_reported;
	unsigned char previous_daa_polarity;
}
Tstdaa_detect;//For detect daa current drop, polarity reverse.

static Tstdaa_detect stdaa_detect[CON_CH_NUM];

//static unsigned int previous_daa_polarity[MAX_VOIP_CH_NUM];
//static unsigned int daa_polarity;
unsigned int pre_daa_status[CON_CH_NUM];
//static unsigned int pre_daa_voltage_abs[MAX_VOIP_CH_NUM];/* current drop detect add 20080811 */
//static unsigned int pre_daa_type = 2173076833;
//static unsigned int off_hook_cnt[MAX_VOIP_CH_NUM];
//static unsigned int stable_pre_daa_polarity[MAX_VOIP_CH_NUM];
//static unsigned int init_daa_polarity[MAX_VOIP_CH_NUM]; /* when offhook and polarity hold 100ms, init OK. */
#endif

extern char fsk_cid_state[];		// for FSK CID
//#ifdef AUDIOCODES_VOIP
//static char fsk_cid_enable[MAX_DSP_AC_CH_NUM]={0};
//#endif
char ntt_skip_dc_loop[MAX_DSP_CH_NUM]={0};	// (dch)

#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
static inline int SLIC_Hook_Polling_DAA( const voip_con_t *p_con )
{
	const uint32 cch = p_con ->cch;
	voip_snd_t * const p_snd = p_con ->snd_ptr;
	
	int status;
	
	// snd_type == SND_TYPE_DAA

#ifdef HW_FXO_REVERSAL_DET
	if ( p_snd ->daa_ops ->DAA_Polarity_Reversal_Det( p_snd ) )	// Thlin add
	{
		voip_event_hook_in( cch, VEID_HOOK_FXO_POLARITY_REVERSAL );
	}
#endif

#ifdef HW_FXO_BAT_DROP_OUT
	if ( p_snd ->daa_ops ->DAA_Bat_DropOut_Det( p_snd ) )	// Thlin add
	{
		voip_event_hook_in( cch, VEID_HOOK_FXO_CURRENT_DROP );
	}
#endif
	status = p_snd ->daa_ops ->DAA_Hook_Status( p_snd, 1 );

#if 1
	if(status) /* off-hook */
	{
		//int daa_voltage;
		int daa_voltage_abs;

		if (stdaa_detect[cch].off_hook_cnt > 10) /* delay 100ms after offhook */
		//if(pre_daa_status[cch])
		{
			//daa_voltage = readDAAReg(hook->CH, 29);
			//daa_voltage_abs = daa_voltage;
			daa_voltage_abs = p_snd ->daa_ops ->DAA_Get_Line_Voltage( p_snd );

#ifdef SW_FXO_REVERSAL_DET
			//daa_polarity = (daa_voltage & 0x80) >>7 ;
			daa_polarity = p_snd ->daa_ops ->DAA_Get_Polarity( p_snd );
			
			//if (daa_polarity != previous_daa_polarity[cch])
			//	hook_in(cch, FXO_POLARITY_REVERSAL);
			if (stdaa_detect[cch].init_daa_polarity)
			{
				if (  (daa_polarity == stdaa_detect[cch].previous_daa_polarity)
				    &&(daa_polarity != stdaa_detect[cch].stable_pre_daa_polarity)       )
				{
					stdaa_detect[cch].daa_polarity_cnt ++;

				}
				else
					stdaa_detect[cch].daa_polarity_cnt = 0;

				if (stdaa_detect[cch].daa_polarity_cnt > 10)
				{
					stdaa_detect[cch].daa_polarity_cnt = 0;
					stdaa_detect[cch].stable_pre_daa_polarity = daa_polarity;
					voip_event_hook_in( cch, VEID_HOOK_FXO_POLARITY_REVERSAL );
				}
			}
			else
			{
				if (daa_polarity == stdaa_detect[cch].previous_daa_polarity)
					stdaa_detect[cch].stable_pre_daa_polarity_cnt++;
				else
					stdaa_detect[cch].stable_pre_daa_polarity_cnt = 0;

				if (stdaa_detect[cch].stable_pre_daa_polarity_cnt > 10)
				{
					stdaa_detect[cch].init_daa_polarity = 1;
					stdaa_detect[cch].stable_pre_daa_polarity = stdaa_detect[cch].previous_daa_polarity;
					stdaa_detect[cch].daa_polarity_cnt = 0;
					stdaa_detect[cch].stable_pre_daa_polarity_cnt = 0;
				}
			}

			stdaa_detect[cch].previous_daa_polarity = daa_polarity;
#endif

			//if (daa_voltage < 0)
				//daa_voltage_abs = ~daa_voltage;
			if (!stdaa_detect[cch].current_drop_reported)
			{
				if (daa_voltage_abs <5 )
					stdaa_detect[cch].current_drop_cnt++;
				else
					stdaa_detect[cch].current_drop_cnt = 0;

				if (stdaa_detect[cch].current_drop_cnt > 10)
				{
					voip_event_hook_in( cch, VEID_HOOK_FXO_CURRENT_DROP );
					stdaa_detect[cch].current_drop_reported = 1;
					stdaa_detect[cch].current_drop_cnt = 0;
				}
			}
			else
			{
				if (daa_voltage_abs >= 5)
				{
					stdaa_detect[cch].current_drop_cnt = 0;
					stdaa_detect[cch].current_Ndrop_cnt++;
				}
				else
					stdaa_detect[cch].current_Ndrop_cnt = 0;

				if (stdaa_detect[cch].current_Ndrop_cnt > 10)
				{
					stdaa_detect[cch].current_drop_reported = 0;
					stdaa_detect[cch].current_Ndrop_cnt = 0;
				}
			}

			//pre_daa_voltage_abs[cch] = daa_voltage_abs;
		}
		else
		{
			stdaa_detect[cch].off_hook_cnt ++;
			//daa_voltage = readDAAReg(hook->CH, 29);
			//daa_voltage_abs = daa_voltage;
			daa_voltage_abs = p_snd ->daa_ops ->DAA_Get_Line_Voltage( p_snd );
#ifdef SW_FXO_REVERSAL_DET
			stdaa_detect[cch].previous_daa_polarity = p_snd ->daa_ops ->DAA_Get_Polarity( p_snd );
#endif
			//if (daa_voltage < 0)
				//daa_voltage_abs = ~daa_voltage;
			//stdaa_detect[cch].pre_daa_voltage_abs = daa_voltage_abs;
		}
	}
	else //on-hook
	{
		stdaa_detect[cch].current_drop_reported = 0;
		stdaa_detect[cch].current_drop_cnt = 0;
		stdaa_detect[cch].current_Ndrop_cnt = 0;
		stdaa_detect[cch].off_hook_cnt = 0;
#ifdef SW_FXO_REVERSAL_DET
		stdaa_detect[cch].init_daa_polarity = 0;
		stdaa_detect[cch].stable_pre_daa_polarity_cnt = 0;
#endif
	}
	pre_daa_status[cch] = status;
	
#endif // #if 0

	return status;
}
#endif

static inline int SLIC_Hook_Polling_FXS__FSK_NTT( const voip_con_t *p_con, int status )
{	
	const uint32 cch = p_con ->cch;
	const uint32 dch = p_con ->dsp_ptr ->dch;
	
	if ((fsk_spec_areas[dch]&7)==FSK_NTT)
	{
		if (fsk_cid_state[dch] || ntt_skip_dc_loop[dch] )    //when send caller id ignore the off hook event.
		{
			stop_poll[dch] = 1;
		}
	
		if ((stop_poll[dch] == 1) && (status == 0)) //when NTT phone, off-hook -> on-hook, continue to poll.
		{
			stop_poll[dch] = 0;
			stop_poll_cnt[dch] = 0;
		}
	
		if (stop_poll[dch] == 1)
		{
			status=pre_status[cch];
	
			if (stop_poll_cnt[dch]++ > 70)
			{
				stop_poll[dch] = 0;
				stop_poll_cnt[dch] = 0;
				PRINT_MSG("Force to start poll hook status for NTT\n");
			}
		}
	}
	
	return status;
}

static inline void SLIC_Hook_Polling_Final( const voip_con_t * p_con, 
		hook_struck *hook, int status,
		unsigned int flash_hook_min_duration, unsigned int flash_hook_duration )
{
	
	const uint32 cch = p_con ->cch;
	
	if (status)
	{
		/* on_cnt[] >= 10*flash_hook_min_duration[cch] ms and
		 * on_cnt[] < 10*flash_hook_duration[cch] ms,
		 * then flash event happen.
		 */
		if (check_flash[ cch ] == 1)
		{
			if (on_cnt[ cch ] <= flash_hook_min_duration)
			{
				if (!(off_set[ cch ]))
				{
					hook->hook_status = PHONE_OFF_HOOK;
					off_set[ cch ] = 1;
					on_set[ cch ] = 0;
					//printk("*** OFF ***\n");
				}
				else
				{
					hook->hook_status = PHONE_STILL_ON_HOOK;
					//printk("*** S-ON 1***\n");
				}

			}
			else if ((on_cnt[ cch ] >= flash_hook_min_duration) && (on_cnt[ cch ] < flash_hook_duration))
			{
				hook->hook_status = PHONE_FLASH_HOOK;
				//printk("*** FLASH ***\n");
			}
			else
			{
				if (!(off_set[ cch ]))
				{
					hook->hook_status = PHONE_OFF_HOOK;
					off_set[ cch ] = 1;
					on_set[ cch ] = 0;
					//printk("*** OFF ***\n");
				}
				else
				{
					hook->hook_status = PHONE_STILL_ON_HOOK;
					//printk("*** S-ON 1***\n");
				}

			}

			check_flash[ cch ] = 0;
		}
		else
		{
			hook->hook_status = PHONE_STILL_OFF_HOOK;
			//printk("*** S-OFF 1***\n");
		}

		on_cnt[ cch ] = 0;

	}
	else
	{
		if (pre_status[cch] == 1) 	/* prev = off-hook */
			check_flash[cch] = 1;

		if (on_cnt[cch] >= flash_hook_duration)
		{
			on_cnt[cch] = flash_hook_duration; /* avoid on_cnt[] to overflow */

			if (on_set[cch] == 0)
			{
				hook->hook_status = PHONE_ON_HOOK;
				on_set[cch] = 1;
				off_set[cch] = 0;
				//printk("*** ON ***\n");
			}
			else
			{
				hook->hook_status = PHONE_STILL_ON_HOOK;
				//printk("*** S-ON 2***\n");
			}
		}
		else
		{
			hook->hook_status = PHONE_STILL_OFF_HOOK;
			//printk("*** S-OFF 2***\n");
		}

		//printk("%d\n", on_cnt[cch]);
		on_cnt[cch]++;
	}

	pre_status[cch] = status;
}

static inline void Pluse_Dial_Det_Handle_Hook( 
	const voip_con_t *p_con, const voip_snd_t * const p_snd, 
	int status );
			
static inline int SLIC_Hook_Polling( 
	const voip_con_t *p_con, voip_snd_t * const p_snd,
	hook_struck *hook, unsigned int fhk_min_time, unsigned int fhk_time)
{
	//static unsigned int flash_hook_min_duration[CON_CH_NUM], flash_hook_duration[CON_CH_NUM];
	int status;		/* 1:off-hook, 0:on-hook, -1: ignore */
	unsigned long flags;
	//extern int Is_DAA_Channel(int chid);
	
	//flash_hook_min_duration[cch] = fhk_min_time;
	//flash_hook_duration[cch] = fhk_time;

	save_flags(flags);
	cli();
	
	switch( p_snd ->snd_type ) {
	case SND_TYPE_FXS:
		// 1.9530
		// 1.8419 inline + dmem
		// 1.6065 (.5519) inline + dmem (gpio/desc)
		// 1.4192 (.7122) inline + no debug 
		status = p_snd ->fxs_ops ->SLIC_Get_Hook_Status( p_snd, 1 ); /* 1:off-hook  0:on-hook */
		status = SLIC_Hook_Polling_FXS__FSK_NTT( p_con, status );
		break;
		
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
	case SND_TYPE_DAA:
		// 3.0881 (2.0226) inline + no debug 
		status = SLIC_Hook_Polling_DAA( p_con );
		break;
#endif
		
	case SND_TYPE_VDAA:
		status = p_snd ->vdaa_ops ->virtual_daa_hook_detect( p_snd ); /* 1:off-hook  0:on-hook */
		break;
		
	case SND_TYPE_DECT:	// dect process handle it
	case SND_TYPE_AC:	// ui process this key. 
	default:
		status = -1;
		break;
	}
	
	restore_flags(flags);
	
	// use 'status' to check 
	if( status < 0 )
		return status;
	
	// fill 'hook' struct 
	SLIC_Hook_Polling_Final( p_con, hook, status, 
								fhk_min_time, fhk_time );
#ifdef PULSE_DIAL_DET
	Pluse_Dial_Det_Handle_Hook( p_con, p_snd, status );
#endif

	return 0;
}

static void ENTRY_SLIC_Hook_Polling( 
						const voip_con_t *p_con, 
						voip_snd_t * const p_snd )
{
	//extern TstVoipFskClid cid_info[VOIP_CH_NUM];

	const uint32 cch = p_con ->cch;
	hook_struck hook_res;
	
	if( SLIC_Hook_Polling( p_con, p_snd, &hook_res, 
			flash_hook_min_time[ cch ], flash_hook_time[ cch ] ) < 0 )
	{
		return;
	}

	if ( (&hook_res)->hook_status == PHONE_ON_HOOK)
	{
		//if (Is_DAA_Channel(i) == 1)
		if (Is_DAA_snd(p_con ->snd_ptr)) {
			voip_event_hook_in( cch, VEID_HOOK_FXO_ON_HOOK );
		} else {
			voip_event_hook_in( cch, VEID_HOOK_PHONE_ON_HOOK );
		}
		//printk("hook_in: ON\n");
	}
	else if ( (&hook_res)->hook_status == PHONE_OFF_HOOK)
	{
		//if (Is_DAA_Channel(i) == 1)
		if (Is_DAA_snd(p_con ->snd_ptr)) {
			voip_event_hook_in( cch, VEID_HOOK_FXO_OFF_HOOK );
		} else
		{
#ifndef CONFIG_AUDIOCODES_VOIP
			voip_dsp_t * p_dsp = p_con ->dsp_ptr;
#endif

			voip_event_hook_in( cch, VEID_HOOK_PHONE_OFF_HOOK );
			//printk("hook_in: OFF\n");

#ifndef CONFIG_AUDIOCODES_VOIP
#if 1	// stop type-1 fsk cid gen when phone off-hook
			if( p_dsp ->dsp_type == DSP_TYPE_REALTEK &&
				p_dsp ->dsp_ops ->stop_type1_fsk_cid_gen_when_phone_offhook( p_dsp ) ) 
			{
				p_con ->con_ops ->bus_fifo_clean_tx( ( voip_con_t * )p_con );
			}	
#endif
#endif
		}
	}
	else if ( (&hook_res)->hook_status == PHONE_FLASH_HOOK)
	{
		//if (Is_DAA_Channel(i) == 1)
		if (Is_DAA_snd(p_con ->snd_ptr))
		{
			//hook_in(i, FXO_FLASH_HOOK);
		}
		else {
			voip_event_hook_in( cch, VEID_HOOK_PHONE_FLASH_HOOK );
		}
		//printk("hook_in: Flash\n");
	}
	else
	{
#if 0	// we don't need these events 
		//if (Is_DAA_Channel(i) == 1)
		if (Is_DAA_snd(p_con ->snd_ptr)) {
			voip_event_hook_in( cch, VEID_HOOK_FXO_UNKNOWN );
		} else {
			voip_event_hook_in( cch, VEID_HOOK_PHONE_UNKNOWN );
		}
#endif
	}

}

void Init_Hook_Polling(unsigned char cch)
{
	check_flash[cch] = 1;
	pre_status[cch] = 0;
	on_cnt[cch] = 0;
	on_set[cch] = 0;
	off_set[cch] = 0;
}

// --------------------------------------------------------
// Polling job - Virtual DAA status 
// --------------------------------------------------------

#ifdef CONFIG_RTK_VOIP_LED
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
	extern unsigned int pstn_ringing;
#else
	extern unsigned int pstn_ringing[];
#endif
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
extern char relay_2_PSTN_flag[];
#endif

////// Virtual DAA Ring Detection ////////
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA

#define VIR_DAA_RING_DET_DEBUG	0		/* Enable to print the (ring_on_cnt, ring_off_cnt), enginner can fine tune the ring on/off threshold by this debug message. */
static unsigned int vir_Ring_On_Ths = 25;	/* This threshold is not mapping to sec, enginner need to tune it for desired Ring pattern.*/
static unsigned int vir_Ring_Off_Ths = 500;	/* 5 sec */
/* (50, 500)->For Ring pattern: 20Hz, AC:75 Vrms, DC:45 V, 1s on- 4s off */

static void vir_daa_ring_det_set(unsigned int on_ths, unsigned int off_ths)
{
	vir_Ring_On_Ths = on_ths;
	vir_Ring_Off_Ths = off_ths;
}

static void ENTRY_VDAA_Hook_Polling( 
						const voip_con_t *p_con, 
						voip_snd_t * const p_snd )
{
	???? // need review 

#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
	static int vir_daa_ring_on_cnt = 0, vir_daa_ring_off_cnt = 0, vir_daa_ring_flag =0;
#endif
	
	#if VIR_DAA_RING_DET_DEBUG
		printk("%d, %d\n", vir_daa_ring_on_cnt, vir_daa_ring_off_cnt);
	#endif
	
	/**************************************************************************************************/
	/***** Reset vir_daa_ring_on_cnt=0 if vir_daa_ring_on_cnt doesn't change for 10 times polling. ******/
	
	static int k=0;
	volatile static int y;
	unsigned int reset_cnt=10;
	
	k = (k+1)%reset_cnt;
	
	if (k==0) y=vir_daa_ring_on_cnt;
	
	if (k==(reset_cnt-1))
	{
		if ( y == vir_daa_ring_on_cnt)
		{
			vir_daa_ring_on_cnt = 0;
		}
	}
	
	/**************************************************************************************************/
	
	if (1 == virtual_daa_ring_incoming_detect())
	{
		if (++vir_daa_ring_on_cnt > vir_Ring_On_Ths )
		{
			vir_daa_ring_on_cnt = vir_Ring_On_Ths; // avoid overflow

			if ( 0 == vir_daa_ring_flag )
			{
				int cch;
				for (cch=0; cch<CON_CH_NUM; cch++) {
					if( get_snd_type_cch( cch ) != SND_TYPE_FXS )
						continue;
						
					voip_event_hook_in( cch, VEID_HOOK_FXO_RING_ON );
				}
				vir_daa_ring_on_cnt = 0;
				vir_daa_ring_flag = 1;
				PRINT_MSG("Virtual DAA Ringing on.\n");
#ifdef CONFIG_RTK_VOIP_LED				
				pstn_ringing = 1;
#endif
			}
		}

		vir_daa_ring_off_cnt = 0;

	}
	else
	{
		if ( 1 == vir_daa_ring_flag )
		{

			vir_daa_ring_off_cnt++;			

			if (vir_daa_ring_off_cnt > vir_Ring_Off_Ths)
			{
				int cch;
				for (cch=0; cch<CON_CH_NUM; cch++) {
					if( get_snd_type_cch( cch ) != SND_TYPE_FXS )
						continue;
			#if 0
					if (relay_2_PSTN_flag[cch] == 0)//relay is at SLIC
					{
						voip_event_hook_in( cch, VEID_HOOK_FXO_RING_OFF );
					}
					else
					{
						// when relay is at PSTN, it means Phone<-> PSTN is connected.
						// So, do nothing.
					}
			#else
					voip_event_hook_in( cch, VEID_HOOK_FXO_RING_OFF );
			#endif
				}

				vir_daa_ring_on_cnt = 0;
				vir_daa_ring_flag = 0;
				PRINT_MSG("Virtual DAA Ringing off.\n");
#ifdef CONFIG_RTK_VOIP_LED
				pstn_ringing = 0;
#endif
			}
		}
	}
}
#endif

// --------------------------------------------------------
// Polling job - DAA status 
// --------------------------------------------------------
#ifdef CONFIG_RTK_VOIP_DRIVERS_DAA_SUPPORT
static int now_stat[CON_CH_NUM]={0}, pre1_stat[CON_CH_NUM]={0},
		pre2_stat[CON_CH_NUM]={0}, pre3_stat[CON_CH_NUM]={0};
#endif

////// DAA Ring Detection ////////
static unsigned int Ring_On_Cnt = 150;
static unsigned int Ring_Off_Cnt = 80;
static unsigned int Ring_Off_Event_Ths = 250;

static unsigned int ring_on_time[CON_CH_NUM];
static unsigned int ring_off_time[CON_CH_NUM];
static int ring_on[CON_CH_NUM];
static int ring_off[CON_CH_NUM];
static int wait_ring_off[CON_CH_NUM], wait_ring_off_timeout[CON_CH_NUM];
static int check_ring_off_time_out[CON_CH_NUM];

#ifdef AUDIOCODES_VOIP
static int fsk_decode_complete_flag[CON_CH_NUM]={0}; // for NTT CID detection usage
#endif

void ring_det_cad_set( unsigned int ring_on_msec, unsigned int first_ringoff_msec, unsigned int ring_off_msec)
{
#ifndef FXO_RING_NO_DET_CADENCE

	Ring_On_Cnt = ring_on_msec/10;
	Ring_Off_Cnt = first_ringoff_msec/10;
	Ring_Off_Event_Ths = ring_off_msec/10;

	/* Accept 12.5% deviation of cadence */
	Ring_On_Cnt = Ring_On_Cnt - (Ring_On_Cnt>>3);
	Ring_Off_Cnt = Ring_Off_Cnt - (Ring_Off_Cnt>>3);

	/* Add 12.5% deviation to Ring-off Event judgement */
	Ring_Off_Event_Ths = Ring_Off_Event_Ths + (Ring_Off_Event_Ths>>3);

	//printk("((((( %d, %d, %d )))))\n", Ring_On_Cnt*10, Ring_Off_Cnt*10, Ring_Off_Event_Ths*10);
#else
	int cch;
	extern long auto_cid_det[], cid_type[];
	//PRINT_R("<<<<<<<< ring_det_cad_set >>>>>>>>>\n");
	
	Ring_On_Cnt = ring_on_msec/10;
	Ring_Off_Cnt = first_ringoff_msec/10;
	Ring_Off_Event_Ths = ring_off_msec/10;
	
	for (cch = 0; cch < con_ch_num; cch++)
	{
		const int dch = cch;
		
		if( get_snd_type_cch( cch ) != SND_TYPE_DAA )
			continue;
		
		//PRINT_R("%d, %d\n", auto_cid_det[i], cid_type[i]);
		
		switch(auto_cid_det[dch])
		{
			case 0:	//AUTO_CID_DET_OFF
				if (3 == cid_type[dch]) //NTT
				{
					if (Ring_On_Cnt < 65)
						Ring_On_Cnt = 65;
					if (Ring_Off_Cnt < 65)
						Ring_Off_Cnt = 65;
				}
				break;
			case 1: //AUTO_CID_DET_ON_NTT
				if (Ring_On_Cnt < 65)
					Ring_On_Cnt = 65;
				if (Ring_Off_Cnt < 65)
					Ring_Off_Cnt = 65;
				break;
			case 2: //AUTO_CID_DET_ON_NO_NTT
				break;
			default:			
				break;
		}
		//PRINT_G("<<<<<<<< Ring_On_Cnt=%d, ch=%d >>>>>>>>>\n", Ring_On_Cnt, i);
	}
	
#endif
}

void ring_times_set(unsigned int cch, unsigned int ringOn, unsigned int ringOff)
{
	ring_on_time[cch] = ringOn;
	ring_off_time[cch] = ringOff;
	//printk("============> (%d, %d)<===============\n", ring_on_time[chid], ring_off_time[chid]);
}

#ifdef CONFIG_RTK_VOIP_DRIVERS_DAA_SUPPORT
static void ENTRY_DAA_Status_Polling( 
						const voip_con_t *p_con, 
						voip_snd_t * const p_snd )
{
	// Polling DAA ring and busy tone flag
	extern void DAA_PollPulseGenFifo(const voip_con_t *p_con);

	int ring_state;
	static int daa_ring_on_cnt[CON_CH_NUM] = {0}, daa_ring_off_cnt[CON_CH_NUM] = {0}, daa_ring_flag[CON_CH_NUM] = {0};
	const uint32 cch = p_con ->cch;
	const uint32 dch = p_con ->dsp_ptr ->dch;

#ifndef AUDIOCODES_VOIP
	int temp_ring_tone_flag_get;
#else
	long daa_stat[CON_CH_NUM]={0};
	static int ring_and_offhook[CON_CH_NUM] = {0};
	static int ntt_offhook_period_cnt[CON_CH_NUM] = {0};
#endif
	
	//printk("%d %d\n", daa_ring_on_cnt[i], daa_ring_off_cnt[i]);
	ring_state = p_snd ->daa_ops ->DAA_Positive_Negative_Ring_Detect( p_snd );
	if (ring_state)
		now_stat[cch] = 1;
	else
		now_stat[cch] = 0;

	if ( now_stat[cch] ==  pre3_stat[cch] )
	{
		pre1_stat[cch] = now_stat[cch];
		pre2_stat[cch] = now_stat[cch];
	}
	else if ( pre1_stat[cch] == pre3_stat[cch]  )
	{
		pre2_stat[cch] = pre1_stat[cch];
	}

	if (pre2_stat[cch])
	{
		check_ring_off_time_out[cch] = 0;
		if (daa_ring_on_cnt[cch] < 32000)
			daa_ring_on_cnt[cch]++;   // avoid overflow

		if (daa_ring_on_cnt[cch] > Ring_On_Cnt )
		{
			if ( 0 == daa_ring_flag[cch])
			{
				if (daa_ring_on_cnt[cch] >= Ring_On_Cnt)	   // should small than 30
				{
					if (wait_ring_off[cch] ==0)
					{
						ring_on[cch] ++;
						PRINT_MSG("ring_on[%d]=%d\n", cch, ring_on[cch]);
					}
					wait_ring_off[cch] = 1;
				}

				if (ring_on[cch] >= ring_on_time[cch] && ring_off[cch] >= ring_off_time[cch])//  ring(1) -- silence -- ring(2) -- silence -- ring(3)
				{
#ifdef CONFIG_RTK_VOIP_LED
					pstn_ringing[cch] = 1;
#endif
					voip_event_hook_in( cch, VEID_HOOK_FXO_RING_ON );
					daa_ring_on_cnt[cch] = 0;
					daa_ring_flag[cch] = 1;
					ring_on[cch] = 0;
					ring_off[cch] = 0;
					wait_ring_off[cch] = 0;
					PRINT_MSG("DAA Ringing on(%d).\n", cch);
				}
			}

		}

		//if (daa_ring_off_cnt[cch] != 0)
		//      PRINT_G("off= %d\n", daa_ring_off_cnt[cch]);
		daa_ring_off_cnt[cch] = 0;
	}
	else
	{
		if ( (1 == daa_ring_flag[cch]) && (1 == p_snd ->daa_ops ->DAA_Hook_Status( p_snd, 0 )) )// daa is off-hook, reset Ring Off counter
		{
			daa_ring_on_cnt[cch] = 0;
			daa_ring_off_cnt[cch] = 0;
			daa_ring_flag[cch] = 0;
			PRINT_MSG("Reset Ring-off counter(%d).\n", cch);
#ifdef CONFIG_RTK_VOIP_LED
			pstn_ringing[cch] = 0;
#endif
		}
	
		if (daa_ring_off_cnt[cch] < 32000)
			daa_ring_off_cnt[cch]++;
			
		if ( 1 == daa_ring_flag[cch])
		{

			if (daa_ring_off_cnt[cch] > Ring_Off_Event_Ths)
			{

				if ( 0 == p_snd ->daa_ops ->DAA_Hook_Status( p_snd, 0 ))// daa is on-hook (must, important)
				{
					voip_event_hook_in( cch, VEID_HOOK_FXO_RING_OFF );
				}
				else
				{
					// when daa is off-hhok, it means FXS<->FXO VoIP is connected.
					// So, do nothing.
				}
				daa_ring_on_cnt[cch] = 0;
				daa_ring_flag[cch] = 0;
				PRINT_MSG("DAA Ringing off(%d).\n", cch);
#ifdef CONFIG_RTK_VOIP_LED
				pstn_ringing[cch] = 0;
#endif
			}
		}
		else
		{

			if (wait_ring_off[cch] == 1)
			{
				if (daa_ring_off_cnt[cch] > Ring_Off_Cnt)
				{
					ring_off[cch] ++;
					PRINT_MSG("ring_off[%d]=%d\n", cch, ring_off[cch]);
					wait_ring_off[cch] = 0;
					
					wait_ring_off_timeout[cch] = 0;
					check_ring_off_time_out[cch] = 1;
				}
			}

			if (check_ring_off_time_out[cch] == 1)
			{
				if ( ++wait_ring_off_timeout[cch] > (Ring_Off_Event_Ths-Ring_Off_Cnt))
				{
					check_ring_off_time_out[cch] = 0;
					wait_ring_off_timeout[cch] = 0;
					ring_on[cch] = 0;
					ring_off[cch] = 0;
					PRINT_MSG("ring off(%d) time out\n", cch);
				}
			}
		}
		//if (daa_ring_on_cnt[cch] != 0)
		//      PRINT_R("on= %d\n", daa_ring_on_cnt[cch]);
		daa_ring_on_cnt[cch] = 0;

	}
	pre3_stat[cch] = pre2_stat[cch];
	pre2_stat[cch] = pre1_stat[cch];
	pre1_stat[cch] = now_stat[cch];

#ifndef AUDIOCODES_VOIP

	if (1 == busy_tone_flag_get(dch))	//should add daa chid
	{
		busy_tone_det_init(dch);	//should add daa chid
		p_snd ->daa_ops ->DAA_On_Hook( p_snd );
		voip_event_hook_in( cch, VEID_HOOK_FXO_BUSY_TONE );
	}

	if (1 == dis_tone_flag_get(dch))
	{
		dis_tone_det_init(dch);
		p_snd ->daa_ops ->DAA_On_Hook( p_snd );
		voip_event_hook_in( cch, VEID_HOOK_FXO_DIS_TONE );
	}

	temp_ring_tone_flag_get = ring_tone_flag_get(dch);
	if (temp_ring_tone_flag_get & 0x5)	//should add daa chid
	{
		//busy_tone_det_init(cch);	//should add daa chid
		//DAA_On_Hook(cch);
		if(1 == temp_ring_tone_flag_get) {
			voip_event_hook_in( cch, VEID_HOOK_FXO_RING_TONE_ON );
		} else {
			voip_event_hook_in( cch, VEID_HOOK_FXO_RING_TONE_OFF );
		}
	}
#endif


	// NTT Short Ring Detection

	if (ring_state)
		NTT_sRing_det(dch, 1);/* ringing */
	else
		NTT_sRing_det(dch, 0);/* non-ringing */

#ifdef AUDIOCODES_VOIP
	// if short(alert) ring is detected, off-hook daa.
	daa_stat[cch] = p_snd ->daa_ops ->DAA_Hook_Status( p_snd, 0 );
	
	if ( (ntt_sRing_on_pass[cch] >= ntt_sRing_on_cnt) &&  (ntt_sRing_off_pass[cch] >= ntt_sRing_off_cnt) )
	{
		if(!daa_stat[cch])// DAA on-hook
		{
			p_snd ->daa_ops ->DAA_Off_Hook( p_snd );	// for ntt cid, need off-hook to recive the cid data.
			ring_and_offhook[cch] = 1;
			PRINT_MSG("offhook(NTT), ch%d\n", cch);
		}

		ntt_sRing_on_pass[cch]= 0 ;
		ntt_sRing_off_pass[cch] = 0;
		ring_and_offhook[cch] = 1;
		ntt_offhook_period_cnt[cch] = 0;	/* reset ntt_offhook_period_cnt to zero , avoid on-hook earily  */
	}
	
	if (daa_stat[cch]) // DAA off-hook
	{
		if (fsk_decode_complete_flag[cch] == 1) // NTT cid is detected
		{
			if (ring_and_offhook[cch] == 1)
			{
				p_snd ->daa_ops ->DAA_On_Hook( p_snd );	// for ntt cid, need on-hook when recive the cid data end.
				ring_and_offhook[cch] = 0;
				PRINT_MSG("onhook(NTT), ch%d\n", cch);
			}
			fsk_decode_complete_flag[cch] = 0;
		}
		else if (ring_and_offhook[cch] == 1)
		{
			ntt_offhook_period_cnt[cch]++;
			if(ntt_offhook_period_cnt[cch]>580)
			{
				p_snd ->daa_ops ->DAA_On_Hook( p_snd );	// for ntt cid, need on-hook when not recive the cid data over 6 sec.
				PRINT_MSG("time out: onhook(NTT), ch%d\n", cch);
				ring_and_offhook[cch] = 0;
				ntt_offhook_period_cnt[cch] = 0;
				fsk_decode_complete_flag[cch] = 0;
			}
		}
	}
#endif //AUDIOCODES_VOIP

#ifdef PULSE_DIAL_GEN
#ifdef CONFIG_RTK_VOIP_PULSE_DIAL_GEN_TIMER
	DAA_PollPulseGenFifo(p_con);
#else
	DAA_PulseGenProcess(p_con);
#endif
#endif
}
#endif //CONFIG_RTK_VOIP_DRIVERS_DAA_SUPPORT


// --------------------------------------------------------
// Polling job - FSK caller ID (nothing)
// --------------------------------------------------------


// --------------------------------------------------------
// Polling job - DTMF caller ID 
// --------------------------------------------------------

#ifdef SW_DTMF_CID
static void ENTRY_DTMF_CID_Hook_Polling( 
						const voip_con_t *p_con, 
						voip_snd_t * const p_snd )
{
	//printk("*");
	extern timetick_t gFirstRingOffTimeOut[];
	extern unsigned char gRingGenAfterCid[];
	//extern char dtmf_cid_state[];
	//ring_struct ringing;
	//extern int ring_off_cnt_start[];
	const uint32 cch = p_con ->cch;
	const uint32 dch = p_con ->dsp_ptr ->dch;
	 
	 //printk("[%d]", gRingGenAfterCid[chid]);
	 //if ((gRingGenAfterCid[chid] == 1) && (ring_off_cnt_start[chid] == 1))
	 if (gRingGenAfterCid[dch] == 1)
	 {
	     //printk("=>%d, J=%d\n", gFirstRingOffTimeOut[chid], timetick);
	     if (timetick_after(timetick, gFirstRingOffTimeOut[dch]))    // after 1st Ring Off cadence time out, gen 2nd Ring.
	     {
	         extern unsigned char ioctrl_ring_set[];
			 //PRINT_R("ioctrl_ring_set[%d] = %d\n", chid, ioctrl_ring_set[chid]);
	
			 if ((ioctrl_ring_set[cch]&0x2) && (!(ioctrl_ring_set[cch]&0x1)) && (!(fsk_spec_areas[dch]&0x08)))
	         {
	              /* App set Ring disable, so DSP no need to Ring atuomatically. */
			 		//PRINT_R("1: ioctrl_ring_set[%d] = %d\n", chid, ioctrl_ring_set[chid]);
	         }
	         else
	         {
			 	  if ((ioctrl_ring_set[cch]&0x2) && (!(ioctrl_ring_set[cch]&0x1))) // CID prior 1st Ring
				  {
	              		/* App set Ring disable, so DSP no need to Ring atuomatically. */
			 			//PRINT_R("2: ioctrl_ring_set[%d] = %d\n", chid, ioctrl_ring_set[chid]);
				  }
				  else
				  {
					//TstVoipSlicRing stVoipSlicRing;
	              	//stVoipSlicRing.ch_id = chid;
	              	//stVoipSlicRing.ring_set = 1;
	              	p_snd ->fxs_ops ->FXS_Ring( p_snd, 1 );
	              	PRINT_MSG("Dsp Ring, ch=%d\n", cch);
				  }
	         }
	         gRingGenAfterCid[dch] = 0;
	         ioctrl_ring_set[cch]&=(~0x2);
	         //printk("s:ioctrl_ring_set[%d]=%d\n", chid, ioctrl_ring_set[chid]);
	     }
	 }
	
}
#endif

// --------------------------------------------------------
// Polling job - CPC 
// --------------------------------------------------------

typedef struct {
	int cpc_start;
	int cpc_stop;
	unsigned char pre_linefeed;
	timetick_t cpc_timeout;
	timetick_t cpc_timeout2;
} stSlicCpc;

static stSlicCpc slic_cpc[ CON_CH_NUM ] = { [0 ... CON_CH_NUM-1] = {0, 0, 0} };

void SLIC_CPC_Gen_con( const voip_con_t *p_con, unsigned int time_in_ms_of_cpc_signal )
{
	const uint32 cch = p_con ->cch;
	voip_snd_t * const p_snd = p_con ->snd_ptr;
	
	if (slic_cpc[ cch ].cpc_start != 0)
	{
		PRINT_R("SLIC CPC gen not stop, ch=%d\n", cch);
		return;
	}
	
	slic_cpc[ cch ].pre_linefeed = p_snd ->fxs_ops ->SLIC_CPC_Gen( p_snd );

	//slic_cpc[cch].cpc_timeout = jiffies + (HZ*time_in_ms_of_cpc_signal/1000);
	slic_cpc[cch].cpc_timeout = timetick + time_in_ms_of_cpc_signal;
	slic_cpc[cch].cpc_start = 1;
	slic_cpc[cch].cpc_stop = 0;
	HookPollFlag[ cch ] = 0; // disable hook pooling
}

static void ENTRY_SLIC_CPC_Polling( 
						const voip_con_t *p_con, 
						voip_snd_t * const p_snd )
{
	// Check CPC signal period, and stop it
	const uint32 cch = p_con ->cch;
	
	if( slic_cpc[ cch ].cpc_start == 0 )
		return;

	if ((slic_cpc[cch].cpc_stop == 0) && (timetick_after(timetick, slic_cpc[cch].cpc_timeout)))
	{
		p_snd ->fxs_ops ->SLIC_CPC_Check( p_snd, slic_cpc[ cch ].pre_linefeed );

		//slic_cpc[cch].cpc_timeout2 = jiffies + (HZ*200/1000);
		slic_cpc[cch].cpc_timeout2 = timetick + 200;
		slic_cpc[cch].cpc_stop = 1;
	}
	
	if ((slic_cpc[cch].cpc_stop == 1) && (timetick_after(timetick, slic_cpc[cch].cpc_timeout2)))
	{
		slic_cpc[cch].cpc_start = 0;
		HookPollFlag[cch] = 1; // enable hook pooling
	}
}

// --------------------------------------------------------
// Polling job - Pluse dial  
// --------------------------------------------------------

#ifdef PULSE_DIAL_DET
#define SUPPORT_20PPS_DET
/* PULSE_DIAL_PAUSE_TIME: the minimum dead time between adjacent dial pulse trains 
 * ex. If the pause duration of two pulse trains(digit 3 and digit 4) is smaller than PULSE_DIAL_PAUSE_TIME,
 * then the detection result will be digit 7 (3+4).
 */
//#define PULSE_DIAL_PAUSE_TIME 45
static unsigned int pulse_dial_pause_time[CON_CH_NUM] = {[0 ... CON_CH_NUM-1] = 45};
unsigned long connect_cnt[CON_CH_NUM];
unsigned long disconnect_cnt[CON_CH_NUM];
unsigned long pulse_cnt[CON_CH_NUM];
static unsigned int pulse_det_flag[CON_CH_NUM] = {0};
#ifdef SUPPORT_20PPS_DET
static unsigned int break_min_ths[CON_CH_NUM] = {[0 ... CON_CH_NUM-1] = 1};// threshold break_ths*10 msec
#else
static unsigned int break_min_ths[CON_CH_NUM] = {[0 ... CON_CH_NUM-1] = 2};// threshold break_ths*10 msec
#endif
static unsigned int break_max_ths[CON_CH_NUM] = {[0 ... CON_CH_NUM-1] = 7};// threshold break_ths*10 msec

void set_pulse_det(unsigned int cch, unsigned int enable, stPulseDetParam* param)
{
	pulse_det_flag[cch] = enable;
#ifdef SUPPORT_20PPS_DET
	pulse_dial_pause_time[cch] = ((param->pause_time)/10)/2;
#else
	pulse_dial_pause_time[cch] = (param->pause_time)/10;
#endif
	break_min_ths[cch] = (param->break_min_ths)/10;
	break_max_ths[cch] = (param->break_max_ths)/10;
	//PRINT_Y("pulse_det_flag[%d]=%d\n", chid, pulse_det_flag[chid]);
}

unsigned int get_pulse_det(unsigned int cch)
{
	return pulse_det_flag[cch];
}

static inline void Pluse_Dial_Det_Handle_Hook( 
	const voip_con_t *p_con, const voip_snd_t * const p_snd, 
	int status )
{
	const uint32 cch = p_con ->cch;
	
	if ( (pulse_det_flag[ cch ] == 1) && (!Is_DAA_snd( p_snd )) )
		;	// not DAA 
	else
		return;
		
	if( status ) {
		if (disconnect_cnt[ cch ] != 0)
		{
			//if (cch == 0)
				//PRINT_Y("%d ", disconnect_cnt[cch]); // print the break(on-hook) duration of a pulse
			if ((disconnect_cnt[ cch ] >= break_min_ths[ cch ]) && (disconnect_cnt[ cch ] <= break_max_ths[ cch ]))
				pulse_cnt[ cch ]++;
			disconnect_cnt[ cch ]=0;
		}
		else
		{
			if (connect_cnt[cch] > pulse_dial_pause_time[cch])
			{
				if (pulse_cnt[cch] != 0)
				{
					if (pulse_cnt[cch] == 10)
						pulse_cnt[cch] = 0;
					voip_event_dtmf_in( cch, pulse_cnt[cch]+48/*char*/, 0, 0, 0 );
					PRINT_MSG("pulse dial |%ld|, ch= %d\n",pulse_cnt[cch], cch);					
				}
				pulse_cnt[cch] = 0;
			}
		}

		connect_cnt[cch]++;
	} else {
		disconnect_cnt[cch]++;
		connect_cnt[cch]=0;
	}
}
#endif

// --------------------------------------------------------
// Polling job - Audiocodes 
// --------------------------------------------------------

#ifdef AUDIOCODES_VOIP
TstCidDet cid_res[MAX_DSP_AC_CH_NUM];// = {0};

static inline void ENTRY_Audiocodes_Event_Polling( void )
{
	/* Polling DTMF, Fax, Modem Events */
	static TeventDetectionResult event_res;
	memset(&event_res, 0, sizeof(TeventDetectionResult));

	RtkAc49xApiEventPolling(&event_res);
	
	DECLARE_CON_FROM_AC_DCH( event_res.channel );
	voip_snd_t * p_snd = p_con ->snd_ptr;

	if (event_res.dtmf_digit != 'Z')
	{
		voip_event_dtmf_in( event_res.channel, event_res.dtmf_digit, 0, 0, 0 );
	}

	if (event_res.ced_flag == 1)
	{
		voip_event_fax_modem_in(event_res.channel, VEID_FAXMDM_AUDIOCODES_FAX );
	}
	else if (event_res.modem_flag == 1)
	{
		voip_event_fax_modem_in(event_res.channel, VEID_FAXMDM_AUDIOCODES_MODEM );
	}

	//if (Is_DAA_Channel(event_res.channel) == 1)
	if (Is_DAA_snd( p_snd ) == 1)
	{
		
#ifdef FXO_CALLER_ID_DET
		if (event_res.dtmf_cid_valid == 1)
		{
			cid_res[event_res.channel].dtmf_cid_valid = 1;
			cid_res[event_res.channel].cid_length = event_res.pCidMsg[1];
			int i=0;
			//printk("CID = ");
			for (i=0; i< cid_res[event_res.channel].cid_length; i++)
			{
				cid_res[event_res.channel].number[i] = event_res.pCidMsg[2+i];
				//printk("%c ", cid_res[event_res.channel].number[i]);
			}
			//printk("\n");
			cid_res[event_res.channel].number[i] = 0;
			cid_res[event_res.channel].cid_name[0] = 0;
			cid_res[event_res.channel].date[0] = 0;
			
		}
		else if (event_res.fsk_cid_valid == 1)
		{
			fsk_decode_complete_flag[event_res.channel] = 1;
			cid_res[event_res.channel].fsk_cid_valid = 1;
			cid_res[event_res.channel].cid_length = event_res.pCidMsg[1];		
			cid_res[event_res.channel].number_absence = event_res.num_absence;
			cid_res[event_res.channel].name_absence = event_res.name_absence;
			cid_res[event_res.channel].visual_indicator = event_res.visual_indicator;
#if 0
			printk("num_abs= %d, name_abs= %d, vmwi= %d\n", event_res.num_absence, event_res.name_absence, event_res.visual_indicator);
			printk("Number = ");
			for (i=0; i < (strlen(event_res.cid_num)); i++)
			{
				printk("%c ", event_res.cid_num[i]);
			}
			printk("\n");
	                        
			printk("Date and Time = ");
			for (i=0; i < (strlen(event_res.cid_date_time)); i++)
			{
				printk("%c ", event_res.cid_date_time[i]);
			}
			printk("\n");
	                        
			printk("Name = ");
			for (i=0; i < (strlen(event_res.cid_name)); i++)
			{
				printk("%c ", event_res.cid_name[i]);
			}
			printk("\n");
#endif

			if (cid_res[event_res.channel].number_absence == 1)
				cid_res[event_res.channel].number[0] = 0;
			else
				strcpy(cid_res[event_res.channel].number, event_res.cid_num);
			
			if (cid_res[event_res.channel].name_absence == 1)
				cid_res[event_res.channel].cid_name[0] = 0;
			else	
				strcpy(cid_res[event_res.channel].cid_name, event_res.cid_name);
			
			strcpy(cid_res[event_res.channel].date, event_res.cid_date_time);
			
#if 0
			printk("Number = ");
			for (i=0; i < (strlen(cid_res[event_res.channel].number)); i++)
			{
				printk("%c ", cid_res[event_res.channel].number[i]);
			}
			printk("\n");
	                        
			printk("Date and Time = ");
			for (i=0; i < (strlen(cid_res[event_res.channel].date)); i++)
			{
				printk("%c ", cid_res[event_res.channel].date[i]);
			}
			printk("\n");

			printk("Name = ");
			for (i=0; i < (strlen(cid_res[event_res.channel].cid_name)); i++)
			{
				printk("%c ", cid_res[event_res.channel].cid_name[i]);
			}
			printk("\n");
#endif	
		}
	
#endif //FXO_CALLER_ID_DET

#ifdef FXO_BUSY_TONE_DET
		if (event_res.IBS_CP == 9) // Busy tone detected
		{
			p_snd ->daa_ops ->DAA_On_Hook( p_snd );
			voip_event_hook_in( event_res.channel, VEID_HOOK_FXO_BUSY_TONE );
		}
		else if (event_res.IBS_CP == 10) // Ringback tone detected
		{
			voip_event_hook_in( event_res.channel, VEID_HOOK_FXO_RING_TONE_ON );
			// Need to know FXO_RING_TONE_OFF
		}
		else if (event_res.IBS_CP == 8) // Dial tone detected
		{
			// Do Nothing
		}
#endif
	}
}
#endif	//AUDIOCODES_VOIP




// --------------------------------------------------------
// Main Entry 
// --------------------------------------------------------



void START_Event_Polling_Use_Timer(unsigned long data)
{
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM
	extern void bus_pcm_checking_timer( void );
#endif

	int i;
	const voip_con_t * const p_con_start = ( const voip_con_t * )data;
	const voip_con_t *p_con;
	voip_snd_t *p_snd;
	
	// D:
	//   51.4226
	//   38.5984 (25.8795)	inline + no debug 
	//   GPIO: r: 384, w: 3792 (29.2176) 
	//   39.4918 (25.8730)  1.5.4 @500
	//   46.3503 (33.2207)  1.5.4 @620
	//   43.9116 (31.4886)  1.5.5 @620 
	// I:
	//   38.2830 (2.1745)	inline + no debug 
	//   39.4918 (2.9569)   1.5.4 @500
	//   46.3503 (2.9365)   1.5.4 @620
	//   43.9116 (2.6857)   1.5.5 @620
	for( i = 0, p_con = p_con_start; i < CON_CH_NUM; i ++, p_con ++ ) 
	{
		// all: 6.7681 (2.4816)
		// 6.0536 (2.7301)	inline 
		// 6.2892 (2.8033) 	inline + dmem (gpio/desc)
		// 5.2948 (2.9973)	inline + no debug 
		// 5.3125 (I 1.5314; D 2.9167) 1.5.4 @500
		// 6.1741 (I 1.8393; D 4.3532) 1.5.4 @620
		// 6.1358 (I 1.7777; D 3.5357) 1.5.4 @620
		const uint32 cch = p_con ->cch;
		
		if( ( p_snd = p_con ->snd_ptr ) == NULL )
			continue;
		
		// 4.1751
		if( HookPollFlag[ cch ] ) {
			// poll FXS/DAA/VDAA hook status 
			ENTRY_SLIC_Hook_Polling( p_con, p_snd );
		}
		
		switch( p_snd ->snd_type ) {
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
		case SND_TYPE_VDAA:
			ENTRY_VDAA_Hook_Polling( p_con, p_snd );
			break;
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_DAA_SUPPORT
		case SND_TYPE_DAA:
			// 2.0338
			ENTRY_DAA_Status_Polling( p_con, p_snd );
			break;
#endif
		case SND_TYPE_FXS:
#ifdef SW_DTMF_CID
			// FXS not running: .1165
			ENTRY_DTMF_CID_Hook_Polling( p_con, p_snd );
#endif

 			// FXS not running: .1104
			ENTRY_SLIC_CPC_Polling( p_con, p_snd );
			break;

		case SND_TYPE_DECT:
		case SND_TYPE_AC:
		default:
			break;
		}
	}
	
	
#ifdef AUDIOCODES_VOIP
	ENTRY_Audiocodes_Event_Polling();
#endif 


//#ifndef AUDIOCODES_VOIP

#ifndef AUDIOCODES_VOIP
#ifdef SW_DTMF_CID
	{
		extern void DTMF_CID_process( void );
		
		DTMF_CID_process();
	}
#endif

	// cost .8182 
	// For fsk CID type-II alert tone
	{
		extern void fsk_alert_procrss(void);
		extern void fsk_cid_process( void );
		
		fsk_alert_procrss();
		
		fsk_cid_process();
	}
#endif

#if defined( SUPPORT_RTCP ) && !defined( AUDIOCODES_VOIP )
	{
		extern void NTP_timetick (void);
		NTP_timetick();
	}
#endif

#if !defined( AUDIOCODES_VOIP )
	{
		extern void rtk_dsp_10ms_timer( void );
		
		rtk_dsp_10ms_timer();
	}
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM
	bus_pcm_checking_timer();
#endif
	
}

void HookPollingEnable(int cch)
{
	HookPollFlag[cch] = 1;
}

void HookPollingDisable(int cch)
{
	HookPollFlag[cch] = 0;
}

static void FXS_FXO_functionality_init( const voip_con_t *p_con )
{
	const unsigned char cch = p_con ->cch;
	
	/* Hook Pooling */
	Init_Hook_Polling(cch);
	HookPollingEnable(cch);
	
	/* Flash Detection */
	flash_hook_time[cch] = 30;
	flash_hook_min_time[cch] = 10;
	
	//if ( 1 == Is_DAA_Channel(cch))
	if( Is_DAA_snd( p_con ->snd_ptr ) )
	{
		/* For FXO Ring Detection */
		ring_on_time[cch] = 1;
		ring_off_time[cch] = 0;
		ring_on[cch] = 0;
		ring_off[cch] = 0;
		wait_ring_off[cch] = 0;
		wait_ring_off_timeout[cch] = 0;
		check_ring_off_time_out[cch] = 0;

		/* FXO Ring Detection */

		ring_det_cad_set(300, 300, 4500);	//unit: msec
		ring_times_set(cch, 1, 0);

#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
		stdaa_detect[cch].current_drop_reported = 0;
		stdaa_detect[cch].current_drop_cnt = 0;
		stdaa_detect[cch].current_Ndrop_cnt = 0;
		stdaa_detect[cch].off_hook_cnt = 0;
		stdaa_detect[cch].init_daa_polarity = 0;
		stdaa_detect[cch].stable_pre_daa_polarity_cnt = 0;
#endif
	}	
}

void FXS_FXO_functionality_init_all( const voip_con_t *p_con )
{
	int i;
	
	for( i = 0; i < CON_CH_NUM; i ++ )
		FXS_FXO_functionality_init( p_con ++ );
}

int voip_hook_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int ch;
	int n = 0;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	if( IS_CH_PROC_DATA( data ) )
	{
		ch = CH_FROM_PROC_DATA( data );
		n = sprintf( buf, "Channel=%d\n", ch );

		n += sprintf( buf + n, "Flash Hook Max.: %d ms\n", 10*flash_hook_time[ch]);
		n += sprintf( buf + n, "Flash Hook Min.: %d ms\n", 10*flash_hook_min_time[ch]);

	}
	
	*eof = 1;
	return n;
}

#ifdef PULSE_DIAL_DET
int voip_pulse_dial_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	int ch;
	int n = 0;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	if( IS_CH_PROC_DATA( data ) )
	{
		ch = CH_FROM_PROC_DATA( data );
		n = sprintf( buf, "Channel=%d\n", ch );

		n += sprintf( buf + n, "Pulse Dial Det Pause Time: %d ms\n", 10*pulse_dial_pause_time[ch]);
		n += sprintf( buf + n, "Pulse Dial Det Max. Break Time: %d ms\n", 10*break_max_ths[ch]);
		n += sprintf( buf + n, "Pulse Dial Det Min. Break Time: %d ms\n", 10*break_min_ths[ch]);

	}
	
	*eof = 1;
	return n;
}
#endif

int __init voip_hook_proc_init( void )
{
	create_voip_channel_proc_read_entry( "hook", voip_hook_read_proc );
#ifdef PULSE_DIAL_DET
	create_voip_channel_proc_read_entry( "pulse_dial", voip_pulse_dial_read_proc );
#endif
	return 0;
}

void __exit voip_hook_proc_exit( void )
{
	remove_voip_channel_proc_entry( "hook" );
#ifdef PULSE_DIAL_DET
	remove_voip_channel_proc_entry( "pulse_dial" );
#endif
}

voip_initcall_proc( voip_hook_proc_init );
voip_exitcall( voip_hook_proc_exit );


