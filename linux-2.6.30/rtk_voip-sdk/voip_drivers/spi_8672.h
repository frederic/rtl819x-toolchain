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


#ifdef CONFIG_BGA_PIN_DEF
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


