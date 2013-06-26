// We don't use these content anymore. 
#if 0
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/string.h>

#include "base_gpio.h"
#include "keypad_map.h"
#include "lcm.h"
unsigned char rtl8186_gpio_async = 0;	//0: GPIOD/F used ,1: GPIOC/E used

/************************** GPIO Driver**********************************/
/*
@func int | _getGpio | abstract GPIO registers 
@parm enum GPIO_FUNC | func | control/data/interrupt register
@parm enum GPIO_PORT | port | GPIO port
@parm unsigned int | pin | pin number
@rvalue unsigned int | value
@comm
This function is for internal use only. You don't need to care what register address of GPIO is.
This function abstracts these information.
*/
static unsigned int _getGpio( enum GPIO_FUNC func, enum GPIO_PORT port, unsigned int pin )
{
		
#if _GPIO_DEBUG_ >= 4	
	printk("[%s():%d] func=%d port=%d pin=%d\n", __FUNCTION__, __LINE__, func, port, pin );
#endif

	switch( func )
	{
				
		case GPIO_FUNC_DIRECTION:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioDirectionRead[port]=0x%08x  bitStartGpioDirectionRead[port]=%d\n", __FUNCTION__, __LINE__, regGpioDirectionRead[port], bitStartGpioDirectionRead[port] );
#endif
			if ( REG32(regGpioDirectionRead[port]) & ( (unsigned int)1 << (pin+bitStartGpioDirectionRead[port]) ) )
				return 1;
			else
				return 0;
			break;
			
		case GPIO_FUNC_DATA:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioDataRead[port]=0x%08x  bitStartGpioDataRead[port]=%d\n", __FUNCTION__, __LINE__, regGpioDataRead[port], bitStartGpioDataRead[port] );
#endif
			if ( REG32(regGpioDataRead[port]) & ( (unsigned int)1 << (pin+bitStartGpioDataRead[port]) ) )
				return 1;
			else
				return 0;
			break;
			
		case GPIO_FUNC_INTERRUPT_ENABLE:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioInterruptEnableRead[port]=0x%08x  bitStartGpioInterruptEnableRead[port]=%d\n", __FUNCTION__, __LINE__, regGpioInterruptEnableRead[port], bitStartGpioInterruptEnableRead[port] );
#endif
			if ( REG32(regGpioInterruptEnableRead[port]) & ( (unsigned int)1 <<(pin+bitStartGpioInterruptEnableRead[port]) ))
				return 1;
			else
				return 0;
			break;

		case GPIO_FUNC_INTERRUPT_STATUS:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioInterruptStatusRead[port]=0x%08x  bitStartGpioInterruptEnableRead[port]=%d\n", __FUNCTION__, __LINE__, regGpioInterruptStatusRead[port], bitStartGpioInterruptStatusRead[port] );
#endif
			if ( REG32(regGpioInterruptStatusRead[port]) & ( (unsigned int)1 << (pin+bitStartGpioInterruptStatusRead[port]) ) )
				return 1;
			else
				return 0;
			break;
			
		case GPIO_FUNC_MAX:
			break;
	}
	return 0xffffffff;
}


/*
@func int | _setGpio | abstract GPIO registers 
@parm enum GPIO_FUNC | func | control/data/interrupt register
@parm enum GPIO_PORT | port | GPIO port
@parm unsigned int | pin | pin number
@parm unsigned int | data | value
@rvalue NONE
@comm
This function is for internal use only. You don't need to care what register address of GPIO is.
This function abstracts these information.
*/
static void _setGpio( enum GPIO_FUNC func, enum GPIO_PORT port, unsigned int pin, unsigned int data )
{

#if _GPIO_DEBUG_ >= 4
	printk("[%s():%d] func=%d port=%d pin=%d data=%d\n", __FUNCTION__, __LINE__, func, port, pin, data );
#endif

	switch( func )
	{
		
		case GPIO_FUNC_DIRECTION:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioDirectionWrite[port]=0x%08x  bitStartGpioDirectionWrite[port]=%d\n", __FUNCTION__, __LINE__, regGpioDirectionWrite[port], bitStartGpioDirectionWrite[port] );
#endif
			if ( data )
				REG32(regGpioDirectionWrite[port]) |= (unsigned int)1 << (pin+bitStartGpioDirectionWrite[port]);
			else
				REG32(regGpioDirectionWrite[port]) &= ~((unsigned int)1 << (pin+bitStartGpioDirectionWrite[port]));
			break;

		case GPIO_FUNC_DATA:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioDataWrite[port]=0x%08x  bitStartGpioDataWrite[port]=%d\n", __FUNCTION__, __LINE__, regGpioDataWrite[port], bitStartGpioDataWrite[port] );
#endif
	if (rtl8186_gpio_async == 0) {
			if ( data )
				REG32(regGpioDataWrite[port]) = (REG32(regGpioDataWrite[port])>>16) | ((unsigned int)1 << (pin+bitStartGpioDataWrite[port]));
			else
				REG32(regGpioDataWrite[port]) = (REG32(regGpioDataWrite[port])>>16) & (~((unsigned int)1 << (pin+bitStartGpioDataWrite[port])));
			break;
	} else if (rtl8186_gpio_async == 1) {
			if ( data )
				REG32(regGpioDataWrite[port]) |= (unsigned int)1 << (pin+bitStartGpioDataWrite[port]);
			else
				REG32(regGpioDataWrite[port]) &= ~((unsigned int)1 << (pin+bitStartGpioDataWrite[port]));
			break;
	}		
		case GPIO_FUNC_INTERRUPT_ENABLE:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioInterruptEnableWrite[port]=0x%08x  bitStartGpioInterruptEnableWrite[port]=%d\n", __FUNCTION__, __LINE__, regGpioInterruptEnableWrite[port], bitStartGpioInterruptEnableWrite[port] );
#endif
			if (data)
				REG32(regGpioInterruptEnableWrite[port]) |= (unsigned int)1 << (pin+bitStartGpioInterruptEnableWrite[port]);
			else
				REG32(regGpioInterruptEnableWrite[port]) &= ~((unsigned int)1 << (pin+bitStartGpioInterruptEnableWrite[port]));
			break;

		case GPIO_FUNC_INTERRUPT_STATUS:
#if _GPIO_DEBUG_ >= 5
			printk("[%s():%d] regGpioInterruptStatusWrite[port]=0x%08x  bitStartGpioInterruptStatusWrite[port]=%d\n", __FUNCTION__, __LINE__, regGpioInterruptStatusWrite[port], bitStartGpioInterruptStatusWrite[port] );
#endif
			if ( data )
				REG32(regGpioInterruptStatusWrite[port]) |= (unsigned int)1 << (pin+bitStartGpioInterruptStatusWrite[port]);
			else
				REG32(regGpioInterruptStatusWrite[port]) &= ~((unsigned int)1 << (pin+bitStartGpioInterruptStatusWrite[port]));
			break;

		case GPIO_FUNC_MAX:
			break;
	}
}


