#include <linux/interrupt.h>
#include "rtk_voip.h"
#include "snd_virtual_daa.h"

char relay_2_PSTN_flag[2]={0};     /* 1: relay to PSTN , 0: relay to SLIC */

// for virtual DAA.GPIOD_14 for on_hook or off_hook detect, GPIOD_13 for ring incoming detect.
// GPIOD_12 for hook det of second relay

// set the virtual-daa-used gpio pin direction to i/p.
static void virtual_daa_init()
{
	
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA_2_RELAY_SUPPORT
	GPCD_DIR = GPCD_DIR & 0xFFFF8FFF;	
#else
	GPCD_DIR = GPCD_DIR & 0xFFFF9FFF;
#endif
	//printk("Virtual DAA Init.\n");
}

//1: off-hook, 0: on-hook
unsigned char virtual_daa_hook_detect(unsigned char relay_id)
{
	if (relay_id == 0)
	{
		if (GPCD_DATA&0x40000000) //GPIOD_14
		{
			return ON_HOOK;
		}
		else
		{
			return OFF_HOOK;
		}
	}
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA_2_RELAY_SUPPORT
	else if (relay_id == 1)
	{
		if (GPCD_DATA&0x10000000) //GPIOD_12
		{
			return ON_HOOK;
		}
		else
		{
			return OFF_HOOK;
		}
	}
#endif
}
	 
//1: ring incoming, 0: no ring incoming
unsigned char virtual_daa_ring_incoming_detect() //GPIO_D bit13: i/p
{	
	int i;
	
	if (!(GPCD_DATA&0x20000000)) 
	{
		return RING_INCOMING;				
	
	}

	return NO_RING;	
}
	

#define CIR_SIZE	100
#define DAA_RING_THS	20
char cir_res[CIR_SIZE] = {0};
int cir_idx = 0;
int summation = 0;

unsigned char virtual_daa_ring_det(void)
{
	summation -= cir_res[cir_idx];
	
	cir_res[cir_idx] = virtual_daa_ring_incoming_detect();
	
	summation += cir_res[cir_idx];
	cir_idx++;
	
	if (cir_idx >= CIR_SIZE)
		cir_idx = 0;
#if 0
	if (summation != 0)	
		printk("%d ", summation);
#endif
	if (summation < 0)
		printk("error: virtual_daa_ring_det() \n");
	
	/* TH: When DAA incomimg ring, summation can reach around 28 */
	if (summation > DAA_RING_THS)
	{
		return RING_INCOMING;
	}
	else
	{
		return NO_RING;
	}
}

//1: pull high, 0: pull low
// relay_id should match to ch_id. i.e. each pcm channel can relay to PSTN.
char virtual_daa_relay_switch(unsigned char relay_id, unsigned char state)	//realy_id=0: GPIO_D bit6: o/p
{
	unsigned long flags;
	save_flags(flags); cli();
	if (state == RELAY_PSTN)	/* pull low */
	{	
		if (relay_id == 0)
		{
			if (relay_2_PSTN_flag[0] == 0)
			{
				GPCD_DATA = (GPCD_DATA>>16) & 0xffbf;
				printk("--- Switch relay %d to PSTN ---\n", relay_id);
				
				/* pkshih: Assumption is off-hook while switching relay. */ 
				if( virtual_daa_hook_detect( relay_id ) == ON_HOOK ) {
					GPCD_DATA = (GPCD_DATA>>16) | 0x40;
					printk("--- Switch relay %d back to SLIC ---\n", relay_id);
					return RELAY_FAIL;
				}
				
				return RELAY_SUCCESS;
			}
			else
				printk("--- Relay %d has been switch to PSTN ---\n", relay_id);
			
			
		}
		else if (relay_id == 1)
		{
		
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA_2_RELAY_SUPPORT

			if (relay_2_PSTN_flag[1] == 0)
			{
				GPAB_DATA = GPAB_DATA & 0xffdf;
				printk("--- Switch relay %d to PSTN ---\n", relay_id);

				/* pkshih: Assumption is off-hook while switching relay. */ 
				if( virtual_daa_hook_detect( relay_id ) == ON_HOOK ) {
					GPAB_DATA = GPAB_DATA | 0x20;
					printk("--- Switch relay %d back to SLIC ---\n", relay_id);
					return RELAY_FAIL;
				}
				
				return RELAY_SUCCESS;
			}
			else
				printk("--- Relay %d has been switch to PSTN ---\n", relay_id);
#else
			printk("** ERROR **: no relay for pcm channel %d.\n", relay_id);
			
			return RELAY_FAIL;
#endif
		}
		else
		{
			printk("** ERROR **: no relay for pcm channel %d.\n", relay_id);
			
			return RELAY_FAIL;
		}
	}
	else if (state == RELAY_SLIC)	/* pull high */
	{
		if (relay_id == 0)
		{
			if (relay_2_PSTN_flag[0] == 1)
			{
				GPCD_DATA = (GPCD_DATA>>16) | 0x40;
				printk("--- Switch relay %d to SLIC ---\n", relay_id);
				
				return RELAY_SUCCESS;
			}
			else
				printk("--- Relay %d has been switch to SLIC ---\n", relay_id);
			
		}
		else if (relay_id == 1)
		{
		
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA_2_RELAY_SUPPORT

			if (relay_2_PSTN_flag[1] == 1)
			{
				GPAB_DATA = GPAB_DATA | 0x20;
				printk("--- Switch relay %d to SLIC ---\n", relay_id);
				
				return RELAY_SUCCESS;
			}
			else
				printk("--- Relay %d has been switch to SLIC ---\n", relay_id);
#else
			printk("** ERROR **: no relay for pcm channel %d.\n", relay_id);
			
			return RELAY_FAIL;
#endif
		}
		else
		{
			printk("** ERROR **: no relay for pcm channel %d.\n", relay_id);
			
			return RELAY_FAIL;
		}
	}
	else
	{ 
		printk("virtual_daa_relay_switch: no such state\n");	
		
		return RELAY_FAIL;
	}
	
	restore_flags(flags);
	
	return RELAY_SUCCESS;
}	

