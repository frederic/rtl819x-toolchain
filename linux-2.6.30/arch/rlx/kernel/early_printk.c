/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2002, 2003, 06, 07 Ralf Baechle (ralf@linux-mips.org)
 * Copyright (C) 2007 MIPS Technologies, Inc.
 *   written by Ralf Baechle (ralf@linux-mips.org)
 */
#include <linux/console.h>
#include <linux/init.h>

#include <asm/setup.h>

#ifdef CONFIG_SERIAL_SC16IS7X0_CONSOLE
extern int __init early_sc16is7x0_init_i2c_and_check( void );
extern int __init early_sc16is7x0_setup(struct console * console, char * option);
static int early_sc16is7x0_workable = -1;
#endif

#include "bspchip.h"

//extern void prom_putchar(char);
int prom_putchar(char c)
{
#ifdef CONFIG_SERIAL_SC16IS7X0_CONSOLE
   extern unsigned int sc16is7x0_serial_out_i2c(int offset, int value);
   extern unsigned int sc16is7x0_serial_in_i2c(int offset);
#endif

#define UART0_BASE		0xB8002000
#define UART0_THR		(UART0_BASE + 0x000)
#define UART0_FCR		(UART0_BASE + 0x008)
#define UART0_LSR       (UART0_BASE + 0x014)
#define TXRST			0x04
#define CHAR_TRIGGER_14	0xC0
#define LSR_THRE		0x20
#define TxCHAR_AVAIL	0x00
#define TxCHAR_EMPTY	0x20
   unsigned int busy_cnt = 0;

#ifndef CONFIG_SERIAL_SC16IS7X0_CONSOLE
   do
   {
      /* Prevent Hanging */
      if (busy_cnt++ >= 30000)
      {
         /* Reset Tx FIFO */
         REG8(UART0_FCR) = TXRST | CHAR_TRIGGER_14;
         return 0;
      }
   } while ((REG8(UART0_LSR) & LSR_THRE) == TxCHAR_AVAIL);

   /* Send Character */
   REG8(UART0_THR) = c;
#endif
	
   // -------------------------------------------------------
   	
#ifdef CONFIG_SERIAL_SC16IS7X0_CONSOLE
   if( early_sc16is7x0_workable < 0 )
      return 0;
   
   #define MEM2REG( x )		( ( x - UART0_BASE ) / 4 )
   
   do
   {
      /* Prevent Hanging */
      if (busy_cnt++ >= 30000)
      {
         /* Reset Tx FIFO */
         sc16is7x0_serial_out_i2c( MEM2REG(UART0_FCR), TXRST | CHAR_TRIGGER_14 );
         
         return 0;
      }
   } while ((sc16is7x0_serial_in_i2c( MEM2REG(UART0_LSR) ) & LSR_THRE) == TxCHAR_AVAIL);

   /* Send Character */
   sc16is7x0_serial_out_i2c( MEM2REG(UART0_THR), c );
   
   #undef MEM2REG
#endif

   return 1;
}

static void __init
early_console_write(struct console *con, const char *s, unsigned n)
{
	while (n-- && *s) {
		if (*s == '\n')
			prom_putchar('\r');
		prom_putchar(*s);
		s++;
	}
}

static struct console early_console __initdata = {
	.name	= "early",
	.write	= early_console_write,
	.flags	= CON_PRINTBUFFER | CON_BOOT,
	.index	= -1
};

static int early_console_initialized __initdata;

void __init setup_early_printk(void)
{
	if (early_console_initialized)
		return;
	early_console_initialized = 1;

#ifdef CONFIG_SERIAL_SC16IS7X0_CONSOLE
	early_sc16is7x0_workable = early_sc16is7x0_setup( NULL, NULL );
#endif
	register_console(&early_console);
}
