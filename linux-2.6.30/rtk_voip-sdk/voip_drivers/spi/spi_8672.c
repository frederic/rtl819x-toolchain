/*****************************************************************************
*	SPI driver shared with SPI flash	
*
*	Initial version: 			2008/04/15
*
*
*****************************************************************************/
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/string.h>
#include <linux/delay.h>
#include "rtk_voip.h"
#include "spi.h"


//#define SHARE_SPI
#define SPI_VOIP_MIN_DELAY 10000

#ifdef CONFIG_RTK_VOIP_8672_SPI_GPIO
extern rtl_spi_dev_t spi_dev[SPI_DEV_NUM];
#endif

// gpio simulated spi interface
#ifdef CONFIG_RTK_VOIP_8672_SPI_GPIO

/************************************* Set GPIO Pin to SPI ***********************************************************/
/*
@func int32 | _rtl867x_spi_init | Initialize SPI device
@parm rtl_spi_dev_t* | pDev | Structure to store device information
@parm uint32 | gpioSCLK | GPIO ID of SCLK
@parm uint32 | gpioCS_ | GPIO ID of CS_
@parm uint32 | gpioSDI | GPIO ID of SDI
@parm uint32 | gpioSDO | GPIO ID of SDO
@parm uint32 | maxSpeed | how fast SPI driver can generate the SCLK signal (unit: HZ)
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
static int32 _rtl867x_spi_init( rtl_spi_dev_t* pDev, uint32 gpioSCLK, uint32 gpioCS_, uint32 gpioSDO, uint32 gpioSDI)
{
	
	pDev->gpioSCLK = gpioSCLK;
	pDev->gpioCS_ = gpioCS_;
	pDev->gpioSDI = gpioSDI;
	pDev->gpioSDO = gpioSDO;

	_rtl867x_initGpioPin( gpioSCLK, GPIO_DIR_OUT);
	_rtl867x_initGpioPin( gpioCS_, GPIO_DIR_OUT);
	_rtl867x_initGpioPin( gpioSDI, GPIO_DIR_IN);
	_rtl867x_initGpioPin( gpioSDO, GPIO_DIR_OUT);
	
	return SUCCESS;
}

/*
@func int32 | _rtl867x_spi_exit | Called when a SPI device is released
@parm rtl_spi_dev_t* | pDev | Structure containing device information
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
static int32 _rtl867x_spi_exit( rtl_spi_dev_t* pDev )
{
	return SUCCESS;
}
 
static void init_channel(int channel, uint32 pin_cs, uint32 pin_reset, uint32 pin_clk, uint32 pin_do, uint32 pin_di)
{
	int i;
	static unsigned char inital = 0;

	_rtl867x_initGpioPin(pin_reset, GPIO_DIR_OUT);
	_rtl867x_initGpioPin(pin_cs, GPIO_DIR_OUT);

	_rtl867x_spi_init(&spi_dev[channel], pin_clk, pin_cs, pin_do, pin_di);

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8671
	static int delay_cnt = 50000;
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8672
	static int delay_cnt = 100000;
#endif
	
	if ((channel == 0) || (inital == 0)) 
	{
		#if speed_booting_rating
			for(i=0;i<delay_cnt;i++);
		#else
			cyg_thread_delay(50);
		#endif
		_rtl867x_setGpioDataBit(pin_cs, 1); 	/* CS_ preset to high state*/
		_rtl867x_setGpioDataBit(pin_clk, 1); 	/* SCLK to high state*/
		
		_rtl867x_setGpioDataBit(pin_reset, 1);	// Reset high
		#if speed_booting_rating
			for(i=0;i<delay_cnt;i++);
		#else
			cyg_thread_delay(50);
		#endif
		_rtl867x_setGpioDataBit(pin_reset, 0);	// set reset low, PCLK and FS id present and stable.
		#if speed_booting_rating
			for(i=0;i<delay_cnt;i++);
		#else
			cyg_thread_delay(50);
		#endif
		_rtl867x_setGpioDataBit(pin_reset, 1);	// release reset
		#if speed_booting_rating
			for(i=0;i<delay_cnt;i++);
		#else
			cyg_thread_delay(50);
		#endif				// wait more than 100ms 
			inital++;

	}

}

