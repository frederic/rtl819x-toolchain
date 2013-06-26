/*
* Copyright c                  Realtek Semiconductor Corporation, 2006  
* All rights reserved.
* 
* Program : GPIO Header File 
* Abstract : 
* Author :                
* 
*/

 
#ifndef __GPIO_8651BV_H
#define __GPIO_8651BV_H

/*==================== FOR RTL865x EVB gpio pin ==================*/

//#define CONFIG_RTK_VOIP_DRIVERS_PCM8651_T	//if T-version board used
//#define CONFIG_RTK_VOIP_DRIVERS_PCM8651_2S_OC

////// For 8651B/BV EV Board /////
#ifdef CONFIG_RTK_VOIP_GPIO_8651B

#define GPIO "A"
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651_T  // T-version board

#define PIN_CS2		GPIO_ID(GPIO_PORT_C,0)
#define PIN_RESET2	GPIO_ID(GPIO_PORT_C,0)

#define PIN_INT1	GPIO_ID(GPIO_PORT_E,1)
#define PIN_CS1		GPIO_ID(GPIO_PORT_E,6)
#define PIN_RESET1	GPIO_ID(GPIO_PORT_C,5)

#define PIN_CLK		GPIO_ID(GPIO_PORT_B,5)
#define PIN_DI		GPIO_ID(GPIO_PORT_B,6)
#define PIN_DO		GPIO_ID(GPIO_PORT_B,7)

#define PIN_RELAY	GPIO_ID(GPIO_PORT_E,0)
#define PIN_LED1	GPIO_ID(GPIO_PORT_D,6)
#define PIN_LED2	GPIO_ID(GPIO_PORT_D,7)

#elif defined CONFIG_RTK_VOIP_DRIVERS_PCM8651_2S_OC //OC's board

#define GPIO "DE"
/*Slic used*/
#define PIN_RESET1	GPIO_ID(GPIO_PORT_D,8)	// No Usage for OC board
#define PIN_CS1		GPIO_ID(GPIO_PORT_E,4)	//output
#define PIN_CLK		GPIO_ID(GPIO_PORT_E,0)	//output
#define PIN_DI		GPIO_ID(GPIO_PORT_D,0)  //input
#define PIN_DO		GPIO_ID(GPIO_PORT_E,1)	//output

//DAA common GPIO pin
#define PIN_RESET3_DAA		GPIO_ID(GPIO_PORT_D,9)  // No Usage for OC board
#define PIN_CLK_DAA		GPIO_ID(GPIO_PORT_E,2)	//output
#define PIN_DO_DAA		GPIO_ID(GPIO_PORT_E,3)	//output

//DAA 1 GPIO pin
#define PIN_CS3_DAA		GPIO_ID(GPIO_PORT_E,7)	//output
#define PIN_DI_DAA		GPIO_ID(GPIO_PORT_D,3) 	//input

//DAA 2 GPIO pin
#define PIN_CS3_DAA_2		GPIO_ID(GPIO_PORT_E,6)	//output
#define PIN_DI_DAA_2		GPIO_ID(GPIO_PORT_D,2) 	//input

/*
DO(SPI_DX0) 
D1(SPI_DX1) 
D2(SPI_DX2) 
D3(SPI_DX3)

E1(SPI_DR0)
E3(SPI_DR2)

EO(SPI_CK0)
E2(SPI_CK2)

E[4..7](SPI_CS[0..3]) 

SLIC: set 0
DAA: set 3

*/

// for pass compile
#define PIN_RELAY	GPIO_ID(GPIO_PORT_A,1)
#define PIN_LED1	GPIO_ID(GPIO_PORT_A,1)
#define PIN_LED2	GPIO_ID(GPIO_PORT_A,1)
#define PIN_CS2		GPIO_ID(GPIO_PORT_A,1)
#define PIN_RESET2	GPIO_ID(GPIO_PORT_A,1)

#else	// RTK-version board

