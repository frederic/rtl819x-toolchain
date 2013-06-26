#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/rtl8186.h>

#define SIO_CNR		0x80	//SIO control register
#define SIO_STR		0x84	//SIO status register
#define SIO_CKDIV	0x88	//SIO clock divisor register
#define SIO_ADDR	0x8C	//SIO address register
#define SIO_DATA0	0x90	//SIO data register0
#define SIO_DATA1	0x94	//SIO data register1
#define SIO_DATA2	0x98	//SIO data register2
#define SIO_DATA3	0x9C	//SIO data register3

#define SIO_SIZE0	0x0000
#define SIO_SIZE1	0x0010
#define	SIO_SIZE2	0x0020
#define SIO_SIZE3	0x0030
#define SIO_SIZE4	0x0040
#define SIO_SIZE5	0x0050
#define SIO_ADDMODE00	0x0000
#define SIO_ADDMODE01	0x0004
#define SIO_ADDMODE02	0x0008
#define SIO_READISR_EN	0x0100
#define SIO_WRITEISR_EN	0x0200

#define SIO_RWC_R      	0x0001
#define SIO_RWC_W      	0x0000

#define ENABLE_SIO 	BIT(0)
#define ENGAGED_SIO	BIT(1)

#define SIO_CLK_DIV32	0x00
#define SIO_CLK_DIV64 	0x01
#define SIO_CLK_DIV128 	0x02
#define SIO_CLK_DIV256 	0x03

#define REG_POWERCTRL	0	//Power Control
#define REG_MODECTRL	1	//Mode Control
#define REG_TXPGA	2	//TXPGA
#define REG_RXPGA	3	//RXPGA
#define REG_HIDTMF	4	//High DTMF
#define REF_LODTMF	5	//Low  DTMF
#define REG_AUX		6	//AUX


#define CODEC_W		0xE2
#define CODEC_R		0xE3

#define M_DEFAULT_SET		0x2
#define M_TX_HPF_DISABLED	BIT(1)
#define M_TX_SF_DISABLED	BIT(0)
#define M_RX_HPF_ENABLED	BIT(2)
#define M_BUZZCON_ENABLED	BIT(3)
#define M_COMPAND		BIT(4)
#define M_ALAW			BIT(7)
#define M_LOOPBAK		BIT(5)
#define M_TONE_ENABLED		BIT(6)



void i2c_byte_write(unsigned int addr, unsigned int data)
{
	unsigned long i;
	
	unsigned long comm= ((0xFF & addr) << 24) | ((data & 0xFF)<< 16);
	rtl_outl(SIO_ADDR, (SIO_RWC_W | 0xE2)); // 0xE2 for write
	
	//rtl_outl(SIO_ADDR, (SIO_RWC_W | (CODEC_W << 1)));
	rtl_outl(SIO_DATA0, comm);//
	rtl_outl(SIO_CNR, (SIO_SIZE1 | SIO_ADDMODE00 | ENABLE_SIO | ENGAGED_SIO));//one word address
	
	for(i=0;i<0xFFF;i++)
	{
		if( (rtl_inl(SIO_CNR) & 0x02)==0) 
		{
			break;
		}	
	}
}

void i2c_read_multi(int addr, int byte_count,volatile unsigned long *data0)
{
	unsigned long i;
	
	rtl_outl(SIO_ADDR,SIO_RWC_R | 0xE2 | (addr<<8) ); // 0xE3 read
	//rtl_outl(SIO_ADDR, (SIO_RWC_R | (CODEC_R << 1) | (addr << 8)));
	rtl_outl(SIO_CNR,(byte_count<<4)|SIO_ADDMODE01|ENABLE_SIO | ENGAGED_SIO);//one word address
	
	for(i=0;i<0xFFF;i++)
	{
		if( (rtl_inl(SIO_CNR) & 0x02)==0)
		{
			break;
		}
	}
	
	*data0 =rtl_inl(SIO_DATA0);
}

//If you modify these, you must modify pcmctrl.c too.
#define LINEAR	0
#define U_LAW	1
#define A_LAW	2

void codec_init(unsigned int mode)
{
	printk("Initialize I2C interface.\n");
	rtl_outl(SIO_CNR, ENABLE_SIO);		//enable SIO
	rtl_outl(SIO_CKDIV, SIO_CLK_DIV256);	//set clock speed
	rtl_outl(SIO_STR,0xFFFFFFFF);		//write 1 to clear all pending flags.

	unsigned int modevalue = M_DEFAULT_SET;

	switch(mode)
	{
		case LINEAR:
			printk("This is linear mode.\n");
			break;
		case U_LAW:
			modevalue  = (modevalue  | M_COMPAND);
			printk("This is u-Law mode.\n");
			break;
		case A_LAW:
			modevalue  = (modevalue  | M_COMPAND | M_ALAW);	
			printk("This is A-Law mode.\n");
			break;
	}
	
	printk("%s - I2C Mode is 0x%X\n", __FUNCTION__, modevalue);

	i2c_byte_write(REG_MODECTRL, modevalue);
	
	unsigned int i;
	for(i = 0 ; i < 0xFFF ; i++)
	{
	}

	unsigned long i2c_data0 = 0;
//	i2c_read_multi(1, 1, &i2c_data0);

//	printk("Register0 = 0x%X\n", i2c_data0);

	unsigned int j;
	
	for(j=0;j<7;j++) {
		i2c_read_multi(j,3,&i2c_data0);
		printk("reg%d = %x\n",j,i2c_data0);
	}


	rtl_outl(SIO_CNR, 0);
	rtl_outl(SIO_STR, 0);
}

//dump register value.
unsigned int dreg(unsigned int reg)
{
	unsigned long data = 0;
	i2c_read_multi(reg, 3, &data);

	return (data & 0xFF000000);
}