//------------------------------------------
void init_spi(int ch_spi)
{
	
	printk("( GPIO %s )  ", GPIO );
	
	if (ch_spi == 0)
	{
		printk("for SLIC[%d]...", ch_spi);
		init_channel(0, PIN_CS1, PIN_RESET1, PIN_CLK, PIN_DO, PIN_DI);
		
		printk("=> OK !\n");
	}
	else if (ch_spi == 1)
	{
		printk("for SLIC/DAA[%d]...", ch_spi);
		init_channel(1, PIN_CS3_DAA, PIN_RESET3_DAA, PIN_CLK_DAA, PIN_DO_DAA, PIN_DI_DAA);
		printk("=> OK !\n");
	}
	else 
	{
		printk("No GPIO Pin assign for any device -> Can't initialize any device\n");
		
	}
}

#endif //CONFIG_RTK_VOIP_8672_SPI_GPIO

// 8672 HW SPI
#ifdef CONFIG_RTK_VOIP_8672_SHARED_SPI

extern spinlock_t spi_lock;
/*---------------- External Function Prototypes -----------------*/
extern void gpioConfig (int gpio_num, int gpio_func);
extern void gpioSet(int gpio_num);

#ifdef  SPI_8672_DEBUG
/*---------------------------------------------------------
*	Name:
*		spi_dump_regs
*	Description:
*		Debug information, dump spi related registers
*	Input:	none
*	Ouput:	none
*	Return:	none
*--------------------------------------------------------*/
void spi_dump_regs(void)
{
	rtl_8672_reg_t	*spi;

	/* System configuration */
	spi = get_spi_reg();
	/*Related Registers*/
	SPIDBG("\r\n GIMR [%08X]: %08X",0xB8003000, *(volatile int*)0xB8003000);
	SPIDBG("\r\n GISR [%08X]: %08X",0xB8003004, *(volatile int*)0xB8003004);
	SPIDBG("\r\n PINMUXCR [%08X]: %08X",RT8672_PINMUX, *(volatile int*)RT8672_PINMUX);
	SPIDBG("\r\n PINSR [%08X]: %08X",0xB8003308, *(volatile int*)0xB8003308);
	SPIDBG("\r\n MECR [%08X]: %08X",0xB800330C, *(volatile int*)0xB800330C);
	SPIDBG("\r\n=======   SPI   ===========\r\n");
	SPIDBG("SPICNR: 0x%08X \r\n", spi->SPICNR);
	SPIDBG("SPISTR: 0x%08X \r\n", spi->SPISTR);
	SPIDBG("SPICKDIV: 0x%08X \r\n", spi->SPICKDIV);
	SPIDBG("SPIRDR: 0x%08X \r\n", spi->SPIRDR);
	SPIDBG("SPITDR: 0x%08X \r\n", spi->SPITDR);
}

/*---------------------------------------------------------
*	Name:
*		spi_pcm_regs
*	Description:
*		Debug information, dump pcm related registers
*	Input:	none
*	Ouput:	none
*	Return:	none
*--------------------------------------------------------*/
void spi_pcm_regs(void)
{
	/* System configuration */
	SPIDBG("\r\n PCM related regs:");
	SPIDBG("\r\n GIMR [%08X]: %08X",0xB8003000, *(volatile int*)0xB8003000);
	SPIDBG("\r\n GISR [%08X]: %08X",0xB8003004, *(volatile int*)0xB8003004);
	SPIDBG("\r\n IRR2 [%08X]: %08X",0xB800300C, *(volatile int*)0xB800300C);
	SPIDBG("\r\n PINMUXCR [%08X]: %08X",RT8672_PINMUX, *(volatile int*)RT8672_PINMUX);
	SPIDBG("\r\n PINSR [%08X]: %08X",0xB8003308, *(volatile int*)0xB8003308);
	SPIDBG("\r\n ->MECR [%08X]: %08X",0xB800330C, *(volatile int*)0xB800330C);
}

struct pinmux_reg_name 
{
	unsigned int bit;
	unsigned char  reason[32];
};

struct pinmux_reg_name pinmux_dbg_set[] = 
{
	{31, "GPIO_SELJTAG"},
	{30, "GPIO_SELPCM"},
	{29, "GPIO_SELPCI"},
	{28, "GPIO_SELUART"},
	{27, "GPIO_SELSPI"},
	{26, "GPA5_SELVOIPCSB"},
	{25, "GPC456_SELVOIP"},
	{24, "MA1920_SELSPI"},
	{23, "GPIO_SELUART1CTRL"},
	{22, "GPC0123_SELPCM"},
	{21, "GPD_SELUART2"},
	{20, "GPIO_SELUART1CTRL"},
	{19, "PCIMode[1]"},
	{18, "PCIMode[0]"},
	{17, "MdMode[1]"},
	{16, "MdMode[0]"},
	{15, "ChipMode[1]"},
	{14, "ChipMode[0]"}	
};


