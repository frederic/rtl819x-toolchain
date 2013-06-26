#ifndef __BOOTCODE__ 
#include <linux/module.h>
#include <linux/init.h>
#include <linux/serial_8250.h>
#include <linux/serial_reg.h>
#include <linux/version.h>
#include <platform.h>		// GPIO_ABCD_IRQ, PABCD_ISR
#include <asm/delay.h>
#endif

#ifdef __BOOTCODE__
#define ENODEV		19      /* No such device */
#define __init
#endif

#if defined( LINUX_VERSION_CODE ) && (LINUX_VERSION_CODE == KERNEL_VERSION(2,6,30))
#define LINUX_KERNEL_VERSION_2_6_30		1
#endif

#ifdef LINUX_KERNEL_VERSION_2_6_30
#include "bspchip.h"		// BSP_GPIO_EFGH_IRQ
#endif


// ------------------------------------------------------------------
// ------------------------------------------------------------------
// Basic setup information 

#define SC16IS7X0_BAUDRATE	38400

#ifdef CONFIG_SERIAL_SC16IS7X0_XTAL1_CLK_1843200
#define XTAL1_CLK	( SC16IS7X0_BAUDRATE * 3 * 16 )		// 1.8432 MHZ
#elif defined( CONFIG_SERIAL_SC16IS7X0_XTAL1_CLK_14746500 )
#define XTAL1_CLK	( SC16IS7X0_BAUDRATE * 24 * 16 )	// 14.7465 MHZ (NXP demoboard)
#endif

// ------------------------------------------------------------------
// ------------------------------------------------------------------
// I2C level function 

#ifndef __BOOTCODE__
#include "i2c.h"
#else
#include "i2c.h"
#endif

#define I2C_GPIO_ID( port, pin )		( ( ( port - 'A' ) << 16 ) | ( pin ) )

#ifdef CONFIG_RTK_VOIP_GPIO_8954C_V200
#define SC16IS7X0_SCL 		I2C_GPIO_ID( 'G', 1 )	// SCL = G1
#define SC16IS7X0_SDA 		I2C_GPIO_ID( 'G', 0 )	// SDA = G0
#define SC16IS7X0_IRQ 		I2C_GPIO_ID( 'E', 2 )	// IRQ = E2
#define SC16IS7X0_IRQ_NUM	BSP_GPIO_EFGH_IRQ			// BSP_GPIO_EFGH_IRQ=33
#define SC16IS7X0_IRQ_STATUS_BIT	( 1 << ( 0 + 2 ) )		// clean pending status of E2 
#define SC16IS7X0_IRQ_STATUS_REG	PEFGH_ISR
#elif defined (CONFIG_RTK_VOIP_GPIO_8954C_V400)
#error "unknown SC16IS7X0 I2C pin assignment"
#else
#define SC16IS7X0_RESET 	I2C_GPIO_ID( 'A', 4 )	// RESET = D3
#define SC16IS7X0_SCL 		I2C_GPIO_ID( 'A', 3 )	// SCL = D4
#define SC16IS7X0_SDA 		I2C_GPIO_ID( 'A', 5 )	// SDA = D5
#define SC16IS7X0_IRQ 		I2C_GPIO_ID( 'A', 6 )	// IRQ = D6
#define SC16IS7X0_IRQ_NUM	BSP_GPIO_ABCD_IRQ			// GPIO_ABCD_IRQ=16
#define SC16IS7X0_IRQ_STATUS_BIT	( 1 << ( 0 + 6 ) )		// clean pending status of D6 
#define SC16IS7X0_IRQ_STATUS_REG	PABCD_ISR
#endif

#define SC16IS7X0_I2C_ADDR	0x9A
#define SC16IS7X0_I2C_WRITE	0x00
#define SC16IS7X0_I2C_READ	0x01

