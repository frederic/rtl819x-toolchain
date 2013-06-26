#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/string.h>

#include "base_gpio.h"
#include "../i2c.h"
#include "base_i2c_ALC5621.h"


/*
@func int | _rtl865xC_i2c_rawWrite_alc5621 | Write several bits to device
@parm i2c_dev_t* | pI2C_Dev | Structure containing device information
@parm unsigned char* | pDEV_ID | i2c id address
@parm unsigned char* | pReg | i2c register address
@parm unsigned short int* | pData | i2c data
@comm
*/
static void __i2c_rawWrite_alc5621( i2c_dev_t* pI2C_Dev, const unsigned char *pDEV_ID, unsigned char *pReg, unsigned short int *pData)
{
	int i;
	//char j;
	//unsigned int buf;
	unsigned char dev_id, reg, data_hibyte, data_lowbyte;

	if ((pData == NULL) || (pDEV_ID == NULL) || (pReg == NULL)) {
		printk("Wrong I2C write function\n");
		return;
	}

start_condition:

	dev_id = (*pDEV_ID<<1) & 0xfe; //shift DEV_ID 1-bit left and unset in writting operation
	reg = *pReg;
	data_hibyte =(unsigned char) (*pData>>8);
	data_lowbyte =(unsigned char) (*pData & 0xff);
#if 1
	i2c_start_condition( pI2C_Dev );
#else
	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
	__i2c_setGpioDataBit( pI2C_Dev->sdio, 1); /* raise sdio*/
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	__i2c_setGpioDataBit( pI2C_Dev->sdio, 0); /* fall down sdio*//*start condition*/
	//delay 2 us.
	#ifdef __kernel_used__
	udelay(2*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<2000*I2C_RATING_FACTOR;i++);
	#endif
#endif

	i2c_serial_write(pI2C_Dev,&dev_id);//write pDEV_ID,from MSB to LSB
	if (i2c_ACK(pI2C_Dev) != 0)
		goto start_condition;
	
	i2c_serial_write(pI2C_Dev,&reg);//write pReg,from MSB to LSB
	if (i2c_ACK(pI2C_Dev) != 0)
		goto start_condition;
	
	i2c_serial_write(pI2C_Dev,&data_hibyte);//write pData(hibtye),from MSB to LSB (bit15 - bit 8)
	if (i2c_ACK(pI2C_Dev) != 0)
		goto start_condition;

	i2c_serial_write(pI2C_Dev,&data_lowbyte);//write pData(lowbtye),from MSB to LSB (bit7 - bit 0)
	if (i2c_ACK(pI2C_Dev) != 0)
		goto start_condition;

#if 1
	i2c_stop_condition( pI2C_Dev );
#else
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
	__i2c_setGpioDataBit( pI2C_Dev->sdio, 1); /* raise sdio*//*stop condition*/
#endif

	return;	
}	


