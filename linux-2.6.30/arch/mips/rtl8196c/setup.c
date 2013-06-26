/*
 *
 *  Copyright (c) 2011 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <linux/config.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/tty.h>
#include <linux/serial.h>
#include <linux/serial_core.h>

#include <asm/cpu.h>
#include <asm/bootinfo.h>
#include <asm/irq.h>
#include <asm/serial.h>
#include <asm/io.h>
#include <asm/time.h>

#include <prom.h>
#include <platform.h>

#include <linux/kernel_stat.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/percpu.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/notifier.h>
#include <linux/thread_info.h>
#include <linux/time.h>
#include <linux/jiffies.h>
#include <linux/posix-timers.h>
#include <linux/cpu.h>
#include <linux/syscalls.h>
#include <linux/delay.h>

#include <asm/uaccess.h>
#include <asm/unistd.h>
#include <asm/div64.h>
#include <asm/timex.h>
#include <asm/io.h>

static void __init serial_init(void);
#if 0
static void __init rtl8652_time_init(void);
static void rtl8652_timer_ack(void);

/*
 * Called from the timer interrupt handler to charge one tick to the current 
 * process.  user_tick is 1 if the tick is user time, 0 for system.
 */

void local_timer_interrupt(int irq, void *dev_id)
{
//	profile_tick(CPU_PROFILING);
	account_process_tick(current, user_mode(get_irq_regs()));
}
/*
 * High-level timer interrupt service routines.  This function
 * is set as irqaction->handler and is invoked through do_IRQ.
 */
 
irqreturn_t timer_interrupt(int irq, void *dev_id)
{
	write_seqlock(&xtime_lock);

//Add for update flash check

	rtl8652_timer_ack();
	
	/*
	 * call the generic timer interrupt handling
	 */
	do_timer(1);


	write_sequnlock(&xtime_lock);

	/*
	 * In UP mode, we call local_timer_interrupt() to do profiling
	 * and process accouting.
	 *
	 * In SMP mode, local_timer_interrupt() is invoked by appropriate
	 * low-level local timer interrupt handler.
	 */
	local_timer_interrupt(irq, dev_id);

	return IRQ_HANDLED;
}
#endif
const char *get_system_type(void)
{
   return "RTL8652";
}

void __init prom_init(void)
{
   prom_console_init();
   prom_meminit();
}

void __init plat_mem_setup(void)
{
   /* Platform Specific Setup */
   /* define io/mem region */
    ioport_resource.start = 0x18000000; 
    ioport_resource.end = 0x1fffffff;

    iomem_resource.start = 0x18000000;
    iomem_resource.end = 0x1fffffff;
       
   serial_init();

//   board_time_init = rtl8652_time_init;
//   mips_timer_ack = rtl8652_timer_ack;
}

#if 0
static void rtl8652_timer_ack(void)
{
   REG32(TCIR) |= TC0IP;
}

static void __init rtl8652_time_init(void)
{

}
#endif

extern int  rtl_clockevent_init(int irq);
//void __init plat_time_init(struct irqaction *irq)
void __init plat_time_init(void)
{
   /* Setup Timer0 */

   /* Clear Timer IP status */
   if (REG32(TCIR) & TC0IP)
      REG32(TCIR) |= TC0IP;

   /* Here irq->handler is passed from outside */
  // irq->handler = timer_interrupt;
   //setup_irq(TC0_IRQ, irq);

   REG32(TCCNR) = 0; /* disable timer before setting CDBR */
   /* extend timer base to 4 times for wireless init process */
#if 0
   REG32(CDBR) = (DIVISOR) << DIVF_OFFSET;
   REG32(TC0DATA) = (MHZ * (1000 / HZ)) << TCD_OFFSET;
#else
   REG32(CDBR)=(DIVISOR*4) << DIVF_OFFSET;
   REG32(TC0DATA) = ((MHZ * 250) / HZ) << TCD_OFFSET;
#endif 

    rtl_clockevent_init(TC0_IRQ);
   /* enable timer */
   REG32(TCCNR) = TC0EN | TC0MODE_TIMER;
   REG32(TCIR) = TC0IE;
   #ifdef CONFIG_RTL865X_WTDOG
   *(volatile unsigned long *)(0xb800311C)=0x00600000;
   #endif
}

extern int __init early_serial_setup(struct uart_port *port);
static void __init serial_init(void)
{
#ifdef CONFIG_SERIAL_8250
   struct uart_port s;

   memset(&s, 0, sizeof(s));

   s.line = 0;
   s.type = PORT_16550A;
   //s.membase = (unsigned char *) UART0_BASE;
   s.mapbase = UART0_MAP_BASE;
   s.membase = ioremap_nocache(s.mapbase, 0x20);
   s.irq = UART0_IRQ;
   //s.uartclk = SYSCLK - BAUDRATE * 24;
   s.uartclk = 200000000;
   //s.flags = UPF_SKIP_TEST | UPF_LOW_LATENCY | UPF_SPD_CUST;
   s.flags = UPF_SKIP_TEST | UPF_LOW_LATENCY;
   s.iotype = UPIO_MEM;
   s.regshift = 2;
   //s.fifosize = 1;
   s.fifosize = 16;
  // s.custom_divisor = SYSCLK / (BAUDRATE * 16) - 1;

   /* Call early_serial_setup() here, to set up 8250 console driver */
   if (early_serial_setup(&s) != 0) {
      panic("Serial setup failed!\n");
   }
#endif
}