/*---------------------------------------------------------
*	Name:
*		spi_dump_pinmux
*	Description:
*		Debug information, dump pcm related registers
*	Input:	none
*	Ouput:	none
*	Return:	none
*--------------------------------------------------------*/
void spi_dump_pinmux()
{
	unsigned int val, index;

	val = *((volatile int*)RT8672_PINMUX);
	printk("\r\n-- %x --",  sizeof(pinmux_dbg_set) / sizeof(pinmux_dbg_set[0]));
	for (index = 0 ; index < sizeof  (pinmux_dbg_set) / sizeof(pinmux_dbg_set[0]); index++ )
	{
			if (val & (1 << pinmux_dbg_set[index].bit) )
				printk("\r\n%s  \t1", pinmux_dbg_set[index].reason);
			else
				printk("\r\n%s  \t0", pinmux_dbg_set[index].reason);
	}

}
#endif
/*---------------------------------------------------------
*	Name:
*		cs_select
*	Description:
*		select cs as slic or daa
*	Input:	
		ch_spi - 0 : SLIC
			     1 : DAA
*	Ouput:	none
*	Return:	none
*--------------------------------------------------------*/
void cs_select(int ch)
{
	
	if (ch  == 0)
	{
#ifdef CONFIG_6028_IAD_BGA_PIN_DEF
		//GPA5_SELVOIPCSB
		*((volatile unsigned int *)(RT8672_PINMUX)) |= (1 << 26);	
		*((volatile unsigned int *)(0xb8003504)) |= (1 << CS_USED_SLIC);	
		*((volatile unsigned int *)(0xb8003504)) &= ~(1 << CS_USED_DAA);	
		//GPA1 = 1
		gpioSet(CS_USED_DAA);
		//*((volatile unsigned int *)(0xb8003508)) |= (1 << CS_USED_DAA);	
		//*((volatile unsigned int *)(0xb800350C)) |= (1 << CS_USED_DAA);	
#else    
        //CONFIG_6166_IAD_SILAB3217X
		*((volatile unsigned int *)(RT8672_PINMUX)) &= ~(1 << 26);	/* set GPA5_SELVOIPCSB n.26=0, let A5 as generic GPIO*/
        *((volatile unsigned int *)(RT8672_PINMUX)) |= (1 << 27);   /* set GPIO_SELSPI n.27=1, let B0 as CS */
		*((volatile unsigned int *)(0xb8003504)) |= (1 << CS_USED_SLIC);	
		*((volatile unsigned int *)(0xb8003504)) &= ~(1 << CS_USED_DAA);	
		gpioSet(CS_USED_DAA);

#endif
	}
	else if (ch == 1) 
	{
#ifdef  CONFIG_6028_IAD_BGA_PIN_DEF
		//GPA5_SELVOIPCSB
		*((volatile unsigned int *)(RT8672_PINMUX)) &= ~(1 << 26);	
		*((volatile unsigned int *)(0xb8003504)) &= ~(1 << CS_USED_SLIC);	
		*((volatile unsigned int *)(0xb8003504)) |= (1 << CS_USED_DAA);	
		//GPA5 = 1
		gpioSet(CS_USED_SLIC);
		//*((volatile unsigned int *)(0xb8003508)) |= (1 << 5);	
		//*((volatile unsigned int *)(0xb800350C)) |= (1 << 5);	

#else   //CONFIG_6166_IAD_SILAB3217X
		*((volatile unsigned int *)(RT8672_PINMUX)) &= ~(1 << 26);	
		*((volatile unsigned int *)(RT8672_PINMUX)) &= ~(1 << 27);	
		*((volatile unsigned int *)(0xb8003504)) &= ~(1 << CS_USED_SLIC);	
		*((volatile unsigned int *)(0xb8003504)) |= (1 << CS_USED_DAA);	
		//GPA5 = 1
		gpioSet(CS_USED_SLIC);

#endif    
	}
		
}