static i2c_dev_t sc16is7x0_i2c_dev = {
	.sclk	= SC16IS7X0_SCL,
	.sdio	= SC16IS7X0_SDA,
	.irq	= SC16IS7X0_IRQ,
#ifdef SC16IS7X0_RESET
	.reset	= SC16IS7X0_RESET,
#endif
};

// Register address byte
//  7  : not used
//  6~3: A[3:0] UART's internal register select
//  2~1: channel select: CH1 = 0?!, CH0 = 0
//  0  : not used 
#define MK_SC16IS7X0_REG_ADDR( uart_reg )	( ( uart_reg & 0x0F ) << 3 )

unsigned int serial_in_i2c(unsigned int addr, int offset)
{
	unsigned short int data;
	
	//printk( "serial_in_i2c(%X):%X\n", addr, offset );
	
	if( addr != SC16IS7X0_I2C_ADDR )
		return 0;
	
	// start 
	i2c_start_condition( &sc16is7x0_i2c_dev );
	
	// addr + write
	i2c_serial_write_byte( &sc16is7x0_i2c_dev, SC16IS7X0_I2C_ADDR | 
											   SC16IS7X0_I2C_WRITE );
	
	// read ACK 
	if( i2c_ACK( &sc16is7x0_i2c_dev ) != 0 )
		return 0;
	
	// write register address 
	i2c_serial_write_byte( &sc16is7x0_i2c_dev, MK_SC16IS7X0_REG_ADDR( offset ) );
	
	// read ACK 
	if( i2c_ACK( &sc16is7x0_i2c_dev ) != 0 )
		return 0;
	
	// start 
	i2c_start_condition( &sc16is7x0_i2c_dev );

	// addr + read
	i2c_serial_write_byte( &sc16is7x0_i2c_dev, SC16IS7X0_I2C_ADDR | 
											   SC16IS7X0_I2C_READ );
	
	// read ACK 
	if( i2c_ACK( &sc16is7x0_i2c_dev ) != 0 )
		return 0;
		
	// read data
	i2c_serial_read( &sc16is7x0_i2c_dev, &data );
	
	// write negative-ACK
	i2c_ACK_w( &sc16is7x0_i2c_dev, 1 );
	
	// stop
	i2c_stop_condition( &sc16is7x0_i2c_dev );
	
	//printk( "in[%X]\n", data );
	
	return data;
}

unsigned int serial_out_i2c(unsigned int addr, int offset, int value)
{
	//printk( "serial_out_i2c(%X):%X,%X\n", addr, offset, value );

	if( addr != SC16IS7X0_I2C_ADDR )
		return 0;
		
	// start 
	i2c_start_condition( &sc16is7x0_i2c_dev );
	
	// addr + write
	i2c_serial_write_byte( &sc16is7x0_i2c_dev, SC16IS7X0_I2C_ADDR | 
											   SC16IS7X0_I2C_WRITE );
	
	// read ACK 
	if( i2c_ACK( &sc16is7x0_i2c_dev ) != 0 )
		return 0;

	// write register address 
	i2c_serial_write_byte( &sc16is7x0_i2c_dev, MK_SC16IS7X0_REG_ADDR( offset ) );
	
	// read ACK 
	if( i2c_ACK( &sc16is7x0_i2c_dev ) != 0 )
		return 0;
	
	// write data 
	i2c_serial_write_byte( &sc16is7x0_i2c_dev, value );

	// read ACK 
	if( i2c_ACK( &sc16is7x0_i2c_dev ) != 0 )
		return 0;
	
	// stop
	i2c_stop_condition( &sc16is7x0_i2c_dev );
	
	return 0;
}

