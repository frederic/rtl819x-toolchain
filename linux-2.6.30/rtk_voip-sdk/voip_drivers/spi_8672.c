/*****************************************************************************
*	SPI driver shared with SPI flash	
*
*	Initial version: 			2008/04/15
*
*
*****************************************************************************/
#include <linux/config.h>
#include <linux/sched.h>	/* jiffies is defined */
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/string.h>
#include "../include/rtk_voip.h"
#include "spi.h"
#include "spi_8672.h"

//#define SHARE_SPI
#define SPI_VOIP_MIN_DELAY 10000


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
#ifdef CONFIG_BGA_PIN_DEF    
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
#ifdef  CONFIG_BGA_PIN_DEF      
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
	uint8 buf;
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
#if defined(CONFIG_BGA_PIN_DEF) || defined(CONFIG_6166_IAD_SILAB3217X)
/* Added for 301 pin BGA*/
	gpioConfig(PIN_RELAY, 2); // config GPA4 as output pin
	gpioSet(PIN_RELAY);
#endif	
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

#ifdef CONFIG_BGA_PIN_DEF
	//GPA5_SELVOIPCSB
    *((volatile unsigned int *)(0xb8003310)) |= (1<<9);
    printk("*****  PCM I/O pad driving(bit 9) reg 0x%08x  *****\n",     *((volatile unsigned int *)(0xb8003310)));  //enhance pcm i/o driving from 2mA to 4mA
    *((volatile unsigned int *)(RT8672_PINMUX)) |= (1 << 26);	

    //!!GPC456 selvoip
	*((volatile unsigned int *)(RT8672_PINMUX)) |= (1 << 25);	
	//enable VoIP
	*((volatile unsigned int *)(RT8672_PINMUX)) |= (1 << 8);	
#endif 
#ifdef CONFIG_6166_IAD_SILAB3217X    
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
unsigned char share_spi_read_nodaisy(unsigned int reg, unsigned char *data)
{
	unsigned int		val,i,spicnr;
	uint8 buf;
	rtl_8672_reg_t	*spi_reg=get_spi_reg();

	return rtl8672_share_spi_read(0, 0, (reg | 0x80), 0, data );
}


void share_spi_write_nodaisy(unsigned int reg, unsigned int data)
{
	unsigned int		val,i;
	uint8 buf;
	rtl_8672_reg_t	*spi_reg=get_spi_reg();

	rtl8672_share_spi_write(0, 0, reg, 0, data);
}

/*---------------------------------------------------------
*	Name:
*		_rtl867x_spi_rawRead
*	Description:
*		Read th spi raw data.
*		This function is kept for compatibility.
*	Input:	
*		*pDev - the specified spi device to read.
*		bits	- the number of bits to read
*	Ouput:
*		pData - the read data.
*	Return:	0 on SUCCESS
*--------------------------------------------------------*/
int32 _rtl867x_spi_rawRead( rtl_spi_dev_t* pDev, void* pData, int32 bits )
{
	//This function can't be supported 
	//must rewrite ReadReg of 3226 api
	return 0;
}
/*---------------------------------------------------------
*	Name:
*		_rtl867x_spi_rawRead
*	Description:
*		write raw data to spi.
*		This function is kept for compatibility.
*	Input:	
*		*pDev - the specified spi device to write.
*		pData - the data to write.
*		bits	- the number of bits to write
*	Ouput: none
*	Return:	0 on SUCCESS
*--------------------------------------------------------*/
int32 _rtl867x_spi_rawWrite( rtl_spi_dev_t* pDev, void* pData, int32 bits )
{
	//This function can't be supported 
	//must rewrite WriteReg of 3226 api
	return 0;
}

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
 unsigned char read_spi(unsigned int chid, unsigned int reg)
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
 void write_spi(unsigned int chid, unsigned int reg, unsigned int data)
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
unsigned char read_spi_daa(int chid, unsigned int reg)
{
	uint8 buf;
	unsigned long flags;
	rtl_8672_reg_t	*spi_reg=get_spi_reg();

	rtl8672_share_spi_read(1, 1, 0x60, reg , &buf);
	return buf;
}

static void write_spi_daa(int chid, unsigned int reg, unsigned int data)
{
	unsigned long flags;
	rtl_8672_reg_t	*spi_reg = get_spi_reg();

	rtl8672_share_spi_write(1, 1, 0x20  , reg,  data);
	return;
}

extern unsigned char R_reg(unsigned char chid, unsigned char address);
extern void W_reg(unsigned char chid, unsigned char regaddr, unsigned char data);
unsigned char readDAAReg(int chid, unsigned char address)
{
	return R_reg( chid, address);
	//return read_spi_daa(chid, address);
}


void writeDAAReg(int chid, unsigned char address, unsigned char data)
{
	return W_reg(chid, address, data);
	//write_spi_daa(chid, address, data);
}




