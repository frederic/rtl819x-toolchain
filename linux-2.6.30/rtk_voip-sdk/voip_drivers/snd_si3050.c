#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/delay.h>

#include "rtk_voip.h"
#include "voip_types.h"
//#include "Daa_api.h"
#include "snd_proslic_daa.h"
#include "spi.h"
#include "con_register.h"
#include "snd_proslic_type.h"

#if 0	// chmap comment it 
static int DAA_Init(int chid, int pcm_mode)
{
	// call this, only si3050 
	daa_init_all(chid, pcm_mode);
	return 0;
}
#endif

static void DAA_Set_Rx_Gain_si3050(voip_snd_t *this, unsigned char rx_gain)
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	vdaaChanType * const daas = container ->daas;
	DAA_Rx_Gain_Web(daas, rx_gain);
}

static void DAA_Set_Tx_Gain_si3050(voip_snd_t *this, unsigned char tx_gain)
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	vdaaChanType * const daas = container ->daas;
	DAA_Tx_Gain_Web(daas, tx_gain);
}

static int DAA_Check_Line_State_si3050(voip_snd_t *this)	/* 0: connect, 1: not connect, 2: busy*/
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	vdaaChanType * const daas = container ->daas;
	return daa_line_check(daas);
}

static void DAA_On_Hook_si3050(voip_snd_t *this)
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	vdaaChanType * const daas = container ->daas;
	on_hook_daa(daas);
}

static int DAA_Off_Hook_si3050(voip_snd_t *this)
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	vdaaChanType * const daas = container ->daas;
	
	off_hook_daa(daas);
	return 1;
}

static unsigned char DAA_Hook_Status_si3050(voip_snd_t *this, int directly)
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	vdaaChanType * const daas = container ->daas;
	unsigned char state;
	
	if( !directly && container ->user.hookStatus != INVALID_HOOK_STATUS ) {
		return container ->user.hookStatus;
	}
	
	state = daa_hook_state(daas);
	
	container ->user.hookStatus = state;
	
	return state;
}


// 0: has not changed states.
// 1: has transitioned from 0 to 1, or from 1 to 0, indicating the polarity of  TIP and RING is switched.
static unsigned char DAA_Polarity_Reversal_Det_si3050(voip_snd_t *this)
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	vdaaChanType * const daas = container ->daas;
	return daa_polarity_reversal_det(daas, container ->daa_det);
}

// 0: has not changed states.
// 1: battery drop out
static unsigned char DAA_Bat_DropOut_Det_si3050(voip_snd_t *this)
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	vdaaChanType * const daas = container ->daas;
	return daa_drop_out_det(daas, container ->daa_det);
}


/* For SI3050 register 5 */
/* bit0; 0= ON-HOOK, 1= OFF-HOOK */
/* bit2; 0= ringing off will delayed, 1= ringing occurring */
/* bit5; 0= no positive ring occuring, 1= positive ring occuring (realtime)*/
/* bit6; 0= no negative ring occuring, 1= negative ring occuring (realtime)*/
/* other bit don't care */

static int DAA_Ring_Detection_si3050(voip_snd_t *this)
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	vdaaChanType * const daas = container ->daas;
	return ring_detection_DAA(daas);
}

static unsigned int DAA_Positive_Negative_Ring_Detect_si3050(voip_snd_t *this)
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	vdaaChanType * const daas = container ->daas;
	return (daa_line_stat(daas)&(RDTP|RDTN));
}

static unsigned int DAA_Get_Polarity_si3050(voip_snd_t *this)
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	vdaaChanType * const daas = container ->daas;
	ctrl_S * const ctrl = ( ctrl_S * )daas ->deviceId ->ctrlInterface ->hCtrl;
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const int chid = daas ->channel;
	return ((readDAAReg(pdev, chid, 29) & 0x80) >>7); // 1 or 0
}

static unsigned short DAA_Get_Line_Voltage_si3050(voip_snd_t *this)
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	vdaaChanType * const daas = container ->daas;
	return daa_get_line_voltage(daas);
}

static void DAA_OnHook_Line_Monitor_Enable_si3050(voip_snd_t *this)
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	vdaaChanType * const daas = container ->daas;
	caller_ID_pass(daas);
}

