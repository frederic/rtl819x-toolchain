#include <linux/config.h>
#include <linux/sched.h>	/* jiffies is defined */
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/string.h>
#include <linux/delay.h>
#include "rtk_voip.h"
#include "spi.h"

#ifdef CONFIG_RTK_VOIP_DECT_SPI_SUPPORT
//static int cur_channel = DECT_SPI_DEV;
#else
// If daisy chain used, no need to change cur_channel.
//static int cur_channel = SLIC0_SPI_DEV;
#endif
//extern unsigned long volatile jiffies;
//extern int slic_ch_num;

//static int daa_spi_dev = 1;

//rtl_spi_dev_t spi_dev[SPI_DEV_NUM];

/******************* SPI raw read/write API *************************/
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK)
#define __udelay_val cpu_data[smp_processor_id()].udelay_val
static __inline__ void __nsdelay(unsigned long nssecs, unsigned long lpj)
{
	/* This function is a copy of __udelay() in linux-2.4.18/include/asm-mips/delay.h */
    unsigned long lo;

    //usecs *= 0x00068db8;        /* 2**32 / (1000000 / HZ) */
    nssecs *= 0x000001AE;         /* 2**32 / (1000000 * 1000 / HZ) */
    __asm__("multu\t%2,%3"
        :"=h" (nssecs), "=l" (lo)
        :"r" (nssecs),"r" (lpj));
    __delay(nssecs);
}

static inline void spi_nsdelay( unsigned long delay )
{
	__nsdelay( delay, __udelay_val );	// __udelay_val defined in linux-2.4.18/include/asm-mips/delay.h
}
#undef spi_rw_delay 
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#if (__GNUC__ == 4) && (__GNUC_MINOR__ == 4)  //RSDK 1.5.4 4.4
#define spi_rw_delay() __ndelay(300)	//data clock period MIN 48ns. GPIO delay latency may large then 122 ns
#else
#define spi_rw_delay() __ndelay(300)	//data clock period MIN 48ns. GPIO delay latency may large then 122 ns
#endif
#define spi_cs_delay() __ndelay(2200)	//chip select off time MIN 2500ns
#else
#define spi_rw_delay() spi_nsdelay(200) 
#define spi_cs_delay() spi_nsdelay(2200) 
#endif

/******************* SPI raw read/write API *************************/
int32 _rtl_spi_rawRead( rtl_spi_dev_t* pDev, void* pData, int32 bits )
{
	uint8* pch = pData;
	*pch = 0;

	if ( pData == NULL ) return FAILED;

	RTK_GPIO_SET( pDev->gpioCS_, 0 ); /* fall down the CS_ */
	RTK_GPIO_SET( pDev->gpioSCLK, 0 ); /* fall down the SCLK */
	spi_rw_delay(); /* delay for a while */
	//RTK_GPIO_SET( pDev->gpioSCLK, 0 ); /* fall down the SCLK */

	for( bits--; bits >= 0; bits-- )
	{
		uint32 buf;

		RTK_GPIO_SET( pDev->gpioSCLK, 0 ); /* raising up the SCLK */
		spi_rw_delay(); /* delay for a while */

		pch[bits/8] &= ~((uint8)1<<(bits&0x7));
		RTK_GPIO_GET( pDev->gpioSDI, &buf );
		pch[bits/8] |= buf?((uint8)1<<(bits&0x7)):0;
		
		RTK_GPIO_SET( pDev->gpioSCLK, 1 ); /* fall down the SCLK */
		spi_rw_delay(); /* delay for a while */
	}	
	
	RTK_GPIO_SET( pDev->gpioCS_, 1 ); /* raise the CS_ */
	RTK_GPIO_SET( pDev->gpioSCLK, 1 ); /* raise the SCLK */
	
	spi_cs_delay(); /* delay for a while >2500 ns */
	
	return SUCCESS;
}