/*---------------------------------------------------------
*	Name:
*		reset_spi
*	Description:
*		Initialize the corresponding spi channel.
*	Input:	
		ch_spi - the spi channel to setup
*	Ouput:	none
*	Return:	none
*--------------------------------------------------------*/
void rtl8672_share_spi_write(unsigned int ch, unsigned int uses_daisy, unsigned int contorl, unsigned int address,unsigned int data)
{
	unsigned int		val,i,spicnr;
	rtl_8672_reg_t	*spi_reg=get_spi_reg();
	int flags;
	
	spin_lock_irqsave(spi_lock,flags);

#ifdef SHARE_SPI	
	//MA1920_SELSPI
	*((volatile unsigned int *)(RT8672_PINMUX)) |= (1 << 24);	
#endif //SHARE_SPI
	cs_select(ch);

	//clear all interrupt
	spi_reg->SPISTR = 0xC0000000;
	
	/*Default uses daisy chain*/	
	spicnr = SPI_CMD(1) | SPI_RDIP(1)| SPI_WDIP(1)| SPI_SPIE(1) | SPI_CSP(0) |SPI_WLEN(0)|SPI_LSB(0) |SPI_CSTIME(0);
	if (uses_daisy == 0)
		spicnr |= SPI_DAISY(1);
	
	//ctrl 
	spicnr |=  (contorl << 8);
	if (uses_daisy == 1)
		spicnr |=  (address << 16) | SPI_START(1);
	else 
		spicnr |=  (contorl << 16) | SPI_START(1);

	//data
	spi_reg->SPITDR =  (data << 24) |(data << 16) ;
	spi_reg->SPIRDR =  (data << 24) |(data << 16) ;

	//start transfer
	spi_reg->SPICNR = spicnr;
	
	//wait unitl finish
	for(i=0;i<SPI_VOIP_MIN_DELAY;i++);
	while (spi_reg->SPISTR  == 0) ;
#ifdef SHARE_SPI		
	//MA1920_SELSPI
	*((volatile unsigned int *)(RT8672_PINMUX)) &= ~(1 << 24);	
#endif
	spin_unlock_irqrestore(spi_lock,flags);

	return;

}
/*---------------------------------------------------------
*	Name:
*		reset_spi
*	Description:
*		Initialize the corresponding spi channel.
*	Input:	
		ch_spi - the spi channel to setup
*	Ouput:	none
*	Return:	none
*--------------------------------------------------------*/
unsigned int  rtl8672_share_spi_read(unsigned int ch, unsigned int uses_daisy, unsigned int control, unsigned int address,unsigned char *data)
{
	unsigned int		val,i,spicnr;
	uint8 buf;
	rtl_8672_reg_t	*spi_reg=get_spi_reg();

	int flags;
	
	spin_lock_irqsave(spi_lock,flags);

#ifdef SHARE_SPI		
	//MA1920_SELSPI
	*((volatile unsigned int *)(RT8672_PINMUX)) |= (1 << 24);	
#endif
	cs_select(ch);

	//clear all interrupt
	spi_reg->SPISTR = 0xC0000000;
	
	/*Default uses daisy chain*/	
	spicnr =  SPI_RDIP(1)| SPI_WDIP(1)| SPI_SPIE(1) | SPI_CSP(0) |SPI_WLEN(0)|SPI_LSB(0) |SPI_CSTIME(0);
	if (uses_daisy == 0)
		spicnr |= SPI_DAISY(1);
	
	//ctrl 
	spicnr |=  (control << 8);
	
	if (uses_daisy == 1)
		spicnr |=  (address << 16) | SPI_START(1);
	else 
		spicnr |=  (control << 16) | SPI_START(1);

	//start transfer
	spi_reg->SPICNR = spicnr;

	//wait unitl finish
	for(i=0;i<SPI_VOIP_MIN_DELAY;i++);
	while (spi_reg->SPISTR  == 0) ;
	
	buf = (unsigned char) (spi_reg->SPIRDR >> 24);
	*data = buf;
	
#ifdef SHARE_SPI	
	//MA1920_SELSPI 
	*((volatile unsigned int *)(RT8672_PINMUX)) &= ~(1 << 24);	
#endif
	spin_unlock_irqrestore(spi_lock,flags);
	return buf; 	

}