/*************************************************************************************/
/*
@func int | _rtl8186_initGpioPin | Initiate a specifed GPIO port.
@parm unsigned int | gpioId | The GPIO port that will be configured
@parm enum GPIO_DIRECTION | direction | Data direction, in or out
@parm enum GPIO_INTERRUPT_TYPE | interruptEnable | Interrupt mode
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
This function is used to initialize GPIO port.
*/
static int _rtl8186_initGpioPin( unsigned int gpioId, enum GPIO_DIRECTION direction, 
                                           enum GPIO_INTERRUPT_TYPE interruptEnable )
 {
	unsigned int port = GPIO_PORT( gpioId );
	unsigned int pin = GPIO_PIN( gpioId );

	if ( port >= GPIO_PORT_MAX ) return FAILED;
	//if ( pin >= 8 ) return FAILED;	/* This judgment does not suit to RTL8186. GPIO PIN definition is not the same as RTL865x. */

		
	_setGpio( GPIO_FUNC_DIRECTION, port, pin, direction );

	_setGpio( GPIO_FUNC_INTERRUPT_ENABLE, port, pin, interruptEnable );
	

	return SUCCESS;
}


/*
@func int | _rtl8186_getGpioDataBit | Get the bit value of a specified GPIO ID.
@parm unsigned int | gpioId | GPIO ID
@parm unsigned int* | data | Pointer to store return value
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
static int _rtl8186_getGpioDataBit( unsigned int gpioId, unsigned int* pData )
{
	unsigned int port = GPIO_PORT( gpioId );
	unsigned int pin = GPIO_PIN( gpioId );

	if ( port >= GPIO_PORT_MAX ) return FAILED;
	//if ( pin >= 8 ) return FAILED;
	if ( pData == NULL ) return FAILED;

	*pData = _getGpio( GPIO_FUNC_DATA, port, pin );
	
#if _GPIO_DEBUG_ >= 3
	printk("[%s():%d] (port=%d,pin=%d)=%d\n", __FUNCTION__, __LINE__, port, pin, *pData );
#endif


	return SUCCESS;
}


/*
@func int | _rtl8186_setGpioDataBit | Set the bit value of a specified GPIO ID.
@parm unsigned int | gpioId | GPIO ID
@parm unsigned int | data | Data to write
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
static int _rtl8186_setGpioDataBit( unsigned int gpioId, unsigned int data )
{
	unsigned int port = GPIO_PORT( gpioId );
	unsigned int pin = GPIO_PIN( gpioId );

	if ( port >= GPIO_PORT_MAX ) return FAILED;
	//if ( pin >= 8 ) return FAILED;
#if 0
	if ( _getGpio( GPIO_FUNC_DIRECTION, port, pin ) == GPIO_DIR_IN ) return FAILED; /* read only */  
#endif

#if _GPIO_DEBUG_ >= 3
	printk("[%s():%d] (port=%d,pin=%d)=%d\n", __FUNCTION__, __LINE__, port, pin, (data&0x01) );
#endif

	_setGpio( GPIO_FUNC_DATA, port, pin, (data&0x01) );


	return SUCCESS;
}