/*
@func int | _rtl865xC_i2c_rawRead | Read several bits from device
@parm i2c_dev_t* | pI2C_Dev | Structure containing device information
@parm unsigned char* | pDEV_ID | i2c id address
@parm unsigned char* | pReg | i2c register address
@parm unsigned short int* | pData | i2c data
@comm
*/
static void __i2c_rawRead_alc5621( i2c_dev_t* pI2C_Dev, const unsigned char *pDEV_ID, unsigned char *pReg, unsigned short int *pData)
{
	int i;
	unsigned short int buf;
	unsigned char dev_id, reg;
	if ((pData == NULL) || (pDEV_ID == NULL) || (pReg == NULL)) {
		printk("Wrong I2C Read function\n");
		return;
	}

start_condition_read:


	//dev_id = (*pDEV_ID<<1) | 0x01;	//shift DEV_ID 1-bit left and set bit0 in reading operation
	dev_id = (*pDEV_ID<<1) & 0xfe; //shift DEV_ID 1-bit left and unset in writting operation
	reg = *pReg;
	*pData = 0;
#if 1
	i2c_start_condition( pI2C_Dev );
#else
	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
	__i2c_setGpioDataBit( pI2C_Dev->sdio, 1); /* raise sdio*/
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	__i2c_setGpioDataBit( pI2C_Dev->sdio, 0); /* fall down sdio*//*start condition*/
	//delay 2 us.
	#ifdef __kernel_used__
	udelay(2*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<2000*I2C_RATING_FACTOR;i++);
	#endif
#endif
	
	i2c_serial_write(pI2C_Dev,&dev_id);//write pDEV_ID,from MSB to LSB
	if (i2c_ACK(pI2C_Dev) != 0)
		goto start_condition_read;

	i2c_serial_write(pI2C_Dev,&reg);//write pReg,from MSB to LSB
	if (i2c_ACK(pI2C_Dev) != 0)
		goto start_condition_read;

#if 1
	i2c_start_condition( pI2C_Dev );
#else
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif

	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
	__i2c_setGpioDataBit( pI2C_Dev->sdio, 1); /* raise sdio*/
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	__i2c_setGpioDataBit( pI2C_Dev->sdio, 0); /* fall down sdio*//*start condition*/
	//delay 2 us.
	#ifdef __kernel_used__
	udelay(2*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<2000*I2C_RATING_FACTOR;i++);
	#endif
#endif

	dev_id = (*pDEV_ID<<1) | 0x01;	//shift DEV_ID 1-bit left and set bit0 in reading operation

	i2c_serial_write(pI2C_Dev,&dev_id);//write pDEV_ID,from MSB to LSB
	if (i2c_ACK(pI2C_Dev) != 0)
		goto start_condition_read;

	buf=0;		//init buf data to 0
	i2c_serial_read(pI2C_Dev,&buf);//read high byte data from device
	i2c_ACK_w(pI2C_Dev, 0);	//write ACK

	i2c_serial_read(pI2C_Dev,pData);//read low byte data from device
	i2c_ACK_w(pI2C_Dev, 1);	//write NACK

	*pData = *pData | (buf <<8);

#if 1	
	i2c_stop_condition( pI2C_Dev );
#else
	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	__i2c_setGpioDataBit( pI2C_Dev->sdio, 0); /* fall down sdio*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	__i2c_setGpioDataBit( pI2C_Dev->sdio, 1); /* raise sdio*//*stop condition*/
#endif

	return;	

}

/************************* I2C data struct ****************************************/
i2c_dev_t i2c_dev;

//-------------------  I2C API ------------------------------//

void init_i2c_gpio(void)
{	
	//printk("( GPIO %s )  ", GPIO_I2C );
	i2c_dev.sclk = I2C_SCLK_PIN;
	i2c_dev.sdio = I2C_SDIO_PIN;
	
	__i2c_initGpioPin(i2c_dev.sclk, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	__i2c_initGpioPin(i2c_dev.sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	printk("For I2C port init=> OK !\n");
	return;	
}

unsigned short int read_ALC5621(unsigned char Reg)
{
	unsigned short int buf;
	const unsigned char dev_id = 0x1A;
#ifdef _ALC5621_CLI_PROTECT	
	unsigned long flags;
	save_flags(flags); cli();
#endif
	__i2c_rawRead_alc5621(&i2c_dev,&dev_id,&Reg,&buf);
#ifdef _ALC5621_CLI_PROTECT	
	restore_flags(flags);
#endif	
	return buf;
}

void write_ALC5621(unsigned char Reg,unsigned short int data)
{
	const unsigned char dev_id = 0x1A;
#ifdef _ALC5621_CLI_PROTECT	
	unsigned long flags;
	save_flags(flags); cli();
#endif
	__i2c_rawWrite_alc5621(&i2c_dev,&dev_id,&Reg,&data);
#ifdef _ALC5621_CLI_PROTECT	
	restore_flags(flags);
#endif	
	return;
}


unsigned short int read_ALC5621_hidden(unsigned char index)
{

	write_ALC5621(0x6a, index);

	return read_ALC5621(0x6c);
}

void write_ALC5621_hidden(unsigned char index, unsigned short int data)
{
	write_ALC5621(0x6a, index);

	write_ALC5621(0x6c, data);
}