// --------------------------------------------------------
// channel mapping architecture 
// --------------------------------------------------------

#include "voip_init.h"
#include "con_register.h"

#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA_2_RELAY_SUPPORT
#define VIRTUAL_DAA_NUM		2
#else
#define VIRTUAL_DAA_NUM		1
#endif

static unsigned char virtual_daa_hook_detect_op( struct voip_snd_s *this )
{
	return virtual_daa_hook_detect( this ->sch );
}

static unsigned char virtual_daa_ring_det_op( struct voip_snd_s *this )
{
	// single virtual DAA only 
	return virtual_daa_ring_det();
}

static char virtual_daa_relay_switch_op( struct voip_snd_s *this, unsigned char state )
{
	return virtual_daa_relay_switch( this ->sch, state );
}

unsigned char virtual_daa_ring_incoming_detect_op( struct voip_snd_s *this )
{
	// single virtual DAA only 
	return virtual_daa_ring_incoming_detect();
}

static int enable_virtual_daa( voip_snd_t *this, int enable )
{
	return 0;
}

static voip_snd_t snd_virtual_daa[ VIRTUAL_DAA_NUM ];
static snd_ops_vdaa_t snd_virtual_daa_ops = {
	// common operation 
	.enable = enable_virtual_daa,
	
	// for each snd_type 
	.virtual_daa_hook_detect				= virtual_daa_hook_detect_op,
	.virtual_daa_ring_det					= virtual_daa_ring_det_op,
	.virtual_daa_relay_switch				= virtual_daa_relay_switch_op,
	.virtual_daa_ring_incoming_detect		= virtual_daa_ring_incoming_detect_op,
};

static int __init voip_snd_virtual_daa_init( void )
{
	int i;
	
	// init virtual daa 
	virtual_daa_init();
	
	// register virtual daa 
	for( i = 0; i < VIRTUAL_DAA_NUM; i ++ ) {
		snd_virtual_daa[ i ].sch = i;
		snd_virtual_daa[ i ].name = "v_daa";
		snd_virtual_daa[ i ].snd_type = SND_TYPE_VDAA;
		snd_virtual_daa[ i ].bus_type_sup = BUS_TYPE_RELAY;
		snd_virtual_daa[ i ].TS1 = 0;
		snd_virtual_daa[ i ].TS2 = 0;
		snd_virtual_daa[ i ].band_mode_sup = 0;
		snd_virtual_daa[ i ].snd_ops = ( const snd_ops_t * )&snd_virtual_daa_ops;
	}
	
	register_voip_snd( &snd_virtual_daa[ 0 ], VIRTUAL_DAA_NUM );
	
	return 0;
}

voip_initcall_snd( voip_snd_virtual_daa_init );

