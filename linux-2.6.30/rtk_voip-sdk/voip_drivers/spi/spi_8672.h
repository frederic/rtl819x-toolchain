#ifndef _SPI_8672_H_
#define _SPI_8672_H_


/* SPI Interface Set number for SLIC, DAA Device */
/* Programer must modify these setting for a new EV Board */
#ifdef CONFIG_RTK_VOIP_MULTIPLE_SI32178
#define SPI_DEV_NUM	4
#define SLIC0_SPI_DEV	0
#define SLIC1_SPI_DEV	1
#define SLIC2_SPI_DEV	2
#define SLIC3_SPI_DEV	3
#define DAA0_SPI_DEV	-1	// for pass compile
#define DAA1_SPI_DEV	-1	// for pass compile
#elif defined(CONFIG_RTK_VOIP_SLIC_SI32176_SI32178_CS)
#define SPI_DEV_NUM	2	// this means total SPI interface set number
#define SLIC0_SPI_DEV	0	// this means SLIC use SPI set 0
#define SLIC1_SPI_DEV	1
#define DAA0_SPI_DEV	1	// this means DAA port 0 use SPI set 1
#define DAA1_SPI_DEV	-1
#elif defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK)
#define SPI_DEV_NUM	2	// this means total SPI interface set number
#define SLIC0_SPI_DEV	0	// this means SLIC use SPI set 0
#define SLIC1_SPI_DEV	1
#define DAA0_SPI_DEV	1	// this means DAA port 0 use SPI set 1
#define DAA1_SPI_DEV	-1
#else
#define SPI_DEV_NUM	2	// this means total SPI interface set number
#define SLIC0_SPI_DEV	0	// this means SLIC use SPI set 0
#define SLIC1_SPI_DEV	-1
#define DAA0_SPI_DEV	1	// this means DAA port 0 use SPI set 1
#define DAA1_SPI_DEV	-1
#endif //!CONFIG_RTK_VOIP_MULTIPLE_SI32178

#define spi_rw_delay() spi_nsdelay(200) // 200 ns
// th: For 8672, if no rw delay for Zarlink V890 series, SLIC init fail.

#ifdef CONFIG_RTK_VOIP_8672_SPI_GPIO
//int32 _rtl867x_spi_init( rtl_spi_dev_t* pDev, uint32 gpioSCLK, uint32 gpioCS_, uint32 gpioSDO, uint32 gpioSDI, int32 maxSpeed );
//int32 _rtl867x_spi_exit( rtl_spi_dev_t* pDev );
//void init_channel(int channel, uint32 pin_cs, uint32 pin_reset, uint32 pin_clk, uint32 pin_do, uint32 pin_di);
void init_spi(int ch_spi);

#if defined (CONFIG_RTK_VOIP_SLIC_SI32178) || defined (CONFIG_RTK_VOIP_SLIC_SI32176_SI32178)
#define speed_booting_rating 0	// Si32178 need more time when reset
#else
#define speed_booting_rating 1
#endif

extern void cyg_thread_delay(int delay);

#endif // CONFIG_RTK_VOIP_8672_SPI_GPIO

#ifdef CONFIG_RTK_VOIP_8672_SHARED_SPI

/* SPI Flash Controller */
/*
 *	Debug macros
 */
//#define SPI_8672_DEBUG
#ifdef SPI_8672_DEBUG
#define SPIDBG( args...)				\
	do {						\
			printk(args);		\
	} while(0)
#else /* SPI_8672_DEBUG */
#define SPIDBG(n, args...) do { } while(0)

#endif /* SPI_8672_DEBUG */

/*
 *  8672 dependet register definitions and macros **
 */
#define RTL8672_SPI_BASE 			0xB8009000
#define RT8672_GIMR					0xB8003000
#define RT8672_PINMUX				0xB8003300
#define RT8672_GPIO_BASE			0xB8003500
#define RT8672_PINSR				0xB8003308
/*
 * Macro Definition
 */
 /*-- SPICNR --*/
