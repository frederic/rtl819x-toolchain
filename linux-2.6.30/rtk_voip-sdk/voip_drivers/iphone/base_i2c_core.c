// move to voip_drivers/i2c.c 

#if 0
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/string.h>

#include "base_gpio.h"
#include "base_i2c_core.h"

/************************* I2C read/write function ************************/
void i2c_serial_write(i2c_dev_t* pI2C_Dev, unsigned char *data)
{
	int i;
	char j;
	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
	for (j=7;j>=0;j--) {
		__i2c_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
		__i2c_setGpioDataBit( pI2C_Dev->sdio, ((*data)>>j)&0x00000001);//write data,from MSB to LSB
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
	return;
}

unsigned char i2c_ACK(i2c_dev_t* pI2C_Dev)	
{
	int i;
	unsigned int buf;
#if 0
	__i2c_setGpioDataBit( pI2C_Dev->sdio, 0); /* fall down sdio*/
	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_IN, GPIO_INT_DISABLE);//change sdio to input
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
#endif
	//__i2c_setGpioDataBit( pI2C_Dev->sdio, 0); /* fall down sdio*/
	
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_IN, GPIO_INT_DISABLE);//change sdio to input
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
		
	__i2c_getGpioDataBit(pI2C_Dev->sdio,&buf);
	if (buf != 0) 
		printk("NO ACK\n");
	//__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_INT_DISABLE);//change sdio to output
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
	#if 0
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	#endif
	return buf;
}

void i2c_ACK_w(i2c_dev_t* pI2C_Dev, unsigned char data)	
{
	int i;

#if 0
	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
	__i2c_setGpioDataBit( pI2C_Dev->sdio, data); /* sdio send 0 for ACK, 1 for NACK*/
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
#endif
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
	//for (i=0;i<500*I2C_RATING_FACTOR;i++);
	
	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
	__i2c_setGpioDataBit( pI2C_Dev->sdio, data); /* sdio send 0 for ACK, 1 for NACK*/

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

	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_IN, GPIO_INT_DISABLE);//change sdio to input
	__i2c_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
	#if 0
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	#endif
	return;
}

void i2c_serial_read(i2c_dev_t* pI2C_Dev, unsigned short int *data)
{
	int i;
	char j;
	unsigned int buf;
	
	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_IN, GPIO_INT_DISABLE);//change sdio to input
	for (j=7;j>=0;j--) {
		__i2c_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
		//delay 2 us.
		#ifdef __kernel_used__
		udelay(2*I2C_RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<2000*I2C_RATING_FACTOR;i++);
		#endif
		__i2c_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
		__i2c_getGpioDataBit( pI2C_Dev->sdio, &buf);//read data,from MSB to LSB
		*data |= (buf<<j); 
		//delay 1 us.
		#ifdef __kernel_used__
		udelay(1*I2C_RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<1000*I2C_RATING_FACTOR;i++);
		#endif
	}
	__i2c_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output	
	return;
}	

void i2c_start_condition(i2c_dev_t* pI2C_Dev)
{
	int i;

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
}

void i2c_stop_condition(i2c_dev_t* pI2C_Dev)
{
	int i;

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
}
#endif