int32 _rtl_spi_rawWrite( rtl_spi_dev_t* pDev, void* pData, int32 bits )
{
	uint8* pch = pData;

	if ( pData == NULL ) return FAILED;

	RTK_GPIO_SET( pDev->gpioSCLK, 0 ); /* fall down SCLK */
	RTK_GPIO_SET( pDev->gpioCS_, 0 ); /* fall down the CS_ */

	for( bits-- ; bits >= 0; bits-- )
	{
		RTK_GPIO_SET( pDev->gpioSDO, (pch[bits/8]&((uint32)1<<(bits&0x7)))?1:0 );
		spi_rw_delay(); /* delay for a while */
		RTK_GPIO_SET( pDev->gpioSCLK, 1 ); /* raising up the SCLK */
		spi_rw_delay(); /* delay for a while */
		RTK_GPIO_SET( pDev->gpioSCLK, 0 ); /* fall down the SCLK */
	}	
	
	spi_rw_delay(); /* delay for a while */
	RTK_GPIO_SET( pDev->gpioCS_, 1 ); /* raise the CS_ */
	RTK_GPIO_SET( pDev->gpioSCLK, 1 ); /* raise the SCLK */
	
	spi_cs_delay(); /* delay for a while >2500 ns */

	return SUCCESS;

}

#else

int32 _rtl_spi_rawRead( rtl_spi_dev_t* pDev, void* pData, int32 bits )
{
	uint8* pch = pData;
	
	*pch = 0;

	if ( pData == NULL ) return FAILED;
	
	RTK_GPIO_SET( pDev->gpioCS_, 1 ); /* raise the CS_ */
	RTK_GPIO_SET( pDev->gpioSCLK, 1 ); /* raise the SCLK */
	spi_rw_delay(); /* delay for a while */

	RTK_GPIO_SET( pDev->gpioCS_, 0 ); /* fall down the CS_ */
	spi_rw_delay(); /* delay for a while */

	for( bits--; bits >= 0; bits-- )
	{
		uint32 buf;

		RTK_GPIO_SET( pDev->gpioSCLK, 0 ); /* fall down the SCLK */
		spi_rw_delay(); /* delay for a while */

		pch[bits/8] &= ~((uint8)1<<(bits&0x7));
		RTK_GPIO_GET( pDev->gpioSDI, &buf );
		pch[bits/8] |= buf?((uint8)1<<(bits&0x7)):0;
		
		RTK_GPIO_SET( pDev->gpioSCLK, 1 ); /* raising up the SCLK */
		spi_rw_delay(); /* delay for a while */
	}	
	
	RTK_GPIO_SET( pDev->gpioCS_, 1 ); /* raise the CS_ */
	RTK_GPIO_SET( pDev->gpioSCLK, 1 ); /* raise the SCLK */
	
	
	return SUCCESS;
}

int32 _rtl_spi_rawWrite( rtl_spi_dev_t* pDev, void* pData, int32 bits )
{
	uint8* pch = pData;

	if ( pData == NULL ) return FAILED;
	
	RTK_GPIO_SET( pDev->gpioCS_, 1 ); /* raise the CS_ */
	RTK_GPIO_SET( pDev->gpioSCLK, 1 ); /* raise the SCLK */
	spi_rw_delay(); /* delay for a while */

	RTK_GPIO_SET( pDev->gpioCS_, 0 ); /* fall down the CS_ */
	spi_rw_delay(); /* delay for a while */

	for( bits-- ; bits >= 0; bits-- )
	{
		RTK_GPIO_SET( pDev->gpioSDO, (pch[bits/8]&((uint32)1<<(bits&0x7)))?1:0 );
		RTK_GPIO_SET( pDev->gpioSCLK, 0 ); /* fall down the SCLK */
		spi_rw_delay(); /* delay for a while */
		RTK_GPIO_SET( pDev->gpioSCLK, 1 ); /* raising up the SCLK */
		spi_rw_delay(); /* delay for a while */
	}	
	
	RTK_GPIO_SET( pDev->gpioCS_, 1 ); /* raise the CS_ */
	RTK_GPIO_SET( pDev->gpioSCLK, 1 ); /* raise the SCLK */
	
	
	return SUCCESS;

}
#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK


/********* Silicon Lab SLIC SPI Register read/write API with "Daisy Chain" *********/
#if 0
unsigned char read_spi_sw(unsigned int chid, unsigned int reg)
{
	uint8 buf;
	unsigned long flags;
	//remove by tim 20100830	
	//save_flags(flags); cli();

	// select SLIC (SPI daisy chain mode)	
	buf = 1 << chid;
	_rtl_spi_rawWrite(&spi_dev[cur_channel], &buf, 8);
	
	buf = reg | 0x80;
	_rtl_spi_rawWrite(&spi_dev[cur_channel], &buf, 8);
		
	_rtl_spi_rawRead(&spi_dev[cur_channel], &buf, 8);
	
	//remove by tim 20100830	
	//restore_flags(flags);
	
	return buf;
}

void write_spi_sw(unsigned int chid, unsigned int reg, unsigned int data)
{
	uint8 buf;
	unsigned long flags;
		
	//remove by tim 20100830
	//save_flags(flags); cli();

	// select SLIC (SPI daisy chain mode)
	buf = 1 << chid;
	_rtl_spi_rawWrite(&spi_dev[cur_channel], &buf, 8);
	
	buf = reg;
	_rtl_spi_rawWrite(&spi_dev[cur_channel], &buf, 8);

	buf = data;
	_rtl_spi_rawWrite(&spi_dev[cur_channel], &buf, 8);
		
	//remove by tim 20100830	
	//restore_flags(flags);

}

static void waitForIndirectReg(unsigned int chid)
{
	while (readDirectReg(chid, 31));
}

void cyg_thread_delay(int delay)
{
	unsigned long t1 = jiffies+delay;
	while(jiffies < t1);
}

unsigned char readDirectReg(unsigned int chid, unsigned char address)
{	
	return RTK_READ_SPI(chid, address);
}


void writeDirectReg(unsigned int chid, unsigned char address, unsigned char data)
{

	RTK_WRITE_SPI(chid, address, data);
}

unsigned short readIndirectReg(unsigned int chid, unsigned char address)
{ 
	waitForIndirectReg(chid);
	 
	writeDirectReg(chid, 30,address); 
	waitForIndirectReg(chid);
	return ( readDirectReg(chid, 28) | (readDirectReg (chid, 29))<<8);
}


void writeIndirectReg(unsigned int chid, unsigned char address, unsigned short data)
{
	waitForIndirectReg(chid);
	writeDirectReg(chid, 28,(unsigned char)(data & 0xFF));
	writeDirectReg(chid, 29,(unsigned char)((data & 0xFF00)>>8));
	writeDirectReg(chid, 30,address);
}
#endif

/******** Silicon Lab SLIC SPI Register read/write API "without" Daisy Chain ***********/

#if 0
unsigned char read_spi_nodaisy_sw(unsigned int reg)
{
	uint8 buf;
	unsigned long flags;
	save_flags(flags); cli();
	
	buf = reg | 0x80;

	_rtl_spi_rawWrite(&spi_dev[cur_channel], &buf, 8);
	_rtl_spi_rawRead(&spi_dev[cur_channel], &buf, 8);

	restore_flags(flags);
	
	return buf;
}

void write_spi_nodaisy_sw(unsigned int reg, unsigned int data)
{
	uint8 buf;
	unsigned long flags;
	save_flags(flags); cli();
	
	buf = reg;

	_rtl_spi_rawWrite(&spi_dev[cur_channel], &buf, 8);
	
	buf = data;
	_rtl_spi_rawWrite(&spi_dev[cur_channel], &buf, 8);

	restore_flags(flags);
}

unsigned char readDirectReg_nodaisy(unsigned char address)
{
	return RTK_READ_SPI_NO_DAISY(address);
}

void writeDirectReg_nodaisy(unsigned char address, unsigned char data)
{
	RTK_WRITE_SPI_NO_DAISY(address, data);
}
#endif

/*********************************** Silicon Lab DAA spi/register read/write API ***********************************/
//The following function is used to DAA(Si3050 Si3018/19).
//Because of differential SPI interface, people must call 
//those function when they want to initialize DAA. 
//chiminer 12_19_2005
/*******************************************************************************************************************/