// For SLIC 0
#define PIN_INT1	GPIO_ID(GPIO_PORT_E,1)
#define PIN_CS1		GPIO_ID(GPIO_PORT_E,3)
#define PIN_RESET1	GPIO_ID(GPIO_PORT_E,7)
#define PIN_LED1	GPIO_ID(GPIO_PORT_C,4)
// For SLIC 1
#define PIN_INT2	GPIO_ID(GPIO_PORT_E,2)
#define PIN_CS2		GPIO_ID(GPIO_PORT_C,0)
#define PIN_RESET2	GPIO_ID(GPIO_PORT_D,5)
#define PIN_LED2	GPIO_ID(GPIO_PORT_C,5)
// For SLIC 0, 1 share
#define PIN_CLK		GPIO_ID(GPIO_PORT_E,4)
#define PIN_DI		GPIO_ID(GPIO_PORT_E,6)
#define PIN_DO		GPIO_ID(GPIO_PORT_E,5)
// For Relay Control
#define PIN_RELAY	GPIO_ID(GPIO_PORT_E,0)

//#define PIN_CS1	GPIO_ID(GPIO_PORT_D,3)
//#define PIN_RESET1	GPIO_ID(GPIO_PORT_D,6)
//#define PIN_INT1	GPIO_ID(GPIO_PORT_D,4)
//#define PIN_CLK	GPIO_ID(GPIO_PORT_D,2)
//#define PIN_DI	GPIO_ID(GPIO_PORT_D,1)
//#define PIN_DO	GPIO_ID(GPIO_PORT_D,0)

//#define PIN_CS1_SiLab		GPIO_ID(GPIO_PORT_D,3)
//#define PIN_RESET1_SiLab	GPIO_ID(GPIO_PORT_D,6)
//#define PIN_INT1_SiLab	GPIO_ID(GPIO_PORT_D,4)
//#define PIN_CLK_SiLab		GPIO_ID(GPIO_PORT_D,2)
//#define PIN_DI_SiLab		GPIO_ID(GPIO_PORT_D,1)
//#define PIN_DO_SiLab		GPIO_ID(GPIO_PORT_D,0)

#endif	//endif CONFIG_RTK_VOIP_DRIVERS_PCM8651_T
#endif

/*====================END FOR RTL865x EVB gpio pin ==================*/

/*==================== FOR RTL865x ==================*/

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8651

#include "asicRegs.h"

#define PDECNR			0xbd012070
//#define PDEDIR			0xbd012074
#define PDEDATA			0xbd012078

#define PECNR			(PDECNR+1)
#define PEDIR			(PDEDIR+1)
#define PEDATA			(PDEDATA+1)

/* define GPIO port */
enum GPIO_PORT
{
	GPIO_PORT_A = 0,
	GPIO_PORT_B,
	GPIO_PORT_C,
	GPIO_PORT_D,
	GPIO_PORT_E,
	GPIO_PORT_F,
	GPIO_PORT_G,
	GPIO_PORT_H,
	GPIO_PORT_I,
	GPIO_PORT_MAX,
};

/* define GPIO dedicate peripheral pin */
enum GPIO_PERIPHERAL
{
	GPIO_PERI_GPIO = 0,
	GPIO_PERI_TYPE0 = 0x2,
	GPIO_PERI_TYPE1 = 0x3,
};


/* define GPIO direction */
enum GPIO_DIRECTION
{
	GPIO_DIR_IN = 0,
	GPIO_DIR_OUT,
};

/* define GPIO Interrupt Type */
enum GPIO_INTERRUPT_TYPE
{
	GPIO_INT_DISABLE = 0,
	GPIO_INT_FALLING_EDGE,
	GPIO_INT_RISING_EDGE,
	GPIO_INT_BOTH_EDGE,
};

int32 _rtl865x_initGpioPin(uint32 gpioId, enum GPIO_PERIPHERAL dedicate, 
                                           enum GPIO_DIRECTION, 
                                           enum GPIO_INTERRUPT_TYPE interruptEnable );
int32 _rtl865x_getGpioDataBit( uint32 gpioId, uint32* data );
int32 _rtl865x_setGpioDataBit( uint32 gpioId, uint32 data );
#if 0
int32 _rtl865x_fetchGpioInterruptStatus( uint32 gpioId, uint32* status );
int32 _rtl865x_clearGpioInterruptStatus( uint32 gpioId );
#endif

#endif //CONFIG_RTK_VOIP_DRIVERS_PCM8651

#endif /* __GPIO_8651BV_H */
