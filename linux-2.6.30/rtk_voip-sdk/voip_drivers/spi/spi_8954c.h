#ifndef _SPI_8954C_H_
#define _SPI_8954C_H_

#include "gpio/gpio.h"

/* SPI Interface Set number for SLIC, DAA Device */
/* Programer must modify these setting for a new EV Board */
#ifdef CONFIG_RTK_VOIP_DECT_SPI_SUPPORT
#define SPI_DEV_NUM		1
#define DECT_SPI_DEV	0
#else
#ifdef CONFIG_RTK_VOIP_MULTIPLE_SI32178
#define SPI_DEV_NUM	4
#define SLIC0_SPI_DEV	0
#define SLIC1_SPI_DEV	1
#define SLIC2_SPI_DEV	2
#define SLIC3_SPI_DEV	3
#define DAA0_SPI_DEV	-1	// for pass compile
#define DAA1_SPI_DEV	-1	// for pass compile
#elif defined(CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_CS)
#define SPI_DEV_NUM	2	// this means total SPI interface set number
#define SLIC0_SPI_DEV	0	// this means SLIC use SPI set 0
#define SLIC1_SPI_DEV	1
#define DAA0_SPI_DEV	1	// this means DAA port 0 use SPI set 1
#define DAA1_SPI_DEV	-1
#else
#define SPI_DEV_NUM	2	// this means total SPI interface set number
#define SLIC0_SPI_DEV	0	// this means SLIC use SPI set 0
#define SLIC1_SPI_DEV	-1
#define DAA0_SPI_DEV	1	// this means DAA port 0 use SPI set 1
#define DAA1_SPI_DEV	-1
#endif	//!CONFIG_RTK_VOIP_MULTIPLE_SI32178
#endif	//!CONFIG_RTK_VOIP_DECT_SPI_SUPPORT

#define spi_rw_delay()	//udelay(10)

#if defined (CONFIG_RTK_VOIP_SLIC_SI32178) || defined (CONFIG_RTK_VOIP_SLIC_SI32176_SI32178)
#define speed_booting_rating 0	// Si32178 need more time when reset
#else
#define speed_booting_rating 1
#endif

struct rtl_spi_dev_s;	// defined in spi.h 

extern void cyg_thread_delay(int delay);
//extern void _rtl8954C_init_spi_channels(int size, struct rtl_spi_dev_s* pDev[], uint32 pin_cs[], uint32 pin_reset, uint32 pin_clk, uint32 pin_do, uint32 pin_di);
extern int32 __rtl8954C_spi_init( struct rtl_spi_dev_s* pDev, uint32 gpioSCLK, uint32 gpioCS_, uint32 gpioSDO, uint32 gpioSDI, int32 gpioINT);
#define _rtl8954C_spi_init( dev, clk, cs, do, di )	__rtl8954C_spi_init( dev, clk, cs, do, di, 0 )
#define _rtl8954C_spi_init_ex( dev, clk, cs, do, di, int )	__rtl8954C_spi_init( dev, clk, cs, do, di, int )


#endif //_SPI_8954C_H_
