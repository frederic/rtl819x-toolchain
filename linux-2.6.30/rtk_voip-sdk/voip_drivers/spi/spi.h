/*
* Copyright c                  Realtek Semiconductor Corporation, 2006  
* All rights reserved.
*/

#ifndef _SPI_H_
#define _SPI_H_

#include "voip_types.h"
#include "gpio/gpio.h"

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
#include "spi_8186.h"
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651)
#include "spi_8651bv.h"
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8671)
#include "spi_8972.h"
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8672)
#include "spi_8672.h"
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8676)
#include "spi_8676.h"
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM865xC)
#include "spi_8952.h"
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
#include "spi_8972b.h"
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
#include "spi_8954c.h"
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD)
#include "spi_8972d.h"
#else
#error "unknown spi driver"
#endif


struct rtl_spi_dev_s
{
	uint32 gpioSCLK;
	uint32 gpioCS_;
	uint32 gpioSDI;
	uint32 gpioSDO;
#ifdef CONFIG_RTK_VOIP_DECT_SPI_SUPPORT
	uint32 gpioINT;
#endif
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK) && defined(CONFIG_RTK_VOIP_8676_SHARED_SPI)
	uint32 SPI_SEL_CS;
#endif

	//uint32 SClkDelayLoop;
};

typedef struct rtl_spi_dev_s rtl_spi_dev_t;

//--chiminer 2006-5-11----------
//The global variable is used to access the data of registers of Le88221.
//After calling read function, this global variable is valid.
//After writting data into this global variable, call the write function. 
typedef struct Le88xxx_register {
	unsigned char byte1;
	unsigned char byte2;
	unsigned char byte3;
	unsigned char byte4;
	unsigned char byte5;
	unsigned char byte6;
	unsigned char byte7;
	unsigned char byte8;
	unsigned char byte9;
	unsigned char byte10;
	unsigned char byte11;
	unsigned char byte12;
	unsigned char byte13;
	unsigned char byte14;	
} Le88xxx;

/*********************  Function Prototype in spi.c  ***********************/
void cyg_thread_delay(int delay);
unsigned char readDirectReg(unsigned int chid, unsigned char address);
void writeDirectReg(unsigned int chid, unsigned char address, unsigned char data);
void writeIndirectReg(unsigned int chid, unsigned char address, unsigned short data);
void init_spi(int ch_spi);
unsigned char read_spi_sw(unsigned int chid, unsigned int reg);
void write_spi_sw(unsigned int chid, unsigned int reg, unsigned int data);
unsigned char read_spi_nodaisy_sw(unsigned int reg);
void write_spi_nodaisy_sw(unsigned int reg, unsigned int data);
unsigned short readIndirectReg(unsigned int chid, unsigned char address);
#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
unsigned char read_spi_sw_daa(rtl_spi_dev_t* pdev, int chid, unsigned int reg);
void write_spi_sw_daa(rtl_spi_dev_t* pdev, int chid, unsigned int reg, unsigned int data);
#endif
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK)
/****The following is used to Legerity slic*****/
int32 _rtl_spi_rawWrite( rtl_spi_dev_t* pDev, void* pData, int32 bits );
int32 _rtl_spi_rawRead( rtl_spi_dev_t* pDev, void* pData, int32 bits );
#endif
#if defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3210 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3215 ) || defined( CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x )
/****The following is used to Silicon Lab. slic and DAA*****/
unsigned char readDirectReg_nodaisy(unsigned char address);
void writeDirectReg_nodaisy(unsigned char address, unsigned char data);
unsigned char readDAAReg(rtl_spi_dev_t *pdev, int chid, unsigned char address);
void writeDAAReg(rtl_spi_dev_t *pdev, int chid, unsigned char address, unsigned char data);
#endif
/***************************************************************************/
#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY)
//#define init_spi_channels( size, dev, cs, rst, clk, do, di )	_rtl8972B_init_spi_channels( size, dev, cs, rst, clk, do, di )
#define init_spi_pins( dev, cs, clk, do, di )  _rtl8972B_spi_init( dev, clk, cs, do, di )
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxC)
//#define init_spi_channels( size, dev, cs, rst, clk, do, di )	_rtl8954C_init_spi_channels( size, dev, cs, rst, clk, do, di )
#define init_spi_pins( dev, cs, clk, do, di )  _rtl8954C_spi_init( dev, clk, cs, do, di )
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM89xxD)
//#define init_spi_channels( size, dev, cs, rst, clk, do, di )	_rtl8972D_init_spi_channels( size, dev, cs, rst, clk, do, di )
#define init_spi_pins( dev, cs, clk, do, di )  _rtl8972D_spi_init( dev, clk, cs, do, di )
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8672)
#ifdef CONFIG_RTK_VOIP_8672_SPI_GPIO
extern int32 __rtl867x_spi_init( rtl_spi_dev_t* pDev, uint32 gpioSCLK, uint32 gpioCS_, uint32 gpioSDO, uint32 gpioSDI);
#define init_spi_pins( dev, cs, clk, do, di )  _rtl867x_spi_init( dev, clk, cs, do, di )
#elif defined (CONFIG_RTK_VOIP_8672_SHARED_SPI)
extern void __rtl867x_spi_init(int ch_spi);
#define init_spi_pins( dev, cs, clk, do, di )  __rtl867x_spi_init(0)
#endif
#elif defined(CONFIG_RTK_VOIP_DRIVERS_PCM8676)
#ifdef CONFIG_RTK_VOIP_8676_SPI_GPIO
extern int32 __rtl867x_spi_init( rtl_spi_dev_t* pDev, uint32 gpioSCLK, uint32 gpioCS_, uint32 gpioSDO, uint32 gpioSDI);
#define init_spi_pins( dev, cs, clk, do, di )  _rtl867x_spi_init( dev, clk, cs, do, di )
#elif defined (CONFIG_RTK_VOIP_8676_SHARED_SPI)
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_SILAB
extern void __rtl867x_spi_init(int ch_spi);
#define init_spi_pins( dev, cs, clk, do, di )	__rtl867x_spi_init(0)
#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK)
extern int32 __rtl867x_spi_init( rtl_spi_dev_t* pDev, uint32 SPI_SEL_CS);
#define init_spi_pins( dev, cs, clk, do, di )	__rtl867x_spi_init( dev, cs)
void vp_read(rtl_spi_dev_t *pDev, unsigned char *data);
void vp_write(rtl_spi_dev_t *pDev, unsigned int data);
#endif 
#endif
#else
#error "older spi driver"
#endif