#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
unsigned char read_spi_sw_daa(rtl_spi_dev_t* pdev, int chid, unsigned int reg)
{
	uint8 buf;
	unsigned long flags;
	save_flags(flags); cli();

	//if (chid == slic_ch_num)
	//	daa_spi_dev = DAA0_SPI_DEV;
	//else if (chid == (slic_ch_num+1))
	//	daa_spi_dev = DAA1_SPI_DEV;

	buf = 0x60;	
	//_rtl_spi_rawWrite(&spi_dev[daa_spi_dev], &buf, 8);
	_rtl_spi_rawWrite(pdev, &buf, 8);
	
	buf = reg;	
	//_rtl_spi_rawWrite(&spi_dev[daa_spi_dev], &buf, 8);
	_rtl_spi_rawWrite(pdev, &buf, 8);
		
	//_rtl_spi_rawRead(&spi_dev[daa_spi_dev], &buf, 8);
	_rtl_spi_rawRead(pdev, &buf, 8);

	restore_flags(flags);
	
	return buf;  	 
}	

void write_spi_sw_daa(rtl_spi_dev_t* pdev, int chid, unsigned int reg, unsigned int data)
{
	uint8 buf;
	unsigned long flags;
	save_flags(flags); cli();
	
	//if (chid == slic_ch_num)
	//	daa_spi_dev = DAA0_SPI_DEV;
	//else if (chid == (slic_ch_num+1))
	//	daa_spi_dev = DAA1_SPI_DEV;

                 
	buf = 0x20;
	//_rtl_spi_rawWrite(&spi_dev[daa_spi_dev], &buf, 8);
	_rtl_spi_rawWrite(pdev, &buf, 8);
	                 
	buf = reg;       
	//_rtl_spi_rawWrite(&spi_dev[daa_spi_dev], &buf, 8);
	_rtl_spi_rawWrite(pdev, &buf, 8);

	buf = data;
	//_rtl_spi_rawWrite(&spi_dev[daa_spi_dev], &buf, 8);
	_rtl_spi_rawWrite(pdev, &buf, 8);

	restore_flags(flags);
}

unsigned char readDAAReg(rtl_spi_dev_t *pdev, int chid, unsigned char address)
{	
#if defined (CONFIG_RTK_VOIP_SLIC_SI32178) || defined (CONFIG_RTK_VOIP_SLIC_SI32176_SI32178)
	//extern unsigned char R_reg(unsigned char chid, unsigned char regaddr);
	//return R_reg(chid, address);
	extern unsigned char R_reg_dev(rtl_spi_dev_t *pdev, unsigned char chid, unsigned char regaddr);
	return R_reg_dev( pdev, chid, address );
#else
	return RTK_READ_SPI_DAA(pdev, chid, address);
#endif
}


void writeDAAReg(rtl_spi_dev_t *pdev, int chid, unsigned char address, unsigned char data)
{
#if defined (CONFIG_RTK_VOIP_SLIC_SI32178) || defined (CONFIG_RTK_VOIP_SLIC_SI32176_SI32178)
	//extern void W_reg(unsigned char chid, unsigned char regaddr, unsigned char data);
	//W_reg(chid, address, data);
	extern void W_reg_dev(rtl_spi_dev_t *pdev, unsigned char chid, unsigned char regaddr, unsigned char data);
	W_reg_dev( pdev, chid, address, data );
#else
	RTK_WRITE_SPI_DAA(pdev, chid, address, data);
#endif
}
#endif //CONFIG_RTK_VOIP_DRIVERS_SI3050

/************** Winbond slic W682388 register read/write API ******************/
#if 0
int WriteReg(unsigned char Address, unsigned char Data)
{
	const unsigned char cmd = 0x04;
	unsigned char buf;
	unsigned short cmd_address;
	
	cmd_address = (Address<<8) | cmd;
	_rtl_spi_rawWrite(&spi_dev[cur_channel], &cmd_address, 16);
	
	buf = Data;
	_rtl_spi_rawWrite(&spi_dev[cur_channel], &buf, 8);


	return 0;
}

unsigned char ReadReg(unsigned char Address)
{
	const unsigned char cmd = 0x84;
	unsigned char buf;
	unsigned short cmd_address;
	
	cmd_address = (Address<<8) | cmd;
	_rtl_spi_rawWrite(&spi_dev[cur_channel], &cmd_address, 16);

	_rtl_spi_rawRead(&spi_dev[cur_channel], &buf, 8);
	
	return buf;

}
#endif

