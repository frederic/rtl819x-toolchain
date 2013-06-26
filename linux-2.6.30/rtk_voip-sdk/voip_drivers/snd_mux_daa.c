#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/delay.h>

#include "rtk_voip.h"
#include "con_register.h"
#include "snd_mux_daa.h"

int g_DAA_used[CON_CH_NUM] = {0};   /* DAA is 0: NOT used, 1: used by SLIC N(use channel n), 2: used and held. */

// retrieve con_ptr, only *mux* can use this!! 
extern voip_con_t *get_voip_con_ptr( uint32 cch );

#define DECLARE_BASIC_SND_DAA( x )		\
	const voip_con_t * const p_con = get_voip_con_ptr( cch );	\
	voip_snd_t * const p_snd = p_con ->snd_ptr;	\
												\
	if( p_snd ->snd_type != SND_TYPE_DAA ) {	\
		CHK_MSG( "snd_type != SND_TYPE_DAA\n" );	\
		return x;						\
	}

#define IMPLEMENT_TYPE0( func, rtype )	\
rtype func(int cch)						\
{										\
	DECLARE_BASIC_SND_DAA( 0 );			\
										\
	return p_snd ->daa_ops ->func( p_snd);	\
}

#define IMPLEMENT_TYPE0_void( func )	\
void func(int cch)						\
{										\
	DECLARE_BASIC_SND_DAA( ; );			\
										\
	p_snd ->daa_ops ->func( p_snd);	\
}

#define IMPLEMENT_TYPE1( func, rtype, v1type )	\
rtype func(int cch, v1type var1)						\
{										\
	DECLARE_BASIC_SND_DAA( 0 );			\
										\
	return p_snd ->daa_ops ->func( p_snd, var1 );	\
}

#define IMPLEMENT_TYPE1_void( func, v1type )	\
void func(int cch, v1type var1)	\
{										\
	DECLARE_BASIC_SND_DAA( ; );			\
										\
	p_snd ->daa_ops ->func( p_snd, var1 );	\
}

#define IMPLEMENT_TYPE2_void( func, v1type, v2type )	\
void func(int cch, v1type var1, v2type var2 )	\
{										\
	DECLARE_BASIC_SND_DAA( ; );			\
										\
	p_snd ->daa_ops ->func( p_snd, var1, var2 );	\
}

#define IMPLEMENT_TYPE3_void( func, v1type, v2type, v3type)	\
void func(int cch, v1type var1, v2type var2, v3type var3)	\
{										\
	DECLARE_BASIC_SND_DAA( ; );			\
										\
	p_snd ->daa_ops ->func( p_snd, var1, var2, var3 );	\
}

#if 0
int DAA_Init(int chid, int pcm_mode)
{
#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
	daa_init_all(chid, pcm_mode);
	return 0;
#elif defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK)
	// Zarlink Le89316 DAA init is include Le89316Init()
#endif
}
#endif

//void DAA_Set_Rx_Gain(int cch, unsigned char rx_gain)
IMPLEMENT_TYPE1_void( DAA_Set_Rx_Gain, unsigned char );

//void DAA_Set_Tx_Gain(int chid, unsigned char tx_gain)
IMPLEMENT_TYPE1_void( DAA_Set_Tx_Gain, unsigned char );

//int DAA_Check_Line_State(int chid)	/* 0: connect, 1: not connect, 2: busy*/
IMPLEMENT_TYPE0( DAA_Check_Line_State, int );

//void DAA_On_Hook(int chid)
IMPLEMENT_TYPE0_void( DAA_On_Hook );

//int DAA_Off_Hook(int chid)
IMPLEMENT_TYPE0( DAA_Off_Hook, int );

//unsigned char DAA_Hook_Status(int chid, int directly)
IMPLEMENT_TYPE1( DAA_Hook_Status, unsigned char, int );

// 0: has not changed states.
// 1: has transitioned from 0 to 1, or from 1 to 0, indicating the polarity of  TIP and RING is switched.
//unsigned char DAA_Polarity_Reversal_Det(int chid)
IMPLEMENT_TYPE0( DAA_Polarity_Reversal_Det, unsigned char );

// 0: has not changed states.
// 1: battery drop out
//unsigned char DAA_Bat_DropOut_Det(int chid)
IMPLEMENT_TYPE0( DAA_Bat_DropOut_Det, unsigned char );

/* For SI3050 register 5 */
/* bit0; 0= ON-HOOK, 1= OFF-HOOK */
/* bit2; 0= ringing off will delayed, 1= ringing occurring */
/* bit5; 0= no positive ring occuring, 1= positive ring occuring (realtime)*/
/* bit6; 0= no negative ring occuring, 1= negative ring occuring (realtime)*/
/* other bit don't care */
//int DAA_Ring_Detection(int chid)
IMPLEMENT_TYPE0( DAA_Ring_Detection, int );

//unsigned int DAA_Positive_Negative_Ring_Detect(int chid)
IMPLEMENT_TYPE0( DAA_Positive_Negative_Ring_Detect, unsigned int );

//unsigned int DAA_Get_Polarity(int chid)
IMPLEMENT_TYPE0( DAA_Get_Polarity, unsigned int );

//unsigned short DAA_Get_Line_Voltage(int chid)
IMPLEMENT_TYPE0( DAA_Get_Line_Voltage, unsigned short );

//void DAA_OnHook_Line_Monitor_Enable(int chid)
IMPLEMENT_TYPE0_void( DAA_OnHook_Line_Monitor_Enable );

#ifdef PULSE_DIAL_GEN // deinfe in rtk_voip.h
void DAA_Set_PulseDial_cch(unsigned int cch, unsigned int pulse_enable)
{
	extern void DAA_Set_Dial_Mode(unsigned int chid, unsigned int mode);
	
	DECLARE_BASIC_SND_DAA( ; );
	
	p_snd ->daa_ops ->DAA_Set_PulseDial( p_snd, pulse_enable );
	
	if( pulse_enable == 1 )
		DAA_Set_Dial_Mode(cch, 1);
	else if( pulse_enable == 0 )
		DAA_Set_Dial_Mode(cch, 0);
}
//IMPLEMENT_TYPE1_void( DAA_Set_PulseDial, unsigned int );
#endif

//void DAA_read_reg( uint32 cch, unsigned int num, unsigned char *val )
IMPLEMENT_TYPE3_void( DAA_read_reg, unsigned int, unsigned char *, unsigned char * );
//void DAA_write_reg( uint32 cch, unsigned char num, unsigned char val )
IMPLEMENT_TYPE3_void( DAA_write_reg, unsigned int, unsigned char *, unsigned char * );
//void DAA_read_ram( uint32 cch, unsigned short num, unsigned int *val )
IMPLEMENT_TYPE2_void( DAA_read_ram, unsigned short, unsigned int * );
//void DAA_write_ram( uint32 cch, unsigned short num, unsigned int val )
IMPLEMENT_TYPE2_void( DAA_write_ram, unsigned short, unsigned int );
//void DAA_dump_reg( uint32 cch )
IMPLEMENT_TYPE0_void( DAA_dump_reg );
//void DAA_dump_ram( uint32 cch )
IMPLEMENT_TYPE0_void( DAA_dump_ram );