/***************************************************************************/

#ifndef SUCCESS
#define SUCCESS 	0
#endif
#ifndef FAILED
#define FAILED 		-1
#endif

#if (defined (CONFIG_RTK_VOIP_DRIVERS_PCM8672) && defined (CONFIG_RTK_VOIP_8672_SHARED_SPI)) || (defined (CONFIG_RTK_VOIP_DRIVERS_PCM8676) && defined (CONFIG_RTK_VOIP_8676_SHARED_SPI))
#define RTK_READ_SPI(ch, reg)		read_spi_hw(ch, reg)
#define RTK_WRITE_SPI(ch, reg, data)	write_spi_hw(ch, reg, data)
#define RTK_READ_SPI_NO_DAISY(reg)		read_spi_nodaisy_hw(reg)
#define RTK_WRITE_SPI_NO_DAISY(reg, data)	write_spi_nodaisy_hw(reg, data)
#else
#define RTK_READ_SPI(ch, reg)		read_spi_sw(ch, reg)
#define RTK_WRITE_SPI(ch, reg, data)	write_spi_sw(ch, reg, data)
#define RTK_READ_SPI_NO_DAISY(reg)		read_spi_nodaisy_sw(reg)
#define RTK_WRITE_SPI_NO_DAISY(reg, data)	write_spi_nodaisy_sw(reg, data)
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
#if (defined (CONFIG_RTK_VOIP_DRIVERS_PCM8672) && defined (CONFIG_RTK_VOIP_8672_SHARED_SPI)) || (defined (CONFIG_RTK_VOIP_DRIVERS_PCM8676) && defined (CONFIG_RTK_VOIP_8676_SHARED_SPI))
#define RTK_READ_SPI_DAA(ch, reg)		read_spi_hw_daa(ch, reg)
#define RTK_WRITE_SPI_DAA(ch, reg, data)	write_spi_hw_daa(ch, reg, data)
#else
#define RTK_READ_SPI_DAA(dev, ch, reg)		read_spi_sw_daa(dev, ch, reg)
#define RTK_WRITE_SPI_DAA(dev, ch, reg, data)	write_spi_sw_daa(dev, ch, reg, data)
#endif
#endif

#endif //_SPI_H_