/*---------------------------------------------------------
*	Name:
*		init_spi
*	Description:
*		Initialize the corresponding spi channel.
*	Input:	
		ch_spi - the spi channel to setup
*	Ouput:	none
*	Return:	none
*--------------------------------------------------------*/
void init_spi(int ch_spi)
{
	rtl_8672_reg_t	*spi_reg;
	unsigned int		val,i;
	int flags;	

	//memset((void*)&spi_dev[ch_spi] , 0, sizeof(rtl_spi_dev_t));		


	if (ch_spi !=0)
		return;
//int flags;
	spin_lock_irqsave(spi_lock,flags);
#ifndef CONFIG_RTK_VOIP_SLIC_NUM_4 //for another slic	
//#if defined(CONFIG_6028_IAD_BGA_PIN_DEF) || defined(CONFIG_6166_IAD_SILAB3217X)
	/* Added for 301 pin BGA*/
	//gpioConfig(PIN_RELAY, 2); // config GPA4 as output pin
	//gpioSet(PIN_RELAY);
//#endif	
#endif 

#ifdef SHARE_SPI	
	//MA1920_SELSPI
	*((volatile unsigned int *)(RT8672_PINMUX)) |= (1 << 24);	
#endif
	spi_reg = get_spi_reg();
	/*Initialize Registers*/	
	spi_reg->SPICNR = SPI_RDIP(1)| SPI_WDIP(1)| SPI_SPIE(1) | SPI_CSP(0) | SPI_DAISY(0)|SPI_WLEN(0)|SPI_LSB(0) |SPI_CSTIME(1);
	spi_reg->SPICKDIV = SPI_DIV(0);

//memset((void*)&spi_dev[ch_spi] , 0, sizeof(rtl_spi_dev_t));

#ifdef SHARE_SPI	
	//check hardware pin ShareSPI
	val = *((volatile unsigned int *)(RT8672_PINSR));
	if ((val & (1 << 9)) == 0)
		printk("FATAL Error, Check ShareSPI HW pin ");
#endif
	//before goto VOIPCS mode,  pull high

	gpioConfig(CS_USED_SLIC, 0x0002);
	gpioSet(CS_USED_SLIC);
	gpioConfig(CS_USED_DAA, 0x0002);
	gpioSet(CS_USED_DAA);	

	//reset process


	gpioConfig(RESET_PIN, 0x0002);
	gpioSet(RESET_PIN);
		for(i=0;i<100000;i++);
	gpioClear(RESET_PIN);
		for(i=0;i<100000;i++);
	gpioSet(RESET_PIN);

#ifdef CONFIG_6028_IAD_BGA_PIN_DEF
	//GPA5_SELVOIPCSB
    *((volatile unsigned int *)(0xb8003310)) |= (1<<9);
    printk("*****  PCM I/O pad driving(bit 9) reg 0x%08x  *****\n",     *((volatile unsigned int *)(0xb8003310)));  //enhance pcm i/o driving from 2mA to 4mA
    *((volatile unsigned int *)(RT8672_PINMUX)) |= (1 << 26);	

    //!!GPC456 selvoip
	*((volatile unsigned int *)(RT8672_PINMUX)) |= (1 << 25);	
	//enable VoIP
	*((volatile unsigned int *)(RT8672_PINMUX)) |= (1 << 8);	
//#endif 
//#ifdef CONFIG_6166_IAD_SILAB3217X    
#else

    *((volatile unsigned int *)(0xb8003504)) |= (1 << CS_USED_SLIC);	

    //use GPIO_SELSPI, set GPA7, GPB0-2 as SPI pin 
	*((volatile unsigned int *)(RT8672_PINMUX)) |= (1 << 27);	
	*((volatile unsigned int *)(RT8672_PINMUX)) &= ~(1 << 23);	

	//enable VoIP
	*((volatile unsigned int *)(RT8672_PINMUX)) |= (1 << 8);	
#endif    
#ifdef SHARE_SPI	
	//MA1920_SELSPI 
	*((volatile unsigned int *)(RT8672_PINMUX)) &= ~(1 << 24);	
#endif
#ifdef  SPI_8672_DEBUG
	spi_dump_pinmux();
#endif
	spin_unlock_irqrestore(spi_lock,flags);


}

/*---------------------------------------------------------
*
*--------------------------------------------------------*/
static unsigned char share_spi_read_nodaisy(unsigned int reg, unsigned char *data)
{
	unsigned int		val,i,spicnr;
	uint8 buf;
	rtl_8672_reg_t	*spi_reg=get_spi_reg();

	return rtl8672_share_spi_read(0, 0, (reg | 0x80), 0, data );
}


static void share_spi_write_nodaisy(unsigned int reg, unsigned int data)
{
	unsigned int		val,i;
	uint8 buf;
	rtl_8672_reg_t	*spi_reg=get_spi_reg();

	rtl8672_share_spi_write(0, 0, reg, 0, data);
}

