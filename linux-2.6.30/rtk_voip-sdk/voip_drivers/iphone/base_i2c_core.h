// move to voip_drivers/i2c.h

#if 0
#ifndef __BASE_I2C_CORE_H__
#define __BASE_I2C_CORE_H__

//-----------------------------
#if 1	//0: kernel used(udelay), 1: test program used(for(;;)) 
#define __test_program_used__
#else
#define __kernel_used__
#endif

#if defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_WM8510)
//----------------------- WM8510 codec ----------------------------------------------
#define I2C_RATING_FACTOR	10	//Adjust I2C read and write rating.
//There is an I2C protocal.
#define _I2C_WM8510_ 	//GPIO pin
//cli() protection for kernel used. Don't define MACRO _WM8510_CLI_PROTECT in test program.
//#define _WM8510_CLI_PROTECT

#elif defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621)
//----------------------- ALC5621 codec ----------------------------------------------
#define I2C_RATING_FACTOR	10	//Adjust I2C read and write rating.
//There is an I2C protocal.
#define _I2C_ALC5621_ 	//GPIO pin
//cli() protection for kernel used. Don't define MACRO _WM8510_CLI_PROTECT in test program.
  #if defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V00 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V01 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V99 )
    #define _ALC5621_CLI_PROTECT	// 8972 share ALC PIN to other 
  #endif
#endif

/*********************** I2C data struct ********************************************/
typedef struct i2c_dev_s
{
	//unsigned int i2c_reset;		//output
	unsigned int sclk;		//output
	unsigned int sdio;		//input or output	
} i2c_dev_t;	

extern i2c_dev_t i2c_dev;

/*********************** I2C API for base_ic2_xxx ********************************************/

extern void i2c_serial_write(i2c_dev_t* pI2C_Dev, unsigned char *data);

extern unsigned char i2c_ACK(i2c_dev_t* pI2C_Dev);

extern void i2c_ACK_w(i2c_dev_t* pI2C_Dev, unsigned char data);

extern void i2c_serial_read(i2c_dev_t* pI2C_Dev, unsigned short int *data);


#endif /* __BASE_I2C_CORE_H__ */
#endif