#define SPI_SPIE(i)			((i) << 0)   /* 0: disable, 1: Enable*/
#define SPI_CSP(i)			((i) << 1)   /* 0: Low active, 1: High active, */
#define SPI_DAISY(i)			((i) << 2)   /* 0: Daisy chain, 1:Non-Daisy chain*/
#define SPI_START(i)			((i) << 4)   /* 1: Start transfer*/
#define SPI_CMD(i)			((i) << 5)   /* 0: read, 1: write*/
#define SPI_WLEN(i)			((i) << 6)   /* 0: 8-bit mode, 1: 16-bit mode*/
#define SPI_LSB(i)			((i) << 7)   /* 0: MSB, 1: LSB*/
#define SPI_RDIE(i)			((i) << 24)   /* 0: Disable , 1: Enable read ok interrupt */
#define SPI_WRIE(i)			((i) << 25)   /* 0: Disable , 1: Enable write ok interrupt */
#define SPI_CSTIME(i)			((i) << 26)   /* 0: 8-bit times , 1: 16-bit times of CS raise time */
 /*-- SPISTR --*/
#define SPI_RDIP(i)			((i) << 31)   /* Read ok interrupt pending bit, 1 to clear  */
#define SPI_WDIP(i)			((i) << 30)   /* Write ok interrupt pending bit, 1 to clear  */
/*-- SPICLKDIV --*/
#define SPI_DIV(i)			((i) << 30)   /* Clock divisor, 0: /32, 1: /64, 2: /128, 3: /256*/

typedef struct rtl_8672_reg
{
	unsigned int SPICNR;
	unsigned int SPISTR;
	unsigned int SPICKDIV;
	unsigned int SPIRDR;
	unsigned int SPITDR;
} rtl_8672_reg_t ;

#define get_spi_reg() (rtl_8672_reg_t *)(RTL8672_SPI_BASE)

/*
 * PIN definitions, board dependent
 */
//-- SLIC and DAA can't use daisychain, 2 CS is MUST.


#ifdef CONFIG_6028_IAD_BGA_PIN_DEF
#define RESET_PIN   3
#define CS_USED_DAA 5
#define PIN_RELAY   4
#define CS_USED_SLIC    5
#endif
#ifdef CONFIG_6166_IAD_SILAB3217X
#define RESET_PIN	28  /*D4*/  //3
#define CS_USED_DAA	8   /*B0*/ //5   
#define PIN_RELAY	27  /*D3*/  //4
#define CS_USED_SLIC	8   /*B0*/  //5
#endif

#define readl(x)		(*(volatile uint32_t *)x)
#define writel(val, addr)	  (void)((*(volatile unsigned int *) (addr)) = (val))

//#define SPICNR 						REG32(RTL8671_SPI_BASE +0x00)//*(volatile int *)(RTL8671_SPI_BASE +0x00)
//#define SPISTR 						REG32(RTL8671_SPI_BASE +0x04)//*(volatile int *)(RTL8671_SPI_BASE +0x04)
//#define SPICKDIV						REG32(RTL8671_SPI_BASE +0x08)//*(volatile int *)(RTL8671_SPI_BASE +0x08)
//#define SPIDATAR					REG32(RTL8671_SPI_BASE +0x0C)//*(volatile int *)(RTL8671_SPI_BASE +0x0c)
// SPI Control Register

unsigned char read_spi_hw(unsigned int chid, unsigned int reg);
void write_spi_hw(unsigned int chid, unsigned int reg, unsigned int data);
unsigned char read_spi_hw_daa(int chid, unsigned int reg);
void write_spi_hw_daa(int chid, unsigned int reg, unsigned int data);
unsigned char read_spi_nodaisy_hw(unsigned int reg);
void write_spi_nodaisy_hw(unsigned int reg, unsigned int data);

#endif // CONFIG_RTK_VOIP_8672_SHARED_SPI

#endif // _SPI_8672_H_


