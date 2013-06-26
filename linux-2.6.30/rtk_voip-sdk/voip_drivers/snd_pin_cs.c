#include "rtk_voip.h"
#include "voip_init.h"
#include "voip_debug.h"
#include "gpio/gpio.h"

#include "snd_pin_cs.h"

/*
 * We have many reasons to create this file. 
 * 1. Many snd_ drivers have their pin_cs[] array, and 
 *    this wastes memroy and maintain is difficult. 
 * 2. We use Kconfig to choose PIN_CS#, and this reason can support 
 *    reason 1. 
 * 3. All PIN_CS's have to be inactive, because system brings up 
 *    snd_ drivers incrementally. When I control 1st SLIC, 4st SLIC
 *    may also response if its PIN_CS is active. 
 */

const uint32 snd_pin_cs[] = {
	PIN_CS1, 
#ifdef PIN_CS2
	PIN_CS2, 
#ifdef PIN_CS4
	PIN_CS3, 
	PIN_CS4, 
#ifdef PIN_CS8
	PIN_CS5, 
	PIN_CS6, 
	PIN_CS7, 
	PIN_CS8, 
#endif
#endif
#endif
};

#define NUM_OF_SND_PIN_CS	( sizeof( snd_pin_cs ) / sizeof( snd_pin_cs[ 0 ] ) )

const uint32 snd_pin_cs_nr = NUM_OF_SND_PIN_CS;

// ------------------------------------------------------
// check whether it uses in range of snd_pin_cs[] 
// ------------------------------------------------------
#define CT_ASSERT_PIN_CS( pin_cs, slic_nr )			\
	CT_ASSERT( NUM_OF_SND_PIN_CS >= ( pin_cs - 1 ) + slic_nr )

//
// Note: The definition of _NR will be defined as zero if undefined. 
//

// silab 
#ifdef CONFIG_RTK_VOIP_SLIC_SI32178_NR		// chip select 
#if CONFIG_RTK_VOIP_SLIC_SI32178_NR > 0
CT_ASSERT_PIN_CS( CONFIG_RTK_VOIP_SLIC_SI32178_PIN_CS,	\
					CONFIG_RTK_VOIP_SLIC_SI32178_NR );
#endif
#endif
#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_NR		// daisy chain 
#if CONFIG_RTK_VOIP_SLIC_SI32176_NR > 0
CT_ASSERT_PIN_CS( CONFIG_RTK_VOIP_SLIC_SI32176_PIN_CS, 1 );
#endif
#endif
#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_CS_NR	// chip select
#if CONFIG_RTK_VOIP_SLIC_SI32176_CS_NR > 0
CT_ASSERT_PIN_CS( CONFIG_RTK_VOIP_SLIC_SI32176_CS_PIN_CS,
					CONFIG_RTK_VOIP_SLIC_SI32176_CS_NR );
#endif
#endif
#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR	// daisy chain 
#if CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR > 0
CT_ASSERT_PIN_CS( CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_PIN_CS, 1 );
#endif
#endif

// zarlink 
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88111_NR
#if CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88111_NR > 0
CT_ASSERT_PIN_CS( CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88111_PIN_CS,	\
					CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88111_NR );
#endif
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221_NR
#if CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221_NR > 0
CT_ASSERT_PIN_CS( CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221_PIN_CS,	\
					CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221_NR );
#endif
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116_NR
#if CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116_NR > 0
CT_ASSERT_PIN_CS( CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116_PIN_CS,	\
					CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116_NR );
#endif
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR
#if CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR > 0 
CT_ASSERT_PIN_CS( CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_PIN_CS,	\
					CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR );
#endif
#endif

// ------------------------------------------------------
// inactive all snd_pin_cs[] 
// ------------------------------------------------------

#define SET_USED_PIN_CS( __pin_cs, __nr )	\
	bit = 0x1 << ( __pin_cs - 1 );			\
											\
	for( i = 0; i < __nr; i ++ ) {			\
		pin_cs_used |= bit;					\
		bit <<= 1;							\
	}


void inactive_snd_pin_cs( void )
{
	uint32 pin_cs_used = 0;
	uint32 pin_cs;
	int i;
	uint32 bit;
	
	CT_ASSERT( ( sizeof( pin_cs_used ) * 4 ) >= NUM_OF_SND_PIN_CS );
	
// silab 
#ifdef CONFIG_RTK_VOIP_SLIC_SI32178_NR		// chip select 
#if CONFIG_RTK_VOIP_SLIC_SI32178_NR > 0
	SET_USED_PIN_CS( CONFIG_RTK_VOIP_SLIC_SI32178_PIN_CS,
					CONFIG_RTK_VOIP_SLIC_SI32178_NR );
#endif
#endif
#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_NR		// daisy chain 
#if CONFIG_RTK_VOIP_SLIC_SI32176_NR > 0
	SET_USED_PIN_CS( CONFIG_RTK_VOIP_SLIC_SI32176_PIN_CS, 1 );
#endif
#endif
#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_CS_NR		// chip select 
#if CONFIG_RTK_VOIP_SLIC_SI32176_CS_NR > 0
	SET_USED_PIN_CS( CONFIG_RTK_VOIP_SLIC_SI32176_CS_PIN_CS,
					CONFIG_RTK_VOIP_SLIC_SI32176_CS_NR );
#endif
#endif
#ifdef CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR	// daisy chain 
#if CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_NR > 0
	SET_USED_PIN_CS( CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_PIN_CS, 1 );
#endif
#endif

// zarlink 
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88111_NR
#if CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88111_NR > 0
	SET_USED_PIN_CS( CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88111_PIN_CS,
					CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88111_NR );
#endif
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221_NR
#if CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221_NR > 0
	SET_USED_PIN_CS( CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221_PIN_CS,
					CONFIG_RTK_VOIP_DRIVERS_SLIC_LE88221_NR );
#endif
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116_NR
#if CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116_NR > 0
	SET_USED_PIN_CS( CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116_PIN_CS,
					CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89116_NR );
#endif
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR
#if CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR > 0 
	SET_USED_PIN_CS( CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_PIN_CS,
					CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316_NR );
#endif
#endif
	
	// pull used PIN_CS to high 
	//pin_cs_used = 0xFFFFFFFFUL;		// inactive all potential PIN_CS 
	
	printk( "Inactive PIN_CS: " );
	
	for( i = 0, bit = 0x01; i < NUM_OF_SND_PIN_CS; i ++ ) {
		
		if( pin_cs_used & bit ) {
			
			pin_cs = snd_pin_cs[ i ];
			
			printk( "%08X ", pin_cs );
			
			_rtl_generic_initGpioPin( pin_cs, GPIO_CONT_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE );
			
			_rtl_generic_setGpioDataBit( pin_cs, 1 );
		}
		
		bit <<= 1;
	}
	
	printk( "\n" );
	
#if 1
	// init CLK/SDI/SDO 
	if( pin_cs_used ) {
		printk( "Initialize PIN_CLK=%08X PIN_DI=%08X PIN_DO=%08X\n", PIN_CLK, PIN_DI, PIN_DO );
		_rtl_generic_initGpioPin( PIN_CLK, GPIO_CONT_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE );
		_rtl_generic_initGpioPin( PIN_DI, GPIO_CONT_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE );
		_rtl_generic_initGpioPin( PIN_DO, GPIO_CONT_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE );
		_rtl_generic_setGpioDataBit( PIN_CLK, 1 );
	}
#endif
}

