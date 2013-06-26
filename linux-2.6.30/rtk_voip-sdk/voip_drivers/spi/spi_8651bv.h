#ifndef _SPI_8651_H_
#define _SPI_8651_H_

#include "gpio/gpio.h"

/* SPI Interface Set number for SLIC, DAA Device */
/* Programer must modify these setting for a new EV Board */
#define SPI_DEV_NUM	4	// this means total SPI interface set number
#define SLIC0_SPI_DEV	0	// this means SLIC use SPI set 0
#define SLIC1_SPI_DEV	1	// this means SLIC use SPI set 1
#define DAA0_SPI_DEV	2
#define DAA1_SPI_DEV	3

#define spi_rw_delay()	udelay(10)

#if defined (CONFIG_RTK_VOIP_SLIC_SI32178) || defined (CONFIG_RTK_VOIP_SLIC_SI32176_SI32178)
#define speed_booting_rating 0	// Si32178 need more time when reset
#else
#define speed_booting_rating 1
#endif

extern void cyg_thread_delay(int delay);

#endif //_SPI_8651_H_