#ifdef PULSE_DIAL_GEN // deinfe in rtk_voip.h
static void DAA_Set_PulseDial_si3050(voip_snd_t *this, unsigned int pulse_enable)
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	vdaaChanType * const daas = container ->daas;
	ctrl_S * const ctrl = ( ctrl_S * )daas ->deviceId ->ctrlInterface ->hCtrl;
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const int chid = daas ->channel;
	int reg31;

	// Set the FOH bit (Fast Off-Hook)
	reg31 = 3;//0:512ms 1:128ms 2:64ms 3:8ms.
	reg31 = readDAAReg(pdev, chid, 31) | (reg31<<5);
	writeDAAReg(pdev, chid, 31,reg31);

	if (pulse_enable == 1)
	{
		// Set RCALD bit to 1 to disable internal resistor calibration
		writeDAAReg(pdev, chid, 25,(readDAAReg(pdev, chid, 25)|0x20));
		//DAA_Set_Dial_Mode(chid, 1);
	}
	else if (pulse_enable == 0)
	{
		// Set RCALD bit to 0 to enable internal resistor calibration
		writeDAAReg(pdev, chid, 25,(readDAAReg(pdev, chid, 25)&0xDF));
		//DAA_Set_Dial_Mode(chid, 0);
	}
}
#endif // PULSE_DIAL_GEN

static int enable_si3050( voip_snd_t *this, int enable )
{
	return 0;
}

static void DAA_read_reg_si3050(voip_snd_t *this, unsigned int num, unsigned char *len, unsigned char *val)
{
	extern unsigned char R_reg_dev(rtl_spi_dev_t *pdev, unsigned char chid, unsigned char regaddr);
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	ctrl_S * const ctrl = ( ctrl_S * )( 
			container ->daas ->deviceId ->ctrlInterface ->hCtrl );
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const unsigned char chid = container ->daas ->channel;

	*val = R_reg_dev(pdev, chid, (unsigned char)num);	
}

static void DAA_write_reg_si3050(voip_snd_t *this, unsigned int num, unsigned char *len, unsigned char *val)
{
	extern void W_reg_dev(rtl_spi_dev_t *pdev, unsigned char chid, unsigned char regaddr, unsigned char data);
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	ctrl_S * const ctrl = ( ctrl_S * )( 
			container ->daas ->deviceId ->ctrlInterface ->hCtrl );
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const unsigned char chid = container ->daas ->channel;
	
	W_reg_dev(pdev, chid, (unsigned char)num, *val);
}

static void DAA_read_ram_si3050(voip_snd_t *this, unsigned short num, unsigned int *val)
{
	extern unsigned int R_ram_dev(rtl_spi_dev_t* pdev, unsigned char chid, unsigned short reg);
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	ctrl_S * const ctrl = ( ctrl_S * )( 
			container ->daas ->deviceId ->ctrlInterface ->hCtrl );
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const unsigned char chid = container ->daas ->channel;
	
	*val = R_ram_dev(pdev, chid, num);
}


static void DAA_write_ram_si3050(voip_snd_t *this, unsigned short num, unsigned int val)
{
	extern void W_ram_dev(rtl_spi_dev_t *pdev, unsigned char chid, unsigned short reg, unsigned int data);
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	ctrl_S * const ctrl = ( ctrl_S * )( 
			container ->daas ->deviceId ->ctrlInterface ->hCtrl );
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const unsigned char chid = container ->daas ->channel;
			
	W_ram_dev(pdev, chid, num, val);
}

static void DAA_dump_reg_si3050(voip_snd_t *this)
{
	unsigned char reg_val;
	unsigned char reg_len;
	int i;

	reg_len = sizeof(reg_val);
	printk("Dump DAA register:\n");
	for (i=0; i <= 126; i++)
	{
		this ->daa_ops ->DAA_read_reg(this, i, &reg_len, &reg_val);
		printk("%d: 0x%x\n", i, reg_val);
	}
}

static void DAA_dump_ram_si3050(voip_snd_t *this)
{
	unsigned int ram_val;
	int i;

	printk("Dump DAA ram:\n");
	for (i=0; i <= 1023; i++)
	{
		this ->daa_ops ->DAA_read_ram(this, i, &ram_val);
		printk("%d: 0x%x\n", i, ram_val);
	}
}

// --------------------------------------------------------
// channel mapping architecture 
// --------------------------------------------------------

#define IMPLEMENT_SI3050_OPS_type0( func, rtype )		\
static rtype func##_si3050( voip_snd_t *this )			\
{														\
	return func( this ->sch );							\
}

#define IMPLEMENT_SI3050_OPS_type0_void( func )			\
static void func##_si3050( voip_snd_t *this )			\
{														\
	func( this ->sch );									\
}

