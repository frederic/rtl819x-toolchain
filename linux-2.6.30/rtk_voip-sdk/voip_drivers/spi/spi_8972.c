#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/string.h>
#include <linux/delay.h>
#include "rtk_voip.h"
#include "gpio/gpio.h"
#include "spi.h"

extern rtl_spi_dev_t spi_dev[SPI_DEV_NUM];

/************************************* Set GPIO Pin to SPI ***********************************************************/
/*
@func int32 | _rtl867x_spi_init | Initialize SPI device
@parm rtl_spi_dev_t* | pDev | Structure to store device information
@parm uint32 | gpioSCLK | GPIO ID of SCLK
@parm uint32 | gpioCS_ | GPIO ID of CS_
@parm uint32 | gpioSDI | GPIO ID of SDI
@parm uint32 | gpioSDO | GPIO ID of SDO
@parm uint32 | maxSpeed | how fast SPI driver can generate the SCLK signal (unit: HZ)
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
static int32 _rtl867x_spi_init( rtl_spi_dev_t* pDev, uint32 gpioSCLK, uint32 gpioCS_, uint32 gpioSDO, uint32 gpioSDI)
{
	
	pDev->gpioSCLK = gpioSCLK;
	pDev->gpioCS_ = gpioCS_;
	pDev->gpioSDI = gpioSDI;
	pDev->gpioSDO = gpioSDO;
	//pDev->SClkDelayLoop = SysClock / maxSpeed;
	/*rtlglue_printf("GetSysClockRate()=%d\n",GetSysClockRate());*/
