#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/string.h>

#include "base_gpio.h"
#include "../i2c.h"
#include "base_i2c_WM8510.h"


/************************* I2C read/write function ************************/

/*
@func int | __i2c_rawWrite_WM8510 | Write several bits to device
@parm i2c_dev_t* | pLCM_Dev | Structure containing device information
@comm
*/
static void __i2c_rawWrite_WM8510( i2c_dev_t* pI2C_Dev, unsigned char *pDEV_ID, unsigned char *pReg, unsigned short int *pData)
{
	int i;
	//char j;
	//unsigned int buf;
	unsigned char dev_id, reg, data;
	if ((pData == NULL) || (pDEV_ID == NULL) || (pReg == NULL)) {
		printk("Wrong I2C write function\n");
		return;
	}
	
start_condition:
	
	//*pDEV_ID <<= 1; //shift DEV_ID 1-bit left and set in writting operation
	dev_id = (*pDEV_ID<<1) | 0; //shift DEV_ID 1-bit left and unset in writting operation
	reg = (*pReg<<1) | (*pData>>8);
	data = *pData & 0xff;
#if 1
	i2c_start_condition( pI2C_Dev );
#else
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
	
#if 0	
	for (j=7;j>=0;j--) {
		__i2c_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
		__i2c_setGpioDataBit( pI2C_Dev->sdio, ((*pDEV_ID)>>j)&0x00000001);//write DEV_ID,from MSB to LSB
		//delay 2 us.
		#ifdef __kernel_used__
		udelay(2*I2C_RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<2000*I2C_RATING_FACTOR;i++);
		#endif
		__i2c_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
		//delay 1 us.
		#ifdef __kernel_used__
		udelay(1*I2C_RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<1000*I2C_RATING_FACTOR;i++);
		#endif
		if (j == 0) {//check ACK
			__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_IN, GPIO_INT_DISABLE);//change sdio to input
			__i2c_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
			//delay 1 us.
			#ifdef __kernel_used__
			udelay(1*I2C_RATING_FACTOR);
			#endif
			#ifdef __test_program_used__
			for (i=0;i<1000*I2C_RATING_FACTOR;i++);
			#endif
			__i2c_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
			__i2c_getGpioDataBit(pI2C_Dev->sdio,&buf);
			if (buf != 0) {
				printk("NO ACK\n");
				__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
				//goto label;
			}
		__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
		}
	}	
#endif
	i2c_serial_write(pI2C_Dev,&dev_id);//write pDEV_ID,from MSB to LSB
	if (i2c_ACK(pI2C_Dev) != 0)
		goto start_condition;
	
	i2c_serial_write(pI2C_Dev,&reg);//write pReg,from MSB to LSB
	if (i2c_ACK(pI2C_Dev) != 0)
		goto start_condition;
	
	i2c_serial_write(pI2C_Dev,&data);//write pData,from MSB to LSB
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
@func int | __i2c_rawRead_WM8510 | Read several bits from device
@parm i2c_dev_t* | pLCM_Dev | Structure containing device information
@comm
*/
static void __i2c_rawRead_WM8510( i2c_dev_t* pI2C_Dev, unsigned char *pDEV_ID, unsigned char *pReg, unsigned short int *pData)
{
	int i;
	char j;
	unsigned int buf;
	unsigned char dev_id, reg;
	if ((pData == NULL) || (pDEV_ID == NULL) || (pReg == NULL)) {
		printk("Wrong I2C Read function\n");
		return;
	}

start_condition_read:
	
	////*pDEV_ID = (*pDEV_ID<<1) | 0x01; //shift DEV_ID 1-bit left
	dev_id = (*pDEV_ID<<1) | 0x01;
	reg = (*pReg<<1);
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

#if 1	
	i2c_serial_write_byte( reg );
#else
	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
	
	for (j=7;j>=0;j--) {
		__i2c_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
		__i2c_setGpioDataBit( pI2C_Dev->sdio, (reg>>j)&0x00000001);//write pReg,from MSB to LSB
		//delay 2 us.
		#ifdef __kernel_used__
		udelay(2*I2C_RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<2000*I2C_RATING_FACTOR;i++);
		#endif
		__i2c_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
		//delay 1 us.
		#ifdef __kernel_used__
		udelay(1*I2C_RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<1000*I2C_RATING_FACTOR;i++);
		#endif
	}
#endif
	
	if (i2c_ACK(pI2C_Dev) != 0)
		goto start_condition_read;
	//__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_IN, GPIO_INT_DISABLE);//change sdio to input
	//__i2c_getGpioDataBit( pI2C_Dev->sdio, &buf);//read data,from LSB of pReg
	//*pData |= (buf<<8);
	
	i2c_serial_read(pI2C_Dev,pData);//read data from device

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


/************************* I2C data struct ****************************************/
static i2c_dev_t i2c_dev;

//-------------------  I2C API ------------------------------//

void init_i2c_gpio(void)
{	
	printk("( GPIO %s )  ", GPIO_I2C );
	i2c_dev.sclk = SCLK_PIN;
	i2c_dev.sdio = SDIO_PIN;
	
	__i2c_initGpioPin(i2c_dev.sclk, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	__i2c_initGpioPin(i2c_dev.sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	printk("For I2C port init=> OK !\n");
	return;	
}


unsigned short int read_WM8510(unsigned char Reg)
{
	unsigned short int buf;
	const unsigned char dev_id = 0x1A;
#ifdef _WM8510_CLI_PROTECT	
	unsigned long flags;
	save_flags(flags); cli();
#endif
	__i2c_rawRead_WM8510(&i2c_dev,&dev_id,&Reg,&buf);
#ifdef _WM8510_CLI_PROTECT	
	restore_flags(flags);
#endif	
	return buf;
}	

void write_WM8510(unsigned char Reg,unsigned short int data)
{
	const unsigned char dev_id = 0x1A;
#ifdef _WM8510_CLI_PROTECT	
	unsigned long flags;
	save_flags(flags); cli();
#endif
	__i2c_rawWrite_WM8510(&i2c_dev,&dev_id,&Reg,&data);
#ifdef _WM8510_CLI_PROTECT	
	restore_flags(flags);
#endif	
	return;
}