static void sc16is7X0_set_baudrate(unsigned int baud)
{
	unsigned int divisor;
	unsigned char c;
	
	//setup baudrate and etc (copy from 8250_early.c)
#define serial_out( port, offset, value )	serial_out_i2c( SC16IS7X0_I2C_ADDR, offset, value )
#define serial_in( port, offset )			serial_in_i2c( SC16IS7X0_I2C_ADDR, offset )
	
	//divisor = port->uartclk / (16 * device->baud);
	divisor = XTAL1_CLK / (16 * SC16IS7X0_BAUDRATE);
	c = serial_in(port, UART_LCR);
	serial_out(port, UART_LCR, c | UART_LCR_DLAB);
	serial_out(port, UART_DLL, divisor & 0xff);
	serial_out(port, UART_DLM, (divisor >> 8) & 0xff);
	serial_out(port, UART_LCR, c & ~UART_LCR_DLAB);

#undef serial_out
#undef serial_in
}


static void __init sc16is7x0_init_i2c( void )
{
	// init SCL / SDA 
	i2c_init_SCL_SDA( &sc16is7x0_i2c_dev );
	
	serial_in_i2c( SC16IS7X0_I2C_ADDR, 1 );		// avoid NO ACK at first time access
	
	// Enable IRQ 
	i2c_enable_irq( &sc16is7x0_i2c_dev );
}

// ------------------------------------------------------------------
// ------------------------------------------------------------------
// Early printk or bootcode 

int __init early_sc16is7x0_init_i2c_and_check( void )
{
	unsigned int bak;
	
	// init i2c 
	sc16is7x0_init_i2c();
	
	// do simple test (write non-zero, because NO ACK return zero)
	bak = serial_in_i2c( SC16IS7X0_I2C_ADDR, 1 );
	
	serial_out_i2c( SC16IS7X0_I2C_ADDR, 1, 0x0F );
	if( serial_in_i2c( SC16IS7X0_I2C_ADDR, 1 ) != 0x0F )
		goto label_out;
	
	// restore 
	serial_out_i2c( SC16IS7X0_I2C_ADDR, 1, bak );
	
	return 0;

label_out:
	return -ENODEV;
}

#ifdef LINUX_KERNEL_VERSION_2_6_30
int __init early_sc16is7x0_setup(struct console * console, char * option)
{
	int ret;
	unsigned int divisor;
	unsigned char c;
	
	ret = early_sc16is7x0_init_i2c_and_check();
	
	if( ret )
		return ret;
	
	// setup baudrate and etc (copy from 8250_early.c)
#define serial_out( port, offset, value )	serial_out_i2c( SC16IS7X0_I2C_ADDR, offset, value )
#define serial_in( port, offset )			serial_in_i2c( SC16IS7X0_I2C_ADDR, offset )
	
	serial_out(port, UART_LCR, 0x3);	/* 8n1 */
	serial_out(port, UART_IER, 0);		/* no interrupt */
	serial_out(port, UART_FCR, 0);		/* no fifo */
	serial_out(port, UART_MCR, 0x3);	/* DTR + RTS */

	//divisor = port->uartclk / (16 * device->baud);
	divisor = XTAL1_CLK / (16 * SC16IS7X0_BAUDRATE);
	c = serial_in(port, UART_LCR);
	serial_out(port, UART_LCR, c | UART_LCR_DLAB);
	serial_out(port, UART_DLL, divisor & 0xff);
	serial_out(port, UART_DLM, (divisor >> 8) & 0xff);
	serial_out(port, UART_LCR, c & ~UART_LCR_DLAB);

#undef serial_out
#undef serial_in
	
	return 0;
}
#endif

// ------------------------------------------------------------------
// ------------------------------------------------------------------
// Bootcode wrapper 
#if defined( __BOOTCODE__ ) || defined( LINUX_KERNEL_VERSION_2_6_30 )
unsigned int sc16is7x0_serial_out_i2c(int offset, int value)
{
	return serial_out_i2c( SC16IS7X0_I2C_ADDR, offset, value );
}

unsigned int sc16is7x0_serial_in_i2c(int offset)
{
	return serial_in_i2c( SC16IS7X0_I2C_ADDR, offset );
}
#endif