#if 0 // conflicts with function prototype
	_rtl867x_initGpioPin( gpioSCLK, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	_rtl867x_initGpioPin( gpioCS_, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	_rtl867x_initGpioPin( gpioSDI, GPIO_DIR_IN, GPIO_INT_DISABLE );
	_rtl867x_initGpioPin( gpioSDO, GPIO_DIR_OUT, GPIO_INT_DISABLE );
#else
	_rtl867x_initGpioPin( gpioSCLK, GPIO_DIR_OUT);
	_rtl867x_initGpioPin( gpioCS_, GPIO_DIR_OUT);
	_rtl867x_initGpioPin( gpioSDI, GPIO_DIR_IN);
	_rtl867x_initGpioPin( gpioSDO, GPIO_DIR_OUT);
	
#endif
	return SUCCESS;
}




/*
@func int32 | _rtl867x_spi_exit | Called when a SPI device is released
@parm rtl_spi_dev_t* | pDev | Structure containing device information
@rvalue SUCCESS | success.
@rvalue FAILED | failed. Parameter error.
@comm
*/
int32 _rtl867x_spi_exit( rtl_spi_dev_t* pDev )
{
	return SUCCESS;
}
 
void init_channel(int channel, uint32 pin_cs, uint32 pin_reset, uint32 pin_clk, uint32 pin_do, uint32 pin_di)
{
	int i;
	static unsigned char inital = 0;
#if 0   // conflicts with function prototype
	_rtl867x_initGpioPin(pin_reset, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl867x_initGpioPin(pin_cs, GPIO_DIR_OUT, GPIO_INT_DISABLE);
#else
	_rtl867x_initGpioPin(pin_reset, GPIO_DIR_OUT);
	_rtl867x_initGpioPin(pin_cs, GPIO_DIR_OUT);

#endif    
	_rtl867x_spi_init(&spi_dev[channel], pin_clk, pin_cs, pin_do, pin_di);

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8671
	static int delay_cnt = 50000;
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8672
	static int delay_cnt = 100000;
#endif
	
	if ((channel == 0) || (inital == 0)) 
	{
		#if speed_booting_rating
			for(i=0;i<delay_cnt;i++);
		#else
			cyg_thread_delay(15);
		#endif
		_rtl867x_setGpioDataBit(pin_cs, 1); 	/* CS_ preset to high state*/
		_rtl867x_setGpioDataBit(pin_clk, 1); 	/* SCLK to high state*/
		
		_rtl867x_setGpioDataBit(pin_reset, 1);	// Reset high
		#if speed_booting_rating
			for(i=0;i<delay_cnt;i++);
		#else
			cyg_thread_delay(15);
		#endif
		_rtl867x_setGpioDataBit(pin_reset, 0);	// set reset low, PCLK and FS id present and stable.
		#if speed_booting_rating
			for(i=0;i<delay_cnt;i++);
		#else
			cyg_thread_delay(15);
		#endif
		_rtl867x_setGpioDataBit(pin_reset, 1);	// release reset
		#if speed_booting_rating
			for(i=0;i<delay_cnt;i++);
		#else
			cyg_thread_delay(15);
		#endif				// wait more than 100ms 
			inital++;

	}
}

//------------------------------------------
void init_spi(int ch_spi)
{
	
	printk("( GPIO %s )  ", GPIO );
	
	if (ch_spi == 0)
	{
		printk("for SLIC[%d]...", ch_spi);
		init_channel(0, PIN_CS1, PIN_RESET1, PIN_CLK, PIN_DO, PIN_DI);
		
		printk("=> OK !\n");
	}
	else if (ch_spi == 1)
	{
		printk("for SLIC/DAA[%d]...", ch_spi);
		init_channel(1, PIN_CS3_DAA, PIN_RESET3_DAA, PIN_CLK_DAA, PIN_DO_DAA, PIN_DI_DAA);
		printk("=> OK !\n");
	}
	else 
	{
		printk("No GPIO Pin assign for any device -> Can't initialize any device\n");
		
	}
	
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8672      
#if 0
        //system configuration

        SPIDBG("\r\ninit_spi");
        val = readl (RT8670_SICR_BASE);
        val |= (1<<6);
        writel(val, RT8670_SICR_BASE);

        spi_reg->SPICNR = SPI_CSP_LOW_ACTIVE |SPI_DAISY_ENABLE|SPI_CMD_READ|SPI_WLEN_8BIT |\
                                  SPI_MODE_MSB|SPI_CSTIME_16BIT ;
        //clear interrupt pending bits
        SPI_CLEAR_READ_INTR();
        SPI_CLEAR_WRITE_INTR();
#endif
#endif
	
	
}

#if 0
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8672

#define SPI_8672_DEBUG
#ifdef SPI_8672_DEBUG
#define SPIDBG( args...)	\
	do {			\
		printk(args);	\
	} while(0)
#else /* CONFIG_MTD_DEBUG */
#define DEBUG(n, args...) do { } while(0)
#endif /* CONFIG_MTD_DEBUG */

unsigned int spi_dbg = 0;
void spi_dbg_on(){spi_dbg = 1;}
void spi_dbg_off(){spi_dbg = 0;}

#define         CYCLES_PER_SPI_SCLK  (int)(218000000/(35000000/32))
#define RT8670_SICR_BASE                        0xB8003300
#define RTL8671_SPI_BASE                        0xB9C02000
typedef struct rtl_8672_reg
{
        unsigned int SPICNR;
        unsigned int SPISTR;
        unsigned int SPICKDIV;
        unsigned int SPIDATAR;
} rtl_8672_reg_t ;

rtl_8672_reg_t  *spi_reg = (rtl_8672_reg_t*) RTL8671_SPI_BASE;

#define get_spi_reg() (rtl_8672_reg_t *)(RTL8671_SPI_BASE)
//#define CONFIG_867X_SPI_CONTROL

#define readl(x)                (*(volatile uint32_t *)x)
#define writel(val, addr)         (void)((*(volatile unsigned int *) (addr)) = (val))

//#define SPICNR                                                REG32(RTL8671_SPI_BASE +0x00)//*(volatile int *)(RTL8671_SPI_BASE +0x00)
//#define SPISTR                                                REG32(RTL8671_SPI_BASE +0x04)//*(volatile int *)(RTL8671_SPI_BASE +0x04)
//#define SPICKDIV                                              REG32(RTL8671_SPI_BASE +0x08)//*(volatile int *)(RTL8671_SPI_BASE +0x08)
//#define SPIDATAR                                      REG32(RTL8671_SPI_BASE +0x0C)//*(volatile int *)(RTL8671_SPI_BASE +0x0c)
// SPI Control Register
#define SPI_ENABLE                                      (0x01 << 0)
#define SPI_CSP_LOW_ACTIVE              (0x00 << 1)
#define SPI_CSP_HIGH_ACTIVE             (0x01 << 1)
#define SPI_DAISY_ENABLE                        (0x01 << 2)
#define SPI_DAISY_DISABLE                       (0x00 << 2)
#define SPI_START                                       (0x01 << 4)
#define SPI_CMD_READ                            (0x00 << 5)
#define SPI_CMD_WRITE                           (0x01 << 5)
#define SPI_WLEN_8BIT                           (0x00 << 6)
#define SPI_WLEN_16BIT                          (0x01 << 6)
#define SPI_MODE_MSB                            (0x00 << 7)
#define SPI_MODE_LSB                            (0x01 << 7)
#define SPI_RDIE_ENABLE                 (0x01 << 24)
#define SPI_WDIE_ENABLE                 (0x01 << 25)
#define SPI_CSTIME_16BIT                        (0x01 << 26)
#define SPI_CSTIME_8BIT                         (0x00 << 26)
//SPI Status Register
#define SPI_IS_READ_INTR()                      (spi_reg->SPISTR & (0x01 << 31)) >>31
#define SPI_CLEAR_READ_INTR()           spi_reg->SPISTR |= (0x01 << 31)
#define SPI_IS_WRITE_INTR()                     (spi_reg->SPISTR & (0x01 << 30)) >>30
#define SPI_CLEAR_WRITE_INTR()          spi_reg->SPISTR |= (0x01 << 30)
//SPI CLOCK DIVSOR REGISTER
#define SPI_CLK_DIV32                           (0x00 << 30)
#define SPI_CLK_DIV64                           (0x01 << 30)

/************************* SPI API ****************************************/
void spi_dump_regs()
{
        rtl_8672_reg_t  *spi;
        unsigned int    xx;

        /* System configuration */
        spi = get_spi_reg();
        SPIDBG("\r\nSome related regs:");
        SPIDBG("\r\n GIMR [%08X]: %08X",0xB8003000, *(volatile int*)0xB8003000);
        SPIDBG("\r\n GISR [%08X]: %08X",0xB8003004, *(volatile int*)0xB8003004);
        SPIDBG("\r\n PINMUXCR [%08X]: %08X",0xB8003300, *(volatile int*)0xB8003300);
        SPIDBG("\r\n PINSR [%08X]: %08X",0xB8003308, *(volatile int*)0xB8003308);
        SPIDBG("\r\n MECR [%08X]: %08X",0xB800330C, *(volatile int*)0xB800330C);
        SPIDBG("\r\n=======   SPI   ===========\r\n");
        SPIDBG("SPICNR: 0x%08X \r\n", spi->SPICNR);
        SPIDBG("SPISTR: 0x%08X \r\n", spi->SPISTR);
        SPIDBG("SPICKDIV: 0x%08X \r\n", spi->SPICKDIV);
        SPIDBG("SPIDATAR: 0x%08X \r\n", spi->SPIDATAR);
        SPIDBG("=======   GPIO   ===========");

        for (xx = 0xB8003500; xx<0xB8003538; xx+=4)
        {
                SPIDBG("\r\n[%08X]:0x%x",xx, *(volatile int*)xx);
        }
}

void spi_cpm_regs()
{
        rtl_8672_reg_t  *spi;
        unsigned int    xx;

        /* System configuration */
        SPIDBG("\r\n PCM related regs:");
        SPIDBG("\r\n GIMR [%08X]: %08X",0xB8003000, *(volatile int*)0xB8003000);
        SPIDBG("\r\n GISR [%08X]: %08X",0xB8003004, *(volatile int*)0xB8003004);
        SPIDBG("\r\n IRR2 [%08X]: %08X",0xB800300C, *(volatile int*)0xB800300C);
        SPIDBG("\r\n PINMUXCR [%08X]: %08X",0xB8003300, *(volatile int*)0xB8003300);
        SPIDBG("\r\n PINSR [%08X]: %08X",0xB8003308, *(volatile int*)0xB8003308);
        SPIDBG("\r\n ->MECR [%08X]: %08X",0xB800330C, *(volatile int*)0xB800330C);
}

unsigned char dbg_read_spi(unsigned int reg)
{
        uint8 buf;
        unsigned long flags;

        save_flags(flags); cli();

        buf = reg | 0x80;
        _rtl867x_spi_rawWrite(&spi_dev[cur_channel], &buf, 8);

        _rtl867x_spi_rawRead(&spi_dev[cur_channel], &buf, 8);

        restore_flags(flags);

        return buf;
}

unsigned char dbg_write_spi(unsigned int reg, unsigned char data)
{
        uint8 buf;
        unsigned long flags;

        save_flags(flags); cli();

        buf = reg;
        _rtl867x_spi_rawWrite(&spi_dev[cur_channel], &buf, 8);

        buf = data;
        _rtl867x_spi_rawWrite(&spi_dev[cur_channel], &buf, 8);

        restore_flags(flags);
}

#endif //CONFIG_RTK_VOIP_DRIVERS_PCM8672
#endif //#if 0