#define IMPLEMENT_SI3050_OPS_type1_void( func, v1type )	\
static void func##_si3050( voip_snd_t *this, v1type v1 )	\
{														\
	func( this ->sch, v1 );								\
}

//IMPLEMENT_SI3050_OPS_type1_void( DAA_Set_Rx_Gain, unsigned char );
//IMPLEMENT_SI3050_OPS_type1_void( DAA_Set_Tx_Gain, unsigned char );
//IMPLEMENT_SI3050_OPS_type0( DAA_Check_Line_State, int );
//IMPLEMENT_SI3050_OPS_type0_void( DAA_On_Hook );
//IMPLEMENT_SI3050_OPS_type0( DAA_Off_Hook, int );
//IMPLEMENT_SI3050_OPS_type0( DAA_Hook_Status, unsigned char );
//IMPLEMENT_SI3050_OPS_type0( DAA_Polarity_Reversal_Det, unsigned char );
//IMPLEMENT_SI3050_OPS_type0( DAA_Bat_DropOut_Det, unsigned char );
//IMPLEMENT_SI3050_OPS_type0( DAA_Ring_Detection, int );
//IMPLEMENT_SI3050_OPS_type0( DAA_Positive_Negative_Ring_Detect, unsigned int );
//IMPLEMENT_SI3050_OPS_type0( DAA_Get_Polarity, unsigned int );
//IMPLEMENT_SI3050_OPS_type0( DAA_Get_Line_Voltage, unsigned short );
//IMPLEMENT_SI3050_OPS_type0_void( DAA_OnHook_Line_Monitor_Enable );
//IMPLEMENT_SI3050_OPS_type1_void( DAA_Set_PulseDial, unsigned int );

//__attribute__ ((section(".snd_desc_data")))
const snd_ops_daa_t snd_si3050_daa_ops = {
	// common operation 
	.enable = enable_si3050,
	
	// for each snd_type 
	.DAA_Set_Rx_Gain = DAA_Set_Rx_Gain_si3050,
	.DAA_Set_Tx_Gain = DAA_Set_Tx_Gain_si3050,
	.DAA_Check_Line_State = DAA_Check_Line_State_si3050,
	.DAA_On_Hook = DAA_On_Hook_si3050,
	.DAA_Off_Hook = DAA_Off_Hook_si3050,
	.DAA_Hook_Status = DAA_Hook_Status_si3050,
	.DAA_Polarity_Reversal_Det = DAA_Polarity_Reversal_Det_si3050,
	.DAA_Bat_DropOut_Det = DAA_Bat_DropOut_Det_si3050,
	.DAA_Ring_Detection = DAA_Ring_Detection_si3050,
	.DAA_Positive_Negative_Ring_Detect = DAA_Positive_Negative_Ring_Detect_si3050,
	.DAA_Get_Polarity = DAA_Get_Polarity_si3050,
	.DAA_Get_Line_Voltage = DAA_Get_Line_Voltage_si3050,
	.DAA_OnHook_Line_Monitor_Enable = DAA_OnHook_Line_Monitor_Enable_si3050,
	.DAA_Set_PulseDial = DAA_Set_PulseDial_si3050,
	
	// read/write register/ram
	.DAA_read_reg = DAA_read_reg_si3050,
	.DAA_write_reg = DAA_write_reg_si3050,
	.DAA_read_ram = DAA_read_ram_si3050,
	.DAA_write_ram = DAA_write_ram_si3050,
	.DAA_dump_reg = DAA_dump_reg_si3050,
	.DAA_dump_ram = DAA_dump_ram_si3050,
};

// --------------------------------------------------------
// init si3050 (only real si3050)
// --------------------------------------------------------

#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
#ifndef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x
#include "voip_init.h"

static voip_snd_t snd_si3050;

static int __init voip_snd_si3050_init( void )
{
	snd_proslic[ i ].sch = 0;
	snd_proslic[ i ].name = "si3050";
	snd_proslic[ i ].snd_type = SND_TYPE_DAA;
	snd_proslic[ i ].bus_type_sup = BUS_TYPE_PCM;
	snd_proslic[ i ].TS1 = 0;
	snd_proslic[ i ].TS2 = 0;
	snd_proslic[ i ].band_mode_sup = BAND_MODE_8K;
	snd_proslic[ i ].snd_ops = ( const snd_ops_t * )&snd_si3050_daa_ops;
	
	register_voip_snd( &snd_si3050, 1 );
}

voip_initcall_snd( voip_snd_si3050_init );
#endif
#endif
#endif