/************************************* Set GPIO Pin to LCM ***********************************************************/
/*
@func int | _rtl8186_lcm_data_port_init 
@parm rtl8186_lcm_dev_t* | pLCM_Dev | Structure to store device information
*/
static void _rtl8186_lcm_data_port_init( rtl8186_lcm_dev_t* pLCM_Dev,unsigned char in_out)
{
		
	if (in_out == 1) {//config DB7~DB4 output
		_rtl8186_initGpioPin( pLCM_Dev->DB7, GPIO_DIR_OUT, GPIO_INT_DISABLE );
		_rtl8186_initGpioPin( pLCM_Dev->DB6, GPIO_DIR_OUT, GPIO_INT_DISABLE );
		_rtl8186_initGpioPin( pLCM_Dev->DB5, GPIO_DIR_OUT, GPIO_INT_DISABLE );
		_rtl8186_initGpioPin( pLCM_Dev->DB4, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	} else if (in_out ==0) {//config DB7~DB4 input	
		_rtl8186_initGpioPin( pLCM_Dev->DB7, GPIO_DIR_IN, GPIO_INT_DISABLE );
		_rtl8186_initGpioPin( pLCM_Dev->DB6, GPIO_DIR_IN, GPIO_INT_DISABLE );
		_rtl8186_initGpioPin( pLCM_Dev->DB5, GPIO_DIR_IN, GPIO_INT_DISABLE );
		_rtl8186_initGpioPin( pLCM_Dev->DB4, GPIO_DIR_IN, GPIO_INT_DISABLE );
	}	
	return;
}


/*
@func void | _rtl8186_lcm_rawRead | Read several bits from LCM
@parm rtl8186_lcm_dev_t* | pLCM_Dev | Structure containing device information
@parm unsigned int* | pData | Pointer to store data
@parm unsigned int | bits | Number bits of data wanted to read
@comm
*/
//rs_flag: 0->instruction register, 1->data register.
static void _rtl8186_lcm_rawRead( rtl8186_lcm_dev_t* pLCM_Dev, void* pData, int bits, enum REGISTER_SELECT rs_flag)
{
	unsigned char* pch = pData;
	unsigned int buf,i;
	unsigned char j=0;
	
	
	_rtl8186_lcm_data_port_init(pLCM_Dev,0);//config DB7~DB4 input
	*pch = 0;
	for (j=0;j<2;j++) {
		if (rs_flag == INSTRUCT_REGISTER)
			_rtl8186_setGpioDataBit( pLCM_Dev->reg_select, 0); /* fall down reg_select*/
		else if (rs_flag == DATA_REGISTER)
			_rtl8186_setGpioDataBit( pLCM_Dev->reg_select, 1); /* raise reg_select*/
		_rtl8186_setGpioDataBit( pLCM_Dev->read_write, 1); /* raise read_write*/
		//delay 1 us.
		#ifdef __kernel_used__
		udelay(1*RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<1000*RATING_FACTOR;i++);
		#endif
		_rtl8186_setGpioDataBit( pLCM_Dev->enable, 1); /* raise enable*/
		//delay 0.2 us.
		#ifdef __kernel_used__
		udelay(1*RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<200*RATING_FACTOR;i++);
		#endif
		//read data from DB7~DB4
		_rtl8186_getGpioDataBit( pLCM_Dev->DB7, &buf );
		*pch |= (buf<<(7-4*j));
		_rtl8186_getGpioDataBit( pLCM_Dev->DB6, &buf );
		*pch |= (buf<<(7-4*j-1));
		_rtl8186_getGpioDataBit( pLCM_Dev->DB5, &buf );
		*pch |= (buf<<(7-4*j-2));
		_rtl8186_getGpioDataBit( pLCM_Dev->DB4, &buf );
		*pch |= (buf<<(7-4*j-3));
		//delay 0.25 us
		#ifdef __kernel_used__
		udelay(1*RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<250*RATING_FACTOR;i++);
		#endif
		_rtl8186_setGpioDataBit( pLCM_Dev->enable, 0); /* fall down enable*/
		//delay 0.25 us
		#ifdef __kernel_used__
		udelay(1*RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<250*RATING_FACTOR;i++);
		#endif
		if (rs_flag == INSTRUCT_REGISTER)
			_rtl8186_setGpioDataBit( pLCM_Dev->reg_select, 1); /* raise reg_select*/
		else if (rs_flag == DATA_REGISTER)
			_rtl8186_setGpioDataBit( pLCM_Dev->reg_select, 0); /* fall down reg_select*/
	}
	_rtl8186_lcm_data_port_init(pLCM_Dev,1);//config DB7~DB4 output
	return;
}


/*
@func int | _rtl8186_lcm_rawWrite | Write several bits from LCM
@parm rtl8186_lcm_dev_t* | pLCM_Dev | Structure containing device information
@parm unsigned int* | pData | Pointer to data
@parm unsigned int | bits | Number bits of data wanting to write
@comm
*/
//rs_flag: 0->instruction register, 1->data register.
//bf_flag: 0->read busy flag(DB7), 1->don't read busy flag
//bit_interface: 0-> 4-bit data length, 1-> 8-bit data_length.
static void _rtl8186_lcm_rawWrite( rtl8186_lcm_dev_t* pLCM_Dev, void* pData, int bits ,enum REGISTER_SELECT rs_flag, enum BUSY_FLAG bf_flag,unsigned char bit_interface)
{
	unsigned char* pch = pData;
	unsigned char flag_DB7, j;
	unsigned int i;
	
	if ( pData == NULL ) {
		printk("no data will be written\n");
		return;
	}
	
	for (j=0;j<(2-bit_interface);j++) {
		if (rs_flag == INSTRUCT_REGISTER)
			_rtl8186_setGpioDataBit( pLCM_Dev->reg_select, 0); /* fall down reg_select*/
		else if (rs_flag == DATA_REGISTER)
			_rtl8186_setGpioDataBit( pLCM_Dev->reg_select, 1); /* raise reg_select*/
		_rtl8186_setGpioDataBit( pLCM_Dev->read_write, 0); /* fall down read_write*/
		//delay 1 us.
		#ifdef __kernel_used__
		udelay(1*RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<1000*RATING_FACTOR;i++);
		#endif
		_rtl8186_setGpioDataBit( pLCM_Dev->enable, 1); /* raise enable*/
		//write data to pin
		_rtl8186_setGpioDataBit( pLCM_Dev->DB7, ((*pch)>>(7-4*j))&0x00000001);
		_rtl8186_setGpioDataBit( pLCM_Dev->DB6, ((*pch)>>(7-4*j-1))&0x00000001);
		_rtl8186_setGpioDataBit( pLCM_Dev->DB5, ((*pch)>>(7-4*j-2))&0x00000001);
		_rtl8186_setGpioDataBit( pLCM_Dev->DB4, ((*pch)>>(7-4*j-3))&0x00000001);
		//delay 0.25 us
		#ifdef __kernel_used__
		udelay(1*RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<250*RATING_FACTOR;i++);
		#endif
		_rtl8186_setGpioDataBit( pLCM_Dev->enable, 0); /* fall down enable*/
		//delay 0.25 us
		#ifdef __kernel_used__
		udelay(1*RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<250*RATING_FACTOR;i++);
		#endif
		if (rs_flag == INSTRUCT_REGISTER)
			_rtl8186_setGpioDataBit( pLCM_Dev->reg_select, 1); /* raise reg_select*/
		else if (rs_flag == DATA_REGISTER)
			_rtl8186_setGpioDataBit( pLCM_Dev->reg_select, 0); /* fall down reg_select*/
		
	}
	
	if (bf_flag == READ_FLAG) {
		do{
			_rtl8186_lcm_rawRead(pLCM_Dev,&flag_DB7,bits,rs_flag);
		} while (flag_DB7&0x80);
	}	
	return;
}


/************************* LCM API ****************************************/

static rtl8186_lcm_dev_t lcm_dev;

//------------------------------------------

static void gpio_pin_config(unsigned int reg_select, unsigned int enable, 
	unsigned int read_write, unsigned int DB7, unsigned int DB6,unsigned int DB5, 
	unsigned int DB4,unsigned char in_out)
{
	
	lcm_dev.reg_select = reg_select;
	lcm_dev.enable = enable;
	lcm_dev.read_write = read_write;
	lcm_dev.DB7 = DB7;
	lcm_dev.DB6 = DB6;
	lcm_dev.DB5 = DB5;
	lcm_dev.DB4 = DB4;
	
	_rtl8186_initGpioPin(lcm_dev.reg_select, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(lcm_dev.enable, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(lcm_dev.read_write, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	
	if (in_out == 1)//config DB7~DB4 output 
		_rtl8186_lcm_data_port_init(&lcm_dev,in_out);	
	else if (in_out == 0)//config DB7~DB4 input
		_rtl8186_lcm_data_port_init(&lcm_dev,in_out);
	else
		printk("Wrong data pin type\n");
		
	return;	

}

//------------------------------------------

void init_lcm_gpio(int lcm_gpio)
{	
	printk("( GPIO %s )  ", GPIO_LCM );
	rtl8186_gpio_async = LCM_GPIO_PIN;
	if (lcm_gpio == 0)
	{
		gpio_pin_config(PIN_RS,PIN_ENABLE,PIN_R_W_,PIN_DB7,PIN_DB6,PIN_DB5,PIN_DB4,1);
		printk("For LCM port init=> OK !\n");
	}
	return;	
}

/************************* Read & Write LCM ***********************************/


unsigned char read_lcm_IR(void)
{
	unsigned char buf;
#ifdef LCM_CLI_PROTECT	
	unsigned long flags;
	save_flags(flags); cli();
#endif	
	rtl8186_gpio_async = LCM_GPIO_PIN;	
	_rtl8186_lcm_rawRead(&lcm_dev,&buf,8,INSTRUCT_REGISTER);
#ifdef LCM_CLI_PROTECT	
	restore_flags(flags);
#endif	
	return buf;
}	

void write_lcm_IR(unsigned char data,enum BUSY_FLAG bf_flag,unsigned char bit_interface)
{
#ifdef LCM_CLI_PROTECT	
	unsigned long flags;
	save_flags(flags); cli();
#endif	
	rtl8186_gpio_async = LCM_GPIO_PIN;	
	_rtl8186_lcm_rawWrite(&lcm_dev,&data,8,INSTRUCT_REGISTER,bf_flag,bit_interface);
#ifdef LCM_CLI_PROTECT	
	restore_flags(flags);
#endif	
	return;
}

unsigned char read_lcm_DR(void)
{
	unsigned char buf;
#ifdef LCM_CLI_PROTECT	
	unsigned long flags;
	save_flags(flags); cli();
#endif	
	rtl8186_gpio_async = LCM_GPIO_PIN;	
	_rtl8186_lcm_rawRead(&lcm_dev,&buf,8,DATA_REGISTER);
#ifdef LCM_CLI_PROTECT	
	restore_flags(flags);
#endif	
	return buf;
}	

void write_lcm_DR(unsigned char data,enum BUSY_FLAG bf_flag,unsigned char bit_interface)
{
#ifdef LCM_CLI_PROTECT	
	unsigned long flags;
	save_flags(flags); cli();
#endif	
	rtl8186_gpio_async = LCM_GPIO_PIN;	
	_rtl8186_lcm_rawWrite(&lcm_dev,&data,8,DATA_REGISTER,bf_flag,bit_interface);
#ifdef LCM_CLI_PROTECT	
	restore_flags(flags);
#endif	
	return;
}

/************************* I2C read/write function ************************/
#if defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_WM8510)
static void serial_write(rtl8186_i2c_dev_t* pI2C_Dev, unsigned char *data)
{
	int i;
	char j;
	for (j=7;j>=0;j--) {
		_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
		_rtl8186_setGpioDataBit( pI2C_Dev->sdio, ((*data)>>j)&0x00000001);//write data,from MSB to LSB
		//delay 2 us.
		#ifdef __kernel_used__
		udelay(2*I2C_RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<2000*I2C_RATING_FACTOR;i++);
		#endif
		_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
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

static unsigned char ACK(rtl8186_i2c_dev_t* pI2C_Dev)	
{
	int i;
	unsigned int buf;
	//_rtl8186_setGpioDataBit( pI2C_Dev->sdio, 0); /* fall down sdio*/
	
	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
	_rtl8186_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_IN, GPIO_INT_DISABLE);//change sdio to input
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	
	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
		
	_rtl8186_getGpioDataBit(pI2C_Dev->sdio,&buf);
	if (buf != 0) 
		printk("NO ACK\n");
	_rtl8186_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
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

#elif defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621)
static void serial_write(rtl8186_i2c_dev_t* pI2C_Dev, unsigned char *data)
{
	int i;
	char j;
	_rtl8186_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
	for (j=7;j>=0;j--) {
		_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
		_rtl8186_setGpioDataBit( pI2C_Dev->sdio, ((*data)>>j)&0x00000001);//write data,from MSB to LSB
		//delay 2 us.
		#ifdef __kernel_used__
		udelay(2*I2C_RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<2000*I2C_RATING_FACTOR;i++);
		#endif
		_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
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

static unsigned char ACK(rtl8186_i2c_dev_t* pI2C_Dev)	
{
	int i;
	unsigned int buf;
#if 0
	_rtl8186_setGpioDataBit( pI2C_Dev->sdio, 0); /* fall down sdio*/
	_rtl8186_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_IN, GPIO_INT_DISABLE);//change sdio to input
	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
#endif
	//_rtl8186_setGpioDataBit( pI2C_Dev->sdio, 0); /* fall down sdio*/
	
	_rtl8186_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_IN, GPIO_INT_DISABLE);//change sdio to input
	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	
	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
		
	_rtl8186_getGpioDataBit(pI2C_Dev->sdio,&buf);
	if (buf != 0) 
		printk("NO ACK\n");
	//_rtl8186_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
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
static void ACK_w(rtl8186_i2c_dev_t* pI2C_Dev, unsigned char data)	
{
	int i;

#if 0
	_rtl8186_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
	_rtl8186_setGpioDataBit( pI2C_Dev->sdio, data); /* sdio send 0 for ACK, 1 for NACK*/
	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
#endif
	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
	//for (i=0;i<500*I2C_RATING_FACTOR;i++);
	
	_rtl8186_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
	_rtl8186_setGpioDataBit( pI2C_Dev->sdio, data); /* sdio send 0 for ACK, 1 for NACK*/

	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	
	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif

	_rtl8186_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_IN, GPIO_INT_DISABLE);//change sdio to input
	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
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
#endif

static void serial_read(rtl8186_i2c_dev_t* pI2C_Dev, unsigned short int *data)
{
	int i;
	char j;
	unsigned int buf;
	
	_rtl8186_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_IN, GPIO_INT_DISABLE);//change sdio to input
	for (j=7;j>=0;j--) {
		_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
		//delay 2 us.
		#ifdef __kernel_used__
		udelay(2*I2C_RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<2000*I2C_RATING_FACTOR;i++);
		#endif
		_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
		_rtl8186_getGpioDataBit( pI2C_Dev->sdio, &buf);//read data,from MSB to LSB
		*data |= (buf<<j); 
		//delay 1 us.
		#ifdef __kernel_used__
		udelay(1*I2C_RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<1000*I2C_RATING_FACTOR;i++);
		#endif
	}
	_rtl8186_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output	
	return;
}	

#if defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_WM8510)
/*
@func int | _rtl8186_i2c_rawWrite | Write several bits to device
@parm rtl8186_i2c_dev_t* | pLCM_Dev | Structure containing device information
@comm
*/
static void _rtl8186_i2c_rawWrite( rtl8186_i2c_dev_t* pI2C_Dev, unsigned char *pDEV_ID, unsigned char *pReg, unsigned short int *pData)
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
	_rtl8186_setGpioDataBit( pI2C_Dev->sdio, 1); /* raise sdio*/
	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	_rtl8186_setGpioDataBit( pI2C_Dev->sdio, 0); /* fall down sdio*//*start condition*/
	//delay 2 us.
	#ifdef __kernel_used__
	udelay(2*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<2000*I2C_RATING_FACTOR;i++);
	#endif
	
#if 0	
	for (j=7;j>=0;j--) {
		_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
		_rtl8186_setGpioDataBit( pI2C_Dev->sdio, ((*pDEV_ID)>>j)&0x00000001);//write DEV_ID,from MSB to LSB
		//delay 2 us.
		#ifdef __kernel_used__
		udelay(2*I2C_RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<2000*I2C_RATING_FACTOR;i++);
		#endif
		_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
		//delay 1 us.
		#ifdef __kernel_used__
		udelay(1*I2C_RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<1000*I2C_RATING_FACTOR;i++);
		#endif
		if (j == 0) {//check ACK
			_rtl8186_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_IN, GPIO_INT_DISABLE);//change sdio to input
			_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
			//delay 1 us.
			#ifdef __kernel_used__
			udelay(1*I2C_RATING_FACTOR);
			#endif
			#ifdef __test_program_used__
			for (i=0;i<1000*I2C_RATING_FACTOR;i++);
			#endif
			_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
			_rtl8186_getGpioDataBit(pI2C_Dev->sdio,&buf);
			if (buf != 0) {
				printk("NO ACK\n");
				_rtl8186_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
				//goto label;
			}
		_rtl8186_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
		}
	}	
#endif
	serial_write(pI2C_Dev,&dev_id);//write pDEV_ID,from MSB to LSB
	if (ACK(pI2C_Dev) != 0)
		goto start_condition;
	
	serial_write(pI2C_Dev,&reg);//write pReg,from MSB to LSB
	if (ACK(pI2C_Dev) != 0)
		goto start_condition;
	
	serial_write(pI2C_Dev,&data);//write pData,from MSB to LSB
	if (ACK(pI2C_Dev) != 0)
		goto start_condition;
	
	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	_rtl8186_setGpioDataBit( pI2C_Dev->sdio, 1); /* raise sdio*//*stop condition*/
	
	return;	
}	


/*
@func int | _rtl8186_i2c_rawRead | Read several bits from device
@parm rtl8186_i2c_dev_t* | pLCM_Dev | Structure containing device information
@comm
*/
static void _rtl8186_i2c_rawRead( rtl8186_i2c_dev_t* pI2C_Dev, unsigned char *pDEV_ID, unsigned char *pReg, unsigned short int *pData)
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
	_rtl8186_setGpioDataBit( pI2C_Dev->sdio, 1); /* raise sdio*/
	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	_rtl8186_setGpioDataBit( pI2C_Dev->sdio, 0); /* fall down sdio*//*start condition*/
	//delay 2 us.
	#ifdef __kernel_used__
	udelay(2*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<2000*I2C_RATING_FACTOR;i++);
	#endif
	
	serial_write(pI2C_Dev,&dev_id);//write pDEV_ID,from MSB to LSB
	if (ACK(pI2C_Dev) != 0)
		goto start_condition_read;
	
	for (j=7;j>=0;j--) {
		_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
		_rtl8186_setGpioDataBit( pI2C_Dev->sdio, (reg>>j)&0x00000001);//write pReg,from MSB to LSB
		//delay 2 us.
		#ifdef __kernel_used__
		udelay(2*I2C_RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<2000*I2C_RATING_FACTOR;i++);
		#endif
		_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
		//delay 1 us.
		#ifdef __kernel_used__
		udelay(1*I2C_RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<1000*I2C_RATING_FACTOR;i++);
		#endif
	}
	if (ACK(pI2C_Dev) != 0)
		goto start_condition_read;
	//_rtl8186_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_IN, GPIO_INT_DISABLE);//change sdio to input
	//_rtl8186_getGpioDataBit( pI2C_Dev->sdio, &buf);//read data,from LSB of pReg
	//*pData |= (buf<<8);
	
	serial_read(pI2C_Dev,pData);//read data from device
	
	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	_rtl8186_setGpioDataBit( pI2C_Dev->sdio, 1); /* raise sdio*//*stop condition*/
	
	return;	
	
}	

#elif defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621)

/*
@func int | _rtl8186_i2c_rawWrite_alc5621 | Write several bits to device
@parm rtl8186_i2c_dev_t* | pI2C_Dev | Structure containing device information
@parm unsigned char* | pDEV_ID | i2c id address
@parm unsigned char* | pReg | i2c register address
@parm unsigned short int* | pData | i2c data
@comm
*/
static void _rtl8186_i2c_rawWrite_alc5621( rtl8186_i2c_dev_t* pI2C_Dev, unsigned char *pDEV_ID, unsigned char *pReg, unsigned short int *pData)
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
	_rtl8186_setGpioDataBit( pI2C_Dev->sdio, 1); /* raise sdio*/
	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	_rtl8186_setGpioDataBit( pI2C_Dev->sdio, 0); /* fall down sdio*//*start condition*/
	//delay 2 us.
	#ifdef __kernel_used__
	udelay(2*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<2000*I2C_RATING_FACTOR;i++);
	#endif


	serial_write(pI2C_Dev,&dev_id);//write pDEV_ID,from MSB to LSB
	if (ACK(pI2C_Dev) != 0)
		goto start_condition;
	
	serial_write(pI2C_Dev,&reg);//write pReg,from MSB to LSB
	if (ACK(pI2C_Dev) != 0)
		goto start_condition;
	
	serial_write(pI2C_Dev,&data_hibyte);//write pData(hibtye),from MSB to LSB (bit15 - bit 8)
	if (ACK(pI2C_Dev) != 0)
		goto start_condition;

	serial_write(pI2C_Dev,&data_lowbyte);//write pData(lowbtye),from MSB to LSB (bit7 - bit 0)
	if (ACK(pI2C_Dev) != 0)
		goto start_condition;

	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	_rtl8186_setGpioDataBit( pI2C_Dev->sdio, 1); /* raise sdio*//*stop condition*/
	
	return;	
}	


/*
@func int | _rtl8186_i2c_rawRead | Read several bits from device
@parm rtl8186_i2c_dev_t* | pI2C_Dev | Structure containing device information
@parm unsigned char* | pDEV_ID | i2c id address
@parm unsigned char* | pReg | i2c register address
@parm unsigned short int* | pData | i2c data
@comm
*/
static void _rtl8186_i2c_rawRead_alc5621( rtl8186_i2c_dev_t* pI2C_Dev, unsigned char *pDEV_ID, unsigned char *pReg, unsigned short int *pData)
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
	_rtl8186_setGpioDataBit( pI2C_Dev->sdio, 1); /* raise sdio*/
	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	_rtl8186_setGpioDataBit( pI2C_Dev->sdio, 0); /* fall down sdio*//*start condition*/
	//delay 2 us.
	#ifdef __kernel_used__
	udelay(2*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<2000*I2C_RATING_FACTOR;i++);
	#endif
	
	serial_write(pI2C_Dev,&dev_id);//write pDEV_ID,from MSB to LSB
	if (ACK(pI2C_Dev) != 0)
		goto start_condition_read;

	serial_write(pI2C_Dev,&reg);//write pReg,from MSB to LSB
	if (ACK(pI2C_Dev) != 0)
		goto start_condition_read;

	_rtl8186_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
	_rtl8186_setGpioDataBit( pI2C_Dev->sdio, 1); /* raise sdio*/
	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	_rtl8186_setGpioDataBit( pI2C_Dev->sdio, 0); /* fall down sdio*//*start condition*/
	//delay 2 us.
	#ifdef __kernel_used__
	udelay(2*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<2000*I2C_RATING_FACTOR;i++);
	#endif

	dev_id = (*pDEV_ID<<1) | 0x01;	//shift DEV_ID 1-bit left and set bit0 in reading operation

	serial_write(pI2C_Dev,&dev_id);//write pDEV_ID,from MSB to LSB
	if (ACK(pI2C_Dev) != 0)
		goto start_condition_read;

	buf=0;		//init buf data to 0
	serial_read(pI2C_Dev,&buf);//read high byte data from device
	ACK_w(pI2C_Dev, 0);	//write ACK

	serial_read(pI2C_Dev,pData);//read low byte data from device
	ACK_w(pI2C_Dev, 1);	//write NACK

	*pData = *pData | (buf <<8);

	_rtl8186_initGpioPin(pI2C_Dev->sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);//change sdio to output
	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 0); /* fall down sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	_rtl8186_setGpioDataBit( pI2C_Dev->sdio, 0); /* fall down sdio*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	_rtl8186_setGpioDataBit( pI2C_Dev->sclk, 1); /* raise sclk*/
	//delay 1 us.
	#ifdef __kernel_used__
	udelay(1*I2C_RATING_FACTOR);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000*I2C_RATING_FACTOR;i++);
	#endif
	_rtl8186_setGpioDataBit( pI2C_Dev->sdio, 1); /* raise sdio*//*stop condition*/

	return;	

}

