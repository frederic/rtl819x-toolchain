#ifndef _SPI_8676_H_
#define _SPI_8676_H_

#include "gpio/gpio.h"

/* SPI Interface Set number for SLIC, DAA Device */
/* Programer must modify these setting for a new EV Board */
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
#elif defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK)
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
#endif //!CONFIG_RTK_VOIP_MULTIPLE_SI32178

//#define spi_rw_delay() spi_nsdelay(200) // 200 ns
// th: For 8672, if no rw delay for Zarlink V890 series, SLIC init fail.

#ifdef CONFIG_RTK_VOIP_8676_SPI_GPIO

//struct rtl_spi_dev_s;	// defined in spi.h 

extern void cyg_thread_delay(int delay);
//extern int32 __rtl867x_spi_init( struct rtl_spi_dev_s* pDev, uint32 gpioSCLK, uint32 gpioCS_, uint32 gpioSDO, uint32 gpioSDI, int32 gpioINT);
#ifdef CONFIG_RTK_VOIP_8676_SPI_GPIO
#define _rtl867x_spi_init( dev, clk, cs, do, di )	__rtl867x_spi_init( dev, clk, cs, do, di)
#elif defined (CONFIG_RTK_VOIP_8676_SHARED_SPI)
#define _rtl867x_spi_init( dev, clk, cs, do, di )	__rtl867x_spi_init(0)
#endif

#if defined (CONFIG_RTK_VOIP_SLIC_SI32178) || defined (CONFIG_RTK_VOIP_SLIC_SI32176_SI32178)
#define speed_booting_rating 0	// Si32178 need more time when reset
#else
#define speed_booting_rating 1
#endif



#endif // CONFIG_RTK_VOIP_8676_SPI_GPIO

#ifdef CONFIG_RTK_VOIP_8676_SHARED_SPI

/* SPI Flash Controller */


/*
 *  8676 dependet register definitions and macros **
 */
#define RTL8676_SPI_BASE 			0xB8009000

/*
 * Macro Definition
 */
 /*-- SPICNR --*/
#define SPI_CSTIME(i)		((i) << 26)   /* 0: 8-bit times , 1: 16-bit times of CS raise time */
#define SPI_WRIE(i)			((i) << 25)   /* 0: Disable , 1: Enable write ok interrupt */
#define SPI_RDIE(i)			((i) << 24)   /* 0: Disable , 1: Enable read ok interrupt */
#define SPI_LSB(i)			((i) << 7)   /* 0: MSB, 1: LSB*/
#define SPI_CMD(i)			((i) << 5)   /* 0: read, 1: write*/
#define SPI_START(i)		((i) << 4)   /* 1: Start transfer*/
#define SPI_SCLK_TYPE(i)	((i) << 3)   /*SCLK TYPE 0: type I SCLK, 1: type II SCLK*/
#define SPI_CSP(i)			((i) << 1)   /* 0: Low active, 1: High active, */ 

 /*-- SPISTR --*/
#define SPI_RDIP(i)			((i) << 31)   /* Read ok interrupt pending bit, 1 to clear  */
#define SPI_WDIP(i)			((i) << 30)   /* Write ok interrupt pending bit, 1 to clear  */
/*-- SPICLKDIV --*/
#define SPI_DIV(i)			((i) << 24)   /* Clock divisor, 0: /2, 1: /4, 2: /6, ... , 255: /512*/ 
/*-- SPITCR --*/
#define SPI_SEL_CS(i)			(1 << (31-(i))) /*select SPI chip 0~5*/
#define SPI_CTL_EN(i)			((i) << 23)   /* CTL_EN*/
#define SPI_ADD_EN(i)			((i) << 22)	  /* ADD_EN*/	
#define SPI_D0_EN(i)			((i) << 21)   /* D0_EN*/ 
#define SPI_D1_EN(i)			((i) << 20)   /* D1_EN*/ 

//#define SPI_CSFL(i)			((i) << 16)   /* CS*/ 


typedef struct rtl_8676_reg
{
	volatile unsigned int SPICNR;
	volatile unsigned int SPISTR;
	volatile unsigned int SPICKDIV;
	volatile unsigned int SPIRDR;
	//unsigned int SPITDR;
	volatile unsigned int SPITCR; //transaction config. reg.
	volatile unsigned int SPICDTCR0; //cs deselect time counter reg. 0
	volatile unsigned int SPICDTCR1; //cs deselect time counter reg. 1
	volatile unsigned int SPITCALR; //SPI Timing Calibration Register
} rtl_8676_reg_t ;


#define get_spi_reg() (rtl_8676_reg_t *)(RTL8676_SPI_BASE)

#define readl(x)		(*(volatile uint32_t *)x)
#define writel(val, addr)	  (void)((*(volatile unsigned int *) (addr)) = (val))


#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_SILAB
unsigned char read_spi_hw(unsigned int chid, unsigned int reg);
void write_spi_hw(unsigned int chid, unsigned int reg, unsigned int data);
unsigned char read_spi_hw_daa(int chid, unsigned int reg);
void write_spi_hw_daa(int chid, unsigned int reg, unsigned int data);
unsigned char read_spi_nodaisy_hw(unsigned int reg);
void write_spi_nodaisy_hw(unsigned int reg, unsigned int data);
#endif

#endif // CONFIG_RTK_VOIP_8676_SHARED_SPI

#endif // _SPI_8676_H_