// ------------------------------------------------------------------
// ------------------------------------------------------------------
// Platform level function 
#ifndef __BOOTCODE__
int sc16is7x0_clean_interrupt( int irq )
{
	if( irq != SC16IS7X0_IRQ_NUM )
		return 0;	// not my irq 
	
	// clean my bit only 
	if( REG32( SC16IS7X0_IRQ_STATUS_REG ) & SC16IS7X0_IRQ_STATUS_BIT ) {
		REG32( SC16IS7X0_IRQ_STATUS_REG ) = SC16IS7X0_IRQ_STATUS_BIT;	
		return 0;
	}
	
	return -1;	// not my gpio  
}
#endif

// ------------------------------------------------------------------
// ------------------------------------------------------------------
// UART level function 
#ifndef __BOOTCODE__

#ifdef LINUX_KERNEL_VERSION_2_6_30
#define PORT_TYPE	PORT_16654
#define PORT_FLAGS	/*UPF_BOOT_AUTOCONF |*/ UPF_SKIP_TEST | UPF_FIXED_TYPE | UPF_SHARE_IRQ
#else
#define PORT_TYPE	0	// autoconf (2.6.19)
#define PORT_FLAGS	/*UPF_BOOT_AUTOCONF |*/ UPF_SKIP_TEST
#endif

#define PORT(_base,_irq)				\
	{						\
		.type		= PORT_TYPE,	\
		.iobase		= _base,			\
		.irq		= _irq,			\
		.uartclk	= XTAL1_CLK,	\
		.iotype		= UPIO_I2C,		\
		.flags		= PORT_FLAGS,	\
	}

static struct plat_serial8250_port sc16is7x0_data[] = {
	PORT(SC16IS7X0_I2C_ADDR, SC16IS7X0_IRQ_NUM),	
	{ },
};

static struct platform_device sc16is7x0_device = {
	.name			= "serial8250",
	.id			= PLAT8250_DEV_SC16IS7X0,
	.dev			= {
		.platform_data	= sc16is7x0_data,
	},
};

void __init sc16is7x0_get_port( struct uart_port *port )
{
	// call by static void __init serial_init(void)
	// in linux-2.6.x/arch/mips/realtek/rtl8196b/setup.c 
	memset(port, 0, sizeof(*port));
	
	port ->type = PORT_16654;
	port ->iobase = SC16IS7X0_I2C_ADDR;
	port ->irq = SC16IS7X0_IRQ_NUM;
	port ->uartclk = XTAL1_CLK;
#ifdef LINUX_KERNEL_VERSION_2_6_30
	port ->flags = PORT_FLAGS;
#else
	//port ->flags = UPF_SKIP_TEST | UPF_LOW_LATENCY | UPF_SPD_CUST;
	port ->flags = UPF_SKIP_TEST | UPF_SPD_CUST;
	//port ->flags = UPF_SKIP_TEST | UPF_SPD_CUST | UART_CAP_FIFO | UART_CAP_EFR | UART_CAP_SLEEP
#endif
	port ->iotype = UPIO_I2C;
	port ->fifosize = 1;
#ifdef LINUX_KERNEL_VERSION_2_6_30
	// linux 2.6.30's early_serial_setup()r doesn't copy custom_divisor 
#else
	port ->custom_divisor = XTAL1_CLK / (SC16IS7X0_BAUDRATE * 16);
#endif
}

static int __init sc16is7x0_init(void)
{
	int ret;

	sc16is7x0_init_i2c();
	
	ret = platform_device_register(&sc16is7x0_device);

	//enable irq
        REG32(BSP_GIMR) |= BSP_GPIO_ABCD_IE;
	return ret;
}

module_init(sc16is7x0_init);

MODULE_AUTHOR("Realtek");
MODULE_DESCRIPTION("8250 serial probe module for SC16IS7x0");
MODULE_LICENSE("GPL");
#endif