#endif

/************************* I2C data struct ****************************************/
static rtl8186_i2c_dev_t i2c_dev;

//-------------------  I2C API ------------------------------//

void init_i2c_gpio(void)
{	
	printk("( GPIO %s )  ", GPIO_I2C );
#if defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_WM8510)
	rtl8186_gpio_async = WM8510_GPIO_PIN;
#elif defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621)
	rtl8186_gpio_async = ALC5621_GPIO_PIN;
#endif
	i2c_dev.sclk = SCLK_PIN;
	i2c_dev.sdio = SDIO_PIN;
	
	_rtl8186_initGpioPin(i2c_dev.sclk, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(i2c_dev.sdio, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	printk("For I2C port init=> OK !\n");
	return;	
}


#if defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_WM8510)
unsigned short int read_WM8510(unsigned char Reg)
{
	unsigned short int buf;
	const unsigned char dev_id = 0x1A;
#ifdef _WM8510_CLI_PROTECT	
	unsigned long flags;
	save_flags(flags); cli();
#endif
	rtl8186_gpio_async = WM8510_GPIO_PIN;	
	_rtl8186_i2c_rawRead(&i2c_dev,&dev_id,&Reg,&buf);
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
	rtl8186_gpio_async = WM8510_GPIO_PIN;	
	_rtl8186_i2c_rawWrite(&i2c_dev,&dev_id,&Reg,&data);
#ifdef _WM8510_CLI_PROTECT	
	restore_flags(flags);
#endif	
	return;
}
#elif defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621)
unsigned short int read_ALC5621(unsigned char Reg)
{
	unsigned short int buf;
	const unsigned char dev_id = 0x1A;
#ifdef _ALC5621_CLI_PROTECT	
	unsigned long flags;
	save_flags(flags); cli();
#endif
	rtl8186_gpio_async = ALC5621_GPIO_PIN;	
	_rtl8186_i2c_rawRead_alc5621(&i2c_dev,&dev_id,&Reg,&buf);
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
	rtl8186_gpio_async = WM8510_GPIO_PIN;	
	_rtl8186_i2c_rawWrite_alc5621(&i2c_dev,&dev_id,&Reg,&data);
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



#endif

/********************* LED_74HC164 data struct******************/
static rtl8186_led_74hc164_dev_t led_74hc164;
//struct timer_list led_74hc164_polling_timer;
//------------- led api-----------------------------------------//

void init_led_74hc164_gpio(void)
{
	printk("( GPIO %s )  ", GPIO_LED_74HC164);
	
	rtl8186_gpio_async = LED_GPIO_PIN;
	led_74hc164.a = LED_74HC164_A;
	led_74hc164.clk = LED_74HC164_CLK;
	//set led_74hc164.a and led_74hc164.clk output
	_rtl8186_initGpioPin(led_74hc164.a, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(led_74hc164.clk, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	
#if LED_POLARITY
	_rtl8186_setGpioDataBit(led_74hc164.a,1); //common-anode
#else
	_rtl8186_setGpioDataBit(led_74hc164.a,0); //common-cathode
#endif	
	_rtl8186_setGpioDataBit(led_74hc164.clk,0);
	
	printk("For LED port init=> OK !\n");
	return;
}

//led_on(16bit):1->on, 0->off. bit15:LED16, bit14:LED15, .....bit1:LED2, bit0:LED1
void led_shower(unsigned short led_on)
{
	unsigned char i, j, on_temp;
	unsigned int count = 0;
	unsigned long flags;
	save_flags(flags); cli();

	rtl8186_gpio_async = LED_GPIO_PIN;
	
#if LED_POLARITY
	led_on = ~led_on;
#endif	
	
	for (j=0; j<LED_NUMBER; j++) {
		_rtl8186_setGpioDataBit(led_74hc164.clk,0); //pull clk down
			
		if (j < LED_NUMBER)	
#if LED_POLARITY				
		_rtl8186_setGpioDataBit(led_74hc164.a,led_on>>(LED_NUMBER-j-1)); //pull a down, common-anode
#else
		_rtl8186_setGpioDataBit(led_74hc164.a,led_on>>(LED_NUMBER-j-1)); //pull a high, common-cathode
#endif							
		for (count=0; count<LED_CLK_FACTOR; count++);
		_rtl8186_setGpioDataBit(led_74hc164.clk,1); //pull clk high
			
			
#ifdef __kernel_used__				
		udelay(LED_MAX_POSITIVE_CLK);
#endif
#ifdef __test_program_used__				
		for (count=0; count<LED_MAX_POSITIVE_CLK; count++);
#endif			
		
	}


#if 0
	if (led_on == 0) {
		for (j=0; j<=LED_NUMBER; j++) {
			_rtl8186_setGpioDataBit(led_74hc164.clk,0); //pull clk down
			
			if (j == 0)
#if LED_POLARITY				
				_rtl8186_setGpioDataBit(led_74hc164.a,1); //pull a high, common-anode
#else
				_rtl8186_setGpioDataBit(led_74hc164.a,0); //pull a down, common-cathode
#endif							
			_rtl8186_setGpioDataBit(led_74hc164.clk,1); //pull clk high
		}
	}
	
	
	for (i=0; i<LED_NUMBER; i++) {
		
		on_temp = ((led_on>>i) & 0x01)? i:0xff;
		if (on_temp == 0xff)
			continue;
				
		for (j=0; j<=LED_NUMBER; j++) {
			_rtl8186_setGpioDataBit(led_74hc164.clk,0); //pull clk down
			
			if (j == 0)
#if LED_POLARITY				
				_rtl8186_setGpioDataBit(led_74hc164.a,0); //pull a down, common-anode
#else
				_rtl8186_setGpioDataBit(led_74hc164.a,1); //pull a high, common-cathode
#endif							
			for (count=0; count<LED_CLK_FACTOR; count++);
			_rtl8186_setGpioDataBit(led_74hc164.clk,1); //pull clk high
			
			if (j == on_temp)
#ifdef __kernel_used__				
				udelay(LED_MAX_POSITIVE_CLK*10);
#endif
#ifdef __test_program_used__				
				for (count=0; count<LED_MAX_POSITIVE_CLK*10000; count++);
#endif			
			else	
				for (count=0; count<LED_CLK_FACTOR; count++);	
	
			if (j == 0)
#if LED_POLARITY			 				
				_rtl8186_setGpioDataBit(led_74hc164.a,1); //pull a high, common-anode
#else
				_rtl8186_setGpioDataBit(led_74hc164.a,0); //pull a down, common-cathode
#endif							
		}	
	
	}
#endif
	
#if LED_POLARITY			 				
	_rtl8186_setGpioDataBit(led_74hc164.a,1); //pull a high, common-anode
#else
	_rtl8186_setGpioDataBit(led_74hc164.a,0); //pull a down, common-cathode
#endif
	_rtl8186_setGpioDataBit(led_74hc164.clk,0);
	
#if 0
	led_74hc164_polling_timer.expires = jiffies + LED_74HC164_RATING_FACTOR;
	led_74hc164_polling_timer.function = led_shower;
	add_timer(&led_74hc164_polling_timer);
#endif	
	restore_flags(flags);
	return;
}
#if 0
void led_74hc164_polling()
{
	init_led_74hc164_gpio();
	led_74hc164_display(0);
	init_timer(&led_74hc164_polling_timer);
	led_74hc164_polling_timer.expires = jiffies + LED_74HC164_RATING_FACTOR;
	led_74hc164_polling_timer.function = led_shower;
	add_timer(&led_74hc164_polling_timer);
	return;
}
module_init(led_74hc164_polling);
#endif
/********************* keypad data struct **********************/
static rtl8186_keypad_dev_t keypad_dev;
keypad_dev_t keypad_data_pool={0,0};
//-------------- keypad api-------------------------------------//
void init_keypad_gpio(void)
{
	printk("( GPIO %s ) ", GPIO_KEYPAD);
	rtl8186_gpio_async = KEYPAD_GPIO_PIN;
	
	keypad_dev.row0 = KEYPAD_ROW_0;
	keypad_dev.row1 = KEYPAD_ROW_1;
	keypad_dev.row2 = KEYPAD_ROW_2;
	keypad_dev.row3 = KEYPAD_ROW_3;
	keypad_dev.row4 = KEYPAD_ROW_4;
	keypad_dev.row5 = KEYPAD_ROW_5;
	keypad_dev.column0 = KEYPAD_COLUMN_0;
	keypad_dev.column1 = KEYPAD_COLUMN_1;
	keypad_dev.column2 = KEYPAD_COLUMN_2;
	keypad_dev.column3 = KEYPAD_COLUMN_3;
	keypad_dev.column4 = KEYPAD_COLUMN_4;
	keypad_dev.column5 = KEYPAD_COLUMN_5;
	keypad_dev.column6 = KEYPAD_COLUMN_6;
	#ifdef KEYPAD_INTERRUPT_ENABLE
	_rtl8186_initGpioPin(keypad_dev.row0, GPIO_DIR_IN, GPIO_INT_ENABLE);
	_rtl8186_initGpioPin(keypad_dev.row1, GPIO_DIR_IN, GPIO_INT_ENABLE);
	_rtl8186_initGpioPin(keypad_dev.row2, GPIO_DIR_IN, GPIO_INT_ENABLE);
	_rtl8186_initGpioPin(keypad_dev.row3, GPIO_DIR_IN, GPIO_INT_ENABLE);
	_rtl8186_initGpioPin(keypad_dev.row4, GPIO_DIR_IN, GPIO_INT_ENABLE);
	_rtl8186_initGpioPin(keypad_dev.row5, GPIO_DIR_IN, GPIO_INT_ENABLE);
	#endif
	#ifdef KEYPAD_INTERRUPT_DISABLE
	_rtl8186_initGpioPin(keypad_dev.row0, GPIO_DIR_IN, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(keypad_dev.row1, GPIO_DIR_IN, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(keypad_dev.row2, GPIO_DIR_IN, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(keypad_dev.row3, GPIO_DIR_IN, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(keypad_dev.row4, GPIO_DIR_IN, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(keypad_dev.row5, GPIO_DIR_IN, GPIO_INT_DISABLE);
	#endif
	_rtl8186_initGpioPin(keypad_dev.column0, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(keypad_dev.column1, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(keypad_dev.column2, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(keypad_dev.column3, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(keypad_dev.column4, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(keypad_dev.column5, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(keypad_dev.column6, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	printk("For KEYPAD port init=> OK !\n");
	
	return;
}

void keyscan(void)
{
	#ifdef INPUT_PULL_HIGH_RESISTOR
	const unsigned char column_port_data[KEYPAD_COLUMN_NUMBER] = {0x7e,0x7d,0x7b,0x77,0x6f,0x5f,0x3f};
	#endif
	#ifdef INPUT_PULL_LOW_RESISTOR
	const unsigned char column_port_data[KEYPAD_COLUMN_NUMBER] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40};
	#endif
	unsigned char column_data, row_data;
	unsigned char column_temp, row_temp, input_data_temp;
	int i;
	unsigned int buf;	
	static unsigned char counter = 0;
	
	rtl8186_gpio_async = KEYPAD_GPIO_PIN;	
	for (column_temp=0; column_temp<KEYPAD_COLUMN_NUMBER; column_temp++)
	{
			
		_rtl8186_setGpioDataBit(keypad_dev.column0,(column_port_data[0]>>column_temp));
		_rtl8186_setGpioDataBit(keypad_dev.column1,(column_port_data[1]>>column_temp));
		_rtl8186_setGpioDataBit(keypad_dev.column2,(column_port_data[2]>>column_temp));
		_rtl8186_setGpioDataBit(keypad_dev.column3,(column_port_data[3]>>column_temp));
		_rtl8186_setGpioDataBit(keypad_dev.column4,(column_port_data[4]>>column_temp));
		_rtl8186_setGpioDataBit(keypad_dev.column5,(column_port_data[5]>>column_temp));
		_rtl8186_setGpioDataBit(keypad_dev.column6,(column_port_data[6]>>column_temp));
		column_data = column_temp;
		//WaitKey();
		for (row_temp=0; row_temp<KEYPAD_ROW_NUMBER; row_temp++)
		{
			row_data = row_temp;
			_rtl8186_getGpioDataBit(*(&keypad_dev.row0+row_temp),&buf);	
			
			if (buf == CHECK_FLAG)
			{
				//debounce check
				#ifdef __kernel_used__
				mdelay(1*KEYPAD_RATING_FACTOR);
				#endif
				#ifdef __test_program_used__
				for (i=0;i<KEYPAD_RATING_FACTOR*100000;i++);
				#endif
				_rtl8186_getGpioDataBit(*(&keypad_dev.row0+row_temp),&buf);
				
				if (buf == CHECK_FLAG)	//finish check 
				{
					printk("  row=%d,column=%d  ",row_data,column_data);	       
					input_data_temp = (row_data<<4) | column_data;         
					if (keypad_data_pool.flags == 0)                       
					{                                                      
						keypad_data_pool.data_string = input_data_temp;
						keypad_data_pool.flags = 1;                    
					}                                                      
				}	
			}//end if	
		}//end for
	}//end for	
		
	

	
#ifdef INPUT_PULL_HIGH_RESISTOR	
	_rtl8186_setGpioDataBit(keypad_dev.column0,0);
	_rtl8186_setGpioDataBit(keypad_dev.column1,0);
	_rtl8186_setGpioDataBit(keypad_dev.column2,0);
	_rtl8186_setGpioDataBit(keypad_dev.column3,0);
	_rtl8186_setGpioDataBit(keypad_dev.column4,0);
	_rtl8186_setGpioDataBit(keypad_dev.column5,0);
	_rtl8186_setGpioDataBit(keypad_dev.column6,0);
#endif

#ifdef INPUT_PULL_LOW_RESISTOR	
	_rtl8186_setGpioDataBit(keypad_dev.column0,1);
	_rtl8186_setGpioDataBit(keypad_dev.column1,1);
	_rtl8186_setGpioDataBit(keypad_dev.column2,1);
	_rtl8186_setGpioDataBit(keypad_dev.column3,1);
	_rtl8186_setGpioDataBit(keypad_dev.column4,1);
	_rtl8186_setGpioDataBit(keypad_dev.column5,1);
	_rtl8186_setGpioDataBit(keypad_dev.column6,1);
#endif
	
	return;
}		
        
/********************* tri_keypad data struct **********************/
static rtl8186_tri_keypad_dev_t tri_keypad_dev;
//keypad_dev_t tri_keypad_data_pool={0,0};
#ifdef _TRI_KEYPAD_POLLING_
struct timer_list tri_keypad_polling_timer;
#endif
//-------------- tri_keypad api-------------------------------------//
void init_tri_keypad_gpio(void)
{
	printk("( GPIO %s ) ", GPIO_TRI_KEYPAD);
	rtl8186_gpio_async = KEYPAD_GPIO_PIN;
	
	tri_keypad_dev.tri_row0 = TRI_KEYPAD_ROW_0;
	tri_keypad_dev.tri_row1 = TRI_KEYPAD_ROW_1;
	tri_keypad_dev.tri_row2 = TRI_KEYPAD_ROW_2;
	tri_keypad_dev.tri_row3 = TRI_KEYPAD_ROW_3;
	tri_keypad_dev.tri_row4 = TRI_KEYPAD_ROW_4;
	tri_keypad_dev.tri_row5 = TRI_KEYPAD_ROW_5;
	tri_keypad_dev.tri_row6 = TRI_KEYPAD_ROW_6;
	tri_keypad_dev.tri_row7 = TRI_KEYPAD_ROW_7;
	tri_keypad_dev.tri_row8 = TRI_KEYPAD_ROW_8;
	tri_keypad_dev.tri_row9 = TRI_KEYPAD_ROW_9;
	tri_keypad_dev.tri_interrupt = TRI_INTERRUPT;
	
	_rtl8186_initGpioPin(tri_keypad_dev.tri_row0, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(tri_keypad_dev.tri_row1, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(tri_keypad_dev.tri_row2, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(tri_keypad_dev.tri_row3, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(tri_keypad_dev.tri_row4, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(tri_keypad_dev.tri_row5, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(tri_keypad_dev.tri_row6, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(tri_keypad_dev.tri_row7, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(tri_keypad_dev.tri_row8, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(tri_keypad_dev.tri_row9, GPIO_DIR_OUT, GPIO_INT_DISABLE);

#ifdef _TRI_KEYPAD_INTERRUPT_	
	_rtl8186_initGpioPin(tri_keypad_dev.tri_interrupt, GPIO_DIR_IN, GPIO_INT_ENABLE);
#endif	
	//Initial state of all data pin(exclusive interrupt pin) is output and set to 0.
	_rtl8186_setGpioDataBit(tri_keypad_dev.tri_row0,0);
	_rtl8186_setGpioDataBit(tri_keypad_dev.tri_row1,0);
	_rtl8186_setGpioDataBit(tri_keypad_dev.tri_row2,0);
	_rtl8186_setGpioDataBit(tri_keypad_dev.tri_row3,0);
	_rtl8186_setGpioDataBit(tri_keypad_dev.tri_row4,0);
	_rtl8186_setGpioDataBit(tri_keypad_dev.tri_row5,0);
	_rtl8186_setGpioDataBit(tri_keypad_dev.tri_row6,0);
	_rtl8186_setGpioDataBit(tri_keypad_dev.tri_row7,0);
	_rtl8186_setGpioDataBit(tri_keypad_dev.tri_row8,0);
	_rtl8186_setGpioDataBit(tri_keypad_dev.tri_row9,0);	
	printk("For TRI_KEYPAD port init=> OK !\n");
	return;
}

void tri_keyscan(void)
{
	unsigned char column_data, row_data;
	unsigned char column_temp, row_temp, input_data_temp;
	int i;
	unsigned int buf, one_more_trigger = 0;
#ifdef _TRI_KEYPAD_POLLING_
	unsigned long flags;
	save_flags(flags); cli();
#endif
	rtl8186_gpio_async = KEYPAD_GPIO_PIN;
		
	for (row_temp=0 ;row_temp<(TRI_KEYPAD_ROW_NUMBER-1) ;row_temp++) {
		_rtl8186_initGpioPin(*(&tri_keypad_dev.tri_row0+row_temp), GPIO_DIR_OUT, GPIO_INT_DISABLE);
		_rtl8186_setGpioDataBit(*(&tri_keypad_dev.tri_row0+row_temp),TRI_CHECK_FLAG);
		row_data = row_temp;
		for (i=row_temp+1 ;i<TRI_KEYPAD_ROW_NUMBER ;i++) {
			_rtl8186_setGpioDataBit(*(&tri_keypad_dev.tri_row0+i),!TRI_CHECK_FLAG);
		}
		
		for (column_temp=row_temp+1 ;column_temp<TRI_KEYPAD_ROW_NUMBER ;column_temp++) {
			column_data = column_temp;
			_rtl8186_initGpioPin(*(&tri_keypad_dev.tri_row0+column_temp), GPIO_DIR_IN, GPIO_INT_DISABLE);
			_rtl8186_getGpioDataBit(*(&tri_keypad_dev.tri_row0+column_temp),&buf);	
			//printk(" buf=%d ",buf);
			if (buf == TRI_CHECK_FLAG) {
				//debounce check
				#ifndef CONFIG_RTK_VOIP_IP_PHONE
				#ifdef __kernel_used__
				mdelay(1*TRI_KEYPAD_RATING_FACTOR);
				#endif
				#ifdef __test_program_used__
				for (i=0;i<TRI_KEYPAD_RATING_FACTOR*100000;i++);
				#endif
				#endif
				_rtl8186_getGpioDataBit(*(&tri_keypad_dev.tri_row0+column_temp),&buf);
				
				if (buf == TRI_CHECK_FLAG)	{//finish check 
					//printk("  row=%d,column=%d  ",row_data,column_data);	       
					input_data_temp = (row_data<<4) | column_data;         
					one_more_trigger++;
					//goto label;
				}	
			}//end if
		}//end for
		_rtl8186_setGpioDataBit(*(&tri_keypad_dev.tri_row0+row_data),!TRI_CHECK_FLAG);
	}//end for
//label:
	for (row_temp=0 ;row_temp<TRI_KEYPAD_ROW_NUMBER ;row_temp++) {
		//Initial state of all data pin(exclusive interrupt pin) is output and set to 0.
		_rtl8186_initGpioPin(*(&tri_keypad_dev.tri_row0+row_temp), GPIO_DIR_OUT, GPIO_INT_DISABLE);
		_rtl8186_setGpioDataBit(*(&tri_keypad_dev.tri_row0+row_temp),TRI_CHECK_FLAG);
	}
#ifdef _TRI_KEYPAD_POLLING_
	//printk("@");
	tri_keypad_polling_timer.expires = jiffies + 1;
	tri_keypad_polling_timer.function = tri_keyscan;
	add_timer(&tri_keypad_polling_timer);
	restore_flags(flags);
#endif		
	if (keypad_data_pool.flags == 0 && one_more_trigger == 1) {                                                      
			keypad_data_pool.data_string = input_data_temp;
			keypad_data_pool.flags = 1;
			//printk("  row=%d,column=%d  ",input_data_temp>>4,input_data_temp&0x0f);                    

#ifdef CONFIG_RTK_VOIP_IP_PHONE
			{
				/* signal to UI */
				extern void keypad_polling_signal_target( const keypad_dev_t *keypad_data_pool );
				keypad_polling_signal_target( &keypad_data_pool );
			}
#endif /* CONFIG_RTK_VOIP_IP_PHONE */
	}  

#ifdef CONFIG_RTK_VOIP_IP_PHONE
	{
		extern void keypad_polling_scan_done( void );
		keypad_polling_scan_done();
	}
#endif
	
	return;
}

#ifdef _TRI_KEYPAD_POLLING_
void tri_keypad_polling()
{
	init_tri_keypad_gpio();
	//del_timer_sync(&tri_keypad_polling_timer);
	init_timer(&tri_keypad_polling_timer);
	tri_keypad_polling_timer.expires = jiffies + 1;
	tri_keypad_polling_timer.function = tri_keyscan;
	add_timer(&tri_keypad_polling_timer);
	return;
}

module_init(tri_keypad_polling);
#endif

#if defined(CONFIG_RTK_VOIP_GPIO_IPP_100)
#define IPONE_HOOK_GPIOD_DIR *((volatile unsigned int *)0xbd010134)
#define IPONE_HOOK_GPIOD_DATA *((volatile unsigned int *)0xbd010130)
#elif defined(CONFIG_RTK_VOIP_GPIO_IPP_101)
#define IPONE_HOOK_GPIOA_DIR *((volatile unsigned int *)0xbd010124)
#define IPONE_HOOK_GPIOA_DATA *((volatile unsigned int *)0xbd010120)
#endif
static void init_iphone_hook_gpio()
{
#if defined(CONFIG_RTK_VOIP_GPIO_IPP_100)
	//GPIOD bit 5 for iphone hook detection
	IPONE_HOOK_GPIOD_DIR &= 0xffdf;
#elif defined(CONFIG_RTK_VOIP_GPIO_IPP_101)
	//GPIOA bit 6 for iphone hook detection
	IPONE_HOOK_GPIOA_DIR &= 0xffbf;
#endif	
	return;
}

//return value 0->on-hook, 1->off-hook
unsigned char iphone_hook_detect()
{
	static unsigned char i = 0;
	if (!i) {
		init_iphone_hook_gpio();
		i = 1;
	}	
#if defined(CONFIG_RTK_VOIP_GPIO_IPP_100)
	return !((IPONE_HOOK_GPIOD_DATA>>21)&0x01);
#elif defined(CONFIG_RTK_VOIP_GPIO_IPP_101)
	return !((IPONE_HOOK_GPIOA_DATA>>6)&0x01);
#endif	
}
#endif // #if 0

