#include "rtk_voip.h"
#include "voip_init.h"
//#include "Slic_api.h"
#include "gpio/gpio.h"

#if defined CONFIG_RTK_VOIP_DRIVERS_PCM8186
#if 1 /* 0: relay for 8186V of Z version  1:relay for 8186V of v210 */
#define _V210_RELAY_R_ 
#else
#define _RELAY_8186V_Z_
#endif
#endif

static void SLIC_Relay_set(unsigned char chid, unsigned char on)
{
#if defined(CONFIG_RTK_VOIP_GPIO_8954C_V100) || defined(CONFIG_RTK_VOIP_GPIO_8954C_V200)
	if( chid == 0 )
		RTK_GPIO_SET( PIN_RELAY0, on );
	else if( chid == 1 )
		RTK_GPIO_SET( PIN_RELAY1, on );
#elif defined (CONFIG_RTK_VOIP_GPIO_8672_VQD01)
	RTK_GPIO_SET( PIN_RELAY, on );
#endif
	printk("Switch Relay to SLIC.\n");
}

static void SLIC_Relay_init( void )
{
#if defined(CONFIG_RTK_VOIP_GPIO_8954C_V100) || defined(CONFIG_RTK_VOIP_GPIO_8954C_V200)
	RTK_GPIO_INIT( PIN_RELAY0, GPIO_CONT_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	RTK_GPIO_INIT( PIN_RELAY1, GPIO_CONT_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	
	SLIC_Relay_set( 0, 1 );
	SLIC_Relay_set( 1, 1 );
#elif defined (CONFIG_RTK_VOIP_GPIO_8672_VQD01)
	// this board is two FXS port, two relay control by one GPIO pin
	RTK_GPIO_INIT( PIN_RELAY, GPIO_CONT_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	SLIC_Relay_set( 0, 1 );
#endif
}

static int __init voip_snd_slic_relay_init( void )
{
#ifdef RELAY_SW_CTRL_GPIOE	//for gpioE used.pull relay high.	
	#define GPEF_DIR  *((volatile unsigned int *)0xbd010144)
	#define GPEF_DATA  *((volatile unsigned int *)0xbd010140)	
	BOOT_MSG("GPEF_DIR = %x\n",GPEF_DIR);
	GPEF_DIR = GPEF_DIR | 0x01; 
	BOOT_MSG("GPEF_DIR = %x\n",GPEF_DIR);
#ifdef _V210_RELAY_R_
	GPEF_DATA = GPEF_DATA | 0x01;
#endif
#ifdef _RELAY_8186V_Z_ 
	GPEF_DATA = GPEF_DATA & 0xfffffffe;
#endif

#endif

#ifndef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
#ifdef RELAY_SW_CTRL_GPIOD //for gpioD used.pull relay high.	
	#define GPCD_DIR  *((volatile unsigned int *)0xbd010134)
	#define GPCD_DATA  *((volatile unsigned int *)0xbd010130)	
	BOOT_MSG("GPCD_DIR = %x\n",GPCD_DIR);
	GPCD_DIR = GPCD_DIR | 0x40; 		// GPIOD6
	BOOT_MSG("GPCD_DIR = %x\n",GPCD_DIR);
	GPCD_DATA = GPCD_DATA | 0x40;	
	
#ifdef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA_2_RELAY_SUPPORT
	#define GPAB_DIR *((volatile unsigned int *)0xbd010124)
	#define GPAB_DATA *((volatile unsigned int *)0xbd010120)
	BOOT_MSG("GPAB_DIR = %x\n",GPAB_DIR);
	GPAB_DIR = GPAB_DIR | 0x20; 		// GPIOA5
	BOOT_MSG("GPAB_DIR = %x\n",GPAB_DIR);
	GPAB_DATA = GPAB_DATA | 0x20;	
#endif
#endif
#endif

	SLIC_Relay_init();

	return 0;
}

voip_initcall_snd( voip_snd_slic_relay_init );