unsigned char read_spi_nodaisy_hw(unsigned int reg)
{
	unsigned char buf;
	share_spi_read_nodaisy(reg, &buf);
	return buf;
}


void write_spi_nodaisy_hw(unsigned int reg, unsigned int data)
{
	share_spi_write_nodaisy(reg, data);
}

/*---------------------------------------------------------
*	Name:
*		read_spi
*	Description:
*		read the spi data from specified channel id. (for Si3215/3210)
*	Input: 
*		chid - channel id (daisy_chain index)
*		reg - reg number
*	Ouput: TBD
*	Return: TBD
*--------------------------------------------------------*/
 unsigned char read_spi_hw(unsigned int chid, unsigned int reg)
{
	uint8 buf;
	unsigned long flags, i;
	volatile rtl_8672_reg_t	*spi_reg=get_spi_reg();

	rtl8672_share_spi_read(0, 1, (chid+1) , (reg | 0x80), &buf );
	return buf;
}

/*---------------------------------------------------------
*	Name:
*		write_spi
*	Description:
*		write the spi data to specified channel id. (for Si3215/3210)
*	Input: 
*		chid - 
*		reg -
*	Ouput: TBD
*	Return: TBD
*--------------------------------------------------------*/
 void write_spi_hw(unsigned int chid, unsigned int reg, unsigned int data)
{
	unsigned long flags;
	int i;
	rtl_8672_reg_t	*spi_reg=get_spi_reg();

	rtl8672_share_spi_write(0, 1, (chid+1)  , reg,  data);
	return;
}

/*---------------------------------------------------------
*	Name:
*		read_spi_daa
*	Description:
*
*	Input: 
*		chid - 
*		reg -
*	Ouput: TBD
*	Return: TBD
*--------------------------------------------------------*/
unsigned char read_spi_hw_daa(int chid, unsigned int reg)
{
	uint8 buf;
	unsigned long flags;
	rtl_8672_reg_t	*spi_reg=get_spi_reg();

	rtl8672_share_spi_read(1, 1, 0x60, reg , &buf);
	return buf;
}

void write_spi_hw_daa(int chid, unsigned int reg, unsigned int data)
{
	unsigned long flags;
	rtl_8672_reg_t	*spi_reg = get_spi_reg();

	rtl8672_share_spi_write(1, 1, 0x20  , reg,  data);
	return;
}
#endif //CONFIG_RTK_VOIP_8672_SHARED_SPI

__inline__ void __nsdelay(unsigned long nssecs, unsigned long lpj)
{
	/* This function is a copy of __udelay() in linux-2.4.18/include/asm-mips/delay.h */
    unsigned long lo;

    nssecs *= 0x000001AE;        /* 2**32 / (1000000 * 1000 / HZ) */
    __asm__("multu\t%2,%3"
        :"=h" (nssecs), "=l" (lo)
        :"r" (nssecs),"r" (lpj));
    __delay(nssecs);
}

__inline__ void spi_nsdelay( unsigned long delay )
{
	__nsdelay( delay, __udelay_val );	// __udelay_val defined in linux-2.4.18/include/asm-mips/delay.h
}

#if 0


/*---------------------------------------------------------
*	Name:
*		read_legerity_spi
*	Description:
*		Read the legerity slic
*	Input: TBD	
*	Ouput: TBD
*	Return: TBD
*--------------------------------------------------------*/
void read_legerity_spi(unsigned int reg, Le88xxx *Legerity)
{
	// TBD
	return;
}

/*---------------------------------------------------------
*	Name:
*		write_legerity_spi
*	Description:
*		Write the legerity slic
*	Input: TBD	
*	Ouput: TBD
*	Return: TBD
*--------------------------------------------------------*/
void write_legerity_spi(unsigned int reg, Le88xxx *data)
{
	//TBD
}

/*---------------------------------------------------------
*	Name:
*		write_legerity_spi
*	Description:
*		Write the legerity slic
*	Input: TBD	
*	Ouput: TBD
*	Return: TBD
*--------------------------------------------------------*/
void readLegerityReg(unsigned char address, Le88xxx *Legerity)
{
	//TBD 
}
/*---------------------------------------------------------
*	Name:
*		write_legerity_spi
*	Description:
*		Write the legerity slic
*	Input: TBD	
*	Ouput: TBD
*	Return: TBD
*--------------------------------------------------------*/
void writeLegerityReg(unsigned char address, Le88xxx *data)
{
	//TBD
}

#endif


